/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_USER_COMMAND_H
#define DCPLUSPLUS_WIN32_USER_COMMAND_H

#include <dcpp/FavoriteManager.h>
#include <dcpp/StringTokenizer.h>
#include "resource.h"

template<class T>
class AspectUserCommand {
public:
	AspectUserCommand() { }
	virtual ~AspectUserCommand() { }

	typedef AspectUserCommand<T> ThisType;

	void prepareMenu(dwt::MenuPtr menu, int ctx, const string& hubUrl) {
		prepareMenu(menu, ctx, StringList(1, hubUrl));
	}

	void prepareMenu(dwt::MenuPtr menu, int ctx, const StringList& hubs) {
		userCommands = FavoriteManager::getInstance()->getUserCommands(ctx, hubs);
		
		if(!userCommands.empty()) {
			menu->appendSeparatorItem();
			dwt::MenuPtr cur = menu;
			for(size_t n = 0; n < userCommands.size(); ++n) {
				
				UserCommand* uc = &userCommands[n];
				
				if(uc->getType() == UserCommand::TYPE_SEPARATOR) {
					// Avoid double separators...
					size_t count = cur->getCount();
					
					if( count > 0 && cur->isSeparator(count-1, true)) {
						cur->appendSeparatorItem();
					}
				} else if(uc->getType() == UserCommand::TYPE_RAW || uc->getType() == UserCommand::TYPE_RAW_ONCE) {
					cur = menu;
					StringTokenizer<tstring> t(Text::toT(uc->getName()), _T('\\'));
					for(TStringIter i = t.getTokens().begin(); i != t.getTokens().end(); ++i) {
						if(i+1 == t.getTokens().end()) {
							cur->appendItem(IDC_USER_COMMAND + n, *i, std::tr1::bind(&T::runUserCommand, static_cast<T*>(this), std::tr1::cref(*uc)));
						} else {
							bool found = false;
							// Let's see if we find an existing item...
							for(int k = 0; k < cur->getCount(); k++) {
								if(cur->isPopup(k, true) && Util::stricmp(cur->getText(k, true), *i) == 0) {
									found = true;
									cur = cur->getChild(k);
								}
							}
							if(!found) {
								cur = cur->appendPopup(*i);
							}
						}
					}
				} else {
					dcasserta(0);
				}
			}
		}
	}
private:
	UserCommand::List userCommands;
};

#endif // !defined(UC_HANDLER_H)
