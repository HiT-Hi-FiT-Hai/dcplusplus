#ifndef ASPECTTIMER_H_
#define ASPECTTIMER_H_

namespace SmartWin {

template< class WidgetType >
class AspectTimer {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	HWND H() { return W().handle(); }

	struct Dispatcher {
		typedef std::tr1::function<bool ()> F;
		
		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			if(!f()) {
				/// @todo remove from message map as well...
				::KillTimer(msg.hwnd, msg.wParam);
			}
			return FALSE;
		}

		F f;
	};


public:
	/// Creates a timer object.
	/** The supplied function must have the signature bool foo() <br>
	  * The event function will be called when at least milliSeconds seconds have elapsed.
	  * If your event handler returns true, it will keep getting called periodically, otherwise 
	  * it will be removed.
	  */
	void createTimer(const typename Dispatcher::F& f, unsigned int milliSeconds, unsigned int id = 0);
	
};

template< class WidgetType >
void AspectTimer< WidgetType >::createTimer( const typename Dispatcher::F& f,
	unsigned int milliSecond, unsigned int id)
{
	::SetTimer( H(), id, static_cast< UINT >( milliSecond ), NULL);
	W().addCallback(Message( WM_TIMER, id ), Dispatcher(f));
}

}

#endif /*ASPECTTIMER_H_*/
