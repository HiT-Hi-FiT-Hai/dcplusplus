/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#if !defined(AFX_SETTINGS_H__2B9FD59C_3F31_40EA_A4FC_C41F30506476__INCLUDED_)
#define AFX_SETTINGS_H__2B9FD59C_3F31_40EA_A4FC_C41F30506476__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Settings  
{
private:
	static string nick;
	static string email;
	static string description;
	static string connection;
	static string server;
	static string port;
	static int slots;
	static int connectionType;
	
public:
	static string getAppPath() {
		
		TCHAR buf[MAX_PATH+1];
		GetModuleFileName(NULL, buf, MAX_PATH);
		
		string b = buf;
		b = b.substr(0, b.rfind('\\') + 1);
		return b;
		
	}
	static char* connectionSpeeds[];
	enum {	SPEED_288K,
			SPEED_336K,
			SPEED_576K,
			SPEED_ISDN,
			SPEED_SATELLITE,
			SPEED_CABLE,
			SPEED_DSL,
			SPEED_T1,
			SPEED_T3,
			SPEED_LAST
	};
	enum {
		CONNECTION_ACTIVE,
		CONNECTION_PASSIVE
	};	
	static const string& getNick() { return nick; }
	static const string& getEmail() { return email; }
	static const string& getDescription() { return description; }
	static const string& getConnection() { return connection; }
	static const string& getServer() { return server; }
	static short getPort() { return atoi(port.c_str()); }
	static const string& getPortString() { return port; };
	static int getConnectionType() { return connectionType; };
	static int getSlots() { return slots; };
	
	static void setNick(const string& aNick) { nick = aNick; };
	static void setEmail(const string& aEmail) { email = aEmail; };
	static void setDescription(const string& aDescription) { description = aDescription; };
	static void setConnection(const string& aConnection) { connection = aConnection; };
	static void setServer(const string& aServer) { server = aServer; };
	static void setPort(const string& aPort) { port = aPort; };
	static void setConnectionType(int aType) { connectionType = aType; };
	static void setSlots(int aSlots) { slots = aSlots; };
	
	static void load() { load(getAppPath() + "DCPlusPlus.xml"); };
	static void save() { save(getAppPath() + "DCPlusPlus.xml"); };

	static void load(const string& aFileName);
	static void save(const string& aFileName);
};

#endif // !defined(AFX_SETTINGS_H__2B9FD59C_3F31_40EA_A4FC_C41F30506476__INCLUDED_)

/**
 * @file Settings.h
 * $Id: Settings.h,v 1.8 2001/12/08 14:25:49 arnetheduck Exp $
 * @if LOG
 * $Log: Settings.h,v $
 * Revision 1.8  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.7  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.6  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.5  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.4  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.3  2001/11/22 20:42:18  arnetheduck
 * Fixed Settings dialog (Speed setting actually works now!)
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

