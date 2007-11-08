/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met :

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
  ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WidgetChooseColor_h
#define WidgetChooseColor_h

#include "../WindowsHeaders.h"
#include "../FreeCommonDialog.h"
#include <memory>

namespace SmartWin
{
// begin namespace SmartWin

/// ChooseColorDialog class
/** \ingroup WidgetControls
  * \image html choosecolor.png
  * Class for showing a common ChooseColorDialog box. <br>
  * Either derive from it or call WidgetFactory::createChooseColor. <br>
  * Note! <br>
  * If you wish to use this class with EventHandlerClass classes other than those from 
  * SmartWin you need to expose a public function called "parent" taking no arguments 
  * returning an HWND. <br>
  * the complete signature of the function will then be "HWND parent()" <br>
  * This is one of few Widgets in SmartWin++ which can be used without linking in the 
  * actual library!   
  */
template< class Parent >
class WidgetChooseColor
{
public:
	/// Class type
	typedef WidgetChooseColor< Parent > ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	/// Helper class for WidgetChooseColor
	/** Is used as the parameter to the WidgetChooseColor to set the custom colors
	  * and to set the initially default color
	  */
	class ColorParams
	{
		friend class WidgetChooseColor< Parent >;
		COLORREF itsColor;
		COLORREF itsCustomColors[16];
		bool itsUserPressedOk;

	public:
		/// Default constructor
		/** Initializes custom colors to "usable colors" and default color to yellow.
		  */
		ColorParams();

		/// Overloaded constructor taking only default color
		/** Initializes custom colors to "usable colors" and default color to the
		  * given color.
		  */
		ColorParams( COLORREF defaultColor );

		/// Overloaded constructor taking default color and custom color
		/** Initializes custom colors and default color to whatever is passed in
		  */
		ColorParams( COLORREF defaultColor, COLORREF customColors[16] );

		/// Returns the currently selected color
		COLORREF getColor() const;

		/// True if user pressed Ok in dialog
		/** Use this one to determine if the user action was "OK". If user pressed
		  * cancel in the dialog this function will return false!
		  */
		bool userPressedOk() const;
	};

	/// Shows the Choose Color Dialog
	/** Returns a ColorParams object, if user presses Ok the userPressedOk will be
	  * true, else if user pressed Cancel the userPressedOk obviously will be false.
	  * <br>
	  * This dialog "remembers" its state from call to call. If you manipulate the 
	  * Custom Colors, the next time it displays, it will have the same Custom 
	  * Colors. <br>
	  * Note! <br>
	  * This is true even across DIFFERENT instances of the dialog!! <br>
	  * If basic is true dialog will be showed with only basic functionality, if 
	  * allowFullOpen is true dialog will allow the user to show "more info".       
	  */
	ColorParams showDialog( bool basic = true, bool allowFullOpen = true );

	/// Shows the Choose Color Dialog
	/** Returns a ColorParams object, if user presses Ok the userPressedOk will be
	  * true, else if user pressed Cancel the userPressedOk obviously will be false.
	  * <br>
	  * Note! <br>
	  * This function "resets" the remembered state of the dialog meaning if you've 
	  * manipulated the "remembered" state the colorParams object will be the next 
	  * state if you press ok! <br>
	  * If you press Cancel the "old" state ( either default one or from a previous 
	  * call to this or sibling function will become it's "current state" ) <br>
	  * If basic is true dialog will be showed with only basic functionality, if 
	  * allowFullOpen is true dialog will allow the user to show "more info".       
	  */
	ColorParams showDialog( const ColorParams & colorParams, bool basic = true, bool allowFullOpen = true );

	/// Expicit constructor taking pointer to parent
	explicit WidgetChooseColor( Parent * parent = 0 );

	virtual ~WidgetChooseColor()
	{}

private:
	Parent * itsParent;

	// Note!
	// This one is STATIC which normally would be potentially dangerous in e.g.
	// Multi Threaded environments, but since ColorParams is immutable in addition
	// to that two different threads should never be allowed to manipulate GUI in
	// Windows API this isn't dangerous after all...
	static ColorParams itsColorParams;
};

template< class Parent >
typename WidgetChooseColor< Parent >::ColorParams WidgetChooseColor< Parent >::itsColorParams;

/// \ingroup GlobalStuff
/// A Free WidgetChooseColor dialog is a dialog which isn't "owned" by another Widget
typedef WidgetChooseColor< FreeCommonDialog > WidgetChooseColorFree;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class Parent >
WidgetChooseColor< Parent >::ColorParams::ColorParams()
	: itsColor( 0x0000FFFF )
	, itsUserPressedOk( false )
{
	itsCustomColors[0] = 0x00000000;
	itsCustomColors[1] = 0x33333333;
	itsCustomColors[2] = 0x66666666;
	itsCustomColors[3] = 0x99999999;
	itsCustomColors[4] = 0xCCCCCCCC;
	itsCustomColors[5] = 0xFFFFFFFF;
	itsCustomColors[6] = 0x0000FFFF;
	itsCustomColors[7] = 0x000000FF;
	itsCustomColors[8] = 0x0000FF00;
	itsCustomColors[9] = 0x00FF0000;
	itsCustomColors[10] = 0x00FFFF00;
	itsCustomColors[11] = 0x00FF00FF;
	itsCustomColors[12] = 0x002244BB;
	itsCustomColors[13] = 0x0000CC99;
	itsCustomColors[14] = 0x00AA7700;
	itsCustomColors[15] = 0x00997755;
}

template< class Parent >
WidgetChooseColor< Parent >::ColorParams::ColorParams( COLORREF defaultColor )
	: itsColor( defaultColor )
	, itsUserPressedOk( false )
{
	itsCustomColors[0] = 0x00000000;
	itsCustomColors[1] = 0x33333333;
	itsCustomColors[2] = 0x66666666;
	itsCustomColors[3] = 0x99999999;
	itsCustomColors[4] = 0xCCCCCCCC;
	itsCustomColors[5] = 0xFFFFFFFF;
	itsCustomColors[6] = 0x0000FFFF;
	itsCustomColors[7] = 0x000000FF;
	itsCustomColors[8] = 0x0000FF00;
	itsCustomColors[9] = 0x00FF0000;
	itsCustomColors[10] = 0x00FFFF00;
	itsCustomColors[11] = 0x00FF00FF;
	itsCustomColors[12] = 0x002244BB;
	itsCustomColors[13] = 0x0000CC99;
	itsCustomColors[14] = 0x00AA7700;
	itsCustomColors[15] = 0x00997755;
}

template< class Parent >
COLORREF WidgetChooseColor< Parent >::ColorParams::getColor() const
{
	return itsColor;
}

template< class Parent >
WidgetChooseColor< Parent >::ColorParams::ColorParams( COLORREF defaultColor, COLORREF customColors[16] )
	: itsColor( defaultColor )
	, itsUserPressedOk( false )
{
	itsCustomColors = customColors;
}

template< class Parent >
bool WidgetChooseColor< Parent >::ColorParams::userPressedOk() const
{
	return itsUserPressedOk;
}

template< class Parent >
typename WidgetChooseColor< Parent >::ColorParams WidgetChooseColor< Parent >::showDialog( bool basic, bool allowFullOpen )
{
	CHOOSECOLOR cc;
	cc.lStructSize = ( DWORD ) sizeof( CHOOSECOLOR );
	cc.hwndOwner = itsParent->handle();
	cc.hInstance = NULL;
	cc.rgbResult = itsColorParams.itsColor;
	cc.lpCustColors = itsColorParams.itsCustomColors;
	cc.Flags = CC_ANYCOLOR | CC_RGBINIT;
	if ( !basic )
		cc.Flags |= CC_FULLOPEN;
	if ( !allowFullOpen )
		cc.Flags |= CC_PREVENTFULLOPEN;
	cc.lCustData = 0;
	cc.lpfnHook = 0;
	cc.lpTemplateName = NULL;

	itsColorParams.itsUserPressedOk = ::ChooseColor( & cc ) == TRUE;
	if ( itsColorParams.itsUserPressedOk )
	{
		itsColorParams.itsColor = cc.rgbResult;
	}
	return itsColorParams;
}

template< class Parent >
typename WidgetChooseColor< Parent >::ColorParams WidgetChooseColor< Parent >::showDialog( const ColorParams & colorParams, bool basic, bool allowFullOpen )
{
	CHOOSECOLOR cc;
	cc.lStructSize = ( DWORD ) sizeof( CHOOSECOLOR );
	cc.hwndOwner = itsParent->handle();
	cc.hInstance = NULL;
	cc.rgbResult = colorParams.itsColor;
	cc.lpCustColors = itsColorParams.itsCustomColors;
	cc.Flags = CC_ANYCOLOR | CC_RGBINIT;
	if ( !basic )
		cc.Flags |= CC_FULLOPEN;
	if ( !allowFullOpen )
		cc.Flags |= CC_PREVENTFULLOPEN;
	cc.lCustData = NULL;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;

	itsColorParams.itsUserPressedOk = ::ChooseColor( & cc ) == TRUE;
	if ( itsColorParams.itsUserPressedOk )
	{
		itsColorParams.itsColor = cc.rgbResult;
	}
	return itsColorParams;
}

template< class Parent >
WidgetChooseColor< Parent >::WidgetChooseColor( Parent * parent )
	: itsParent( parent )
{
}

// end namespace SmartWin
}

#endif
