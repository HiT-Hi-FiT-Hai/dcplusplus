/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#if !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)
#define AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserConnection.h"
#include "ConnectionManager.h"
#include "ShareManager.h"

class UploadManager : public UserConnectionListener
{
public:
	virtual void onDisconnected(UserConnection* aSource) {
		removeConnection(aSource);
	}
	virtual void onError(UserConnection* aSource, const string& aError) {
		Upload * u;
		Upload::MapIter i = uploads.find(aSource);
		if(i == uploads.end()) {
			// Something strange happened?
			aSource->disconnect();
			aSource->removeListener(this);
			ConnectionManager::getInstance()->putUploadConnection(aSource);
		}
		u = i->second;
		uploads.erase(i);
		delete u;
	}

	virtual void onTransmitDone(UserConnection* aSource) {
		Upload * u;
		Upload::MapIter i = uploads.find(aSource);
		if(i == uploads.end()) {
			// Something strange happened?
			aSource->disconnect();
			aSource->removeListener(this);
			ConnectionManager::getInstance()->putUploadConnection(aSource);
		}
		u = i->second;
		uploads.erase(i);
		delete u;
	}

	virtual void onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume) {
		Upload* u;
		try {
			string file = ShareManager::getInstance()->translateFileName(aFile);
			Upload::MapIter i = uploads.find(aSource);
			if(i != uploads.end()) {
				delete i->second;
				uploads.erase(i);
			} else {
				u = new Upload();
			}

			u->file = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if(u->file == INVALID_HANDLE_VALUE) {
				delete u;
				aSource->error("File Not Available");
				return;
			}
			char buf[24];
			DWORD high = 0;
			long h = (int) (aResume >> 32);
			DWORD l = (DWORD) (aResume & 0xffffffff);
			SetFilePointer(u->file, l, &h, FILE_BEGIN);
			aSource->fileLength(_i64toa((LONGLONG)GetFileSize(u->file, &high) | ((LONGLONG)high) << 32, buf, 10));

			uploads[aSource] = u;

		} catch (ShareException e) {
			aSource->error("File Not Available");
		}
	}
	
	virtual void onSend(UserConnection* aSource) {
		Upload * u;
		Upload::MapIter i = uploads.find(aSource);
		if(i == uploads.end()) {
			// Something strange happened?
			aSource->disconnect();
			aSource->removeListener(this);
			ConnectionManager::getInstance()->putUploadConnection(aSource);
		}
		u = i->second;
		try {
			aSource->transmitFile(u->file);
		} catch(Exception e) {
			delete u;
			uploads.erase(i);
			aSource->disconnect();
			aSource->removeListener(this);
			ConnectionManager::getInstance()->putUploadConnection(aSource);
		}
	}
	
	virtual void onGetListLen(UserConnection* aSource) {
		aSource->listLen(ShareManager::getInstance()->getListLenString());
	}

	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		connections.push_back(conn);
	}

	void removeConnection(UserConnection::Ptr aConn) {
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			if(*i == aConn) {
				aConn->removeListener(this);
				ConnectionManager::getInstance()->putDownloadConnection(aConn);
				connections.erase(i);
			}
		}
	}
	void removeConnections() {
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			(*i)->removeListener(this);
			ConnectionManager::getInstance()->putDownloadConnection(*i);
			i = connections.erase(i);
		}
	}
	
	static UploadManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void newInstance() {
		if(instance)
			delete instance;

		instance = new UploadManager();
	}
	static void deleteInstance() {
		delete instance;
		instance = NULL;
	}
private:
	class Upload {
	public:
		typedef Upload* Ptr;
		typedef map<UserConnection::Ptr, Ptr> Map;
		typedef Map::iterator MapIter;
		
		UserConnection* user;
		HANDLE file;
		
		Upload() : file(NULL), user(NULL) { };
		~Upload() {
			if(file)
				CloseHandle(file);
		}
	};

	static UploadManager* instance;
	UserConnection::List connections;
	Upload::Map uploads;

	UploadManager() { };
	~UploadManager() {
		UserConnection::List tmp = connections;
		
		for(Upload::MapIter j = uploads.begin(); j != uploads.end(); ++j) {
			delete j->second;
		}

		for(UserConnection::Iter i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->removeListener(this);
			(*i)->disconnect();
			ConnectionManager::getInstance()->putUploadConnection(*i);
		}
	}
};

#endif // !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)

/**
 * @file UploadManger.h
 * $Id: UploadManager.h,v 1.6 2001/12/03 20:52:19 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.h,v $
 * Revision 1.6  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
