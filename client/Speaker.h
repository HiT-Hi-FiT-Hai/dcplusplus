/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SPEAKER_H
#define SPEAKER_H

#include "CriticalSection.h"

template<typename Listener>
class Speaker {
	typedef vector<Listener*> ListenerList;
	typedef typename ListenerList::iterator ListenerIter;

public:

	void fire(typename Listener::Types type) throw() {
		Lock l(listenerCS);
		ListenerList tmp = listeners;
		for(ListenerIter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type);
		}
	};

	template<class T> 
		void fire(typename Listener::Types type, const T& param) throw () {
			Lock l(listenerCS);
			ListenerList tmp = listeners;
			for(ListenerIter i=tmp.begin(); i != tmp.end(); ++i) {
				(*i)->onAction(type, param);
			}
		};

	template<class T, class T2> 
		void fire(typename Listener::Types type, const T& p, const T2& p2) throw() {
			Lock l(listenerCS);
			ListenerList tmp = listeners;
			for(ListenerIter i=tmp.begin(); i != tmp.end(); ++i) {
				(*i)->onAction(type, p, p2);
			}
		};
	template<class T, class T2, class T3> 
		void fire(typename Listener::Types type, const T& p, const T2& p2, const T3& p3) throw() {
			Lock l(listenerCS);
			ListenerList tmp = listeners;
			for(ListenerIter i=tmp.begin(); i != tmp.end(); ++i) {
				(*i)->onAction(type, p, p2, p3);
			}
		};
	template<class T, class T2, class T3, class T4> 
		void fire(typename Listener::Types type, const T& p, const T2& p2, const T3& p3, const T4& p4) throw() {
			Lock l(listenerCS);
			ListenerList tmp = listeners;
			for(ListenerIter i=tmp.begin(); i != tmp.end(); ++i) {
				(*i)->onAction(type, p, p2, p3, p4);
			}
		};
	template<class T, class T2, class T3, class T4, class T5> 
		void fire(typename Listener::Types type, const T& p, const T2& p2, const T3& p3, const T4& p4, const T5& p5) throw() {
			Lock l(listenerCS);
			ListenerList tmp = listeners;
			for(ListenerIter i=tmp.begin(); i != tmp.end(); ++i) {
				(*i)->onAction(type, p, p2, p3, p4, p5);
			}
		};
	template<class T, class T2, class T3, class T4, class T5, class T6> 
		void fire(typename Listener::Types type, const T& p, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6) throw() {
			Lock l(listenerCS);
			ListenerList tmp = listeners;
			for(ListenerIter i=tmp.begin(); i != tmp.end(); ++i) {
				(*i)->onAction(type, p, p2, p3, p4, p5, p6);
			}
		};


	void addListener(Listener* aListener) {
		Lock l(listenerCS);
		if(find(listeners.begin(), listeners.end(), aListener) == listeners.end())
			listeners.push_back(aListener);
	}

	void removeListener(Listener* aListener) {
		Lock l(listenerCS);

		ListenerIter i = find(listeners.begin(), listeners.end(), aListener);
		if(i != listeners.end())
			listeners.erase(i);
	}

	void removeListeners() {
		Lock l(listenerCS);
		listeners.clear();
	}
protected:
	ListenerList listeners;
	CriticalSection listenerCS;
};

#endif // SPEAKER_H
/**
 * @file
 * $Id: Speaker.h,v 1.1 2003/04/16 17:06:19 arnetheduck Exp $
 */
