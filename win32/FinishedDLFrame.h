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

#ifndef DCPLUSPLUS_WIN32_FINISHED_DL_FRAME_H
#define DCPLUSPLUS_WIN32_FINISHED_DL_FRAME_H

#include "FinishedFrameBase.h"
#include "resource.h"

class FinishedDLFrame : public FinishedFrameBase<FinishedDLFrame, false>
{
public:
	FinishedDLFrame(SmartWin::WidgetTabView* mdiParent);
	virtual ~FinishedDLFrame() { }

	typedef FinishedFrameBase<FinishedDLFrame, false> BaseType;
	
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::FINISHED_DOWNLOADS;
	static const unsigned ICON_RESOURCE = IDR_FINISHED_DL;
};

#endif // !defined(DCPLUSPLUS_WIN32_FINISHED_DL_FRAME_H)
