/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(FINISHED_UL_FRAME_H)
#define FINISHED_UL_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FinishedFrameBase.h"

class FinishedULFrame : public FinishedFrameBase<FinishedULFrame, ResourceManager::FINISHED_UPLOADS, IDC_FINISHED_UL>
{
public:
	FinishedULFrame() {
		upload = true;
		boldFinished = SettingsManager::BOLD_FINISHED_UPLOADS;
		columnOrder = SettingsManager::FINISHED_UL_ORDER;
		columnWidth = SettingsManager::FINISHED_UL_WIDTHS;
	};

	virtual ~FinishedULFrame() { };

	DECLARE_FRAME_WND_CLASS_EX(_T("FinishedULFrame"), IDR_FINISHED_UL, 0, COLOR_3DFACE);
		
private:

	virtual void on(AddedUl, FinishedItem* entry) throw() {
		PostMessage(WM_SPEAKER, SPEAK_ADD_LINE, (WPARAM)entry);
	}
	virtual void on(RemovedUl, FinishedItem* entry) throw() { 
		totalBytes -= entry->getChunkSize();
		totalTime -= entry->getMilliSeconds();
		PostMessage(WM_SPEAKER, SPEAK_REMOVE);
	}
	virtual void on(RemovedAllUl) throw() { 
		PostMessage(WM_SPEAKER, SPEAK_REMOVE_ALL);
		totalBytes = 0;
		totalTime = 0;
	}
};

#endif // !defined(FINISHED_UL_FRAME_H)

/**
 * @file
 * $Id: FinishedULFrame.h,v 1.21 2006/01/12 22:32:44 arnetheduck Exp $
 */
