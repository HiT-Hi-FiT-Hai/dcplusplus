#ifndef SPLASHWINDOW_H_
#define SPLASHWINDOW_H_

class SplashWindow : public SmartWin::WidgetFactory<SmartWin::WidgetWindow>  {
public:
	SplashWindow();
	~SplashWindow();
	void operator()(const string& str);
private:
	SmartWin::WidgetWindow* tmp;
	TextBoxPtr text;
};

#endif /*SPLASHWINDOW_H_*/
