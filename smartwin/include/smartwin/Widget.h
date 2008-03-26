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
#ifndef Widget_h
#define Widget_h

#include "Atom.h"
#include "Rectangle.h"
#include "Message.h"
#include "../../SmartUtil/tstring.h"

#include <boost/noncopyable.hpp>
#include <memory>
#include <list>
#include <functional>
#include <tr1/unordered_map>

namespace SmartWin
{
// begin namespace SmartWin

using namespace std::tr1::placeholders;

class Application;
class Widget;

template<typename T>
T hwnd_cast(HWND hwnd);

/// Abstract Base class for all Widgets
/** Basically (almost) all Widgets derive from this class, this is the root class for
  * (almost) every single Widget type. <br>
  * This class contains the functionality that all Widgets must or should have
  * support for. <br>
  * E.g. the handle to the specific Widgets are contained here, plus all the
  * commonalities between Widgets <br>
  * The Widget class inherits from boost::noncopyable to indicate it's not to be
  * copied
  */
class Widget
	: public boost::noncopyable
{
public:
	/** Most Widgets can override the creational parameters which sets the style and the
	  * initial position of the Widget, those Widgets will take an object of this type to
	  * their creational function(s).
	  */
	struct Seed {
		LPCTSTR className;

		/// Initial caption
		/** Windows with a title bar will use this string in the title bar. Controls with
		  * caption (e.g. static control, edit control) will use it in the control. <br>
		  * It is feed directly to CreateWindowEx, this means that it follows its
		  * conventions. In particular, the string "#num" has a special meaning.
		  */
		SmartUtil::tstring caption;

		/// The style of the object (starts with WS_ or BS_ etc...)
		/** WARNING: The creation of most of the controls require WS_CHILD to be set.
		  * This is done, by default, in the appropriate controls. If you override the
		  * default style, then be sure that WS_CHILD is set (if needed).
		  */
		DWORD style;

		/// The Extended Style of the object (starts often with WS_EX_ etc)
		DWORD exStyle;

		/// The initial position / size of the Widget
		Rectangle location;

		HMENU menuHandle;

		/// Constructor initializing all member variables to default values
		Seed(LPCTSTR className_, DWORD style_ = WS_VISIBLE, DWORD exStyle_ = 0, 
			const SmartUtil::tstring& caption_ = SmartUtil::tstring(), 
			const Rectangle& location_ = letTheSystemDecide, HMENU menuHandle_ = NULL)
			: className(className_), caption(caption_), style( style_ ), exStyle( exStyle_ ), location( location_ ), menuHandle( menuHandle_ )
		{}

	};
	
	/// Returns the HWND to the Widget
	/** Returns the HWND to the inner window of the Widget. <br>
	  * If you need to do directly manipulation of the window use this function to
	  * retrieve the HWND of the Widget.
	  */
	HWND handle() const;

	/// Send a message to the Widget
	/** If you need to be able to send a message to a Widget then use this function
	  * as it will unroll into <br>
	  * a ::SendMessage from the Windows API
	  */
	LRESULT sendMessage( UINT msg, WPARAM wParam = 0, LPARAM lParam = 0) const;
	
	bool postMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0) const;

	/// Returns the parent Widget of the Widget
	/** Most Widgets have got a parent, this function will retrieve a pointer to the
	  * Widgets parent, if the Widget doesn't have a parent it will return a null
	  * pointer.
	  */
	Widget* getParent() const;

	/// Repaints the whole window
	/** Invalidate the window and repaints it.
	  */
	void updateWidget();

	/// Add this widget to the update area.
	/** Same as updateWidget except that this does not force an immediate redraw.
	  */
	void invalidateWidget();

	/// Use this function to add or remove windows styles.
	/** The first parameter is the type of style you wish to add/remove. <br>
	  * The second argument is a boolean indicating if you wish to add or remove the
	  * style (if true add style, else remove)
	  */
	void addRemoveStyle( DWORD addStyle, bool add );
	
	bool hasStyle(DWORD style);

	/// Use this function to add or remove windows exStyles.
	/** The first parameter is the type of style you wish to add/remove. <br>
	  * The second argument is a boolean indicating if you wish to add or remove the
	  * style (if true add style, else remove)
	  */
	void addRemoveExStyle( DWORD addStyle, bool add );
	
	typedef std::tr1::function<bool(const MSG& msg, LRESULT& ret)> CallbackType;
	typedef std::list<CallbackType> CallbackList;
	typedef std::tr1::unordered_map<Message, CallbackList> CallbackCollectionType;
	
	/// Adds a new callback - multiple callbacks for the same message will be called in the order they were added
	void addCallback(const Message& msg, const CallbackType& callback );

	/// Sets the callback for msg - clears any other callbacks registered for the same message
	void setCallback(const Message& msg, const CallbackType& callback );
	
	CallbackCollectionType & getCallbacks();

	/// Returns true if fired, else false
	virtual bool tryFire( const MSG & msg, LRESULT & retVal );
		
	/** This will be called when it's time to delete the widget */
	virtual void kill();

	/// Subclasses the dialog item with the given dialog item id
	/** Subclasses a dialog item, the id is the dialog item id from the resource
	  * editor. <br>
	  * Should normally not be called directly but rather called from e.g. one of the
	  * creational functions found in the WidgetFactory class.
	  */
	void attach( unsigned id );

	virtual void attach(HWND wnd);

protected:
	Widget(Widget * parent);

	virtual ~Widget();

	// Creates the Widget, should NOT be called directly but overridden in the
	// derived class (with no parameters)
	virtual HWND create( const Seed & cs );

private:
	friend class Application;
	template<typename T> friend T hwnd_cast(HWND hwnd);
	
	// Contains the list of signals we're (this window) processing
	CallbackCollectionType itsCallbacks;

	Widget * itsParent;
	HWND itsHandle;

	/// The atom with which the pointer to the MessageMapBase is registered on the HWND
	static GlobalAtom propAtom;
};

inline Widget::Widget( Widget * parent ) : itsParent(parent), itsHandle(NULL) {
	
}

inline LRESULT Widget::sendMessage( UINT msg, WPARAM wParam, LPARAM lParam) const {
	return ::SendMessage(handle(), msg, wParam, lParam);
}

inline bool Widget::postMessage(UINT msg, WPARAM wParam, LPARAM lParam) const {
	return ::PostMessage(handle(), msg, wParam, lParam);
}

inline HWND Widget::handle() const { 
	return itsHandle;
}

inline Widget* Widget::getParent() const { 
	return itsParent; 
}

inline bool Widget::hasStyle(DWORD style) {
	return (::GetWindowLong(this->handle(), GWL_STYLE) & style) == style;	
}

inline Widget::CallbackCollectionType& Widget::getCallbacks() { 
	return itsCallbacks;
}

template<typename T>
T hwnd_cast(HWND hwnd) {
	Widget* w = reinterpret_cast<Widget*>(::GetProp(hwnd, Widget::propAtom));
	return dynamic_cast<T>(w);
}

// end namespace SmartWin
}

#endif
