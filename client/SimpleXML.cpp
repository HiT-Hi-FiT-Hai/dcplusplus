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

#include "stdinc.h"
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
	if(tmp.size() > 0) {
		tmp.erase(tmp.size() - 1);
	}
	return tmp;
}

string SimpleXML::Tag::toXML(int indent) {
	if(children.empty()) {
		if(data.empty()) {
			if(attribs.empty()) {
				return string(indent, '\t') + "<" + name + "/>\r\n";
			} else {
				return string(indent, '\t') + "<" + name + " " + getAttribString() + "/>\r\n";
			}
		} else {
			if(attribs.empty()) {
				string tmp = escape(data, false);
				char* buf = new char[indent + name.length()*2 + tmp.length() + 10];
				sprintf(buf, "%s<%s>%s</%s>\r\n", string(indent, '\t').c_str(), name.c_str(), tmp.c_str(), name.c_str());
				tmp = buf;
				delete[] buf;
				return tmp;
			} else {
				string tmp = escape(data, false);
				string attrib = getAttribString();
				char* buf = new char[indent + name.length()*2 + tmp.length() + attrib.length() + 10];
				sprintf(buf, "%s<%s %s>%s</%s>\r\n", string(indent, '\t').c_str(), name.c_str(), attrib.c_str(), tmp.c_str(), name.c_str());
				tmp = buf;
				delete[] buf;
				return tmp;
			}
		}
	} else if(attribs.empty()) {
		string tmp =  string(indent, '\t') + "<" + name + ">\r\n";
		for(Iter i = children.begin(); i!=children.end(); ++i) {
			tmp += (*i)->toXML(indent + 1);
		}
		return tmp + string(indent, '\t') + "</" + name + ">\r\n";
	} else {		
		string tmp =  string(indent, '\t') + "<" + name + " " + getAttribString() + ">\r\n";
		for(Iter i = children.begin(); i!=children.end(); ++i) {
			tmp += (*i)->toXML(indent + 1);
		}
		return tmp + string(indent, '\t') + "</" + name + ">\r\n";
	}

}

void SimpleXML::Tag::fromXML(const string& tmp, string::size_type start, string::size_type end) throw(SimpleXMLException) {
	string::size_type i = start;
	string::size_type j;

	dcassert(tmp.size() > 0);

	
	while( (j = tmp.find('<', i)) != string::npos ) {
		// Check that we have at least 3 more characters as the shortest valid xml tag is <a/>...
		if((j + 3) > tmp.size()) {
			throw SimpleXMLException("Too few characters after <");
		}

		if( (j+3) > end)
			break;

		Ptr child = NULL;

		i = j + 1;

		if(tmp[i] == '?') {
			// <? processing instruction ?>, ignore...
			i = tmp.find("?>", i);
			if(i == string::npos) {
				throw SimpleXMLException("Missing ?> tag");
			}
			continue;
		}
		
		if(tmp[i] == '!' && tmp[i+1] == '-' && tmp[i+2] == '-') {
			// <!-- comment -->, ignore...
			i = tmp.find("-->", i);
			if(i == string::npos) {
				throw SimpleXMLException("Missing --> tag");
			}
			continue;
		}

		// Alright, we have a real tag for sure...
		j = tmp.find_first_of(" >", i);

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

		child = new Tag(name, Util::emptyString, this);

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
			string::size_type dataPos = tmp.find('<', i);

			if(dataPos != string::npos && dataPos < j) {
				child->fromXML(tmp, i, j);
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

void SimpleXML::addAttrib(const string& aName, const string& aData) throw(SimpleXMLException) {
	if(current==root)
		throw SimpleXMLException("No tag is currently selected");

	current->attribs[aName] = aData;
}

void SimpleXML::addChildAttrib(const string& aName, const string& aData) throw(SimpleXMLException) {
	checkChildSelected();

	(*currentChild)->attribs[aName] = aData;
}

void SimpleXML::fromXML(const string& aXML) throw(SimpleXMLException) {
	if(root) {
		delete root;
	}
	root = new Tag("BOGUSROOT", Util::emptyString, NULL);

	root->fromXML(aXML, 0, aXML.size());
	
	if(root->children.size() != 1) {
		throw SimpleXMLException("Invalid XML file, missing or multiple root tags");
	}
	
	current = root;
	currentChild = current->children.begin();
}

/**
 * @file SimpleXML.cpp
 * $Id: SimpleXML.cpp,v 1.15 2002/04/16 16:45:54 arnetheduck Exp $
 */

