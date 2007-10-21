#include "../../include/smartwin/widgets/WidgetMenu.h"

namespace SmartWin {

void WidgetMenu::appendItem
	( unsigned int id, const SmartUtil::tstring & name
	, ULONG_PTR data, const IdDispatcher::F& f
	)
{
	appendItem(id, name, data);
	callbacks.insert(std::make_pair(id, IdDispatcher(f)));
}

void WidgetMenu::appendItem
	( unsigned int id, const SmartUtil::tstring & name
	, ULONG_PTR data, const SimpleDispatcher::F& f
	)
{
	appendItem(id, name, data);
	callbacks.insert(std::make_pair(id, SimpleDispatcher(f)));
}

void WidgetMenu::appendItem( unsigned int id, const SmartUtil::tstring & name
	, ULONG_PTR data) {
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };

	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
	mii.fType = MFT_STRING;
	mii.dwTypeData = const_cast<LPTSTR>(name.c_str());
	mii.dwItemData = data;
	mii.wID = id;
	::InsertMenuItem(this->handle(), this->getCount(), TRUE, &mii);
}

SmartUtil::tstring WidgetMenu::getText( unsigned id, bool byPosition )
{
	MENUITEMINFO mi = { sizeof(MENUITEMINFO) };

	mi.fMask = MIIM_TYPE;
	if ( ::GetMenuItemInfo( this->handle(), id, byPosition, & mi ) == 0 )
	{
		xAssert( false, _T( "Error while trying to get MenuItemInfo in WidgetMenu::getText..." ) );
	}
	boost::scoped_array< TCHAR > buffer( new TCHAR[++mi.cch] );
	mi.dwTypeData = buffer.get();
	if ( ::GetMenuItemInfo( this->handle(), id, byPosition, & mi ) == 0 )
	{
		xAssert( false, _T( "Error while trying to get MenuItemInfo in WidgetMenu::getText..." ) );
	}
	return mi.dwTypeData;
}

void WidgetMenu::setText( unsigned id, const SmartUtil::tstring& text )
{
	MENUITEMINFO info = { sizeof( MENUITEMINFO ) };

	// set flag
	info.fMask = MIIM_STRING;
	info.dwTypeData = (TCHAR*) text.c_str();

	if ( ::SetMenuItemInfo( this->handle(), id, FALSE, & info ) == FALSE )
		throw xCeption( _T( "Couldn't set item info in setItemText()" ) );
}

void WidgetMenuBase::addCommands(Widget* widget) {
	for(CallbackMap::iterator i = callbacks.begin(); i != callbacks.end(); ++i) {
		widget->setCallback(Message(WM_COMMAND, i->first), i->second);
	}
	for(std::vector< std::tr1::shared_ptr<WidgetMenu> >::iterator i = itsChildren.begin(); i != itsChildren.end(); ++i) {
		(*i)->addCommands(widget);
	}
}

unsigned WidgetMenu::trackPopupMenu( Widget * mainWindow, const ScreenCoordinate& sc, unsigned flags )
{
	xAssert( mainWindow != 0, _T( "EventHandlerClass can't be null while trying to display Popup Menu" ) );
	addCommands(mainWindow);
	
	long x = sc.getPoint().x, y = sc.getPoint().y;

	if ( x == - 1 && y == - 1 ) {
		DWORD pos = ::GetMessagePos();
		x = LOWORD( pos );
		y = HIWORD( pos );
	}
	
	int retVal = ::TrackPopupMenu(this->handle(), flags, x, y, 0, mainWindow->handle(), 0);
	return retVal;
}

WidgetMenu::ObjectType WidgetMenu::getChild( unsigned position ) {
	HMENU h = ::GetSubMenu(handle(), position);
	for(size_t i = 0; i < this->itsChildren.size(); ++i) {
		ObjectType& menu = this->itsChildren[i];
		if(menu->handle() == h) {
			return menu;
		}
	}
	return ObjectType();
}

}
