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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "SimpleXML.h"

string SimpleXML::emptyString;

static string escape(const string& aString, bool aAttrib, bool aReverse = false) {
	string::size_type i;
	string tmp = aString;
	const char* chars = aAttrib ? "<&>'\"" : "<&>";
	
	if(aReverse) {
		while( (i=tmp.find("&lt;")) != string::npos) {
			tmp.replace(i, 4, 1, '<');
		}
		while( (i=tmp.find("&amp;")) != string::npos) {
			tmp.replace(i, 4, 1, '&');
		}
		while( (i=tmp.find("&gt;")) != string::npos) {
			tmp.replace(i, 4, 1, '>');
		}
		if(aAttrib) {
			while( (i=tmp.find("&apos;")) != string::npos) {
				tmp.replace(i, 4, 1, '\'');
			}
			while( (i=tmp.find("&quot;")) != string::npos) {
				tmp.replace(i, 4, 1, '"');
			}
		}
	} else {
		while( (i = tmp.find_first_of(chars)) != string::npos) {
			
			switch(tmp[i]) {
			case '<': tmp.replace(i, 1, "&lt;"); break;
			case '&': tmp.replace(i, 1, "&amp;"); break;
			case '>': tmp.replace(i, 1, "&gt;"); break;
			case '\'': tmp.replace(i, 1, "&apos;"); break;
			case '"': tmp.replace(i, 1, "&quot;"); break;
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
			delete buf;
			return tmp;
		}
	} else {
		string tmp =  "<" + name + " " + getAttribString() + ">";
		for(Iter i = children.begin(); i!=children.end(); ++i) {
			tmp = tmp + (*i)->toXML();
		}
		return tmp + "</" + name + ">";
	}

	dcassert(0);
}

void SimpleXML::Tag::fromXML(const string& aString) throw(SimpleXMLException) {
	
	string tmp = aString;
	while(!tmp.empty()) {
		Ptr child = NULL;

		if(aString.length()<4 || aString[0] != '<')
			throw SimpleXMLException("Unexpected character during XML decoding");
		
		string tag = tmp.substr(1, tmp.find('>')-1);
		string name = tag.substr(0, tag.find_first_of(" /"));
		tag = tag.substr(name.length());

		tmp = tmp.substr(tmp.find('>')+1);
		
		string::size_type i;

		if(tag.length() > 0 && tag[tag.length()-1] == '/') {
			child = new Tag(name, "", this);

			while( (i=tag.find('=')) != string::npos) {
				tag = tag.substr(tag.find_first_not_of(' '));
				string attr = tag.substr(0, tag.find('='));
				tag = tag.substr(tag.find('=')+2);
				child->attribs[attr] = escape(tag.substr(0, tag.find('"')), true, true);
				tag = tag.substr(tag.find('"')+1);
			}
		} else {
			string endTag = "</" + name + ">";
			string data = tmp.substr(0, tmp.find(endTag));
			tmp = tmp.substr(tmp.find(endTag)+endTag.length());

			if(!data.empty() && data[0] == '<') {
				child = new Tag(name, "", this);
				child->fromXML(escape(data, false, true));
			} else {
				child = new Tag(name, data, this);
			}
		}

		children.push_back(child);
	}	
}

void SimpleXML::addTag(const string& aName, const string& aData /* = "" */) throw(SimpleXMLException) {
	if(aName.empty()) {
		throw SimpleXMLException("Empty tag names not allowed");
	}

	if(root.empty()) {
		root.push_back(new Tag(aName, aData, NULL));
		currentChild = root.begin();
	} else if(current == NULL) {
		throw SimpleXMLException("Only one root tag allowed");
	} else {
		current->children.push_back(new Tag(aName, aData, current));
		currentChild = current->children.end() - 1;
	}
}

void SimpleXML::addAttrib(const string& aName, const string& aData) {
	if(current==NULL)
		throw SimpleXMLException("No tag is currently selected");

	current->attribs[aName] = aData;
}

void SimpleXML::addChildAttrib(const string& aName, const string& aData) {
	checkChildSelected();

	(*currentChild)->attribs[aName] = aData;
}

string SimpleXML::cleanUp(const string& aString) {
	string tmp = aString;
	string ret;

	while(!tmp.empty()) {
		tmp = tmp.substr(tmp.find('<'));
		if(tmp[1] == '?') {
			// Directive, skip
			tmp = tmp.substr(tmp.find("?>")+2);
		} else if(tmp.substr(0, 4) == "<!--") {
			// Comment, skip
			tmp = tmp.substr(tmp.find("-->")+3);
		} else if(tmp.find("/>") < tmp.find('>')) {
			// Simple tag, OK
			ret = ret + tmp.substr(0, tmp.find('>')+1);
			tmp = tmp.substr(tmp.find('>')+1);
		} else {
			// Normal tag with end tag...find it
			string name = tmp.substr(1, tmp.find_first_of(" >")-1);
			string endTag = "</" + name + ">";
			
			// Add start tag
			ret = ret + tmp.substr(0, tmp.find('>')+1);
			tmp = tmp.substr(tmp.find('>')+1);

			string data = tmp.substr(0, tmp.find(endTag));
			tmp = tmp.substr(tmp.find(endTag) + endTag.length());
			if(data.find('<') != string::npos) {
				// We have tags inside...
				data = cleanUp(data);
			}

			ret = ret + data + endTag;
		}
	}
	return ret;
}

void SimpleXML::fromXML(const string& aXML) {
	string tmp = cleanUp(aXML);
	Tag::Ptr child;

	if(tmp.length()<4 || tmp[0] != '<')
		throw SimpleXMLException("Unexpected character during XML decoding");

	if(!root.empty()) {
		delete root[0];
		root.clear();
	}
	
	string tag = tmp.substr(1, tmp.find('>')-1);
	string name = tag.substr(0, tag.find_first_of(" /"));
	tag = tag.substr(name.length());
	
	tmp = tmp.substr(tmp.find('>')+1);
	
	string::size_type i;
	
	if(tag.length()>0 && tag[tag.length()-1] == '/') {
		child = new Tag(name, "", NULL);
		
		while( (i=tag.find('=')) != string::npos) {
			tag = tag.substr(tag.find_first_not_of(' '));
			string attr = tag.substr(0, i);
			tag = tag.substr(i+2);
			child->attribs[attr] = escape(tag.substr(0, tag.find('"')), true, true);
			tag = tag.substr(tag.find('"'));
		}
	} else {
		string endTag = "</" + name + ">";
		string data = tmp.substr(0, tmp.find(endTag));
		tmp = tmp.substr(tmp.find(endTag)+endTag.length());
		
		if(!data.empty() && data[0] == '<') {
			child = new Tag(name, "", NULL);
			child->fromXML(escape(data, false, true));
		} else {
			child = new Tag(name, data, NULL);
		}
	}
	
	root.push_back(child);
	current = NULL;
	currentChild = root.begin();
}
/**
 * @file SimpleXML.cpp
 * $Id: SimpleXML.cpp,v 1.3 2001/12/02 23:47:35 arnetheduck Exp $
 * @if LOG
 * $Log: SimpleXML.cpp,v $
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

