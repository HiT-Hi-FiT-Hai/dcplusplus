#ifndef FORWARD_H_
#define FORWARD_H_

namespace SmartWin {

template< class WidgetType >
class WidgetCreator;

class Button;
typedef Button* ButtonPtr;

class CheckBox;
typedef CheckBox* CheckBoxPtr;

class DateTime;
typedef DateTime* DateTimePtr;

class Label;
typedef Label* LabelPtr;

class MDIChild;
typedef MDIChild* MDIChildPtr;

class MDIFrame;
typedef MDIFrame* MDIFramePtr;

class MDIParent;
typedef MDIParent* MDIParentPtr;

class RadioButton;
typedef RadioButton* RadioButtonPtr;

class Spinner;
typedef Spinner* SpinnerPtr;

class Slider;
typedef Slider* SliderPtr;

class Table;
typedef Table* TablePtr;

class Tree;
typedef Tree* TreePtr;


}
#endif /*FORWARD_H_*/
