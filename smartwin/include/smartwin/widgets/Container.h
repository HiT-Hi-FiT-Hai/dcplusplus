#ifndef CONTAINER_H_
#define CONTAINER_H_

#include "../Policies.h"
#include "Composite.h"

namespace SmartWin {

class Container :
	public Composite<Policies::Normal>
{
	friend class WidgetCreator<Container>;
public:
	typedef Container ThisType;
	
	typedef ThisType* ObjectType;
	
	typedef Composite<Policies::Normal> BaseType;

	struct Seed : public BaseType::Seed {
		Seed();
	};

	// Use our seed type
	virtual void create( const Seed& cs = Seed() );

protected:
	Container(Widget* parent) : BaseType(parent) { }
};

inline Container::Seed::Seed() : BaseType::Seed(WS_CHILD | WS_CLIPSIBLINGS) {
	
}

inline void Container::create(const Seed& cs) {
	BaseType::create(cs);
}
}
#endif /*CONTAINER_H_*/
