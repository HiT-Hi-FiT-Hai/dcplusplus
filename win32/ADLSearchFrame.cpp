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
#include "resource.h"
#include <dcpp/DCPlusPlus.h>
#include <dcpp/Client.h>
#include "ADLSearchFrame.h"
#include "ADLSProperties.h"
#include <dcpp/ResourceManager.h>

int ADLSearchFrame::columnIndexes[] = { COLUMN_ACTIVE_SEARCH_STRING, COLUMN_SOURCE_TYPE, COLUMN_DEST_DIR, COLUMN_MIN_FILE_SIZE, COLUMN_MAX_FILE_SIZE };
int ADLSearchFrame::columnSizes[] = { 120, 90, 90, 90, 90 };
static ResourceManager::Strings columnNames[] = { ResourceManager::ACTIVE_SEARCH_STRING,
	ResourceManager::SOURCE_TYPE, ResourceManager::DESTINATION, ResourceManager::MIN_SIZE, ResourceManager::MAX_SIZE,
};

ADLSearchFrame::ADLSearchFrame(SmartWin::Widget* mdiParent) : 
	SmartWin::Widget(mdiParent),
	BaseType(mdiParent),
	add(0),
	remove(0),
	properties(0),
	up(0),
	down(0),
	help(0)
{
	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER;
		cs.exStyle = WS_EX_CLIENTEDGE;
		items = createDataGrid(cs);
	//	items->onClicked(&ADLSearchFrame::handleCheckBox);
		addWidget(items);

		items->setFullRowSelect(true); // should be in the style?

		items->setCheckBoxes(true); // We need a onClicked, too
		
		items->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		items->setColumnOrder(WinUtil::splitTokens(SETTING(ADLSEARCHFRAME_ORDER), columnIndexes));
		items->setColumnWidths(WinUtil::splitTokens(SETTING(ADLSEARCHFRAME_WIDTHS), columnSizes));

		items->setBackgroundColor(WinUtil::bgColor);
		items->setFont(WinUtil::font);
#ifdef PORT_ME
		ctrlList.SetTextBkColor(WinUtil::bgColor);
#endif

	}
	
	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;
		
		cs.caption = TSTRING(NEW);
		add = createButton(cs);
		add->onClicked(std::tr1::bind(&ADLSearchFrame::handleAdd, this));
		addWidget(add);
		add->setFont(WinUtil::font);
		
		cs.caption = TSTRING(REMOVE);
		remove = createButton(cs);
		remove->onClicked(std::tr1::bind(&ADLSearchFrame::handleRemove, this));
		addWidget(remove);
		remove->setFont(WinUtil::font);
		
		cs.caption = TSTRING(PROPERTIES);
		properties = createButton(cs);
		properties->onClicked(std::tr1::bind(&ADLSearchFrame::handleProperties, this));
		addWidget(properties);
		properties->setFont(WinUtil::font);
		
		cs.caption = TSTRING(MOVE_UP);
		up = createButton(cs);
		up->onClicked(std::tr1::bind(&ADLSearchFrame::handleUp, this));
		addWidget(up);
		up->setFont(WinUtil::font);
		
		cs.caption = TSTRING(MOVE_DOWN);
		down = createButton(cs);
		down->onClicked(std::tr1::bind(&ADLSearchFrame::handleDown, this));
		addWidget(down);
		down->setFont(WinUtil::font);

		cs.caption = TSTRING(MENU_HELP);
		help = createButton(cs);
		help->onClicked(std::tr1::bind(&ADLSearchFrame::handleHelp, this));
		addWidget(help);
		help->setFont(WinUtil::font);
	}
	initStatus();
	{
/*
		contextMenu = createMenu();
		
		WidgetMenuPtr file = contextMenu->appendPopup(CSTRING(NEW));

		
		file->appendItem(IDC_ADD, TSTRING(NEW), &ADLSearchFrame::popupNew);
		contextMenu->attach(this);*
	//	addWidget(file);
	//	addWidget(contextMenu);
		//contextMenu->setFont(WinUtil::font);
	//	contextMenu->appendItem(1, TSTRING(NEW), &ADLSearchFrame::popupNew);


		//cs.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
/*
		//cs.caption = TSTRING(NEW);
		add = createButton(cs);
		add->onClicked(&ADLSearchFrame::handleAdd);
		addWidget(add);
		add->setFont(WinUtil::font);
		*/
#ifdef PORT_ME
		
		contextMenu.CreatePopupMenu();
		contextMenu.AppendMenu(MF_STRING, IDC_ADD,    CTSTRING(NEW));
		contextMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
		contextMenu.AppendMenu(MF_STRING, IDC_EDIT,   CTSTRING(PROPERTIES));
#endif
	}

	layout();

	LoadAll();
}

ADLSearchFrame::~ADLSearchFrame() {
	
}



void ADLSearchFrame::layout() {
	SmartWin::Rectangle r(SmartWin::Point(0, 0), getClientAreaSize());
	
	SmartWin::Rectangle rs = layoutStatus();
	
	r.size.y -= rs.size.y;
	
	/// @todo dynamic width
	const int ybutton = add->getTextSize("A").y + 10;
	const int xbutton = 90;
	const int xborder = 10;
	
	SmartWin::Rectangle rb(r.getBottom(ybutton));
	r.size.y -= ybutton;
	items->setBounds(r);

	rb.size.x = xbutton;
	add->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	remove->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	properties->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	up->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	down->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	help->setBounds(rb);
}

bool ADLSearchFrame::preClosing() {
	

	ADLSearchManager::getInstance()->Save();
#ifdef PORT_ME
		WinUtil::saveHeaderOrder(items, SettingsManager::ADLSEARCHFRAME_ORDER,
			SettingsManager::ADLSEARCHFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);
#endif
	return true;
}

void ADLSearchFrame::handleAdd() {
	ADLSearch search;
#ifdef PORT_ME
	ADLSProperties dlg(&search);
	if(dlg.run == IDOK)
	{
		// Add new search to the end or if selected, just before
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

		int i = items->getSelectedRow();
		if(i < 0)
		{
			// Add to end
			collection.push_back(search);
			i = collection.size() - 1;
		}
		else
		{
			// Add before selection
			collection.insert(collection.begin() + i, search);
		}

		// Update list control
		int j = i;
		while(j < (int)collection.size())
		{
			UpdateSearch(j++);
		}
	}
#endif
}

void ADLSearchFrame::handleRemove() {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	int i;
	while((i = items->getSelectedIndex()) >= 0)
	{
		collection.erase(collection.begin() + i);
		items->removeRow(i);
	}
}

void ADLSearchFrame::handleProperties() {
	// Get selection info
	int i = items->getSelectedIndex();
	if(i < 0)
	{
		// Nothing selected
		return;
	}

	// Edit existing
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	ADLSearch search = collection[i];

	// Invoke dialog with selected search
#ifdef PORT_ME
	ADLSProperties dlg(&search);
	if(dlg.run() == IDOK)
	{
		// Update search collection
		collection[i] = search;

		// Update list control
		UpdateSearch(i);
	}
#endif
}

void ADLSearchFrame::handleUp() {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Get selection
	vector<unsigned int> sel = items->getSelectedRows();
	
	if(sel.size() < 1)
	{
		return;
	}

	// Find out where to insert
	int i0 = sel[0];
	if(i0 > 0)
	{
		i0 = i0 - 1;
	}

	// Backup selected searches
	ADLSearchManager::SearchCollection backup;
	int i;
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
		items->setSelectedIndex(i0+i);
	}


}

void ADLSearchFrame::handleDown() {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Get selection
	vector<unsigned int> sel = items->getSelectedRows();
	
	if(sel.size() < 1)
	{
		return;
	}

	// Find out where to insert
	int i0 = sel[sel.size() - 1] + 2;
	if(i0 > (int)collection.size())
	{
		i0 = collection.size();
	}

	// Backup selected searches
	ADLSearchManager::SearchCollection backup;
	int i;
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
		items->setSelectedIndex(i0+i);
	}
}

void ADLSearchFrame::handleHelp() {
#ifdef PORT_ME
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDR_ADLSEARCH);
#endif
}
/*
void ADLSearchFrame::handleCheckBox(WidgetButtonPtr) {
	//int row = items->getSelectedIndex();
	//items->setRowChecked(row, !items->getIsRowChecked(row));


}
*/
void ADLSearchFrame::LoadAll()
{
	// Clear current contents
	items->removeAllRows();

	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Load all searches
	for(unsigned long l = 0; l < collection.size(); l++)
	{
		UpdateSearch(l, FALSE);
	}

	
}

void ADLSearchFrame::UpdateSearch(int index, BOOL doDelete = false)
{
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

	// Check args
	if(index >= (int)collection.size())
	{
		return;
	}
	ADLSearch& search = collection[index];

	// Delete
	if(doDelete)
	{
		items->deleteColumn(index);
	}

	// Generate values
	TStringList line;
	tstring fs;
	line.push_back(Text::toT(search.searchString));
	line.push_back(search.SourceTypeToDisplayString(search.sourceType));
	line.push_back(Text::toT(search.destDir));

	fs = _T("");
	if(search.minFileSize >= 0)
	{
		fs = Text::toT(Util::toString(search.minFileSize));
		fs += _T(" ");
		fs += search.SizeTypeToDisplayString(search.typeFileSize);
	}
	line.push_back(fs);

	fs = _T("");
	if(search.maxFileSize >= 0)
	{
		fs = Text::toT(Util::toString(search.maxFileSize));
		fs += _T(" ");
		fs += search.SizeTypeToDisplayString(search.typeFileSize);
	}
	line.push_back(fs);

	// Insert
	items->insertRow(line);
	items->setRowChecked(index, search.isActive);
}

void ADLSearchFrame::popupNew() {
	ADLSearch search;
#ifdef PORT_ME
	ADLSProperties dlg(&search);
	if(dlg.run == IDOK)
	{
		// Add new search to the end or if selected, just before
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

		int i = items->getSelectedRow();
		if(i < 0)
		{
			// Add to end
			collection.push_back(search);
			i = collection.size() - 1;
		}
		else
		{
			// Add before selection
			collection.insert(collection.begin() + i, search);
		}

		// Update list control
		int j = i;
		while(j < (int)collection.size())
		{
			UpdateSearch(j++);
		}
	}
#endif
}

#ifdef PORT_ME

// Keyboard shortcuts
LRESULT ADLSearchFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey)
	{
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_ADD, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		break;
	case VK_RETURN:
		PostMessage(WM_COMMAND, IDC_EDIT, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT ADLSearchFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(reinterpret_cast<HWND>(wParam) == ctrlList) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlList, pt);
		}

		int status = ctrlList.GetSelectedCount() > 0 ? MFS_ENABLED : MFS_GRAYED;
		contextMenu.EnableMenuItem(IDC_EDIT, status);
		contextMenu.EnableMenuItem(IDC_REMOVE, status);
		contextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		return TRUE;
	}
	bHandled = FALSE;
	return FALSE;
}

// Help
LRESULT ADLSearchFrame::onHelpButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDR_ADLSEARCH);
	return 0;
}

LRESULT ADLSearchFrame::onHelpKey(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDR_ADLSEARCH);
	return 0;
}

// Clicked 'Active' check box
LRESULT ADLSearchFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if((item->uChanged & LVIF_STATE) == 0)
		return 0;
	if((item->uOldState & INDEXTOSTATEIMAGEMASK(0xf)) == 0)
		return 0;
	if((item->uNewState & INDEXTOSTATEIMAGEMASK(0xf)) == 0)
		return 0;

	if(item->iItem >= 0)
	{
		// Set new active status check box
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
		ADLSearch& search = collection[item->iItem];
		search.isActive = (ctrlList.GetCheckState(item->iItem) != 0);
	}
	return 0;
}

// Double-click on list control
LRESULT ADLSearchFrame::onDoubleClickList(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if(item->iItem >= 0) {
		// Treat as onEdit command
		PostMessage(WM_COMMAND, IDC_EDIT, 0);
	} else if(item->iItem == -1) {
		PostMessage(WM_COMMAND, IDC_ADD, 0);
	}

	return 0;
}
#endif
