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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "SimpleXML.h"

static string escape(const string& aString, bool aAttrib, bool aReverse = false) {
	string::size_type i;
	string tmp = aString;
	const char* chars = aAttrib ? "<&>'\"" : "<&>";
	
	if(aReverse) {
		while( (i=tmp.find("&lt;")) != string::npos) {
			tmp.replace(i, 4, 1, '<');
		}
		while( (i=tmp.find("&amp;")) != string::npos) {
			tmp.replace(i, 5, 1, '&');
		}
		while( (i=tmp.find("&gt;")) != string::npos) {
			tmp.replace(i, 4, 1, '>');
		}
		if(aAttrib) {
			while( (i=tmp.find("&apos;")) != string::npos) {
				tmp.replace(i, 6, 1, '\'');
			}
			while( (i=tmp.find("&quot;")) != string::npos) {
				tmp.replace(i, 6, 1, '"');
			}
		}
		if( (i = tmp.find('\n')) != string::npos) {
			if(i > 0 && tmp[i-1] != '\r') {
				// This is a unix thing...decode it...
				i = 0;
				while( (i = tmp.find('\n', i) ) != string::npos) {
					if(tmp[i-1] != '\r')
						tmp.insert(i, 1, '\r');

					i+=2;
				}
			}
		}
	} else {
		i = 0;
		while( (i = tmp.find_first_of(chars, i)) != string::npos) {
			
			switch(tmp[i]) {
			case '<': tmp.replace(i, 1, "&lt;"); i+=4; break;
			case '&': tmp.replace(i, 1, "&amp;"); i+=5; break;
			case '>': tmp.replace(i, 1, "&gt;"); i+=4; break;
			case '\'': tmp.replace(i, 1, "&apos;"); i+=6; break;
			case '"': tmp.replace(i, 1, "&quot;"); i+=6; break;
			default: dcassert(0);
			}
		}
	}
	return tmp;
}

string SimpleXML::Tag::getAttribString() {
	string tmp;
	for(StringMapIter i = attribs.begin(); i!= attribs.end(); ++i) {
		tmp = tmp + i->first + "=\"" + escape(i->second, true) + "\" ";
	}
	return tmp;
}
string SimpleXML::Tag::toXML() {
	if(children.empty()) {
		if(data.empty()) {
			return "<" + name + " " + getAttribString() + "/>";
		} else {
			string tmp = escape(data, false);
			string attrib = getAttribString();
			char* buf = new char[name.length()*2 + tmp.length() + attrib.length() + 8];
			sprintf(buf, "<%s %s>%s</%s>", name.c_str(), attrib.c_str(), tmp.c_str(), name.c_str());
			tmp = buf;
			delete[] buf;
			return tmp;
		}
	} else {
		string tmp =  "<" + name + " " + getAttribString() + ">";
		for(Iter i = children.begin(); i!=children.end(); ++i) {
			tmp += (*i)->toXML();
		}
		return tmp + "</" + name + ">";
	}

	dcassert(0);
}

void SimpleXML::Tag::fromXML(const string& tmp) throw(SimpleXMLException) {
	string::size_type i = 0;
	string::size_type j;

	dcassert(tmp.size() > 0);
	while( i < tmp.size() && tmp[i] == '<' ) {
		Ptr child = NULL;

		i += 1;
		j = tmp.find_first_of(" >", i);
		if(j == string::npos) {
			throw SimpleXMLException("Missing '>'");
		}

		string name;
		string tag;
		bool simpleTag = false;
		if(tmp[j] == ' ') {
			name = tmp.substr(i, j-i);
			i = j+1;
			j = tmp.find(">", i);
			if(j == string::npos) {
				throw SimpleXMLException("Missing '>'");
			}
			if(tmp[j-1] == '/') {
				tag = tmp.substr(i, j-i-1);
				simpleTag = true;
			} else {
				tag = tmp.substr(i, j-i);
			}
		} else {
			if(tmp[j-1] == '/') {
				simpleTag = true;
				name = tmp.substr(i, j-i-1);
			} else {
				name = tmp.substr(i, j-i);
			}
		}
		
		i = j + 1;

		string::size_type x = 0;
		string::size_type y;

		child = new Tag(name, "", this);

		while( (y=tag.find('=', x)) != string::npos) {
			x = tag.find_first_not_of(' ', x);
			
			string attr = tag.substr(x, y-x);
			x = y + 2;
			y = tag.find('"', x);
			if(y == string::npos) {
				throw SimpleXMLException("Missing '\"'");
			}

			child->attribs[attr] = escape(tag.substr(x, y-x), true, true);
			x = y + 1;
		}
		
		if(!simpleTag) {
			string endTag = "</" + name + ">";
			j = tmp.find(endTag, i);
			if(tmp[i] == '<') {
				child->fromXML(tmp.substr(i, j-i));
			} else {
				child->data = escape(tmp.substr(i, j-i), false, true);
			}
			i = j + endTag.length();
		}

		children.push_back(child);
	}	
}

void SimpleXML::addTag(const string& aName, const string& aData /* = "" */) throw(SimpleXMLException) {
	if(aName.empty()) {
		throw SimpleXMLException("Empty tag names not allowed");
	}

	if(current == root) {
		if(current->children.empty()) {
			current->children.push_back(new Tag(aName, aData, NULL));
			currentChild = current->children.begin();
		} else {
			throw SimpleXMLException("Only one root tag allowed");
		}
	} else {
		current->children.push_back(new Tag(aName, aData, current));
		currentChild = current->children.end() - 1;
	}
}

void SimpleXML::addAttrib(const string& aName, const string& aData) {
	if(current==root)
		throw SimpleXMLException("No tag is currently selected");

	current->attribs[aName] = aData;
}

void SimpleXML::addChildAttrib(const string& aName, const string& aData) {
	checkChildSelected();

	(*currentChild)->attribs[aName] = aData;
}

string SimpleXML::cleanUp(const string& tmp) {
	string::size_type i = 0;
	string::size_type j;

	string ret;
	ret.reserve(tmp.size());

	while( (j = tmp.find('<', i)) != string::npos) {
		if(j + 1 >= tmp.size()) {
			break;
		} else if(tmp[j+1] == '?') {
			// Directive, skip
			i = tmp.find("?>", j);
			if(i == string::npos)
				break;
			i += 2;
			continue;
		} else if(tmp.substr(0, 4) == "<!--") {
			// Comment, skip
			i = tmp.find("-->", j);
			if(i == string::npos)
				break;
			i += 3;
			continue;
		} 

		// Find the end of this tag...
		i = tmp.find('>', j);
		if(i == string::npos)
			break;
		if(tmp[i-1] == '/') {
			// Simple tag, ok...
			i++;
			ret += tmp.substr(j, i-j);
		} else {
			// Normal tag with end tag...find it
			string name = tmp.substr(j + 1, tmp.find_first_of(" >", j)-1-j);
			string endTag = "</" + name + ">";
			
			// Add start tag
			i++;
			ret += tmp.substr(j, i-j);
			j = i;
			i = tmp.find(endTag, j);
			if(i == string::npos) {
				// No end tag...add it and return...
				ret += endTag;
				break;
			}
			
			string::size_type x = tmp.find('<', j);

			if( (x != string::npos) && x < i ) {
				ret += cleanUp(tmp.substr(j, i-j));
			} else {
				ret += tmp.substr(j, i-j);
			}

			i += endTag.length();
			ret += endTag;
		}
	}
	return ret;
}

void SimpleXML::fromXML(const string& aXML) throw(SimpleXMLException) {
	if(root) {
		delete root;
	}
	root = new Tag("BOGUSROOT", "", NULL);

	root->fromXML(cleanUp(aXML));
	
	if(root->children.size() != 1) {
		throw SimpleXMLException("Invalid XML file, missing or multiple root tags");
	}
	
	current = root;
	currentChild = current->children.begin();
}

/**
 * @file SimpleXML.cpp
 * $Id: SimpleXML.cpp,v 1.12 2002/03/19 00:41:37 arnetheduck Exp $
 * @if LOG
 * $Log: SimpleXML.cpp,v $
 * Revision 1.12  2002/03/19 00:41:37  arnetheduck
 * 0.162, hub counting and cpu bug
 *
 * Revision 1.11  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.10  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.9  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.8  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.7  2002/01/06 13:24:53  arnetheduck
 * Fixed an XML parsing bug
 *
 * Revision 1.6  2002/01/06 11:13:07  arnetheduck
 * Last fixes before 0.10
 *
 * Revision 1.5  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.4  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.3  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * @endif
 */

