#include "../../include/smartwin/widgets/CoolBar.h"

namespace SmartWin {

CoolBar::Seed::Seed() : 
	Widget::Seed(REBARCLASSNAME, WS_CHILD | WS_VISIBLE | RBS_VARHEIGHT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER)
{
}

void CoolBar::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	//TODO: use CreationalInfo parameters
}

void CoolBar::addChild( Widget * child,
	unsigned width, unsigned height, const SmartUtil::tstring & txt
	)
{
	REBARBANDINFO rbBand;
	rbBand.cbSize = sizeof( REBARBANDINFO );
	rbBand.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE;
	if ( txt != _T( "" ) )
	{
		rbBand.fMask |= RBBIM_TEXT;
		rbBand.lpText = const_cast < TCHAR * >( txt.c_str() );
	}
	rbBand.hwndChild = child->handle();
	rbBand.cxMinChild = width;
	rbBand.cyMinChild = height;
	rbBand.cx = width;
	rbBand.fStyle = 0; //RBBS_GRIPPERALWAYS;
	if ( SendMessage( this->handle(), RB_INSERTBAND, ( WPARAM ) - 1, ( LPARAM ) & rbBand ) == 0 )
	{
		throw xCeption( _T( "There was a problem when trying to insert a band into your Coolbar object!" ) );
	}
}


}
