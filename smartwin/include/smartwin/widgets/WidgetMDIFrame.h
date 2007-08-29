// $Revision: 1.30 $
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
#ifndef WidgetMDIFrame_h
#define WidgetMDIFrame_h

#include "../BasicTypes.h"
#include "../WindowClass.h"
#include "WidgetWindowBase.h"
#include <boost/scoped_ptr.hpp>

namespace SmartWin
{
// begin namespace SmartWin

class WidgetMDIParent;

/// "MDI Frame" class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html window.PNG
  * Class for creating a "normal" window. <br>
  * A normal window is what most applications would use as the basic main structure, 
  * in it you can add other Widgets like for instance buttons, comboboxes etc. <br>
  * A WidgetMDIFrame is basically a container Widget for other Widgets to reside in. 
  * <br>
  * Class is a public superclass of WidgetMDIFrameBase and therefore can use all 
  * features of WidgetMDIFrameBase.   
  */
class WidgetMDIFrame
	: public WidgetWindowBase< Policies::MDIFrame<WidgetMDIFrame > >
{
	typedef WidgetWindowBase< Policies::MDIFrame<WidgetMDIFrame > > BaseType;

public:
	/// Class type
	typedef WidgetMDIFrame ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetMDIFrame::ThisType WidgetType;

		//TODO: put variables to be filled here
		IconPtr icon;
		IconPtr iconSmall;
		HBRUSH background;
		LPCTSTR menuName;
		HCURSOR cursor;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	/// Actually creates the window
	/** This one creates the window. <br>
	  * All WidgetMDIFrames, and classes derived from them must create the Window
	  * before using it with functions such as setBounds() or setVisible( false ). <br>
      * The simple version "createWindow()" uses a default Seed for the window attributes.
	  * The seed is not taken a constant because the class name will be generated at registration.
	  */
	virtual void createWindow( Seed = getDefaultSeed() );

	/// Creates an invisible window, for quiet initialization.
	/** Same as createWindow, except that the window lacks WS_VISIBLE.
	  * Since you must create the window before you add other Widgets,
	  * and doing so causes a bit of screen flash before the final window
	  * is ready, createInvisibleWindow() lets you add Widgets while
	  * the main Widget is not visible.  Of course you could do code like <br>
	  *
	  *   Seed defInvisible = getDefaultSeed(); <br>
	  *   defInvisible.style= defInvisible.style & ( ~ WS_VISIBLE ); <br>
	  *   createWindow( defInvisible ); <br>
	  *
	  * but this is cleaner: <br>
	  *
	  *   createInvisibleWindow(); <br>
	  *   do init <br>
	  *   setVisible( true ); <br>
	  *
	  * The other styles are either defaulted with createInvisibleWindow()
	  * or specified with createInvisibleWindow( Seed ).
	  */
	virtual void createInvisibleWindow( Seed = getDefaultSeed() );

	WidgetMDIParent* getMDIParent() { return mdi; }
protected:
	// Protected since this Widget we HAVE to inherit from
	explicit WidgetMDIFrame( Widget * parent = 0 );

	virtual ~WidgetMDIFrame();
private:
	boost::scoped_ptr<WindowClass> windowClass;
	WidgetMDIParent* mdi;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetMDIFrame::Seed::Seed()
{
	* this = WidgetMDIFrame::getDefaultSeed();
}

inline WidgetMDIFrame::WidgetMDIFrame( Widget * parent )
	: BaseType( parent ), mdi(0)
{}

inline WidgetMDIFrame::~WidgetMDIFrame()
{
}

// end namespace SmartWin
}

#endif
