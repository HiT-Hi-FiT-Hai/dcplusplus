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
#ifndef Tree_h
#define Tree_h

#include "../Widget.h"
#include "../Rectangle.h"
#include "../resources/ImageList.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectCollection.h"
#include "../aspects/AspectColor.h"
#include "../aspects/AspectData.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectSelection.h"
#include "Control.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// TreeView class
 /** \ingroup WidgetControls
   * \WidgetUsageInfo
   * \image html treeview.PNG
   * A Tree is a treview control, like for instance the documentation to
   * SmartWin which you are probably reading right now would ( in the web version )
   * have a tree view to the left. <br>
   * Another good example of a tree view is the Explorer of Windows, it has a tree
   * view to the left where you can see the different directories.
   */

class Tree :
	public Control,
	// Aspects
	public AspectClickable< Tree >,
	public AspectCollection<Tree, HTREEITEM>,
	public AspectColor<Tree>,
	public AspectData<Tree, HTREEITEM>,
	public AspectDblClickable< Tree >,
	public AspectFocus< Tree >,
	public AspectFont< Tree >,
	public AspectSelection< Tree, HTREEITEM >
{
protected:
	struct Dispatcher
	{
		typedef std::tr1::function<bool (const SmartUtil::tstring&)> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			bool update = false;
			NMTVDISPINFO * nmDisp = reinterpret_cast< NMTVDISPINFO * >( msg.lParam );
			if ( nmDisp->item.pszText != 0 )
			{
				SmartUtil::tstring newText = nmDisp->item.pszText;
				update = f(newText);
			}
			return update ? TRUE : FALSE;
		}

		F f;
	};

	friend class WidgetCreator< Tree >;
	friend class AspectCollection<Tree, HTREEITEM>;
	friend class AspectColor<Tree>;
	friend class AspectData<Tree, HTREEITEM>;
	friend class AspectSelection<Tree, HTREEITEM>;
	
public:
	/// Class type
	typedef Tree ThisType;

	/// Object type
	typedef ThisType* ObjectType;

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

		/// Fills with default parameters
		Seed();
	};

	/// Inserts a "node" into the TreeView
	/** The return value from a call to this function is a Node. <br>
	  * If you later wish to inserts CHILDREN to that node, pass the return value
	  * from the first call as the second parameter into this function. <br>
	  * If you wish to insert a ( a TreeView can have several "root" nodes ) "root"
	  * node then don't pass anything as the second parameter. ( or pass Node() )
	  * <br>
	  * The "param" parameter ( optionally ) is a unique unsigned integer which must
	  * be higher than 0 and can later be used to retrieve unique identification of
	  * which item was e.g. selected etc... <br>
	  * Especially useful when text of nodes is not unique or text might change.
	  * The "iconIndex" optionally specifies the icon index of the item in the
	  * associated image list, if there is one. <br>
	  * The "selectedIconIndex" optionally specifies the icon index of the item in the
	  * selected state (if not specified or -1, it defaults to the iconIndex)
	  */
	HTREEITEM insert( const SmartUtil::tstring & text, HTREEITEM parent = NULL, LPARAM param = 0, int iconIndex = - 1, int selectedIconIndex = - 1 );

	HTREEITEM getNext(HTREEITEM node, unsigned flag);

	HTREEITEM getChild(HTREEITEM node);
	
	HTREEITEM getNextSibling(HTREEITEM node);
	
	HTREEITEM getParent(HTREEITEM node);
	
	HTREEITEM getRoot();
	
	ScreenCoordinate getContextMenuPos();
	
	void expand(HTREEITEM node);
	
	void select(const ScreenCoordinate& pt);
	
	HTREEITEM hitTest(const ScreenCoordinate& pt);
	
	Rectangle getItemRect(HTREEITEM item);
	/// Deletes just the children of a "node" from the TreeView< br >
	/** Cycles through all the children of node, and deletes them. <br>
	  * The node itself is preserved.
	  */
	void eraseChildren( HTREEITEM node );

	/// Edits the label of the "node"
	/** The label of the node is put to edit modus. The node edited must be visible.
	  */
	void editLabel( HTREEITEM node );

	/// Ensures that the node is visible.
	/** Nodes may not be visible if they are indicated with a + , or do not appear in
	  * the window due to scrolling.
	  */
	void ensureVisible( HTREEITEM node );

	/// Adds a plus/minus sign in front of items.
	/** To add items also at the root node call setLinesAtRoot
	  */
	void setHasButtons( bool value = true );

	/// Adds lines in front of the root item.
	/** Is ignored if you don't also call setHasLines.
	  */
	void setLinesAtRoot( bool value = true );

	/// Adds lines in front of items.
	/** To set lines in front of also the root item call setLinesAtRoot.
	  */
	void setHasLines( bool value = true );

	/// Add Track Selection to the Tree View Control
	/** Pass true to let the Tree View Control go into Track Selction modus which is
	  * quite a neat visual style.
	  */
	void setTrackSelect( bool value = true );

	/// Enables full - row selection in the tree view.
	/** The entire row of the selected item is highlighted, and clicking anywhere on
	  * an item's row causes it to be selected. <br>
	  * This style is mutually exclusive with setHasLines and setLinesAtRoot
	  */
	void setFullRowSelect( bool value = true );

	/// Allows the user to edit the labels of tree - view items.
	/** Note that if the onValidate event handler is defined it will override this
	  * function no matter what
	  */
	void setEditLabels( bool value = true );

	/// Set the normal image list for the Tree View.
	/** normalImageList is the image list that contains the images
	  * for the selected and nonselected item icons.
	  */
	void setNormalImageList( ImageListPtr normalImageList );

	/// Set the state image list for the Tree View.
	/** stateImageList is the image list that contains the images
	  * for the item states.
	  */
	void setStateImageList( ImageListPtr stateImageList );
	/// Returns the text of the current selected node
	/** Returns the text of the current selected node in the tree view.
	  */
	SmartUtil::tstring getSelectedText();

	/// Returns the text of a particular node
	/** Returns the text of a particular node.
	  */
	SmartUtil::tstring getText( HTREEITEM node );

	/// \ingroup EventHandlersTree
	/// Sets the event handler for what function to be called when a label is edited.
	/** Event handler signature is must be "bool foo( Tree *,
	  * SmartUtil::tstring & )" and it must be contained as a member of the class
	  * that is defined as the EventHandlerClass, normally either the WidgetWindow
	  * derived class or the class derived from Tree. <br>
	  * Return true from your event handler if you wish the label to actually become
	  * updated or false if you want to disallow the item to actually become updated!
	  */
	void onValidateEditLabels(const Dispatcher::F& f) {
		addCallback(
			Message( WM_NOTIFY, TVN_ENDLABELEDIT ), Dispatcher(f)
		);		
	}

	// Contract needed by AspectClickable Aspect class
	static const Message & getSelectionChangedMessage();

	// Contract needed by AspectClickable Aspect class
	static const Message& getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static const Message& getDblClickMessage();

	/// Returns true if fired, else false
	virtual bool tryFire( const MSG & msg, LRESULT & retVal );
		
	/// Actually creates the TreeView
	/** You should call WidgetFactory::createTreeView if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	void create( const Seed & cs = Seed() );

protected:
	// Constructor Taking pointer to parent
	explicit Tree( Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~Tree()
	{}

private:
	ImageListPtr itsNormalImageList;
	ImageListPtr itsStateImageList;
	
	// AspectData
	LPARAM getDataImpl(HTREEITEM item);
	void setDataImpl(HTREEITEM item, LPARAM data);

	// AspectCollection
	void eraseImpl( HTREEITEM node );
	void clearImpl();
	size_t sizeImpl() const;
	
	// AspectColor
	void setColorImpl(COLORREF text, COLORREF background);

	// AspectSelection
	HTREEITEM getSelectedImpl() const;
	void setSelectedImpl( HTREEITEM item );
	size_t countSelectedImpl() const;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline HTREEITEM Tree::getNext( HTREEITEM node, unsigned flag ) {
	return TreeView_GetNextItem( this->handle(), node, flag );
}

inline HTREEITEM Tree::getChild(HTREEITEM node) {
	return TreeView_GetChild(this->handle(), node);
}

inline HTREEITEM Tree::getNextSibling(HTREEITEM node) {
	return TreeView_GetNextSibling(this->handle(), node);
}

inline HTREEITEM Tree::getParent(HTREEITEM node) {
	return TreeView_GetParent(this->handle(), node);
}

inline HTREEITEM Tree::getRoot() {
	return TreeView_GetRoot(this->handle());
}

inline void Tree::setColorImpl(COLORREF text, COLORREF background) {
	TreeView_SetTextColor(this->handle(), text);
	TreeView_SetBkColor(this->handle(), background);
}

inline Rectangle Tree::getItemRect(HTREEITEM item) {
	RECT rc;
	TreeView_GetItemRect(this->handle(), item, &rc, TRUE);
	return rc;
}

inline HTREEITEM Tree::hitTest(const ScreenCoordinate& pt) {
	ClientCoordinate cc(pt, this);
	TVHITTESTINFO tvhti = { cc.getPoint() };
	return TreeView_HitTest(this->handle(), &tvhti);
}

inline void Tree::expand(HTREEITEM node) {
	TreeView_Expand(this->handle(), node, TVE_EXPAND);
}

inline void Tree::clearImpl() {
	TreeView_DeleteAllItems( this->handle() );
}

inline void Tree::eraseImpl( HTREEITEM node ) {
	TreeView_DeleteItem( this->handle(), node );
}

inline size_t Tree::sizeImpl() const {
	return static_cast<size_t>(TreeView_GetCount(this->handle()));
}

inline void Tree::editLabel( HTREEITEM node ) {
	TreeView_EditLabel( this->handle(), node );
}

inline void Tree::ensureVisible( HTREEITEM node ) {
	TreeView_EnsureVisible( this->handle(), node );
}

inline void Tree::setHasButtons( bool value ) {
	this->Widget::addRemoveStyle( TVS_HASBUTTONS, value );
}

inline void Tree::setLinesAtRoot( bool value ) {
	this->Widget::addRemoveStyle( TVS_LINESATROOT, value );
}

inline void Tree::setHasLines( bool value ) {
	this->Widget::addRemoveStyle( TVS_HASLINES, value );
}

inline void Tree::setTrackSelect( bool value ) {
	this->Widget::addRemoveStyle( TVS_TRACKSELECT, value );
}

inline void Tree::setFullRowSelect( bool value ) {
	this->Widget::addRemoveStyle( TVS_FULLROWSELECT, value );
}

inline void Tree::setEditLabels( bool value ) {
	this->Widget::addRemoveStyle( TVS_EDITLABELS, value );
}

inline const Message & Tree::getSelectionChangedMessage() {
	static const Message retVal( WM_NOTIFY, TVN_SELCHANGED );
	return retVal;
}

inline const Message & Tree::getClickMessage() {
	static const Message retVal( WM_NOTIFY, NM_CLICK );
	return retVal;
}

inline const Message & Tree::getDblClickMessage() {
	static const Message retVal( WM_NOTIFY, NM_DBLCLK );
	return retVal;
}

inline HTREEITEM Tree::getSelectedImpl() const {
	return TreeView_GetSelection( this->handle() );
}

inline void Tree::setSelectedImpl(HTREEITEM item) {
	TreeView_SelectItem( this->handle(), item );
}

inline size_t Tree::countSelectedImpl() const {
	return getSelected() == NULL ? 0 : 1;
}

inline Tree::Tree( Widget * parent )
	: ControlType( parent )
{
}

// end namespace SmartWin
}

#endif
