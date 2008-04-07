#ifndef WIDGETTABVIEW_H_
#define WIDGETTABVIEW_H_

#include "../WindowClass.h"
#include "../Policies.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectMouse.h"
#include <list>
#include <vector>

namespace SmartWin {
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
	void handleLeftMouseDown(const MouseEventResult& mouseEventResult);
	void handleLeftMouseUp(const MouseEventResult& mouseEventResult);
	bool handleContextMenu(SmartWin::ScreenCoordinate pt);
	void handleMiddleMouseDown(const MouseEventResult& mouseEventResult);
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
