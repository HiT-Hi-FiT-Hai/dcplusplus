// $Revision: 1.3 $
/*
  Copyright (c) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

	  * Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice, 
		this list of conditions and the following disclaimer in the documentation 
		and/or other materials provided with the distribution.
	  * Neither the name of the SmartWin++ nor the names of its contributors 
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
#ifndef null_pointer_H
#define null_pointer_H

namespace SmartUtil
{
	/// Policy for null_pointer
	/** Basically just copies the contained pointer by calling "new"
	  */
	template< class T >
	class NewClone
	{
	public:
		/// Clones and returns the given pointer by "return new(*input);"
		static T * clone( T * input )
		{
			if ( input )
				return new T( * input );
			return 0;
		}
	};

	/// Policy for null_pointer
	/** Calls "clone()" which is expected to be a member method on the object "t"
	  */
	template< class T >
	class ObjectClone
	{
	public:
		/// Clones and returns the given pointer by "return input->clone();"
		static T * clone( T * input )
		{
			if ( input )
				return input->clone();
			return 0;
		}
	};

	/// SmartPointer basically to give support for "null objects" without loosing RAII
	/** This pointer have ONE thing it does, it makes sure it's possible to have "null objects" meaning objects that have a "null" value.
	  * This behaviour could easily have been done by normal pointers, but the problem then is RAII or cleanup of heap memory.
	  * Therefor we can use this smart pointer instead.
	  * Note also that the copy constructor and assignment operator does a DEEP copy of the contain pointer!
	  * This means that the copy CTOR and the assignment operator might have a big runtime overhead!
	  * If you need a shared_ptr use the boost namespace!
	  */
	template< class T, template< class > class CopyPolicy = NewClone >
	class null_pointer
	{
		T * itsT;

	public:
		/// Constructs a smart pointer with the value of t
		/** Note that the smart pointer will take "ownership" over the t parameter meaning that it will
		  * delete t when the null_pointer object goes out of scope!
		  */
		explicit null_pointer( T * t )
			: itsT( t )
		{}

		/// Constructs an empty null_pointer
		null_pointer()
			: itsT( 0 )
		{}

		/// Copy constructor
		/** Makes a DEEP copy of the contained object!!
		  */
		null_pointer( const null_pointer & rhs )
			: itsT( CopyPolicy< T >::clone( rhs.itsT ) )
		{}

		/// Destructor, deletes the contained pointer
		~null_pointer()
		{
			delete itsT;
		}

		/// Assignment operator
		/** Makes a DEEP copy of the contained pointer if there is data in it after freeing up it's old contained value (if there was an old value)
		  */
		null_pointer & operator =( const null_pointer & rhs )
		{
			delete itsT;
			if ( rhs.itsT )
				itsT = CopyPolicy< T >::clone( rhs.itsT );
			return * this;
		}

		/// Assignment operator
		/** Note that this assignment operator takes OWNERSHIP over the contained pointer.
		  * Meaning it will free or delete the contained pointer when the object goes out of scope!
		  */
		null_pointer & operator =( T * rhs )
		{
			delete itsT;
			if ( rhs )
				itsT = rhs;
			return * this;
		}

		/// Returns the contained pointer value
		/** Use this one to check if pointer actually have a value or not
		  */
		T * get()
		{
			return itsT;
		}

		/// Returns the contained pointer value
		/** Use this one to check if pointer actually have a value or not
		  */
		const T * get() const
		{
			return itsT;
		}

		/// Returns the contained pointer content by reference
		/** Basically a dereference operator
		  */
		T & operator *()
		{
			return * itsT;
		}

		/// Returns the contained pointer content by reference
		/** Basically a dereference operator
		  */
		const T & operator *() const
		{
			return * itsT;
		}

		/// Returns the contained pointer value
		T * operator -> ()
		{
			return itsT;
		}

		/// Returns the contained pointer value
		const T * operator -> () const
		{
			return itsT;
		}
	};
}

#endif
