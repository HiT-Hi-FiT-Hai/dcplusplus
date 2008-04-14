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

#ifndef DWT_ToolTip_H_
#define DWT_ToolTip_H_

#include "../Policies.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectVisible.h"

namespace dwt {

class ToolTip :
	public MessageMap< Policies::Subclassed >,
	
	// Aspects
	public AspectEnabled< ToolTip >,
	public AspectFont< ToolTip >,
	public AspectRaw< ToolTip >,
	public AspectVisible< ToolTip >
{
	typedef MessageMap< Policies::Subclassed > BaseType;
	friend class WidgetCreator< ToolTip >;

	struct Dispatcher
	{
		typedef std::tr1::function<const tstring& ()> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			LPNMTTDISPINFO ttdi = reinterpret_cast< LPNMTTDISPINFO >( msg.lParam );
			ttdi->lpszText = const_cast<LPTSTR>(f().c_str());
			return 0;
		}

		F f;
	};
	
public:
	/// Class type
	typedef ToolTip ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		/// Fills with default parameters
		Seed();
	};

	void relayEvent(const MSG& msg);
	
	void setTool(Widget* widget, const Dispatcher::F& callback);
	
	void setMaxTipWidth(int width);

	/// Actually creates the Toolbar
	/** You should call WidgetFactory::createToolbar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	void create( const Seed & cs = Seed() );

protected:
	// Constructor Taking pointer to parent
	explicit ToolTip( Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~ToolTip()
	{}
};

inline ToolTip::ToolTip( Widget * parent )
	: BaseType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a ToolTip without a parent..." ) );
}

inline void ToolTip::setMaxTipWidth(int width) {
	sendMessage(TTM_SETMAXTIPWIDTH, 0, static_cast<LPARAM>(width));
}

}

#endif /*ToolTip_H_*/
