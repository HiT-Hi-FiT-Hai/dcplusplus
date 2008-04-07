#include "../../include/smartwin/widgets/FontDialog.h"

namespace SmartWin {

bool FontDialog::open(DWORD dwFlags, LOGFONT& font, DWORD& rgbColors)
{
	CHOOSEFONT cf = { sizeof(CHOOSEFONT) };

	// Initialize CHOOSEFONT
	cf.hwndOwner = getParentHandle();
	cf.Flags = dwFlags | CF_INITTOLOGFONTSTRUCT;
	cf.lpLogFont = &font;
	cf.rgbColors = rgbColors;

	if ( ::ChooseFont( & cf ) )
	{
		rgbColors = cf.rgbColors;
		return true;
	}
	return false;
}

}
