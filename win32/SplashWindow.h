#ifndef SPLASHWINDOW_H_
#define SPLASHWINDOW_H_

class SplashWindow : public SmartWin::WidgetFactory<SmartWin::WidgetWindow>  {
public:
	SplashWindow();
	
	void operator()(const string& str);
private:
	WidgetTextBoxPtr text;
};

#endif /*SPLASHWINDOW_H_*/
