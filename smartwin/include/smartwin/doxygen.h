// $Revision: 1.6 $
/*  Definitions for doxygen
	(Never included by c++ compilers)
*/

/** \defgroup WidgetControls Widgets
  */

/** \defgroup AspectClasses Aspect Classes
  */

/** \defgroup GlobalStuff Global typedefs and functions
  * Global functions and typedefs or things in general not belonging to any
  * particular class.
  */

/** \defgroup WidgetLayout Layout of widgets in windows
  * \htmlinclude layout.html
  */

/** \defgroup SubclassDialog Dialog Subclassing Functions
  */

/**
  * \defgroup eventsSignatures  Event Signatures
  *
  * See MessageMapControl.h
  *
*/


/** \defgroup EventHandlersAspectClickable Event Handlers for AspectClickable
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectDblClickable Event Handlers for AspectDblClickable
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectKeyPressed Event Handlers for AspectKeyPressed
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectMouseClicks Event Handlers for AspectMouseClicks
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectPainting Event Handlers for AspectPainting
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectSelection Event Handlers for AspectSelection
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectSizable Event Handlers for AspectSizable
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectEnabled Event Handlers for AspectEnabled
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectEraseBackground Event Handlers for AspectEraseBackground
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectActivate Event Handlers for AspectActivate
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectBackgroundColor Event Handlers for AspectBackgroundColor
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectAspectFocus Event Handlers for AspectFocus
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectScrollable Event Handlers for AspectScrollable
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectText Event Handlers for AspectText
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectUpdate Event Handlers for AspectUpdate
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectVisible Event Handlers for AspectVisible
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersAspectRightClickable Event Handlers for AspectRightClickable
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersWidgetDataGrid Event Handlers for WidgetDataGrid
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersWidgetMenu Event Handlers for WidgetMenu
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersWidgetToolbar Event Handlers for WidgetToolbar
  * \EventHandlerGenericInfo
  */

/** \defgroup EventHandlersWidgetTreeView Event Handlers for WidgetTreeView
  * \EventHandlerGenericInfo
  */


/** \mainpage Documentation for SmartWin++
  * \htmlinclude guide.html
  */

/**
  * \page license SmartWin++ license
  * \htmlinclude license.html
*/

/**
  * \page msvc71  MS Visual C++ 7.1
  * \htmlinclude msvc71.html
*/

/**
  * \page devcpp  DEV C++
  * \htmlinclude devcpp.html
*/

/**
  * \page msvctoolkit  MSVC 2003 toolkit
  * \htmlinclude msvcToolkit.html
*/

/**
  * \page eclipse  Eclipse C++ Development Environment
  * \htmlinclude eclipse.html
*/

/**
  * \page linuxwine  Using Winelib to run on Linux
  * \htmlinclude linuxwine.html
*/

/**
  * \page samplecode1  Hello World (small)
   * 
   * This is a program to show the basics of SmartWin++
   *
   * \include main.cpp
   * 
   * This program compiles into an executable of 155 KB without any extra DLLs needed.
   * Here is a screen shot of two instances running:
   *
   * \image html hello.jpg
   *
   * The two message boxes either say "Hello !" or "Hello World !" depending on the
   * Global checkbox.  The layout logic puts the button either to the right or below
   * the checkbox depending on the window shape.
   *
   * Notice the way events are handled in SmartWin++: you specify a member function
   * that executes when the event happens.
   *
   * Notice also that the button and the checkbox's size are set once.  But their
   * runtime position depends on how the window is resized by the user. The resource
   * editor was not used in this sample. although SmartWin++ can use the MSVC
   * resource editor if you want to.
   *
   * <a href="samplecode2.html"> Here is a larger example </a>
   * There are 40 other source code samples.
*/

/**
  * \page samplecode2  Hello World (big)
   * This is a program to show most of the features being used together.
   *
   * \include hello.cpp
   * 
   * This program compiles into an executable of 268 KB without any extra DLLs needed.
   *
   * \image html helloSmartWin.jpg
*/
