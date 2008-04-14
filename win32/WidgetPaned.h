/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

template< bool horizontal >
class WidgetPaned :
	public dwt::MessageMap< dwt::Policies::Normal >,
	public dwt::AspectMouse< WidgetPaned< horizontal > >,
	public dwt::AspectSizable< WidgetPaned< horizontal > >,
	public dwt::AspectVisible< WidgetPaned< horizontal > >,
	public dwt::AspectRaw< WidgetPaned< horizontal > >
{
	typedef dwt::MessageMap< dwt::Policies::Normal > BaseType;
	friend class dwt::WidgetCreator< WidgetPaned >;
public:
	/// Class type
	typedef WidgetPaned< horizontal > ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		double pos;

		explicit Seed(double pos_ = 0.5);
	};

	void setRelativePos(double pos_) {
		pos = pos_;
		resizeChildren();
	}
	
	dwt::Widget* getFirst() {
		return children.first;
	}
	void setFirst(dwt::Widget* child) {
		children.first = child;
		resizeChildren();
	}
	
	dwt::Widget* getSecond() {
		return children.second;
	}
	void setSecond(dwt::Widget* child) {
		children.second = child;
		resizeChildren();
	}

	void create( const Seed & cs = Seed() );

	void setRect(dwt::Rectangle r) {
		rect = r;
		resizeChildren();
	}
	
protected:
	// Constructor Taking pointer to parent
	explicit WidgetPaned( dwt::Widget * parent );

	/// Protected to avoid direct instantiation, you can inherit and use
	/// WidgetFactory class which is friend
	virtual ~WidgetPaned()
	{}

private:
	std::pair<dwt::Widget*, dwt::Widget*> children;
	
	double pos;
	
	bool moving;
	
	dwt::Rectangle rect;
	
	dwt::Rectangle getSplitterRect();
	void resizeChildren();
	
	void handleLButtonDown(const dwt::MouseEvent&) {
		::SetCapture( this->handle() );
		moving = true;
	}
	void handleMouseMove(const dwt::MouseEvent& event) {
		if ( event.ButtonPressed == dwt::MouseEvent::LEFT && moving )
		{
			dwt::ClientCoordinate cc(event.pos, getParent());
			int x = horizontal ? cc.y() : cc.x();
			int w = horizontal ? rect.size.y : rect.width();
			pos = 1. - (static_cast<double>(w - x) / static_cast<double>(w));
			resizeChildren();
		}
	}
	void handleLButtonUp(const dwt::MouseEvent&) {
		::ReleaseCapture();
		moving = false;
	}
	
	static dwt::WindowClass windowClass; 
};

template<bool horizontal>
dwt::WindowClass WidgetPaned<horizontal>::windowClass(horizontal ? _T("WidgetPanedH") : _T("WidgetPanedV"), 
	&WidgetPaned<horizontal>::wndProc, NULL, ( HBRUSH )( COLOR_3DFACE + 1 ), 
	dwt::IconPtr(), dwt::IconPtr(), LoadCursor( 0, horizontal ? IDC_SIZENS : IDC_SIZEWE ));

template< bool horizontal >
WidgetPaned< horizontal >::Seed::Seed(double pos_) :
	BaseType::Seed(windowClass.getClassName(), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS),
	pos(pos_)
{
}

template< bool horizontal >
WidgetPaned< horizontal >::WidgetPaned( dwt::Widget * parent )
	: BaseType( parent )
	, pos(0.5)
	, moving(false)
{
	children.first = children.second = 0;
}

template< bool horizontal >
void WidgetPaned< horizontal >::create( const Seed & cs )
{
	pos = cs.pos;
	BaseType::create(cs);
	
	onLeftMouseDown(std::tr1::bind(&ThisType::handleLButtonDown, this, _1));
	onMouseMove(std::tr1::bind(&ThisType::handleMouseMove, this, _1));
	onLeftMouseUp(std::tr1::bind(&ThisType::handleLButtonUp, this, _1));
}

template< bool horizontal >
dwt::Rectangle WidgetPaned< horizontal >::getSplitterRect()
{
	// Sanity check
	if(pos < 0.) {
		pos = 0.0;
	} else if(pos > 1.0) {
		pos = 1.0;
	}
	
	dwt::Rectangle rc;
	if(!children.first || !children.second) {
		return rc;
	}

	if(horizontal) {
		rc.size.x = rect.width();
		rc.pos.x = rect.x();

		int cwidth = rect.height();
		int swidth = ::GetSystemMetrics(SM_CYEDGE) + 2;
		int realpos = static_cast<int>(pos * cwidth);
		rc.pos.y = realpos - swidth / 2;
		rc.size.y = swidth;
	} else {
		rc.size.y = rect.size.y;
		rc.pos.y = rect.pos.y;
	
		int cwidth = rect.width();
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
			::MoveWindow(children.second->handle(), rect.x(), rect.y(), rect.width(), rect.height(), TRUE);
		}
		return;
	}
	if(!children.second) {
		::MoveWindow(children.first->handle(), rect.x(), rect.y(), rect.width(), rect.height(), TRUE);
		return;
	}
	
	dwt::Rectangle left = rect, right = rect;
	dwt::Rectangle rcSplit = getSplitterRect();
	
	if(horizontal) {
		left.size.y = rcSplit.y() - left.y();
		right.pos.y = rcSplit.y() + rcSplit.height();
		right.size.y = rect.height() - rcSplit.height() - left.height();		
	} else {
		left.size.x = rcSplit.x() - left.x();
		right.pos.x = rcSplit.x() + rcSplit.width();
		right.size.x = rect.width() - rcSplit.width() - left.width();
	}

	::MoveWindow(children.first->handle(), left.x(), left.y(), left.width(), left.height(), TRUE);
	::MoveWindow(children.second->handle(), right.x(), right.y(), right.width(), right.height(), TRUE);

	this->setBounds(rcSplit);
}

#endif
