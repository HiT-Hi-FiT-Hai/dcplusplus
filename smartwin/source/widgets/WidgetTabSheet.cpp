#include "../../include/smartwin/widgets/WidgetTabSheet.h"

namespace SmartWin {

const WidgetTabSheet::Seed & WidgetTabSheet::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = WC_TABCONTROL;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetTabSheet::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
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
	::RECT d_Answer;
	Point d_Size = this->getSize();

	d_Answer.left = d_Answer.top = 0;
	d_Answer.right = d_Size.x;
	d_Answer.bottom = d_Size.y;
	TabCtrl_AdjustRect( this->handle(), false, & d_Answer );
	return Rectangle::FromRECT( d_Answer );
}

}
