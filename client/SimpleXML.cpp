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

string SimpleXML::escape(const string& aString, bool aAttrib, bool aReverse /* = false */) {
	string::size_type i;
	const char* chars = aAttrib ? "<&>'\"" : "<&>";
	
	if(aReverse) {
		if(aString.find('&') == string::npos) {
			return aString;
		} else {
			string tmp = aString;
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
			return tmp;
		}
	} else {
		i = aString.find_first_of(chars);
		if(i == string::npos) {
			return aString;
		} else {
			string tmp = aString;
			do {
				switch(tmp[i]) {
				case '<': tmp.replace(i, 1, "&lt;"); i+=4; break;
				case '&': tmp.replace(i, 1, "&amp;"); i+=5; break;
				case '>': tmp.replace(i, 1, "&gt;"); i+=4; break;
				case '\'': tmp.replace(i, 1, "&apos;"); i+=6; break;
				case '"': tmp.replace(i, 1, "&quot;"); i+=6; break;
				default: dcassert(0);
				}
			} while( (i = tmp.find_first_of(chars, i)) != string::npos);
			return tmp;
		}
	}
	dcasserta(0);
}

string SimpleXML::Tag::getAttribString() {
	string tmp;
	for(AttribIter i = attribs.begin(); i!= attribs.end(); ++i) {
		tmp.append(i->first);
		tmp.append("=\"", 2);
		tmp.append(needsEscape(i->second, true) ? escape(i->second, true) : i->second);
		tmp.append("\" ", 2);
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
				string tmp;
				tmp.reserve(indent + name.length() + 5);
				tmp.append(indent, '\t');
				tmp.append(1, '<');
				tmp.append(name);
				tmp.append("/>\r\n", 4);
				return tmp;
			} else {
				string attr = getAttribString();
				string tmp;
				tmp.reserve(indent + name.length() + attr.length() + 6);
				tmp.append(indent, '\t');
				tmp.append(1, '<');
				tmp.append(name);
				tmp.append(1, ' ');
				tmp.append(attr);
				tmp.append("/>\r\n", 4);
				return tmp;
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
		string tmp(indent, '\t');
		tmp.append(1, '<');
		tmp.append(name);
		tmp.append(">\r\n", 3);
		for(Iter i = children.begin(); i!=children.end(); ++i) {
			tmp.append((*i)->toXML(indent + 1));
		}
		tmp.append(indent, '\t');
		tmp.append("</", 2);
		tmp.append(name);
		tmp.append(">\r\n", 3);
		return tmp;
	} else {
		string tmp(indent, '\t');
		tmp.append(1, '<');
		tmp.append(name);
		tmp.append(1, ' ');
		tmp.append(getAttribString());
		tmp.append(">\r\n", 3);
		for(Iter i = children.begin(); i!=children.end(); ++i) {
			tmp.append((*i)->toXML(indent + 1));
		}
		tmp.append(indent, '\t');
		tmp.append("</", 2);
		tmp.append(name);
		tmp.append(">\r\n", 3);
		return tmp;
	}
}

/**
 * The same as the version above, but writes to a file instead...yes, this could be made
 * with streams and only one code set but streams are slow...the file f should be a buffered
 * file, otherwise things will be very slow (I assume write is not expensive and call it a lot
 */
void SimpleXML::Tag::toXML(int indent, File* f) {
	if(children.empty()) {
		if(data.empty()) {
			if(attribs.empty()) {
				for(int i = 0; i < indent; i++)
					f->write("\t", 1);
				f->write("<", 1);
				f->write(name);
				f->write("/>\r\n", 4);
				return;
			} else {
				string attr = getAttribString();
				string tmp(indent, '\t');
				for(int i = 0; i < indent; i++)
					f->write("\t", 1);

				f->write("<", 1);
				f->write(name);
				f->write(" ", 1);
				f->write(attr);
				f->write("/>\r\n", 4);
				return;
			}
		} else {
			if(attribs.empty()) {
				for(int i = 0; i < indent; i++)
					f->write("\t", 1);
				f->write("<", 1);
				f->write(name);
				f->write(">", 1);
				f->write(needsEscape(data, false) ? escape(data, false) : data);
				f->write("</", 2);
				f->write(name);
				f->write(">\r\n", 3);
				return;
			} else {
				for(int i = 0; i < indent; i++)
					f->write("\t", 1);
				f->write("<", 1);
				f->write(name);
				f->write(" ", 1);
				f->write(getAttribString());
				f->write(">", 1);
				f->write(needsEscape(data, false) ? escape(data, false) : data);
				f->write("</", 2);
				f->write(name);
				f->write(">\r\n", 3);
				return;
			}
		}
	} else if(attribs.empty()) {
		int i;
		for(i = 0; i < indent; i++)
			f->write("\t", 1);
		f->write("<", 1);
		f->write(name);
		f->write(">\r\n", 3);
		for(Iter it = children.begin(); it!=children.end(); ++it) {
			(*it)->toXML(indent + 1, f);
		}
		for(i = 0; i < indent; i++)
			f->write("\t", 1);
		f->write("</", 2);
		f->write(name);
		f->write(">\r\n", 3);
		return;
	} else {
		int i;
		for(i = 0; i < indent; i++)
			f->write("\t", 1);
		f->write("<", 1);
		f->write(name);
		f->write(" ", 1);
		f->write(getAttribString());
		f->write(">\r\n", 3);
		for(Iter it = children.begin(); it!=children.end(); ++it) {
			(*it)->toXML(indent + 1, f);
		}
		for(i = 0; i < indent; i++)
			f->write("\t", 1);
		f->write("</", 2);
		f->write(name);
		f->write(">\r\n", 3);
		return;
	}
}

bool SimpleXML::findChild(const string& aName) throw() {
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


void SimpleXML::Tag::fromXML(const string& tmp, string::size_type start, string::size_type end, int aa /* = 0 */) throw(SimpleXMLException) {
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
		string::size_type tagStart = 0;
		string::size_type tagEnd = 0;
		bool simpleTag = false;
		if(tmp[j] == ' ') {
			name = tmp.substr(i, j-i);
			i = j+1;
			j = tmp.find(">", i);
			if(j == string::npos) {
				throw SimpleXMLException("Missing '>'");
			}
			tagStart = i;
			if(tmp[j-1] == '/') {
				tagEnd = j - 1;
				simpleTag = true;
			} else {
				tagEnd = j;
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

		string::size_type x = tagStart;
		string::size_type y;

		child = new Tag(name, Util::emptyString, this, aa);
		while( (x < tagEnd) && (y=tmp.find('=', x)) != string::npos && (y < tagEnd)) {
			x = tmp.find_first_not_of(' ', x);
			
			string::size_type x2 = y + 2;
			string::size_type y2 = tmp.find('"', x2);
			if(y2 == string::npos || y2 > tagEnd) {
				throw SimpleXMLException("Missing '\"'");
			}
			child->attribs.push_back(make_pair(tmp.substr(x, y-x), tmp.substr(x2, y2-x2)));
			if(needsEscape(child->attribs.back().second, true, true))
				child->attribs.back().second = escape(child->attribs.back().second, true, true);
			x = y2 + 1;
		}
		
		if(!simpleTag) {
			string endTag = "</" + name + ">";
			j = tmp.find(endTag, i);
			string::size_type dataPos = tmp.find('<', i);

			if(dataPos != string::npos && dataPos < j) {
				child->fromXML(tmp, i, j, aa);
			} else {
				child->data = tmp.substr(i, j-i);
				if(needsEscape(child->data, false, true))
					child->data = escape(child->data, false, true);
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
			current->children.push_back(new Tag(aName, aData, root, attribs));
			currentChild = current->children.begin();
		} else {
			throw SimpleXMLException("Only one root tag allowed");
		}
	} else {
		current->children.push_back(new Tag(aName, aData, current, attribs));
		currentChild = current->children.end() - 1;
	}
}

void SimpleXML::addAttrib(const string& aName, const string& aData) throw(SimpleXMLException) {
	if(current==root)
		throw SimpleXMLException("No tag is currently selected");

	current->attribs.push_back(make_pair(aName, aData));
}

void SimpleXML::addChildAttrib(const string& aName, const string& aData) throw(SimpleXMLException) {
	checkChildSelected();

	(*currentChild)->attribs.push_back(make_pair(aName, aData));
}

void SimpleXML::fromXML(const string& aXML) throw(SimpleXMLException) {
	if(root) {
		delete root;
	}
	root = new Tag("BOGUSROOT", Util::emptyString, NULL, 0);

	root->fromXML(aXML, 0, aXML.size(), attribs);
	
	if(root->children.size() != 1) {
		throw SimpleXMLException("Invalid XML file, missing or multiple root tags");
	}
	
	current = root;
	currentChild = current->children.begin();
}

/**
 * @file SimpleXML.cpp
 * $Id: SimpleXML.cpp,v 1.17 2002/12/28 01:31:49 arnetheduck Exp $
 */

