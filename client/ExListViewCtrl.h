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

#if !defined(AFX_EXLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_)
#define AFX_EXLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ExListViewCtrl : public CListViewCtrl
{
	int sortColumn;
	int sortType;
	bool ascending;
	int (*fun)(LPARAM, LPARAM);

public:
	enum {	SORT_STRING,
			SORT_STRING_NOCASE,
			SORT_INT,
			SORT_FUNC,
			SORT_FUNC_ITEM
	};
	void setSort(int aColumn, int aType, bool aAscending = true, int (*aFun)(LPARAM, LPARAM) = NULL) {
		sortColumn = aColumn;
		sortType = aType;
		ascending = aAscending;
		fun = aFun;
		resort();
	}

	void resort() {
		if(sortColumn != -1)
			SortItemsEx(&CompareFunc, (LPARAM)this);
	}

	bool getSortDirection() { return ascending; };
	int getSortColumn() { return sortColumn; };
	int getSortType() { return sortType; };

	int insert(int nItem, StringList& aList, int iImage = 0, LPARAM lParam = NULL);
	int insert(StringList& aList, int iImage = 0, LPARAM lParam = NULL);
	int insert(int nItem, const string& aString, int iImage = 0, LPARAM lParam = NULL) {
		return InsertItem(LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE, nItem, aString.c_str(), 0, 0, iImage, lParam);
	}

	int getItemImage(int aItem) {
		LVITEM lvi;
		lvi.iItem = aItem;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_IMAGE;
		GetItem(&lvi);
		return lvi.iImage;
	}
	
	int find(LPARAM lParam, int aStart = -1) {
		LV_FINDINFO fi;
		fi.flags = LVFI_PARAM;
		fi.lParam = lParam;
		return FindItem(&fi, aStart);
	}

	int find(const string& aText, int aStart = -1, bool aPartial = false) {
		LV_FINDINFO fi;
		if(aPartial) {
			fi.flags = LVFI_PARTIAL;
		} else {
			fi.flags = LVFI_STRING;
		}
		fi.psz = aText.c_str();
		return FindItem(&fi, aStart);
	}
	void deleteItem(const string& aItem, int col = 0) {
		for(int i = 0; i < GetItemCount(); i++) {
			char buf[256];
			GetItemText(i, col, buf, 256);
			if(aItem == buf) {
				DeleteItem(i);
				break;
			}
		}
	}
	int moveItem(int oldPos, int newPos);
	void setSortDirection(bool aAscending) { setSort(sortColumn, sortType, aAscending, fun); };

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
		ExListViewCtrl* p = (ExListViewCtrl*) lParamSort;
		char buf[128];
		char buf2[128];
		string a, b;

		switch(p->sortType) {
		case SORT_STRING:
			p->GetItemText(lParam1, p->sortColumn, buf, 128);
			a = buf;
			p->GetItemText(lParam2, p->sortColumn, buf, 128);
			b = buf;
			return p->ascending ? compare(a, b) : -compare(a, b);
		case SORT_STRING_NOCASE:
			p->GetItemText(lParam1, p->sortColumn, buf, 128);
			p->GetItemText(lParam2, p->sortColumn, buf2, 128);
			return p->ascending ? stricmp(buf,buf2) : -stricmp(buf, buf2);
		case SORT_INT:
			p->GetItemText(lParam1, p->sortColumn, buf, 128);
			p->GetItemText(lParam2, p->sortColumn, buf2, 128);
			return p->ascending ? compare(atoi(buf), atoi(buf2)) : -compare(atoi(buf), atoi(buf2));
		case SORT_FUNC:
			return p->ascending ? p->fun(p->GetItemData(lParam1), p->GetItemData(lParam2)) : -p->fun(p->GetItemData(lParam1), p->GetItemData(lParam2));
		case SORT_FUNC_ITEM: 
			{
				LVITEM a;
				a.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
				a.iItem = lParam1;
				a.iSubItem = 0;
				p->GetItem(&a);

				LVITEM b;
				b.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
				b.iItem = lParam2;
				b.iSubItem = 0;
				p->GetItem(&b);
				return p->ascending ? p->fun((LPARAM)&a, (LPARAM)&b) : -p->fun((LPARAM)&a, (LPARAM)&b);
			}
		default:
			return -1;
		}
	}
	
	template<class T> static int compare(const T& a, const T& b) {
		if(a < b)
			return -1;
		else if(a == b)
			return 0;
		else
			return 1;

		dcassert(0);
	}

	ExListViewCtrl() : sortType(SORT_STRING), ascending(true), sortColumn(-1) { };
	virtual ~ExListViewCtrl() { };

};

#endif // !defined(AFX_EXLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_)

/**
 * @file ExListViewCtrl.h
 * $Id: ExListViewCtrl.h,v 1.16 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: ExListViewCtrl.h,v $
 * Revision 1.16  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.15  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.14  2002/01/16 20:56:26  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.13  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.12  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.11  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.10  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.9  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.8  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.7  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.6  2001/12/02 14:05:36  arnetheduck
 * More sorting work, the hub list is now fully usable...
 *
 * Revision 1.5  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * @endif
 */

