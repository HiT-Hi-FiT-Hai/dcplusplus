/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
        may be used to endorse or promote products derived from this software 
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../include/smartwin/Point.h"
#include "../include/smartwin/Widget.h"

namespace SmartWin
{
// begin namespace SmartWin

void Point::maxOf( const Point & p ) {
	if ( p.x > x )
		x = p.x;
	if ( p.y > y )
		y = p.y;
}

void Point::minOf( const Point & p ) {
	if ( p.x < x )
		x = p.x;
	if ( p.y < y )
		y = p.y;
}

Point & operator += ( Point & lhs, const Point & rhs ) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

Point operator +( const Point & lhs, const Point & rhs ) {
	Point retVal = lhs;
	retVal += rhs;
	return retVal;
}

Point & operator -= ( Point & lhs, const Point & rhs ) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

Point operator -( const Point & lhs, const Point & rhs ) {
	Point retVal = lhs;
	retVal -= rhs;
	return retVal;
}

ClientCoordinate::ClientCoordinate(const ClientCoordinate& cc, Widget* w_) : point(cc.getPoint()), w(w_) {
	if(cc.w != w) {
		::MapWindowPoints(cc.w->handle(), w->handle(), &point, 1);
	}
}

ClientCoordinate::ClientCoordinate(const ScreenCoordinate& sc, Widget* w_) : point(sc.getPoint()), w(w_) {
	::ScreenToClient(w->handle(), &point);
}

ClientCoordinate::operator ScreenCoordinate() const {
	ScreenCoordinate pt(getPoint());
	::ClientToScreen(w->handle(), &pt.getPoint());
	return pt;
}

// end namespace SmartWin
}
