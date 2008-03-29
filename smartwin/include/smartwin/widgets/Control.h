#ifndef CONTROL_H_
#define CONTROL_H_

#include "../Policies.h"
#include "../xCeption.h"

#include "../aspects/AspectBorder.h"
#include "../aspects/AspectContextMenu.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectHelp.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectMouse.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"

namespace SmartWin {

/** Base class is for windows common controls */

class Control : 
	public MessageMapPolicy< Policies::Subclassed >,

	public AspectBorder<Control>,
	public AspectContextMenu<Control>,
	public AspectEnabled<Control>,
	public AspectHelp<Control>,
	public AspectKeyboard<Control>,
	public AspectMouse<Control>,
	public AspectRaw<Control>,
	public AspectSizable<Control>,
	public AspectVisible<Control>
{
public:
	unsigned int getControlId();
	
	virtual HWND create(const Seed& cs);
protected:
	typedef Control ControlType;

	Control(Widget* parent);
};

inline Control::Control(Widget* parent) : PolicyType(parent) {
	xAssert( parent, _T( "Common Controls must have a parent" ) );
}

inline HWND Control::create(const Seed& cs) {
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Common controls must have WS_CHILD style"));
	return PolicyType::create(cs);
}

inline unsigned int Control::getControlId() {
	return static_cast<unsigned int>(::GetWindowLongPtr(handle(), GWLP_ID));
}

}

#endif /*CONTROL_H_*/
