#ifndef ASPECTDATA_H_
#define ASPECTDATA_H_

/** A Widget that associates some sort of data with each item */
template<typename WidgetType, typename IndexType>
class AspectData {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
public:
	IndexType findData(LPARAM data) {
		return W().findDataImpl(data);
	}
	
	IndexType findData(LPARAM data, IndexType start) {
		return W().findDataImpl(data, start);
	}
	
	LPARAM getData(IndexType i) {
		return W().getDataImpl(i);
	}
	
	void setData(IndexType i, LPARAM data) {
		return W().setDataImpl(i, data);
	}
	
	LPARAM operator[](IndexType i) {
		return getData(i);
	}
};

#endif /*ASPECTDATA_H_*/
