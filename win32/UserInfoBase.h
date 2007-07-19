/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef USERINFOBASE_H_
#define USERINFOBASE_H_

#include <dcpp/forward.h>
#include <dcpp/ResourceManager.h>
#include "resource.h"

class UserInfoBase {
public:
	UserInfoBase(const UserPtr& u) : user(u) { }

	void getList();
	void browseList();
	void matchQueue();
	void pm();
	void grant();
	void addFav();
	void removeAll();

	UserPtr& getUser() { return user; }
	UserPtr user;
	
	struct ADCOnly {
		ADCOnly() : adcOnly(true) { }
		void operator()(UserInfoBase* ui);

		bool adcOnly;
	};

};

template<class T>
class AspectUserInfo {
public:
	typedef AspectUserInfo<T> ThisType;
	
	void handleMatchQueue() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::matchQueue);
	}
	void handleGetList() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::getList);
	}
	void handleBrowseList() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::browseList);
	}
	void handleAddFavorite() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::addFav);
	}
	void handlePrivateMessage() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::pm);
	}
	void handleGrantSlot() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::grant);
	}
	void handleRemoveAll() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::removeAll);
	}

	template<typename MenuType>
	void appendUserItems(MenuType menu) {
		bool adc = static_cast<T*>(this)->getUserList()->forEachSelectedT(UserInfoBase::ADCOnly()).adcOnly;
		menu->appendItem(IDC_GETLIST, TSTRING(GET_FILE_LIST), std::tr1::bind(&ThisType::handleGetList, this));
		if(adc)
			menu->appendItem(IDC_BROWSELIST, TSTRING(BROWSE_FILE_LIST), std::tr1::bind(&ThisType::handleBrowseList, this));
		menu->appendItem(IDC_MATCH_QUEUE, TSTRING(MATCH_QUEUE), std::tr1::bind(&ThisType::handleMatchQueue, this));
		menu->appendItem(IDC_PRIVATEMESSAGE, TSTRING(SEND_PRIVATE_MESSAGE), std::tr1::bind(&ThisType::handlePrivateMessage, this));
		menu->appendItem(IDC_ADD_TO_FAVORITES, TSTRING(ADD_TO_FAVORITES), std::tr1::bind(&ThisType::handleAddFavorite, this));
		menu->appendItem(IDC_GRANTSLOT, TSTRING(GRANT_EXTRA_SLOT), std::tr1::bind(&ThisType::handleGrantSlot, this));
#ifdef PORT_ME
		menu->appendItem(IDC_CONNECT, TSTRING(CONNECT_FAVUSER_HUB), &T::handle);
#endif
		menu->appendSeparatorItem();
		menu->appendItem(IDC_REMOVE_ALL, TSTRING(REMOVE_FROM_ALL), std::tr1::bind(&ThisType::handleRemoveAll, this));
	}
};


#endif /*USERINFOBASE_H_*/
