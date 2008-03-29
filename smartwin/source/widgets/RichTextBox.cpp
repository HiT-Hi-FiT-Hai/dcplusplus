#include "../../include/smartwin/widgets/RichTextBox.h"

namespace SmartWin {

RichTextBox::Seed::Seed() :
	Widget::Seed(RICHEDIT_CLASS, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | WS_BORDER | ES_WANTRETURN),
	font(new Font(DefaultGuiFont)),
	backgroundColor(RGB( 255, 255, 255 )),
	scrollBarHorizontallyFlag(false),
	scrollBarVerticallyFlag(false)
{
}

void RichTextBox::create( const Seed & cs )
{
	// Need to load up RichEdit library!
	static LibraryLoader richEditLibrary( _T( "riched20.dll" ) );

	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create( cs );
	if(cs.font)
		setFont( cs.font );
	setBackgroundColor( cs.backgroundColor );
	setScrollBarHorizontally( cs.scrollBarHorizontallyFlag );
	setScrollBarVertically( cs.scrollBarVerticallyFlag );
}


}
