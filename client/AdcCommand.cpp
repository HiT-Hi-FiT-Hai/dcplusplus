/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "ClientManager.h"

AdcCommand::AdcCommand(u_int32_t aCmd, char aType /* = TYPE_CLIENT */) : cmdInt(aCmd), from(0), type(aType) { }
AdcCommand::AdcCommand(u_int32_t aCmd, const u_int32_t aTarget) : cmdInt(aCmd), from(0), to(aTarget), type(TYPE_DIRECT) { }
AdcCommand::AdcCommand(Severity sev, Error err, const string& desc, char aType /* = TYPE_CLIENT */) : cmdInt(CMD_STA), from(0), type(aType) {
	addParam(Util::toString(sev) + Util::toString(err));
	addParam(desc);
}
AdcCommand::AdcCommand(const string& aLine, bool nmdc /* = false */) throw(ParseException) : cmdInt(0), type(TYPE_CLIENT) {
	parse(aLine, nmdc);
}

void AdcCommand::parse(const string& aLine, bool nmdc /* = false */) throw(ParseException) {
	string::size_type i = 5;

	if(nmdc) {
		// "$ADCxxx ..."
		if(aLine.length() < 7)
			throw ParseException("Too short");
		type = TYPE_CLIENT;
		cmd[0] = aLine[4];
		cmd[1] = aLine[5];
		cmd[2] = aLine[6];
		i += 3;
	} else {
		// "yxxx ..."
		if(aLine.length() < 4)
			throw ParseException("Too short");
		type = aLine[0];
		cmd[0] = aLine[1];
		cmd[1] = aLine[2];
		cmd[2] = aLine[3];
	}

	if(type == TYPE_INFO) {
		from = HUB_SID;
	}

	string::size_type len = aLine.length();
	const char* buf = aLine.c_str();
	string cur;
	cur.reserve(128);

	bool toSet = false;
	bool featureSet = false;
	bool fromSet = nmdc; // $ADCxxx never have a from CID...

	while(i < len) {
		switch(buf[i]) {
		case '\\':
			++i;
			if(i == len)
				throw ParseException("Escape at eol");
			if(buf[i] == 's')
				cur += ' ';
			else if(buf[i] == 'n')
				cur += '\n';
			else if(buf[i] == '\\')
				cur += '\\';
			else if(buf[i] == ' ' && nmdc)	// $ADCGET escaping, leftover from old specs
				cur += ' ';
			else
				throw ParseException("Unknown escape");
			break;
		case ' ': 
			// New parameter...
			{
				if((type == TYPE_BROADCAST || type == TYPE_DIRECT || type == TYPE_FEATURE) && !fromSet) {
					from = toSID(cur);
					fromSet = true;
				} else if(type == TYPE_DIRECT && !toSet) {
					to = toSID(cur);
					toSet = true;
				} else if(type == TYPE_FEATURE && !featureSet) {
					// Skip...
					featureSet = true;
				} else {
					parameters.push_back(cur);
				}
				cur.clear();
			}
			break;
		default:
			cur += buf[i];
		}
		++i;
	}
	if(!cur.empty()) {
		if((type == TYPE_BROADCAST || type == TYPE_DIRECT || type == TYPE_FEATURE) && !fromSet) {
			from = toSID(cur);
			fromSet = true;
		} else if(type == TYPE_DIRECT && !toSet) {
			to = toSID(cur);
			toSet = true;
		} else if(type == TYPE_FEATURE && !featureSet) {
			// Skip...
			featureSet = true;
		} else {
			parameters.push_back(cur);
		}
	}
}

string AdcCommand::toString(u_int32_t sid /* = 0 */, bool nmdc /* = false */) const {
	string tmp;
	if(nmdc) {
		tmp += "$ADC";
	} else {
		tmp += getType();
	}

	tmp += cmdChar;

	if(type == TYPE_BROADCAST || type == TYPE_DIRECT || type == TYPE_FEATURE) {
		tmp += ' ';
		tmp += fromSID(sid);
	}

	if(type == TYPE_DIRECT) {
		tmp += ' ';
		tmp += fromSID(to);
	}

	if(type == TYPE_FEATURE) {
		tmp += ' ';
		tmp += features;
	}

	for(StringIterC i = getParameters().begin(); i != getParameters().end(); ++i) {
		tmp += ' ';
		tmp += escape(*i, nmdc);
	}
	if(nmdc) {
		tmp += '|';
	} else {
		tmp += '\n';
	}
	return tmp;
}

bool AdcCommand::getParam(const char* name, size_t start, string& ret) const {
	for(string::size_type i = start; i < getParameters().size(); ++i) {
		if(toCode(name) == toCode(getParameters()[i].c_str())) {
			ret = getParameters()[i].substr(2);
			return true;
		}
	}
	return false;
}

bool AdcCommand::hasFlag(const char* name, size_t start) const {
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
 * $Id: AdcCommand.cpp,v 1.16 2006/02/11 21:01:54 arnetheduck Exp $
 */
