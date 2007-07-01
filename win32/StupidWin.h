#ifndef STUPIDWIN_H_
#define STUPIDWIN_H_

/** @file
 * Stuff missing from SmartWin that should at some point be sent as a patch to them
 */
namespace StupidWin {
namespace { 
	LRESULT sm(SmartWin::Widget* w, UINT m, WPARAM wp = 0, LPARAM lp = 0) { return ::SendMessage(w->handle(), m, wp, lp); } 
}

inline int getWindowTextLength(SmartWin::Widget* w) { return ::GetWindowTextLength(w->handle()); }
inline int lineFromChar(SmartWin::Widget* w, int c = -1) { return sm(w, EM_LINEFROMCHAR, c); }
inline int lineIndex(SmartWin::Widget* w, int l = -1) { return sm(w, EM_LINEINDEX, l); }
inline BOOL isIconic(SmartWin::Widget* w) { return ::IsIconic(w->handle()); }
inline BOOL postMessage(SmartWin::Widget* w, UINT m, WPARAM wp = 0, LPARAM lp = 0) { return ::PostMessage(w->handle(), m, wp, lp); }
inline LRESULT sendMessage(SmartWin::Widget* w, UINT m, WPARAM wp = 0, LPARAM lp = 0) { return ::SendMessage(w->handle(), m, wp, lp); }
inline bool getModify(SmartWin::Widget* w) { return sm(w, EM_GETMODIFY) > 0; }
inline void setModify(SmartWin::Widget* w, bool modified) { sm(w, EM_SETMODIFY, modified); }
inline void emptyUndoBuffer(SmartWin::Widget* w) { sm(w, EM_EMPTYUNDOBUFFER); }
inline DWORD_PTR getTreeItemData(SmartWin::Widget* w, SmartWin::TreeViewNode tvn) {
	TVITEM item = { TVIF_PARAM };
	item.hItem = tvn.handle;
	return (DWORD_PTR)((BOOL)sm(w, TVM_GETITEM, 0, (LPARAM)&item) ? item.lParam : 0);
}
}

#endif /*STUPIDWIN_H_*/
