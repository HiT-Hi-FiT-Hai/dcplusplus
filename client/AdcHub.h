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

#include "Client.h"
#include "AdcCommand.h"

class ClientManager;

class AdcHub : public Client, public CommandHandler<AdcHub> {
public:

	virtual void connect(const User* user);
	virtual void connect(const User* user, string const& token);
	virtual void disconnect();
	
	virtual void hubMessage(const string& aMessage);
	virtual void privateMessage(const User* user, const string& aMessage);
	virtual void send(const string& aMessage) { socket->write(aMessage); };
	virtual void sendUserCmd(const string& aUserCmd) { send(aUserCmd); }
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString);
	virtual void password(const string& pwd);
	virtual void info(bool alwaysSend);

	virtual size_t getUserCount() const { return 0;};
	virtual int64_t getAvailable() const { return 0; };
	virtual const string& getName() const { return (hub ? hub->getNick() : getAddressPort()); };
	virtual bool getOp() const { return getMe() ? getMe()->isSet(User::OP) : false; };

	virtual User::NickMap& lockUserList() { return nickMap; };
	virtual void unlockUserList() { };

	template<typename T> void handle(T, AdcCommand&) { 
		//Speaker<AdcHubListener>::fire(t, this, c);
	}

	void send(const AdcCommand& cmd) { socket->write(cmd.toString(false)); };

	void handle(AdcCommand::SUP, AdcCommand& c) throw();
	void handle(AdcCommand::MSG, AdcCommand& c) throw();
	void handle(AdcCommand::INF, AdcCommand& c) throw();
	void handle(AdcCommand::GPA, AdcCommand& c) throw();
	void handle(AdcCommand::QUI, AdcCommand& c) throw();
	void handle(AdcCommand::CTM, AdcCommand& c) throw();
	void handle(AdcCommand::RCM, AdcCommand& c) throw();
	void handle(AdcCommand::STA, AdcCommand& c) throw();

	virtual string escape(string const& str) const { return AdcCommand::escape(str, false); };

private:
	friend class ClientManager;

	enum States {
		STATE_PROTOCOL,
		STATE_IDENTIFY,
		STATE_VERIFY,
		STATE_NORMAL
	} state;

	AdcHub(const string& aHubURL);

	AdcHub(const AdcHub&);
	AdcHub& operator=(const AdcHub&);
	virtual ~AdcHub() throw();
	User::NickMap nickMap;
	User::Ptr hub;
	StringMap lastInfoMap;
	CriticalSection cs;

	string salt;

	static const string CLIENT_PROTOCOL;
	 
	virtual string checkNick(const string& nick);
	virtual string getHubURL();
	
	void clearUsers();

	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw();
	virtual void on(Line, const string& aLine) throw();
	virtual void on(Failed, const string& aLine) throw();
};

/**
 * @file
 * $Id: AdcHub.h,v 1.25 2005/03/12 13:36:34 arnetheduck Exp $
 */
