/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
        may be used to endorse or promote products derived from this software 
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <dwt/widgets/TabView.h>

#include <dwt/widgets/Container.h>
#include <dwt/widgets/ToolTip.h>
#include <dwt/widgets/TabSheet.h>
#include <dwt/WidgetCreator.h>
#include <dwt/util/StringUtils.h>

namespace SmartWin {

WindowClass TabView::windowClass(_T("TabView"), &TabView::wndProc, NULL, ( HBRUSH )( COLOR_WINDOW + 1 ));

TabView::Seed::Seed(bool toggleActive_) :
	BaseType::Seed(windowClass.getClassName(), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE),
	toggleActive(toggleActive_)
{
}

TabView::TabView(Widget* w) :
	BaseType(w),
	tab(0),
	tip(0),
	toggleActive(false),
	inTab(false),
	active(-1),
	dragging(0)
	{ }

void TabView::create(const Seed & cs) {
	PolicyType::create(cs);
	toggleActive = cs.toggleActive;

	TabSheet::Seed tcs;
	tcs.style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |
		 TCS_HOTTRACK | TCS_MULTILINE | TCS_RAGGEDRIGHT | TCS_TOOLTIPS | TCS_FOCUSNEVER;
	tab = WidgetCreator<TabSheet>::create(this, tcs);
	tab->setImageList(ImageListPtr(new ImageList(16, 16, ILC_COLOR32 | ILC_MASK)));
	tab->onSelectionChanged(std::tr1::bind(&TabView::handleTabSelected, this));
	onSized(std::tr1::bind(&TabView::handleSized, this, _1));
	tab->onLeftMouseDown(std::tr1::bind(&TabView::handleLeftMouseDown, this, _1));
	tab->onLeftMouseUp(std::tr1::bind(&TabView::handleLeftMouseUp, this, _1));
	tab->onContextMenu(std::tr1::bind(&TabView::handleContextMenu, this, _1));
	tab->onMiddleMouseDown(std::tr1::bind(&TabView::handleMiddleMouseDown, this, _1));
	tab->onHelp(std::tr1::bind(&TabView::handleHelp, this, _1, _2));

	tip = WidgetCreator<ToolTip>::attach(this, tab->getToolTips()); // created and managed by the tab control thanks to the TCS_TOOLTIPS style
	if(tip) {
		tip->addRemoveStyle(TTS_NOPREFIX, true);
		tip->onRaw(std::tr1::bind(&TabView::handleToolTip, this, _2), Message(WM_NOTIFY, TTN_GETDISPINFO));
	}
}

void TabView::add(Container* w, const IconPtr& icon) {
	int image = addIcon(icon);
	size_t tabs = tab->size();
	TabInfo* ti = new TabInfo(w);
	tab->addPage(formatTitle(w->getText()), tabs, reinterpret_cast<LPARAM>(ti), image);

	viewOrder.push_front(w);

	if(viewOrder.size() == 1 || w->hasStyle(WS_VISIBLE)) {
		if(viewOrder.size() > 1) {
			swapWidgets(viewOrder.back(), w);
		} else {
			swapWidgets(0, w);
		}
		setActive(tabs);
	}
	
	layout();

	w->onTextChanging(std::tr1::bind(&TabView::handleTextChanging, this, w, _1));
}

Container* TabView::getActive() {
	TabInfo* ti = getTabInfo(tab->getSelected());
	return ti ? ti->w : 0;
}

void TabView::remove(Container* w) {
	if(viewOrder.size() > 1 && viewOrder.back() == w) {
		setActive(*(--(--viewOrder.end())));
	}
	
	Container* cur = getTabInfo(tab->getSelected())->w;
	
	viewOrder.remove(w);

	if(w == dragging)
		dragging = 0;

	int i = findTab(w);
	if(i != -1) {
		delete getTabInfo(i);
		tab->erase(i);
		layout();
	}
	active = findTab(cur);

	// when no tab is opened
	if(titleChangedFunction && (active == -1))
		titleChangedFunction(SmartUtil::tstring());
}

SmartUtil::tstring TabView::getTabText(Container* w) {
	int i = findTab(w);
	if(i != -1)
		return tab->getText(i);
	return SmartUtil::tstring();
}

void TabView::onTabContextMenu(Container* w, const ContextMenuFunction& f) {
	TabInfo* ti = getTabInfo(w);
	if(ti) {
		ti->handleContextMenu = f;
	}
}

void TabView::setActive(int i) {
	if(i == -1)
		return;

	tab->setSelected(i);
	handleTabSelected();
}

void TabView::swapWidgets(Container* oldW, Container* newW) {
	sendMessage(WM_SETREDRAW, FALSE);

	if(oldW) {
		oldW->sendMessage(WM_ACTIVATE, WA_INACTIVE, reinterpret_cast<LPARAM>(newW->handle()));
		oldW->setVisible(false);
	}
	
	newW->setVisible(true);
	newW->setBounds(clientSize, false);
	newW->sendMessage(WM_ACTIVATE, WA_ACTIVE, oldW ? reinterpret_cast<LPARAM>(oldW->handle()) : 0);

	sendMessage(WM_SETREDRAW, TRUE);
	::RedrawWindow(handle(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

void TabView::handleTabSelected() {
	int i = tab->getSelected();
	if(i == active) {
		return;
	}
	
	TabInfo* old = getTabInfo(active);

	TabInfo* ti = getTabInfo(i);
	
	if(ti == old)
		return;
	
	swapWidgets(old ? old->w : 0, ti->w);
	
	if(!inTab)
		setTop(ti->w);
	active = i;
	tab->setHighlight(i, false);

	if(titleChangedFunction)
		titleChangedFunction(ti->w->getText());
}

void TabView::mark(Container* w) {
	int i = findTab(w);
	if(i != -1 && i != tab->getSelected()) {
		tab->setHighlight(i, true);
	}
}

int TabView::findTab(Container* w) {
	for(size_t i = 0; i < tab->size(); ++i) {
		if(getTabInfo(i)->w == w) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

TabView::TabInfo* TabView::getTabInfo(Container* w) {
	return getTabInfo(findTab(w));
}

TabView::TabInfo* TabView::getTabInfo(int i) {
	return i == -1 ? 0 : reinterpret_cast<TabInfo*>(tab->getData(i));
}

bool TabView::handleTextChanging(Container* w, const SmartUtil::tstring& newText) {
	int i = findTab(w);
	if(i != -1) {
		tab->setText(i, formatTitle(newText));
		layout();

		if((i == active) && titleChangedFunction)
			titleChangedFunction(newText);
	}
	return true;
}

SmartUtil::tstring TabView::formatTitle(SmartUtil::tstring title) {
	if(title.length() > MAX_TITLE_LENGTH)
		title = title.substr(0, MAX_TITLE_LENGTH - 3) + _T("...");
	return SmartUtil::escapeMenu(title);
}

void TabView::handleSized(const SizedEvent& sz) {
	tab->setBounds(Rectangle(sz.size));
	layout();
}

void TabView::layout() {
	Rectangle tmp = tab->getUsableArea(true);
	if(!(tmp == clientSize)) {
		int i = tab->getSelected();
		if(i != -1) {
			getTabInfo(i)->w->setBounds(tmp);
		}
		clientSize = tmp;
	}
}

void TabView::next(bool reverse) {
	if(viewOrder.size() < 2) {
		return;
	}
	Container* wnd = getActive();
	if(!wnd) {
		return;
	}
	
	WindowIter i;
	if(inTab) {
		i = std::find(viewOrder.begin(), viewOrder.end(), wnd);
		if(i == viewOrder.end()) {
			return;
		}
		
		if(!reverse) {
			if(i == viewOrder.begin()) {
				i = viewOrder.end();
			}
			--i;
		} else {
			if(++i == viewOrder.end()) {
				i = viewOrder.begin();
			}
			++i;
		}
	} else {
		if(!reverse) {
			i = --(--viewOrder.end());
		} else {
			i = ++viewOrder.begin();
		}
	}
	
	setActive(*i);
	return;
}

void TabView::setTop(Container* wnd) {
	WindowIter i = std::find(viewOrder.begin(), viewOrder.end(), wnd);
	if(i != viewOrder.end() && i != --viewOrder.end()) {
		viewOrder.erase(i);
		viewOrder.push_back(wnd);
	}
}

int TabView::addIcon(const IconPtr& icon) {
	int image = -1;
	if(icon) {
		for(size_t i = 0; i < icons.size(); ++i) {
			if(icon == icons[i]) {
				image = i;
				break;
			}
		}
		if(image == -1) {
			image = icons.size();
			icons.push_back(icon);
			tab->getImageList()->add(*icon);
		}
	}
	return image;
}

LRESULT TabView::handleToolTip(LPARAM lParam) {
	LPNMTTDISPINFO ttdi = reinterpret_cast<LPNMTTDISPINFO>(lParam);
	TabInfo* ti = getTabInfo(ttdi->hdr.idFrom); // here idFrom corresponds to the index of the tab
	if(ti) {
		tipText = ti->w->getText();
		ttdi->lpszText = const_cast<LPTSTR>(tipText.c_str());
	}
	return 0;
}

void TabView::handleLeftMouseDown(const MouseEvent& mouseEventResult) {
	TabInfo* ti = getTabInfo(tab->hitTest(mouseEventResult.pos));
	if(ti) {
		if(mouseEventResult.isShiftPressed)
			ti->w->close();
		else {
			dragging = ti->w;
			::SetCapture(tab->handle());
		}
	}
}

void TabView::handleLeftMouseUp(const MouseEvent& mouseEventResult) {
	::ReleaseCapture();

	if(dragging) {
		int dragPos = findTab(dragging);
		dragging = 0;

		if(dragPos == -1)
			return;

		int dropPos = tab->hitTest(mouseEventResult.pos);

		if(dropPos == -1) {
			// not in the tab control; move the tab to the end
			dropPos = tab->size() - 1;
		}

		if(dropPos == dragPos) {
			// the tab hasn't moved; handle the click
			if(dropPos == active) {
				if(toggleActive)
					next();
			} else
				setActive(dropPos);
			return;
		}

		// save some information about the tab before we erase it
		TabInfo* ti = getTabInfo(dragPos);
		int image = tab->getImage(dragPos);

		tab->erase(dragPos);

		tab->addPage(formatTitle(ti->w->getText()), dropPos, reinterpret_cast<LPARAM>(ti), image);

		active = tab->getSelected();

		layout();
	}
}

bool TabView::handleContextMenu(ScreenCoordinate pt) {
	TabInfo* ti = 0;
	if(pt.x() == -1 && pt.y() == -1) {
		int i = tab->getSelected();
		
		RECT rc;
		if(i == -1 || !TabCtrl_GetItemRect(tab->handle(), i, &rc)) {
			return false;
		}
		pt = ScreenCoordinate(Point(rc.left, rc.top));
		ti = getTabInfo(i);
	} else {
		int i = tab->hitTest(pt);
		if(i == -1) {
			return false;
		}
		ti = getTabInfo(i);
	}
	
	if(ti->handleContextMenu && ti->handleContextMenu(pt)) {
		return true;
	}
	
	return false;
}

void TabView::handleMiddleMouseDown(const MouseEvent& mouseEventResult) {
	TabInfo* ti = getTabInfo(tab->hitTest(mouseEventResult.pos));
	if(ti)
		ti->w->close();
}

void TabView::handleHelp(HWND hWnd, unsigned id) {
	if(helpFunction) {
		// hWnd and id are those of the whole tab control; not those of the specific tab on which the user wants help for
		TabInfo* ti = getTabInfo(tab->hitTest(ScreenCoordinate(Point::fromLParam(::GetMessagePos()))));
		if(ti)
			id = ti->w->getHelpId();

		// even if no tab was found below the cursor, forward the message to the application so that it can display its default help
		helpFunction(hWnd, id);
	}
}

bool TabView::filter(const MSG& msg) {
	if(tip)
		tip->relayEvent(msg);

	if(msg.message == WM_KEYUP && msg.wParam == VK_CONTROL) {
		inTab = false;

		TabInfo* ti = getTabInfo(tab->getSelected());
		if(ti) {
			setTop(ti->w);
		}
	} else if(msg.message == WM_KEYDOWN && msg.wParam == VK_TAB && ::GetKeyState(VK_CONTROL) < 0) {
		inTab = true;
		next(::GetKeyState(VK_SHIFT) < 0);
		return true;
	}
	return false;
}

}
