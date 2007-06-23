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

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy = SmartWin::MessageMapPolicyNormalWidget >
class WidgetPaned :
	public SmartWin::MessageMapControl< EventHandlerClass, WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >, MessageMapPolicy >,

	public SmartWin::AspectSizable< EventHandlerClass, WidgetPaned<EventHandlerClass, horizontal, MessageMapPolicy >,
		SmartWin::MessageMapControl< EventHandlerClass, WidgetPaned<EventHandlerClass, horizontal, MessageMapPolicy >, MessageMapPolicy > >,
	public SmartWin::AspectVisible< EventHandlerClass, WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >,
		SmartWin::MessageMapControl< EventHandlerClass, WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >, MessageMapPolicy > >,
	public SmartWin::AspectRaw< EventHandlerClass, WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >,
		SmartWin::MessageMapControl< EventHandlerClass, WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef SmartWin::MessageMapControl< EventHandlerClass, WidgetPaned, MessageMapPolicy > ThisMessageMap;
	friend class SmartWin::WidgetCreator< WidgetPaned >;
public:
	/// Class type
	typedef WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy > * ObjectType;

	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetPaned::ThisType WidgetType;

		explicit Seed();

		/// Doesn't fill any values
		Seed( SmartWin::DontInitialize )
		{}
	};

	static const Seed & getDefaultSeed();

	void setRelativePos(double pos);
	
	void setFirst(SmartWin::Widget* child) {
		children.first = child;
		resizeChildren();
	}
	
	void setSecond(SmartWin::Widget* child) {
		children.second = child;
		resizeChildren();
	}

	virtual void create( const Seed & cs = getDefaultSeed() );

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
	
	RECT getSplitterRect();
	
	void resizeChildren();
	
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );
};

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy >
const typename WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::Seed & WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( SmartWin::DontInitializeMe );

	if ( d_NeedsInit )
	{
		SMARTWIN_WNDCLASSEX wc = { sizeof(SMARTWIN_WNDCLASSEX) };

		SmartWin::Application::instance().generateLocalClassName( d_DefaultValues );
		wc.hInstance = SmartWin::Application::instance().getAppHandle();
		wc.lpszClassName = d_DefaultValues.getClassName().c_str();
		wc.hbrBackground = ( HBRUSH )( COLOR_3DFACE + 1 );
		wc.lpfnWndProc = SmartWin::MessageMapPolicyNormalWidget::mainWndProc_;
		ATOM registeredClass = SmartWinRegisterClass( & wc );
		if ( 0 == registeredClass )
		{
			assert( false && "WidgetPaned::create() SmartWinRegisterClass fizzled..." );
			SmartWin::xCeption x( _T( "WidgetPaned::create() SmartWinRegisterClass fizzled..." ) );
			throw x;
		}
		SmartWin::Application::instance().addLocalWindowClassToUnregister( d_DefaultValues );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy >
WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetPaned::getDefaultSeed();
}

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy >
WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::WidgetPaned( SmartWin::Widget * parent )
	: SmartWin::Widget( parent, 0 )
	, pos(0.5)
	, moving(false)
{
	children.first = children.second = 0;
}

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy >
void WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::create( const Seed & cs )
{
	// TODO: MessageMap instead of MessageMapControl
	this->ThisMessageMap::isSubclassed = false;
	// TODO: use CreationalInfo parameters
	if ( cs.style & WS_CHILD )
		SmartWin::Widget::create( cs );
	else
	{
		typename WidgetPaned::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		SmartWin::Widget::create( d_YouMakeMeDoNastyStuff );
	}
}

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy >
RECT WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::getSplitterRect()
{
	// Sanity check
	if(pos < 0.) {
		pos = 0.0;
	} else if(pos > 1.0) {
		pos = 1.0;
	}
	
	RECT rc = { 0 };
	if(!children.first || !children.second) {
		return rc;
	}
	
	::GetClientRect(this->SmartWin::Widget::handle(), &rc);
	int cwidth = rc.right - rc.left;
	int swidth = ::GetSystemMetrics(SM_CXEDGE) + 2;
	int realpos = static_cast<int>(pos * cwidth);
	rc.left = realpos - swidth / 2;
	rc.right = realpos - (swidth - swidth/2); // Catch odd size
	return rc;
}

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy >
void WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::resizeChildren( )
{
	RECT rc;
	::GetClientRect(this->SmartWin::Widget::handle(), &rc);
	
	if(!children.first) {
		if(children.second) {
			::MoveWindow(children.second->handle(), rc.left, rc.top, rc.right-rc.left, rc.bottom - rc.top, TRUE);
		}
		return;
	}
	if(!children.second) {
		::MoveWindow(children.first->handle(), rc.left, rc.top, rc.right-rc.left, rc.bottom - rc.top, TRUE);
		return;
	}
	
	RECT left = rc, right = rc;
	RECT rcSplit = getSplitterRect();
	left.right = rcSplit.left;
	right.left = rcSplit.right;

	::MoveWindow(children.first->handle(), left.left, left.top, left.right-left.left, left.bottom - left.top, TRUE);
	::MoveWindow(children.second->handle(), right.left, right.top, right.right-right.left, right.bottom - right.top, TRUE);

	::InvalidateRect(this->SmartWin::Widget::handle(), &rcSplit, TRUE);
}

template< typename EventHandlerClass, bool horizontal, class MessageMapPolicy >
LRESULT WidgetPaned< EventHandlerClass, horizontal, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	switch ( msg )
	{
		case WM_LBUTTONDOWN :
		{
			POINT pt = { GET_X_LPARAM(lPar), GET_Y_LPARAM(lPar) };
			RECT rc = getSplitterRect();

			if(::PtInRect(&rc, pt)) {
				::SetCapture( this->SmartWin::Widget::handle() );
			}

			return 0;
		}
		case WM_MOUSEMOVE :
		{
			if ( wPar & MK_LBUTTON && moving )
			{
				
				RECT rc;
				::GetClientRect(this->SmartWin::Widget::handle(), &rc);
				int x = GET_X_LPARAM(lPar);
				int w = rc.right - rc.left;
				pos = (static_cast<double>(w - x) / static_cast<double>(w));
				resizeChildren();
			}
			return 0;
		}
		case WM_LBUTTONUP :
		{
			::ReleaseCapture();
			moving = false;

			return 0;
		}
		case WM_SIZE :
		{
			resizeChildren();
			return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
		} break;
		case WM_MOVE :
		{
			resizeChildren();
			return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
		} break;
		default:
			return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
	}

	// Removing compiler hickup...
	return 0;
}

#endif

