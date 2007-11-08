#ifndef ASPECTDATA_H_
#define ASPECTDATA_H_

/** A Widget that associates some sort of data with each item */
template<typename WidgetType, typename IndexType>
class AspectData {
public:
	IndexType findData(LPARAM data) {
		return static_cast<WidgetType*>(this)->findDataImpl(data);
	}
	
	IndexType findData(LPARAM data, IndexType start) {
		return static_cast<WidgetType*>(this)->findDataImpl(data, start);
	}
	
	LPARAM getData(IndexType i) {
		return static_cast<WidgetType*>(this)->getDataImpl(i);
	}
	
	void setData(IndexType i, LPARAM data) {
		return static_cast<WidgetType*>(this)->setDataImpl(i, data);
	}
	
	LPARAM operator[](IndexType i) {
		return getData(i);
	}
};

#endif /*ASPECTDATA_H_*/
