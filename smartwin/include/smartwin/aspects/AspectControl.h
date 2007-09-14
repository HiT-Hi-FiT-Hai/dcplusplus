#ifndef ASPECTCONTROL_H_
#define ASPECTCONTROL_H_

namespace SmartWin {

/** This class is for all windows common controls */
template<typename WidgetType >
class AspectControl {
public:
	/// Class type
	typedef WidgetType ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	unsigned int getControlId() {
		return static_cast<unsigned int>(::GetWindowLongPtr(static_cast<WidgetType*>(this)->handle(), GWLP_ID));
	}
};

}

#endif /*ASPECTCONTROL_H_*/
