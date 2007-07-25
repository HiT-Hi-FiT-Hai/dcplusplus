// $Revision: 1.26 $
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

#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectBackgroundColor.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"

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
	public MessageMapPolicy< Policies::Subclassed >,
	
	// Aspect classes
	public AspectBackgroundColor< WidgetCheckBox >,
	public AspectBorder< WidgetCheckBox >,
	public AspectClickable< WidgetCheckBox >,
	public AspectDblClickable< WidgetCheckBox >,
	public AspectEnabled< WidgetCheckBox >,
	public AspectFocus< WidgetCheckBox >,
	public AspectFont< WidgetCheckBox >,
	public AspectPainting< WidgetCheckBox >,
	public AspectRaw< WidgetCheckBox >,
	public AspectSizable< WidgetCheckBox >,
	public AspectText< WidgetCheckBox >,
	public AspectThreads< WidgetCheckBox >,
	public AspectVisible< WidgetCheckBox >
{
	friend class WidgetCreator< WidgetCheckBox >;
public:
	/// Class type
	typedef WidgetCheckBox ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	/// Policy type
	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetCheckBox::ThisType WidgetType;

		FontPtr font;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getClickMessage();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getDblClickMessage();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getBackgroundColorMessage();

	/// Returns the checked state of the Check Box
	/** Return value is true if Check Box is checked, otherwise false.
	  */
	bool getChecked();

	/// Sets the checked state of the Check Box
	/** Call this one to programmaticially check a Check Box.
	  */
	void setChecked( bool value = true );

	/// Actually creates the Check Box Control
	/** You should call WidgetFactory::createCheckBox if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

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

inline WidgetCheckBox::Seed::Seed()
{
	* this = WidgetCheckBox::getDefaultSeed();
}


inline void WidgetCheckBox::setChecked( bool value )
{
	this->sendMessage(BM_SETCHECK, static_cast< WPARAM >( value ? BST_CHECKED : BST_UNCHECKED ) );
}


inline Message & WidgetCheckBox::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_CLICKED );
	return retVal;
}


inline Message & WidgetCheckBox::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_DBLCLK );
	return retVal;
}


inline Message & WidgetCheckBox::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORBTN );
	return retVal;
}


inline bool WidgetCheckBox::getChecked()
{
	return this->sendMessage(BM_GETCHECK) == BST_CHECKED;
}

// Protected to avoid direct instantiation, you can inherit and use WidgetFactory
// class which is friend

inline WidgetCheckBox::WidgetCheckBox( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Cant have a TextBox without a parent..." ) );
}

// end namespace SmartWin
}

#endif
