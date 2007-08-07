#ifndef MDITAB_H_
#define MDITAB_H_

class MainWindow;

class MDITab : 
	public SmartWin::WidgetTabSheet
{
public:
	enum { MAX_TITLE_LENGTH = 20 };
	typedef SmartWin::WidgetTabSheet BaseType;
	typedef MDITab ThisType;
	typedef ThisType* ObjectType;

	static MDITab* getInstance() { return instance; }
	
	void addTab(SmartWin::WidgetMDIChild* w, const SmartWin::IconPtr& icon);
	/** Mark tab until it's selected */
	void markTab(SmartWin::WidgetMDIChild* w);
	void removeTab(SmartWin::WidgetMDIChild* w);
	void onTabContextMenu(SmartWin::WidgetMDIChild* w, const std::tr1::function<bool (const SmartWin::Point& pt)>& f);
	
	virtual void create( const Seed & cs = getDefaultSeed() );
	
	void onResized(const std::tr1::function<void (const SmartWin::Rectangle&)>& f_ ) { resized = f_; }
private:
	friend class MainWindow;
	friend class SmartWin::WidgetCreator<MDITab>;
	
	typedef std::list<HWND> WindowList;
	typedef WindowList::iterator WindowIter;
	WindowList viewOrder;
	SmartWin::Rectangle clientSize;
	
	std::vector<SmartWin::IconPtr> icons;

	std::tr1::function<void (const SmartWin::Rectangle&)> resized;
	bool inTab;
	SmartWin::WidgetMDIParent::ObjectType mdi;
	
	int findTab(SmartWin::WidgetMDIChild* w);
	void layout();
	
	MDITab(SmartWin::Widget* parent);
	~MDITab();
	
	bool handleTextChanging(SmartWin::WidgetMDIChild* w, const SmartUtil::tstring& newText);
	void handleSelectionChanged(size_t i);
	LRESULT handleMdiActivate(SmartWin::WidgetMDIChild* w, WPARAM wParam, LPARAM lParam);
	void handleNext(bool reverse);
	LRESULT handleContextMenu(WPARAM wParam, LPARAM lParam);
	
	void setTop(HWND wnd);
	
	static LRESULT CALLBACK keyboardProc(int code, WPARAM wParam, LPARAM lParam);
	static HHOOK hook;

	tstring cutTitle(const tstring& title);
	static MDITab* instance;
};

#endif /*MDITAB_H_*/
