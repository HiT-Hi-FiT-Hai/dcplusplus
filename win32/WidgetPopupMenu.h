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

#ifndef DCPLUSPLUS_WIN32_WIDGETPOPUPMENU_H_
#define DCPLUSPLUS_WIN32_WIDGETPOPUPMENU_H_

template< class EventHandlerClass, class MessageMapPolicy >
class WidgetPopupMenu : public SmartWin::WidgetMenu<EventHandlerClass, MessageMapPolicy> {
private:
	typedef SmartWin::WidgetMenu<EventHandlerClass, MessageMapPolicy> BaseType;
public:
	typedef WidgetPopupMenu<EventHandlerClass, MessageMapPolicy>* ObjectType;

	// Constructor Taking pointer to parent
	explicit WidgetPopupMenu( SmartWin::Widget * parent ) : SmartWin::Widget(parent), BaseType(parent) { }

	void create()
	{
		HMENU handle = ::CreatePopupMenu();
		if ( !handle )
		{
			SmartWin::xCeption x( _T( "CreatePopupMenu in WidgetPopupManu::create fizzled..." ) );
			throw x;
		}

		// TODO: Is this safe ? ! ?
		// At least we should verify it...
		BOOST_STATIC_ASSERT( sizeof( HWND ) == sizeof( HMENU ) );
		this->SmartWin::Widget::itsHandle = reinterpret_cast< HWND >( handle );

		// Menu controls uses the Windows Message Procedure of the parent window
		BaseType::MessageMapType::itsDefaultWindowProc = NULL;

		this->SmartWin::Widget::registerWidget();
	}
};

#endif /*WIDGETPOPUPMENU_H_*/
