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
#ifndef CommandLine_h
#define CommandLine_h

#include <string>
#include <vector>

namespace SmartWin
{
// begin namespace SmartWin

class Application;

/// Class declaration for the CommandLine class
/** The CommandLine class is a helper class for extracting the Command line
  * parameters which are being sent into the WinMain/main function. <br>
  * If you need to retrieve the Command line parameters use this class. <br>
  * Note! <br>
  * An object of type CommandLine can be copied and destroyed by anyone but it cannot
  * be constructed by any other means than through the Application::getCommandLine()
  * function! <br>
  * This means that the ONLY way you can access the Command line parameters is
  * through the  Application::getCommandLine() function <br>
  * But since the Application is a Natural singleton class it should be easy to
  * extract the Command line parameters from anywhere you wish! <br>
  * Just remember that the constructing of an object (which is being done in the
  * getCommandLine) do come with a bit of overhead! (parsing of  the actual params)
  * <br>
  * So instead of calling the getCommandLine() several times either cache the
  * CommandLine object or cache the return value from the getParams() function of the
  * object!
  */
class CommandLine
{
	friend class Application;
public:
	/// Returns a vector of the actual params
	/** You can escape params by embracing them inside " and you can add a " inside a
	  * " by adding a ~ (escape character) in front of it. <br>
	  * Note! <br>
	  * The actual " will NOT show up in the param! <br>
	  * Meaning that the param "xyz" will become xyz... <br>
	  * Note! <br>
	  * It will ALWAYS return char * strings and NEVER wchar_t <br>
	  * Use the Ascii2CurrentBuild::doConvert() function if you need to get the
	  * parameter transformed into the "current build" type (if UNICODE && _UNICODE
	  * is defined UNICODE else ASCII) <br>
	  * Escape characters OUTSIDE a string (surrounded by ") means nothing or will
	  * not be specially treated <br>
	  * Redundant spaces OUTSIDE a string will be removed. <br>
	  * Meaning that
	  * -h "~~ ~"" testing  -t --t~~~ "Thomas~~~~~"   hhh"    heisann-- ~~    thomas2    thomasHansen..<br>
	  * will become
	  * <ul>
	  * <li>-h</li>
	  * <li>~ "</li>
	  * <li>testing</li>
	  * <li>-t</li>
	  * <li>--t~~~</li>
	  * <li>Thomas~~"   hhh</li>
	  * <li>heisann--</li>
	  * <li>~~</li>
	  * <li>thomas2</li>
	  * <li>thomasHansen..</li>
	  * </ul>
	  * The reason that not \ was chosen as the escaping character was because it's
	  * just too often used e.g. in paths etc and demanding that users of your
	  * application would have to escape all literals of type \ in their paths would
	  * impose so much overhead that the class would render useless and everybody
	  * would just resemble to using the "raw" getter...
	  */
	const std::vector< std::string > & getParams() const;

	/// Returns the "raw" command line parameter
	/** For those of you which MUST have the actual RAW command line parameter you
	  * can use this function which will return them as given to the application.
	  * Note! <br>
	  * It will ALWAYS return char * and NEVER wchar_t <br>
	  * Use one of the UNICODE converters if you must have it in UNICODE or
	  * "CURRENT_BUILD"...
	  */
	const char * getParamsRaw() const;

	~CommandLine();

private:
	// Only Application which is friend can instantiate an object of this type!!
	CommandLine( const char * cmdLine );

	// Uncopyable!
	CommandLine( const CommandLine & ); // Never implemented!

	const std::vector< std::string > itsCmdLine;
	const char * itsRawCmdLine;
};

// end namespace SmartWin
}

#endif
