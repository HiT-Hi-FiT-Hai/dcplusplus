#include "../../include/smartwin/widgets/WidgetDateTimePicker.h"

namespace SmartWin {

const WidgetDateTimePicker::Seed & WidgetDateTimePicker::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = DATETIMEPICK_CLASS;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT;
		d_DefaultValues.backgroundColor = 0x000080;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_DefaultValues.format = _T( "yyyy.MM.dd" ); //TODO: should be filled out with locale from OS
		GetSystemTime( & d_DefaultValues.initialDateTime );
		d_DefaultValues.monthBackgroundColor = 0x808080;
		d_DefaultValues.monthTextColor = 0xFFFFFF;
		d_DefaultValues.titleBackgroundColor = 0x202020;
		d_DefaultValues.titleTextColor = 0x008080;
		d_DefaultValues.trailingTextColor = 0x000000;
		//TODO: initialize the values here

		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetDateTimePicker::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	//TODO: use CreationalInfo parameters
	setFont( cs.font );
	setFormat( cs.format );
	setDateTime( cs.initialDateTime );
	setBackgroundColor( cs.backgroundColor );
	setMonthBackgroundColor( cs.monthBackgroundColor );
	setMonthTextColor( cs.monthTextColor );
	setTitleBackgroundColor( cs.titleBackgroundColor );
	setTrailingTextColor( cs.trailingTextColor );
	setTitleTextColor( cs.titleTextColor );
}

}
