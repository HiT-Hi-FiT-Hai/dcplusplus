/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#if !defined(AFX_QUEUEMANAGERLISTENER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)
#define AFX_QUEUEMANAGERLISTENER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class QueueItem;

class QueueManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Added;
	typedef X<1> Finished;
	typedef X<2> Removed;
	typedef X<3> Moved;
	typedef X<4> SourcesUpdated;
	typedef X<5> StatusUpdated;
	typedef X<6> SearchStringUpdated;

	virtual void on(Added, QueueItem*) throw() { }
	virtual void on(Finished, QueueItem*) throw() { }
	virtual void on(Removed, QueueItem*) throw() { }
	virtual void on(Moved, QueueItem*) throw() { }
	virtual void on(SourcesUpdated, QueueItem*) throw() { }
	virtual void on(StatusUpdated, QueueItem*) throw() { }
	virtual void on(SearchStringUpdated, QueueItem*) throw() { }
};

#endif

/**
 * @file
 * $Id: QueueManagerListener.h,v 1.7 2004/09/06 12:32:42 arnetheduck Exp $
 */
