#ifndef SPLASHWINDOW_H_
#define SPLASHWINDOW_H_

class SplashWindow : public dwt::WidgetFactory<dwt::Window>  {
public:
	SplashWindow();
	~SplashWindow();
	void operator()(const string& str);
private:
	dwt::Window* tmp;
	TextBoxPtr text;
};

#endif /*SPLASHWINDOW_H_*/
