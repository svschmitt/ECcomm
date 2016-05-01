/* ECcmdmap.cpp - Part of ECcomm - Loads keymapped E&C commands

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

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include "ECpacket.h"
using namespace std;
typedef map<char, unsigned long> keymap_t;
typedef map<char, string> descmap_t;
void ReadCMDFile(ifstream& cfg_file, keymap_t& cmd_map, descmap_t& descriptions);
void ParseMapLine(keymap_t& cmd_map, descmap_t& descriptions, istringstream& linestream);
void ParseCodeString(keymap_t& cmd_map, descmap_t& descriptions, istringstream& codestream, char& key, string desc);
void ParseCodeString(keymap_t& cmd_map, istringstream& codestream, char& key);




keymap_t MakeCMDMap(descmap_t& descriptions) {
	keymap_t cmd_map;
	ifstream cfg_file;

	// Read the configuration in the local directory
	const char *home;
	if ((home = getenv("HOME")) == NULL) {
		home = getpwuid(getuid())->pw_dir;
	}
	string userfile(home);
	userfile += "/.ECcomm";
	cfg_file.open(userfile.c_str());
	if (cfg_file.is_open()) {
		ReadCMDFile(cfg_file, cmd_map, descriptions);
		cfg_file.close();
	}

	cfg_file.clear();

	// Read the configuration in the local directory. Will overwrite conflicting home directory entries.
	cfg_file.open("ECcomm.cfg");
	if (cfg_file.is_open()) {
		ReadCMDFile(cfg_file, cmd_map, descriptions);
		cfg_file.close();
	}

	return cmd_map;
}




void ReadCMDFile(ifstream& cfg_file, keymap_t& cmd_map, descmap_t& descriptions) {
	string line;
	while (getline(cfg_file, line)) {
		istringstream linestream(line);
		ParseMapLine(cmd_map, descriptions, linestream);
	}
}




void ParseMapLine(keymap_t& cmd_map, descmap_t& descriptions, istringstream& linestream) {
	string chunk;
	char key;
	linestream >> chunk;
	if (chunk == "map") {
		linestream >> chunk;
		if (chunk.length() == 1) {
			key = chunk[0];
			linestream >> chunk;
			if (chunk == "to") {
				linestream >> chunk;
				istringstream codestream(chunk);
				// Remove whitespace to get to start of description.
				while (linestream.peek() == ' ' || linestream.peek() == 9)
					linestream.get();
				getline(linestream, chunk);
				ParseCodeString(cmd_map, descriptions, codestream, key, chunk);
			}
		}
	}
}




void ParseCodeString(keymap_t& cmd_map, descmap_t& descriptions, istringstream& codestream, char& key, string desc) {
	unsigned char priority = 0;
	unsigned char address = 0;
	unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;
	string chunk;
	getline(codestream, chunk, '-');
	if (!codestream.eof() && CheckByte(chunk, priority, 1, 3)) {
		getline(codestream, chunk, '-');
		if (!codestream.eof() && CheckByte(chunk, address, 0, 63)) {
			getline(codestream, chunk, '-');
			if (CheckByte(chunk, byte1, 0, 255)) {
				if (codestream.eof()) {
					// good packet with 1 byte
					// cmd_map.insert(make_pair(key, BuildPacket(priority, address, byte1, 0, 0)));
					cmd_map[key] = BuildPacket(priority, address, byte1, 0, 0);
					descriptions[key] = desc;
				}
				else {
					getline(codestream, chunk, '-');
					if (CheckByte(chunk, byte2, 0, 255)) {
						if (codestream.eof()) {
							// good packet with 2 bytes
							// cmd_map.insert(make_pair(key, BuildPacket(priority, address, byte1, byte2, 0)));
							cmd_map[key] = BuildPacket(priority, address, byte1, byte2, 0);
							descriptions[key] = desc;
						}
						else {
							getline(codestream, chunk, '-');
							if (CheckByte(chunk, byte3, 0, 255)) {
								if (codestream.eof()) {
									// good packet with 3 bytes
									// cmd_map.insert(make_pair(key, BuildPacket(priority, address, byte1, byte2, byte3)));
									cmd_map[key] = BuildPacket(priority, address, byte1, byte2, byte3);
									descriptions[key] = desc;
								}
							}
						}
					}
				}
			}
		}
	}
}




void ParseCodeString(keymap_t& cmd_map, istringstream& codestream, char& key) {
	descmap_t null_map;
	string null_str;
	ParseCodeString(cmd_map, null_map, codestream, key, null_str);
}




unsigned long FindCMD(keymap_t& cmd_map, char key) {
	for (keymap_t::iterator i = cmd_map.begin(); i != cmd_map.end(); i++) {
		if (i->first == key)
			return i->second;
	}
	return 0;
}


void PrintMap(keymap_t& cmd_map, descmap_t& descriptions) {
	if (cmd_map.size() > 0) {
		cout << "================================================================" << endl;
		cout << "  KEY  MESSAGE           DESCRIPTION" << endl;
		cout << "----------------------------------------------------------------" << endl;
		/* Iterate on the description map rather than on the key map. We can use
		   the FindCMD() function to retrieve the codes from the key map. */
		for (descmap_t::iterator i = descriptions.begin(); i != descriptions.end(); i++) {
			string code = ECnumString(FindCMD(cmd_map, i->first));
			ostringstream whitespace;
			for (char j = code.length(); j < 18; j++) {
				whitespace << ' ';
			}
			cout << "   " << i->first << "   " << code << whitespace.str()
			     << i->second << endl;
		}
		cout << "================================================================" << endl;
	}
}
