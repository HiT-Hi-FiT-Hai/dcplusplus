// $Revision: 1.7 $
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
#ifndef Command_h
#define Command_h

#include "SmartUtil.h"
#include <memory>
#include <typeinfo>
#include "boost.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Class for wrapping a custom command object, SHOULD be derived from instead of
/// used directly
/** Can if nothing else then a unique name is needed be instantiated directly... <br>
  * Used e.g. in createTimer...
  */
class Command
{
public:
	/// Constructor, unless you derive from the Command class you SHOULD use this
	/// constructor
	/** Constructor taking a SmartUtil::tstring which then can be used for
	  * identifying the Command to make sure you have the "right" command object.
	  */
	explicit Command( const SmartUtil::tstring & name )
		: itsName( name )
	{}

	/// Virtual DTOR since we often will destroy object from a pointer to a base class
	/** This assures you can destroy the Command from a base class pointer (normally
	  * a Command will be instantiated from a derived class but often destroyed from
	  * the base class Command)
	  */
	virtual ~Command()
	{}

	// TODO: Rename... (coding standard)
	/// Returns the "friendly" name of the Command
	/** This is the name that was supplied when constructing your Command (if you
	  * used the Constructor taking a string)
	  */
	const SmartUtil::tstring & Name() const
	{ return itsName;
	}

	/// Virtual copy constructor
	/** If you derive from this class you MUST override this function to return an
	  * instance of your <br>
	  * own custom Command object. <br>
	  * Otherwise you WILL experience "slicing" of your Command object
	  */
	virtual std::auto_ptr< Command > clone() const
	{
		xAssert( typeid( * this ) == typeid( Command ), _T( "Internal Error. Application haven't overridden virtual Copy Constructor function clone in class Command, slicing of object occured, contact vendor." ) );
		// If the above assert fails you probably haven't implemented the clone
		// function in a class derived from Command

		return std::auto_ptr< Command >( new Command( * this ) );
	}

protected:
	/// Constructor taking nothing.
	/** DON'T use this Constructor unless you derive from the class. <br>
	  * Sets the "name" of the Command to "Undefined".
	  */
	Command()
		: itsName( _T( "Undefined" ) )
	{}

private:
	// Private COPY ctor to ensure use of the clone function
	Command( const Command & rhs )
		: itsName( rhs.Name() )
	{}

	// Private assignment operator declaration to ensure use of the "clone" logic
	// or "virtual copy Constructor"
	Command & operator =( const Command & rhs )
	{
		itsName = rhs.Name();
		return * this;
	}

	SmartUtil::tstring itsName;
};

/// \ingroup GlobalStuff
// Command Smart Pointer
/** Wraps up a Command to a Smart Pointer, help abstracts away the internals of the
  * library!
  */
typedef boost::shared_ptr< Command > CommandPtr;

// end namespace SmartWin
}

#endif
