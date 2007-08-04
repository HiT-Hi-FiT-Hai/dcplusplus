#include "../../include/smartwin/widgets/WidgetRichTextBox.h"

namespace SmartWin {

const WidgetRichTextBox::Seed & WidgetRichTextBox::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = RICHEDIT_CLASS;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | WS_BORDER | ES_WANTRETURN;
		d_DefaultValues.backgroundColor = RGB( 255, 255, 255 );
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_DefaultValues.scrollBarHorizontallyFlag = false;
		d_DefaultValues.scrollBarVerticallyFlag = false;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetRichTextBox::create( const Seed & cs )
{
	// Need to load up RichEdit library!
	static LibraryLoader richEditLibrary( _T( "riched20.dll" ) );

	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create( cs );
	setFont( cs.font );
	setBackgroundColor( cs.backgroundColor );
	setScrollBarHorizontally( cs.scrollBarHorizontallyFlag );
	setScrollBarVertically( cs.scrollBarVerticallyFlag );
}


}
