#ifndef ASPECTSPEAKER_H_
#define ASPECTSPEAKER_H_

#include "StupidWin.h"

template<typename T>
class AspectSpeaker {
public:
	AspectSpeaker() {
	}
	
	template<typename F>
	void onSpeaker(F t) {
		static_cast<T*>(this)->onRaw(t, WM_SPEAKER);
	}
	
	BOOL speak(LPARAM l = 0, WPARAM w = 0) { return StupidWin::postMessage(static_cast<T*>(this), WM_SPEAKER, w, l); }
	
private:
};

#endif /*ASPECTSPEAKER_H_*/
