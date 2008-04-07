#include "../../include/smartwin/widgets/ColorDialog.h"

namespace SmartWin {

bool ColorDialog::open( ColorParams & colorParams, bool basic, bool allowFullOpen ) {
	CHOOSECOLOR cc = { sizeof( CHOOSECOLOR ) };
	cc.hwndOwner = getParentHandle();
	cc.rgbResult = colorParams.itsColor;
	cc.lpCustColors = itsColorParams.itsCustomColors;
	cc.Flags = CC_ANYCOLOR | CC_RGBINIT;
	if ( !basic )
		cc.Flags |= CC_FULLOPEN;
	if ( !allowFullOpen )
		cc.Flags |= CC_PREVENTFULLOPEN;

	if(::ChooseColor( & cc )) {
		colorParams.itsColor = cc.rgbResult;
		return true;
	}
	return false;
}

COLORREF defaultColors[16] = {
	0x00000000, 0x33333333, 0x66666666, 0x99999999, 
	0xCCCCCCCC, 0xFFFFFFFF,	0x0000FFFF, 0x000000FF, 
	0x0000FF00, 0x00FF0000, 0x00FFFF00,	0x00FF00FF,
	0x002244BB,	0x0000CC99,	0x00AA7700,	0x00997755
};

ColorDialog::ColorParams::ColorParams()
	: itsColor( 0x0000FFFF )
{
	memcpy(itsCustomColors, defaultColors, sizeof(itsCustomColors));
}

ColorDialog::ColorParams::ColorParams( COLORREF defaultColor )
	: itsColor( defaultColor )
{
	memcpy(itsCustomColors, defaultColors, sizeof(itsCustomColors));
}

}
