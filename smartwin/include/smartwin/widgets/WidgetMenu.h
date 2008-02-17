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
#ifndef WINCE
#ifndef WidgetMenu_h
#define WidgetMenu_h

#include "../Application.h"
#include "../BasicTypes.h"
#include "../CanvasClasses.h"
#include "../Dispatchers.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// Contains information about menu item
struct MenuItemData
{
	/// Menu item text color
	COLORREF TextColor;

	/// Menu item font
	FontPtr Font;

	/// Menu item image
	BitmapPtr Image;

	/// Creates new menu item with specified data
	MenuItemData(
		FontPtr font = FontPtr( new SmartWin::Font( ( HFONT )::GetStockObject( DEFAULT_GUI_FONT ), false ) ),
		BitmapPtr image = BitmapPtr( new Bitmap( ( HBITMAP ) NULL ) ), // defaults to empty bitmap
		COLORREF textColor = ::GetSysColor( COLOR_MENUTEXT ) )
		: TextColor( textColor ),
		Font( font ),
		Image( image )
	{}
};

/// \ingroup GlobalStuff
// MenuItemDataPtr type, contains rendering data for e.g. WidgetMenu
/** Helps easily create color values and so on for a WidgetMenu item! <br>
* Each Menu Item can have different colors and so on, use this smart pointer to set
* those values!
*/
typedef std::tr1::shared_ptr< MenuItemData > MenuItemDataPtr;

/// Struct for coloring different areas of WidgetMenu
/** Contains the different color settings of the WidgetMenu <br>
* Default values to constructor makes menu look roughly like MSVC++7.1 menus
*/
struct MenuColorInfo
{
	/// Menu color
	COLORREF colorMenu;

	/// Strip bar color
	COLORREF colorStrip;

	/// Menu bar color
	COLORREF colorMenuBar;

	/// Highlighted menu item color
	COLORREF colorHighlight;

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
		COLORREF menuBarColor = ::GetSysColor( COLOR_MENUBAR ),
		COLORREF highlightColor = ::GetSysColor( COLOR_HIGHLIGHT ),
		COLORREF titleTextColor = ::GetSysColor( COLOR_MENUTEXT ),
		COLORREF imageBackground = RGB( 0, 0, 0 ) ) // black
		: colorMenu( menuColor ),
		colorStrip( stripColor ),
		colorMenuBar( menuBarColor ),
		colorHighlight( highlightColor ),
		colorTitleText( titleTextColor ),
		colorImageBackground( imageBackground )
	{}
};

/// Menu class
/** \ingroup WidgetControls
* \WidgetUsageInfo
* \image html menu.png
* Class for creating a Menu Control which then can be attached to e.g. a
* WidgetWindow. <br>
* Note for Desktop version only! <br>
*/
class WidgetMenu : public boost::enable_shared_from_this< WidgetMenu >, boost::noncopyable
{
	// friends
	friend class WidgetCreator< WidgetMenu >;

public:
	/// Type of object
	typedef WidgetMenu ThisType;

	/// Object type
	typedef std::tr1::shared_ptr<WidgetMenu> ObjectType;

	struct Seed {
		Seed(bool ownerDrawn_ = true, const MenuColorInfo& colorInfo_ = MenuColorInfo()) : popup(true), ownerDrawn(ownerDrawn_), colorInfo(colorInfo_) { }
		bool popup;
		bool ownerDrawn;
		MenuColorInfo colorInfo;
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

	/// Attaches the menu to the parent window
	void attach();

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
	* See the WidgetMenu project for a demonstration.
	*/
	ObjectType appendPopup( const SmartUtil::tstring & text, MenuItemDataPtr itemData = MenuItemDataPtr(new MenuItemData()) );

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
	* See the WidgetMenu sample project for a demonstration.
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
	* WidgetWindow derived class or the class derived from WidgetMenu. <br>
	* See e.g. WidgetMenu for an example. <br>
	* The reason to why we have this "id" is because the same event handler can be
	* defined for several menu items even in fact across menu objects, therefore
	* this number should be unique across the application.
	*/
	void appendItem(unsigned int id, const SmartUtil::tstring & text, MenuItemDataPtr itemData = MenuItemDataPtr(new MenuItemData()));

	template<typename DispatcherType>
	void appendItem(unsigned int id, const SmartUtil::tstring & text, const typename DispatcherType::F& f, MenuItemDataPtr itemData = MenuItemDataPtr(new MenuItemData())) {
		appendItem(id, text, itemData);
		callbacks.insert(std::make_pair(id, DispatcherType(f)));
	}

	void appendItem(unsigned int id, const SmartUtil::tstring & text, const IdDispatcher::F& f, MenuItemDataPtr itemData = MenuItemDataPtr(new MenuItemData())) {
		appendItem<IdDispatcher>(id, text, f, itemData);
	}

	void appendItem(unsigned int id, const SmartUtil::tstring & text, BitmapPtr image);

	template<typename DispatcherType>
	void appendItem(unsigned int id, const SmartUtil::tstring & text, const typename DispatcherType::F& f, BitmapPtr image) {
		MenuItemDataPtr itemData(new MenuItemData());
		if(ownerDrawn)
			itemData->Image = image;
		appendItem<DispatcherType>(id, text, f, itemData);
	}

	void appendItem(unsigned int id, const SmartUtil::tstring & text, const IdDispatcher::F& f, BitmapPtr image) {
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
	int getCount();

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
	/** A WidgetMenu can have a title, this function sets that title
	*/
	void setTitle( const SmartUtil::tstring & title, bool drawSidebar = false );

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
	SmartUtil::tstring getText( unsigned idOrPos, bool byPos );

	/// Sets the text of a specific menu item
	/** Which menu item you wish to set the text is defined by the "id"
	  * parameter of the function.
	  */
	void setText( unsigned id, const SmartUtil::tstring& text );

	/// Returns item data
	MenuItemDataPtr getData( int itemIndex );

	ObjectType getChild(UINT position);

	virtual ~WidgetMenu();

private:
	/// Constructor Taking pointer to parent
	explicit WidgetMenu( SmartWin::Widget * parent );

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
		const WidgetMenu* menu;

		// Item index in the menu
		// This is needed, because ID's for items
		// are not unique (although Windows claims)
		// e.g. we can have an item with ID 0,
		// that is either separator or popup menu
		int index;

		// Specifies if item is menu title
		bool isMenuTitleItem;

		// Contains item data
		MenuItemDataPtr data;

		// Wrapper  Constructor
		ItemDataWrapper( const WidgetMenu* menu_, int itemIndex, MenuItemDataPtr itemData, bool isTitleItem = false )
			: menu( menu_ )
			, index( itemIndex )
			, isMenuTitleItem( isTitleItem )
			, data( itemData )
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
	// work around for gcc
	std::vector< ObjectType > & itsChildrenRef;

	// its item data
	std::vector < ItemDataWrapper * > itsItemData;
	// work around for gcc
	std::vector < ItemDataWrapper * > & itsItemDataRef;

	HMENU itsHandle;

	Widget* itsParent;

	bool ownerDrawn;

	// Contains information about menu colors
	MenuColorInfo itsColorInfo;

	// Menu title
	SmartUtil::tstring itsTitle;

	// Menu title font
	FontPtr itsTitleFont;

	// if true title is drawn as sidebar
	bool drawSidebar;

	typedef std::map<unsigned, Widget::CallbackType> CallbackMap;
	CallbackMap callbacks;

	void addCommands();

	// Returns item index in the menu item list
	// If no item with specified id is found, - 1 is returned
	int getItemIndex( unsigned int id );
};

// end namespace SmartWin
}

#endif
#endif
