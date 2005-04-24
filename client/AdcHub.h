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

#if !defined(ADC_HUB_H)
#define ADC_HUB_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Client.h"
#include "AdcCommand.h"
#include "User.h"

class ClientManager;

class AdcHub : public Client, public CommandHandler<AdcHub> {
public:

	virtual void connect(const OnlineUser& user);
	virtual void connect(const OnlineUser& user, string const& token);
	virtual void disconnect();
	
	virtual void hubMessage(const string& aMessage);
	virtual void privateMessage(const OnlineUser& user, const string& aMessage);
	virtual void send(const string& aMessage) { socket->write(aMessage); };
	virtual void sendUserCmd(const string& aUserCmd) { send(aUserCmd); }
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	virtual void password(const string& pwd);
	virtual void info(bool alwaysSend);

	virtual size_t getUserCount() const { Lock l(cs); return users.size(); }
	virtual int64_t getAvailable() const;

	template<typename T> void handle(T, AdcCommand&) { 
		//Speaker<AdcHubListener>::fire(t, this, c);
	}

	void send(const AdcCommand& cmd) { socket->write(cmd.toString(false)); };
	void sendUDP(const AdcCommand& cmd);

	void handle(AdcCommand::SUP, AdcCommand& c) throw();
	void handle(AdcCommand::MSG, AdcCommand& c) throw();
	void handle(AdcCommand::INF, AdcCommand& c) throw();
	void handle(AdcCommand::GPA, AdcCommand& c) throw();
	void handle(AdcCommand::QUI, AdcCommand& c) throw();
	void handle(AdcCommand::CTM, AdcCommand& c) throw();
	void handle(AdcCommand::RCM, AdcCommand& c) throw();
	void handle(AdcCommand::STA, AdcCommand& c) throw();
	void handle(AdcCommand::SCH, AdcCommand& c) throw();

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

	typedef HASH_MAP_X(CID, OnlineUser*, CID::Hash, equal_to<CID>, less<CID>) CIDMap;
	typedef CIDMap::iterator CIDIter;

	CIDMap users;
	StringMap lastInfoMap;
	mutable CriticalSection cs;

	string salt;

	static const string CLIENT_PROTOCOL;
	 
	virtual string checkNick(const string& nick);
	
	OnlineUser& getUser(const CID& cid);
	OnlineUser* findUser(const CID& cid);
	void putUser(const CID& cid);

	void clearUsers();

	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw();
	virtual void on(Line, const string& aLine) throw();
	virtual void on(Failed, const string& aLine) throw();
};

#endif // !defined(ADC_HUB_H)

/**
 * @file
 * $Id: AdcHub.h,v 1.31 2005/04/24 08:13:36 arnetheduck Exp $
 */
