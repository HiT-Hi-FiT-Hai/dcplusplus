#ifndef PEN_H_
#define PEN_H_

#include "../WindowsHeaders.h"
#include "Handle.h"
#include <boost/intrusive_ptr.hpp>

namespace SmartWin {

class Pen;

typedef boost::intrusive_ptr< Pen > PenPtr;

class Pen : public Handle<GdiPolicy<HPEN> > {
public:
	enum PenStyle {
		Solid = PS_SOLID,
		Dash = PS_DASH,
		Dot = PS_DOT,
		DashDot = PS_DASHDOT,
		DashDotDot = PS_DASHDOTDOT,
		Null = PS_NULL,
		InsideFrame = PS_INSIDEFRAME
	};
	explicit Pen(HPEN h, bool own = true);
	
	explicit Pen(COLORREF color, PenStyle style = Solid, int width = 0);

private:
	friend class Handle<GdiPolicy<HPEN> >;
	typedef Handle<GdiPolicy<HPEN> > ResourceType;

};
}

#endif /*PEN_H_*/
