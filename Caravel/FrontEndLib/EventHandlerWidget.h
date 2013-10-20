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

//SUMMARY
//
//A class derived from CEventHandlerWidget is responsible for handling events, sending
//event notifications to derived classes, and providing control methods for selecting
//focusable widgets.
//
//USAGE
//
//Derive your new widget class from CEventHandlerWidget or the two classes which
//are derived from CEventHandlerWidget--CDialogWidget and CScreenWidget.  Default UI 
//behaviour will be given to your class.  Overide the event notifier methods for
//events you are interested in.  For example, if you want to play a song when the user
//presses the "Play Song" button, override OnClick() in your derived class.
//
//Event notifiers that apply to a specific widget will receive a dwTagNo parameter
//indicating to which widget the event applied.  Event notifiers corresponding directly 
//to an SDL event will receive SDL event information in a parameter.  See descriptions 
//of event notifiers below to determine what criteria must be true before they are called.
//
//Event polling and notification begin when Activate() is called, and end when
//Deactivate() is called.  If you are deriving from CScreen, you should leave 
//these calls to the CScreenManager loop, and just use GoToScreen() to specify the
//next destination screen.  If you are deriving from CDialog, you should use the
//all-in-one CDialog::Display() method, which handles activation and deactivation.
//
//You can explicitly change the focus by using one of the widget selection methods.  
//Default focus-changing behaviour is provided without this.
//
//ADDING NEW EVENT NOTIFIERS
//
//It should be pretty straightforward if you look at the other notifiers and follow their
//example.  One design consideration is that you can call an event notifier from 
//widget event-handling code, as opposed to calling from this class.  For example, 
//OnSelectChange() is called by CListBoxWidget when it learns that its selection has 
//changed.  The information needed to determine CListBoxWidget's selection change is
//not available from CEventHandlerWidget, so the solution is to call the notifier
//from CListBoxWidget.
//
//GETTING NEW WIDGETS HOOKED IN
//
//If your widget doesn't respond to input, then there is nothing extra to do.  It will
//not be affected by events.
//
//Derive your new widget from CFocusWidget if it should be selectable.  Your painting
//code should call CFocusWidget::IsSelected() and paint for the two different states.
//
//If your widget responds to input, look at the CWidget::Handle*() methods.  
//You should write your widget to respond to the existing Handle*() methods that are
//already called from CEventHandlerWidget, and not add widget-type-specific code 
//into CEventHandlerWidget.  If the existing Handle*() methods do not provide the 
//appropriate stimuli for your widget to take correct behaviour, try to make 
//modifications that are not special-cased to your widget's event-handling, but 
//general to all widgets.
//
//The Handle*() methods are described in Widget.h.

#ifndef EVENTHANDLERWIDGET_H
#define EVENTHANDLERWIDGET_H

#include "Widget.h"
#include <BackEndLib/Assert.h>

class CScreenManager;
class CEventHandlerWidget : public CWidget
{
public:
	CWidget *	GetSelectedWidget();
	void			SelectFirstWidget(const bool bPaint = true);
	void			SelectNextWidget(const bool bPaint = true);
	void			SelectPrevWidget(const bool bPaint = true);
	void			SelectWidget(const DWORD dwSelectTagNo, const bool bPaint = true);
	void			SelectWidget(CWidget *pSelectWidget, const bool bPaint = true);
	void			StopKeyRepeating();
   void        StopMouseRepeating();

	CWidget *	MouseDraggingInWidget() {return this->pHeldDownWidget;}
	bool			RightMouseButton() {return this->wButtonIndex == SDL_BUTTON_RIGHT;}

	//
	//Overridable event notifiers.
	//

	virtual void	OnActiveEvent(const SDL_ActiveEvent &Active);
	//Called when the window received an active event, including losing/gaining
   //focus, minimizing/restoring, and mouse entering/leaving window.

	virtual void	OnBetweenEvents() { }
	//Called periodically when no events are being processed.  The guaranteed minimum
	//interval can be set by SetBetweenEventsInterval() and defaults to 33ms (30 fps).

	virtual void	OnSelectChange(const DWORD /*dwTagNo*/) { }
	//Called when a widget's selection changes.  Not every widget is used to select
	//information, and it is up to event-handling code within the widget to decide
	//when its selection has changed.

	virtual void	OnClick(const DWORD /*dwTagNo*/) { }
	//Called when a widget which uses the button metaphor has been chosen by the user.
	//This choice could be made with a matching mousedown/mouseup pair, hotkey press, or
	//other criteria chosen by event-handling code within the widget.

	virtual void	OnDeactivate() { }
	//Called right after the event loop has exited.

	virtual void	OnDoubleClick(const DWORD /*dwTagNo*/) { }
	//Called when two OnClick events are received within a short time of each other.

	virtual void	OnDragUp(const DWORD /*dwTagNo*/, const SDL_MouseButtonEvent &/*Button*/) { }
	//Called when a widget previously received a mouse down within its area,
	//and the mouse button has been released.  The mouse pointer may or may not be within
	//the widget area.

	virtual void	OnKeyDown(const DWORD /*dwTagNo*/, const SDL_KeyboardEvent &/*Key*/) { }
	//Called when a key has been pressed.  If a widget is selected that is affected by
	//keydown events, the dwTagNo param will be set to the selected widget's tag#.
	//Otherwise dwTagNo will be set to the event-handling widget's tag#.

	virtual void	OnKeyUp(const DWORD /*dwTagNo*/, const SDL_KeyboardEvent &/*Key*/) { }
	//Called when a key has been released.  For now, dwTagNo will always be set to the
	//event-handling widget's tag#.  If there is a need to respond to key releases inside
	//of specific widgets, I recommend making changes to Activate_HandleKeyUp().

	virtual void	OnMouseDown(const DWORD /*dwTagNo*/, const SDL_MouseButtonEvent &/*Button*/) { }
	//Called when a mouse button has been pressed.  If the coords of the click are within
	//a specific widget, dwTagNo will be set to that widget's tag#.  Otherwise dwTagNo
	//will be set to the event-handling widget's tag#.

	virtual void	OnMouseUp(const DWORD /*dwTagNo*/, const SDL_MouseButtonEvent &/*Button*/) { }
	//Called when a mouse button has been released.  If the coords of the click are within
	//a specific widget, dwTagNo will be set to that widget's tag#.  Otherwise dwTagNo
	//will be set to the event-handling widget's tag#.

	virtual void	OnMouseMotion(const DWORD /*dwTagNo*/, const SDL_MouseMotionEvent &/*Motion*/) { }
	//Called when the mouse is moving.  If the coords of the click are within
	//a specific widget, dwTagNo will be set to that widget's tag#.  Otherwise dwTagNo
	//will be set to the event-handling widget's tag#.

   virtual void   OnMouseWheel(const SDL_MouseButtonEvent &/*Button*/) { }
   //Called when the mouse wheel has been rolled.

   virtual bool	OnQuit() {return false;}
	//Called when user sends an application exit signal.  (Probably clicked on close window
	//button.)

protected:
	friend class CScreenManager;
	friend class CWidget;

	CEventHandlerWidget(WIDGETTYPE eSetType, DWORD dwSetTagNo,
		int nSetX, int nSetY, UINT wSetW, UINT wSetH);
	virtual ~CEventHandlerWidget() {}

	void			Activate();
	void			AddAnimatedWidget(CWidget *pWidget);
	void			AddFocusWidget(CWidget *pWidget);
	void			Deactivate() {ASSERT(!this->bDeactivate); this->bDeactivate=true;};
	bool			IsDeactivating() const {return this->bDeactivate;}
	void			RemoveAnimatedWidget(CWidget *pWidget);
	void			RemoveFocusWidget(CWidget *pWidget);
	virtual bool	SetForActivate() {return true;}
	void			SetBetweenEventsInterval(const DWORD dwSetMSecs);
	void			SetKeyRepeat(const DWORD dwContinueMSecs, const DWORD dwStartMSecs=300L);

	bool bPaused;	//whether animation is paused

private:
	//Event handling methods called directly by Activate().
   void        Activate_HandleActiveEvent(const SDL_ActiveEvent &Active);
	void			Activate_HandleBetweenEvents();
	void			Activate_HandleKeyDown(const SDL_KeyboardEvent &Key);
	void			Activate_HandleKeyUp(const SDL_KeyboardEvent &Key);
	void			Activate_HandleMouseDown(const SDL_MouseButtonEvent &Button);
	void			Activate_HandleMouseUp(const SDL_MouseButtonEvent &Button);
	void			Activate_HandleMouseMotion(const SDL_MouseMotionEvent &Motion);
	void			Activate_HandleQuit();

	void			ChangeSelection(WIDGET_ITERATOR iSelect, const bool bPaint);
	bool			CheckForSelectionChange(const SDL_KeyboardEvent &KeyboardEvent);
	bool			IsKeyRepeating(DWORD &dwRepeatTagNo);
	CWidget *		pHeldDownWidget;
	bool			bIsFirstMouseDownRepeat;
	DWORD			dwLastMouseDownRepeat;
	UINT			wButtonIndex;

	//for double-click
	DWORD			dwTimeOfLastClick;
 	DWORD			dwXOfLastClick, dwYOfLastClick;

	bool			bDeactivate;
	DWORD			dwBetweenEventsInterval;
	DWORD			dwLastOnBetweenEventsCall;
	DWORD			dwStartKeyRepeatDelay, 	dwContinueKeyRepeatDelay;

	list<CWidget *>	AnimatedList;
	list<CWidget *>	FocusList;
	WIDGET_ITERATOR	iSelectedWidget;
};

#endif //#ifndef EVENTHANDLERWIDGET_H

// $Log: EventHandlerWidget.h,v $
// Revision 1.9  2003/08/18 16:40:39  mrimer
// Added StopMouseRepeat() to fix bug: stopping key repeat stops mouse down.
//
// Revision 1.8  2003/06/30 19:30:09  mrimer
// Added OnActiveEvent() to make sound/music quiet when app is inactive.
//
// Revision 1.7  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/06/16 20:40:04  mrimer
// Added OnMouseWheel().
//
// Revision 1.5  2003/06/16 18:46:04  mrimer
// Added OnDragUp() event and handling.
//
// Revision 1.4  2003/06/09 23:51:27  mrimer
// Moved them back.
//
// Revision 1.3  2003/06/09 19:25:08  mrimer
// Made Add(Remove)FocusWidget public, as they should be.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.19  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.18  2003/04/17 21:04:45  mrimer
// Refined tracking of mouse button and right clicks.
//
// Revision 1.17  2003/04/13 01:59:20  mrimer
// Added double-click event handling (on behalf of moldar).
//
// Revision 1.16  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.15  2002/11/15 02:44:50  mrimer
// Added recognition of mouse dragging and right-clicks.
//
// Revision 1.14  2002/10/04 17:56:09  mrimer
// Changed OnQuit() to return a bool.
// Added bPaused.
//
// Revision 1.13  2002/07/22 18:37:33  mrimer
// Made key repeat rate modifiable.
//
// Revision 1.12  2002/07/20 23:09:48  erikh2000
// Class now supports automatic animation for widgets that support animation.
//
// Revision 1.11  2002/07/17 19:15:10  mrimer
// Added CheckForSelectionChange(), and a call allowing it to repeat.
//
// Revision 1.10  2002/07/10 04:11:24  erikh2000
// Make routine to stop key repeating public, so that key-repeating could be explicitly stopped.
//
// Revision 1.9  2002/07/05 10:33:27  erikh2000
// Many changes--read comments at top of EventHandlerWidget.h for info.
//
// Revision 1.8  2002/06/26 17:12:20  mrimer
// Removed UnFocus().
//
// Revision 1.7  2002/06/25 02:35:26  mrimer
// Added StopRepeat().
//
// Revision 1.6  2002/06/24 22:25:08  mrimer
// Added FOCUS_UNSPECIFIED class var.
//
// Revision 1.5  2002/06/21 04:53:55  mrimer
// Added InitFocus().
//
// Revision 1.4  2002/06/20 00:55:07  erikh2000
// Added OnBeforeActivate() method.
//
// Revision 1.3  2002/06/13 21:53:41  mrimer
// Refactored more event code from CScreen to CEventHandlerWidget.
//
// Revision 1.2  2002/06/11 22:42:43  mrimer
// Added focus and key repeat handling from CScreen.
//
// Revision 1.1  2002/06/03 22:51:55  mrimer
// Added CEventHandlerWidget base class.
//
