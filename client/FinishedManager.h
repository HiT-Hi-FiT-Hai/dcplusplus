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

	FinishedItem(string const& aTarget, string const& aUser, string const& aHub, 
		int64_t aSize, int64_t aChunkSize, int64_t aMSeconds, string const& aTime,
		bool aCrc32) : 
		target(aTarget), user(aUser), hub(aHub), size(aSize), chunkSize(aChunkSize),
		milliSeconds(aMSeconds), time(aTime), crc32Checked(aCrc32) {
	}

	int64_t getAvgSpeed() { return milliSeconds > 0 ? (chunkSize * ((int64_t)1000) / milliSeconds) : 0; };

	GETSET(string, target, Target);
	GETSET(string, user, User);
	GETSET(string, hub, Hub);
	GETSET(int64_t, size, Size);
	GETSET(int64_t, chunkSize, ChunkSize);
	GETSET(int64_t, milliSeconds, MilliSeconds);
	GETSET(string, time, Time);
	GETSET(bool, crc32Checked, Crc32Checked)
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

	virtual void onAction(Types, FinishedItem*) throw() = 0;
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
			for_each(list.begin(), list.end(), DeleteFunction<FinishedItem*>());
			list.clear();
		}

		fire(FinishedManagerListener::MAJOR_CHANGES, (FinishedItem *)0);
	}

private:
	friend class Singleton<FinishedManager>;
	
	FinishedManager() { DownloadManager::getInstance()->addListener(this); }
	virtual ~FinishedManager();

	virtual void onAction(DownloadManagerListener::Types type, Download* d) throw();

	CriticalSection cs;
	FinishedItem::List list;
};

#endif // FINISHEDMANAGER_H

/**
 * @file FinishedManager.h
 * $Id: FinishedManager.h,v 1.4 2003/03/13 13:31:21 arnetheduck Exp $
 */
