#include "../../include/smartwin/widgets/DateTime.h"

namespace SmartWin {

DateTime::Seed::Seed() :
	Widget::Seed(DATETIMEPICK_CLASS, WS_CHILD | WS_VISIBLE | WS_TABSTOP | DTS_SHORTDATEFORMAT),
	font(new Font(DefaultGuiFont)),
	format(_T( "yyyy.MM.dd" )),
	backgroundColor(0x000080),
	monthBackgroundColor(0x808080),
	monthTextColor(0xFFFFFF),
	titleBackgroundColor(0x202020),
	titleTextColor(0x008080),
	trailingTextColor(0x000000)
{
	::GetSystemTime( &initialDateTime );
}

void DateTime::create( const Seed & cs )
{
	ControlType::create(cs);
	if(cs.font)
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
