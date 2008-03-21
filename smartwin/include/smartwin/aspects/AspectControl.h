#ifndef ASPECTCONTROL_H_
#define ASPECTCONTROL_H_

#include "../Policies.h"
#include "../xCeption.h"

#include "AspectContextMenu.h"
#include "AspectEnabled.h"
#include "AspectKeyboard.h"
#include "AspectMouse.h"
#include "AspectRaw.h"
#include "AspectSizable.h"
#include "AspectVisible.h"

namespace SmartWin {

/** This class is for all windows common controls */
template<typename WidgetType >
class AspectControl : 
	public MessageMapPolicy< Policies::Subclassed >,

	public AspectContextMenu<WidgetType>,
	public AspectEnabled<WidgetType>,
	public AspectKeyboard<WidgetType>,
	public AspectMouse<WidgetType>,
	public AspectRaw<WidgetType>,
	public AspectSizable<WidgetType>,
	public AspectVisible<WidgetType>
{
public:
	/// Class type
	typedef WidgetType ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Policy type
	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;
	
	unsigned int getControlId();
	
	virtual HWND create(const Seed& cs);
protected:
	typedef AspectControl<WidgetType> ControlType;

	AspectControl(Widget* parent);
};

template<typename WidgetType>
AspectControl<WidgetType>::AspectControl(Widget* parent) : PolicyType(parent) {
	xAssert( parent, _T( "Common Controls must have a parent" ) );
}

template<typename WidgetType>
HWND AspectControl<WidgetType>::create(const Seed& cs) {
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Common controls must have WS_CHILD style"));
	return PolicyType::create(cs);
}

template<typename WidgetType>
unsigned int AspectControl<WidgetType>::getControlId() {
	return static_cast<unsigned int>(::GetWindowLongPtr(static_cast<WidgetType*>(this)->handle(), GWLP_ID));
}

}

#endif /*ASPECTCONTROL_H_*/
