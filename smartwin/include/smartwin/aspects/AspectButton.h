#ifndef ASPECTBUTTON_H_
#define ASPECTBUTTON_H_

#include "AspectBackgroundColor.h"
#include "AspectBorder.h"
#include "AspectClickable.h"
#include "AspectControl.h"
#include "AspectDblClickable.h"
#include "AspectFocus.h"
#include "AspectFont.h"
#include "AspectPainting.h"
#include "AspectText.h"

namespace SmartWin {
/** Common stuff for all buttons */
template<typename WidgetType>
class AspectButton :
	public AspectBackgroundColor<WidgetType>,
	public AspectBorder< WidgetType >,
	public AspectClickable<WidgetType>,
	public AspectControl<WidgetType>,
	public AspectDblClickable<WidgetType>,
	public AspectFocus< WidgetType >,
	public AspectFont< WidgetType >,
	public AspectPainting< WidgetType >,
	public AspectText< WidgetType >
{
public:
	
	// Contract needed by AspectBackgroundColor Aspect class
	static const Message & getBackgroundColorMessage();

	// Contract needed by AspectClickable Aspect class
	Message getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	Message getDblClickMessage();
	
	template<typename SeedType>
	void create(const SeedType& cs);
	
protected:
	typedef AspectButton<WidgetType> ButtonType;
	
	AspectButton(Widget* parent);
};

template<typename WidgetType>
inline const Message & AspectButton<WidgetType>::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORBTN );
	return retVal;
}

template<typename WidgetType>
Message AspectButton<WidgetType>::getClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(static_cast<WidgetType*>(this)->getControlId(), BN_CLICKED) );
}

template<typename WidgetType>
Message AspectButton<WidgetType>::getDblClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(static_cast<WidgetType*>(this)->getControlId(), BN_CLICKED) );
}

template<typename WidgetType>
AspectButton<WidgetType>::AspectButton(Widget* parent) : AspectControl<WidgetType>(parent) {
	
}

template<typename WidgetType>
template<typename SeedType>
void AspectButton<WidgetType>::create( const SeedType & cs ) {
	WidgetType::ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

}

#endif /*ASPECTBUTTON_H_*/
