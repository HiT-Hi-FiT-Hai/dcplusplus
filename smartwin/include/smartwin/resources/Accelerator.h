#ifndef ACCELERATOR_H_
#define ACCELERATOR_H_

#include "../Application.h"
#include "../../SmartUtil.h"
#include "Handle.h"
#include <boost/intrusive_ptr.hpp>

namespace SmartWin {

class Accelerator : public Handle<NullPolicy<HACCEL> > {
public:
	Accelerator(Widget* widget, unsigned int id);
	
	bool translate(const MSG& msg);
	
private:
	typedef Handle<NullPolicy<HACCEL> > ResourceType;
	
	Widget* widget;

};

typedef boost::intrusive_ptr< Accelerator > AcceleratorPtr;

inline Accelerator::Accelerator(Widget* widget_, unsigned id) : 
	ResourceType(::LoadAccelerators(Application::instance().getAppHandle(), MAKEINTRESOURCE(id))),
	widget(widget_)
{
	
}

inline bool Accelerator::translate(const MSG& msg) {
	if(!handle()) {
		return false;
	}
	return ::TranslateAccelerator(widget->handle(), handle(), const_cast<MSG*>(&msg)) > 0;
}

}

#endif /*ACCELERATOR_H_*/
