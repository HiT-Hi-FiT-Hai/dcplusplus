#include "../../include/smartwin/widgets/WidgetTabView.h"
#include "../../include/smartwin/WidgetCreator.h"
#include "../../include/smartwin/WindowClass.h"

namespace SmartWin {

WindowClass WidgetTabView::windowClass(_T("WidgetTabView"), &WidgetTabView::wndProc, NULL, ( HBRUSH )( COLOR_WINDOW + 1 ));

WidgetTabView::Seed::Seed() :
	Widget::Seed(windowClass.getClassName(), WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE),
	toggleActive(false)
{
}

WidgetTabView::WidgetTabView(Widget* w) :
	PolicyType(w),
	tab(0),
	tip(0),
	toggleActive(false),
	inTab(false),
	active(-1),
	dragging(0)
	{ }

void WidgetTabView::create(const Seed & cs) {
	PolicyType::create(cs);
	toggleActive = cs.toggleActive;

	WidgetTabSheet::Seed tcs;
	tcs.style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |
		 TCS_HOTTRACK | TCS_MULTILINE | TCS_RAGGEDRIGHT | TCS_TOOLTIPS | TCS_FOCUSNEVER;
	tab = WidgetCreator<WidgetTabSheet>::create(this, tcs);
	tab->setImageList(ImageListPtr(new ImageList(16, 16, ILC_COLOR32 | ILC_MASK)));
	tab->onSelectionChanged(std::tr1::bind(&WidgetTabView::handleTabSelected, this));
	onSized(std::tr1::bind(&WidgetTabView::handleSized, this, _1));
	tab->onLeftMouseDown(std::tr1::bind(&WidgetTabView::handleLeftMouseDown, this, _1));
	tab->onLeftMouseUp(std::tr1::bind(&WidgetTabView::handleLeftMouseUp, this, _1));
	tab->onContextMenu(std::tr1::bind(&WidgetTabView::handleContextMenu, this, _1));
	tab->onMiddleMouseDown(std::tr1::bind(&WidgetTabView::handleMiddleMouseDown, this, _1));

	tip = WidgetCreator<WidgetToolTip>::attach(this, tab->getToolTips()); // created and managed by the tab control thanks to the TCS_TOOLTIPS style
	if(tip) {
		tip->addRemoveStyle(TTS_NOPREFIX, true);
		tip->onRaw(std::tr1::bind(&WidgetTabView::handleToolTip, this, _2), Message(WM_NOTIFY, TTN_GETDISPINFO));
	}
}

void WidgetTabView::add(WidgetChildWindow* w, const IconPtr& icon) {
	int image = addIcon(icon);
	size_t tabs = tab->size();
	TabInfo* ti = new TabInfo(w);
	tab->addPage(formatTitle(w->getText()), tabs, reinterpret_cast<LPARAM>(ti), image);

	viewOrder.push_front(w);

	if(viewOrder.size() == 1 || w->getVisible()) {
		if(viewOrder.size() > 1) {
			swapWidgets(viewOrder.back(), w);
		} else {
			swapWidgets(0, w);
		}
		setActive(tabs);
	}
	
	layout();

	w->onTextChanging(std::tr1::bind(&WidgetTabView::handleTextChanging, this, w, _1));
}

WidgetChildWindow* WidgetTabView::getActive() {
	TabInfo* ti = getTabInfo(tab->getSelected());
	return ti ? ti->w : 0;
}

void WidgetTabView::remove(WidgetChildWindow* w) {
	if(viewOrder.size() > 1 && viewOrder.back() == w) {
		setActive(*(--(--viewOrder.end())));
	}
	
	WidgetChildWindow* cur = getTabInfo(tab->getSelected())->w;
	
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

SmartUtil::tstring WidgetTabView::getTabText(WidgetChildWindow* w) {
	int i = findTab(w);
	if(i != -1)
		return tab->getText(i);
	return SmartUtil::tstring();
}

void WidgetTabView::onTabContextMenu(WidgetChildWindow* w, const std::tr1::function<bool (const ScreenCoordinate& pt)>& f) {
	TabInfo* ti = getTabInfo(w);
	if(ti) {
		ti->handleContextMenu = f;
	}
}

void WidgetTabView::setActive(int i) {
	if(i == -1)
		return;

	tab->setSelected(i);
	handleTabSelected();
}

void WidgetTabView::swapWidgets(WidgetChildWindow* oldW, WidgetChildWindow* newW) {
	sendMessage(WM_SETREDRAW, FALSE);

	if(oldW) {
		oldW->sendMessage(WM_ACTIVATE, WA_INACTIVE, reinterpret_cast<LPARAM>(newW->handle()));
		::ShowWindow(oldW->handle(), SW_HIDE);
	}
	
	::ShowWindow(newW->handle(), SW_SHOW);
	::MoveWindow(newW->handle(), clientSize.x(), clientSize.y(), clientSize.width(), clientSize.height(), FALSE);
	
	newW->sendMessage(WM_ACTIVATE, WA_ACTIVE, oldW ? reinterpret_cast<LPARAM>(oldW->handle()) : 0);
	sendMessage(WM_SETREDRAW, TRUE);
	::RedrawWindow(handle(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	
}

void WidgetTabView::handleTabSelected() {
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

void WidgetTabView::mark(WidgetChildWindow* w) {
	int i = findTab(w);
	if(i != -1 && i != tab->getSelected()) {
		tab->setHighlight(i, true);
	}
}

int WidgetTabView::findTab(WidgetChildWindow* w) {
	for(size_t i = 0; i < tab->size(); ++i) {
		if(getTabInfo(i)->w == w) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

WidgetTabView::TabInfo* WidgetTabView::getTabInfo(WidgetChildWindow* w) {
	return getTabInfo(findTab(w));
}

WidgetTabView::TabInfo* WidgetTabView::getTabInfo(int i) {
	return i == -1 ? 0 : reinterpret_cast<TabInfo*>(tab->getData(i));
}

bool WidgetTabView::handleTextChanging(WidgetChildWindow* w, const SmartUtil::tstring& newText) {
	int i = findTab(w);
	if(i != -1) {
		tab->setText(i, formatTitle(newText));
		layout();

		if((i == active) && titleChangedFunction)
			titleChangedFunction(newText);
	}
	return true;
}

SmartUtil::tstring WidgetTabView::formatTitle(SmartUtil::tstring title) {
	if(title.length() > MAX_TITLE_LENGTH)
		title = title.substr(0, MAX_TITLE_LENGTH - 3) + _T("...");
	return SmartUtil::escapeMenu(title);
}

void WidgetTabView::handleSized(const SizedEvent& sz) {
	tab->setBounds(Rectangle(sz.size));
	layout();
}

void WidgetTabView::layout() {
	Rectangle tmp = tab->getUsableArea(true);
	if(!(tmp == clientSize)) {
		int i = tab->getSelected();
		if(i != -1) {
			::MoveWindow(getTabInfo(i)->w->handle(), tmp.x(), tmp.y(), tmp.width(), tmp.height(), TRUE);
		}
		clientSize = tmp;
	}
}

void WidgetTabView::next(bool reverse) {
	if(viewOrder.size() < 2) {
		return;
	}
	WidgetChildWindow* wnd = getActive();
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

void WidgetTabView::setTop(WidgetChildWindow* wnd) {
	WindowIter i = std::find(viewOrder.begin(), viewOrder.end(), wnd);
	if(i != viewOrder.end() && i != --viewOrder.end()) {
		viewOrder.erase(i);
		viewOrder.push_back(wnd);
	}
}

int WidgetTabView::addIcon(const IconPtr& icon) {
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

LRESULT WidgetTabView::handleToolTip(LPARAM lParam) {
	LPNMTTDISPINFO ttdi = reinterpret_cast<LPNMTTDISPINFO>(lParam);
	TabInfo* ti = getTabInfo(ttdi->hdr.idFrom); // here idFrom corresponds to the index of the tab
	if(ti) {
		tipText = ti->w->getText();
		ttdi->lpszText = const_cast<LPTSTR>(tipText.c_str());
	}
	return 0;
}

void WidgetTabView::handleLeftMouseDown(const MouseEventResult& mouseEventResult) {
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

void WidgetTabView::handleLeftMouseUp(const MouseEventResult& mouseEventResult) {
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

bool WidgetTabView::handleContextMenu(ScreenCoordinate pt) {
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

void WidgetTabView::handleMiddleMouseDown(const MouseEventResult& mouseEventResult) {
	TabInfo* ti = getTabInfo(tab->hitTest(mouseEventResult.pos));
	if(ti)
		ti->w->close();
}

bool WidgetTabView::filter(const MSG& msg) {
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

bool WidgetTabView::tryFire(const MSG& msg, LRESULT& retVal) {
	bool handled = PolicyType::tryFire(msg, retVal);
	if(!handled && msg.message == WM_COMMAND && getTab()) {
		TabInfo* ti = getTabInfo(getTab()->getSelected());
		if(ti) {
			handled = ti->w->tryFire(msg, retVal);
		}
	}
	return handled;
}

}
