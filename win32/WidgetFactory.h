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

#ifndef DCPLUSPLUS_WIN32_WIDGETFACTORY2_H_
#define DCPLUSPLUS_WIN32_WIDGETFACTORY2_H_

#include "WidgetDataGrid.h"

/**
 * This is where stuff that eventually should be moved to smartwin goes
 */
template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy = SmartWin::MessageMapPolicyNormalWidget >
class WidgetFactory : public SmartWin::WidgetFactory<ContainerWidgetType, EventHandlerClass, MessageMapPolicy> {
public:
	WidgetFactory() : SmartWin::WidgetFactory<ContainerWidgetType, EventHandlerClass, MessageMapPolicy>() { }
	explicit WidgetFactory(SmartWin::Widget* parent) : SmartWin::WidgetFactory<ContainerWidgetType, EventHandlerClass, MessageMapPolicy>(parent) { }
	
	/// DataGrid class type.
	typedef ::WidgetDataGrid< EventHandlerClass, MessageMapPolicy > WidgetDataGrid;

	/// DataGrid object type.
	typedef typename WidgetDataGrid::ObjectType WidgetDataGridPtr;
	
	WidgetDataGridPtr createDataGrid( const typename WidgetDataGrid::Seed & cs = WidgetDataGrid::getDefaultSeed() );
};

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetDataGridPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createDataGrid( const typename WidgetDataGrid::Seed & cs )
{
	return SmartWin::WidgetCreator< WidgetDataGrid >::create( this, cs );
}

#endif /*WIDGETFACTORY2_H_*/
