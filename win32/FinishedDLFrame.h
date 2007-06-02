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

class FinishedDLFrame : public FinishedFrameBase<FinishedDLFrame, ResourceManager::FINISHED_DOWNLOADS, IDC_FINISHED_DL>
{
public:
	FinishedDLFrame(SmartWin::Widget* mdiParent);
	virtual ~FinishedDLFrame() { }

#ifdef PORT_ME
	DECLARE_FRAME_WND_CLASS_EX(_T("FinishedFrame"), IDR_FINISHED_DL, 0, COLOR_3DFACE);
#endif
};

#endif // !defined(DCPLUSPLUS_WIN32_FINISHED_DL_FRAME_H)
