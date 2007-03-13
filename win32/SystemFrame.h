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

#ifndef DCPLUSPLUS_WIN32_SYSTEM_FRAME_H
#define DCPLUSPLUS_WIN32_SYSTEM_FRAME_H

#include "StaticFrame.h"

#include <client/LogManager.h>
#include <client/ResourceManager.h>

class SystemFrame : public StaticFrame<SystemFrame>,
	private LogManagerListener
{
public:
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::SYSTEM_LOG;
protected:
	friend class StaticFrame<SystemFrame>;
	friend class MDIChildFrame<SystemFrame>;
	
	SystemFrame(SmartWin::Widget* parent);
	virtual ~SystemFrame();

	void layout();
	void spoken(WPARAM wp, LPARAM lp);
	bool preClosing();

private:
	WidgetTextBoxPtr log;
	
	void addLine(time_t t, const tstring& msg);

	// LogManagerListener
	virtual void on(Message, time_t t, const string& message) throw();
};

#endif