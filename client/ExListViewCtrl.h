/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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
	boolean ascending;
public:
	enum {	SORT_STRING,
			SORT_INT
	};
	void setSort(int aColumn, int aType, boolean aAscending = true) {
		sortColumn = aColumn;
		sortType = aType;
		ascending = aAscending;
		resort();
	}

	void resort() {
		if(sortColumn != -1)
			SortItemsEx(&CompareFunc, (LPARAM)this);
	}

	boolean getSortDirection() { return ascending; };
	int getSortColumn() { return sortColumn; };
	int getSortType() { return sortType; };

	int insertItem(StringList& aList) {

		int loc;
		if(sortColumn == -1) {
			loc = GetItemCount();
		} else {
			string b = aList[sortColumn];
			int c = atoi(b.c_str());
			for(loc = 0; loc<GetItemCount(); loc++) {
				int comp;
				string b = aList[sortColumn];
				char buf[1024];
				GetItemText(loc, sortColumn, buf, 1024);
				string a = buf;

				switch(sortType) {
				case SORT_STRING:
					comp = compare(a, b, ascending); break;
				case SORT_INT:
					comp = compare(atoi(a.c_str()), c, ascending); break;
				default:
					dcassert(0);
				}
				if(comp > 0)
					break;
			}
		}
		int i = AddItem(loc, 0, aList[0].c_str());
		int k = 0;
		for(StringIter j = aList.begin(); j != aList.end(); ++j, k++) {
			SetItemText(i, k, j->c_str());
		}
		return loc;
	}
	
	void deleteItem(const string& aItem, int col = 0) {
		for(int i = 0; i < GetItemCount(); i++) {
			char buf[1024];
			GetItemText(i, col, buf, 1024);
			if(aItem == buf) {
				DeleteItem(i);
				break;
			}
		}
	}
	void setSortDirection(boolean aAscending) { setSort(sortColumn, sortType, aAscending); };

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
		ExListViewCtrl* p = (ExListViewCtrl*) lParamSort;
		
		char buf[1024];
		string a, b;
		p->GetItemText(lParam1, p->sortColumn, buf, 1024);
		a = buf;
		p->GetItemText(lParam2, p->sortColumn, buf, 1024);
		b = buf;

		switch(p->sortType) {
		case SORT_STRING:
			return compare(a, b, p->ascending);
		case SORT_INT:
			return compare(atoi(a.c_str()), atoi(b.c_str()), p->ascending);
		default:
			dcassert(0);
		}
		dcassert(0);
	}
	
	template<class T> static int compare(const T& a, const T& b, boolean d) {
		if(d) {
			if(a < b)
				return -1;
			else if(a == b)
				return 0;
			else
				return 1;
		} else {
			if(a < b)
				return 1;
			else if(a == b)
				return 0;
			else
				return -1;
		}
		dcassert(0);
	}

	ExListViewCtrl() : sortType(SORT_STRING), ascending(true), sortColumn(-1) { };
	virtual ~ExListViewCtrl() { };

};

#endif // !defined(AFX_EXLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_)

/**
 * @file ExListViewCtrl.h
 * $Id: ExListViewCtrl.h,v 1.1 2001/11/24 10:39:00 arnetheduck Exp $
 * @if LOG
 * $Log: ExListViewCtrl.h,v $
 * Revision 1.1  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * @endif
 */

