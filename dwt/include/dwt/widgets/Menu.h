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

#ifndef WINCE
#ifndef DWT_Menu_h
#define DWT_Menu_h

#include "../resources/Bitmap.h"
#include "../resources/Font.h"
#include "../CanvasClasses.h"
#include "../Dispatchers.h"

#include <memory>
#include <vector>
#include <tr1/unordered_map>

namespace dwt {

/// Struct for coloring different areas of Menu
/** Contains the different color settings of the Menu <br>
* Default values to constructor makes menu look roughly like MSVC++7.1 menus
*/
struct MenuColorInfo
{
	/// Menu color
	COLORREF colorMenu;

	/// Strip bar color
	COLORREF colorStrip;

	/// Highlighted menu item color
	COLORREF colorHighlight;

	/// Text of highlighted menu item color
	COLORREF colorHighlightText;	

	/// Title text color
	COLORREF colorTitleText;

	/// Item image background color, used for transparency effects
	COLORREF colorImageBackground;

	/// Constructs MenuColorInfo objects
	/** If all the default arguments are used it will construct an object making
	* menus look roughly like they do in MSVC++ 7.1 <br>
	* Pass your own arguments to construct other color effects
	*/
	MenuColorInfo( COLORREF menuColor = ColorUtilities::darkenColor( ::GetSysColor( COLOR_WINDOW ), 0.02 ),
		COLORREF stripColor = ColorUtilities::darkenColor( ::GetSysColor( COLOR_3DFACE ), 0.02 ),
		COLORREF highlightColor = ::GetSysColor( COLOR_HIGHLIGHT ),
		COLORREF highlightTextColor = ::GetSysColor( COLOR_HIGHLIGHTTEXT ),
		COLORREF titleTextColor = ::GetSysColor( COLOR_MENUTEXT ),
		COLORREF imageBackground = RGB( 0, 0, 0 ) ) // black
		: colorMenu( menuColor ),
		colorStrip( stripColor ),
		colorHighlight( highlightColor ),
		colorHighlightText( highlightTextColor ),
		colorTitleText( titleTextColor ),
		colorImageBackground( imageBackground )
	{}
};

/// Menu class
/** \ingroup WidgetControls
* \WidgetUsageInfo
* \image html menu.png
* Class for creating a Menu Control which then can be attached to e.g. a
* Window. <br>
* Note for Desktop version only! <br>
*/
class Menu : public boost::noncopyable
{
	// friends
	friend class WidgetCreator< Menu >;

public:
	/// Type of object
	typedef Menu ThisType;

	/// Object type
	typedef std::tr1::shared_ptr<Menu> ObjectType;

	struct Seed {
		typedef ThisType WidgetType;
		
		Seed(bool ownerDrawn_ = true, const MenuColorInfo& colorInfo_ = MenuColorInfo(), FontPtr font_ = 0);
		bool popup;
		bool ownerDrawn;
		MenuColorInfo colorInfo;
		FontPtr font;
	};

	struct IdDispatcher
	{
		typedef std::tr1::function<void (unsigned)> F;

		IdDispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			f(LOWORD(msg.wParam));
			return true;
		}

		F f;
	};

	typedef Dispatchers::VoidVoid<> SimpleDispatcher;

	struct DrawItemDispatcher {
		typedef std::tr1::function<bool (int, LPDRAWITEMSTRUCT)> F;

		DrawItemDispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			return f(msg.wParam, reinterpret_cast<LPDRAWITEMSTRUCT>(msg.lParam));
		}

		F f;
	};

	struct MeasureItemDispatcher {
		typedef std::tr1::function<bool (LPMEASUREITEMSTRUCT)> F;

		MeasureItemDispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			return f(reinterpret_cast<LPMEASUREITEMSTRUCT>(msg.lParam));
		}

		F f;
	};

	/// Rendering settting settings
	static const int borderGap; /// Gap between the border and item
	static const int pointerGap; /// Gap between item text and sub - menu pointer
	static const int textIconGap; /// Gap between text and icon
	static const int textBorderGap; /// Gap between text and rectangel border
	static const int separatorHeight; /// Defines default height for rectangle containing separator
	static const int minSysMenuItemWidth; /// Minimum width for system menu items
	static Point defaultImageSize; /// Default image size, used when no image is available

	HMENU handle() const {
		return itsHandle;
	}

	Widget* getParent() const {
		return itsParent;
	}

	/// Actually creates the menu
	/** Creates the menu, the menu will be created initially empty!
	*/
	void create(const Seed& cs);

	void attach(HMENU hMenu, const Seed& cs);

	void setMenu();

	/// Appends a popup to the menu
	/** Everything you "append" to a menu is added sequentially to the menu <br>
	* This specific "append" function appends a "popup" menu which is a menu
	* containing other menus. <br>
	* With other words a menu which is not an "option" but rather a new "subgroup".
	* <br>
	* The "File" menu of most application is for instance a "popup" menu while the
	* File/Print is often NOT a popup. <br>
	* To append items to the popup created call one of the appendItem overloaded
	* functions on the returned value of this function. <br>
	* Also, although references to all menu objects must be kept ( since they're
	* not collected automatically like other Widgets ) <br>
	* you don't have to keep a reference to the return value of this function since
	* it's being added as a reference to the children list of the "this" object.
	* <br>
	* A popup is basically another branch in the menu hierarchy <br>
	* See the Menu project for a demonstration.
	*/
	ObjectType appendPopup( const tstring & text );

	/// Returns the "System Menu"
	/** The system menu is a special menu that ( normally ) is accessed by pressing
	* the "window icon" at the top left of the window. <br>
	* In SmartWin++ this menu can ALSO be easily manipulated and added items to
	* etc... <br>
	* Also, although references to all menu objects must be kept ( since they're
	* not collected automatically like other Widgets ) <br>
	* you don't have to keep a reference to the return value of this function since
	* it's being added as a reference to the children list <br>
	* of the "this" object. <br>
	* See the Menu sample project for a demonstration.
	*/
	ObjectType getSystemMenu();

	/// Setting event handler for Draw Item Event
	/** The Draw Item Event will be raised when the menu needs to draw itself, if you
	* wish to truly be creative and be 100% in control you must handle this Event
	* and do the actualy drawing of the Menu yourself, but for most people it will
	* be enough to just manipulate the background colors etc of the MenuItemData
	* given to the menu in the appendItem function <br>
	* Note! <br>
	* If this event is handled you also MUST handle the Measure Item Event!!
	*/
	bool handleDrawItem(int id, LPDRAWITEMSTRUCT drawInfo);

	/// Setting event handler for Measure Item Event
	/** The Measure Item Event is nessecary to handle if you want to draw the menu
	* yourself since it is inside this Event Handler you're telling the system how
	* much space you need to actually do the drawing <br>
	* Note! <br>
	* If this event is handled you also MUST handle the Draw Item Event!!
	*/
	bool handleMeasureItem(LPMEASUREITEMSTRUCT measureInfo);

	/// Appends a separator item to the menu
	/** A menu separator is basically just "air" between menu items.< br >
	* A separator cannot be "clicked" or "chosen".
	*/
	void appendSeparatorItem();

	/// Appends a Menu Item
	/** eventHandler is the function that will receive the "click" event from the
	* menu item. <br>
	* Event handler's signature must be "void foo( ObjectType, unsigned
	* int )" and it must be contained as a member <br>
	* of the class that is defined as the Widget, normally either the
	* Window derived class or the class derived from Menu. <br>
	* See e.g. Menu for an example. <br>
	* The reason to why we have this "id" is because the same event handler can be
	* defined for several menu items even in fact across menu objects, therefore
	* this number should be unique across the application.
	*/

	void appendItem(unsigned int id, const tstring & text, BitmapPtr image = BitmapPtr());

	template<typename DispatcherType>
	void appendItem(unsigned int id, const tstring & text, const typename DispatcherType::F& f, BitmapPtr image = BitmapPtr()) {
		itsParent->setCallback(Message(WM_COMMAND, id), DispatcherType(f));
		appendItem(id, text, image);
	}

	void appendItem(unsigned int id, const tstring & text, const IdDispatcher::F& f, BitmapPtr image = BitmapPtr()) {
		appendItem<IdDispatcher>(id, text, f, image);
	}

	/// Removes specified item from this menu
	/** Call this function to actually DELETE a menu item from the menu hierarchy.
	* Note that you have to specify the item position; and whenever you remove an item,
	* all subsequent items change positions. To remove a range of items, remove from
	* end to start.
	*/
	void removeItem( unsigned itemIndex );

	/// Remove all items from the menu
	/** Will also delete any submenus.
	*/
	void removeAllItems();

	/// Return the number of items in the menu
	int getCount() const;

	/// Displays and handles a menu which can appear anywhere in the window.
	/** Typically called by a Right Mouse click. If both the x and the y coordinate
	* is - 1 ( default ) it'll show the context menu on the position the mouse was
	* at when the system last recieved a message, basically the "right" place...
	* <br>
	* Depending on the flags it might return the id of the menu item selected, or 0
	* if none was chosen. Flags with TPM_RETURNCMD will return the menu - item, but
	* not do the menu command.
	* < ul >
	* < li >TPM_CENTERALIGN : Centers the shortcut menu horizontally relative to the coordinate specified by the x parameter< /li >
	* < li >TPM_LEFTALIGN : Function positions the shortcut menu so that its left side is aligned with the coordinate specified by the x parameter< /li >
	* < li >TPM_RIGHTALIGN : Opposite of LEFTALIGN< /li >
	* < li >TPM_BOTTOMALIGN : Aligns menu bottoms to the coordinate specified by the y parameter< /li >
	* < li >TPM_TOPALIGN : Opposite of BOTTOMALIGN< /li >
	* < li >TPM_VCENTERALIGN : Centers vertically relative to the y parameter< /li >
	* < li >TPM_NONOTIFY : Restricts the menu from sending notifications when user clicks item< /li >
	* < li >TPM_RETURNCMD : returns the menu item identifier of the user's selection in the return value but DOES NOT carry out the event handler< /li >
	* < li >TPM_LEFTBUTTON  : Restricts users to selecting menu items with only left mouse button< /li >
	* < li >TPM_RIGHTBUTTON : User can choose menu item with both mouse buttons< /li >
	* < /ul >
	* None of the following are used by default but can be manually chosen if you
	* manually call SystemParametersInfo
	* < ul >
	* < li >TPM_HORNEGANIMATION : Animates the menu from right to left< /li >
	* < li >TPM_HORPOSANIMATION : Animates the menu from left to right< /li >
	* < li >TPM_NOANIMATION : Displays menu without animation< /li >
	* < li >TPM_VERNEGANIMATION : Animates the menu from bottom to top< /li >
	* < li >TPM_VERPOSANIMATION : Animates the menu from top to bottom< /li >
	* < /ul >
	*/
	unsigned trackPopupMenu( const ScreenCoordinate& sc, unsigned flags = 0 );

	/// Sets menu title
	/** A Menu can have a title, this function sets that title
	*/
	void setTitle( const tstring & title, bool drawSidebar = false );

	/// Sets title font
	/** Create a font through e.g. createFont in WidgetFactory or similar and set the
	* title font to the menu title through using this function
	*/
	void setTitleFont( FontPtr font );

	/// Removes menu title
	/** If clearSidebar is true, sidebar is removed
	*/
	void clearTitle( bool clearSidebar = false );

	/// Checks (or uncheck) a specific menu item
	/** Which menu item you wish to check ( or uncheck ) is passed in as the "id"
	  * parameter. <br>
	  * If the "value" parameter is true the item will be checked, otherwise it will 
	  * be unchecked       
	  */
	void checkItem( unsigned id, bool value = true );

	/// Enables (or disables) a specific menu item
	/** Which menu item you wish to enable ( or disable ) is passed in as the "id"
	  * parameter. <br>
	  * If the "value" parameter is true the item becomes enabled, otherwise disabled       
	  */
	void setItemEnabled( unsigned id, bool byPosition = false, bool value = true );

	UINT getMenuState(UINT id, bool byPosition = false);

	/// Return true if the item is a separator (by position)
	bool isSeparator(UINT id, bool byPosition = false);
	/// Return true if the menu item is checked
	bool isChecked(UINT id, bool byPosition = false);
	/// Return true if the menu item is a popup menu
	bool isPopup(UINT id, bool byPosition = false);
	/// Return true if the menu item is enabled (not grey and not disabled)
	bool isEnabled(UINT id, bool byPosition = false);

	void setDefaultItem(UINT id, bool byPosition = false);

	/// Returns true if menu is "system menu" (icon in top left of window)
	bool isSystemMenu()
	{
		return isSysMenu;
	}

	/// Returns the text of a specific menu item
	/** Which menu item you wish to retrieve the text for is defined by the "id"
	  * parameter of the function.
	  */
	tstring getText( unsigned idOrPos, bool byPos );

	/// Sets the text of a specific menu item
	/** Which menu item you wish to set the text is defined by the "id"
	  * parameter of the function.
	  */
	void setText( unsigned id, const tstring& text );

	ObjectType getChild(UINT position);

	virtual ~Menu();

private:
	/// Constructor Taking pointer to parent
	explicit Menu( dwt::Widget * parent );

	// ////////////////////////////////////////////////////////////////////////
	// Menu item data wrapper, used internally
	// MENUITEMINFO's dwItemData *should* point to it
	// ////////////////////////////////////////////////////////////////////////
	struct ItemDataWrapper
	{
		// The menu item belongs to
		// For some messages (e.g. WM_MEASUREITEM),
		// Windows doesn't specify it, so
		// we need to keep this
		const Menu* menu;

		// Item index in the menu
		// This is needed, because ID's for items
		// are not unique (although Windows claims)
		// e.g. we can have an item with ID 0,
		// that is either separator or popup menu
		int index;

		// Specifies if item is menu title
		bool isMenuTitleItem;

		/// Menu item text color
		COLORREF textColor;

		/// Menu item image
		BitmapPtr image;

		// Wrapper  Constructor
		ItemDataWrapper( const Menu* menu_, int itemIndex, bool isTitleItem = false, BitmapPtr image_ = BitmapPtr()) : 
			menu( menu_ ), 
			index( itemIndex ),
			isMenuTitleItem( isTitleItem ),
			textColor(::GetSysColor(COLOR_MENUTEXT)),
			image(image_)
		{}

		~ItemDataWrapper()
		{}
	};

	// This is used during menu destruction
	static void destroyItemDataWrapper( ItemDataWrapper * wrapper );

	// True is menu is "system menu" (icon in top left of window)
	bool isSysMenu;

	// its sub menus
	std::vector< ObjectType > itsChildren;

	// its item data
	std::vector < ItemDataWrapper * > itsItemData;

	HMENU itsHandle;

	Widget* itsParent;

	bool ownerDrawn;

	// Contains information about menu colors
	MenuColorInfo itsColorInfo;

	FontPtr font;

	// Menu title
	tstring itsTitle;

	// Menu title font
	FontPtr itsTitleFont;

	// if true title is drawn as sidebar
	bool drawSidebar;

	void createHelper(const Seed& cs);

	// Returns item index in the menu item list
	// If no item with specified id is found, - 1 is returned
	int getItemIndex( unsigned int id );
};

}

#endif
#endif
