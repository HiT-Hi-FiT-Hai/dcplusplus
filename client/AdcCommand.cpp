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
 * $Id: AdcCommand.cpp,v 1.1 2004/10/29 15:53:38 arnetheduck Exp $
 */
