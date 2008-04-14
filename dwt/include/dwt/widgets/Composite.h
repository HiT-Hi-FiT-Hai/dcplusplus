/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
        may be used to endorse or promote products derived from this software 
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DWT_COMPOSITE_H_
#define DWT_COMPOSITE_H_

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

namespace dwt {

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
	typedef Control<Policy> BaseType;
	
public:
	typedef Composite<Policy> ThisType;
	
	typedef ThisType* ObjectType;
	
	// TODO Maybe move this to a separate class?
	// This brings these classes into the namespace of classes that inherit from Composite
	// Note; only child windows should be here...
	typedef dwt::Button Button;
	typedef dwt::ButtonPtr ButtonPtr;
	typedef dwt::CheckBox CheckBox;
	typedef dwt::CheckBoxPtr CheckBoxPtr;
	typedef dwt::ComboBox ComboBox;
	typedef dwt::ComboBoxPtr ComboBoxPtr;
	typedef dwt::Container Container;
	typedef dwt::ContainerPtr ContainerPtr;
	typedef dwt::CoolBar CoolBar;
	typedef dwt::CoolBarPtr CoolBarPtr;
	typedef dwt::DateTime DateTime;
	typedef dwt::DateTimePtr DateTimePtr;
	typedef dwt::GroupBox GroupBox;
	typedef dwt::GroupBoxPtr GroupBoxPtr;
	typedef dwt::Label Label;
	typedef dwt::LabelPtr LabelPtr;
	typedef dwt::ProgressBar ProgressBar;
	typedef dwt::ProgressBarPtr ProgressBarPtr;
	typedef dwt::RadioButton RadioButton;
	typedef dwt::RadioButtonPtr RadioButtonPtr;
	typedef dwt::Slider Slider;
	typedef dwt::SliderPtr SliderPtr;
	typedef dwt::Spinner Spinner;
	typedef dwt::SpinnerPtr SpinnerPtr;
	typedef dwt::StatusBar StatusBar;
	typedef dwt::StatusBarPtr StatusBarPtr;
	typedef dwt::Table Table;
	typedef dwt::TablePtr TablePtr;
	typedef dwt::TabView TabView;
	typedef dwt::TabViewPtr TabViewPtr;
	typedef dwt::TextBox TextBox;
	typedef dwt::TextBoxPtr TextBoxPtr;
	typedef dwt::ToolBar ToolBar;
	typedef dwt::ToolBarPtr ToolBarPtr;
	typedef dwt::ToolTip ToolTip;
	typedef dwt::ToolTipPtr ToolTipPtr;
	typedef dwt::Tree Tree;
	typedef dwt::TreePtr TreePtr;
	
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
		Seed(const tstring& caption, DWORD style, DWORD exStyle);
	};

	template<typename SeedType>
	typename SeedType::WidgetType::ObjectType addChild(const SeedType& seed) {
		return WidgetCreator<typename SeedType::WidgetType>::create(this, seed);
	}

	void create(const Seed& cs);
protected:
	friend class WidgetCreator<Composite<Policy> >;
	
	explicit Composite( Widget * parent ) : BaseType( parent ) 
	{};

private:
	boost::scoped_ptr<WindowClass> windowClass;
};

template<typename Policy>
Composite<Policy>::Seed::Seed(const tstring& caption, DWORD style, DWORD exStyle) : 
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
