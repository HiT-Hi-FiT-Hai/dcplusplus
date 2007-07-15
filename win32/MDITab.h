#ifndef MDITAB_H_
#define MDITAB_H_

class MainWindow;

class MDITab : 
	public SmartWin::WidgetTabSheet<MDITab>
{
public:
	typedef SmartWin::WidgetTabSheet<MDITab> BaseType;
	typedef MDITab ThisType;
	typedef ThisType* ObjectType;

	static MDITab* getInstance() { return instance; }
	
	template<typename T>
	void addTab(T* w) {
		this->addPage(w->getText(), this->size(), reinterpret_cast<LPARAM>(static_cast<SmartWin::Widget*>(w)));
		w->onTextChanging(std::tr1::bind(&MDITab::handleTextChanging, this, static_cast<SmartWin::Widget*>(w), _1));
		if(resized)
			resized();
	}
	void removeTab(SmartWin::Widget* w);
	
	virtual void create( const Seed & cs = getDefaultSeed() );
	
	void onResized(const std::tr1::function<void ()>& f_ ) { resized = f_; }
private:
	friend class MainWindow;
	friend class SmartWin::WidgetCreator<MDITab>;
	
	int findTab(SmartWin::Widget* w);
	
	MDITab(SmartWin::Widget* parent);
	~MDITab();
	
	bool handleTextChanging(SmartWin::Widget* w, const SmartUtil::tstring& newText);
	bool handleSelectionChanging(size_t i);

	std::tr1::function<void ()> resized;
	
	static MDITab* instance;
};

#endif /*MDITAB_H_*/
