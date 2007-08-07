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

#ifndef DCPLUSPLUS_WIN32_STATIC_FRAME_H_
#define DCPLUSPLUS_WIN32_STATIC_FRAME_H_

#include "MDIChildFrame.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/Text.h>

template<class T>
class StaticFrame : public MDIChildFrame<T> {
public:
	StaticFrame(SmartWin::WidgetMDIParent* mdiClient) : 
		MDIChildFrame<T>(mdiClient, TSTRING_I(T::TITLE_RESOURCE), SmartWin::IconPtr(new SmartWin::Icon(T::ICON_RESOURCE))) 
	{ 
	}
	
	virtual ~StaticFrame() { 
		frame = 0; 
	}

	static void openWindow(SmartWin::WidgetMDIParent* mdiClient) {
		if(frame) {
			HWND active = mdiClient->getActive();
			if(active != frame->handle()) {
				frame->activate();
			} else if(BOOLSETTING(TOGGLE_ACTIVE_WINDOW)) {
				mdiClient->next();
			}
		} else {
			frame = new T(mdiClient);
		}
	}
	
private:
	friend class MDIChildFrame<T>;
	static T* frame;
};

template<class T>
T* StaticFrame<T>::frame = 0;

#endif /*STATICFRAME_H_*/
