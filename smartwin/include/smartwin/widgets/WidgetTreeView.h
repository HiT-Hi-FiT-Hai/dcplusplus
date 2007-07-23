// $Revision: 1.36 $
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
#ifndef WidgetTreeView_h
#define WidgetTreeView_h

#include "../BasicTypes.h"
#include "../ImageList.h"
#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectRightClickable.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"

#include <commctrl.h>

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// One "node" in the TreeView.
 /** Used e.g. when inserting nodes into the TreeView. <br>
   * When you add a node by calling WidgetTreeView::insertNode the return value is an
   * instance of this class, if you later wish to insert CHILDREN to that very node,
   * then use the returned node from your first call as the second parameter to the
   * insertNode function.
   */
struct TreeViewNode
{
	TreeViewNode() : handle( NULL )
	{}

	TreeViewNode(HTREEITEM handle_) : handle( handle_ )
	{}

	HTREEITEM handle;
	
	operator HTREEITEM() { return handle; }
};

/// TreeView class
 /** \ingroup WidgetControls
   * \WidgetUsageInfo
   * \image html treeview.PNG
   * A WidgetTreeView is a treview control, like for instance the documentation to
   * SmartWin which you are probably reading right now would ( in the web version )
   * have a tree view to the left. <br>
   * Another good example of a tree view is the Explorer of Windows, it has a tree
   * view to the left where you can see the different directories.
   */

class WidgetTreeView :
	public MessageMapPolicy< Policies::Subclassed >,
	
	// Aspects
	public AspectBorder< WidgetTreeView >,
	public AspectClickable< WidgetTreeView >,
	public AspectDblClickable< WidgetTreeView >,
	public AspectEnabled< WidgetTreeView >,
	public AspectFocus< WidgetTreeView >,
	public AspectFont< WidgetTreeView >,
	public AspectKeyboard< WidgetTreeView >,
	public AspectRaw< WidgetTreeView >,
	public AspectRightClickable< WidgetTreeView >,
	public AspectSelection< WidgetTreeView >,
	public AspectSizable< WidgetTreeView >,
	public AspectVisible< WidgetTreeView >
{
protected:
	struct Dispatcher
	{
		typedef std::tr1::function<bool (const SmartUtil::tstring&)> F;

		Dispatcher(const F& f_) : f(f_) { }

		HRESULT operator()(private_::SignalContent& params) {
			bool update = false;
			NMTVDISPINFO * nmDisp = reinterpret_cast< NMTVDISPINFO * >( params.Msg.LParam );
			if ( nmDisp->item.pszText != 0 )
			{
				SmartUtil::tstring newText = nmDisp->item.pszText;
				update = f(newText);
			}
			return update ? TRUE : FALSE;
		}

		F f;
	};

	friend class WidgetCreator< WidgetTreeView >;
public:
	/// Class type
	typedef WidgetTreeView ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	/// Seed class
	 /** This class contains all of the values needed to create the widget. It also
	   * knows the type of the class whose seed values it contains. Every widget
	   * should define one of these.
	   */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetTreeView::ThisType WidgetType;

		FontPtr font;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

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
	TreeViewNode insertNode( const SmartUtil::tstring & text, const TreeViewNode & parent = TreeViewNode(), unsigned param = 0, int iconIndex = - 1, int selectedIconIndex = - 1 );

	/// Returns the node with a relationship in flags to the specified node.
	/** The node specified comes from insertNode< br >
	  * Flag definitions : < br >
	  * TVGN_CARET...........Retrieves the currently selected item.< br >
	  * TVGN_CHILD...........Retrieves the first child node of the specified node.< br >
	  * TVGN_DROPHILITE......Retrieves the target mode of a drag - and - drop operation.< br >
	  * TVGN_FIRSTVISIBLE....Retrieves the first visible node.
	  * TVGN_NEXT............Retrieves the next sibling node. ( TVGN_CHILD is the first )< br >
	  * TVGN_NEXTVISIBLE.....Retrieves the next visible node after the specified node.< br >
	  * TVGN_PARENT..........Retrieves the parent of the specified node.< br >
	  * TVGN_PREVIOUS........Retrieves the previous sibling node.< br >
	  * TVGN_PREVIOUSVISIBLE.Retrieves the first visible node that precedes the specified node.< br >
	  * TVGN_ROOT............Retrieves the topmost or very first node of the tree - view control.< br >
	  */
	bool getNode( const TreeViewNode & node, unsigned flag, TreeViewNode & resultNode );

	/// Deletes every node in the tree.
	/** Every node and all of its children are deleted.
	  */
	void DeleteAllItems();

	/// Deletes a "node" and its children from the TreeView< br >
	/** The node and all of its children are deleted.
	  */
	void deleteNode( const TreeViewNode & node );

	/// Deletes just the children of a "node" from the TreeView< br >
	/** Cycles through all the children of node, and deletes them. <br>
	  * The node itself is preserved.
	  */
	void deleteChildrenOfNode( const TreeViewNode & node );

	/// Edits the label of the "node"
	/** The label of the node is put to edit modus. The node edited must be visible.
	  */
	void editLabel( const TreeViewNode & node );

	/// Ensures that the node is visible.
	/** Nodes may not be visible if they are indicated with a + , or do not appear in
	  * the window due to scrolling.
	  */
	void ensureVisible( const TreeViewNode & node );

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
	SmartUtil::tstring getSelectedItemText();

	/// Returns the text of a particular node
	/** Returns the text of a particular node.
	  */
	SmartUtil::tstring getText( const TreeViewNode & node );

	/// Returns the param of the current selected node
	/** The return value is a unique application defined unsigned number ( optionally
	  * ) given when inserting nodes. <br>
	  * Note! <br>
	  * It is pointless calling this function if no param was given when inserting
	  * the nodes. <br>
	  * 0 is special case indicating failure
	  */
	virtual int getSelectedIndex() const;

	/// Sets the currently selected node
	/** The parameter given is the param given when inserting the nodes <br>
	  * Note! <br>
	  * It is pointless calling this function if no param was given when inserting
	  * the nodes.
	  */
	virtual void setSelectedIndex( int idx );

	/// \ingroup EventHandlersWidgetTreeView
	/// Sets the event handler for what function to be called when a label is edited.
	/** Event handler signature is must be "bool foo( WidgetTreeView *,
	  * SmartUtil::tstring & )" and it must be contained as a member of the class
	  * that is defined as the EventHandlerClass, normally either the WidgetWindow
	  * derived class or the class derived from WidgetTreeView. <br>
	  * Return true from your event handler if you wish the label to actually become
	  * updated or false if you want to disallow the item to actually become updated!
	  */
	void onValidateEditLabels(const Dispatcher::F& f) {
		setCallback(
			Message( WM_NOTIFY, TVN_ENDLABELEDIT ), Dispatcher(f)
		);		
	}

	// Contract needed by AspectClickable Aspect class
	static Message & getSelectionChangedMessage();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static Message & getDblClickMessage();

	/// Actually creates the TreeView
	/** You should call WidgetFactory::createTreeView if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	static bool isValidSelectionChanged( LPARAM lPar )
	{ return true;
	}

protected:
	// Constructor Taking pointer to parent
	explicit WidgetTreeView( SmartWin::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~WidgetTreeView()
	{}

	private:
		ImageListPtr itsNormalImageList;
		ImageListPtr itsStateImageList;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetTreeView::Seed::Seed()
{
	 * this = WidgetTreeView::getDefaultSeed();
}

inline bool WidgetTreeView::getNode( const TreeViewNode & node, unsigned flag, TreeViewNode & resultNode )
{
	resultNode.handle = TreeView_GetNextItem( this->Widget::itsHandle, node.handle, flag );
	return ( NULL != resultNode.handle );
}

inline void WidgetTreeView::DeleteAllItems()
{
	TreeView_DeleteAllItems( this->Widget::itsHandle );
}

inline void WidgetTreeView::deleteNode( const TreeViewNode & node )
{
	TreeView_DeleteItem( this->Widget::itsHandle, node.handle );
}

inline void WidgetTreeView::editLabel( const TreeViewNode & node )
{
	TreeView_EditLabel( this->Widget::itsHandle, node.handle );
}

inline void WidgetTreeView::ensureVisible( const TreeViewNode & node )
{
	TreeView_EnsureVisible( this->Widget::itsHandle, node.handle );
}

inline void WidgetTreeView::setHasButtons( bool value )
{
	this->Widget::addRemoveStyle( TVS_HASBUTTONS, value );
}

inline void WidgetTreeView::setLinesAtRoot( bool value )
{
	this->Widget::addRemoveStyle( TVS_LINESATROOT, value );
}

inline void WidgetTreeView::setHasLines( bool value )
{
	this->Widget::addRemoveStyle( TVS_HASLINES, value );
}

inline void WidgetTreeView::setTrackSelect( bool value )
{
	this->Widget::addRemoveStyle( TVS_TRACKSELECT, value );
}

inline void WidgetTreeView::setFullRowSelect( bool value )
{
	this->Widget::addRemoveStyle( TVS_FULLROWSELECT, value );
}

inline void WidgetTreeView::setEditLabels( bool value )
{
	this->Widget::addRemoveStyle( TVS_EDITLABELS, value );
}

inline void WidgetTreeView::setNormalImageList( ImageListPtr imageList )
{
	  itsNormalImageList = imageList;
	  TreeView_SetImageList( this->Widget::handle(), imageList->getImageList(), TVSIL_NORMAL );
}

inline void WidgetTreeView::setStateImageList( ImageListPtr imageList )
{
	  itsStateImageList = imageList;
	  TreeView_SetImageList( this->Widget::handle(), imageList->getImageList(), TVSIL_STATE );
}

inline SmartUtil::tstring WidgetTreeView::getSelectedItemText()
{
	TreeViewNode selNode;
	HTREEITEM hSelItem = TreeView_GetSelection( this->Widget::itsHandle );
	selNode.handle = hSelItem;
	SmartUtil::tstring retVal = getText( selNode );
	return retVal;
}

inline SmartUtil::tstring WidgetTreeView::getText( const TreeViewNode & node )
{
	TVITEMEX item;
	item.mask = TVIF_HANDLE | TVIF_TEXT;
	item.hItem = node.handle;
	TCHAR buffer[1024];
	buffer[0] = '\0';
	item.cchTextMax = 1022;
	item.pszText = buffer;
	if ( TreeView_GetItem( this->Widget::itsHandle, & item ) )
	{
		SmartUtil::tstring retVal( buffer );
		return retVal;
	}
	return _T( "" );
}

inline int WidgetTreeView::getSelectedIndex() const
{
	HTREEITEM hSelItem = TreeView_GetSelection( this->Widget::itsHandle );
	TVITEM item;
	ZeroMemory( & item, sizeof( TVITEM ) );
	item.mask = TVIF_HANDLE | TVIF_PARAM;
	item.hItem = hSelItem;
	if ( TreeView_GetItem( this->Widget::itsHandle, & item ) )
		return static_cast< unsigned >( item.lParam );
	return 0;
}

inline void WidgetTreeView::setSelectedIndex( int idx )
{
	TVITEM item;
	item.mask = TVIF_PARAM;
	item.lParam = idx;
	if ( TreeView_GetItem( this->Widget::itsHandle, & item ) == FALSE )
	{
		throw xCeption( _T( "Couldn't find given item" ) );
	}
	TreeView_Select( this->Widget::itsHandle, item.hItem, TVGN_FIRSTVISIBLE );
}

inline Message & WidgetTreeView::getSelectionChangedMessage()
{
	static Message retVal = Message( WM_NOTIFY, TVN_SELCHANGED );
	return retVal;
}

inline Message & WidgetTreeView::getClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, NM_CLICK );
	return retVal;
}

inline Message & WidgetTreeView::getDblClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, NM_DBLCLK );
	return retVal;
}

inline WidgetTreeView::WidgetTreeView( Widget * parent )
	: PolicyType( parent )
{
	// Can't have a list view without a parent...
	xAssert( parent, _T( "Cant have a WidgetTreeView without a parent..." ) );
}

// end namespace SmartWin
}

#endif
