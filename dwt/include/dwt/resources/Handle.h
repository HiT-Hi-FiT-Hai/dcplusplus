/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
        may be used to endorse or promote products derived from this software 
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
