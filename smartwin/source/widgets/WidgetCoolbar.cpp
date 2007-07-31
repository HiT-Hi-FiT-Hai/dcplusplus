#include "../../include/smartwin/widgets/WidgetCoolbar.h"

namespace SmartWin {

const WidgetCoolbar::Seed & WidgetCoolbar::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = REBARCLASSNAME;
		d_DefaultValues.exStyle = WS_EX_TOOLWINDOW;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | RBS_VARHEIGHT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER;
		//TODO: fill the values
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetCoolbar::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	//TODO: use CreationalInfo parameters
}

void WidgetCoolbar::addChild( Widget * child,
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
