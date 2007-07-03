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
#include "WidgetTextBox.h"
#include "WidgetPaned.h"

/**
 * This is where stuff that eventually should be moved to smartwin goes
 */
template< template< class> class ContainerWidgetType, class EventHandlerClass>
class WidgetFactory : public SmartWin::WidgetFactory<ContainerWidgetType, EventHandlerClass> {
public:
	WidgetFactory() : SmartWin::Widget(0), SmartWin::WidgetFactory<ContainerWidgetType, EventHandlerClass>() { }
	explicit WidgetFactory(SmartWin::Widget* parent) : SmartWin::WidgetFactory<ContainerWidgetType, EventHandlerClass>(parent) { }

	/// DataGrid class type.
	typedef ::WidgetDataGrid< EventHandlerClass > WidgetDataGrid;

	/// DataGrid object type.
	typedef typename WidgetDataGrid::ObjectType WidgetDataGridPtr;

	WidgetDataGridPtr createDataGrid( const typename WidgetDataGrid::Seed & cs = WidgetDataGrid::getDefaultSeed() ) {
		return SmartWin::WidgetCreator< WidgetDataGrid >::create( this, cs );
	}

	/// TextBox class type.
	typedef ::WidgetTextBox< EventHandlerClass > WidgetTextBox;

	/// TextBox object type.
	typedef typename WidgetTextBox::ObjectType WidgetTextBoxPtr;

	WidgetTextBoxPtr createTextBox( const typename WidgetTextBox::Seed & cs = WidgetTextBox::getDefaultSeed() ) {
		return SmartWin::WidgetCreator< WidgetTextBox >::create( this, cs );
	}

	/// VPaned class type.
	typedef WidgetPaned< EventHandlerClass, false > WidgetVPaned;

	/// VPaned object type.
	typedef typename WidgetVPaned::ObjectType WidgetVPanedPtr;

	WidgetVPanedPtr createVPaned( const typename WidgetVPaned::Seed & cs = WidgetVPaned::getDefaultSeed() ) {
		return SmartWin::WidgetCreator< WidgetVPaned >::create( this, cs );
	}

	/// HPaned class type.
	typedef WidgetPaned< EventHandlerClass, true > WidgetHPaned;

	/// HPaned object type.
	typedef typename WidgetHPaned::ObjectType WidgetHPanedPtr;

	WidgetHPanedPtr createHPaned( const typename WidgetHPaned::Seed & cs = WidgetHPaned::getDefaultSeed() ) {
		return SmartWin::WidgetCreator< WidgetHPaned >::create( this, cs );
	}

};

#endif /*WIDGETFACTORY2_H_*/
