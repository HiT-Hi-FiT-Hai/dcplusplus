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
#ifndef AspectText_h
#define AspectText_h

#include "../../SmartUtil.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Aspect class used by Widgets that have the possibility of setting the "text"
/// property of their objects.
/** \ingroup AspectClasses
  * E.g. the AspectTextBox have a "text" Aspect therefore they realize the AspectText
  * through inheritance.
  */
template< class WidgetType >
class AspectText
{
	struct Dispatcher
	{
		typedef std::tr1::function<void (const SmartUtil::tstring &)> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			f(SmartUtil::tstring( reinterpret_cast< TCHAR * >( msg.lParam ) ));
			return false;
		}

		F f;
	};
public:
	/// Sets the text of the AspectText realizing class
	/** The txt parameter is the new text to put into the realizing object.
	  */
	void setText( const SmartUtil::tstring & txt );

	/// Sets the text in the Edit Control so that endl causes a new line.
	/** Just the same as setText except that CR are expanded to LF CR
	  * Replaces \n with \r\n so that Windows textbox understands "endl"
	  */
	void setTextLines( const SmartUtil::tstring & txt );

	/// Returns a string in which \n is replaced with with \r\n
	/** The purpose is to enable Windows textboxs to understand "endl"
	  */
	static SmartUtil::tstring replaceEndlWithLfCr( const SmartUtil::tstring & txt );

	/// Gets the text of the AspectText realizing class
	/** The Return value is the text of the realizing class.
	  */
	SmartUtil::tstring getText() const;
	
	/// Length of text in characters
	size_t length() const;

	/// \ingroup EventHandlersAspectText
	/// Setting the event handler for the "setText" event
	/** When the text changes in the Widget this event will be raised. <br>
	  * The parameter passed is SmartUtil::tstring & which is the new text of the
	  * Widget.
	  */
	void onTextChanging(const typename Dispatcher::F& f) {
		static_cast<WidgetType*>(this)->setCallback(
			Message( WM_SETTEXT ), Dispatcher(f)
		);
	}

protected:
	virtual ~AspectText()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class WidgetType >
void AspectText< WidgetType >::setText( const SmartUtil::tstring & txt )
{
	static_cast< WidgetType * >( this )->sendMessage(WM_SETTEXT, 0, reinterpret_cast< LPARAM >(txt.c_str()) );
}


template< class WidgetType >
SmartUtil::tstring AspectText< WidgetType >::replaceEndlWithLfCr( const SmartUtil::tstring & txt )
{
	// Replaces \n with \r\n so that Windows textbox understands "endl"
	SmartUtil::tstring	txtEndl= txt;

	SmartUtil::tstring::size_type pos= txtEndl.find( _T('\n'), 0 );
	while ( SmartUtil::tstring::npos != pos ) {
		if(pos > 0 && txtEndl[pos-1] != _T('\r')) {
			txtEndl.replace( pos, 1, _T("\r\n") );
			pos += 2;	// Don't find the replacement \n.
		} else {
			pos++;
		}
		pos = txtEndl.find( _T('\n'), pos );
	}  
	return txtEndl;
}

template< class WidgetType >
void AspectText< WidgetType >::setTextLines( const SmartUtil::tstring & inTxt )
{
	setText( replaceEndlWithLfCr( inTxt ) );
}

template< class WidgetType >
size_t AspectText< WidgetType >::length( ) const {
	return static_cast<size_t>(static_cast<const WidgetType*>(this)->sendMessage(WM_GETTEXTLENGTH));
}

template< class WidgetType >
SmartUtil::tstring AspectText< WidgetType >::getText() const
{
	size_t textLength = length();
	if ( textLength == 0 )
		return _T( "" );
	SmartUtil::tstring retVal(textLength + 1, 0);
	retVal.resize(static_cast<const WidgetType*>(this)->sendMessage(WM_GETTEXT, static_cast<WPARAM>(textLength + 1), reinterpret_cast<LPARAM>(&retVal[0])));
	return retVal;
}

// end namespace SmartWin
}

#endif
