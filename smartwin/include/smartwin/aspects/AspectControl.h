#ifndef ASPECTCONTROL_H_
#define ASPECTCONTROL_H_

#include "../Policies.h"
#include "../xCeption.h"

#include "AspectBorder.h"
#include "AspectContextMenu.h"
#include "AspectEnabled.h"
#include "AspectHelp.h"
#include "AspectKeyboard.h"
#include "AspectMouse.h"
#include "AspectRaw.h"
#include "AspectSizable.h"
#include "AspectVisible.h"

namespace SmartWin {

/** This class is for all windows common controls */

class AspectControl : 
	public MessageMapPolicy< Policies::Subclassed >,

	public AspectBorder<AspectControl>,
	public AspectContextMenu<AspectControl>,
	public AspectEnabled<AspectControl>,
	public AspectHelp<AspectControl>,
	public AspectKeyboard<AspectControl>,
	public AspectMouse<AspectControl>,
	public AspectRaw<AspectControl>,
	public AspectSizable<AspectControl>,
	public AspectVisible<AspectControl>
{
public:
	unsigned int getControlId();
	
	virtual HWND create(const Seed& cs);
protected:
	typedef AspectControl ControlType;

	AspectControl(Widget* parent);
};

inline AspectControl::AspectControl(Widget* parent) : PolicyType(parent) {
	xAssert( parent, _T( "Common Controls must have a parent" ) );
}

inline HWND AspectControl::create(const Seed& cs) {
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Common controls must have WS_CHILD style"));
	return PolicyType::create(cs);
}

inline unsigned int AspectControl::getControlId() {
	return static_cast<unsigned int>(::GetWindowLongPtr(handle(), GWLP_ID));
}

}

#endif /*ASPECTCONTROL_H_*/
