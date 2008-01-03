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

#include "stdafx.h"

#include "MainWindow.h"

#include "ADLSearchFrame.h"
#include "DownloadsFrame.h"
#include "FavHubsFrame.h"
#include "FinishedDLFrame.h"
#include "FinishedULFrame.h"
#include "NotepadFrame.h"
#include "PublicHubsFrame.h"
#include "QueueFrame.h"
#include "SearchFrame.h"
#include "SpyFrame.h"
#include "StatsFrame.h"
#include "SystemFrame.h"
#include "UsersFrame.h"
#include "WaitingUsersFrame.h"

void MainWindow::handleOpenWindow(unsigned id) {
	switch (id) {
	case IDC_PUBLIC_HUBS:
		PublicHubsFrame::openWindow(getMDIParent());
		break;
	case IDC_FAVORITE_HUBS:
		FavHubsFrame::openWindow(getMDIParent());
		break;
	case IDC_FAVUSERS:
		UsersFrame::openWindow(getMDIParent());
		break;
	case IDC_QUEUE:
		QueueFrame::openWindow(getMDIParent());
		break;
	case IDC_FINISHED_DL:
		FinishedDLFrame::openWindow(getMDIParent());
		break;
	case IDC_WAITING_USERS:
		WaitingUsersFrame::openWindow(getMDIParent());
		break;
	case IDC_FINISHED_UL:
		FinishedULFrame::openWindow(getMDIParent());
		break;
	case IDC_SEARCH:
		SearchFrame::openWindow(getMDIParent());
		break;
	case IDC_ADL_SEARCH:
		ADLSearchFrame::openWindow(getMDIParent());
		break;
	case IDC_SEARCH_SPY:
		SpyFrame::openWindow(getMDIParent());
		break;
	case IDC_NOTEPAD:
		NotepadFrame::openWindow(getMDIParent());
		break;
	case IDC_SYSTEM_LOG:
		SystemFrame::openWindow(getMDIParent());
		break;
	case IDC_NET_STATS:
		StatsFrame::openWindow(getMDIParent());
		break;
	case IDC_DOWNLOADS:
		DownloadsFrame::openWindow(getMDIParent());
		break;
	default:
		dcassert(0);
		break;
	}
}
