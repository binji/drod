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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "EventHandlerWidget.h"
#include "OptionButtonWidget.h"
#include "Sound.h"
#include <BackEndLib/Assert.h>

//A focus list iterator pointing to the end will mean that no widget is selected.
#define NO_SELECTION (this->FocusList.end())

//Key repeating is reset whenever a CEventHandlerWidget is activated.  For
//nested activations, I don't want the parents to have a repeating key when
//their call to activation occurs.  To accomplish this, one set of key repeat
//state vars is used for all event-handling widgets.
static SDL_KeyboardEvent	m_RepeatingKey; //Will be initialized in constructor.
static DWORD				m_dwLastKeyDown = 0L;
static DWORD				m_dwLastKeyRepeat = 0L;

//************************************************************************************
CEventHandlerWidget::CEventHandlerWidget(
//Constructor.
//
	WIDGETTYPE eSetType,					//(in)	Required params for CWidget
	DWORD dwSetTagNo,							//		constructor.
	int nSetX, int nSetY,					//
	UINT wSetW, UINT wSetH)
	: CWidget(eSetType, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, bPaused(false)

   , pHeldDownWidget(NULL)
	, bIsFirstMouseDownRepeat(true)
	, dwLastMouseDownRepeat(0L)
	, wButtonIndex(0)

	, dwTimeOfLastClick(0L)
	, dwXOfLastClick(0L), dwYOfLastClick(0L)

	, bDeactivate(false)

   , dwBetweenEventsInterval(33L) //33ms = 30 frames per second.
	, dwLastOnBetweenEventsCall(SDL_GetTicks())

	, dwStartKeyRepeatDelay(300L)
	, dwContinueKeyRepeatDelay(33L)	//33ms = 30x per second.
{
	this->iSelectedWidget = NO_SELECTION;

	m_dwLastKeyDown = 0L;
	m_dwLastKeyRepeat = 0L;
	m_RepeatingKey.keysym.sym = SDLK_UNKNOWN;
}

//
// Public methods
//

//*****************************************************************************
void CEventHandlerWidget::StopKeyRepeating()
//Stop any current key from repeating.
{
	m_RepeatingKey.keysym.sym = SDLK_UNKNOWN;
}

//*****************************************************************************
void CEventHandlerWidget::StopMouseRepeating()
//Stop mouse button from repeating.
{
   this->pHeldDownWidget = NULL;
}

//******************************************************************************
CWidget * CEventHandlerWidget::GetSelectedWidget()
//Returns the selected widget or NULL if no selectable widgets available.
{
	if (this->FocusList.size()==0 || this->iSelectedWidget == NO_SELECTION) 
		return NULL;

	CWidget *pWidget = *(this->iSelectedWidget);
	ASSERT(pWidget->IsSelectable());
	return pWidget;
}

//******************************************************************************
void CEventHandlerWidget::SetBetweenEventsInterval(
//Set the interval between calls to OnBetweenEvents().
//
//Params:
	const DWORD dwSetMSecs)	//(in)	Interval expressed in milliseconds.
{
	this->dwBetweenEventsInterval = dwSetMSecs;
}

//******************************************************************************
void CEventHandlerWidget::SetKeyRepeat(
//Set the interval between key repeats.
//
//Params:
	const DWORD dwContinueMSecs,	//(in) Interval between repeats
	const DWORD dwStartMSecs)		//(in) Interval before starting repeat
{
	this->dwStartKeyRepeatDelay = dwStartMSecs;
	this->dwContinueKeyRepeatDelay = dwContinueMSecs;
}

//******************************************************************************
void CEventHandlerWidget::Activate()
//Handle the input loop while event-handling widget is active.  
//The method exits when Deactivate() is called.
{
    DWORD dwStartFrame, dwSince;
    
    //The delay between frames will be calculated to fit inside key repeat setting plus this many 
    //milliseconds.  Also, don't delay past 30fps or things will look bad.
    const UINT FRAME_DELAY_PAD = 5;
    UINT wMaxFrameDelay = this->dwContinueKeyRepeatDelay + FRAME_DELAY_PAD;
    if (wMaxFrameDelay > 30) wMaxFrameDelay = 30;

	StopKeyRepeating();
	StopMouseRepeating();

	//Process events in loop below.
	//To avoid difficult-to-debug intermittent errors, no event-handling should
	//occur after deactivation.  I.e. don't handle a SDL_KEYUP when SDL_KEYDOWN
	//handling caused deactivation.  You are not guaranteed to receive a matching
	//SDL_KEYUP in every case, so it is best to consistently see an error caused
	//by not receiving the SDL_KEYUP.

	this->bDeactivate = false; //A callee will call Deactivate() which sets this.
	SDL_Event event;
	while (!this->bDeactivate)
	{	
        dwStartFrame = SDL_GetTicks();
            
		//Get any events waiting in the queue.
		while (!this->bDeactivate && SDL_PollEvent(&event)) 
		{
			switch (event.type)
			{
            case SDL_ACTIVEEVENT:
               Activate_HandleActiveEvent(event.active);
            break;

            case SDL_KEYDOWN:
					Activate_HandleKeyDown(event.key);
				break;

				case SDL_KEYUP:
					Activate_HandleKeyUp(event.key);
				break;

				case SDL_MOUSEBUTTONDOWN:
					Activate_HandleMouseDown(event.button);
				break;

				case SDL_MOUSEBUTTONUP:
					Activate_HandleMouseUp(event.button);
				break;

				case SDL_MOUSEMOTION:
					Activate_HandleMouseMotion(event.motion);
				break;

				case SDL_QUIT:
					Activate_HandleQuit();
				break;
			}
		}	//...while I have events.

      const Uint8 state = SDL_GetAppState();
      const bool bActive = (state & SDL_APPACTIVE) == SDL_APPACTIVE;
      const bool bHasFocus = (state & SDL_APPINPUTFOCUS) == SDL_APPINPUTFOCUS;
      if (bActive || bHasFocus)
      {
         if (!this->bDeactivate)
			   Activate_HandleBetweenEvents();

         //These calls will cause Windows and maybe other O/Ss to sleep a tiny bit,
		   //freeing up the processor.  On my computer, CPU usage was cut from 100 to 50%
		   //on the game screen during idle moments.
         if (bActive && bHasFocus)
         {
            dwSince = SDL_GetTicks() - dwStartFrame;
            SDL_Delay(dwSince > wMaxFrameDelay ? 1 : wMaxFrameDelay - dwSince);
         }
         else
         {
            //Make app less aggressive when it doesn't have focus.
            SDL_Delay(50);
            StopKeyRepeating();
            StopMouseRepeating();
         }
      } else {
         //Slow it down even more when minimized.
         SDL_Delay(200);
         StopKeyRepeating();
         StopMouseRepeating();
      }
	}

	//Let derived class handle deactivation tasks it may have.
	OnDeactivate();
}

//
// Protected methods
//

//**********************************************************************************
void CEventHandlerWidget::AddAnimatedWidget(
//Add widget to list of widgets that will be animated between events.
//
//Params:
	CWidget *pWidget) //(in)
{
	ASSERT(pWidget->IsAnimated());
	this->AnimatedList.push_back(pWidget);
}

//**********************************************************************************
void CEventHandlerWidget::RemoveAnimatedWidget(
//Remove widget from list of widgets that will be animated between events.
//
//Params:
	CWidget *pWidget) //(in)
{
	ASSERT(pWidget->IsAnimated());	
	this->AnimatedList.remove(pWidget);
}

//**********************************************************************************
void CEventHandlerWidget::AddFocusWidget(
//Add widget to list of focusable widgets that may be selected.
//
//Params:
	CWidget *pWidget) //(in)
{
	ASSERT(pWidget->IsFocusable());

	this->FocusList.push_back(pWidget);
	
	//If I don't have anything selected, and this widget is selectable, then 
	//select it.
	if (this->iSelectedWidget == NO_SELECTION && pWidget->IsSelectable())
	{
		WIDGET_ITERATOR iLast = this->FocusList.end();
		--iLast;
		ChangeSelection(iLast, false);
	}
}

//**********************************************************************************
void CEventHandlerWidget::RemoveFocusWidget(
//Remove widget from list of focusable widgets that may be selected.
//
//Params:
	CWidget *pWidget) //(in)
{
	ASSERT(pWidget->IsFocusable());

	//Is the currently selected widget about to be removed?
	if (this->iSelectedWidget != NO_SELECTION && 
			*(this->iSelectedWidget) == pWidget) //Yes.
	{
		//Move selection to next widget, since this one is going away.
		WIDGET_ITERATOR iBefore = this->iSelectedWidget;
		SelectNextWidget();
		if (this->iSelectedWidget == iBefore) //This is the last widget.
			ChangeSelection(NO_SELECTION, true); //Select nothing.
	}
	
	this->FocusList.remove(pWidget);
}

//**********************************************************************************
void CEventHandlerWidget::Activate_HandleActiveEvent(
//Handles SDL_ACTIVEEVENT event.
//
//Params:
	const SDL_ActiveEvent &Active)	//(in) Event to handle.
{
   OnActiveEvent(Active);
}

//*****************************************************************************
void CEventHandlerWidget::OnActiveEvent(const SDL_ActiveEvent &/*Active*/)
//Always take care of this no matter what widget is receiving event commands.
{
   static int musicVolume = -1, soundVolume = -1;

   const Uint8 state = SDL_GetAppState();
   if ((state & SDL_APPINPUTFOCUS) == SDL_APPINPUTFOCUS &&
         (state & SDL_APPACTIVE) == SDL_APPACTIVE)
   {
      //Application is minimised/iconified or restored.
      if (musicVolume != -1 && soundVolume != -1)
      {
         //Restore sound/music.
         g_pTheSound->SetMusicVolume(musicVolume);
         g_pTheSound->SetSoundEffectsVolume(soundVolume);
         musicVolume = soundVolume = -1;
      }
   } else {
      //Disable sound/music when app is inactive.
      if (musicVolume == -1 && soundVolume == -1)
      {
         musicVolume = g_pTheSound->GetMusicVolume();
         soundVolume = g_pTheSound->GetSoundVolume();
      }
      g_pTheSound->SetMusicVolume(0);
      g_pTheSound->SetSoundEffectsVolume(0);
   }
}

//**********************************************************************************
void CEventHandlerWidget::Activate_HandleBetweenEvents()
//Handle things that do not occur in response to an SDL event.
{
	const UINT MOUSEDOWN_REPEAT_INITIAL_DELAY = 500;
	const UINT MOUSEDOWN_REPEAT_CONTINUE_DELAY = 100;
	
	//If user has held a widget down long enough, call its HandleMouseDownRepeat().
	const DWORD dwNow = SDL_GetTicks();
	if ( this->pHeldDownWidget && 
			dwNow - this->dwLastMouseDownRepeat > ((this->bIsFirstMouseDownRepeat) ?
			MOUSEDOWN_REPEAT_INITIAL_DELAY : MOUSEDOWN_REPEAT_CONTINUE_DELAY) )
	{
		int nX, nY;
		SDL_GetMouseState(&nX, &nY);
		if (this->pHeldDownWidget->ContainsCoords(nX, nY))
		{
			//Populate an event struct to pass to the handler.
			static SDL_MouseButtonEvent Button = {SDL_MOUSEBUTTONDOWN, 0, 0, 
					SDL_PRESSED, 0, 0};
			Button.x = nX;
			Button.y = nY;
			
			this->dwLastMouseDownRepeat = dwNow;
			this->bIsFirstMouseDownRepeat = false;
			this->pHeldDownWidget->HandleMouseDownRepeat(Button);
		}
	}
	
	//Check for a repeating keypress.
	DWORD dwRepeatTagNo;
	if (IsKeyRepeating(dwRepeatTagNo))
	{
      if (!CheckForSelectionChange(m_RepeatingKey))
      {
			CWidget *pSelectedWidget = GetSelectedWidget();
		   //Call keydown and keyup handlers.
		   m_RepeatingKey.type = SDL_KEYDOWN;
		   m_RepeatingKey.state = SDL_PRESSED;
			if (pSelectedWidget)
         {
				pSelectedWidget->HandleKeyDown(m_RepeatingKey);
            if (!IsDeactivating())
				   OnKeyDown(pSelectedWidget->GetTagNo(), m_RepeatingKey);
         }
			else
				OnKeyDown(dwRepeatTagNo, m_RepeatingKey);
		   m_RepeatingKey.type = SDL_KEYUP;
		   m_RepeatingKey.state = SDL_RELEASED;
			if (pSelectedWidget)
				pSelectedWidget->HandleKeyUp(m_RepeatingKey);
			else
				OnKeyUp(dwRepeatTagNo, m_RepeatingKey);
      }
	}

	//Animate widgets and call between events handler if interval has elapsed.
	if (!this->bPaused &&
		dwNow - this->dwLastOnBetweenEventsCall > this->dwBetweenEventsInterval)
	{
		//Animate widgets.
		for (WIDGET_ITERATOR iSeek = this->AnimatedList.begin();
				iSeek != this->AnimatedList.end(); ++iSeek)
		{
			if ((*iSeek)->IsVisible())
				(*iSeek)->HandleAnimate();
		}

		OnBetweenEvents();
		this->dwLastOnBetweenEventsCall = dwNow;
	}
}

//******************************************************************************
void CEventHandlerWidget::Activate_HandleQuit()
//Handles SDL_QUIT event.
{
	OnQuit();
}

//******************************************************************************
void CEventHandlerWidget::Activate_HandleKeyDown(
//Handles SDL_KEYDOWN event.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
{
    //Remember information that will be used for repeating the keypress later
    //if user holds the key down long enough.
    m_dwLastKeyDown = SDL_GetTicks();
    m_dwLastKeyRepeat = m_dwLastKeyDown;
    m_RepeatingKey = KeyboardEvent;

    CWidget *pSelectedWidget = GetSelectedWidget();

    //Alt-enter does not affect focus or selected widget.
    if (!(KeyboardEvent.keysym.mod & KMOD_ALT &&
            KeyboardEvent.keysym.sym == SDLK_RETURN))
    {
        if (CheckForSelectionChange(KeyboardEvent)) return;

        //Check for widget affected by an ALT + hot key combination.
        const DWORD dwHotkeyTag = GetHotkeyTag(KeyboardEvent.keysym.sym);
        CWidget *pHotkeyWidget = GetWidget(dwHotkeyTag,true);
        if (pHotkeyWidget && !pHotkeyWidget->IsSelectable()) pHotkeyWidget = NULL;
        if (pHotkeyWidget && (KeyboardEvent.keysym.mod & KMOD_ALT))
        {
	        SelectWidget(pHotkeyWidget);
	        OnClick(dwHotkeyTag);
        }
        else
        {
            if (pSelectedWidget)
            {
	            //Handle key down in selected widget.
	            pSelectedWidget->HandleKeyDown(KeyboardEvent);

                if (IsDeactivating())
                   return;  //if widget became inactive after key, stop handling event

	            //If widget doesn't accept text entry then hot key without ALT 
	            //can be used.
	            //If widget does accept text entry, the Enter keys will act
	            //as a hotkey for the OK command.
	            if (pHotkeyWidget && (!pSelectedWidget->AcceptsTextEntry() ||
			            KeyboardEvent.keysym.sym == SDLK_RETURN ||
			            KeyboardEvent.keysym.sym == SDLK_KP_ENTER))
	            {
		            SelectWidget(pHotkeyWidget);
		            OnClick(dwHotkeyTag);
	            }
            }
        }
	}
    if (IsDeactivating())
        return;  //if widget became inactive after hotkey, stop handling event

	//Call notifier.
	if (pSelectedWidget)
		//Selected widget gets event.
		OnKeyDown(pSelectedWidget->GetTagNo(), KeyboardEvent);
	else
		//Event-handler gets event.
		OnKeyDown(this->dwTagNo, KeyboardEvent);
}

//******************************************************************************
void CEventHandlerWidget::Activate_HandleKeyUp(
//Handles SDL_KEYUP event.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
{
	//Sometimes a key up event for the previous repeating key will come
	//in after the key down for the current repeating key.  To prevent cancelling
	//the current key's repeating, only stop key repeating when they key up event 
	//is for the current key.
	if (KeyboardEvent.keysym.sym == m_RepeatingKey.keysym.sym)
		StopKeyRepeating();
	
	OnKeyUp(this->dwTagNo, KeyboardEvent);
}

//******************************************************************************
void CEventHandlerWidget::Activate_HandleMouseDown(
//Handles SDL_MOUSEBUTTONDOWN event.
//
//Params:
	const SDL_MouseButtonEvent &Button) //(in) Event to handle.
{
	static const UINT DOUBLE_CLICK_DELAY = 400; //in ms
	static const UINT DOUBLE_CLICK_PROXIMITY = 10; //in pixels

	//Mouse wheel events don't actually cause a mouse down event.
	if (Button.button > 3)
	{
		CWidget *pWidget = GetSelectedWidget();
		if (pWidget)
			pWidget->HandleMouseWheel(Button);
		else
			HandleMouseWheel(Button);
      OnMouseWheel(Button);
		return;
	}

	//Reset these members so that actions repeating in response to a widget
	//being held down will be correctly timed in OnBetweenEvents().
	this->dwLastMouseDownRepeat = SDL_GetTicks();
	this->bIsFirstMouseDownRepeat = true;
	this->wButtonIndex = Button.button;

	//Check for mouse down inside any widget.
	CWidget *pWidget = GetWidgetContainingCoords(Button.x, Button.y, WT_Unspecified);
	if (pWidget)
	{
		pWidget->HandleMouseDown(Button);
		this->pHeldDownWidget = pWidget;

		if (pWidget->IsSelectable())
			SelectWidget(pWidget);
	}

	//Check for double-click event.
	//Notice it is observed on the second mouse-down, not mouse-up.
	const DWORD dwTagNo = pWidget ? pWidget->GetTagNo() : this->dwTagNo;
 	if (static_cast<UINT>(abs(Button.x - (int)this->dwXOfLastClick) +
 			abs(Button.y - (int)this->dwYOfLastClick)) < DOUBLE_CLICK_PROXIMITY &&
         (SDL_GetTicks() - this->dwTimeOfLastClick) < DOUBLE_CLICK_DELAY)
   {
		OnDoubleClick(dwTagNo);
      this->dwTimeOfLastClick = 0;  //reset for next double-click
   } else {
		OnMouseDown(dwTagNo, Button);

 	   //Store the details so that we can process a possible double-click later.
 	   this->dwTimeOfLastClick = SDL_GetTicks();
 	   this->dwXOfLastClick = Button.x;
 	   this->dwYOfLastClick = Button.y;
   }
}

//******************************************************************************
void CEventHandlerWidget::Activate_HandleMouseUp(
//Handles SDL_MOUSEBUTTONDOWN event.
//
//Params:
	const SDL_MouseButtonEvent &Button) //(in) Event to handle.
{
	//Mouse wheel events don't actually cause a mouse up event.
	if (Button.button > 3) return;

	//If already holding down a widget, that widget handles the mouse up
	//regardless of coords.
	if (this->pHeldDownWidget)
   {
		this->pHeldDownWidget->HandleMouseUp(Button);
      //If a widget was being dragged, notify the dragging has stopped.
      OnDragUp(pHeldDownWidget->GetTagNo(), Button);
   }

	//Check for mouse up inside any other widgets.
	CWidget *pWidget = GetWidgetContainingCoords(Button.x, Button.y, WT_Unspecified);
	if (pWidget)
	{
		const DWORD dwEventTagNo = pWidget->GetTagNo();
		if (pWidget == this->pHeldDownWidget) 
			//A click occurs if the mouseup occurred within same widget as mousedown.
			//For some controls, like buttons, OnClick() means the user chose to 
			//perform that button's action, when OnMouseUp() would be an unreliable
			//indicator.
			OnClick(dwEventTagNo);
		else
			pWidget->HandleMouseUp(Button);
		OnMouseUp(dwEventTagNo, Button);
	}
	else
	{
		//If exec got here, then the mouse up event is for the event-handler widget.
		OnMouseUp(this->dwTagNo, Button);
	}
	this->pHeldDownWidget = NULL;
}

//******************************************************************************
void CEventHandlerWidget::Activate_HandleMouseMotion(
//Handles SDL_MOUSEMOTION event.
//
//Params:
	const SDL_MouseMotionEvent &Motion) //(in) Event to handle.
{
	//Handle mouse motion for any dragging widget.
	if (this->pHeldDownWidget)
		this->pHeldDownWidget->HandleDrag(Motion);

	//Send mouse motion event for a child widget that mouse is over.
	CWidget *pWidget = GetWidgetContainingCoords(Motion.x, Motion.y, WT_Unspecified);
	if (pWidget)
	{
		pWidget->HandleMouseMotion(Motion);
		OnMouseMotion(pWidget->GetTagNo(), Motion);
	}
	else
		OnMouseMotion(this->dwTagNo, Motion); //Mouse motion event for event-handling widget.
}

//*****************************************************************************
bool CEventHandlerWidget::IsKeyRepeating(
//Handles repeating keypresses.
//Repeat key after an initial delay, and then continue repeating after a smaller
//delay.
//
//Params:
	DWORD &dwRepeatTagNo)	//(out)	Tag# of widget receiving key repeat.  
							//		Not set if method returns false.
//
//Returns:
//True if a key is repeating, false if not.
{
	if (m_RepeatingKey.keysym.sym != SDLK_UNKNOWN) 
	{
		const Uint32 dwNow = SDL_GetTicks();
		if (dwNow - m_dwLastKeyDown > dwStartKeyRepeatDelay)
		{
			//key has been held down long enough to repeat
			if (dwNow - m_dwLastKeyRepeat > dwContinueKeyRepeatDelay)
			{
				dwRepeatTagNo = this->dwTagNo;
				m_dwLastKeyRepeat = dwNow;
				return true;
			}
		}
	}

	return false;
}

//
//Private methods.
//

//*************************************************************************************
void CEventHandlerWidget::SelectFirstWidget(
//Select first selectable widget.
//
//Params:
	const bool bPaint)			//(in)	If true (default) then widget selection will
								//		be painted.
{
	//Exit early if focus list contains no widgets.
	if (this->FocusList.size()==0) 
	{
		this->iSelectedWidget = NO_SELECTION;
		return;
	}

	//Search for first selectable widget from beginning of focus list.
	for (WIDGET_ITERATOR iSeek = this->FocusList.begin(); iSeek != this->FocusList.end();
			++iSeek)
	{
		CWidget *pWidget = *iSeek;
		if (pWidget && pWidget->IsSelectable())
		{
			ChangeSelection(iSeek, bPaint);
			return;
		}
	}

	//There are no selectable widgets.
	ChangeSelection(NO_SELECTION, bPaint);
}

//*************************************************************************************
void CEventHandlerWidget::SelectNextWidget(
//Select next selectable widget.
//
//Params:
	const bool bPaint)			//(in)	If true (default) then widget selection will
								//		be painted.
{
	//Exit early if focus list contains no widgets.
	if (this->FocusList.size()==0) 
	{
		this->iSelectedWidget = NO_SELECTION;
		return;
	}

	WIDGET_ITERATOR iSeek = this->iSelectedWidget;
	
	//Search for next selectable widget to end of focus list.
	CWidget *pWidget;
	if (iSeek != this->FocusList.end())
	{
		for (++iSeek; iSeek != this->FocusList.end(); ++iSeek)
		{
			pWidget = *iSeek;
			if (pWidget && pWidget->IsSelectable())
			{
				ChangeSelection(iSeek, bPaint);
				return;
			}
		}
	}

	//Search for next selectable widget from beginning of focus list.
	iSeek = this->FocusList.begin();
	do
	{
		ASSERT(iSeek != this->FocusList.end()); //Should have found iSelectedWidget first.
		pWidget = *iSeek;
		if (pWidget && pWidget->IsSelectable())
		{
			ChangeSelection(iSeek, bPaint);
			return;
		}
		++iSeek;
	} while (iSeek != this->iSelectedWidget);

	//There is only one focus widget.  Keep it selected.
	if (iSeek == this->iSelectedWidget)
		return;

	//There are no selectable widgets.
	ChangeSelection(NO_SELECTION, bPaint);
}

//*************************************************************************************
void CEventHandlerWidget::SelectPrevWidget(
//Select previous selectable widget.
//
//Params:
	const bool bPaint)			//(in)	If true (default) then widget selection will
								//		be painted.
{
	//Exit early if focus list contains no widgets.
	if (this->FocusList.size()==0) 
	{
		this->iSelectedWidget = NO_SELECTION;
		return;
	}

	WIDGET_ITERATOR iSeek = this->iSelectedWidget;
	
	//Search for next selectable widget to beginning of focus list.
	CWidget *pWidget;
	if (iSeek != this->FocusList.begin())
	{
		--iSeek;
		do
		{
			pWidget = *iSeek;
			if (pWidget && pWidget->IsSelectable())
			{
				ChangeSelection(iSeek, bPaint);
				return;
			}
			--iSeek;
		}
		while (iSeek != this->FocusList.begin());
	}

	//Search for next selectable widget from end of focus list.
	iSeek = this->FocusList.end();
	--iSeek;
	do
	{
		pWidget = *iSeek;
		if (pWidget && pWidget->IsSelectable())
		{
			ChangeSelection(iSeek, bPaint);
			return;
		}
		--iSeek;
	}
	while (iSeek != this->iSelectedWidget);

	//There is only one focus widget.  Keep it selected.
	if (iSeek == this->iSelectedWidget)
		return;

	//There are no selectable widgets.
	ChangeSelection(NO_SELECTION, bPaint);
}

//*************************************************************************************
void CEventHandlerWidget::SelectWidget(
//Select a specific widget.
//
//Params:
	CWidget *pWidget,			//(in)	Widget to select.
	const bool bPaint)			//(in)	If true (default) then widget selection will
								//		be painted.
{
	if (pWidget && pWidget->IsSelectable()) 
	{
		for (WIDGET_ITERATOR iSeek = this->FocusList.begin(); 
				iSeek != this->FocusList.end(); ++iSeek)
		{
			if (*iSeek == pWidget)
			{
				ChangeSelection(iSeek, bPaint);
				return;
			}
		}

		ASSERTP(false, "Caller is trying to select a widget that isn't in the focus list.");
	}
	else
		ASSERTP(false, "Caller is trying to select a nonexistent or unselectable widget.");
}
void CEventHandlerWidget::SelectWidget(
//Overload to select by widget tag#.
//
//Params:
	const DWORD dwSelectTagNo,	//(in)	Widget to select.
	const bool bPaint)			//(in)	If true (default) then widget selection will
								//		be painted.
{
	ASSERT(dwSelectTagNo);

	CWidget *pWidget = GetWidget(dwSelectTagNo);
	SelectWidget(pWidget, bPaint);
}

//*************************************************************************************
void CEventHandlerWidget::ChangeSelection(
//Change the selected widget to a new widget.
//
//Params:
	WIDGET_ITERATOR iSelect,		//(in)	Widget to select.  Passing NO_SELECTION  
									//		will cause no selection.
	const bool bPaint)				//(in)	If true then widget selection will
									//		be painted.
{
	if (this->iSelectedWidget == iSelect) return; //Nothing to do.

	if (this->iSelectedWidget != NO_SELECTION)
	{
		CFocusWidget *pFrom = DYN_CAST(CFocusWidget *, CWidget *, *(this->iSelectedWidget));
		pFrom->Unselect(bPaint);
	}

	if (iSelect != NO_SELECTION)
	{
		CFocusWidget *pTo = DYN_CAST(CFocusWidget *, CWidget *, *(iSelect));
		pTo->Select(bPaint);
		this->iSelectedWidget = iSelect;
	}
}

//**********************************************************************************
bool CEventHandlerWidget::CheckForSelectionChange(
//Returns true if widget selection is changed by KeyboardEvent.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
{
    //One not-obvious thing below: alt-tab is ignored to prevent a weird effect with
    //alt-tabbing in Windows.  Unfortunately SDL can't seem to detect when the alt-tab
    //menu has appeared, otherwise I might have handled this by disabling all unput while
    //alt-tab menu is visible.

	if (KeyboardEvent.keysym.sym == SDLK_TAB && !(KeyboardEvent.keysym.mod & KMOD_ALT))
	{
		if (KeyboardEvent.keysym.mod & KMOD_SHIFT)
			SelectPrevWidget();
		else
			SelectNextWidget();
      return true;
	}
   return false;
}

// $Log: EventHandlerWidget.cpp,v $
// Revision 1.22  2003/11/09 05:22:22  mrimer
// Fixed erroneous cascading double-clicks.
//
// Revision 1.21  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.20  2003/09/19 19:01:38  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.19  2003/09/15 22:15:44  erikh2000
// Oops.  Bug in the last commit.
//
// Revision 1.18  2003/09/15 21:58:55  erikh2000
// Added logic to keep frame rate above 30fps when possible.
//
// Revision 1.17  2003/09/15 21:54:21  erikh2000
// Changed delay between frames to be longer if it won't impact the desired key repeat rate.
//
// Revision 1.16  2003/08/18 16:40:39  mrimer
// Added StopMouseRepeat() to fix bug: stopping key repeat stops mouse down.
//
// Revision 1.15  2003/08/09 17:17:32  mrimer
// Fixed bug: key keeps repeating when app loses focus.
//
// Revision 1.14  2003/08/07 19:55:16  erikh2000
// Fixed bug with alt-tabbing causing widget focus change.
//
// Revision 1.13  2003/08/02 20:17:30  mrimer
// Fixed bug: event handler widget doesn't receive repeating key down events.
//
// Revision 1.12  2003/08/02 01:28:29  mrimer
// Fixed a potential assertion.
//
// Revision 1.11  2003/07/25 23:22:09  mrimer
// Restricted double-click recognition.
//
// Revision 1.10  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.9  2003/07/15 00:24:37  mrimer
// Fixed an assertion (deactivating twice because two key events are handled at once).
//
// Revision 1.8  2003/07/03 21:33:33  mrimer
// Port warning fix (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/06/30 21:57:19  mrimer
// Made app sleep to free up processor when inactive.
//
// Revision 1.6  2003/06/30 19:30:09  mrimer
// Added OnActiveEvent() to make sound/music quiet when app is inactive.
//
// Revision 1.5  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/16 20:40:04  mrimer
// Added OnMouseWheel().
//
// Revision 1.3  2003/06/16 18:46:04  mrimer
// Added OnDragUp() event and handling.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.31  2003/04/17 21:06:50  mrimer
// Added logic calling HandleMouseWheel().
//
// Revision 1.30  2003/04/13 01:59:19  mrimer
// Added double-click event handling (on behalf of moldar).
//
// Revision 1.29  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.28  2002/11/15 02:44:50  mrimer
// Added recognition of mouse dragging and right-clicks.
//
// Revision 1.27  2002/10/21 20:23:23  mrimer
// Fixed hotkey handling to only select from visible widgets.
//
// Revision 1.26  2002/10/16 02:29:12  erikh2000
// Fixed problem with 100% CPU usage at all times.
//
// Revision 1.25  2002/10/11 17:37:27  mrimer
// Key repeat now works for selected widget.
//
// Revision 1.24  2002/10/11 01:56:17  erikh2000
// Revised keydown-handling logic to avoid interpreting alt-enter as a hot key selection.
//
// Revision 1.23  2002/10/04 18:02:11  mrimer
// Added pause control logic.
//
// Revision 1.22  2002/09/24 21:42:47  mrimer
// Allow TextBoxWidgets to send ENTER hotkeys.
//
// Revision 1.21  2002/08/31 00:17:08  erikh2000
// Widgets are animated after a minimum interval has passed.
//
// Revision 1.20  2002/08/30 22:41:06  erikh2000
// Invisible widgets are no longer animated.
//
// Revision 1.19  2002/07/22 18:37:33  mrimer
// Made key repeat rate modifiable.
//
// Revision 1.18  2002/07/21 01:40:19  erikh2000
// Revised hot-key handling logic to fix a problem.
//
// Revision 1.17  2002/07/20 23:09:48  erikh2000
// Class now supports automatic animation for widgets that support animation.
//
// Revision 1.16  2002/07/19 20:38:37  mrimer
// Revised HandleKeyDown() to return no type.
//
// Revision 1.15  2002/07/17 19:15:09  mrimer
// Added CheckForSelectionChange(), and a call allowing it to repeat.
//
// Revision 1.14  2002/07/10 04:11:24  erikh2000
// Make routine to stop key repeating public, so that key-repeating could be explicitly stopped.
//
// Revision 1.13  2002/07/05 10:33:27  erikh2000
// Many changes--read comments at top of EventHandlerWidget.h for info.
//
// Revision 1.12  2002/06/26 17:14:58  mrimer
// Removed UnFocus().  Fixed focus assertian bugs.  Fixed button clicking bug.
//
// Revision 1.11  2002/06/25 05:44:35  mrimer
// Added call to StopRepeat().  Refined activeKey and focus code.
//
// Revision 1.10  2002/06/24 22:21:44  mrimer
// Robust dialog box focus init when it has no focus widgets.
//
// Revision 1.9  2002/06/21 18:32:30  mrimer
// Fixed list box event behavior.
//
// Revision 1.8  2002/06/21 05:04:43  mrimer
// Added InitFocus().  Revised UnFocus().
//
// Revision 1.7  2002/06/20 00:55:07  erikh2000
// Added OnBeforeActivate() method.
//
// Revision 1.6  2002/06/17 18:01:03  mrimer
// Don't allow focus change to inactive widget.
//
// Revision 1.5  2002/06/17 17:43:47  mrimer
// Added FOCUS_UNSPECIFIED const.  Fixed selecting first focus widget bug.
//
// Revision 1.4  2002/06/14 18:33:41  mrimer
// Fixed some event handling.
//
// Revision 1.3  2002/06/13 21:43:54  mrimer
// Refactored more event code from CScreen to CEventHandlerWidget.
//
// Revision 1.2  2002/06/11 22:58:23  mrimer
// Added focus and key repeat handling from CScreen.
//
// Revision 1.1  2002/06/03 22:51:54  mrimer
// Added CEventHandlerWidget base class.
//
