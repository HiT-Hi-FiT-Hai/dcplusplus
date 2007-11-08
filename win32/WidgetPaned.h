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

#ifndef DCPLUSPLUS_WIN32_WIDGETPANED_H_
#define DCPLUSPLUS_WIN32_WIDGETPANED_H_

#include <boost/scoped_ptr.hpp>

template< bool horizontal >
class WidgetPaned :
	public SmartWin::MessageMapPolicy< SmartWin::Policies::Normal >,
	public SmartWin::AspectMouse< WidgetPaned< horizontal > >,
	public SmartWin::AspectSizable< WidgetPaned< horizontal > >,
	public SmartWin::AspectVisible< WidgetPaned< horizontal > >,
	public SmartWin::AspectRaw< WidgetPaned< horizontal > >
{
	friend class SmartWin::WidgetCreator< WidgetPaned >;
public:
	/// Class type
	typedef WidgetPaned< horizontal > ThisType;

	typedef SmartWin::MessageMapPolicy< SmartWin::Policies::Normal > PolicyType;
	
	/// Object type
	typedef ThisType * ObjectType;

	class Seed
		: public Widget::Seed
	{
	public:
		explicit Seed();
	};

	void setRelativePos(double pos_) {
		pos = pos_;
		resizeChildren();
	}
	
	SmartWin::Widget* getFirst() {
		return children.first;
	}
	void setFirst(SmartWin::Widget* child) {
		children.first = child;
		resizeChildren();
	}
	
	SmartWin::Widget* getSecond() {
		return children.second;
	}
	void setSecond(SmartWin::Widget* child) {
		children.second = child;
		resizeChildren();
	}

	void create( const Seed & cs = Seed() );

	void setRect(SmartWin::Rectangle r) {
		rect = r;
		resizeChildren();
	}
	
protected:
	// Constructor Taking pointer to parent
	explicit WidgetPaned( SmartWin::Widget * parent );

	/// Protected to avoid direct instantiation, you can inherit and use
	/// WidgetFactory class which is friend
	virtual ~WidgetPaned()
	{}

private:
	std::pair<SmartWin::Widget*, SmartWin::Widget*> children;
	
	double pos;
	
	bool moving;
	
	SmartWin::Rectangle rect;
	
	SmartWin::Rectangle getSplitterRect();
	void resizeChildren();
	
	void handleLButtonDown(const SmartWin::MouseEventResult&) {
		::SetCapture( this->handle() );
		moving = true;
	}
	void handleMouseMove(const SmartWin::MouseEventResult& event) {
		if ( event.ButtonPressed == SmartWin::MouseEventResult::LEFT && moving )
		{
			SmartWin::ClientCoordinate cc(event.pos, getParent());
			int x = horizontal ? cc.y() : cc.x();
			int w = horizontal ? rect.size.y : rect.size.x;
			pos = 1. - (static_cast<double>(w - x) / static_cast<double>(w));
			resizeChildren();
		}
	}
	void handleLButtonUp(const SmartWin::MouseEventResult&) {
		::ReleaseCapture();
		moving = false;
	}
	
	static SmartWin::WindowClass windowClass; 
};

template<bool horizontal>
SmartWin::WindowClass WidgetPaned<horizontal>::windowClass(horizontal ? _T("WidgetPanedH") : _T("WidgetPanedV"), 
	&WidgetPaned<horizontal>::wndProc, NULL, ( HBRUSH )( COLOR_3DFACE + 1 ), 
	SmartWin::IconPtr(), SmartWin::IconPtr(), LoadCursor( 0, horizontal ? IDC_SIZENS : IDC_SIZEWE ));

template< bool horizontal >
WidgetPaned< horizontal >::Seed::Seed() : SmartWin::Widget::Seed(windowClass.getClassName(), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS)
{
}

template< bool horizontal >
WidgetPaned< horizontal >::WidgetPaned( SmartWin::Widget * parent )
	: PolicyType( parent )
	, pos(0.5)
	, moving(false)
{
	children.first = children.second = 0;
}

template< bool horizontal >
void WidgetPaned< horizontal >::create( const Seed & cs )
{
	PolicyType::create(cs);
	
	onLeftMouseDown(std::tr1::bind(&ThisType::handleLButtonDown, this, _1));
	onMouseMove(std::tr1::bind(&ThisType::handleMouseMove, this, _1));
	onLeftMouseUp(std::tr1::bind(&ThisType::handleLButtonUp, this, _1));
}

template< bool horizontal >
SmartWin::Rectangle WidgetPaned< horizontal >::getSplitterRect()
{
	// Sanity check
	if(pos < 0.) {
		pos = 0.0;
	} else if(pos > 1.0) {
		pos = 1.0;
	}
	
	SmartWin::Rectangle rc;
	if(!children.first || !children.second) {
		return rc;
	}

	if(horizontal) {
		rc.size.x = rect.size.x;
		rc.pos.x = rect.pos.x;

		int cwidth = rect.size.y;
		int swidth = ::GetSystemMetrics(SM_CYEDGE) + 2;
		int realpos = static_cast<int>(pos * cwidth);
		rc.pos.y = realpos - swidth / 2;
		rc.size.y = swidth;
	} else {
		rc.size.y = rect.size.y;
		rc.pos.y = rect.pos.y;
	
		int cwidth = rect.size.x;
		int swidth = ::GetSystemMetrics(SM_CXEDGE) + 2;
		int realpos = static_cast<int>(pos * cwidth);
		rc.pos.x = realpos - swidth / 2;
		rc.size.x = swidth;
	}
	return rc;
}

template< bool horizontal >
void WidgetPaned< horizontal >::resizeChildren( )
{
	if(!children.first) {
		if(children.second) {
			::MoveWindow(children.second->handle(), rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, TRUE);
		}
		return;
	}
	if(!children.second) {
		::MoveWindow(children.first->handle(), rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, TRUE);
		return;
	}
	
	SmartWin::Rectangle left = rect, right = rect;
	SmartWin::Rectangle rcSplit = getSplitterRect();
	
	if(horizontal) {
		left.size.y = rcSplit.pos.y - left.pos.y;
		right.pos.y = rcSplit.pos.y + rcSplit.size.y;
		right.size.y = rect.size.y - rcSplit.size.y - left.size.y;		
	} else {
		left.size.x = rcSplit.pos.x - left.pos.x;
		right.pos.x = rcSplit.pos.x + rcSplit.size.x;
		right.size.x = rect.size.x - rcSplit.size.x - left.size.x;
	}

	::MoveWindow(children.first->handle(), left.pos.x, left.pos.y, left.size.x, left.size.y, TRUE);
	::MoveWindow(children.second->handle(), right.pos.x, right.pos.y, right.size.x, right.size.y, TRUE);

	this->setBounds(rcSplit);
}

#endif
