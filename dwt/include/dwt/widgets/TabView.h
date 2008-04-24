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

#ifndef DWT_WIDGETTABVIEW_H_
#define DWT_WIDGETTABVIEW_H_

#include "../resources/ImageList.h"
#include "../Rectangle.h"
#include "../aspects/AspectCollection.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectText.h"
#include "Control.h"

#include <list>
#include <vector>

namespace dwt {
/** 
 * A container that keeps widgets in tabs and handles switching etc
 */
class TabView :
	public CommonControl,
	// Aspects
	private AspectCollection<TabView, int>,
	public AspectKeyboard< TabView >,
	public AspectFont< TabView >,
	public AspectPainting< TabView >,
	public AspectSelection< TabView, int >,
	public AspectText< TabView >
{
	typedef CommonControl BaseType;
	friend class AspectCollection<TabView, int>;
	friend class AspectSelection<TabView, int>;
	friend class WidgetCreator< TabView >;
	typedef std::tr1::function<void (const tstring&)> TitleChangedFunction;
	typedef std::tr1::function<bool (const ScreenCoordinate&)> ContextMenuFunction;

public:
	/// Class type
	typedef TabView ThisType;

	/// Object type
	typedef ThisType* ObjectType;
	
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		FontPtr font;

		bool toggleActive;

		/// Fills with default parameters
		Seed(bool toggleActive_ = false);
	};

	void add(ContainerPtr w, const IconPtr& icon = IconPtr());

	void mark(ContainerPtr w);
	
	void remove(ContainerPtr w);
	
	void next(bool reverse = false);
	
	ContainerPtr getActive();
	void setActive(ContainerPtr w) { setActive(findTab(w)); }

	void setTabIcon(ContainerPtr w, const IconPtr& icon);

	tstring getTabText(ContainerPtr w);

	void onTitleChanged(const TitleChangedFunction& f) {
		titleChangedFunction = f;
	}

	void onTabContextMenu(ContainerPtr w, const ContextMenuFunction& f);

	bool filter(const MSG& msg);
	
	const Rectangle& getClientSize() const { return clientSize; }
	
	void create( const Seed & cs = Seed() );

	virtual bool tryFire( const MSG & msg, LRESULT & retVal );

protected:
	explicit TabView(Widget* parent);
	
	virtual ~TabView() { }

private:
	enum { MAX_TITLE_LENGTH = 20 };

	struct TabInfo {
		TabInfo(ContainerPtr w_) : w(w_) { }
		ContainerPtr w;
		ContextMenuFunction handleContextMenu;
	};
	
	ToolTipPtr tip;

	TitleChangedFunction titleChangedFunction;

	bool toggleActive;

	bool inTab;
	
	typedef std::list<ContainerPtr> WindowList;
	typedef WindowList::iterator WindowIter;
	WindowList viewOrder;
	Rectangle clientSize;
	ImageListPtr imageList;
	std::vector<IconPtr> icons;
	int active;
	ContainerPtr dragging;
	tstring tipText;
	
	int findTab(ContainerPtr w);
	
	void setActive(int i);
	TabInfo* getTabInfo(ContainerPtr w);
	TabInfo* getTabInfo(int i);
	
	void setTop(ContainerPtr w);

	bool handleTextChanging(ContainerPtr w, const tstring& newText);
	void handleSized(const SizedEvent&);
	void handleTabSelected();
	LRESULT handleToolTip(LPARAM lParam);
	void handleLeftMouseDown(const MouseEvent& mouseEventResult);
	void handleLeftMouseUp(const MouseEvent& mouseEventResult);
	bool handleContextMenu(dwt::ScreenCoordinate pt);
	void handleMiddleMouseDown(const MouseEvent& mouseEventResult);
	
	tstring formatTitle(tstring title);
	void layout();
	
	int addIcon(const IconPtr& icon);
	void swapWidgets(ContainerPtr oldW, ContainerPtr newW);
	
	tstring getText(unsigned idx) const;
	
	void setText(unsigned idx, const tstring& text);

	// AspectCollection
	void eraseImpl( int row );
	void clearImpl();
	size_t sizeImpl() const;

	// AspectHelp
	void helpImpl(unsigned& id);

	// AspectSelection
	int getSelectedImpl() const;
	void setSelectedImpl( int idx );
	// AspectSelection expectation implementation
	static Message getSelectionChangedMessage();

	const ImageListPtr& getImageList() const;
	
	int hitTest(const ScreenCoordinate& pt);
	
	/// Get the area not used by the tabs
	/** This function should be used after adding the pages, so that the area not used by
	  * the tabs can be calculated accurately. It returns coordinates respect to the
	  * TabControl, this is, you have to adjust for the position of the control itself.   
	  */
	Rectangle getUsableArea(bool cutBorders = false) const;

};

inline Message TabView::getSelectionChangedMessage() {
	return Message( WM_NOTIFY, TCN_SELCHANGE );
}

inline void TabView::eraseImpl(int i) {
	TabCtrl_DeleteItem(this->handle(), i);
}

inline void TabView::clearImpl() {
	TabCtrl_DeleteAllItems(handle());
}

inline size_t TabView::sizeImpl() const {
	return static_cast<size_t>(TabCtrl_GetItemCount(this->handle()));
}

inline void TabView::setSelectedImpl( int idx ) {
	TabCtrl_SetCurSel( this->handle(), idx );
}

inline int TabView::getSelectedImpl() const {
	return TabCtrl_GetCurSel( this->handle() );
}
	
}
#endif /*WIDGETTABVIEW_H_*/
