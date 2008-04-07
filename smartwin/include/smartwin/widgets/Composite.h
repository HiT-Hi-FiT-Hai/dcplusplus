#ifndef COMPOSITE_H_
#define COMPOSITE_H_

#include "../forward.h"

#include "../aspects/AspectActivate.h"
#include "../aspects/AspectCommand.h"
#include "../aspects/AspectDragDrop.h"
#include "../aspects/AspectEraseBackground.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectText.h"
#include "../resources/Icon.h"
#include "../Policies.h"
#include "../WidgetCreator.h"
#include "../WindowClass.h"
#include "Control.h"

#include <boost/scoped_ptr.hpp>

namespace SmartWin {

template<typename Policy>
class Composite : 
	public Control<Policy>,
	// Aspects
	public AspectActivate< Composite< Policy > >,
	public AspectCommand< Composite< Policy > >,
	public AspectDragDrop< Composite< Policy > >,
	public AspectEraseBackground< Composite< Policy > >,
	public AspectFocus< Composite< Policy > >,
	public AspectFont< Composite< Policy > >,
	public AspectPainting< Composite< Policy > >,
	public AspectText< Composite< Policy > >
{
public:
	typedef Composite<Policy> ThisType;
	
	typedef ThisType* ObjectType;
	
	typedef Control<Policy> BaseType;
	
	// TODO Maybe move this to a separate class?
	
	typedef SmartWin::ButtonPtr ButtonPtr;
	typedef SmartWin::CheckBoxPtr CheckBoxPtr;
	typedef SmartWin::DateTimePtr DateTimePtr;
	typedef SmartWin::LabelPtr LabelPtr;
	typedef SmartWin::TablePtr TablePtr;
	typedef SmartWin::TreePtr TreePtr;
	typedef SmartWin::SpinnerPtr SpinnerPtr;
	
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	struct Seed : public BaseType::Seed {
		IconPtr icon;
		IconPtr smallIcon;
		HBRUSH background;
		LPCTSTR menuName;
		HCURSOR cursor;

		/// Fills with default parameters
		Seed(const SmartUtil::tstring& caption, DWORD style, DWORD exStyle);
	};

	template<typename SeedType>
	typename SeedType::WidgetType::ObjectType addChild(const SeedType& seed) {
		return WidgetCreator<typename SeedType::WidgetType>::create(this, seed);
	}

	virtual void create(const Seed& cs);
protected:
	friend class WidgetCreator<Composite<Policy> >;
	
	explicit Composite( Widget * parent ) : BaseType( parent ) 
	{};

private:
	boost::scoped_ptr<WindowClass> windowClass;
};

template<typename Policy>
Composite<Policy>::Seed::Seed(const SmartUtil::tstring& caption, DWORD style, DWORD exStyle) : 
	BaseType::Seed(NULL, style | WS_CLIPCHILDREN, 0, caption),
	background(( HBRUSH )( COLOR_APPWORKSPACE + 1 )),
	menuName(NULL),
	cursor(NULL)
{
}

template<typename Policy>
void Composite<Policy>::create(const Seed& cs) {
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, cs.menuName, cs.background, cs.icon, cs.smallIcon, cs.cursor));
	
	Seed cs2 = cs;
	cs2.className = windowClass->getClassName();
	BaseType::create( cs2 );
}

}

#endif /*COMPOSITE_H_*/
