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

#if !defined(AFX_SIMPLEXML_H__3FDC96DD_A4D6_4357_9557_9D7585529A98__INCLUDED_)
#define AFX_SIMPLEXML_H__3FDC96DD_A4D6_4357_9557_9D7585529A98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Exception.h"
#include "Util.h"

STANDARD_EXCEPTION(SimpleXMLException);

/**
 * A simple XML class that loads an XML-ish structure into an internal tree
 * and allows easy access to each element through a "current location".
 */
class SimpleXML  
{
public:
	SimpleXML() : found(false) { root = current = new Tag("BOGUSROOT", "", NULL); };
	~SimpleXML() { delete root; }
	
	void addTag(const string& aName, const string& aData = "");
	void addTag(const string& aName, int aData) {
		addTag(aName, Util::toString(aData));
	}

	void addAttrib(const string& aName, const string& aData) throw(SimpleXMLException);
	void addAttrib(const string& aName, int aData) throw(SimpleXMLException) {
		addAttrib(aName, Util::toString(aData));
	}
	void addAttrib(const string& aName, LONGLONG aData) throw(SimpleXMLException) {	
		addAttrib(aName, Util::toString(aData));
	}
	void addAttrib(const string& aName, bool aData) throw(SimpleXMLException) {	
		addAttrib(aName, string(aData ? "1" : "0"));
	}
	
	void addChildAttrib(const string& aName, const string& aData) throw(SimpleXMLException);
	void addChildAttrib(const string& aName, int aData) throw(SimpleXMLException) {	
		addChildAttrib(aName, Util::toString(aData));
	}
	void addChildAttrib(const string& aName, LONGLONG aData) throw(SimpleXMLException) {	
		addChildAttrib(aName, Util::toString(aData));
	}
	void addChildAttrib(const string& aName, bool aData) throw(SimpleXMLException) {	
		addChildAttrib(aName, string(aData ? "1" : "0"));
	}
	
	const string& getData() const {
		dcassert(current != NULL);
		return current->data;
	}
	
	void stepIn() throw(SimpleXMLException) {
		checkChildSelected();
		current = *currentChild;
		currentChild = current->children.begin();
		found = false;
	}

	void stepOut() throw(SimpleXMLException) {
		if(current == root)
			throw SimpleXMLException("Already at lowest level");

		dcassert(current->parent != NULL);

		currentChild = find(current->parent->children.begin(), current->parent->children.end(), current);
		
		current = current->parent;
		found = true;
	}

	void resetCurrentChild() {
		found = false;
		dcassert(current != NULL);
		currentChild = current->children.begin();
	}

	bool findChild(const string& aName) {
		dcassert(current != NULL);

		if(found && currentChild != current->children.end())
			currentChild++;

		while(currentChild!=current->children.end()) {
			if((*currentChild)->name == aName) {
				found = true;
				return true;
			} else
				currentChild++;
		}
		return false;
	}

	const string& getChildData() const throw(SimpleXMLException) {
		checkChildSelected();
		return (*currentChild)->data;
	}

	const string& getChildAttrib(const string& aName) throw(SimpleXMLException) {
		checkChildSelected();
		return (*currentChild)->attribs[aName];
	}

	int getIntChildAttrib(const string& aName) throw(SimpleXMLException) {
		checkChildSelected();
		return Util::toInt(getChildAttrib(aName));
	}
	LONGLONG getLongLongChildAttrib(const string& aName) throw(SimpleXMLException) {
		checkChildSelected();
		return Util::toInt64(getChildAttrib(aName));
	}
	bool getBoolChildAttrib(const string& aName) throw(SimpleXMLException) {
		checkChildSelected();
		const string& tmp = getChildAttrib(aName);

		return (tmp.size() > 0) && tmp[0] == '1';
	}
	
	void fromXML(const string& aXML) throw(SimpleXMLException);
	string toXML() { return (!root->children.empty()) ? root->children[0]->toXML() : Util::emptyString; };
	

private:
	class Tag {
	public:
		typedef Tag* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;

		/**
		 * A simple list of children. To find a tag, one must search the entire list.
		 */ 
		List children;
		/**
		 * Attributes of this tag. According to the XML standard the names
		 * must be unique (case-sensitive).
		 */
		StringMap attribs;
		
		/** Tag data, may be empty. */
		string data;
		
		/** Tag name */
		string name;
		
		/** Parent tag, for easy traversal */
		Ptr parent;

		Tag(const string& aName, const string& aData, Ptr aParent) : name(aName), data(aData), parent(aParent) { };
		
		string toXML();
		void fromXML(const string& aXML);
		string getAttribString();
		/** Delete all children! */
		~Tag() {
			for(Iter i = children.begin(); i != children.end(); ++i) {
				delete *i;
			}
		}
	};

	/** Bogus root tag, should be only one child! */
	Tag::Ptr root;

	/** Current position */
	Tag::Ptr current;

	Tag::Iter currentChild;

	string cleanUp(const string& aString);
	void checkChildSelected() const throw(SimpleXMLException) {
		dcassert(current != NULL);
		if(currentChild == current->children.end()) {
			throw SimpleXMLException("No child selected");				
		}
	}

	bool found;

};

#endif // !defined(AFX_SIMPLEXML_H__3FDC96DD_A4D6_4357_9557_9D7585529A98__INCLUDED_)

/**
 * @file SimpleXML.cpp
 * $Id: SimpleXML.h,v 1.13 2002/03/10 22:41:08 arnetheduck Exp $
 * @if LOG
 * $Log: SimpleXML.h,v $
 * Revision 1.13  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.12  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.11  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.10  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.9  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.8  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.7  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.6  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.5  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.4  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.3  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.2  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.1  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * @endif
 */

