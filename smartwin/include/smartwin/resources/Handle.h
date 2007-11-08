#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace SmartWin {

/** Policy for all handles that have null as NULL_HANDLE */
template<typename H>
struct NullPolicy {
	typedef H HandleType;
	static const H NULL_HANDLE;
	void release(H) { }
};

template<typename H> const H NullPolicy<H>::NULL_HANDLE = NULL;

/** A policy for GDI objects (bitmap, pen, brush etc) */
template<typename H>
struct GdiPolicy : public NullPolicy<H> {
	void release(H h) { ::DeleteObject(h); }
};

template<typename Policy>
class Handle;

template<typename Policy>
void intrusive_ptr_add_ref(Handle<Policy>* resource);

template<typename Policy>
void intrusive_ptr_release(Handle<Policy>* resource);

/**
 * Ref-counted base class for resources - nothing stops you from using this one the stack as well
 */
template<typename Policy>
class Handle : protected Policy, public boost::noncopyable {
public:
	typedef typename Policy::HandleType HandleType;

	operator HandleType() const { return h; }
	HandleType handle() const { return *this; } 
	
protected:
	Handle() : h(Policy::NULL_HANDLE), owned(false), ref(0) { }
	Handle(HandleType h_, bool owned_ = true) : h(h_), owned(owned_), ref(0) { } 
	
	void init(HandleType h_, bool owned_) { 
		h = h_; 
		owned = owned_;
	}
	
	virtual ~Handle() {
		if(owned && h != Policy::NULL_HANDLE)
			release(h);
	}
	
private:
	friend void intrusive_ptr_add_ref<Policy>(Handle<Policy>*);
	friend void intrusive_ptr_release<Policy>(Handle<Policy>*);
	
	HandleType h;
	bool owned;
	
	long ref;
};

template<typename Policy>
void intrusive_ptr_add_ref(Handle<Policy>* resource) {
	::InterlockedIncrement(&resource->ref);
}

template<typename Policy>
void intrusive_ptr_release(Handle<Policy>* resource) {
	if(::InterlockedDecrement(&resource->ref) == 0) {
		delete resource;
	}
}

}
#endif /*RESOURCE_H_*/
