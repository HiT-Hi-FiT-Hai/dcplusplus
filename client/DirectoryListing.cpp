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

void DirectoryListing::load(const string& in) 
{
	StringTokenizer t(in);

	StringList& tokens = t.getTokens();
	string::size_type ident = 0;

	// Prepare ADLSearch manager
	ADLSearchManager* pADLSearch = ADLSearchManager::getInstance();
	Directory *dAdlsSub = NULL, *dAdlsSubRoot = NULL;
	pADLSearch->PrepareDestinationDirectories(root);

	Directory* cur = root;
	string fullPath;

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
			if(dAdlsSub != NULL)
			{
				dAdlsSub = dAdlsSub->getParent();
				if(dAdlsSub == dAdlsSubRoot)
				{
					dAdlsSub = NULL;
				}
			}
		}
		string::size_type k = tok.find('|', j);
		if(k != string::npos) 
		{
			// this must be a file...
			cur->files.push_back(new File(cur, tok.substr(j, k-j), Util::toInt64(tok.substr(k+1))));
			File*& currentFile = cur->files.back();

			// ADLSearch
			Directory *dAdls = pADLSearch->MatchesFile(currentFile->getName(), fullPath, currentFile->getSize());
			if(dAdls != NULL)
			{
				// Add file to destination directory
				dAdls->files.push_back(currentFile);
			}
			if(dAdlsSub != NULL)
			{
				// Add file to substructure being stored
				dAdlsSub->files.push_back(currentFile);
			}
		} 
		else 
		{
			// A directory
			Directory* d = new Directory(cur, tok.substr(j, tok.length()-j-1));
			cur->directories.push_back(d);
			cur = d;
			ident++;
			fullPath += (string)"\\" + d->getName();

			// ADLSearch
			if(dAdlsSub == NULL)
			{
				Directory *dAdls = pADLSearch->MatchesDirectory(d->getName());
				if(dAdls != NULL)
				{
					// Start to store a new substructure
					dAdlsSubRoot = dAdls;
					dAdlsSub     = new Directory(dAdlsSubRoot, d->getName(), true);
					dAdlsSubRoot->directories.push_back(dAdlsSub);
				}
			}
			else
			{
				// Add directory to substructure being stored
				Directory* d2 = new Directory(dAdlsSub, d->getName(), true);
				dAdlsSub->directories.push_back(d2);
				dAdlsSub = d2;
			}
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

void DirectoryListing::download(Directory* aDir, const User::Ptr& aUser, const string& aTarget) {
	string target = (aDir == getRoot()) ? aTarget : aTarget + aDir->getName() + '\\';
	// First, recurse over the directories
	for(Directory::Iter j = aDir->directories.begin(); j != aDir->directories.end(); ++j) {
		download(*j, aUser, target);
	}
	// Then add the files
	for(File::Iter i = aDir->files.begin(); i != aDir->files.end(); ++i) {
		File* file = *i;
		try {
			download(file, aUser, target + file->getName());
		} catch(QueueException e) {
			// Catch it here to allow parts of directories to be added...
		} catch(FileException e) {
			//..
		}
	}
}

void DirectoryListing::download(const string& aDir, const User::Ptr& aUser, const string& aTarget) {
	dcassert(aDir.size() > 2);
	dcassert(aDir[aDir.size() - 1] == '\\');
	Directory* d = find(aDir, getRoot());
	if(d != NULL)
		download(d, aUser, aTarget);
}

DirectoryListing::Directory* DirectoryListing::find(const string& aName, Directory* current) {
	string::size_type end = aName.find('\\');
	dcassert(end != string::npos);
	string cur = aName.substr(0, end);
	for(Directory::Iter i = current->directories.begin(); i != current->directories.end(); ++i) {
		Directory* d = *i;
		if(Util::stricmp(d->getName().c_str(), cur.c_str()) == 0) {
			if(end == (aName.size() - 1))
				return d;
			else
				return find(aName.substr(end + 1), d);
		}
	}
	return NULL;
}

int64_t DirectoryListing::Directory::getTotalSize(bool nosymbolic) {
	int64_t x = getSize();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(nosymbolic && (*i)->getSymbolic()))
			x += (*i)->getTotalSize();
	}
	return x;
}

int DirectoryListing::Directory::getTotalFileCount(bool nosymbolic) {
	int x = getFileCount();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(nosymbolic && (*i)->getSymbolic()))
			x += (*i)->getTotalFileCount();
	}
	return x;
}

/**
 * @file DirectoryListing.cpp
 * $Id: DirectoryListing.cpp,v 1.11 2003/03/13 13:31:19 arnetheduck Exp $
 */
