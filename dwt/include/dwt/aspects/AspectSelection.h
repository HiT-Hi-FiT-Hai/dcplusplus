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

#ifndef AspectSelection_h
#define AspectSelection_h

#include "../Dispatchers.h"

namespace dwt {

/// Aspect class used by Widgets that have the possibility of being "selecting"
/// item(s).
/** \ingroup AspectClasses
  * E.g. the ComboBox have a "selected" Aspect therefore it realizes the
  * AspectSelection through inheritance.
  */
template< class WidgetType, typename IndexType >
class AspectSelection {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	const WidgetType& W() const { return *static_cast<const WidgetType*>(this); }

	typedef Dispatchers::VoidVoid<> Dispatcher;
public:
	/// \ingroup EventHandlersAspectSelection
	/// Setting the event handler for the "selection changed" event
	/** This event will be raised when the selected property of the Widget have
	  * changed either due to user interaction or due to some other reason. <br>
	  * No parameters are passed.
	  */
	void onSelectionChanged(const typename Dispatcher::F& f) {
		W().addCallback(WidgetType::getSelectionChangedMessage(), Dispatcher(f));
	}

	/// Sets the selected index of the Widget
	/** The idx parameter is the (zero indexed) value of the item to set as the
	  * selected item.         You must add the items before you set the selected
	  * index.
	  */
	void setSelected( IndexType item );

	/// Return the selected index of the Widget
	/** The return value is the selected items index of the Widget, if no item is
	  * selected the return value will be -1. <br>
	  * Note! <br>
	  * Some Widgets have the possibillity of selecting multiple items, if so you
	  * should not use this function but rather the multiple selection value getter.
	  */
	IndexType getSelected() const;
	
	size_t countSelected() const;
	
	bool hasSelected() const;

protected:
	virtual ~AspectSelection() { }
};

template< class WidgetType, typename IndexType >
IndexType AspectSelection<WidgetType, IndexType>::getSelected() const {
	return W().getSelectedImpl();
}

template< class WidgetType, typename IndexType >
void AspectSelection<WidgetType, IndexType>::setSelected(IndexType item) {
	W().setSelectedImpl(item);
}

template< class WidgetType, typename IndexType >
size_t AspectSelection<WidgetType, IndexType>::countSelected() const {
	return W().countSelectedImpl();
}

template< class WidgetType, typename IndexType >
bool AspectSelection<WidgetType, IndexType>::hasSelected() const {
	return countSelected() > 0;
}

}

#endif
