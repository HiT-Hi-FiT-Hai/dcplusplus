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

#ifndef WIDGETTABVIEW_H_
#define WIDGETTABVIEW_H_

#include "../WindowClass.h"
#include "../Policies.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectMouse.h"
#include <list>
#include <vector>

namespace dwt {
/** 
 * A container that keeps widgets in tabs and handles switching etc
 */
class TabView :
	public MessageMap< Policies::Normal >,

	public AspectRaw<TabView>,
	public AspectSizable<TabView>
{
	typedef std::tr1::function<void (const SmartUtil::tstring&)> TitleChangedFunction;
	typedef std::tr1::function<void (HWND, unsigned)> HelpFunction;
	typedef std::tr1::function<bool (const ScreenCoordinate&)> ContextMenuFunction;

public:
	/// Class type
	typedef TabView ThisType;

	/// Object type
	typedef ThisType* ObjectType;
	
	typedef MessageMap<Policies::Normal> BaseType;

	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		bool toggleActive;

		/// Fills with default parameters
		Seed(bool toggleActive_ = false);
	};

	void add(ContainerPtr w, const IconPtr& icon);

	void mark(ContainerPtr w);
	
	void remove(ContainerPtr w);
	
	void next(bool reverse = false);
	
	ContainerPtr getActive();
	void setActive(ContainerPtr w) { setActive(findTab(w)); }

	SmartUtil::tstring getTabText(ContainerPtr w);

	void onTitleChanged(const TitleChangedFunction& f) {
		titleChangedFunction = f;
	}

	void onTabContextMenu(ContainerPtr w, const ContextMenuFunction& f);

	void onHelp(const HelpFunction& f) {
		helpFunction = f;
	}

	bool filter(const MSG& msg);
	
	TabSheetPtr getTab();

	const Rectangle& getClientSize() const { return clientSize; }
	
	void create( const Seed & cs = Seed() );

protected:
	friend class WidgetCreator<TabView>;
	
	explicit TabView(Widget* parent);
	
	virtual ~TabView() { }

private:
	enum { MAX_TITLE_LENGTH = 20 };

	struct TabInfo {
		TabInfo(ContainerPtr w_) : w(w_) { }
		ContainerPtr w;
		ContextMenuFunction handleContextMenu;
	};
	
	static WindowClass windowClass;
	
	TabSheetPtr tab;
	ToolTipPtr tip;

	TitleChangedFunction titleChangedFunction;
	HelpFunction helpFunction;

	bool toggleActive;

	bool inTab;
	
	typedef std::list<ContainerPtr> WindowList;
	typedef WindowList::iterator WindowIter;
	WindowList viewOrder;
	Rectangle clientSize;
	std::vector<IconPtr> icons;
	int active;
	ContainerPtr dragging;
	SmartUtil::tstring tipText;
	
	int findTab(ContainerPtr w);
	
	void setActive(int i);
	TabInfo* getTabInfo(ContainerPtr w);
	TabInfo* getTabInfo(int i);
	
	void setTop(ContainerPtr w);

	bool handleTextChanging(ContainerPtr w, const SmartUtil::tstring& newText);
	void handleSized(const SizedEvent&);
	void handleTabSelected();
	LRESULT handleToolTip(LPARAM lParam);
	void handleLeftMouseDown(const MouseEvent& mouseEventResult);
	void handleLeftMouseUp(const MouseEvent& mouseEventResult);
	bool handleContextMenu(dwt::ScreenCoordinate pt);
	void handleMiddleMouseDown(const MouseEvent& mouseEventResult);
	void handleHelp(HWND hWnd, unsigned id);
	
	SmartUtil::tstring formatTitle(SmartUtil::tstring title);
	void layout();
	
	int addIcon(const IconPtr& icon);
	void swapWidgets(ContainerPtr oldW, ContainerPtr newW);
};

inline TabSheetPtr TabView::getTab() {
	return tab;
}

}
#endif /*WIDGETTABVIEW_H_*/
