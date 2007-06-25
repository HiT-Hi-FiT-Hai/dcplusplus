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

#include <client/forward.h>
#include <client/ResourceManager.h>
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

	template<typename MenuType>
	void handleMatchQueue(MenuType, unsigned) {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::matchQueue);
	}
	
	template<typename MenuType>
	void handleGetList(MenuType, unsigned) {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::getList);
	}
	template<typename MenuType>
	void handleBrowseList(MenuType, unsigned) {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::browseList);
	}
	template<typename MenuType>
	void handleAddFavorite(MenuType, unsigned) {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::addFav);
	}
	template<typename MenuType>
	void handlePrivateMessage(MenuType, unsigned) {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::pm);
	}
	template<typename MenuType>
	void handleGrantSlot(MenuType, unsigned) {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::grant);
	}
	template<typename MenuType>
	void handleRemoveAll(MenuType, unsigned) {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::removeAll);
	}

	template<typename MenuType>
	void appendUserItems(MenuType menu) {
		bool adc = static_cast<T*>(this)->getUserList()->forEachSelectedT(UserInfoBase::ADCOnly()).adcOnly;
		menu->appendItem(IDC_GETLIST, TSTRING(GET_FILE_LIST), &T::handleGetList);
		if(adc)
			menu->appendItem(IDC_BROWSELIST, TSTRING(BROWSE_FILE_LIST), &T::handleBrowseList);
		menu->appendItem(IDC_MATCH_QUEUE, TSTRING(MATCH_QUEUE), &T::handleMatchQueue);
		menu->appendItem(IDC_PRIVATEMESSAGE, TSTRING(SEND_PRIVATE_MESSAGE), &T::handlePrivateMessage);
		menu->appendItem(IDC_ADD_TO_FAVORITES, TSTRING(ADD_TO_FAVORITES), &T::handleAddFavorite);
		menu->appendItem(IDC_GRANTSLOT, TSTRING(GRANT_EXTRA_SLOT), &T::handleGrantSlot);
#ifdef PORT_ME
		menu->appendItem(IDC_CONNECT, TSTRING(CONNECT_FAVUSER_HUB), &T::handle);
#endif
		menu->appendSeparatorItem();
		menu->appendItem(IDC_REMOVE_ALL, TSTRING(REMOVE_FROM_ALL), &T::handleRemoveAll);
	}
};


#endif /*USERINFOBASE_H_*/
