// $Revision: 1.18 $
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
#ifndef WidgetMDIParent_h
#define WidgetMDIParent_h

#include "../BasicTypes.h"
#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/** sideeffect= \par Side Effects :
  */
/// MDI Parent Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html mdi.PNG
  * Class for creating a MDI Parent Widget. <br>
  * An MDI Parent is a Widget which can contain several "inner Widgets" which must be 
  * of type WidgetMDIChild, think of it as a special "container" Widget for only 
  * WidgetMDIChild widgets. <br>
  * Note! <br>
  * You can either inherit your own WidgetMDIChild Widgets or use the factory method 
  * createMDIChild to create MDI Children Widgets. <br>
  * Related class : <br>
  * WidgetMDIChild 
  */
class WidgetMDIParent :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspects
	public AspectSizable< WidgetMDIParent >,
	public AspectVisible< WidgetMDIParent >,
	public AspectEnabled< WidgetMDIParent >,
	public AspectFocus< WidgetMDIParent >,
	public AspectRaw< WidgetMDIParent >
{
	friend class WidgetCreator< WidgetMDIParent >;
public:
	/// Class type
	typedef WidgetMDIParent ThisType;

	/// Object type
	typedef ThisType * ObjectType;

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
		typedef WidgetMDIParent::ThisType WidgetType;

		// TODO: put variables to be filled here

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
		
		/** 
		 * First child id for mdi menu, must be different from any other main menu id. 
		 * Also, the menuHandle parameter of cs should point to the menu that will receive 
		 * 
		 **/
		UINT idFirstChild;
		
		HMENU windowMenu;
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	/// Actually creates the MDI EventHandlerClass Control
	/** You should call WidgetFactory::createMDIParent if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	
	void cascade() {
		this->sendMessage(WM_MDICASCADE);
	}
	
	void tile(bool horizontal) {
		this->sendMessage(WM_MDITILE, horizontal ? MDITILE_HORIZONTAL : MDITILE_VERTICAL);
	}
	
	void arrange() {
		this->sendMessage(WM_MDIICONARRANGE);
	}
	
	HWND getActive() {
		return reinterpret_cast<HWND>(this->sendMessage(WM_MDIGETACTIVE));
	}
	
	bool isActiveMaximized() {
		BOOL max = FALSE;
		this->sendMessage(WM_MDIGETACTIVE, 0, reinterpret_cast<LPARAM>(&max));
		return (max > 0);
	}
	
	void setActive(SmartWin::Widget* widget) {
		// TODO check that this is an instance of mdichild
		this->sendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(widget->handle()));
	}
	
	void next() {
		this->sendMessage(WM_MDINEXT);
	}
protected:
	/// Constructor Taking pointer to parent
	explicit WidgetMDIParent( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use WidgetFactory class which is friend
	virtual ~WidgetMDIParent()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetMDIParent::Seed::Seed()
{
	* this = WidgetMDIParent::getDefaultSeed();
}

inline WidgetMDIParent::WidgetMDIParent( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

// end namespace SmartWin
}

#endif
