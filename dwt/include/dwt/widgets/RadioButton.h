/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
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

#ifndef DWT_RadioButton_h
#define DWT_RadioButton_h

#include "Button.h"

namespace dwt {

/// Button Control class
 /** \ingroup WidgetControls
   * \WidgetUsageInfo
   * \image html radiogroup.PNG
   * Class for creating a Radio Button Control. <br>
   * A Radio Button is a Widget which can be grouped together with other Radio Button
   * controls and only ONE of them can be "selected" at a time, it can in addition
   * contain descriptive text. <br>
   * By selecting one of the Radio Buttons grouped together you will also deselect the
   * previously selected one.
   */
class RadioButton :
	public Button
{
	typedef Button BaseType;
	friend class WidgetCreator< RadioButton >;
public:
	/// Class type
	typedef RadioButton ThisType;

	/// Object type
	typedef ThisType* ObjectType;
	
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		/// Fills with default parameters
		Seed(const tstring& caption_ = tstring());
	};

	/// Returns true if the RadioButton is selected
	 /** Call this function to determine if the RadioButton is selected or not,
	   * returns true if it is selected
	   */
	bool getChecked();

	/// Sets the checked value of the RadioButton
	 /** Call this function to either check the RadioButton or to uncheck the
	   * RadioButton
	   */
	void setChecked( bool value = true );

	/// Actually creates the Button Control
	/** You should call WidgetFactory::createRadioButton if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	void create( const Seed & cs = Seed() );

protected:
	// Constructor Taking pointer to parent
	explicit RadioButton( Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~RadioButton()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline RadioButton::RadioButton( Widget * parent )
	: BaseType( parent )
{
}

inline bool RadioButton::getChecked()
{
	return this->sendMessage(BM_GETCHECK) == BST_CHECKED;
}

inline void RadioButton::setChecked( bool value )
{
	this->sendMessage(BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED);
}

}

#endif
