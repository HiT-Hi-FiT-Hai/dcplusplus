/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "AdcCommand.h"

void Command::parse(const string& aLine, bool nmdc /* = false */) {
	string::size_type i = 5;

	if(nmdc) {
		// "$ADCxxx ..."
		if(aLine.length() < 7)
			return;
		type = Command::TYPE_CLIENT;
		memcpy(cmd, &aLine[4], 3);
		i += 3;
	} else {
		// "yxxx ..."
		if(aLine.length() < 4)
			return;
		type = aLine[0];
		memcpy(cmd, &aLine[1], 3);
	}

	string::size_type len = aLine.length();
	const char* buf = aLine.c_str();
	string cur;
	cur.reserve(128);

	bool toSet = false;
	bool fromSet = false;

	while(i < len) {
		switch(buf[i]) {
		case '\\': i++; cur += buf[i]; break;
		case ' ': 
			// New parameter...
			{
				if(type == TYPE_DIRECT && !toSet) {
					to = CID(cur);
					toSet = true;
				} else if(!fromSet && type != TYPE_CLIENT) {
					from = CID(cur);
					fromSet = true;
				} else {
					parameters.push_back(cur);
				}
				cur.clear();
			}
			break;
		default:
			cur += buf[i];
		}
		i++;
	}
	if(!cur.empty()) {
		if(!fromSet && type != TYPE_CLIENT) {
			from = CID(cur);
			fromSet = true;
		} else 	if(type == TYPE_DIRECT && !toSet) {
			to = CID(cur);
			toSet = true;
		} else {
			parameters.push_back(cur);
		}
		cur.clear();
	}
}

string Command::toString(bool nmdc /* = false */) const {
	string tmp;
	if(nmdc) {
		tmp += "$ADC";
	} else {
		tmp += getType();
	}
	tmp += cmdChar;
	if(getType() != TYPE_CLIENT) {
		tmp += ' ';
		tmp += from.toBase32();
	}
	if(getType() == TYPE_DIRECT) {
		tmp += ' ';
		tmp += to.toBase32();
	}
	for(StringIterC i = getParameters().begin(); i != getParameters().end(); ++i) {
		tmp += ' ';
		tmp += escape(*i);
	}
	if(nmdc) {
		tmp += '|';
	} else {
		tmp += '$';
	}
	return tmp;
}

bool Command::getParam(const char* name, size_t start, string& ret) const {
	for(string::size_type i = start; i < getParameters().size(); ++i) {
		if(toCode(name) == toCode(getParameters()[i].c_str())) {
			ret = getParameters()[i].substr(2);
			return true;
		}
	}
	return false;
}

bool Command::hasFlag(const char* name, size_t start) const {
	for(string::size_type i = start; i < getParameters().size(); ++i) {
		if(toCode(name) == toCode(getParameters()[i].c_str()) && 
			getParameters()[i][2] == '1' &&
			getParameters()[i].size() == 3)
		{
			return true;
		}
	}
	return false;
}

/**
 * @file
 * $Id: AdcCommand.cpp,v 1.2 2004/11/22 00:13:30 arnetheduck Exp $
 */
