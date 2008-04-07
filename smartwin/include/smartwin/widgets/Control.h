#ifndef CONTROL_H_
#define CONTROL_H_

#include "../Policies.h"
#include "../xCeption.h"

#include "../aspects/AspectBorder.h"
#include "../aspects/AspectCloseable.h"
#include "../aspects/AspectContextMenu.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectHelp.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectMouse.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectTimer.h"
#include "../aspects/AspectVisible.h"

namespace SmartWin {

/** Base class for all windows */
template<typename Policy>
class Control: 
	public MessageMap<Policy>,

	public AspectBorder<Control<Policy> >,
	public AspectCloseable<Control<Policy> >,
	public AspectContextMenu<Control<Policy> >,
	public AspectEnabled<Control<Policy> >,
	public AspectHelp<Control<Policy> >,
	public AspectKeyboard<Control<Policy> >,
	public AspectMouse<Control<Policy> >,
	public AspectRaw<Control<Policy> >,
	public AspectSizable<Control<Policy> >,
	public AspectTimer<Control<Policy> >,
	public AspectVisible<Control<Policy> >
{
public:
	typedef MessageMap<Policy> BaseType;

	struct Seed : public BaseType::Seed {
		Seed(LPCTSTR className, DWORD style);
	};

protected:
	typedef Control<Policy> ControlType;

	Control(Widget* parent);
};

template<typename Policy>
Control<Policy>::Control(Widget* parent) : MessageMap<Policy>(parent) {
	
}

typedef Control<Policies::Subclassed> CommonControl;

template<typename Policy>
Control<Policy>::Seed::Seed(LPCTSTR className, DWORD style) : 
	BaseType::Seed(NULL, style | WS_VISIBLE)
{
	
}

}

#endif /*CONTROL_H_*/
