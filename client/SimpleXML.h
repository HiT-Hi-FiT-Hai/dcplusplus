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
		
//		static string escape(const string& aString, boolean aAttrib, boolean aReverse = false);
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

	/** Root tag, should be only one! */
	Tag::List root;

	/** Current position */
	Tag::Ptr current;

	Tag::Iter currentChild;

	static string emptyString;
	string cleanUp(const string& aString);
	void checkChildSelected() throw(SimpleXMLException) {
		if(current != NULL) {
			if(currentChild == current->children.end()) {
				throw SimpleXMLException("No child selected");				
			}
		} else {
			if(currentChild == root.end()) {
				throw SimpleXMLException("No child selected");				
			}
		}
	}

	bool found;
public:

	void addTag(const string& aName, const string& aData = "");
	void addTag(const string& aName, int aData) {
		char buf[32];
		addTag(aName, itoa(aData, buf, 10));
	}

	void addAttrib(const string& aName, const string& aData) throw(SimpleXMLException);
	void addAttrib(const string& aName, int aData) {
		char buf[32];
		addAttrib(aName, itoa(aData, buf, 10));
	}

	void addChildAttrib(const string& aName, const string& aData) throw(SimpleXMLException);
	void addChildAttrib(const string& aName, int aData) throw(SimpleXMLException) {	
		char buf[32];
		addChildAttrib(aName, itoa(aData, buf, 10));
	}
		
	const string& getData() {
		if(current != NULL) {
			return current->data;
		} else {
			return emptyString;
		}
	}
	
	void stepIn() throw(SimpleXMLException) {
		checkChildSelected();
		current = *currentChild;
		currentChild = current->children.begin();
		found = false;
	}

	void stepOut() throw(SimpleXMLException) {
		if(current == NULL)
			throw SimpleXMLException("Already at lowest level");
		current = current->parent;
		found = false;
		if(current != NULL)
			currentChild = current->children.begin();
		else
			currentChild = root.begin();
	}

	void resetCurrentChild() {
		found = false;
		if(current)
			currentChild=current->children.begin();
		else
			currentChild=root.begin();
	}

	bool findChild(const string& aName) {
		if(current == NULL) {
			while(currentChild!=root.end()) {
				if((*currentChild)->name == aName) {
					found = true;
					return true;
				} else
					currentChild++;
			}
			return false;
		} else {
			if(found)
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
	}

	string getChildData() throw(SimpleXMLException) {
		checkChildSelected();
		return (*currentChild)->data;
	}

	string getChildAttrib(const string& aName) throw(SimpleXMLException) {
		checkChildSelected();
		return (*currentChild)->attribs[aName];
	}

	int getIntChildAttrib(const string& aName) throw(SimpleXMLException) {
		checkChildSelected();
		return atoi(getChildAttrib(aName).c_str());
	}

	void fromXML(const string& aXML);
	string toXML() { return (!root.empty()) ? root[0]->toXML() : emptyString; };
	
	SimpleXML() : current(NULL), found(false) {  };
	~SimpleXML() { if(!root.empty()) delete root[0]; }

};

#endif // !defined(AFX_SIMPLEXML_H__3FDC96DD_A4D6_4357_9557_9D7585529A98__INCLUDED_)

/**
 * @file SimpleXML.cpp
 * $Id: SimpleXML.h,v 1.5 2001/12/13 19:21:57 arnetheduck Exp $
 * @if LOG
 * $Log: SimpleXML.h,v $
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

