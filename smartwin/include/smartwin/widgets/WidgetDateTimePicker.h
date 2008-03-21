/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

	  * Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.
	  * Neither the name of the SmartWin ++ nor the names of its contributors
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
#ifndef WidgetDateTimePicker_h
#define WidgetDateTimePicker_h

#include "../Widget.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectControl.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"

namespace SmartWin
{
// begin namespace SmartWin


// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// DateTimePicker Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html datetimepicker.PNG
  * Class for creating a DateTimePicker control. <br>
  * A datetimepicker is a control for choosing a date consisting of year, month and 
  * day. It resembles a calender and is quite neat to use if you need to specifically 
  * declare a point in time within 1800 - 2100   
  */
class WidgetDateTimePicker :
	// Aspects
	public AspectClickable< WidgetDateTimePicker >,
	public AspectControl<WidgetDateTimePicker>,
	public AspectFocus< WidgetDateTimePicker >,
	public AspectFont< WidgetDateTimePicker >,
	public AspectPainting< WidgetDateTimePicker >
{
	struct Dispatcher
	{
		typedef std::tr1::function<void (const SYSTEMTIME &)> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			f(reinterpret_cast< NMDATETIMECHANGE * >( msg.lParam )->st);
			return true;
		}

		F f;
	};

public:

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	friend class WidgetCreator< WidgetDateTimePicker >;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public Widget::Seed
	{
	public:
		FontPtr font;

		SmartUtil::tstring format;
		COLORREF backgroundColor;
		COLORREF monthBackgroundColor;
		COLORREF monthTextColor;
		COLORREF titleBackgroundColor;
		COLORREF titleTextColor;
		COLORREF trailingTextColor;
		SYSTEMTIME initialDateTime;
	
		/// Fills with default parameters
		Seed();
	};

	// Aspect expectation implementation
	static Message & getClickMessage();

	/// Member function Setting the event handler for the "date changed" event
	/** The event handler must have the signature "void foo( WidgetDateTimePickerPtr
	  * date, const SYSTEMTIME & st )" <br>
	  * where the date is the WidgetDateTimePicker that triggered the event. <br>
	  * If you supply an event handler for this event your handler will be called 
	  * when the WidgetDateTimePicker date value is changed. 
	  */
	void onDateTimeChanged(const Dispatcher::F& f) {
		addCallback(
			Message( WM_NOTIFY, DTN_DATETIMECHANGE ), Dispatcher(f)
		);
	}

	/// Retrieves the time value of the DateTimePicker control
	/** Returns a SYSTEMTIME struct containing down to resolution in milliseconds the
	  * selected value of the date time picker control.
	  */
	SYSTEMTIME getDateTime();

	/// Sets the date and time of the control
	/** Sets the date end time of the control to the given SYSTEMTIME.
	  */
	void setDateTime( const SYSTEMTIME & st );

	/// Sets the format to use when displaying the date and time in the control
	/** The format can be any combinations in any order of the given ones
	  * < ul >
	  * < li >"d" The one - or two - digit day.< /li >
	  * < li >"dd" The two - digit day. Single - digit day values are preceded by a zero.< /li >
	  * < li >"ddd" The three - character weekday abbreviation.< /li >
	  * < li >"dddd" The full weekday name.< /li >
	  * < li >"h" The one - or two - digit hour in 12 - hour format.< /li >
	  * < li >"hh" The two - digit hour in 12 - hour format. Single - digit values are preceded by a zero.< /li >
	  * < li >"H" The one - or two - digit hour in 24 - hour format.< /li >
	  * < li >"HH" The two - digit hour in 24 - hour format. Single - digit values are preceded by a zero.< /li >
	  * < li >"m" The one - or two - digit minute.< /li >
	  * < li >"mm" The two - digit minute. Single - digit values are preceded by a zero.< /li >
	  * < li >"M" The one - or two - digit month number.< /li >
	  * < li >"MM" The two - digit month number. Single - digit values are preceded by a zero.< /li >
	  * < li >"MMM" The three - character month abbreviation.< /li >
	  * < li >"MMMM" The full month name.< /li >
	  * < li >"t" The one - letter AM/PM abbreviation ( that is, AM is displayed as "A" ).< /li >
	  * < li >"tt" The two - letter AM/PM abbreviation ( that is, AM is displayed as "AM" ).< /li >
	  * < li >"yy" The last two digits of the year ( that is, 1996 would be displayed as "96" ).< /li >
	  * < li >"yyyy" The full year ( that is, 1996 would be displayed as "1996" ).< /li >
	  * < /ul >
	  * E.g. if you send in "yyyy - MM - dd HH:mm:ss" the control would show e.g.: 
	  * "2004 - 12 - 24 17:50:14"       
	  */
	void setFormat( const SmartUtil::tstring & format );

	/// Sets the bacground color used between months in the calender
	/** The color filled between months in the calender
	  */
	void setBackgroundColor( COLORREF color );

	/// Sets the month bacground color of the calender
	/** The background color is the face behind the days in the month, the "main"
	  * area of the control
	  */
	void setMonthBackgroundColor( COLORREF color );

	/// Sets the month text color
	/** This is the text the days in the month will display in
	  */
	void setMonthTextColor( COLORREF color );

	/// Sets the title background color
	/** The title is the area in the top of the control
	  */
	void setTitleBackgroundColor( COLORREF color );

	/// Sets the text color of the title
	/** The title text color is the top text on top of the title background
	  */
	void setTitleTextColor( COLORREF color );

	///Sets the text color of the months "left outside" the current month
	/** E.g if you're in June and you see e.g. 3 days in July too the days in July
	  * will display in this color.
	  */
	void setTrailingTextColor( COLORREF color );

	/// Actually creates the Date Time Picker Control
	/** You should call WidgetFactory::createDateTimePicker if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	void create( const Seed & cs = Seed() );

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetDateTimePicker( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetDateTimePicker()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline Message & WidgetDateTimePicker::getClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, DTN_DROPDOWN );
	return retVal;
}

inline SYSTEMTIME WidgetDateTimePicker::getDateTime()
{
	SYSTEMTIME st;
	DateTime_GetSystemtime( this->handle(), & st );
	return st;
}

inline void WidgetDateTimePicker::setDateTime( const SYSTEMTIME & st )
{
	DateTime_SetSystemtime( this->handle(), GDT_VALID, & st );
}

inline void WidgetDateTimePicker::setFormat( const SmartUtil::tstring & format )
{
	DateTime_SetFormat( this->handle(), format.c_str() );
}

inline WidgetDateTimePicker::WidgetDateTimePicker( SmartWin::Widget * parent )
	: ControlType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a TextBox without a parent..." ) );
}

inline void WidgetDateTimePicker::setBackgroundColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->handle(), MCSC_BACKGROUND, color );
}

inline void WidgetDateTimePicker::setMonthBackgroundColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->handle(), MCSC_MONTHBK, color );
}

inline void WidgetDateTimePicker::setMonthTextColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->handle(), MCSC_TEXT, color );
}

inline void WidgetDateTimePicker::setTitleBackgroundColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->handle(), MCSC_TITLEBK, color );
}

inline void WidgetDateTimePicker::setTitleTextColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->handle(), MCSC_TITLETEXT, color );
}

inline void WidgetDateTimePicker::setTrailingTextColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->handle(), MCSC_TRAILINGTEXT, color );
}

// end namespace SmartWin
}

#endif
