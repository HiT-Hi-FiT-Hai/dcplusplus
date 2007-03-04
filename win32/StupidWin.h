#ifndef STUPIDWIN_H_
#define STUPIDWIN_H_

/** @file
 * Stuff missing from SmartWin that should at some point be sent as a patch to them
 */
namespace StupidWin {
int getWindowTextLength(SmartWin::Widget* w) {
	return ::GetWindowTextLength(w->handle());	
}

int lineFromChar(SmartWin::Widget* w, int c = -1) {
	return ::SendMessage(w->handle(), EM_LINEFROMCHAR, static_cast<WPARAM>(c), 0);
}
int lineIndex(SmartWin::Widget* w, int l = -1) {
	return ::SendMessage(w->handle(), EM_LINEINDEX, static_cast<WPARAM>(l), 0);	
}
}

#endif /*STUPIDWIN_H_*/
