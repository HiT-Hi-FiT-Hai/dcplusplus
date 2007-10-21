#ifndef ASPECTCONTROL_H_
#define ASPECTCONTROL_H_

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

	unsigned int getControlId() {
		return static_cast<unsigned int>(::GetWindowLongPtr(static_cast<WidgetType*>(this)->handle(), GWLP_ID));
	}
};

}

#endif /*ASPECTCONTROL_H_*/
