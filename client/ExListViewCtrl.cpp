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

#include "ExListViewCtrl.h"

int ExListViewCtrl::moveItem(int oldPos, int newPos) {

	char buf[256];
	LVITEM lvi;
	lvi.iItem = oldPos;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
	GetItem(&lvi);
	StringList l;

	for(int j = 0; j < GetHeader().GetItemCount(); j++) {
		GetItemText(oldPos, j, buf, 256);
		l.push_back(buf);
	}

	SetRedraw(FALSE);
	
	lvi.iItem = newPos;
	int i = InsertItem(&lvi);
	j = 0;
	for(StringIter k = l.begin(); k != l.end(); ++k, j++) {
		SetItemText(i, j, k->c_str());
	}
	
	if(i < oldPos)
		DeleteItem(oldPos + 1);
	else
		DeleteItem(oldPos);

	SetRedraw(TRUE);

	RECT rc;
	GetItemRect(i, &rc, LVIR_BOUNDS);
	InvalidateRect(&rc);
	
	return i;
}

int ExListViewCtrl::insert(StringList& aList, int iImage, LPARAM lParam) {

	char buf[128];
	int loc = 0;
	int count = GetItemCount();

	LVITEM a;
	a.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE | LVIF_TEXT;
	a.iItem = -1;
	a.iSubItem = sortColumn;
	a.iImage = iImage;
	a.state = 0;
	a.stateMask = 0;
	a.lParam = lParam;
	a.iIndent = 0;
	a.pszText = const_cast<char*>(sortColumn == -1 ? aList[0].c_str() : aList[sortColumn].c_str());
	a.cchTextMax = sortColumn == -1 ? aList[0].size() : aList[sortColumn].size();
	
	if(sortColumn == -1) {
		loc = count;
	} else if(count == 0) {
		loc = 0;
	} else {

		string& b = aList[sortColumn];
		int c = 0;
		LPARAM data = NULL;			
		int low = 0;
		int high = count-1;
		int comp = 0;
		while(low <= high)
		{
			loc = (low + high)/2;
			
			switch(sortType) {
			case SORT_STRING:
				GetItemText(loc, sortColumn, buf, 128);
				comp = compare(b, string(buf)); break;
			case SORT_STRING_NOCASE:
				GetItemText(loc, sortColumn, buf, 128);
				comp =  stricmp(b.c_str(), buf);
				break;
			case SORT_INT:
				GetItemText(loc, sortColumn, buf, 128);
				c = atoi(b.c_str());
				comp = compare(c, atoi(buf)); break;
			case SORT_FUNC:
				data = GetItemData(loc);
				comp = fun(lParam, data); break;
			case SORT_FUNC_ITEM:
				{
					LVITEM b;
					b.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
					b.iItem = loc;
					b.iSubItem = 0;
					GetItem(&b);
					comp = fun((LPARAM)&a, (LPARAM)&b);
				} 
				break;
			default:
				dcassert(0);
			}
			
			if(!ascending)
				comp = -comp;

			if(comp == -1) {
				high = loc - 1;
			} else if(comp == 1) {
				low = loc + 1;
			} else {
				break;
			} 
		}

		switch(sortType) {
		case SORT_STRING:
			comp = compare(b, string(buf)); break;
		case SORT_STRING_NOCASE:
			comp =  stricmp(b.c_str(), buf);
			break;
		case SORT_INT:
			comp = compare(c, atoi(buf)); break;
		case SORT_FUNC:
			comp = fun(lParam, data); break;
		case SORT_FUNC_ITEM:
			{
				LVITEM b;
				b.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
				b.iItem = loc;
				b.iSubItem = 0;
				GetItem(&b);
				comp = fun((LPARAM)&a, (LPARAM)&b);
			}
			break;
		default:
			dcassert(0);
		}

		if(!ascending)
			comp = -comp;
		
		if(comp == 1)
			loc++;
		
	}
	SetRedraw(FALSE);
	dcassert(loc >= 0 && loc <= GetItemCount());
	a.iItem = loc;
	a.iSubItem = 0;
	int i = InsertItem(&a);
	int k = 0;
	for(StringIter j = aList.begin(); j != aList.end(); ++j, k++) {
		SetItemText(i, k, j->c_str());
	}
	SetRedraw(TRUE);
	RECT rc;
	GetItemRect(i, &rc, LVIR_BOUNDS);
	InvalidateRect(&rc);
	return loc;
}

int ExListViewCtrl::insert(int nItem, StringList& aList, int iImage, LPARAM lParam) {

	SetRedraw(FALSE);
	dcassert(aList.size() > 0);

	int i = insert(nItem, aList[0], iImage, lParam);

	int k = 0;
	for(StringIter j = aList.begin(); j != aList.end(); ++j, k++) {
		SetItemText(i, k, j->c_str());
	}
	SetRedraw(TRUE);
	RECT rc;
	GetItemRect(i, &rc, LVIR_BOUNDS);
	InvalidateRect(&rc);
	return i;
	
}

/**
 * @file ExListViewCtrl.cpp
 * $Id: ExListViewCtrl.cpp,v 1.6 2002/03/04 23:52:31 arnetheduck Exp $
 * @if LOG
 * $Log: ExListViewCtrl.cpp,v $
 * Revision 1.6  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.5  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.4  2002/01/26 12:06:39  arnetheduck
 * Småsaker
 *
 * Revision 1.3  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.2  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.1  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * @endif
 */

