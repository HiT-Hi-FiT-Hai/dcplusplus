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

/** Base class for all windows */
template<typename Policy>
class Control: 
	public MessageMap<Policy>,

	public AspectBorder<Control<Policy> >,
	public AspectContextMenu<Control<Policy> >,
	public AspectEnabled<Control<Policy> >,
	public AspectHelp<Control<Policy> >,
	public AspectKeyboard<Control<Policy> >,
	public AspectMouse<Control<Policy> >,
	public AspectRaw<Control<Policy> >,
	public AspectSizable<Control<Policy> >,
	public AspectVisible<Control<Policy> >
{
public:
	
protected:
	typedef Control<Policy> ControlType;

	Control(Widget* parent);
};

template<typename Policy>
Control<Policy>::Control(Widget* parent) : MessageMap<Policy>(parent) {
	
}

typedef Control<Policies::Subclassed> CommonControl;

}

#endif /*CONTROL_H_*/
