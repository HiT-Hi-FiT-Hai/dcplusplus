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

#ifndef DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_
#define DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_

template<typename T>
class MDIChildFrame : public SmartWin::WidgetFactory< SmartWin::WidgetMDIChild, T, SmartWin::MessageMapPolicyMDIChildWidget > {
public:
	typedef SmartWin::WidgetFactory< SmartWin::WidgetMDIChild, T, SmartWin::MessageMapPolicyMDIChildWidget> FactoryType;
	MDIChildFrame() {
		// If something fails here, you've not called SmartWin::Widget with a good parent
		// (because of smartwin's shitty inheritance) from the most derived class 
		FactoryType::createMDIChild();
	}
	
private:
	
};

#endif
