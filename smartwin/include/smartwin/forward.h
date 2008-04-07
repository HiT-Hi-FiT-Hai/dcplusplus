#ifndef FORWARD_H_
#define FORWARD_H_

namespace SmartWin {

template< class WidgetType >
class WidgetCreator;

class Button;
typedef Button* ButtonPtr;

class CheckBox;
typedef CheckBox* CheckBoxPtr;

class ComboBox;
typedef ComboBox* ComboBoxPtr;

class Container;
typedef Container* ContainerPtr;

class CoolBar;
typedef CoolBar* CoolBarPtr;

class DateTime;
typedef DateTime* DateTimePtr;

class GroupBox;
typedef GroupBox* GroupBoxPtr;

class Label;
typedef Label* LabelPtr;

class MDIChild;
typedef MDIChild* MDIChildPtr;

class MDIFrame;
typedef MDIFrame* MDIFramePtr;

class MDIParent;
typedef MDIParent* MDIParentPtr;

class ProgressBar;
typedef ProgressBar* ProgressBarPtr;

class RadioButton;
typedef RadioButton* RadioButtonPtr;

class Spinner;
typedef Spinner* SpinnerPtr;

class Slider;
typedef Slider* SliderPtr;

class Table;
typedef Table* TablePtr;

class TabSheet;
typedef TabSheet* TabSheetPtr;

class ToolBar;
typedef ToolBar* ToolBarPtr;

class Tree;
typedef Tree* TreePtr;

class Window;
typedef Window* WindowPtr;

}
#endif /*FORWARD_H_*/
