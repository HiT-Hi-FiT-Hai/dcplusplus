/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#ifndef FINISHEDMANAGER_H
#define FINISHEDMANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "CriticalSection.h"
#include "Exception.h"
#include "DownloadManager.h"

class FinishedItem
{
public:
	typedef FinishedItem* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	FinishedItem(string const& aTarget, string const& aUser, int64_t aSize,
		int64_t aSpeed, string const& aTime)
		: target(aTarget), user(aUser), size(aSize), avgSpeed(aSpeed), time(aTime)
	{
	}

	GETSET(string, target, Target);
	GETSET(string, user, User);
	GETSET(int64_t, size, Size);
	GETSET(int64_t, avgSpeed, AvgSpeed);
	GETSET(string, time, Time);

private:
	friend class FinishedManager;
};

class FinishedManagerListener {
public:
	typedef FinishedManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum Types {
		ADDED,
		REMOVED,
		MAJOR_CHANGES
	};

	virtual void onAction(Types, FinishedItem*) { }
}; 

class FinishedManager : public Singleton<FinishedManager>,
	public Speaker<FinishedManagerListener>, private DownloadManagerListener
{
public:
	FinishedItem::List& lockList() { cs.enter(); return list; };
	void unlockList() { cs.leave(); };

	void remove(FinishedItem *item)
	{
		{
			Lock l(cs);
			FinishedItem::Iter it = find(list.begin(), list.end(), item);

			if(it != list.end())
				list.erase(it);
			else
				return;
		}

		fire(FinishedManagerListener::REMOVED, item);
		delete item;		
	}

	void removeAll()
	{
		{
			Lock l(cs);
			FinishedItem::Iter it = list.begin();
			for(; it != list.end(); it++)
				delete *it;
			list.clear();
		}

		fire(FinishedManagerListener::MAJOR_CHANGES, (FinishedItem *)0);
	}

private:
	friend class Singleton<FinishedManager>;
	
	FinishedManager() { DownloadManager::getInstance()->addListener(this); }
	virtual ~FinishedManager();

	virtual void onAction(DownloadManagerListener::Types type, Download* d);

	CriticalSection cs;
	FinishedItem::List list;
};

#endif // FINISHEDMANAGER_H

/**
 * @file FinishedManager.h
 * $Id: FinishedManager.h,v 1.1 2002/06/13 17:50:38 arnetheduck Exp $
 */
