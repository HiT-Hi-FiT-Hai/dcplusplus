#include "../../include/smartwin/widgets/WidgetTabSheet.h"

namespace SmartWin {

WidgetTabSheet::Seed::Seed() :
	Widget::Seed(WC_TABCONTROL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN),
	font(new Font(DefaultGuiFont))
{
}

void WidgetTabSheet::create( const Seed & cs )
{
	ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

unsigned int WidgetTabSheet::addPage( const SmartUtil::tstring & header, unsigned index, LPARAM data, int image )
{
	TCITEM item;
	item.mask = TCIF_TEXT | TCIF_PARAM;
	item.pszText = const_cast < TCHAR * >( header.c_str() );
	item.lParam = data;
	if(image != -1) {
		item.mask |= TCIF_IMAGE;
		item.iImage = image;
	}
	
	int newIdx = TabCtrl_InsertItem( this->handle(), index, & item );
	if ( newIdx == - 1 )
	{
		xCeption x( _T( "Error while trying to add page into Tab Sheet" ) );
		throw x;
	}
	return ( unsigned int ) newIdx;
}

SmartWin::Rectangle WidgetTabSheet::getUsableArea(bool cutBorders) const
{
	RECT rc;
	::GetWindowRect(handle(), &rc);
	::MapWindowPoints(NULL, getParent()->handle(), (LPPOINT)&rc, 2);
	TabCtrl_AdjustRect( this->handle(), false, &rc );
	Rectangle rect( rc );
	if(cutBorders) {
		Rectangle rctabs(getClientAreaSize());
		// Get rid of ugly border...assume y border is the same as x border
		long border = (rctabs.width() - rect.width()) / 2;
		rect.pos.x = rctabs.x();
		rect.size.x = rctabs.width();
		rect.size.y += border;
	}
	return rect;
}

void WidgetTabSheet::setImageList(const ImageListPtr& imageList_)
{
	imageList = imageList_;
	TabCtrl_SetImageList(handle(), imageList->handle());
}

SmartUtil::tstring WidgetTabSheet::getText(unsigned idx) const
{
	TCITEM item = { TCIF_TEXT };
	TCHAR buffer[200];
	item.cchTextMax = 198;
	item.pszText = buffer;
	if ( !TabCtrl_GetItem( this->handle(), idx, & item ) )
	{
		throw xCeption( _T( "Couldn't retrieve text in WidgetTabSheet::getText." ) );
	}
	return buffer;
}

}
