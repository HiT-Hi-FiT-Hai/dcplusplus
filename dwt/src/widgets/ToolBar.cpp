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

#include <dwt/widgets/ToolBar.h>

namespace dwt {

ToolBar::Seed::Seed() :
	BaseType::Seed(TOOLBARCLASSNAME, WS_CHILD | TBSTYLE_LIST | TBSTYLE_TOOLTIPS)
{
}

void ToolBar::create( const Seed & cs )
{
	BaseType::create(cs);

	this->sendMessage(TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);

	//// Telling the toolbar what the size of TBBUTTON struct is
	this->sendMessage(TB_BUTTONSTRUCTSIZE, ( WPARAM ) sizeof( TBBUTTON ));
}

void ToolBar::appendSeparator()
{
	TBBUTTON tb = { 0 };
	tb.fsStyle = BTNS_SEP;
	if ( this->sendMessage(TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( &tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
}

void ToolBar::appendItem( int image, const tstring& toolTip, const Dispatcher::F& f)
{
	int id = -1;
	
	if(f) {
		for(id = 0; id < (int)commands.size(); ++id) {
			if(!commands[id])
				break;
		}
		if(id == (int)commands.size()) {
			commands.push_back(f);
		} else {
			commands[id] = f;
		}
	}
	
	// Adding button
	TBBUTTON tb = { 0 };
	tb.iBitmap = image;
	tb.idCommand = id;
	tb.fsState = TBSTATE_ENABLED;
	tb.fsStyle = BTNS_AUTOSIZE;
	tb.iString = reinterpret_cast<INT_PTR>(toolTip.c_str());
	if ( this->sendMessage(TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( &tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
}

bool ToolBar::tryFire( const MSG & msg, LRESULT & retVal ) {
	if(msg.message == WM_COMMAND && msg.lParam == reinterpret_cast<LPARAM>(handle())) {
		size_t id = LOWORD(msg.wParam);
		if(id < commands.size() && commands[id]) {
			commands[id]();
			return true;
		}
	}
	return PolicyType::tryFire(msg, retVal);
}

}
