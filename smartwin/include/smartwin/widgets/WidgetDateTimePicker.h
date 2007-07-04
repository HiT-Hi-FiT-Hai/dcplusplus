// $Revision: 1.28 $
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

#include "SmartUtil.h"
#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectAdapter.h"
#include "../xCeption.h"
#include "../Message.h"

namespace SmartWin
{
// begin namespace SmartWin


// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

struct AspectDateTimePickerDispatcher
{
	typedef std::tr1::function<void (const SYSTEMTIME &)> F;

	AspectDateTimePickerDispatcher(const F& f_) : f(f_) { }

	HRESULT operator()(private_::SignalContent& params) {
		f(reinterpret_cast< NMDATETIMECHANGE * >( params.Msg.LParam )->st);
		return 0;
	}

	F f;
};
/// DateTimePicker Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html datetimepicker.PNG
  * Class for creating a DateTimePicker control. <br>
  * A datetimepicker is a control for choosing a date consisting of year, month and 
  * day. It resembles a calender and is quite neat to use if you need to specifically 
  * declare a point in time within 1800 - 2100   
  */
template< class EventHandlerClass >
class WidgetDateTimePicker :
	public MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > >,
	private virtual TrueWindow,

	// Aspects
	public AspectClickable< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass> > >,
	public AspectEnabled< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectFocus< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectFont< WidgetDateTimePicker< EventHandlerClass > >,
	public AspectKeyboard< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectMouseClicks< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectPainting< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectRaw< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectSizable< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectThreads< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >,
	public AspectVisible< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetDateTimePicker< EventHandlerClass > > >
{
public:
	/// Class type
	typedef WidgetDateTimePicker< EventHandlerClass > ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	/// Message map type
	typedef MessageMapControl< EventHandlerClass, ThisType > MessageMapType;
	typedef AspectEnableDispatcher Dispatcher;
	typedef AspectAdapter<Dispatcher::F, EventHandlerClass, MessageMapType::IsControl> Adapter;
	friend class WidgetCreator< WidgetDateTimePicker >;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetDateTimePicker::ThisType WidgetType;

		FontPtr font;
		SmartUtil::tstring format;
		COLORREF backgroundColor;
		COLORREF monthBackgroundColor;
		COLORREF monthTextColor;
		COLORREF titleBackgroundColor;
		COLORREF titleTextColor;
		COLORREF trailingTextColor;
		SYSTEMTIME initialDateTime;
		//TODO: put variables to be filled here

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	// Aspect expectation implementation
	static Message & getClickMessage();

	/// Member function Setting the event handler for the "date changed" event
	/** The event handler must have the signature "void foo( WidgetDateTimePickerPtr
	  * date, const SYSTEMTIME & st )" <br>
	  * where the date is the WidgetDateTimePicker that triggered the event. <br>
	  * If you supply an event handler for this event your handler will be called 
	  * when the WidgetDateTimePicker date value is changed. 
	  */
	void onDateTimeChanged( typename MessageMapType::itsVoidFunctionTakingSystemTime eventHandler ) {
		onDateTimeChanged(Adapter::adapt1(boost::polymorphic_cast<ThisType*>(this), eventHandler));
	}
	void onDateTimeChanged( typename MessageMapType::voidFunctionTakingSystemTime eventHandler ) {
		onDateTimeChanged(Adapter::adapt1(boost::polymorphic_cast<ThisType*>(this), eventHandler));
	}
	void onDateTimeChanged(const Dispatcher::F& f) {
		MessageMapBase * ptrThis = boost::polymorphic_cast< MessageMapBase * >( this );
		ptrThis->setCallback(
			typename MessageMapType::SignalTupleType(
				Message( WM_NOTIFY, DTN_DATETIMECHANGE ), Dispatcher(f)
			)
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
	virtual void create( const Seed & cs = getDefaultSeed() );

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

template< class EventHandlerClass >
const typename WidgetDateTimePicker< EventHandlerClass >::Seed & WidgetDateTimePicker< EventHandlerClass >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, DATETIMEPICK_CLASS );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT;
		d_DefaultValues.backgroundColor = 0x000080;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_DefaultValues.format = _T( "yyyy.MM.dd" ); //TODO: should be filled out with locale from OS
		GetSystemTime( & d_DefaultValues.initialDateTime );
		d_DefaultValues.monthBackgroundColor = 0x808080;
		d_DefaultValues.monthTextColor = 0xFFFFFF;
		d_DefaultValues.titleBackgroundColor = 0x202020;
		d_DefaultValues.titleTextColor = 0x008080;
		d_DefaultValues.trailingTextColor = 0x000000;
		//TODO: initialize the values here

		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass >
WidgetDateTimePicker< EventHandlerClass >::Seed::Seed()
{
	* this = WidgetDateTimePicker::getDefaultSeed();
}

template< class EventHandlerClass >
Message & WidgetDateTimePicker< EventHandlerClass >::getClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, DTN_DROPDOWN );
	return retVal;
}

template< class EventHandlerClass >
SYSTEMTIME WidgetDateTimePicker< EventHandlerClass >::getDateTime()
{
	SYSTEMTIME st;
	DateTime_GetSystemtime( this->Widget::itsHandle, & st );
	return st;
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setDateTime( const SYSTEMTIME & st )
{
	DateTime_SetSystemtime( this->Widget::itsHandle, GDT_VALID, & st );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setFormat( const SmartUtil::tstring & format )
{
	DateTime_SetFormat( this->Widget::itsHandle, format.c_str() );
}

template< class EventHandlerClass >
WidgetDateTimePicker< EventHandlerClass >::WidgetDateTimePicker( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a TextBox without a parent..." ) );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetDateTimePicker::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	MessageMapType::createMessageMap();
	//TODO: use CreationalInfo parameters
	setFont( cs.font );
	setFormat( cs.format );
	setDateTime( cs.initialDateTime );
	setBackgroundColor( cs.backgroundColor );
	setMonthBackgroundColor( cs.monthBackgroundColor );
	setMonthTextColor( cs.monthTextColor );
	setTitleBackgroundColor( cs.titleBackgroundColor );
	setTrailingTextColor( cs.trailingTextColor );
	setTitleTextColor( cs.titleTextColor );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setBackgroundColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->Widget::itsHandle, MCSC_BACKGROUND, color );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setMonthBackgroundColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->Widget::itsHandle, MCSC_MONTHBK, color );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setMonthTextColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->Widget::itsHandle, MCSC_TEXT, color );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setTitleBackgroundColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->Widget::itsHandle, MCSC_TITLEBK, color );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setTitleTextColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->Widget::itsHandle, MCSC_TITLETEXT, color );
}

template< class EventHandlerClass >
void WidgetDateTimePicker< EventHandlerClass >::setTrailingTextColor( COLORREF color )
{
	DateTime_SetMonthCalColor( this->Widget::itsHandle, MCSC_TRAILINGTEXT, color );
}

// end namespace SmartWin
}

#endif
