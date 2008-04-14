/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
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

#ifndef TabSheet_h
#define TabSheet_h

#include "../resources/ImageList.h"
#include "../Rectangle.h"
#include "../aspects/AspectCollection.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectText.h"
#include "Control.h"

namespace dwt {

/// Tab Sheet Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html tabsheet.png
  * Class for creating a Tab Control Widget. <br>
  * A Tab Control is a control consisting of "tab buttons" normally on the top of the
  * Widget where the user can select different pages to switch between to group
  * related information within. E.g. Visual Studio has got tab controls on the top of
  * the code area where the user can switch between the different opened files. Use
  * the onSelectionChanged event to make visible/invisible the different controls you
  * wish to use in the different tab pages! <br>
  * Normally you would add up one Container for each Tab Page the Tab Control
  * has.
  */
class TabSheet :
	public CommonControl,
	// Aspects
	public AspectCollection<TabSheet, int>,
	public AspectFocus< TabSheet >,
	public AspectFont< TabSheet >,
	public AspectPainting< TabSheet >,
	public AspectSelection< TabSheet, int >,
	public AspectText< TabSheet >
{
	friend class AspectCollection<TabSheet, int>;
	friend class AspectSelection<TabSheet, int>;
	friend class WidgetCreator< TabSheet >;
	
	struct ChangingDispatcher
	{
		typedef std::tr1::function<bool (unsigned)> F;

		ChangingDispatcher(const F& f_, TabSheet* widget_) : f(f_), widget(widget_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			unsigned param = TabCtrl_GetCurSel( widget->handle() );
			ret = f(param) ? FALSE : TRUE;
			return true;
		}

		F f;
		TabSheet* widget;
	};


public:
	/// Class type
	typedef TabSheet ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	typedef CommonControl BaseType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		FontPtr font;

		/// Fills with default parameters
		explicit Seed();
	};

	SmartUtil::tstring getText(unsigned idx) const;
	
	void setText(unsigned idx, const SmartUtil::tstring& text);

	/// Setting the event handler for the "selection changing" event
	/** The event handler must have the signature "bool foo( TabSheet * Widget,
	  * unsigned indexNo )" whereby if you return true the user will be allowed to
	  * actually CHANGE the page but if you return false the page will not be allowed
	  * to change and the onSelectionChanged event will not fire ( good for
	  * validation of fields etc...)
	  */
	void onSelectionChanging(const ChangingDispatcher::F& f) {
		addCallback(
			Message( WM_NOTIFY, TCN_SELCHANGING ), ChangingDispatcher(f, this )
		);
	}

	/// Appends a "page" to the Tab Sheet
	/** The return value is the index of the new item appended. The input index is
	  * where you wish to put the new page
	  */
	// the negative values are already covered by throwing an exception
	unsigned int addPage( const SmartUtil::tstring & header, unsigned index, LPARAM lParam = 0, int image = -1 );

	int getImage(unsigned idx) const;

	HWND getToolTips() const;

	LPARAM getData(unsigned idx);
	
	void setData(unsigned idx, LPARAM data);

	/// Actually creates the Tab Sheet Control
	/** You should call WidgetFactory::createTabSheet if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	void create( const Seed & cs = Seed() );

	/// Set tab buttons at bottom of control
	/** If passed true to this function tabs will appear at the bottom of the control
	  */
	void setTabsAtBottom( bool value = true );

	/// Set tabs to "button" style
	/** If passed true to this function tabs will appear as buttons instead of
	  * default as pages
	  */
	void setButtonStyle( bool value = true );

	/// Set tabs to "flat button" style
	/** If passed true to this function tabs will appear as flat buttons instead of
	  * default as pages
	  */
	void setFlatButtonStyle( bool value = true );

	/// Turns hot tracking of tabs on or off
	/** If passed true hottracking of items will be turned on
	  */
	void setHotTrack( bool value = true );

	/// Set tabs to "multiline" style
	/** If passed true to this function tabs will be able to span across multiple
	  * lines
	  */
	void setMultiline( bool value = true );

	/// Set tabs to "ragged right" style
	/** If passed true to this function tabs be ragged to the right to make pages
	  * "span" across whole area if multiple lines are inserted
	  */
	void setRaggedRight( bool value = true );

	/// Set tabs to appear vertically instead of horizontally which is the default style
	/** If passed true to this function tabs will appear vertically instead of
	  * horizontally
	  */
	void setVerticalTabs( bool value = true );

	/// Set tabs to appear vertically to the right
	/** This one also turns on vertical style
	  */
	void setRightTabs( bool value = true );

	void setHighlight(int item, bool highlight = true);
	
	/// Set tabs to appear with a flat separator between different tabs
	/** If true passed flat separator style will be turned ON else OFF
	  */
	void setFlatSeparators( bool value = true );

	void setImageList(const ImageListPtr& imageList);

	const ImageListPtr& getImageList() const;
	
	int hitTest(const ScreenCoordinate& pt);
	
	/// Get the area not used by the tabs
	/** This function should be used after adding the pages, so that the area not used by
	  * the tabs can be calculated accurately. It returns coordinates respect to the
	  * TabControl, this is, you have to adjust for the position of the control itself.   
	  */
	Rectangle getUsableArea(bool cutBorders = false) const;
protected:
	// Constructor Taking pointer to parent
	explicit TabSheet( Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~TabSheet()
	{}
	
private:
	// AspectSelection expectation implementation
	static Message getSelectionChangedMessage();

	// Keep a copy so it won't get deallocated...
	ImageListPtr imageList;

	// AspectCollection
	void eraseImpl( int row );
	void clearImpl();
	size_t sizeImpl() const;
	
	// AspectSelection
	int getSelectedImpl() const;
	void setSelectedImpl( int idx );

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline Message  TabSheet::getSelectionChangedMessage() {
	return Message( WM_NOTIFY, TCN_SELCHANGE );
}

inline int TabSheet::getSelectedImpl() const {
	return TabCtrl_GetCurSel( this->handle() );
}

inline int TabSheet::getImage(unsigned idx) const
{
	TCITEM item = { TCIF_IMAGE };
	if ( !TabCtrl_GetItem( this->handle(), idx, & item ) )
	{
		throw xCeption( _T( "Couldn't get image of item." ) );
	}
	return item.iImage;
}

inline HWND TabSheet::getToolTips() const
{
	HWND wnd = TabCtrl_GetToolTips(this->handle());
	if(wnd == NULL)
	{
		throw xCeption( _T( "Couldn't get tooltips HWND." ) );
	}
	return wnd;
}

inline LPARAM TabSheet::getData(unsigned idx)
{
	TCITEM item = { TCIF_PARAM };
	if ( !TabCtrl_GetItem( this->handle(), idx, & item ) )
	{
		throw xCeption( _T( "Couldn't get data of item." ) );
	}
	return item.lParam;
}

inline void TabSheet::setSelectedImpl( int idx ) {
	TabCtrl_SetCurSel( this->handle(), idx );
}

inline void TabSheet::setText( unsigned index, const SmartUtil::tstring& text )
{
	TCITEM item = { TCIF_TEXT };
	item.pszText = const_cast < TCHAR * >( text.c_str() );
	TabCtrl_SetItem(this->handle(), index, &item);
}

inline void TabSheet::setData( unsigned index, LPARAM lParam )
{
	TCITEM item = { TCIF_PARAM };
	item.lParam = lParam;
	TabCtrl_SetItem(this->handle(), index, &item);
}

inline TabSheet::TabSheet( dwt::Widget * parent )
	: BaseType( parent )
{
}

inline void TabSheet::setTabsAtBottom( bool value )
{
	this->addRemoveStyle( TCS_BOTTOM, value );
}

inline void TabSheet::setButtonStyle( bool value )
{
	this->addRemoveStyle( TCS_BUTTONS, value );
}

inline void TabSheet::setFlatButtonStyle( bool value )
{
	this->addRemoveStyle( TCS_BUTTONS, value );
	this->addRemoveStyle( TCS_FLATBUTTONS, value );
}

inline void TabSheet::setHotTrack( bool value )
{
	this->addRemoveStyle( TCS_HOTTRACK, value );
}

inline void TabSheet::setMultiline( bool value )
{
	this->addRemoveStyle( TCS_MULTILINE, value );
}

inline void TabSheet::setRaggedRight( bool value )
{
	this->addRemoveStyle( TCS_RAGGEDRIGHT, value );
}

inline void TabSheet::setVerticalTabs( bool value )
{
	this->addRemoveStyle( TCS_VERTICAL, value );
}

inline void TabSheet::setRightTabs( bool value )
{
	this->addRemoveStyle( TCS_VERTICAL | TCS_RIGHT, value );
}

inline void TabSheet::setFlatSeparators( bool value )
{
	this->sendMessage( TCM_SETEXTENDEDSTYLE, TCS_EX_FLATSEPARATORS, value );
}

inline const ImageListPtr& TabSheet::getImageList() const
{
	return imageList;
}

inline void TabSheet::setHighlight(int item, bool highlight) {
	TabCtrl_HighlightItem(handle(), item, highlight);
}

inline int TabSheet::hitTest(const ScreenCoordinate& pt) {
	TCHITTESTINFO tci = { ClientCoordinate(pt, this).getPoint() };
	
	return TabCtrl_HitTest(handle(), &tci);
}

inline void TabSheet::eraseImpl(int i) {
	TabCtrl_DeleteItem(this->handle(), i);
}

inline void TabSheet::clearImpl() {
	TabCtrl_DeleteAllItems(handle());
}

inline size_t TabSheet::sizeImpl() const {
	return static_cast<size_t>(TabCtrl_GetItemCount(this->handle()));
}

}

#endif
