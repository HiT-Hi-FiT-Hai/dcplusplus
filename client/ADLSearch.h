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

/*
 * Automatic Directory Listing Search
 * Henrik Engstr�m, henrikengstrom@home.se
 */

#if !defined(__ADLSEARCH_H__)
#define __ADLSEARCH_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Util.h"

///////////////////////////////////////////////////////////////////////////////
//
//	QuickSearch substring search (must be prepared before searching)
//
///////////////////////////////////////////////////////////////////////////////
typedef int _QsTable[256];
typedef struct _Qs { _QsTable table; string pattern; } Qs;
typedef vector<Qs> QsVector;

inline
void PrepareQuickSearch(Qs& qs)
{
	// Prepare QuickSearch lookup table
	int i, m = qs.pattern.size();
	for(i = 0; i < 256; ++i)
	{
		qs.table[i] = m + 1;
	}	
	for(i = 0; i < m; ++i)
	{
		qs.table[(unsigned char)qs.pattern[i]] = m - i;
	}
}

inline
void PrepareQuickSearchVector(const string& multistring, QsVector& qsv)
{
	// Split multiple substrings into single substrings
	Qs substring;
	qsv.clear();
	bool inSubstring = false;
	for(unsigned long p = 0; p < multistring.size(); ++p)
	{
		if(isgraph(multistring[p]))
		{
			if(inSubstring)
			{
				// Continue substring
				substring.pattern.push_back(multistring[p]);
			}
			else
			{
				// Start of new substring
				inSubstring = true;
				substring.pattern.clear();
				substring.pattern.push_back(multistring[p]);
			}
		}
		else
		{
			if(inSubstring)
			{
				// End of substring
				inSubstring = false;
				PrepareQuickSearch(substring);
				qsv.push_back(substring);
			}
		}
	}
	if(inSubstring)
	{
		// End of last substring
		PrepareQuickSearch(substring);
		qsv.push_back(substring);
	}
}

inline
bool QuickSearch(const Qs& qs, const string& s)
{
	// Do QuickSearch
	int j = 0, m = qs.pattern.size(), n = s.size();
	while(j <= n - m) 
	{
		if(memcmp(qs.pattern.begin(), s.begin() + j, m) == 0)
		{
			return true;
		}
		j += qs.table[(unsigned char)s[j + m]];
	}
	return false;
}

inline 
bool QuickSearchAll(QsVector& qsv, const string& s)
{
	// Match all substrings
	for(QsVector::iterator q = qsv.begin(); q != qsv.end(); ++q)
	{
		if(!QuickSearch(*q, s))
		{
			return false;
		}
	}
	return (qsv.size() != 0);
}


///////////////////////////////////////////////////////////////////////////////
//
//	Class that represent an ADL search
//
///////////////////////////////////////////////////////////////////////////////
class ADLSearch
{
public:

	// Constructor
	ADLSearch() : searchString("<Enter string>"), isActive(true), sourceType(OnlyFile), 
		minFileSize(-1), maxFileSize(-1), destDir("ADLSearch"), ddIndex(0), typeFileSize(SizeBytes) {}

	// Prepare search
	void Prepare()
	{
		// Prepare quick search of substrings
		PrepareQuickSearchVector(Util::toLower(searchString), qsv);
	}

	// The search string
	string searchString;									 

	// Active search
	bool isActive;

	// Search source type
	enum SourceType
	{
		TypeFirst = 0,
		OnlyFile = TypeFirst,
		OnlyDirectory,
		FullPath,
		TypeLast
	} sourceType;
	SourceType StringToSourceType(const string& s)
	{
		if(Util::stricmp(s.c_str(), "Filename") == 0)
		{
			return OnlyFile;
		}
		else if(Util::stricmp(s.c_str(), "Directory") == 0)
		{
			return OnlyDirectory;
		}
		else if(Util::stricmp(s.c_str(), "Full Path") == 0)
		{
			return FullPath;
		}
		else
		{
			return OnlyFile;
		}
	}
	string SourceTypeToString(SourceType t)
	{
		switch(t)
		{
		default:
		case OnlyFile:		return "Filename";
		case OnlyDirectory:	return "Directory";
		case FullPath:		return "Full Path";
		}
	}

	// Maximum & minimum file sizes (in bytes). 
	// Negative values means do not check.
	int64_t minFileSize;
	int64_t maxFileSize;
	enum SizeType
	{
		SizeBytes     = TypeFirst,
		SizeKiloBytes,
		SizeMegaBytes,
		SizeGigaBytes
	};
	SizeType typeFileSize;
	SizeType StringToSizeType(const string& s)
	{
		if(Util::stricmp(s.c_str(), "B") == 0)
		{
			return SizeBytes;
		}
		else 
		if(Util::stricmp(s.c_str(), "kB") == 0)
		{
			return SizeKiloBytes;
		}
		else 
		if(Util::stricmp(s.c_str(), "MB") == 0)
		{
			return SizeMegaBytes;
		}
		else 
		if(Util::stricmp(s.c_str(), "GB") == 0)
		{
			return SizeGigaBytes;
		}
		else
		{
			return SizeBytes;
		}
	}
	string SizeTypeToString(SizeType t)
	{
		switch(t)
		{
		default:
		case SizeBytes:		return "B";
		case SizeKiloBytes:	return "kB";
		case SizeMegaBytes:	return "MB";
		case SizeGigaBytes:	return "GB";
		}
	}
	string SizeTypeToStringInternational(SizeType t)
	{
		switch(t)
		{
		default:
		case SizeBytes:		return CSTRING(B);
		case SizeKiloBytes:	return CSTRING(KB);
		case SizeMegaBytes:	return CSTRING(MB);
		case SizeGigaBytes:	return CSTRING(GB);
		}
	}
	int64_t GetSizeBase()
	{
		switch(typeFileSize)
		{
		default:
		case SizeBytes:		return (int64_t)1;
		case SizeKiloBytes:	return (int64_t)1024;
		case SizeMegaBytes:	return (int64_t)1024 * (int64_t)1024;
		case SizeGigaBytes:	return (int64_t)1024 * (int64_t)1024 * (int64_t)1024;
		}
	}

	// Name of the destination directory (empty = 'ADLSearch') and its index
	string destDir;
	unsigned long ddIndex;

	// Search for file match 
	bool MatchesFile(const string& f, const string& fp, int64_t size)
	{
		// Check status
		if(!isActive)
		{
			return false;
		}

		// Check size for files
		if(size >= 0 && (sourceType == OnlyFile || sourceType == FullPath))
		{
			if(minFileSize >= 0 && size < minFileSize * GetSizeBase())
			{
				// Too small
				return false;
			}
			if(maxFileSize >= 0 && size > maxFileSize * GetSizeBase())
			{
				// Too large
				return false;
			}
		}

		// Do search
		switch(sourceType)
		{
		default:
		case OnlyDirectory:	return false;
		case OnlyFile:		return QuickSearchAll(qsv, f);
		case FullPath:		return QuickSearchAll(qsv, fp);
		}
	}

	// Search for directory match 
	bool MatchesDirectory(const string& d)
	{
		// Check status
		if(!isActive)
		{
			return false;
		}
		if(sourceType != OnlyDirectory)
		{
			return false;
		}

		// Do search
		return QuickSearchAll(qsv, d);
	}

private:

	// Substring searches
	QsVector qsv;
};

///////////////////////////////////////////////////////////////////////////////
//
//	Class that holds all active searches
//
///////////////////////////////////////////////////////////////////////////////
#include "DirectoryListing.h"
class ADLSearchManager : public Singleton<ADLSearchManager>
{
public:

	// Constructor/destructor
	ADLSearchManager() { Load(); }
	virtual ~ADLSearchManager() { Save(); }

	// Search collection
	typedef vector<ADLSearch> SearchCollection;
	SearchCollection collection;

	// Load/save search collection to XML file
	void Load();
	void Save();

	// Search for file match
	void MatchesFile(DirectoryListing::File *currentFile, string& fullPath)
	{
		// Add to any substructure being stored
		for(vector<DestDir>::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id)
		{
			if(id->subdir != NULL)
			{
				DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile);
				copyFile->setAdls(true);
				id->subdir->files.push_back(copyFile);
			}
			id->fileAdded = false;	// Prepare for next stage
		}

		// Prepare to match searches
		if(currentFile->getName().size() < 1)
		{
			return;
		}
		string fileName = Util::toLower(currentFile->getName());
		string filePath = Util::toLower(fullPath);
		filePath += "\\";
		filePath += fileName;

		// Match searches
		for(SearchCollection::iterator is = collection.begin(); is != collection.end(); ++is)
		{
			if(destDirVector[is->ddIndex].fileAdded)
			{
				continue;
			}
			if(is->MatchesFile(fileName, filePath, currentFile->getSize()))
			{
				DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile);
				copyFile->setAdls(true);
				destDirVector[is->ddIndex].dir->files.push_back(copyFile);
				destDirVector[is->ddIndex].fileAdded = true;
			}
		}
	}

	// Search for directory match
	void MatchesDirectory(DirectoryListing::Directory* currentDir, string& fullPath)
	{
		// Add to any substructure being stored
		for(vector<DestDir>::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id)
		{
			if(id->subdir != NULL)
			{
				DirectoryListing::Directory* newDir = 
					new DirectoryListing::AdlDirectory(fullPath, id->subdir, currentDir->getName());
				id->subdir->directories.push_back(newDir);
				id->subdir = newDir;
			}
		}

		// Prepare to match searches
		if(currentDir->getName().size() < 1)
		{
			return;
		}
		string dirName = Util::toLower(currentDir->getName());

		// Match searches
		for(SearchCollection::iterator is = collection.begin(); is != collection.end(); ++is)
		{
			if(destDirVector[is->ddIndex].subdir != NULL)
			{
				continue;
			}
			if(is->MatchesDirectory(dirName))
			{
				destDirVector[is->ddIndex].subdir = 
					new DirectoryListing::AdlDirectory(fullPath, destDirVector[is->ddIndex].dir, currentDir->getName());
				destDirVector[is->ddIndex].dir->directories.push_back(destDirVector[is->ddIndex].subdir);
			}
		}
	}

	// Step up directory
	void StepUpDirectory()
	{
		for(vector<DestDir>::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id)
		{
			if(id->subdir != NULL)
			{
				id->subdir = id->subdir->getParent();
				if(id->subdir == id->dir)
				{
					id->subdir = NULL;
				}
			}
		}
	}

	// Prepare destination directory indexing
	void PrepareDestinationDirectories(DirectoryListing::Directory* root)
	{
		// Load default destination directory (index = 0)
		destDirVector.clear();
		vector<DestDir>::iterator id = destDirVector.insert(destDirVector.end());
		id->name = "ADLSearch";
		id->dir  = new DirectoryListing::Directory(root, "<<<" + id->name + ">>>", true);

		// Scan all loaded searches
		for(SearchCollection::iterator is = collection.begin(); is != collection.end(); ++is)
		{
			// Check empty destination directory
			if(is->destDir.size() == 0)
			{
				// Set to default
				is->ddIndex = 0;
				continue;
			}

			// Check if exists
			bool isNew = true;
			long ddIndex = 0;
			for(id = destDirVector.begin(); id != destDirVector.end(); ++id, ++ddIndex)
			{
				if(Util::stricmp(is->destDir.c_str(), id->name.c_str()) == 0)
				{
					// Already exists, reuse index
					is->ddIndex = ddIndex;
					isNew = false;
					break;
				}
			}
			if(isNew)
			{
				// Add new destination directory
				id = destDirVector.insert(destDirVector.end());
				id->name = is->destDir;
				id->dir  = new DirectoryListing::Directory(root, "<<<" + id->name + ">>>", true);
				is->ddIndex = ddIndex;
			}
		}
	}

	// Finalize destination directories
	void FinalizeDestinationDirectories(DirectoryListing::Directory* root)
	{
		// Add non-empty destination directories to the top level
		for(vector<DestDir>::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id)
		{
			if(id->dir->files.size() == 0 && id->dir->directories.size() == 0)
			{
				delete (id->dir);
			}
			else
			{
				root->directories.push_back(id->dir);
			}
		}
	}

private:

	// Destination directory indexing
	struct DestDir
	{
		string name;
		DirectoryListing::Directory* dir;
		DirectoryListing::Directory* subdir;
		bool fileAdded;
		DestDir() : name(""), dir(NULL), subdir(NULL) {}
	};
	vector<DestDir> destDirVector;
};


#endif

/**
 * @file
 * $Id: ADLSearch.h,v 1.4 2003/05/07 09:52:09 arnetheduck Exp $
 */
