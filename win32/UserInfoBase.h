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
	void pm(SmartWin::WidgetMDIParent*);
	void grant();
	void addFav();
	void removeAll();
	void connectFav(SmartWin::WidgetMDIParent*);

	UserPtr& getUser() { return user; }
	UserPtr user;
	
	struct UserTraits {
		UserTraits() : adcOnly(true), favOnly(true), nonFavOnly(true) { }
		void operator()(UserInfoBase* ui);

		bool adcOnly;
		bool favOnly;
		bool nonFavOnly;
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
	// std::tr1::bind(&UserInfoBase::connectFav, _1, parent) doesn't seem to work with g++ svn 2007-07-30...
	// wonder if it's me or the implementation as boost::bind/function swallows it...
	struct Caller {
		Caller(SmartWin::WidgetMDIParent* parent_, void (UserInfoBase::*f_)(SmartWin::WidgetMDIParent*)) : parent(parent_), f(f_) { }
		void operator()(UserInfoBase* uib) { (uib->*f)(parent); }
		SmartWin::WidgetMDIParent* parent;
		void (UserInfoBase::*f)(SmartWin::WidgetMDIParent*);
	};
	
	void handlePrivateMessage(SmartWin::WidgetMDIParent* parent) {
		static_cast<T*>(this)->getUserList()->forEachSelectedT(Caller(parent, &UserInfoBase::pm));
	}
	void handleGrantSlot() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::grant);
	}
	void handleRemoveAll() {
		static_cast<T*>(this)->getUserList()->forEachSelected(&UserInfoBase::removeAll);
	}
	void handleConnectFav(SmartWin::WidgetMDIParent* parent) {
		static_cast<T*>(this)->getUserList()->forEachSelectedT(Caller(parent, &UserInfoBase::connectFav));
	}

	template<typename MenuType>
	void appendUserItems(SmartWin::WidgetMDIParent* parent, MenuType menu) {
		T* This = static_cast<T*>(this);
		UserInfoBase::UserTraits traits = This->getUserList()->forEachSelectedT(UserInfoBase::UserTraits());
		menu->appendItem(IDC_GETLIST, TSTRING(GET_FILE_LIST), std::tr1::bind(&T::handleGetList, This));
		if(traits.adcOnly)
			menu->appendItem(IDC_BROWSELIST, TSTRING(BROWSE_FILE_LIST), std::tr1::bind(&T::handleBrowseList, This));
		menu->appendItem(IDC_MATCH_QUEUE, TSTRING(MATCH_QUEUE), std::tr1::bind(&T::handleMatchQueue, This));
		menu->appendItem(IDC_PRIVATEMESSAGE, TSTRING(SEND_PRIVATE_MESSAGE), std::tr1::bind(&T::handlePrivateMessage, This, parent));
		if(!traits.favOnly)
			menu->appendItem(IDC_ADD_TO_FAVORITES, TSTRING(ADD_TO_FAVORITES), std::tr1::bind(&T::handleAddFavorite, This));
		menu->appendItem(IDC_GRANTSLOT, TSTRING(GRANT_EXTRA_SLOT), std::tr1::bind(&T::handleGrantSlot, This));
		if(!traits.nonFavOnly)
			menu->appendItem(IDC_CONNECT, TSTRING(CONNECT_FAVUSER_HUB), std::tr1::bind(&T::handleConnectFav, This, parent));
		menu->appendSeparatorItem();
		menu->appendItem(IDC_REMOVE_ALL, TSTRING(REMOVE_FROM_ALL), std::tr1::bind(&T::handleRemoveAll, This));
	}
};

#endif /*USERINFOBASE_H_*/
