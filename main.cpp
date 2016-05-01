/* ECcomm - Communicate with microcontroller interface for
            General Motors Entertainment & Comfort serial bus.    

    Copyright (C) 2016 Stuart V. Schmitt

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <unistd.h> // Required for STDIN_FILENO on some systems
#include <fcntl.h>
#include <termios.h>
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <map>
#include "ECpacket.h"
#include "ECcmdmap.h"

using namespace std;
void SerialLoop(int& tty_fd, unsigned char& mode, keymap_t& cmd_map, descmap_t& descriptions, termios& stdio);
void LineParse(istringstream& linestream, int& tty_fd, keymap_t& cmd_map, descmap_t& descriptions);




int main(int argc, char** argv) {
	int tty_fd;

	tty_fd = open(argv[1], O_RDWR | O_NONBLOCK);      
	if (tty_fd == -1) {
		cout << "Error opening serial port." << endl;
		return 1;
	}
	else {
		unsigned char mode; // byte that gets used for a few tasks (be careful!)

		cout << endl;
		cout << "ECcomm 1.0.0" << endl;
		cout << "Copyright (C) 2016 Stuart V. Schmitt" << endl;
		cout << endl;
		cout << "This program comes with ABSOLUTELY NO WARRANTY; for details press TAB and" << endl;
		cout << "type 'show w'. This is free software, and you are welcome to redistribute" << endl;
		cout << "it under certain conditions; for details, read the COPYING file or visit" << endl;
		cout << "http://www.gnu.org/licenses/gpl-3.0-standalone.html" << endl;
		cout << endl;
		cout << "To quit, press ESC, CTRL-C, or CTRL-D." << endl;
		cout << "To enable local decoding of E&C bus messages, press CTRL-B." << endl;
		cout << "To enable plain text display of E&C bus messages, press SPACE." << endl;
		cout << "To manually enter an E&C command, press TAB or CTRL-I." << endl;
		cout << "To reload and show keymap configuration, press CTRL-R." << endl;
		cout << "To show current keymap, press CTRL-S." << endl;
		cout << endl;

		termios serial, stdio, sane_stdio;

		tcgetattr(tty_fd, &serial);
		// The following four lines are from the termios(3) man page.
		serial.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
		serial.c_oflag &= ~OPOST;
		serial.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		serial.c_cflag &= ~(CSIZE | PARENB);
		serial.c_cflag |= CS8 | CREAD | CLOCAL; // 8-bit, enable rx, ignore control lines
		serial.c_cc[VMIN] = 1;
		serial.c_cc[VTIME] = 0;
		cfsetospeed(&serial, B9600);
		cfsetispeed(&serial, B9600);
		tcsetattr(tty_fd, TCSANOW, &serial);

		// Save the original STDIO settings to be restored at program exit
		tcgetattr(STDIN_FILENO, &sane_stdio);
		stdio = sane_stdio;
		stdio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
		stdio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		stdio.c_cflag &= ~(CSIZE | PARENB);
		stdio.c_cc[VMIN] = 1;
		tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
		fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);		// make the reads non-blocking

		keymap_t cmd_map;
		descmap_t descriptions;
		cmd_map = MakeCMDMap(descriptions);
		// Flush out the buffers
		tcflush(tty_fd, TCIOFLUSH);
		tcflush(STDIN_FILENO, TCIFLUSH);
		// Wait two seconds to give the Arduino a chance to finish booting up
		clock_t start = clock();
		while ((clock() - start) / (double) CLOCKS_PER_SEC < 2);
		mode = 2;  // Run loop, display instructions first time manual code entry occurs
		// Sending the 2 puts the interface in binary mode and checks serial link
		if (write(tty_fd, &mode, 1) == -1)
			cout << "Serial port send error" << endl;
		else
			while (mode)
				SerialLoop(tty_fd, mode, cmd_map, descriptions, stdio);
		close(tty_fd);
		tcsetattr(STDOUT_FILENO, TCSANOW, &sane_stdio);  // restore the STDIO settings
		return 0;
	}
}




void SerialLoop(int& tty_fd, unsigned char& mode, keymap_t& cmd_map, descmap_t& descriptions, termios& stdio) {
	unsigned char key, input;
	const unsigned char timeout = 3;
	if (read(tty_fd, &input, 1) > 0) {
		// If an ESC comes in, 11 more bytes will follow with packet and timestamp information.
		// The program will do nothing else while waiting for those bytes to come in.
		// The read aborts if a timeout occurs, or if an incorrectly formatted byte comes in.
		if (input == 27) {
			unsigned char repaired = 0;
			unsigned long timer = 0;
			unsigned long packet = 0;
			clock_t start;
			start = clock();
			while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			if (input > 31 && input < 96) {
			 timer = input - 32;
			 start = clock();
			 while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			 if (input > 31 && input < 96) {
			  timer += ((unsigned long) input - 32) << 6;
			  start = clock();
			  while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			  if (input > 31 && input < 96) {
			   timer += ((unsigned long) input - 32) << 12;
			   start = clock();
			   while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			   if (input > 31 && input < 96) {
			    timer += ((unsigned long) input - 32) << 18;
			    start = clock();
			    while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			    if (input > 31 && input < 96) {
			     timer += ((unsigned long) input - 32) << 24;
			     start = clock();
			     while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			     if (input > 31 && input < 96) {
			      timer += (((unsigned long) input - 32) & 3) << 30;
			      repaired = ((input - 32) & 12) >> 2;
			      packet = (input - 32) >> 4;
			      start = clock();
			      while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			       if (input > 31 && input < 96) {
			        packet += ((unsigned long) input - 32) << 2;
				    start = clock();
			        while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			        if (input > 31 && input < 96) {
			         packet += ((unsigned long) input - 32) << 8;
			         start = clock();
			         while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			         if (input > 31 && input < 96) {
			          packet += ((unsigned long) input - 32) << 14;
			          start = clock();
			          while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			          if (input > 31 && input < 96) {
			           packet += ((unsigned long) input - 32) << 20;
			           start = clock();
			           while(read(tty_fd, &input, 1) < 1 && (clock() - start)/ (double) CLOCKS_PER_SEC < timeout);
			           if (input > 31 && input < 96)
			            packet += ((unsigned long) input - 32) << 26;
			}}}}}}}}}}
			cout << ECtimerString(timer, repaired) << ECnumString(packet) << ECbinaryString(packet)
				 << ECdescription(packet) << endl;
		}
		else {
			cout << input;
		}
	}
	if (read(STDIN_FILENO, &key, 1) > 0) {
		unsigned long command = 0;
		switch (key) {
		case ' ': // Space. Start ASCII data.
		case 2:   // CTRL-B. Start binary data.
			if (write(tty_fd, &key, 1) == -1)
				cout << "Serial port send error" << endl;
			break;
		case 3:   // CTRL-C. Exit.
		case 4:   // CTRL-D. Exit.
			mode = 0;
			break;
		case 9:	  // CTRL-I or TAB. Manual input.
			{
				stdio.c_iflag |= ICRNL;
				stdio.c_lflag |= ECHO | ICANON | IEXTEN | ISIG;
				tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
				int opts = fcntl(STDIN_FILENO, F_GETFL);
				fcntl(STDIN_FILENO, F_SETFL, opts & ~O_NONBLOCK);
				if (mode == 2) {
					cout << endl;
					cout << "Manually enter either an E&C command or a key mapping. For example," << endl;
					cout << "  3-44-127" << endl;
					cout << "will send that command immediately after ENTER is pressed. Typing" << endl;
					cout << "  map q to 3-44-127" << endl;
					cout << "will add that command to the key map, so that future 'q' keystrokes will" << endl;
					cout << "result in transmission of that command. CTRL-R will clear this map entry." << endl;
					cout << endl;
					mode = 1;
				}
				cout << ">> ";
				string line;
				getline(cin, line);
				istringstream linestream(line);
				LineParse(linestream, tty_fd, cmd_map, descriptions);
				fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
				stdio.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
				stdio.c_iflag &= ~ICRNL;
				tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
				cin.clear();
			}
			break;
		case 13:  // CTRL-M or ENTER. Line feed.
			cout << endl;
			break;
		case 18:  // CTRL-R. Reload config files.
			cmd_map.clear();
			descriptions.clear();
			cmd_map = MakeCMDMap(descriptions);
			// no break--proceed to show keymap
		case 19:  // CTRL-S. Show keymap.
			PrintMap(cmd_map, descriptions);
			break;
		case 27:  // ESC, but filtering out multibyte escape codes
			if (read(STDIN_FILENO, &key, 1) > 0) 
				while (read(STDIN_FILENO, &key, 1) > 0);
			else
				mode = 0;
			break;
		default:  // Search for the keystroke in the map and send associated command if found
			command = FindCMD(cmd_map, key);
			if (command) {
				char send[] = {27, command & 255, (command >> 8) & 255, (command >> 16) & 255, (command >> 24) & 255};
				if (write(tty_fd, &send, 5) == -1)
					cout << "Serial port send error" << endl;
			}
		}
	}
}




void LineParse(istringstream& linestream, int& tty_fd, keymap_t& cmd_map, descmap_t& descriptions) {
	string chunk;
	linestream >> chunk;
	linestream.seekg(0);
	if (chunk == "map")
		ParseMapLine(cmd_map, descriptions, linestream);
	else if (chunk == "show") {
		linestream >> chunk;
		linestream.get();
		if (linestream.get() == 'w') {
			cout << endl;
			cout << "THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY" << endl;
			cout << "APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT" << endl;
			cout << "HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY" << endl;
			cout << "OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO," << endl;
			cout << "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR" << endl;
			cout << "PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM" << endl;
			cout << "IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF" << endl;
			cout << "ALL NECESSARY SERVICING, REPAIR OR CORRECTION." << endl;
			cout << endl;
		}
	}
	else {
		linestream.clear();
		keymap_t manual_map;
		char key = 0;
		ParseCodeString(manual_map, linestream, key);
		unsigned long command = FindCMD(manual_map, key);
		if (command) {
			unsigned char send[] = {27, command & 255, (command >> 8) & 255, (command >> 16) & 255, (command >> 24) & 255};
			if (write(tty_fd, &send, 5) == -1)
				cout << "Serial port send error." << endl;
		}
		else
			cout << "Invalid command." << endl;
	}
}
