/* ECpacket.cpp - Part of ECcomm - Subroutines for handling E&C packets

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

#include <string>
#include <sstream>

using namespace std;
unsigned BitRange(unsigned long packet, unsigned char first, unsigned char last);



string ECtimerString(unsigned long timer, unsigned char repaired) {
	ostringstream timer_ostr;
	timer_ostr << "     " << timer;
	if (repaired)
		timer_ostr << "# ";
	else
		timer_ostr << ": ";
	unsigned char length = timer_ostr.tellp();
	return timer_ostr.str().substr(length - 7, 7);
}




string ECnumString(unsigned long packet) {
	unsigned char priority = packet & 3;
	unsigned char address = (packet & 252) >> 2;
	unsigned long byte1 = (packet & 0xFF00) >> 8;
	unsigned long byte2 = (packet & 0xFF0000) >> 16;
	unsigned long byte3 = (packet & 0xFF000000) >> 24;
	ostringstream output;
	output << (int) priority << '-' << (int) address << '-' << byte1;
	if (byte2 || byte3)
		output << '-' << byte2;
	if (byte3)
		output << '-' << byte3;
	unsigned char length = output.tellp();
	for (unsigned char i = 0; i < (17 - length); i++)
		output << ' ';
	return output.str();
}




string ECbinaryString(unsigned long packet) {
	ostringstream output;
	unsigned char bits = 10;
	unsigned char parity = 0;
	while (packet >> bits)
		bits += 2;
	output << "1 ";
	for (unsigned char i = 0; i < bits; i++) {
		parity ^= (packet & 1);
		if (packet & 1)
			output << '1';
		else
			output << '0';
		if ((i == 1 || i%8 == 7) && i != (bits - 1))
			output << ' ';
		packet >>= 1;
	}
	output << ' ' << (int) parity;
	unsigned char length = output.tellp();
	for (unsigned char i = 0; i < (42 - length); i++)
		output << ' ';
	return output.str();
}




string ECdescription(unsigned long packet) {
	ostringstream output;
	switch (packet & 3) {
	case 3:
		switch ((packet >> 2) & 63) {
		case 57:
			if (BitRange(packet, 0, 6) == 46)
				output << "To radio: load memory " << BitRange(packet, 7, 8);
			break;
		case 52:
			switch (BitRange(packet, 0, 8)) {
			case 6:
				output << "To radio: unmute OnStar audio";
				break;
			case 302: // 46-1
				output << "To radio: OnStar call established and underway";
				break;
			}
			break;
		case 51:
			switch (BitRange(packet, 0, 7)) {
			case 100:
				output << "To OnStar: Is module present?";
				break;
			case 127:
				output << "To OnStar: Source button was pressed during call";
				break;
			}
			break;
		case 44:
			switch (BitRange(packet, 0, 7)) {
			case 193:
				output << "Radio: Cassette player selected";
				break;
			case 163:
				output << "Radio: OnStar selected";
				break;
			case 127: // discovered by trying it
				output << "To radio: What is power status?";
				break;
			case 124:
				output << "To radio: Prepare for OnStar audio and enable status polling";
				break;
			case 120:
				output << "To radio: Deselect OnStar audio and disable status polling";
				break;
			case 99:
				output << "Radio: CD changer selected";
				break;
			case 3:
				output << "Radio: No external audio source selected";
				break;
			}
			break;
		case 41:
			switch (BitRange(packet, 0, 9)) {
			case 531: // 19-2
				output << "To IPM: Temperature up button pressed";
				break;
			case 521: // 9-2
				output << "To CD changer (anomalous address): Eject magazine";
				break;
			case 515: // 3-2
				output << "To IPM: Temperature down button pressed";
				break;
			case 3:
				output << "To IPM: Temperature button released";
				break;
			}
			break;
		case 40:
			if (BitRange(packet, 0, 7) == 127)
				output << "To radio: OnStar module acknowledges power status";
			break;
		case 30:
			output << "To CD changer: ";
			switch (BitRange(packet, 0, 7)) {
			case 221:
				output << "Random mode (221), data: " << BitRange(packet, 8, 24);
				break;
			case 217:
				output << "Load disc " << 10*BitRange(packet, 13, 16) + BitRange(packet, 9, 12);
				break;
			case 201:
				if (BitRange(packet, 8, 9) == 2)
					output << "Seek to " << 10*BitRange(packet, 14, 17) + BitRange(packet, 10, 13) << " seconds";
				break;
			case 177:
				output << "Load track " << 10*BitRange(packet, 12, 15) + BitRange(packet, 8, 11) << " and seek to 0:00";
				break;
			case 169:
				output << "Fast reverse scan, speed " << BitRange(packet, 8, 9) << " of 4";
				break;
			case 161:
				output << "Fast forward scan, speed " << BitRange(packet, 8, 9) << " of 4";
				break;
			case 153:
				switch (BitRange(packet, 8, 13)) {
				case 63:
					output << "Upload the disc data";
					break;
				case 0:
					output << "Is module present?";
					break;
				}
				break;
			case 145:
				output << "Stop playback";
				break;
			case 93:
				output << "Random mode (93), data: " << BitRange(packet, 8, 24);
				break;
			case 65:
				output << "Seek to " << 10*BitRange(packet, 13, 16) + BitRange(packet, 9, 12)
					   << " minutes, pre-flag " << BitRange(packet, 8, 8);
				break;
			case 61:
				output << "OK";
				break;
			case 49:
				output << "Load track " << 10*BitRange(packet, 12, 15) + BitRange(packet, 8, 11)
					   << ", wait for further seek instruction";
				break;
			case 41:
				output << "Fast reverse scan, speed " << BitRange(packet, 8, 9) << " of 4";
				break;
			case 33:
				output << "Fast forward scan, speed " << BitRange(packet, 8, 9) << " of 4";
				break;
			case 17:
				output << "Stop; no magazine present";
				break;
			case 9:
				output << "Request status, track, and minute";
				break;
			case 1:
				output << "Button released";
				break;
			}
			break;
		case 28:
			output << "To cassette: ";
/*			switch (BitRange(packet, 0, 7)) {
			case 60:
				output << "query 60";
				break;
			case 56:
				output << "query 56";
				break;
			case 52:
				output << "forward scan";
				break;
			case 44:
				output << "fast forward";
				break;
			case 28:
				output << "switch playback side";
				break;
			case 24:
				output << "query 24";
				break;
			case 20:
				output << "reverse scan";
				break;
			case 12:
				output << "rewind";
				break;
			case 2:
				output << "stop";
				break;
			}*/
			break;
		}
		break;
	case 2:
		switch ((packet >> 2) & 63) {
		case 40:
			if (BitRange(packet, 0, 1) == 1) {
				output << "Clock time " << BitRange(packet, 8, 12) << ':';
				if (BitRange(packet, 2, 7) < 10)
					output << '0';
				output << BitRange(packet, 2, 7) << ", day " << BitRange(packet, 13, 23);
			}
			break;
		case 26:
			output << "CD changer: ";
			switch (BitRange(packet, 0, 6)) {
			case 105:
				output << "Time, seconds part: " << 10*BitRange(packet, 11, 14) + BitRange(packet, 7, 10);
				break;
			case 97:
				output << "Time, minutes part: " << 10*BitRange(packet, 11, 14) + BitRange(packet, 7, 10);
				break;
			case 89:
				output << "Track count: " << 10*BitRange(packet, 11, 14) + BitRange(packet, 7, 10);
				break;
			case 81: // 209
				if (BitRange(packet, 7, 7))
					output << "Begin disc data";
				break;
			case 73:
				output << "Playback second: " << 10*BitRange(packet, 12, 15) + BitRange(packet, 8, 11);
				break;
			case 65:
				output << "Playback minute: " << 10*BitRange(packet, 12, 15) + BitRange(packet, 8, 11);
				break;
			case 61:
				output << "Playback, start of new minute";
				break;
			case 49:
				output << "Playback track: " << 10*BitRange(packet, 11, 14) + BitRange(packet, 7, 10);
				break;
			case 41:
				output << "Disc " << 10*BitRange(packet, 11, 14) + BitRange(packet, 7, 10);
				break;
			case 25:
				output << "Time, frames (1/75 s) part: " << 10*BitRange(packet, 11, 14) + BitRange(packet, 7, 10);
				break;
			}
			break;
		}
		break;
	case 1:
		switch ((packet >> 2) & 63) {
		case 59:
			output << "ECcomm module: ";
			switch (BitRange(packet, 0, 7)) {
			case 0:
				output << "Extra notifications disabled";
				break;
			case 1:
				output << "Parrot button codes:";
				if (BitRange(packet, 8, 15) == 0)
					output << " released";
				else {
					if (BitRange(packet, 8, 8))
						output << " +red";
					if (BitRange(packet, 9, 9))
						output << " +green";
					if (BitRange(packet, 10, 10))
						output << " +center";
					if (BitRange(packet, 11, 15) == 1)
						output << " +up";
					else if (BitRange(packet, 11, 15) == 31)
						output << " +down";
				}
				break;
			case 2:
				output << "LED mode 0x" << hex << BitRange(packet, 8, 15);
				break;
			case 3:
				output << "ADC value: " << BitRange(packet, 8, 15);
				break;
			case 4:
				output << "Button mode: " << BitRange(packet, 8, 15);
				break;
			case 5:
				break;
			case 6:
				output << "Accessory power disabled";
				break;
			case 7:
				output << "Accessory power enabled";
				break;
			case 8:
				output << "Aux input selector locked";
				break;
			case 9:
				output << "Aux input selector unlocked";
				break;
			case 10:
				output << "E&C bus disabled";
				break;
			case 11:
				output << "E&C bus enabled";
				break;
			case 127:
				output << "Reset to default operation";
				break;
			case 128:
				output << "Extra notifications enabled";
				break;
			}
			break;
		case 57:
			switch (BitRange(packet, 0, 6)) {
			case 102:
				output << "Radio: No personalization selected:";
				break;
			case 38:
				output << "Radio: Loaded personalization " << BitRange(packet, 7, 8);
				break;
			}
			break;
		case 48:
			switch (BitRange(packet, 0, 8)) {
			case 302: // 46-1
				output << "Radio: OnStar call in progress";
				break;
			case 6:
				output << "Radio: Aux audio enabled for OnStar";
				break;
			}
			break;
		case 41:
			switch (BitRange(packet, 0, 7)) {
			case 6:
				output << "Radio: OnStar audio mode active";
				break;
			case 2:
				output << "Radio: OnStar audio mode not active";
				break;
			}
			break;
		case 40:
			if (BitRange(packet, 0, 7) == 63) {
				output << "Operation status: ";
				if (BitRange(packet, 8, 8))
					output << '+';
				else
					output << '-';
				output << "radio, ";
				if (BitRange(packet, 9, 9))
					output << '+';
				else
					output << '-';
				output << "onstar, ";
				if (BitRange(packet, 10, 10))
					output << '+';
				else
					output << '-';
				output << "vehicle";
				}
			break;
		case 26:
			output << "CD changer: ";
			switch (BitRange(packet, 0, 3)) {
			case 1:
				output << "Read status:";
				if (BitRange(packet, 4, 4))
					output << " bit-4";
				if (BitRange(packet, 5, 5))
					output << " bit-5";
				if (BitRange(packet, 6, 6))
					output << " bit-6";
				if (BitRange(packet, 7, 7))
					output << " laser-on";
				if (BitRange(packet, 8, 8))
					output << " playing";
				if (BitRange(packet, 9, 9))
					output << " ready-to-seek";
				if (BitRange(packet, 10, 10))
					output << " done-seeking";
				if (BitRange(packet, 11, 11))
					output << " bit-11";
				if (BitRange(packet, 12, 12))
					output << " bit-12";
				if (BitRange(packet, 13, 13))
					output << " bit-13";
				if (BitRange(packet, 14, 14))
					output << " disc-data-still-OK";
				if (BitRange(packet, 15, 15))
					output << " battery-was-disconnected";
				break;
			case 9:
				output << "Mechanism status:";
				if (BitRange(packet, 4, 4))
					output << " bit-4";
				if (BitRange(packet, 5, 5))
					output << " bit-5";
				if (BitRange(packet, 6, 6))
					output << " bit-6";
				if (BitRange(packet, 7, 7))
					output << " ready";
				if (BitRange(packet, 8, 8))
					output << " forward-seek";
				if (BitRange(packet, 9, 9))
					output << " reverse-seek";
				if (BitRange(packet, 10, 10))
					output << " changer-busy";
				if (BitRange(packet, 11, 11))
					output << " ejector-busy";
				if (BitRange(packet, 12, 12))
					output << " disc-data-still-OK";
				if (BitRange(packet, 13, 13))
					output << " bit-13";
				if (BitRange(packet, 14, 14))
					output << " magazine-present";
				if (BitRange(packet, 15, 15))
					output << " playback-not-possible";
				if (BitRange(packet, 16, 16))
					output << " bit-16";
				if (BitRange(packet, 17, 17))
					output << " door-open";
				break;
			case 13:
				switch (BitRange(packet, 4, 7)) {
				case 1:
					output << "Discs present: "
						   << BitRange(packet, 10, 10) << BitRange(packet, 11, 11) << BitRange(packet, 12, 12) << BitRange(packet, 13, 13) << ' '
						   << BitRange(packet, 14, 14) << BitRange(packet, 15, 15) << BitRange(packet, 16, 16) << BitRange(packet, 17, 17) << ' '
						   << BitRange(packet, 18, 18) << BitRange(packet, 19, 19) << BitRange(packet, 20, 20) << BitRange(packet, 21, 21);
					break;
				case 9:
					if (BitRange(packet, 8, 9) == 2)
						output << "Random mode (221), data: " << BitRange(packet, 10, 24);
					else if (BitRange(packet, 8, 9) == 0)
						output << "Random mode (93), data: " << BitRange(packet, 10, 24);
					break;
				}
				break;
			}
			break;
		case 24:
			output << "Cassette player: ";
			break;
		case 19:
			output << "OnStar: ";
			switch (BitRange(packet, 0, 15)) {
			case 6:    // 6-0
				output << "Present and not active";
				break;
			case 518:  // 6-2 (step 2)
				output << "Mute audio and enable radio controls";
				break;
			case 4102: // 6-16 (step 4)
				output << "Mute audio and disable radio controls";
				break;
			case 4614: // 6-18 (step 3)
				output << "Unmute audio and disable radio controls";
				break;
			case 20:   // 20-0 (step 5)
				output << "Disable SOURCE button on radio during OnStar";
				break;
			case 2068: // 20-8 (step 1)
				output << "Enable SOURCE button on radio during OnStar";
				break;
			}
			break;
		}
		break;
	}
	return output.str();
}




unsigned long BuildPacket(
	unsigned char priority,
	unsigned char address,
	unsigned char byte1,
	unsigned char byte2,
	unsigned char byte3) {
	return
		((unsigned long) priority & 3) |
		(((unsigned long) address & 63) << 2) |
		((unsigned long) byte1 << 8) |
		((unsigned long) byte2 << 16) |
		((unsigned long) byte3 << 24);
}




unsigned char CheckByte(string bstr, unsigned char& value, unsigned char minval, unsigned char maxval) {
    /* Check if a number contained in string "bstr" is within the range [minval, maxval],
	   and if so, set "value" to that number and return TRUE. Else, return FALSE. */
	unsigned char maxlen = (maxval > 9)?((maxval > 99)?3:2):1;
	unsigned char len = bstr.length();
	unsigned char mag = 1;
	unsigned local = 0;		// larger than one byte in case of values >255
	value = 0;
	// Fail if too short or too long.
	if (len <= maxlen && len > 0) {
		for (unsigned char i = 0; i < len; i++) {
			// Check that all characters are digits.
			if (bstr[i] < '0' || bstr[i] > '9')
				return 0;
			// Build up the value
			local += mag*(bstr[len - i - 1] - 48);
			mag *= 10;
		}
		// Fail if outside of range [minval, maxval]
		if (local < minval || local > maxval) {
			value = 0;
			return 0;
		}
		// We made it--return the calculated value.
		value = local;
		return 1;
	}
	return 0;
}




unsigned BitRange(unsigned long packet, unsigned char first, unsigned char last) {
	/* Returns the numerical value stored between bits "first" and "last,"
	   inclusive. Bit 0 is actually the ninth bit in the packet; it's the
	   first bit of the datagram. This matches the BitRange() syntax of
	   the Arduino code. */
	unsigned value = (packet >> 8) >> first;
	unsigned mask = 0;
	for (unsigned char i = 0; i < (last - first + 1); i++)
		mask |= 1 << i;
	return value & mask;
}
