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
#include "TimerManager.h"
#include "User.h"

class ClientManager;

class AdcHub : public Client, public CommandHandler<AdcHub>, private TimerManagerListener {
public:
	using Client::send;

	virtual void connect(const OnlineUser& user);
	void connect(const OnlineUser& user, string const& token, bool secure);
	virtual void disconnect(bool graceless);
	
	virtual void hubMessage(const string& aMessage);
	virtual void privateMessage(const OnlineUser& user, const string& aMessage);
	virtual void sendUserCmd(const string& aUserCmd) { send(aUserCmd); }
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	virtual void password(const string& pwd);
	virtual void info(bool alwaysSend);

	virtual size_t getUserCount() const { Lock l(cs); return users.size(); }
	virtual int64_t getAvailable() const;

	virtual string escape(string const& str) const { return AdcCommand::escape(str, false); }
	virtual void send(const AdcCommand& cmd);

	string getMySID() { return AdcCommand::fromSID(sid); }
private:
	friend class ClientManager;
	friend class CommandHandler<AdcHub>;

	enum States {
		STATE_PROTOCOL,
		STATE_IDENTIFY,
		STATE_VERIFY,
		STATE_NORMAL
	} state;

	AdcHub(const string& aHubURL, bool secure);

	AdcHub(const AdcHub&);
	AdcHub& operator=(const AdcHub&);
	virtual ~AdcHub() throw();

	/** Map session id to OnlineUser */
	typedef HASH_MAP<u_int32_t, OnlineUser*> SIDMap;
	typedef SIDMap::iterator SIDIter;

	SIDMap users;
	StringMap lastInfoMap;
	mutable CriticalSection cs;

	string salt;

	u_int32_t sid;
	bool reconnect;

	static const string CLIENT_PROTOCOL;
	static const string SECURE_CLIENT_PROTOCOL;
	static const string ADCS_FEATURE;
	static const string TCP4_FEATURE;
	static const string UDP4_FEATURE;
	 
	virtual string checkNick(const string& nick);
	
	OnlineUser& getUser(const u_int32_t aSID, const CID& aCID);
	OnlineUser* findUser(const u_int32_t sid);
	void putUser(const u_int32_t sid);

	void clearUsers();

	void handle(AdcCommand::SUP, AdcCommand& c) throw();
	void handle(AdcCommand::SID, AdcCommand& c) throw();
	void handle(AdcCommand::MSG, AdcCommand& c) throw();
	void handle(AdcCommand::INF, AdcCommand& c) throw();
	void handle(AdcCommand::GPA, AdcCommand& c) throw();
	void handle(AdcCommand::QUI, AdcCommand& c) throw();
	void handle(AdcCommand::CTM, AdcCommand& c) throw();
	void handle(AdcCommand::RCM, AdcCommand& c) throw();
	void handle(AdcCommand::STA, AdcCommand& c) throw();
	void handle(AdcCommand::SCH, AdcCommand& c) throw();
	void handle(AdcCommand::CMD, AdcCommand& c) throw();

	template<typename T> void handle(T, AdcCommand&) { 
		//Speaker<AdcHubListener>::fire(t, this, c);
	}

	void sendUDP(const AdcCommand& cmd);

	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw();
	virtual void on(Line, const string& aLine) throw();
	virtual void on(Failed, const string& aLine) throw();
	virtual void on(TimerManagerListener::Second, u_int32_t aTick) throw();
};

#endif // !defined(ADC_HUB_H)

/**
 * @file
 * $Id: AdcHub.h,v 1.42 2006/02/19 20:39:20 arnetheduck Exp $
 */
