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

SmartWin::Rectangle WidgetTabSheet::getUsableArea() const
{
	RECT rc;
	::GetWindowRect(handle(), &rc);
	::MapWindowPoints(NULL, getParent()->handle(), (LPPOINT)&rc, 2);
	TabCtrl_AdjustRect( this->handle(), false, &rc );
	return Rectangle( rc );
}

void WidgetTabSheet::setImageList(const ImageListPtr& imageList_)
{
	imageList = imageList_;
	TabCtrl_SetImageList(handle(), imageList->handle());
}

}
