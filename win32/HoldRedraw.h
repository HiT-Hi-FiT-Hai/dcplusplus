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

#ifndef DCPLUSPLUS_WIN32_HOLDREDRAW_H_
#define DCPLUSPLUS_WIN32_HOLDREDRAW_H_

class HoldRedraw {
public:
	HoldRedraw(SmartWin::Widget* w_, bool reallyHold = true) : w(w_) {
		if(reallyHold) {
			::SendMessage(w->handle(), WM_SETREDRAW, FALSE, 0);
		} else {
			w = 0;
		}
	}
	~HoldRedraw() {
		if(w)
			::SendMessage(w->handle(), WM_SETREDRAW, TRUE, 0);
	}
private:
	SmartWin::Widget* w;
};

#endif /*DCPLUSPLUS_WIN32_HOLDREDRAW_H_*/
