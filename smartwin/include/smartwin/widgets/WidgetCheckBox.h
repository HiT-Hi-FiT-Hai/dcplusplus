/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met :

	  * Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.
	  * Neither the name of the SmartWin++ nor the names of its contributors
		may be used to endorse or promote products derived from this software
		without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WidgetCheckBox_h
#define WidgetCheckBox_h

#include "../Widget.h"
#include "../aspects/AspectButton.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// Check Box Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html checkbox.PNG
  * Class for creating a checkbox control Widget. <br>
  * A checkbox is a Widget which can be pressed to "pop in" and pressed again to "pop 
  * out". <br>
  * It can contain descriptive text etc. 
  */
class WidgetCheckBox :
	// Aspect classes
	public AspectButton< WidgetCheckBox >
{
	friend class WidgetCreator< WidgetCheckBox >;
public:
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed : public Widget::Seed
	{
	public:
		FontPtr font;

		/// Fills with default parameters
		Seed(const SmartUtil::tstring& caption_ = SmartUtil::tstring());
	};

	/// Returns the checked state of the Check Box
	/** Return value is true if Check Box is checked, otherwise false.
	  */
	bool getChecked();

	/// Sets the checked state of the Check Box
	/** Call this one to programmaticially check a Check Box.
	  */
	void setChecked( bool value = true );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetCheckBox( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetCheckBox()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void WidgetCheckBox::setChecked( bool value )
{
	this->sendMessage(BM_SETCHECK, static_cast< WPARAM >( value ? BST_CHECKED : BST_UNCHECKED ) );
}

inline bool WidgetCheckBox::getChecked()
{
	return this->sendMessage(BM_GETCHECK) == BST_CHECKED;
}

inline WidgetCheckBox::WidgetCheckBox( SmartWin::Widget * parent )
	: ButtonType( parent )
{
}

// end namespace SmartWin
}

#endif
