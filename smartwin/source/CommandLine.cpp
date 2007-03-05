// $Revision: 1.5 $
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
#include "../include/smartwin/CommandLine.h"

namespace SmartWin
{
// begin namespace SmartWin

std::vector< std::string > buildCommandLine( const char * cmdLine )
{
	using namespace std;
	vector< string > retVal;
	if ( cmdLine == 0 )
		return std::vector< std::string >( 0 );
	string strCmdLine( cmdLine );
	enum POS
	{ EMPTY = '\0', STRING = '"', ESCAPED = '~'
	};
	POS pos = EMPTY;
	string buffer;

	// Looping over string to parse out Command line Arguments
	for ( std::string::iterator idx = strCmdLine.begin();
		idx != strCmdLine.end();
		++idx )
	{
		switch ( * idx )
		{
			case STRING :
			{
				if ( pos == ESCAPED )
				{
					buffer.push_back( * idx );
					pos = STRING;
					break;
				}
				pos = pos == EMPTY ? STRING : EMPTY;
			} break;
			case ESCAPED :
			{
				if ( STRING != pos )
					buffer.push_back( * idx );
				pos = pos == ESCAPED ? STRING : pos == STRING ? ESCAPED : EMPTY;
			} break;
			default:
			{
				if ( pos == STRING )
					buffer.push_back( * idx );
				else if ( * idx != ' ' )
					buffer.push_back( * idx );
				else
				{
					if ( buffer.size() > 0 )
					{
						retVal.push_back( buffer );
						buffer.clear();
					}
				}
			}
		}
	}

	// Adding the last one...
	if ( buffer.size() > 0 )
	{
		retVal.push_back( buffer );
		buffer.clear();
	}

	return retVal;
}

CommandLine::CommandLine( const char * cmdLine )
	: itsCmdLine( buildCommandLine( cmdLine ) ),
	itsRawCmdLine( cmdLine )
{
}

CommandLine::~CommandLine()
{
}

const std::vector< std::string > & CommandLine::getParams() const
{
	return itsCmdLine;
}

const char * CommandLine::getParamsRaw() const
{
	return itsRawCmdLine;
}

// end namespace SmartWin
}
