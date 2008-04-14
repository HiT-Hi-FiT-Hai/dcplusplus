/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
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

#include <dwt/widgets/TextBox.h>

namespace dwt {

TextBox::Seed::Seed(const tstring& caption) : 
	BaseType::Seed(WC_EDIT, WS_CHILD | WS_TABSTOP, WS_EX_CLIENTEDGE, caption),
	font(new Font(DefaultGuiFont))
{
}

void TextBox::create( const Seed & cs ) {
	BaseType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

tstring TextBox::getLine(int line) {
	tstring tmp;
	tmp.resize(std::max(2, lineLength(lineIndex(line))));
	
	*reinterpret_cast<WORD*>(&tmp[0]) = static_cast<WORD>(tmp.size());
	tmp.resize(::SendMessage(this->handle(), EM_GETLINE, static_cast<WPARAM>(line), reinterpret_cast<LPARAM>(&tmp[0])));
	return tmp;
}

tstring TextBox::textUnderCursor(const ScreenCoordinate& p) {
	int i = charFromPos(p);
	int line = lineFromPos(p);
	int c = (i - lineIndex(line)) & 0xFFFF;
	
	tstring tmp = getLine(line);
	
	tstring::size_type start = tmp.find_last_of(_T(" <\t\r\n"), c);
	if(start == tstring::npos)
		start = 0;
	else
		start++;
	
	tstring::size_type end = tmp.find_first_of(_T(" >\t\r\n"), start + 1);
	if(end == tstring::npos)
		end = tmp.size();
	
	return tmp.substr(start, end-start);
}

tstring TextBoxBase::getSelection() const
{
	DWORD start, end;
	this->sendMessage( EM_GETSEL, reinterpret_cast< WPARAM >( & start ), reinterpret_cast< LPARAM >( & end ) );
	tstring retVal = this->getText().substr( start, end - start );
	return retVal;
}

ScreenCoordinate TextBox::getContextMenuPos() {
	RECT rc;
	::GetClientRect(this->handle(), &rc);
	return ClientCoordinate (Point(rc.right/2, rc.bottom/2), this);
}

}
