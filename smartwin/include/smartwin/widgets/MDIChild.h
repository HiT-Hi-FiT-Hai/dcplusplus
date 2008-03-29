/*
  Copyright (c) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

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
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MDIChild_h
#define MDIChild_h

#ifndef WINCE

#include "../../SmartUtil.h"
#include "../Rectangle.h"
#include "../resources/Icon.h"
#include "../Policies.h"
#include "../WindowClass.h"
#include "MDIParent.h"
#include "WidgetWindowBase.h"
#include <sstream>
#include <boost/scoped_ptr.hpp>

namespace SmartWin
{
// begin namespace SmartWin

/** sideeffect=\par Side Effects:
  */
/// MDI Child Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html mdi.PNG
  * Class for creating a MDI Child Widget. <br>
  * An MDI Child is a Widget which is kind of like a special case of WidgetWindow, it 
  * exists only for two purposes which is 1. Contained in the MDIParent class 
  * and 2. To serve as a container widget for your control widgets. <br>
  * Use either the MDIParent::createMDIChild or inherit from this class and roll 
  * your own logic. <br>
  * Related classes: <br>
  * MDIParent 
  */
class MDIChild
	: public WidgetWindowBase< Policies::MDIChild >
{
public:
	typedef WidgetWindowBase<Policies::MDIChild> BaseType;
	
	/// Class type
	typedef MDIChild ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public Widget::Seed
	{
	public:
		typedef MDIChild::ThisType WidgetType;

		IconPtr smallIcon;
		IconPtr icon;
		HBRUSH background;
		bool activate;
		
		/// Fills with default parameters
		Seed(const SmartUtil::tstring& caption = SmartUtil::tstring());

	};

	/// Creates a MDIChild Window
	/** This version creates a MessageMapPolicyMDIChildWidget to plug into MDIParent
	  * container window.
	  */
	void createMDIChild( Seed cs = Seed() );
	
	virtual bool tryFire(const MSG& msg, LRESULT& retVal);
	
	void activate();

	MDIParent* getParent() { return static_cast<MDIParent*>(PolicyType::getParent()); }
protected:
	// Protected since this Widget we HAVE to inherit from
	explicit MDIChild( Widget * parent );

	virtual ~MDIChild();

private:
	boost::scoped_ptr<WindowClass> windowClass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline MDIChild::~MDIChild()
{
}

inline MDIChild::MDIChild( Widget * parent )
	: BaseType( parent )
{}

// end namespace SmartWin
}

#endif //! WINCE

#endif
