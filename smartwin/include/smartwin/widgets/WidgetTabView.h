#ifndef WIDGETTABVIEW_H_
#define WIDGETTABVIEW_H_

#include "TabSheet.h"
#include "ToolTip.h"
#include "Window.h"
#include "../WindowClass.h"
#include <list>
#include <vector>

namespace SmartWin {
/** 
 * A container that keeps widgets in tabs and handles switching etc
 */
class WidgetTabView :
	public MessageMap< Policies::Normal >,

	public AspectRaw<WidgetTabView>,
	public AspectSizable<WidgetTabView>
{
	typedef std::tr1::function<void (const SmartUtil::tstring&)> TitleChangedFunction;
	typedef std::tr1::function<void (HWND, unsigned)> HelpFunction;
	typedef std::tr1::function<bool (const ScreenCoordinate&)> ContextMenuFunction;

public:
	/// Class type
	typedef WidgetTabView ThisType;

	/// Object type
	typedef ThisType * ObjectType;
	
	typedef MessageMap<Policies::Normal> PolicyType;

	class Seed : public Widget::Seed {
	public:
		/// Fills with default parameters
		Seed();

		bool toggleActive;
	};

	void add(WidgetChildWindow* w, const IconPtr& icon);

	void mark(WidgetChildWindow* w);
	
	void remove(WidgetChildWindow* w);
	
	void next(bool reverse = false);
	
	WidgetChildWindow* getActive();
	void setActive(WidgetChildWindow* w) { setActive(findTab(w)); }

	SmartUtil::tstring getTabText(WidgetChildWindow* w);

	void onTitleChanged(const TitleChangedFunction& f) {
		titleChangedFunction = f;
	}

	void onHelp(const HelpFunction& f) {
		helpFunction = f;
	}

	void onTabContextMenu(WidgetChildWindow* w, const ContextMenuFunction& f);

	bool filter(const MSG& msg);
	
	TabSheet::ObjectType getTab();

	const Rectangle& getClientSize() const { return clientSize; }
	
	void create( const Seed & cs = Seed() );

protected:
	friend class WidgetCreator<WidgetTabView>;
	
	explicit WidgetTabView(Widget* parent);
	
	virtual ~WidgetTabView() { }

private:
	enum { MAX_TITLE_LENGTH = 20 };

	struct TabInfo {
		TabInfo(WidgetChildWindow* w_) : w(w_) { }
		WidgetChildWindow* w;
		ContextMenuFunction handleContextMenu;
	};
	
	static WindowClass windowClass;
	
	TabSheet::ObjectType tab;
	ToolTip::ObjectType tip;

	TitleChangedFunction titleChangedFunction;
	HelpFunction helpFunction;

	bool toggleActive;

	bool inTab;
	
	typedef std::list<WidgetChildWindow*> WindowList;
	typedef WindowList::iterator WindowIter;
	WindowList viewOrder;
	Rectangle clientSize;
	std::vector<IconPtr> icons;
	int active;
	WidgetChildWindow* dragging;
	SmartUtil::tstring tipText;
	
	int findTab(WidgetChildWindow* w);
	
	void setActive(int i);
	TabInfo* getTabInfo(WidgetChildWindow* w);
	TabInfo* getTabInfo(int i);
	
	void setTop(WidgetChildWindow* w);

	bool handleTextChanging(WidgetChildWindow* w, const SmartUtil::tstring& newText);
	void handleSized(const SizedEvent&);
	void handleTabSelected();
	LRESULT handleToolTip(LPARAM lParam);
	void handleLeftMouseDown(const MouseEventResult& mouseEventResult);
	void handleLeftMouseUp(const MouseEventResult& mouseEventResult);
	bool handleContextMenu(SmartWin::ScreenCoordinate pt);
	void handleMiddleMouseDown(const MouseEventResult& mouseEventResult);
	void handleHelp(HWND hWnd, unsigned id);
	
	SmartUtil::tstring formatTitle(SmartUtil::tstring title);
	void layout();
	
	int addIcon(const IconPtr& icon);
	void swapWidgets(WidgetChildWindow* oldW, WidgetChildWindow* newW);
};

inline TabSheet::ObjectType WidgetTabView::getTab()
{
	return tab;
}

}
#endif /*WIDGETTABVIEW_H_*/
