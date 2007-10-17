#ifndef WIDGETTABVIEW_H_
#define WIDGETTABVIEW_H_

#include "WidgetTabSheet.h"
#include <list>

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

	class Seed : public SmartWin::Seed {
	public:
		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
		
	};
	
	/// Default values for creation
	static const Seed & getDefaultSeed();
	
	template<typename T>
	void add(T* w, const IconPtr& icon) {
		addWidget(w, icon, w->getText(), w->getVisible());
		w->onTextChanging(std::tr1::bind(&WidgetTabView::handleTextChanging, this, w, _1));
	}

	void mark(Widget* w);
	
	void remove(Widget* w);
	
	void next(bool reverse = false);
	
	Widget* getActive();
	void setActive(Widget* w) { setActive(findTab(w)); }
	
	void onTabContextMenu(Widget* w, const std::tr1::function<bool (const ScreenCoordinate& pt)>& f);

	bool filter(const MSG& msg);
	
	WidgetTabSheet::ObjectType getTab();

	virtual bool tryFire(const MSG& msg, LRESULT& retVal);
	
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	friend class WidgetCreator<WidgetTabView>;
	
	explicit WidgetTabView(Widget* parent);
	
	virtual ~WidgetTabView() { }

private:
	enum { MAX_TITLE_LENGTH = 20 };
	struct TabInfo {
		TabInfo(Widget* w_) : w(w_) { }
		Widget* w;
		std::tr1::function<bool (const ScreenCoordinate& pt)> handleContextMenu;
	};
	
	WidgetTabSheet::ObjectType tab;

	bool inTab;
	
	typedef std::list<Widget*> WindowList;
	typedef WindowList::iterator WindowIter;
	WindowList viewOrder;
	Rectangle clientSize;
	std::vector<IconPtr> icons;
	int active;

	int findTab(Widget* w);
	
	void setActive(int i);
	TabInfo* getTabInfo(Widget* w);
	TabInfo* getTabInfo(int i);
	
	void setTop(Widget* w);

	bool handleTextChanging(Widget* w, const SmartUtil::tstring& newText);
	bool handleSized(const WidgetSizedEventResult&);
	void handleTabSelected();
	bool handleContextMenu(SmartWin::ScreenCoordinate pt);
	
	SmartUtil::tstring cutTitle(const SmartUtil::tstring& title);
	void layout();
	
	int addIcon(const IconPtr& icon);
	void addWidget(Widget* w, const IconPtr& icon, const SmartUtil::tstring& title, bool visible);
	void swapWidgets(Widget* oldW, Widget* newW);
};

inline WidgetTabView::Seed::Seed()
{
	* this = WidgetTabView::getDefaultSeed();
}

inline WidgetTabSheet::ObjectType WidgetTabView::getTab()
{
	return tab;
}

}
#endif /*WIDGETTABVIEW_H_*/
