#ifndef TABLELAYOUT_H_
#define TABLELAYOUT_H_

class TableLayout {
public:
	enum Options {
		EXPAND = 1,
		FILL = 2
	};
	
	TableLayout(int cols_, int rows_);
	
	// it'd be nice if we could pass aspectsizable
	void add(SmartWin::Widget* widget, const SmartWin::Point wantedSize, size_t x, size_t y, size_t xspan, size_t yspan, int xoptions, int yoptions);
	void resize(const SmartWin::Rectangle& clientRect);
	
private:

	int cols;
	int rows;
	
	long resetSizes(long (SmartWin::Point::*x));
	long expand(long used, long total, long (SmartWin::Point::*x));
	
	void fill();
	void updateWidgets();
	
	struct WidgetInfo {
		WidgetInfo() : widget(0), wantedSize(0, 0), x(0), y(0), xspan(0), yspan(0), xoptions(0), yoptions(0) { }
		WidgetInfo(SmartWin::Widget* widget_, const SmartWin::Point& wantedSize_, size_t x_, size_t y_, size_t xspan_, size_t yspan_, int xoptions_, int yoptions_) : widget(widget_), wantedSize(wantedSize_), x(x_), y(y_), xspan(xspan_), yspan(yspan_), xoptions(xoptions_), yoptions(yoptions_) { } 
		SmartWin::Widget* widget;
		SmartWin::Point wantedSize;
		size_t x;
		size_t y;
		size_t xspan;
		size_t yspan;
		int xoptions;
		int yoptions;
	};
	
	struct CellInfo {
		CellInfo() : size(0), expand(false) { }
		long size;
		bool expand;
	};
	
	typedef std::vector<CellInfo> CellList;
	typedef std::vector<WidgetInfo> WidgetList;
	
	WidgetList widgets;
};

#endif /*TABLELAYOUT_H_*/
