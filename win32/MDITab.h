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
	
	void addTab(SmartWin::WidgetMDIChild::ObjectType w);
	void removeTab(SmartWin::Widget* w);
	
	virtual void create( const Seed & cs = getDefaultSeed() );
	
	void onResized(const std::tr1::function<void ()>& f_ ) { resized = f_; }
private:
	friend class MainWindow;
	friend class SmartWin::WidgetCreator<MDITab>;
	
	typedef std::list<SmartWin::Widget*> WidgetList;
	typedef WidgetList::iterator WidgetIter;
	WidgetList viewOrder;
	WidgetIter nextTab;

	std::tr1::function<void ()> resized;
	bool activating;
	SmartWin::WidgetMDIParent::ObjectType mdi;
	
	int findTab(SmartWin::Widget* w);
	
	MDITab(SmartWin::Widget* parent);
	~MDITab();
	
	bool handleTextChanging(SmartWin::Widget* w, const SmartUtil::tstring& newText);
	void handleSelectionChanged(size_t i);
	LRESULT handleMdiActivate(SmartWin::Widget* w, WPARAM wParam, LPARAM lParam);
	LRESULT handleMdiNext(WPARAM wParam, LPARAM lParam);
	
	
	tstring cutTitle(const tstring& title);
	static MDITab* instance;
};

#endif /*MDITAB_H_*/
