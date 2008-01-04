#ifndef WIDGETTABVIEW_H_
#define WIDGETTABVIEW_H_

#include "WidgetTabSheet.h"
#include "WidgetWindow.h"
#include "../WindowClass.h"
#include <list>
#include <vector>

namespace SmartWin {
/** 
 * A container that keeps widgets in tabs and handles switching etc
 */
class WidgetTabView :
	public MessageMapPolicy< Policies::Normal >,

	public AspectRaw<WidgetTabView>,
	public AspectSizable<WidgetTabView>
{
public:
	/// Class type
	typedef WidgetTabView ThisType;

	/// Object type
	typedef ThisType * ObjectType;
	
	typedef MessageMapPolicy<Policies::Normal> PolicyType;

	class Seed : public Widget::Seed {
	public:
		/// Fills with default parameters
		Seed();
	};

	void add(WidgetChildWindow* w, const IconPtr& icon);

	void mark(WidgetChildWindow* w);
	
	void remove(WidgetChildWindow* w);
	
	void next(bool reverse = false);
	
	WidgetChildWindow* getActive();
	void setActive(WidgetChildWindow* w) { setActive(findTab(w)); }

	void onTitleChanged(const std::tr1::function<void (const SmartUtil::tstring&)>& f) {
		titleChangedFunction = f;
	}

	void onTabContextMenu(WidgetChildWindow* w, const std::tr1::function<bool (const ScreenCoordinate& pt)>& f);

	bool filter(const MSG& msg);
	
	WidgetTabSheet::ObjectType getTab();

	virtual bool tryFire(const MSG& msg, LRESULT& retVal);
	
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
		std::tr1::function<bool (const ScreenCoordinate& pt)> handleContextMenu;
	};
	
	static WindowClass windowClass;
	
	WidgetTabSheet::ObjectType tab;

	std::tr1::function<void (const SmartUtil::tstring&)> titleChangedFunction;

	bool inTab;
	
	typedef std::list<WidgetChildWindow*> WindowList;
	typedef WindowList::iterator WindowIter;
	WindowList viewOrder;
	Rectangle clientSize;
	std::vector<IconPtr> icons;
	int active;
	int dragging;
	
	int findTab(WidgetChildWindow* w);
	
	void setActive(int i);
	TabInfo* getTabInfo(WidgetChildWindow* w);
	TabInfo* getTabInfo(int i);
	
	void setTop(WidgetChildWindow* w);

	bool handleTextChanging(WidgetChildWindow* w, const SmartUtil::tstring& newText);
	bool handleSized(const WidgetSizedEventResult&);
	void handleTabSelected();
	void handleLeftMouseDown(const MouseEventResult& mouseEventResult);
	void handleLeftMouseUp(const MouseEventResult& mouseEventResult);
	bool handleContextMenu(SmartWin::ScreenCoordinate pt);
	void handleMiddleMouseDown(const MouseEventResult& mouseEventResult);
	
	SmartUtil::tstring cutTitle(const SmartUtil::tstring& title);
	void layout();
	
	int addIcon(const IconPtr& icon);
	void swapWidgets(WidgetChildWindow* oldW, WidgetChildWindow* newW);
};

inline WidgetTabSheet::ObjectType WidgetTabView::getTab()
{
	return tab;
}

}
#endif /*WIDGETTABVIEW_H_*/
