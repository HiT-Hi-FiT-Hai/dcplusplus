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

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

class Exception  
{
public:
	Exception(const string& aError = "") : error(aError) { };

	virtual const string& getError() const { return error; };
protected:
	string error;
};

#define STANDARD_EXCEPTION(name) class name : public Exception { \
public:\
	name(const Exception& e, const string& aError) : Exception(aError + ":" + e.getError()) { }; \
	name(const string& aError) : Exception(aError) { }; \
}

STANDARD_EXCEPTION(FileException);

#endif // _EXCEPTION_H

/**
 * @file Exception.h
 * $Id: Exception.h,v 1.2 2001/12/13 19:21:57 arnetheduck Exp $
 * @if LOG
 * $Log: Exception.h,v $
 * Revision 1.2  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

