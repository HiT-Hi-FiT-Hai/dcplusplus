/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

/*
 * Automatic Directory Listing Search
 * Henrik Engström, henrikengstrom@home.se
 */

#include "stdafx.h"
#include "Resource.h"
#include "../client/DCPlusPlus.h"
#include "../client/Client.h"
#include "ADLSearchFrame.h"

ADLSearchFrame* ADLSearchFrame::frame = NULL;

int ADLSearchFrame::columnIndexes[] = { 
	COLUMN_SEARCH_STRING,
	COLUMN_SOURCE_TYPE,
	COLUMN_DEST_DIR,
	COLUMN_IS_ACTIVE,
	COLUMN_MIN_FILE_SIZE,
	COLUMN_MAX_FILE_SIZE
};
int ADLSearchFrame::columnSizes[] = { 
	120, 
	90, 
	90, 
	50, 
	90, 
	90 
};
static ResourceManager::Strings columnNames[] = { 
	ResourceManager::SEARCH_STRING, 
	ResourceManager::SOURCE_TYPE, 
	ResourceManager::DESTINATION, 
	ResourceManager::ACTIVE, 
	ResourceManager::SIZE_MIN, 
	ResourceManager::SIZE_MAX, 
};

// Frame creation
LRESULT ADLSearchFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;

	// Set frame title
	SetWindowText(CSTRING(ADL_SEARCH));
	
	// Create status bar
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	int w[1] = { 0 };
	ctrlStatus.SetParts(1, w);

	// Create list control
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_ADLLIST);

	// Must have full row select, otherwise inline edit will not work
	ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	// Set background color
	ctrlList.SetBkColor(WinUtil::bgColor);
	ctrlList.SetTextBkColor(WinUtil::bgColor);
	ctrlList.SetTextColor(WinUtil::textColor);

	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(ADLSEARCHFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(ADLSEARCHFRAME_WIDTHS), COLUMN_LAST);
	for(int j = 0; j < COLUMN_LAST; j++) 
	{
		int fmt = LVCFMT_LEFT;
		ctrlList.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	ctrlList.SetColumnOrderArray(COLUMN_LAST, columnIndexes);

	// Create context menu
	contextMenu.CreatePopupMenu();
	contextMenu.AppendMenu(MF_STRING, IDC_ADD,       CSTRING(ADD));
	contextMenu.AppendMenu(MF_STRING, IDC_REMOVE,    CSTRING(REMOVE));
	contextMenu.AppendMenu(MF_STRING, IDC_MOVE_UP,   CSTRING(MOVE_UP));
	contextMenu.AppendMenu(MF_STRING, IDC_MOVE_DOWN, CSTRING(MOVE_DOWN));

	// Load all searches
	LoadAll();

	bHandled = FALSE;
	return TRUE;
}

// Close window
LRESULT ADLSearchFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) 
{
	ADLSearchManager::getInstance()->Save();

	string tmp1;
	string tmp2;

	ctrlList.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
	for(int j = COLUMN_FIRST; j != COLUMN_LAST; j++) {
		columnSizes[j] = ctrlList.GetColumnWidth(j);
		tmp1 += Util::toString(columnIndexes[j]) + ",";
		tmp2 += Util::toString(columnSizes[j]) + ",";
	}
	tmp1.erase(tmp1.size()-1, 1);
	tmp2.erase(tmp2.size()-1, 1);
	
	SettingsManager::getInstance()->set(SettingsManager::ADLSEARCHFRAME_ORDER, tmp1);
	SettingsManager::getInstance()->set(SettingsManager::ADLSEARCHFRAME_WIDTHS, tmp2);

	bHandled = FALSE;
	return 0;
}

// Recalculate frame control layout
void ADLSearchFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) 
{
	RECT rect;
	GetClientRect(&rect);

	// Position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	if(ctrlStatus.IsWindow()) 
	{
		CRect sr;
		int w[1];
		ctrlStatus.GetClientRect(sr);
		w[0] = sr.Width() - 16;
		ctrlStatus.SetParts(1, w);
	}

	// Possition list control
	CRect rc = rect;
	rc.top += 2;
	ctrlList.MoveWindow(rc);
}

// Invoke context menu
LRESULT ADLSearchFrame::OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) 
{
	// Get the bounding rectangle of the client area. 
	RECT rc;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	ctrlList.GetClientRect(&rc);
	ctrlList.ScreenToClient(&pt); 
	
	// Hit-test
	if(PtInRect(&rc, pt)) 
	{ 
		ctrlList.ClientToScreen(&pt);
		contextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		return TRUE; 
	}
	
	return FALSE; 
}

// Add new search
LRESULT ADLSearchFrame::OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) 
{
	// Add a new blank search to the end or if selected, just before
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	int i = ctrlList.GetNextItem(-1, LVNI_SELECTED);
	if(i < 0)
	{
		// Add to end
		collection.push_back(ADLSearch());
		i = collection.size() - 1;
	}
	else
	{
		// Add before selection
		collection.insert(collection.begin() + i, ADLSearch());
	}

	// Update list control
	int j = i;
	while(j < (int)collection.size())
	{
		UpdateSearch(j++);
	}

	// Ensure visible
	ctrlList.EnsureVisible(i, FALSE);

	// Start editing new item
	EditSubItem(i, COLUMN_SEARCH_STRING);

	return 0;
}

// Remove searches
LRESULT ADLSearchFrame::OnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) 
{
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Loop over all selected items
	int i;
	while((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) >= 0)
	{
		collection.erase(collection.begin() + i);
		ctrlList.DeleteItem(i);
	}
	return 0;
}

// Move selected entries up one step
LRESULT ADLSearchFrame::OnMoveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) 
{
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Get selection
	vector<int> sel;
	int i = -1;
	while((i = ctrlList.GetNextItem(i, LVNI_SELECTED)) >= 0)
	{
		sel.push_back(i);
	}
	if(sel.size() < 1)
	{
		return 0;
	}

	// Find out where to insert
	int i0 = sel[0];
	if(i0 > 0)
	{
		i0 = i0 - 1;
	}

	// Backup selected searches
	ADLSearchManager::SearchCollection backup;
	for(i = 0; i < (int)sel.size(); ++i)
	{
		backup.push_back(collection[sel[i]]);
	}

	// Erase selected searches
	for(i = sel.size() - 1; i >= 0; --i)
	{
		collection.erase(collection.begin() + sel[i]);
	}

	// Insert (grouped together)
	for(i = 0; i < (int)sel.size(); ++i)
	{
		collection.insert(collection.begin() + i0 + i, backup[i]);
	}

	// Update UI
	LoadAll();

	// Restore selection
	for(i = 0; i < (int)sel.size(); ++i)
	{
		ctrlList.SetItemState(i0 + i, LVNI_SELECTED, LVNI_SELECTED);
	}

	return 0;
}

// Move selected entries down one step
LRESULT ADLSearchFrame::OnMoveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) 
{
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Get selection
	vector<int> sel;
	int i = -1;
	while((i = ctrlList.GetNextItem(i, LVNI_SELECTED)) >= 0)
	{
		sel.push_back(i);
	}
	if(sel.size() < 1)
	{
		return 0;
	}

	// Find out where to insert
	int i0 = sel[sel.size() - 1] + 2;
	if(i0 > (int)collection.size())
	{
		i0 = collection.size();
	}

	// Backup selected searches
	ADLSearchManager::SearchCollection backup;
	for(i = 0; i < (int)sel.size(); ++i)
	{
		backup.push_back(collection[sel[i]]);
	}

	// Erase selected searches
	for(i = sel.size() - 1; i >= 0; --i)
	{
		collection.erase(collection.begin() + sel[i]);
		if(i < i0)
		{
			i0--;
		}
	}

	// Insert (grouped together)
	for(i = 0; i < (int)sel.size(); ++i)
	{
		collection.insert(collection.begin() + i0 + i, backup[i]);
	}

	// Update UI
	LoadAll();

	// Restore selection
	for(i = 0; i < (int)sel.size(); ++i)
	{
		ctrlList.SetItemState(i0 + i, LVNI_SELECTED, LVNI_SELECTED);
	}

	return 0;
}

// Double-click on list control
LRESULT ADLSearchFrame::OnDoubleClickList(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) 
{
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	// Check if no items
	if(ctrlList.GetItemCount() <= 0)
	{
		// Treat as 'add'-command
		return OnAdd(0, 0, 0, bHandled);
	}

	// Hit-test
	LVHITTESTINFO info;
	info.pt = item->ptAction;
    int iItem = ctrlList.SubItemHitTest(&info);

	if(iItem < 0)
	{
		// No direct hit. If clicked last row, treat as 'add'-command
		RECT label;
		memset(&label, 0, sizeof(label));
		ctrlList.GetSubItemRect(0, 0, LVIR_LABEL, &label);
		int cy = label.bottom - label.top;
		label.top    += ctrlList.GetItemCount() * cy;
		label.bottom += ctrlList.GetItemCount() * cy;
		if(PtInRect(&label, item->ptAction))
		{
			return OnAdd(0, 0, 0, bHandled);
		}
	}
	else
	{
		// Get search info
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
		if(iItem >= (int)collection.size())
		{
			return 0;
		}
		ADLSearch& search = collection[iItem];

		// Dedicated handling
		BOOL bUpdate = TRUE;
		if(info.iSubItem == COLUMN_SEARCH_STRING)
		{
			EditSubItem(info.iItem, info.iSubItem);
			return FALSE;
		}
		else
		if(info.iSubItem == COLUMN_SOURCE_TYPE)
		{
			search.sourceType = (ADLSearch::SourceType)((long)search.sourceType + 1);
			if(search.sourceType >= ADLSearch::TypeLast)
			{
				search.sourceType = ADLSearch::TypeFirst;
			}
		}
		else
		if(info.iSubItem == COLUMN_DEST_DIR)
		{
			EditSubItem(info.iItem, info.iSubItem);
			return FALSE;
		}
		else
		if(info.iSubItem == COLUMN_IS_ACTIVE)
		{
			search.isActive = !search.isActive;
		}
		else
		if(info.iSubItem == COLUMN_MIN_FILE_SIZE)
		{
			EditSubItem(info.iItem, info.iSubItem);
			return FALSE;
		}
		else
		if(info.iSubItem == COLUMN_MAX_FILE_SIZE)
		{
			EditSubItem(info.iItem, info.iSubItem);
			return FALSE;
		}

		// Optionally update search
		if(bUpdate)
		{
			UpdateSearch(info.iItem);
		}
	}

	return 0;
}

// End of inline editing of search string
LRESULT ADLSearchFrame::OnEndLabelEditList(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) 
{
	NMLVDISPINFO* info = (NMLVDISPINFO*)pnmh;
	
	if(info->item.iItem >= 0 && info->item.pszText != NULL)
	{
		// Get current search
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
		if(info->item.iItem >= (int)collection.size())
		{
			return 0;
		}
		ADLSearch& search = collection[info->item.iItem];

		// Update search after inline editing
		if(info->item.iSubItem == COLUMN_SEARCH_STRING)
		{
			search.searchString = info->item.pszText;
			search.Prepare();
		}
		else
		if(info->item.iSubItem == COLUMN_DEST_DIR)
		{
			search.destDir = info->item.pszText;
		}
		else
		if(info->item.iSubItem == COLUMN_MIN_FILE_SIZE)
		{
			if(strlen(info->item.pszText) > 0)
			{
				search.minFileSize = Util::toInt64(info->item.pszText);
			}
			else
			{
				search.minFileSize = (int64_t)-1;
			}
		}
		else
		if(info->item.iSubItem == COLUMN_MAX_FILE_SIZE)
		{
			if(strlen(info->item.pszText) > 0)
			{
				search.maxFileSize = Util::toInt64(info->item.pszText);
			}
			else
			{
				search.maxFileSize = (int64_t)-1;
			}
		}

		// Update UI
		ctrlList.SetItemText(info->item.iItem, info->item.iSubItem, info->item.pszText);

		bHandled = TRUE;
		return 1;
	}

	return 0;
}

// Load all searches from manager
void ADLSearchFrame::LoadAll()
{
	// Clear current contents
	ctrlList.DeleteAllItems();

	// Load all searches
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	for(unsigned long l = 0; l < collection.size(); l++)
	{
		UpdateSearch(l, FALSE);
	}
}

// Update a specific search item
void ADLSearchFrame::UpdateSearch(int index, BOOL doDelete)
{
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Check args
	if(index >= (int)collection.size())
	{
		return;
	}
	ADLSearch& search = collection[index];

	// Delete from list control
	if(doDelete)
	{
		ctrlList.DeleteItem(index);
	}

	// Generate values
	char buf[32];
	StringList line;
	line.push_back(search.searchString);
	line.push_back(search.SourceTypeToString(search.sourceType));
	line.push_back(search.destDir);
	line.push_back(search.isActive ? CSTRING(YES) : CSTRING(NO));
	line.push_back(search.minFileSize >= 0 ? _i64toa(search.minFileSize, buf, 10) : "");
	line.push_back(search.maxFileSize >= 0 ? _i64toa(search.maxFileSize, buf, 10) : "");

	// Insert in list control
	ctrlList.insert(index, line);
}

// Begin editing a subitem
void ADLSearchFrame::EditSubItem(int iItem, int iSubItem)
{
	// Save input for return
	inlineEdit.iItem = iItem;
	inlineEdit.iSubItem = iSubItem;

	// Deselect all
	ctrlList.SetItemState(-1, 0, LVIS_SELECTED);

	// Get rect for new edit control
	RECT label;
	memset(&label, 0, sizeof(label));
	ctrlList.GetSubItemRect(iItem, iSubItem, LVIR_LABEL, &label);

	// Create a new edit control for inline editing
	HWND hInlineEdit = inlineEdit.Create(ctrlList.m_hWnd, label, "InlineEdit", 
		WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL);
	if(hInlineEdit != NULL)
	{
		// Set font
		inlineEdit.SetFont(ctrlList.GetFont(), FALSE);

		// Transfer text
		char text[256];
		ctrlList.GetItemText(iItem, iSubItem, text, 255);
		inlineEdit.SetWindowText(text);
		inlineEdit.SetSel(0, -1);

		// Show window
		inlineEdit.ShowWindow(SW_SHOW);
		inlineEdit.UpdateWindow();
		inlineEdit.SetFocus();
	}
}
