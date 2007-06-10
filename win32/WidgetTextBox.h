/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_WIDGETTEXTBOX_H_
#define DCPLUSPLUS_WIN32_WIDGETTEXTBOX_H_

template< class EventHandlerClass, class MessageMapPolicy >
class WidgetTextBox : public SmartWin::WidgetTextBox<EventHandlerClass, MessageMapPolicy> {
private:
	typedef SmartWin::WidgetTextBox<EventHandlerClass, MessageMapPolicy> BaseType;
public:
	typedef WidgetTextBox<EventHandlerClass, MessageMapPolicy>* ObjectType;

	explicit WidgetTextBox( SmartWin::Widget * parent ) : SmartWin::Widget(parent), BaseType(parent) { }

	POINT getContextMenuPos() {
		RECT rc;
		::GetClientRect(this->handle(), &rc);
		POINT pt = { rc.right/2, rc.bottom/2};
		this->clientToScreen(pt);
		return pt;
	}
	
	int charFromPos(POINT pt) {		
		LPARAM lp = MAKELPARAM(pt.x, pt.y);
		return LOWORD(::SendMessage(this->handle(), EM_CHARFROMPOS, 0, lp));
	}
	
	int lineFromPos(POINT pt) {
		LPARAM lp = MAKELPARAM(pt.x, pt.y);
		return HIWORD(::SendMessage(this->handle(), EM_CHARFROMPOS, 0, lp));
	}
	
	int lineIndex(int line) {
		return static_cast<int>(::SendMessage(this->handle(), EM_LINEINDEX, static_cast<WPARAM>(line), 0));
	}
	
	int lineLength(int c) {
		return static_cast<int>(::SendMessage(this->handle(), EM_LINELENGTH, static_cast<WPARAM>(c), 0));
	}
	
	tstring getLine(int line) {
		tstring tmp;
		tmp.resize(std::max(2, lineLength(lineIndex(line))));
		
		*reinterpret_cast<WORD*>(&tmp[0]) = static_cast<WORD>(tmp.size());
		tmp.resize(::SendMessage(this->handle(), EM_GETLINE, static_cast<WPARAM>(line), reinterpret_cast<LPARAM>(&tmp[0])));
		return tmp;
	}

	tstring textUnderCursor(POINT& p) {
		int i = charFromPos(p);
		int line = lineFromPos(p);
		int c = i - lineIndex(line);
		
		tstring tmp = getLine(line);
		
		string::size_type start = tmp.find_last_of(_T(" <\t\r\n"), c);
		if(start == string::npos)
			start = 0;
		else
			start++;
		
		string::size_type end = tmp.find_first_of(_T(" >\t\r\n"), start + 1);
		if(end == string::npos)
			end = tmp.size();
		
		return tmp.substr(start, end-start);
	}
	
};

#endif /*WIDGETTEXTBOX_H_*/
