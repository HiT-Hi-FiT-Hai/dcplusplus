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
		SmartWin::Widget* widget = static_cast<SmartWin::Widget*>(w);
		size_t tabs = this->size();
		this->addPage(w->getText(), tabs, reinterpret_cast<LPARAM>(static_cast<SmartWin::Widget*>(w)));

		if(w->getParent()->sendMessage(WM_MDIGETACTIVE) == reinterpret_cast<LPARAM>(widget->handle())) {
			activating = true;
			this->setSelectedIndex(tabs);
		}
		w->onTextChanging(std::tr1::bind(&MDITab::handleTextChanging, this, static_cast<SmartWin::Widget*>(w), _1));
		w->onRaw(std::tr1::bind(&MDITab::handleMdiActivate, this, static_cast<SmartWin::Widget*>(w), _1, _2), SmartWin::Message(WM_MDIACTIVATE));

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
	void handleSelectionChanged(size_t i);
	HRESULT handleMdiActivate(SmartWin::Widget* w, WPARAM wParam, LPARAM lParam);
	
	std::tr1::function<void ()> resized;
	bool activating;
	
	static MDITab* instance;
};

#endif /*MDITAB_H_*/
