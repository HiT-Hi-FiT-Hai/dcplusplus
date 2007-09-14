// $Revision: 1.28 $
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
#ifndef WidgetButton_h
#define WidgetButton_h

#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectButton.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/** sideeffect = \par Side Effects :
  */
/// Button Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html button.PNG
  * Class for creating a button control Widget. <br>
  * A button is a Widget which can be pressed, it can contain descriptive text etc.
  */
class WidgetButton :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspects
	public AspectButton< WidgetButton >
{
	friend class WidgetCreator< WidgetButton >;
public:

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
		typedef WidgetButton::ThisType WidgetType;

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

	/// Actually creates the Button Control
	/** You should call WidgetFactory::createButton if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetButton( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetCreator class which is friend
	virtual ~WidgetButton()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetButton::Seed::Seed()
{
	* this = WidgetButton::getDefaultSeed();
}

inline WidgetButton::WidgetButton( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

// end namespace SmartWin
}

#endif
