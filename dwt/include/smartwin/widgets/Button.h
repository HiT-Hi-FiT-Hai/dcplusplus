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

/** Common stuff for all buttons */
class Button :
	public CommonControl,
	public AspectClickable<Button>,
	public AspectColor<Button>,
	public AspectColorCtlImpl<Button>,
	public AspectDblClickable<Button>,
	public AspectFocus< Button >,
	public AspectFont< Button >,
	public AspectPainting< Button >,
	public AspectText< Button >
{
	friend class AspectClickable<Button>;
	friend class AspectDblClickable<Button>;
	friend class WidgetCreator<Button>;
public:
	/// Class type
	typedef Button ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	typedef CommonControl BaseType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;
		
		FontPtr font;

		/// Fills with default parameters
		Seed(const SmartUtil::tstring& caption_ = SmartUtil::tstring(), DWORD style = 0);
	};

	template<typename SeedType>
	void create(const SeedType& cs = SeedType());

protected:
	typedef Button ButtonType;
	
	Button(Widget* parent);
	
private:
	// Contract needed by AspectClickable Aspect class
	static Message getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static Message getDblClickMessage();

};

inline Message Button::getClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(0, BN_CLICKED) );
}

inline Message Button::getDblClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(0, BN_DBLCLK) );
}

inline Button::Button(Widget* parent) : BaseType(parent) {
	
}

template<typename SeedType>
void Button::create( const SeedType & cs ) {
	BaseType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

}

#endif /*ASPECTBUTTON_H_*/
