/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "Client.h"
#include "BufferedSocket.h"
#include "CID.h"
#include "AdcCommand.h"
#include "TigerHash.h"

class AdcHub;
class ClientManager;

class AdcHub : public Client, CommandHandler<AdcHub> {
public:

	virtual void connect(const User* user);
	
	virtual void hubMessage(const string& aMessage);
	virtual void privateMessage(const User* user, const string& aMessage);
	virtual void kick(const User* user, const string& aMessage);
	virtual void ban(const User* user, const string& aMessage, time_t aSeconds);
	virtual void send(const string& aMessage) { socket->write(aMessage); };
	virtual void redirect(const User* user, const string& aHub, const string& aMessage);
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString);
	virtual void password(const string& pwd);
	virtual void info();

	virtual int getUserCount() const { return 0;};
	virtual int64_t getAvailable() const { return 0; };
	virtual const string& getName() const { return (hub ? hub->getNick() : getAddressPort()); };
	virtual bool getOp() const { return false;};

	virtual User::NickMap& lockUserList() { return nickMap; };
	virtual void unlockUserList() { };

	template<typename T> void handle(T, Command&) { 
		//Speaker<AdcHubListener>::fire(t, this, c);
	}

	void handle(Command::SUP, Command& c) throw();
	void handle(Command::MSG, Command& c) throw();
	void handle(Command::INF, Command& c) throw();
	void handle(Command::GPA, Command& c) throw();
	void handle(Command::QUI, Command& c) throw();

private:
	friend class ClientManager;

	AdcHub(const string& aHubURL);

	AdcHub(const AdcHub&);
	AdcHub& operator=(const AdcHub&);

	User::NickMap nickMap;
	User::Ptr hub;
	string lastInfo;

	string salt;

	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw();
	virtual void on(Line, const string& aLine) throw() { 
		fire(ClientListener::Message(), this, "CMD: " + aLine +"\r\n");
		dispatch(aLine); 
	}
	virtual void on(Failed, const string& aLine) throw();
};

/**
 * @file
 * $Id: AdcHub.h,v 1.5 2004/04/24 09:40:58 arnetheduck Exp $
 */
