#ifndef ASPECTCOLLECTION_H_
#define ASPECTCOLLECTION_H_

/** A control that holds a collection of items, such as a list or tree */
template<typename WidgetType, typename IndexType>
class AspectCollection {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	const WidgetType& W() const { return *static_cast<const WidgetType*>(this); }
public:
	/** Erase a particular item */
	void erase(IndexType i) {
		W().eraseImpl(i);
	}
	
	/** Erase all items from collection */
	void clear() {
		W().clearImpl();
	}
	
	/** Return number of items in collection */
	size_t size() const {
		return W().sizeImpl();
	}
	
	bool empty() const {
		return size() == 0;
	}
	
};

#endif /*ASPECTCOLLECTION_H_*/
