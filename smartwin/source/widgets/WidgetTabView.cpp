#include "../../include/smartwin/widgets/WidgetTabView.h"
#include "../../include/smartwin/WidgetCreator.h"
#include "../../include/smartwin/WindowClass.h"
#include "../../include/smartwin/widgets/WidgetMenu.h"

#include <boost/scoped_ptr.hpp>

namespace SmartWin {

const WidgetTabView::Seed & WidgetTabView::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );
	static boost::scoped_ptr<WindowClass> windowClass;

	if ( d_NeedsInit )
	{
		windowClass.reset(new WindowClass(_T("WidgetTabView"), &PolicyType::wndProc, NULL, ( HBRUSH )( COLOR_GRAYTEXT + 1 )));

		d_DefaultValues.className = windowClass->getClassName();
		d_DefaultValues.style = WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

WidgetTabView::WidgetTabView(Widget* w) : 
	PolicyType(w), 
	tab(0), 
	inTab(false),
	active(-1)
	{ }

void WidgetTabView::create(const Seed & cs) {
	PolicyType::create(cs);

	WidgetTabSheet::Seed tcs;
	tcs.style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_FOCUSNEVER | 
		TCS_MULTILINE | TCS_HOTTRACK | TCS_RAGGEDRIGHT;
	tab = WidgetCreator<WidgetTabSheet>::create(this, tcs);
	tab->setImageList(ImageListPtr(new ImageList(16, 16, ILC_COLOR32 | ILC_MASK)));
	tab->onSelectionChanged(std::tr1::bind(&WidgetTabView::handleTabSelected, this));

	onSized(std::tr1::bind(&WidgetTabView::handleSized, this, _1));
	onRaw(std::tr1::bind(&WidgetTabView::handleContextMenu, this, _1, _2), Message(WM_CONTEXTMENU));
}

void WidgetTabView::addWidget(Widget* w, const IconPtr& icon, const SmartUtil::tstring& title, bool visible) {
	int image = addIcon(icon);
	size_t tabs = tab->size();
	TabInfo* ti = new TabInfo(w);
	tab->addPage(cutTitle(title), tabs, reinterpret_cast<LPARAM>(ti), image);

	viewOrder.push_front(w);

	if(viewOrder.size() == 1 || visible) {
		if(viewOrder.size() > 1) {
			swapWidgets(viewOrder.back(), w);
		} else {
			swapWidgets(0, w);
		}
		setActive(tabs);
	}
	
	layout();
}

void WidgetTabView::remove(Widget* w) {
	if(viewOrder.size() > 1 && viewOrder.back() == w) {
		setActive(*(--(--viewOrder.end())));
	}
	
	Widget* cur = getTabInfo(tab->getSelectedIndex())->w;
	
	viewOrder.remove(w);

	int i = findTab(w);
	if(i != -1) {
		delete getTabInfo(i);
		tab->erase(i);
		layout();
	}
	active = findTab(cur);
}

void WidgetTabView::onTabContextMenu(Widget* w, const std::tr1::function<bool (const Point& pt)>& f) {
	TabInfo* ti = getTabInfo(w);
	if(ti) {
		ti->handleContextMenu = f;
	}
}

void WidgetTabView::setActive(int i) {
	if(i == -1)
		return;

	tab->setSelectedIndex(i);
	handleTabSelected();
}

void WidgetTabView::swapWidgets(Widget* oldW, Widget* newW) {
	sendMessage(WM_SETREDRAW, FALSE);

	if(oldW)
		::ShowWindow(oldW->handle(), SW_HIDE);
	::ShowWindow(newW->handle(), SW_SHOW);
	::SetWindowPos(newW->handle(), tab->handle(), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
	::MoveWindow(newW->handle(), clientSize.pos.x, clientSize.pos.y, clientSize.size.x, clientSize.size.y, FALSE);
	
	sendMessage(WM_SETREDRAW, TRUE);
	::RedrawWindow(handle(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	
	::SetFocus(newW->handle());
}

void WidgetTabView::handleTabSelected() {
	int i = tab->getSelectedIndex();
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
}

void WidgetTabView::mark(Widget* w) {
	int i = findTab(w);
	if(i != -1 && i != tab->getSelectedIndex()) {
		tab->setHighlight(i, true);
	}
}

int WidgetTabView::findTab(Widget* w) {
	for(size_t i = 0; i < tab->size(); ++i) {
		if(getTabInfo(i)->w == w) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

WidgetTabView::TabInfo* WidgetTabView::getTabInfo(Widget* w) {
	return getTabInfo(findTab(w));
}

WidgetTabView::TabInfo* WidgetTabView::getTabInfo(int i) {
	return i == -1 ? 0 : reinterpret_cast<TabInfo*>(tab->getData(i));
}

bool WidgetTabView::handleTextChanging(Widget* w, const SmartUtil::tstring& newText) {
	int i = findTab(w);
	if(i != -1) {
		tab->setHeader(i, cutTitle(newText));
		layout();
	}
	return true;
}

SmartUtil::tstring WidgetTabView::cutTitle(const SmartUtil::tstring& title) {
	if(title.length() > MAX_TITLE_LENGTH) {
		return title.substr(0, MAX_TITLE_LENGTH - 3) + _T("...");
	}
	return title;
}

bool WidgetTabView::handleSized(const WidgetSizedEventResult& sz) {
	tab->setBounds(Rectangle(sz.newSize));
	layout();
	return true;
}

void WidgetTabView::layout() {
	Rectangle tmp = tab->getUsableArea();
	if(!(tmp == clientSize)) {
		Rectangle rctabs(tab->getClientAreaSize());
		// Get rid of ugly border...assume y border is the same as x border
		long border = (rctabs.size.x - tmp.size.x) / 2;
		tmp.pos.x = rctabs.pos.x;
		tmp.size.x = rctabs.size.x;
		tmp.size.y += border;
		int i = tab->getSelectedIndex();
		if(i != -1) {
			::MoveWindow(getTabInfo(i)->w->handle(), tmp.pos.x, tmp.pos.y, tmp.size.x, tmp.size.y, TRUE);
		}
		clientSize = tmp;
	}
}

void WidgetTabView::next(bool reverse) {
	if(viewOrder.size() < 2) {
		return;
	}
	Widget* wnd = getActive();
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

void WidgetTabView::setTop(Widget* wnd) {
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

LRESULT WidgetTabView::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	Point pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	TabInfo* ti = 0;
	if(pt.x == -1 && pt.y == -1) {
		int i = tab->getSelectedIndex();
		
		RECT rc;
		if(i == -1 || !TabCtrl_GetItemRect(tab->handle(), i, &rc)) {
			return FALSE;
		}
		pt.x = rc.left;
		pt.y = rc.top;
		
		ti = getTabInfo(i);
	} else {
		int i = tab->hitTest(pt);
		if(i == -1) {
			return FALSE;
		}
		ti = getTabInfo(i);
	}
	
	if(ti->handleContextMenu && ti->handleContextMenu(pt)) {
		return TRUE;
	}
	
	return TRUE;
}

bool WidgetTabView::filter(const MSG& msg) {
	if(msg.message == WM_KEYUP && msg.wParam == VK_CONTROL) {
		inTab = false;
		TabInfo* ti = getTabInfo(tab->getSelectedIndex());
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
		TabInfo* ti = getTabInfo(getTab()->getSelectedIndex());
		if(ti) {
			handled = ti->w->tryFire(msg, retVal);
		}
	}
	return handled;
}

}
