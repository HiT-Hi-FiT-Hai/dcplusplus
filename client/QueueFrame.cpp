/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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
#include "DCPlusPlus.h"

#include "QueueFrame.h"
#include "SimpleXML.h"

QueueFrame* QueueFrame::frame = NULL;

LRESULT QueueFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlQueue.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER, WS_EX_CLIENTEDGE, IDC_TRANSFERS);
	
	
	ctrlQueue.InsertColumn(COLUMN_TARGET, "Target file", LVCFMT_LEFT, 200, COLUMN_TARGET);
	ctrlQueue.InsertColumn(COLUMN_STATUS, "Status", LVCFMT_LEFT, 300, COLUMN_STATUS);
	ctrlQueue.InsertColumn(COLUMN_SIZE, "Size", LVCFMT_RIGHT, 75, COLUMN_SIZE);
	ctrlQueue.InsertColumn(COLUMN_PRIORITY, "Priority", LVCFMT_LEFT, 75, COLUMN_PRIORITY);
	ctrlQueue.InsertColumn(COLUMN_USERS, "User(s)", LVCFMT_LEFT, 200, COLUMN_USERS);
	
	ctrlQueue.SetBkColor(Util::bgColor);
	ctrlQueue.SetTextBkColor(Util::bgColor);
	ctrlQueue.SetTextColor(Util::textColor);
	
	bHandled = FALSE;
	return 1;
}

void QueueFrame::onQueueAdded(QueueItem* aQI) {
	StringListInfo* i = new StringListInfo((LPARAM)aQI);

	i->columns[COLUMN_TARGET] = aQI->getTarget();
	i->columns[COLUMN_STATUS] = "Waiting to connect...";
	i->columns[COLUMN_SIZE] = aQI->getSize() == -1 ? "Unknown" : Util::formatBytes(aQI->getSize());
	switch(aQI->getPriority()) {
	case QueueItem::PAUSED: i->columns[COLUMN_PRIORITY] = "Paused"; break;
	case QueueItem::LOW: i->columns[COLUMN_PRIORITY] = "Low"; break;
	case QueueItem::NORMAL: i->columns[COLUMN_PRIORITY] = "Normal"; break;
	case QueueItem::HIGH: i->columns[COLUMN_PRIORITY] = "High"; break;
	default: dcassert(0); break;
	}
	
	PostMessage(WM_SPEAKER, ADD_ITEM, (LPARAM)i);
}

void QueueFrame::onQueueRemoved(QueueItem* aQI) {
	PostMessage(WM_SPEAKER, REMOVE_ITEM, (LPARAM) aQI);
}

void QueueFrame::onQueueStatus(QueueItem* aQI) {
	StringListInfo*i = new StringListInfo((LPARAM) aQI);

	switch(aQI->getStatus()) {
	case QueueItem::FINISHED: i->columns[COLUMN_STATUS] = "Download Finished"; break;
	case QueueItem::RUNNING: i->columns[COLUMN_STATUS] = "Running..."; break;
	case QueueItem::WAITING: break;
	default: dcassert(0); break;
	}

	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void QueueFrame::onQueueUpdated(QueueItem* aQI) {
	if(aQI->getStatus() != QueueItem::FINISHED) {
		StringListInfo* i = new StringListInfo((LPARAM)aQI);
		string tmp;
			
		int online = 0;
		for(QueueItem::Source::Iter j = aQI->getSources().begin(); j != aQI->getSources().end(); ++j) {
			if(tmp.size() > 0)
				tmp += ", ";
			
			QueueItem::Source::Ptr sr = *j;
			if(sr->getUser()) {
				
				if(sr->getUser()->isOnline())
					online++;
				
				tmp += sr->getUser()->getNick() + " (" + sr->getUser()->getClientName() + ")";
			} else {
				tmp += sr->getNick() + " (Offline)";
			}
		}
		i->columns[COLUMN_USERS] = tmp;
		
		char buf[64];
		if(online > 0) {
			
			if(aQI->getSources().size() == 1) {
				i->columns[COLUMN_STATUS] = "Waiting to connect (User online)";
			} else {
				sprintf(buf, "Waiting to connect (%d of %d users online)", online, aQI->getSources().size());
				i->columns[COLUMN_STATUS] = buf;
			}
		} else {
			if(aQI->getSources().size() == 0) {
				i->columns[COLUMN_STATUS] = "No users to download from";
			} else if(aQI->getSources().size() == 1) {
				i->columns[COLUMN_STATUS] = "User offline";
			} else if(aQI->getSources().size() == 2) {
				i->columns[COLUMN_STATUS] = "Both users offline";
			} else {
				sprintf(buf, "All %d users offline", aQI->getSources().size());
				i->columns[COLUMN_STATUS] = buf;
			}
		}
		PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
	}
}

LRESULT QueueFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		
	if(wParam == ADD_ITEM) {
		StringListInfo* i = (StringListInfo*)lParam;
		StringList l;
		for(int j = 0; j < COLUMN_LAST; j++) {
			l.push_back(i->columns[j]);
		}
		dcassert(ctrlQueue.find(i->lParam) == -1);
		ctrlQueue.insert(l, 0, i->lParam);
		delete i;
	} else if(wParam == REMOVE_ITEM) {
		dcassert(ctrlQueue.find(lParam) != -1);
		ctrlQueue.DeleteItem(ctrlQueue.find(lParam));
	} else if(wParam == SET_TEXT) {
		StringListInfo* l = (StringListInfo*)lParam;
		int n = ctrlQueue.find(l->lParam);
		dcassert(n != -1);
		ctrlQueue.SetRedraw(FALSE);
		for(int i = 0; i < COLUMN_LAST; i++) {
			if(!l->columns[i].empty()) {
				ctrlQueue.SetItemText(n, i, l->columns[i].c_str());
			}
		}
		ctrlQueue.SetRedraw(TRUE);
		delete l;
	}

	return 0;
}

/**
 * @file QueueFrame.cpp
 * $Id: QueueFrame.cpp,v 1.1 2002/02/01 02:00:40 arnetheduck Exp $
 * @if LOG
 * $Log: QueueFrame.cpp,v $
 * Revision 1.1  2002/02/01 02:00:40  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.2  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.1  2002/01/23 08:45:37  arnetheduck
 * New files for the notepad
 *
 * Revision 1.9  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.8  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.7  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.6  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.5  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.4  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.3  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.2  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.1  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * @endif
 */


