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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "DirectoryListing.h"
#include "StringTokenizer.h"
#include "ADLSearch.h"
#include "QueueManager.h"
#include "SearchManager.h"

void DirectoryListing::load(const string& in) 
{
	StringTokenizer t(in);

	StringList& tokens = t.getTokens();
	string::size_type ident = 0;

	// Prepare ADLSearch manager
	ADLSearchManager* pADLSearch = ADLSearchManager::getInstance();
	StringMap params;
	params["nick"] = getUser()->getNick();
	pADLSearch->PrepareDestinationDirectories(root, params);
	pADLSearch->setBreakOnFirst(BOOLSETTING(ADLS_BREAK_ON_FIRST));

	Directory* cur = root;
	string fullPath;
	
	File::Iter lastFileIter = cur->files.begin();

	for(StringIter i = tokens.begin(); i != tokens.end(); ++i) 
	{
		string& tok = *i;
		string::size_type j = tok.find_first_not_of('\t');
		if(j == string::npos)
		{
			break;
		}

		while(j < ident) 
		{
			// Wind up directory structure
			cur = cur->getParent();
			dcassert(cur != NULL);
			ident--;
			string::size_type l = fullPath.find_last_of('\\');
			if(l != string::npos)
			{
				fullPath.erase(fullPath.begin() + l, fullPath.end());
			}
			pADLSearch->StepUpDirectory();

			lastFileIter = cur->files.begin();
		}

		string::size_type k = tok.find('|', j);
		if(k != string::npos) {
			// this must be a file...
			lastFileIter = cur->files.insert(lastFileIter, new File(cur, tok.substr(j, k-j), Util::toInt64(tok.substr(k+1))));

			// ADLSearch
			pADLSearch->MatchesFile(*lastFileIter, fullPath);
		} else {
			// A directory
			string name = tok.substr(j, tok.length()-j-1);
			Directory* d = new Directory(cur, tok.substr(j, tok.length()-j-1));
			Directory::Iter di = cur->directories.find(d);
			if(di != cur->directories.end()) {
				delete d;
				d = *di;
			}
			cur->directories.insert(d);
			cur = d;
			lastFileIter = cur->files.begin();

			ident++;
			fullPath += (string)"\\" + d->getName();

			// ADLSearch
			pADLSearch->MatchesDirectory(d, fullPath);
		}
	}

	// Finalize ADLSearch manager
	pADLSearch->FinalizeDestinationDirectories(root);
}

string DirectoryListing::getPath(Directory* d) {
	string dir;
	dir.reserve(128);
	dir.append(d->getName());
	dir.append(1, '\\');

	Directory* cur = d->getParent();
	while(cur!=root) {
		dir.insert(0, cur->getName() + '\\');
		cur = cur->getParent();
	}
	return dir;
}

void DirectoryListing::download(Directory* aDir, const string& aTarget) {
	string target = (aDir == getRoot()) ? aTarget : aTarget + aDir->getName() + '\\';
	// First, recurse over the directories
	for(Directory::Iter j = aDir->directories.begin(); j != aDir->directories.end(); ++j) {
		download(*j, target);
	}
	// Then add the files
	for(File::Iter i = aDir->files.begin(); i != aDir->files.end(); ++i) {
		File* file = *i;
		try {
			download(file, target + file->getName());
		} catch(const QueueException&) {
			// Catch it here to allow parts of directories to be added...
		} catch(const FileException&) {
			//..
		}
	}
}

void DirectoryListing::download(const string& aDir, const string& aTarget) {
	dcassert(aDir.size() > 2);
	dcassert(aDir[aDir.size() - 1] == '\\');
	Directory* d = find(aDir, getRoot());
	if(d != NULL)
		download(d, aTarget);
}

DirectoryListing::Directory* DirectoryListing::find(const string& aName, Directory* current) {
	string::size_type end = aName.find('\\');
	dcassert(end != string::npos);
	Name n(aName.substr(0, end));
	Directory::Iter i = current->directories.find((Directory*)&n);
	if(i != current->directories.end()) {
		if(end == (aName.size() - 1))
			return *i;
		else
			return find(aName.substr(end + 1), *i);
	}
	return NULL;
}

int64_t DirectoryListing::Directory::getTotalSize(bool adls) {
	int64_t x = getSize();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adls && (*i)->getAdls()))
			x += (*i)->getTotalSize(adls);
	}
	return x;
}

int DirectoryListing::Directory::getTotalFileCount(bool adls) {
	int x = getFileCount();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adls && (*i)->getAdls()))
			x += (*i)->getTotalFileCount(adls);
	}
	return x;
}

void DirectoryListing::download(File* aFile, const string& aTarget, bool view /* = false */) {
	QueueManager::getInstance()->add(getPath(aFile) + aFile->getName(), aFile->getSize(), user, aTarget, 
		Util::emptyString, (view ? (QueueItem::FLAG_TEXT | QueueItem::FLAG_CLIENT_VIEW) : QueueItem::FLAG_RESUME));
}

/**
 * @file
 * $Id: DirectoryListing.cpp,v 1.20 2003/11/06 18:54:39 arnetheduck Exp $
 */
