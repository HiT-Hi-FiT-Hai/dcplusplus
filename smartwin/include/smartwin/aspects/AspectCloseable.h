#ifndef ASPECTCLOSE_H_
#define ASPECTCLOSE_H_

#include "../Message.h"

namespace SmartWin {

template< class WidgetType >
class AspectCloseable {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }

	struct Dispatcher {
		typedef std::tr1::function<bool ()> F;
		
		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			return !f();
		}

		F f;
	};

public:
	/// Closes the window
	/** Call this function to raise the "Closing" event. <br>
	  * This will normally try to close the window. <br>
	  * Note! <br>
	  * If this event is trapped and we in that event handler state that we DON'T 
	  * want to close the window (by returning false) the window will not be close. 
	  * <br>
	  * Note! <br>
	  * If the asyncron argument is true the message will be posted to the message 
	  * que meaning that the close event will be done asyncronously and therefore the 
	  * function will return immediately and the close event will be handled when the 
	  * close event pops up in the event handler que.       
	  */
	void close( bool asyncron = false );
	
	/// Event Handler setter for the Closing Event
	/** If supplied event handler is called before the window is closed. <br>
	  * Signature of event handler must be "bool foo()" <br>
	  * If you return true from your event handler the window is closed, otherwise 
	  * the window is NOT allowed to actually close!!       
	  */
	void onClosing(const typename Dispatcher::F& f);
};

template< class WidgetType >
void AspectCloseable< WidgetType >::close( bool asyncron ) {
	if ( asyncron )
		W().postMessage(WM_CLOSE); // Return now
	else
		W().sendMessage(WM_CLOSE); // Return after close is done.
}

template<typename WidgetType>
void AspectCloseable<WidgetType>::onClosing(const typename Dispatcher::F& f) {
	W().addCallback(Message(WM_CLOSE), Dispatcher(f));
}

}

#endif /*ASPECTCLOSE_H_*/
