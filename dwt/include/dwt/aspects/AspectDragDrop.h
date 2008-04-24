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

#ifndef DWT_AspectDragDrop_h
#define DWT_AspectDragDrop_h

#ifndef WINCE // Not supported on WINCE platform

#include "../Message.h"
#include "../Point.h"
#include <vector>
#include <shellapi.h>

namespace dwt {

/// Aspect class used by dialog Widgets that have the possibility of trapping "drop files events".
/** \ingroup AspectClasses
  * E.g. the ModalDialog can trap "drop files events" therefore they realize the AspectDragDrop through inheritance.
  */
template< class WidgetType >
class AspectDragDrop
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	const WidgetType& W() const { return *static_cast<const WidgetType*>(this); }

	HWND H() const { return W().handle(); }

	struct Dispatcher {
		typedef std::tr1::function<void (std::vector< tstring>, Point )> F;

		Dispatcher(const F& f_) : f(f_) { }
		
		bool operator()(const MSG& msg, LRESULT& ret) {
			std::vector<tstring> files;
			Point pt;
			HDROP handle = (HDROP)msg.wParam;
			if (handle) { 
				int iFiles = DragQueryFile(handle, (UINT)-1, NULL, 0);
				TCHAR pFilename[MAX_PATH];
				for(int i=0;i<iFiles;i++) {
					memset(pFilename,0,MAX_PATH * sizeof(TCHAR));
					DragQueryFile(handle, i, pFilename, MAX_PATH);
					files.push_back(pFilename);
				}
				POINT p;
				DragQueryPoint(handle,&p);
				pt = Point(p.x,p.y);
				DragFinish(handle); 
			}
			handle = 0;
			f(files, pt);

			return false;
		}
		
		F f;
	};

public:
	/// \ingroup EventHandlersAspectAspectDragDrop
	/// Setting the event handler for the "drop files" event
	/** If supplied event handler is called when a file is dropped over the widget.
	  * The function would receive a vector with all file paths and the coordinats where the files were dropped <br>
	  *
	  * Example:
	  *
	  * void DropFile(std::vector<tstring> files, Point droppoint) {
	  * 	tstring path = files.at(0);
	  * 	setText(path); 
	  * 	int x = droppoint.x;
	  * 	int y = droppoint.y;
	  * }
	  */
	void onDragDrop(const typename Dispatcher::F& f) {
		W().addCallback(Message( WM_DROPFILES ), Dispatcher(f));
	}
	/// Setup Drag & Drop for this dialog 
	/** This setup the ability to receive an WM_DROPFILES msg if you drop a file on dialog 
	*/ 
	void setDragAcceptFiles(bool accept = true); 

protected:
	virtual ~AspectDragDrop()
	{}
};


template< class WidgetType >
void AspectDragDrop< WidgetType >::setDragAcceptFiles(bool accept)
{
	DragAcceptFiles(H(), accept); 
}

}

#else

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectDragDrop { };

#endif

#endif
