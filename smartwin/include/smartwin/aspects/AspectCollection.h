#ifndef ASPECTCOLLECTION_H_
#define ASPECTCOLLECTION_H_

/** A control that holds a collection of items, such as a list or tree */
template<typename WidgetType, typename IndexType>
class AspectCollection {
public:
	/** Erase a particular item */
	void erase(IndexType i) {
		static_cast<WidgetType*>(this)->eraseImpl(i);
	}
	
	/** Erase all items from collection */
	void clear() {
		static_cast<WidgetType*>(this)->clearImpl();
	}
	
	/** Return number of items in collection */
	size_t size() const {
		return static_cast<const WidgetType*>(this)->sizeImpl();
	}
	
	bool empty() const {
		return size() == 0;
	}
	
};

#endif /*ASPECTCOLLECTION_H_*/
