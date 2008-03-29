#ifndef ASPECTBUTTON_H_
#define ASPECTBUTTON_H_

#include "../aspects/AspectColor.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectText.h"
#include "Control.h"

namespace SmartWin {

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/** Common stuff for all buttons */
class Button :
	public Control,
	public AspectClickable<Button>,
	public AspectColor<Button>,
	public AspectColorCtlImpl<Button>,
	public AspectDblClickable<Button>,
	public AspectFocus< Button >,
	public AspectFont< Button >,
	public AspectPainting< Button >,
	public AspectText< Button >
{
	friend class WidgetCreator<Button>;
public:
	/// Class type
	typedef Button ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed : public Widget::Seed {
	public:
		FontPtr font;

		/// Fills with default parameters
		Seed(const SmartUtil::tstring& caption_ = SmartUtil::tstring());
	};

	// Contract needed by AspectClickable Aspect class
	Message getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	Message getDblClickMessage();
	
	template<typename SeedType>
	void create(const SeedType& cs);
	
protected:
	typedef Button ButtonType;
	
	Button(Widget* parent);
};

inline Message Button::getClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(getControlId(), BN_CLICKED) );
}

inline Message Button::getDblClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(getControlId(), BN_DBLCLK) );
}

inline Button::Button(Widget* parent) : ControlType(parent) {
	
}

template<typename SeedType>
void Button::create( const SeedType & cs ) {
	ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

}

#endif /*ASPECTBUTTON_H_*/
