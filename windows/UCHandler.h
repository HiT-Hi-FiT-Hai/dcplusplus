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

#ifndef __UCHANDLER_H
#define __UCHANDLER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/HubManager.h"

template<class T>
class UCHandler {
public:
	UCHandler() : menuPos(0) { };

	typedef UCHandler<T> thisClass;
	BEGIN_MSG_MAP(thisClass)
		COMMAND_RANGE_HANDLER(IDC_USER_COMMAND, IDC_USER_COMMAND + userCommands.size(), onUserCommand)
	END_MSG_MAP()

	LRESULT onUserCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		dcassert(wID >= IDC_USER_COMMAND);
		size_t n = (size_t)wID - IDC_USER_COMMAND;
		dcassert(n < userCommands.size());

		UserCommand& uc = userCommands[n];

		T* t = static_cast<T*>(this);

		t->runUserCommand(uc);
		return 0;
	}

	void prepareMenu(CMenu& menu, int ctx, const string& server, bool op) {
		userCommands = HubManager::getInstance()->getUserCommands(ctx, server, op);
		int n = 0;

		menuPos = menu.GetMenuItemCount();
		if(!userCommands.empty()) {
			menu.AppendMenu(MF_SEPARATOR);
			for(UserCommand::Iter ui = userCommands.begin(); ui != userCommands.end(); ++ui) {
				UserCommand& uc = *ui;
				if(uc.getType() == UserCommand::TYPE_SEPARATOR) {
					// Avoid double separators...
					if( (menu.GetMenuItemCount() >= 1) && 
						!(menu.GetMenuState(menu.GetMenuItemCount()-1, MF_BYPOSITION) & MF_SEPARATOR))
					{
						menu.AppendMenu(MF_SEPARATOR);
					}
				} else if(uc.getType() == UserCommand::TYPE_RAW) {
					menu.AppendMenu(MF_STRING, IDC_USER_COMMAND+n, uc.getName().c_str());
				} else {
					dcasserta(0);
				}
				n++;
			}
		}
	}
	void cleanMenu(CMenu& menu) {
		if(!userCommands.empty()) {
			for(size_t i = 0; i < userCommands.size()+1; ++i) {
				menu.DeleteMenu(menuPos, MF_BYPOSITION);
			}
		}
	}
private:
	UserCommand::List userCommands;
	int menuPos;
};

#endif

/**
* @file
* $Id: UCHandler.h,v 1.2 2003/11/12 21:45:00 arnetheduck Exp $
*/
