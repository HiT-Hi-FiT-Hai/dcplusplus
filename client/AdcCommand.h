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

#if !defined(ADC_COMMAND_H)
#define ADC_COMMAND_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CID.h"
#include "SettingsManager.h"
#include "Exception.h"

STANDARD_EXCEPTION(ParseException);

class AdcCommand {
public:
	template<u_int32_t T>
	struct Type {
		enum { CMD = T };
	};

	enum Error {
		ERROR_GENERIC = 0,
		ERROR_HUB_GENERIC = 10,
		ERROR_HUB_FULL = 11,
		ERROR_HUB_DISABLED = 12,
		ERROR_LOGIN_GENERIC = 20,
		ERROR_NICK_INVALID = 21,
		ERROR_NICK_TAKEN = 22,
		ERROR_BAD_PASSWORD = 23,
		ERROR_CID_TAKEN = 24,
		ERROR_COMMAND_ACCESS = 25,
		ERROR_REGGED_ONLY = 26,
		ERROR_BANNED_GENERIC = 30,
		ERROR_PERM_BANNED = 31,
		ERROR_TEMP_BANNED = 32,
		ERROR_PROTOCOL_GENERIC = 40,
		ERROR_PROTOCOL_UNSUPPORTED = 41,
		ERROR_INF_MISSING = 42,
		ERROR_BAD_STATE = 43,
		ERROR_TRANSFER_GENERIC = 50,
		ERROR_FILE_NOT_AVAILABLE = 51,
		ERROR_FILE_PART_NOT_AVAILABLE = 52,
		ERROR_SLOTS_FULL = 53
	};

	enum Severity {
		SEV_SUCCESS = 0,
		SEV_RECOVERABLE = 1,
		SEV_FATAL = 2
	};

	static const char TYPE_ACTIVE = 'A';
	static const char TYPE_BROADCAST = 'B';
	static const char TYPE_CLIENT = 'C';
	static const char TYPE_DIRECT = 'D';
	static const char TYPE_INFO = 'I';
	static const char TYPE_HUB = 'H';
	static const char TYPE_PASSIVE = 'P';
	static const char TYPE_UDP = 'U';

#define CMD(n, a, b, c) static const u_int32_t CMD_##n = (((u_int32_t)a) | (((u_int32_t)b)<<8) | (((u_int32_t)c)<<16)); typedef Type<CMD_##n> n
	CMD(SUP, 'S','U','P');
	CMD(STA, 'S','T','A');
	CMD(INF, 'I','N','F');
	CMD(MSG, 'M','S','G');
	CMD(SCH, 'S','C','H');
	CMD(RES, 'R','E','S');
	CMD(CTM, 'C','T','M');
	CMD(RCM, 'R','C','M');
	CMD(GPA, 'G','P','A');
	CMD(PAS, 'P','A','S');
	CMD(QUI, 'Q','U','I');
	CMD(DSC, 'D','S','C');
	CMD(GET, 'G','E','T');
	CMD(GFI, 'G','F','I');
	CMD(SND, 'S','N','D');
	CMD(NTD, 'N','T','D');
#undef CMD

	explicit AdcCommand(u_int32_t aCmd, char aType = TYPE_CLIENT) : cmdInt(aCmd), from(SETTING(CLIENT_ID)), type(aType) { }
	explicit AdcCommand(u_int32_t aCmd, const CID& aTarget) : cmdInt(aCmd), from(SETTING(CLIENT_ID)), to(aTarget), type(TYPE_DIRECT) { }
	explicit AdcCommand(Severity sev, Error err, const string& desc, char aType = TYPE_CLIENT) : cmdInt(CMD_STA), from(SETTING(CLIENT_ID)), type(aType) {
		addParam(Util::toString(sev) + Util::toString(err));
		addParam(desc);
	}
	explicit AdcCommand(const string& aLine, bool nmdc = false) throw(ParseException) : cmdInt(0), type(TYPE_CLIENT) {
		parse(aLine, nmdc);
	}

	void parse(const string& aLine, bool nmdc = false) throw(ParseException);

	u_int32_t getCommand() const { return cmdInt; }
	char getType() const { return type; }
	void setType(char t) { type = t; }

	StringList& getParameters() { return parameters; }
	const StringList& getParameters() const { return parameters; }

	string toString(bool nmdc = false, bool old = false) const;

	AdcCommand& addParam(const string& name, const string& value) {
		parameters.push_back(name);
		parameters.back() += value;
		return *this;
	}
	AdcCommand& addParam(const string& str) {
		parameters.push_back(str);
		return *this;
	}
	const string& getParam(size_t n) const {
		return getParameters().size() > n ? getParameters()[n] : Util::emptyString;
	}
	/** Return a named parameter where the name is a two-letter code */
	bool getParam(const char* name, size_t start, string& ret) const;
	bool hasFlag(const char* name, size_t start) const;
	static u_int16_t toCode(const char* x) { return *((u_int16_t*)x); }

	bool operator==(u_int32_t aCmd) { return cmdInt == aCmd; }

	static string escape(const string& str, bool old) {
		string tmp = str;
		string::size_type i = 0;
		while( (i = tmp.find_first_of(" \n\\", i)) != string::npos) {
			if(old) {
				tmp.insert(i, "\\");
			} else {
				switch(tmp[i]) {
				case ' ': tmp.replace(i, 1, "\\s"); break;
				case '\n': tmp.replace(i, 1, "\\n"); break;
				case '\\': tmp.replace(i, 1, "\\\\"); break;
				}
			}
			i+=2;
		}
		return tmp;
	}
	const CID& getTo() const { return to; }
	AdcCommand& setTo(const CID& cid) { to = cid; return *this; }
	const CID& getFrom() const { return from; }

private:
	StringList parameters;
	union {
		char cmdChar[4];
		u_int8_t cmd[4];
		u_int32_t cmdInt;
	};
	CID from;
	CID to;
	char type;

};

template<class T>
class CommandHandler {
public:
	void dispatch(const string& aLine, bool nmdc = false) {
		try {
			AdcCommand c(aLine, nmdc);

#define CMD(n) case AdcCommand::CMD_##n: ((T*)this)->handle(AdcCommand::n(), c); break;
			switch(c.getCommand()) {
				CMD(SUP);
				CMD(STA);
				CMD(INF);
				CMD(MSG);
				CMD(SCH);
				CMD(RES);
				CMD(CTM);
				CMD(RCM);
				CMD(GPA);
				CMD(PAS);
				CMD(QUI);
				CMD(DSC);
				CMD(GET);
				CMD(GFI);
				CMD(SND);
				CMD(NTD);
			default: 
				dcdebug("Unknown ADC command: %.50s\n", aLine.c_str());
				break;
#undef CMD

			}
		} catch(const ParseException&) {
			dcdebug("Invalid ADC command: %.50s\n", aLine.c_str());
			return;
		}
	}
};

#endif // !defined(ADC_COMMAND_H)

/**
 * @file
 * $Id: AdcCommand.h,v 1.22 2005/04/24 08:13:11 arnetheduck Exp $
 */
