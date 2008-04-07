#ifndef BRUSH_H_
#define BRUSH_H_

#include "../WindowsHeaders.h"
#include "Handle.h"
#include <boost/intrusive_ptr.hpp>

namespace SmartWin {

class Brush;

typedef boost::intrusive_ptr< Brush > BrushPtr;

class Brush : public Handle<GdiPolicy<HBRUSH> > {
public:
	enum SysColor
	{
		Scrollbar = COLOR_SCROLLBAR,
		Background = COLOR_BACKGROUND,
		ActiveCaption = COLOR_ACTIVECAPTION,
		InActiveCaption = COLOR_INACTIVECAPTION,
		Menu = COLOR_MENU,
		Window = COLOR_WINDOW,
		WindowFrame = COLOR_WINDOWFRAME,
		MenuText = COLOR_MENUTEXT,
		WindowText = COLOR_WINDOWTEXT,
		CaptionText = COLOR_CAPTIONTEXT,
		ActiveBorder = COLOR_ACTIVEBORDER,
		InActiveBorder = COLOR_INACTIVEBORDER,
		AppWorkSpace = COLOR_APPWORKSPACE,
		HighLight = COLOR_HIGHLIGHT,
		HighLightText = COLOR_HIGHLIGHTTEXT,
		BtnFace = COLOR_BTNFACE,
		BtnShadow = COLOR_BTNSHADOW,
		GrayText = COLOR_GRAYTEXT,
		BtnText = COLOR_BTNTEXT,
		InActiveCaptionText = COLOR_INACTIVECAPTIONTEXT,
		BtnHighLight = COLOR_BTNHIGHLIGHT,
		Face3D = COLOR_3DFACE,
		Highlight3D = COLOR_3DHIGHLIGHT,
		Shadow3D = COLOR_3DSHADOW,
		InfoText = COLOR_INFOTEXT,
		InfoBk = COLOR_INFOBK,
#ifdef WINCE
		Static = COLOR_STATIC,
		StaticText = COLOR_STATICTEXT,
#else  //! WINCE
		Static = COLOR_BACKGROUND, // try ?
		StaticText = COLOR_BTNTEXT,
#endif
		GradientActiveCaption = COLOR_GRADIENTACTIVECAPTION,
		GradientInActiveCaption = COLOR_GRADIENTINACTIVECAPTION
	};

	explicit Brush(HBRUSH h, bool own = true);
	
	explicit Brush(SysColor color);
	
	explicit Brush(COLORREF color);

private:
	friend class Handle<GdiPolicy<HBRUSH> >;
	typedef Handle<GdiPolicy<HBRUSH> > ResourceType;

};
}
#endif /*BRUSH_H_*/
