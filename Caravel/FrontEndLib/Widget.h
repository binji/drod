/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * The Initial Developer of the Original Code is
 * Caravel Software.
 * Portions created by the Initial Developer are Copyright (C) 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef WIDGET_H
#define WIDGET_H

#ifdef WIN32
#pragma warning(disable:4786)
#endif

//SUMMARY
//
//CWidget is an abstract base class from which all widgets derive.  A widget is 
//a graphical area which paints by certain rules and can optionally handle events.  
//Widgets may contain other widgets, and many operations take advantage of this 
//hierarchical grouping.  For example, moving one widget will move all of its
//children at the same time.
//
//Several drawing utility functions are implemented in CWidget.  It is convenient
//to put them here because nearly all drawing in this app must be done in the
//context of a widget.
//
//USAGE
//
//You can derive new classes from CWidget.  Typically, you would want widgets to
//appear on a screen, and this can be accomplished by constructing a new 
//CWidget-derived class and calling the AddWidget() method of the CScreen-derived
//class.  You can add a widget to any other widget, not just a CScreen-derived class.  
//Painting, loading, unloading, moving, hiding, and disabling of a parent widget 
//will affect all of its children.
//
//The sections that follow describe implementing a new CWidget-derived class.
//
//FOCUSABLE WIDGETS
//
//If you want your widget to be capable of receiving the focus, you should derive
//it from CFocusWidget instead of CWidget.  When a widget is derived from CFocusWidget, 
//its IsFocusable() method will return true instead of false.
//
//After focusable widgets have been added to a parent, they may be explicitly removed 
//and added from the focus list with calls to CEventHandlerWidget::AddFocusWidget()
//and RemoveFocusWidget().
//
//ANIMATED WIDGETS
//
//If you want your widget to be automatically animated between events, then you 
//should override CWidget::IsAnimated() to return true and CWidget::HandleAnimate()
//to perform the animation.  
//
//If you call AddWidget() to add a widget to a parent without an event-handler, 
//the widget will not be automatically animated.  For example, adding a CScalerWidget 
//to a CFrameWidget before the CFrameWidget is added to a CScreen, will result in the 
//CScalerWidget not being animated.  However, if the CScalerWidget was added to the 
//CFrameWidget AFTER CFrameWidget was added to the CScreen, the CScalerWidget
//would animate.
//
//After animated widgets have been added to a parent, they may be explicitly removed 
//and added from the animation list with calls to CEventHandlerWidget::AddAnimatedWidget()
//and RemoveAnimatedWidget().
//
//PAINTING
//
//You must implement the pure Paint() method in your CWidget-derived class.  When
//you want to draw something, call GetDestSurface() for the surface.  It goes against
//the widget design to draw directly to the screen, which may or may not be the 
//destination surface (See CWidget::SetDestSurface()).
//
//If you need to directly write to the pixel data, enclose your writes in a
//LockDestSurface()/UnlockDestSurface() pair.  Also, you must choose whether to make
//your routine safe for clipping or not.  Right now there is no clipping being done
//except by accident (debug errors).  You can override CWidget::PaintClipped() and
//put an "ASSERTP(false, "Can't paint clipped.")" statement in it.  This will designate 
//your widget as unsafe for clipping and keep it out of trouble.  If you are not directly 
//writing to the pixel data, your widget is safe for clipping.
//
//The last two lines of your CWidget::Paint() should probably be:
//
//  PaintChildren();
//  if (bUpdateRect) UpdateRect();
//
//The first line is not strictly necessary if your widget is not meant to contain other
//widgets, but adding the call adds just a tiny bit of overhead.  It makes your
//widget paint any contained widgets.
//
//The second is used by the painting machinery to handle all surface updates, and needs
//to be present.
//
//LOADING AND UNLOADING
//
//You only need to override Load() if your widget requires resource loading or has some
//failable initialization.  If you override Load(), follow this sequence:
//
//  Perform loading tasks for your widget.
//  If any failures occurred then return false.
//  "return LoadChildren();"
//
//If you override Load(), you should also override Unload() to provide a symmetrical
//unloading.  It should follow this sequence:
//
//  "UnloadChildren();"
//  Perform unloading tasks for your widget.
//
//Note that a widget may be constructed, and then loaded and unloaded MULTIPLE times
//before it is destructed.  This is under the control of the screen manager, which
//will leave some screens (and their children) in memory, and unload others to reduce 
//memory consumption.  You want to retain state information after an Unload(), but not
//resources.  For example, CTextBoxWidget will release the bitmap is uses to draw
//itself in Unload(), but will not release its text data until the destructor is called.
//
//HANDLING EVENTS
//
//If your widget is a child of an event-handling widget (CScreen and CDialog-derived 
//classes), it will receive events from an active event-handling widget.  The 
//CWidget::Handle*() methods will be called with information about the event.  You
//override the Handle*() methods that your widget should respond to.  See descriptions
//of the handlers below.
//
//Handlers are responsible for repainting the widget when it changes.  If your 
//handler is going to call an event notifier, such as OnSelectChange(), paint your 
//widget first.  This generally makes your widget appear to respond more quickly
//to user interaction.
//
//ACCEPTING TEXT ENTRY
//
//If your widget is meant to record a text entry in response to keypresses, then
//you should override CWidget::AcceptsTextEntry() to return true.  If your widget
//merely responds to keypresses, but doesn't convert them to text directly, then
//you shouldn't override AcceptsTextEntry().

#include "Colors.h"
#include <BackEndLib/Types.h>

#include <list>
using namespace std;

//Tag constants.
const DWORD TAG_UNSPECIFIED = 0L;
const DWORD TAG_QUIT = (DWORD)-1L;
const DWORD TAG_ESCAPE = (DWORD)-2L;
const DWORD TAG_SCREENSIZE = (DWORD)-3L;

//OK button has same tag value everywhere.
const DWORD TAG_OK = 9000L;

const DWORD TAG_FIRST_RESERVED = (DWORD)-3L;
#define IS_RESERVED_TAG(t) ((DWORD)(t) >= TAG_FIRST_RESERVED)

//Rect-related macros.
#define COPY_RECT(sr,dr) memcpy(&(dr), &(sr), sizeof(SDL_Rect))
#define CLEAR_RECT(r) memset(&(r), 0, sizeof(SDL_Rect))
#define IS_IN_RECT(x_,y_,r) ((x_) >= (r).x && (x_) < (r).x + (r).w && \
		(y_) >= (r).y && (y_) < (r).y + (r).h)
#define SET_RECT(r, x_, y_, w_, h_) (r).x = (x_); (r).y = (y_); (r).w = (w_); (r).h = (h_)
#define ARE_RECTS_EQUAL(r1, r2) ( (r1).x == (r2).x && (r1).y == (r2).y && \
		(r1).w == (r2).w && (r1).h == (r2).h )

//UI colors.
#define RGB_FOCUS		138, 126, 103
#define RGB_PLACEHOLDER	163, 143, 137

//Widget types.
//Add new enums as new CWidget-derived classes are created.
typedef enum tagWidgetType
{
	WT_Unspecified = -1,
	WT_Screen,
	WT_Button,
	WT_Dialog,
	WT_Face,
	WT_Label,
	WT_Map,
	WT_Room,
	WT_Frame,
	WT_OptionButton,
	WT_Slider,
	WT_ListBox,
	WT_TextBox,
	WT_ScrollingText,
	WT_Scaler,
	WT_TabbedMenu,
	WT_ObjectMenu,
    WT_ProgressBar,

    WT_Count
} WIDGETTYPE;

//Hotkey information.
typedef struct Hotkey {
	SDLKey key;
	DWORD tag;
} HOTKEY;

//*******************************************************************************
class CEffectList;
class CEventHandlerWidget;
class CScalerWidget;
class CScreenManager;
class CWidget
{
protected:
   friend class CEffectList;
	friend class CEventHandlerWidget;
	friend class CScalerWidget;
	friend class CScreenManager;

	CWidget(WIDGETTYPE eSetType, DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH);
	virtual ~CWidget();

	void			ClearChildren();
	CEventHandlerWidget * GetEventHandlerWidget() const;
	CWidget *		GetFirstSibling();
	CWidget *		GetLastSibling();
	CWidget *		GetNextSibling();
	CWidget *		GetPrevSibling();
	DWORD			GetHotkeyTag(const SDLKey key);
	void			GetScrollOffset(int &nOffsetX, int &nOffsetY) const;
	virtual bool	IsAnimated() const {return false;}
	Uint32			IsLocked() const;
	virtual bool	IsFocusable() const {return false;}
	bool			IsSelectable() const {return IsEnabled() && IsVisible() && IsFocusable();}
	bool			IsScrollOffset() const;
	virtual bool	Load();
	bool			LoadChildren();	
	void			PaintChildren(bool bUpdateRects = false);
	void			RemoveHotkey(const DWORD tag);
	void			SetParent(CWidget *const pSetParent);
	virtual void	Unload();
	void			UnloadChildren();
	
	//
	//Event handlers.
	//

	//Called between events for an animated widget (CWidget::IsAnimated() was overrode to 
	//return true).
	virtual void	HandleAnimate() { }

	//Called when the widget previously received a mouse down within its area, 
	//a mouse up has not yet occurred, and mouse motion is occurring.  The mouse pointer 
	//may be inside or outside of the widget area.
	virtual void	HandleDrag(const SDL_MouseMotionEvent &/*Motion*/) { }

	//Called when the widget is receiving a mouse down within its area.
	virtual	void	HandleMouseDown(const SDL_MouseButtonEvent &/*Button*/) { }

	//Called when the widget previously received a mouse down within its area,
	//a mouse up has not yet occurred, the mouse pointer is inside the widget area,
	//and a certain interval has passed.
	virtual	void	HandleMouseDownRepeat(const SDL_MouseButtonEvent &/*Button*/) { }

	//Called when the widget previously received a mouse down within its area,
	//and a mouse up is occurring.  The mouse pointer may or may not be within
	//the widget area.
	virtual	void	HandleMouseUp(const SDL_MouseButtonEvent &/*Button*/) { }

	//Called when the widget is receiving a mouse wheel scroll.
	virtual	void	HandleMouseWheel(const SDL_MouseButtonEvent &/*Button*/) { }

	//Called when the mouse pointer is moving inside the widget area.
	virtual	void	HandleMouseMotion(const SDL_MouseMotionEvent &/*Motion*/) { }

	//Called when the widget has the focus and a key is pressed.  Should return
	//true if this widget is generally affected by key down events, or false if
	//not.
	virtual void	HandleKeyDown(const SDL_KeyboardEvent &/*Key*/) { }

	//Called when the widget has the focus and a key is released.
	virtual void	HandleKeyUp(const SDL_KeyboardEvent &/*Key*/) { }

	bool			bIsLoaded;
	bool			bIsEnabled;
	DWORD			dwTagNo;
	int				x, y;
	UINT			w, h;
	int				nChildrenScrollOffsetX, nChildrenScrollOffsetY;
	WIDGETTYPE		eType;
	CWidget *	pParent;

	bool				bIsVisible;
	list<CWidget *>		Children;

	static SDL_Surface *	pPartsSurface; //visual components used by widgets
   static UINT wPartsSurfaceRefs;   //number of widgets open

private:
	void			RemoveAllHotkeys();

	HOTKEY *		pHotkeys;	//dynamic array of hotkey assignments
	UINT			nHotkeys;
   SDL_Surface *	pSurface;

public:
	virtual bool	AcceptsTextEntry() {return false;}
	void			AddHotkey(const SDLKey key, const DWORD tag);
	virtual CWidget *		AddWidget(CWidget *pNewWidget, bool bLoad = false);
	void			Center();
	bool			ContainsCoords(const int nX, const int nY) const;
	void			ClipWHToDest();
	virtual void	Disable() {bIsEnabled = false;}
	void			DrawPlaceholder();
	void			Enable() {bIsEnabled = true;}
	CEventHandlerWidget * GetParentEventHandlerWidget() const;
	void			GetRect(SDL_Rect &rect) const
			{rect.x = this->x; rect.y = this->y; rect.w = this->w; rect.h = this->h;}
	void			GetRectContainingChildren(SDL_Rect &ChildContainerRect) const;
	DWORD			GetTagNo() const {return this->dwTagNo;}
	WIDGETTYPE	GetType() const {return this->eType;}
	CWidget *	GetWidget(const DWORD dwTagNo, const bool bFindVisibleOnly=false);
	CWidget *	GetWidgetContainingCoords(const int nX, const int nY,
			WIDGETTYPE eWidgetType) const;
	int			GetX() const {return this->x;}
	int			GetY() const {return this->y;}
	UINT			GetW() const {return this->w;}
	UINT			GetH() const {return this->h;}

	virtual void	Hide(const bool /*bPaint*/ = true) {this->bIsVisible = false;}
	void			HideChildren();
	bool			IsActive() const {return IsEnabled() && IsVisible();}
	bool			IsEnabled() const {return bIsEnabled;}
	bool			IsInsideOfParent() const;
	bool			IsInsideOfRect(const int nX, const int nY, const UINT wW,
			const UINT wH) const;
	bool			IsLoaded() const {return this->bIsLoaded;}
	bool			IsVisible() const {return this->bIsVisible;}
	virtual void	Move(const int nSetX, const int nSetY);
	bool			OverlapsRect(const int nX, const int nY, const UINT wW,
			const UINT wH) const;
	virtual void	Paint(bool bUpdateRect = true)=0;
	virtual void	PaintClipped(const int nX, const int nY, const UINT wW, const UINT wH,
			const bool bUpdateRect = true);
	void			PaintClippedInsideParent(bool bUpdateRect = true);
	virtual void	RemoveWidget(CWidget *pRemoveWidget);
	virtual void	Resize(const UINT wSetW, const UINT wSetH);
	void			Scroll(const int dx, const int dy);
	void			ScrollAbsolute(const int nScrollX, const int nScrollY);
	void			Show() {this->bIsVisible = true;}
	void			ShowChildren();

	//SDL helper functions that act on screen surface.
	void			DrawCol(const int nX, const int nY, const UINT wH,
			const SURFACECOLOR &Color, SDL_Surface *pDestSurface = NULL);
	void			DrawFilledRect(const SDL_Rect &rect, const SURFACECOLOR &Color,
         SDL_Surface *pDestSurface=NULL);
	void			DrawRect(const SDL_Rect &rect, const SURFACECOLOR &Color,
         SDL_Surface *pDestSurface=NULL);
	void			DrawRow(const int nX, const int nY, const UINT wW,
			const SURFACECOLOR &Color, SDL_Surface *pDestSurface = NULL);
	void			GetDestSurfaceColor(Uint8 bytRed, Uint8 bytGreen, 
			Uint8 bytBlue, SURFACECOLOR &Color) const;
	SDL_Surface *	GetDestSurface() const;
	SDL_Surface *	LockDestSurface(SDL_Surface *pDestSurface=NULL);
	void			SetChildrenDestSurface(SDL_Surface *pSurface);
	void			SetDestSurface(SDL_Surface *pSurface);
	void			UnlockDestSurface(SDL_Surface *pDestSurface=NULL);
	virtual void	UpdateRect() const;
};

typedef list<CWidget *>::const_iterator WIDGET_ITERATOR;

void SetWidgetScreenSurface(SDL_Surface *pSetScreenSurface);
SDL_Surface * GetWidgetScreenSurface();

#endif //...#ifndef WIDGET_H

// $Log: Widget.h,v $
// Revision 1.11  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.10  2003/10/01 15:57:30  mrimer
// Fixed UpdateRect() method inheritance from CWidget to CScreen.
//
// Revision 1.9  2003/08/16 00:45:43  mrimer
// Augmented surface methods with optional surface parameter.
//
// Revision 1.8  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.7  2003/07/09 11:56:51  schik
// Made PaintClipped() arguments consistent among derived classes (VS.NET warnings)
//
// Revision 1.6  2003/07/07 23:37:55  mrimer
// Made pParent non-const.
//
// Revision 1.5  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/09 19:21:02  mrimer
// Made Add(Remove)FocusWidget methods public, like they should be.
//
// Revision 1.3  2003/05/28 22:58:49  erikh2000
// Corrected an error in calculated widget offsets.
// Added WT_ProgressBar.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.45  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.44  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.43  2003/04/17 21:02:29  mrimer
// Added HandleMouseWheel().
//
// Revision 1.42  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.41  2003/01/08 00:52:28  mrimer
// Added an include.
//
// Revision 1.40  2002/12/22 02:16:04  mrimer
// Added GetParentEventHandlerWidget().
//
// Revision 1.39  2002/11/15 02:15:38  mrimer
// Added TabbedMenu and ObjectMenu widgets.
// Made several parameters const.
//
// Revision 1.38  2002/10/21 20:17:21  mrimer
// Modified GetWidget() to optionally retrieve only visible widgets.
//
// Revision 1.37  2002/10/03 19:02:41  mrimer
// Removed friend to itself.
//
// Revision 1.36  2002/09/24 21:54:19  mrimer
// Removed TAG_OK from reserved tag check.
//
// Revision 1.35  2002/09/24 21:39:54  mrimer
// Added TAG_OK as a reserved tag.
//
// Revision 1.34  2002/09/04 20:40:15  mrimer
// Moved UpdateRect(rect) form CWidget to CScreen.
//
// Revision 1.33  2002/09/03 23:50:26  mrimer
// Overloaded UpdateRect to update only part of the screen.
//
// Revision 1.32  2002/09/03 21:41:48  erikh2000
// ClearChildren() is more robust.
// Scrolling offsets can be applied to all children of a widget and several methods taken them into account.
//
// Revision 1.31  2002/07/21 01:40:52  erikh2000
// Documented usage of AcceptsTextEntry().
//
// Revision 1.30  2002/07/20 23:20:16  erikh2000
// A widget can now be animated.  An animated widget is automatically added to an event handler's animation list.  The event handler calls a new HandleAnimate() method between events.
// Changed LockDestSurface() to return a point to dest surface for convenience.
// Added new documentation to top of Widget.h.
//
// Revision 1.29  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.28  2002/07/17 21:18:51  mrimer
// Fixed widgets appearing at wrong times with dialog boxes.
//
// Revision 1.27  2002/07/09 23:15:09  mrimer
// Added dimension query functions.  Added WT_ScrollingText type.
//
// Revision 1.26  2002/07/05 10:40:04  erikh2000
// Wrote usage comments.
// Added overridable event-handling methods.
// Changed AddWidget() and RemoveWidget() so that they affect the focus list.
// Wrote some methods for accessing sibling widgets.
//
// Revision 1.25  2002/06/21 22:29:57  mrimer
// Made Disable() virtual function.
//
// Revision 1.24  2002/06/21 03:32:24  erikh2000
// Added ShowChildren() and HideChildren() methods.
//
// Revision 1.23  2002/06/20 00:57:51  erikh2000
// Added method to draw a placeholder rect over the widget area.
//
// Revision 1.22  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.21  2002/06/14 01:08:13  erikh2000
// Made several changes to CWidget methods that make them use destination surface instead of screen surface.  Renamed some methods to reflect these changes.
// Made CWidget::pSurface value of NULL represent the screen surface, so that a Widget dest surface for the screen will not become invalid during fullscreen/windowed switches.
//
// Revision 1.20  2002/06/11 22:45:53  mrimer
// Changed screens to destination surfaces.
// Added TAG_SCREENSIZE.
//
// Revision 1.19  2002/06/07 22:56:26  mrimer
// Made AddHotkey() public.
//
// Revision 1.18  2002/06/05 03:09:07  mrimer
// Added focus graphic.  Added widget focusability and keyboard control.
//
// Revision 1.17  2002/06/03 22:53:56  mrimer
// Made RemoveAllHotkeys() private.
//
// Revision 1.16  2002/05/31 23:45:42  mrimer
// Added hotkey support.
//
// Revision 1.15  2002/05/20 18:38:46  mrimer
// Added enabled/disabled flag to widget.
//
// Revision 1.14  2002/05/16 23:09:50  mrimer
// Added FlashMessageEffect and level cleared message.
//
// Revision 1.13  2002/05/12 03:24:38  erikh2000
// Change AddWidget() so that you could optionally add a widget and load it in one call.
// Added RemoveWidget() method.
// Added a second reserved tag--TAG_ESCAPE.
//
// Revision 1.12  2002/05/10 22:44:08  erikh2000
// Renamed "InitWidget()" to "SetWidgetScreenSurface()" to more accurately describe usage.
//
// Revision 1.11  2002/04/29 00:40:37  erikh2000
// Added some SDL_Rect macros.
//
// Revision 1.10  2002/04/26 20:46:36  erikh2000
// Added new WT_TextBox enum.
//
// Revision 1.9  2002/04/25 18:30:22  mrimer
// Added functions to switch between windowed and full-screen modes.
//
// Revision 1.8  2002/04/25 09:35:11  erikh2000
// Added WT_ListBox enumeration.
//
// Revision 1.7  2002/04/24 08:17:19  erikh2000
// Added new enum values for sliders and option buttons.
//
// Revision 1.6  2002/04/19 21:47:19  erikh2000
// Added GetWidgetContainingCoords() method.
//
// Revision 1.5  2002/04/14 00:41:24  erikh2000
// Added DrawCol() and DrawRow() methods.
// Changed parameters of methods to SURFACECOLOR for consistency.
// Moved GetSurfaceColor() to a mainline function in Colors.cpp, and created a new GetScreenSurfaceColor() method that calls it.
//
// Revision 1.4  2002/04/13 19:34:17  erikh2000
// Widget position is now set with relative coords and child widgets are moved with widget.
// Widget has a type variable that must be set at construction.
// Added some coordinate/rect calculation methods.
// Added clipping during paints.
//
// Revision 1.3  2002/04/12 05:20:13  erikh2000
// Added GetRect() method.
//
// Revision 1.2  2002/04/09 10:05:41  erikh2000
// Fixed revision log macro.
//
