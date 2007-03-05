#ifndef STUPIDWIN_H_
#define STUPIDWIN_H_

/** @file
 * Stuff missing from SmartWin that should at some point be sent as a patch to them
 */
namespace StupidWin {
namespace { template<typename W, typename L> int sm(SmartWin::Widget* w, UINT m, W wp, L lp) { return ::SendMessage(w->handle(), m, static_cast<WPARAM>(wp), static_cast<LPARAM>(lp)); } }

int getWindowTextLength(SmartWin::Widget* w) { return ::GetWindowTextLength(w->handle()); }
int lineFromChar(SmartWin::Widget* w, int c = -1) { return sm(w, EM_LINEFROMCHAR, c, 0); }
int lineIndex(SmartWin::Widget* w, int l = -1) { return sm(w, EM_LINEINDEX, l, 0); }
void setRedraw(SmartWin::Widget* w, bool redraw) { sm(w, WM_SETREDRAW, redraw, 0); }
}

#endif /*STUPIDWIN_H_*/
