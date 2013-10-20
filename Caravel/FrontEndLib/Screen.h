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

//Screen.h
//Declarations for CScreen.
//CScreen handles all input and output for a given screen.

//SUMMARY
//
//CScreen is an abstract base class from which all screens derive.  A screen is just a big
//top-level widget that can contain child widgets.  So pretty much all the comments found at the
//top of Widget.h apply.  A screen always takes up the entire window space (CX_SCREEN pixels
//across and CY_SCREEN pixels down).  Only one screen can be visible at once.  Sizable screens are
//not supported.
//
//USAGE
//
//You can derive new classes from CScreen.  It's necessary to add a new SCREENTYPE enumeration
//at the top of ScreenManager.h and add a switch handler in CScreenManager::GetNewScreen() that will
//construct your new class.
//
//To write code that sends the user to your new screen, determine the event that should send
//the user there, i.e. user clicks a "Go To Scary Screen" button.  Code in the event handler, (i.e.
//OnClick()) should make a call to its own class's GoToScreen() method.  When the event handler
//exits, the screen manager will transition to the new screen.
//
//If you followed all the above instructions without doing anything else, you would have a black
//screen that doesn't do much.  The user can hit escape from the new screen to return to whatever
//screen they were at previously.
//
//You can show things on the screen in two different ways.  1. Add widgets to the screen that will
//be painted automatically without writing code in your CScreen-derived class.  2. Override the
//default Paint() method to draw something besides that big black rectangle.  More about writing
//widget-adding and painting code is found at the top of Widget.h.  Both methods may be used
//together.  Generally, you only want to use method 2 when you are painting something that only
//appears on one screen.  Otherwise, you'd use existing widgets or write new widgets to paint what
//you want.
//
//You can respond to events like the user pressing a key or pushing a button.  CScreen
//has default event-handling methods that you can override in your CScreen-derived class.  They
//are all prefixed with "On".  When overriding a CScreen event-handling method it is almost always
//a good idea to call CScreen's method at the top of your overriding method.  This ensures that
//you get the nice default behaviour that makes the screens consistent.  More about writing
//event-handling code is found at the top of EventHandlerWidget.h and all of the event-handlers
//are described in the CEventHandlerWidget class declaration.
//
//DIALOGS
//
//Dialogs are another type of widget that can be a child of a screen, but they are different in
//that they prevent the screen from receiving events while they are on the screen.  In other words,
//dialogs are modal.  More about writing dialogs and code to call them is found at the top of
//DialogWidget.h.
//
//There are some common dialogs that can be activated by calling CScreen methods.  These methods
//are named in the form Show*Message(), i.e. ShowYesNoMessage().  You pick the one that suits the
//type of information you want to display and input you want to collect from the user.  If you
//create a new dialog that is useful to show on more than one screen, and can be used in one
//call, then consider writing a new CScreen method like the others to activate the dialog and
//return user input.
//
//TWO SCREENS OR ONE SCREEN WITH TWO MODES?
//
//You may have a screen which has two different modes to it.  I.e. if you are in "Scary" mode
//everything is drawn with scary graphics and if you are in "happy" mode everything looks happy.
//Your first impulse may be to put an "if (scary) ... else" statement in your Paint() method and input
//handlers.  It's almost always better to use two separate screens and share a base class between
//them.  The base class can have static members for shared data between the two classes.  You can
//call g_pTheSM->SetDestTransition(Cut) before calling GoToScreen() and you will have an instant
//seamless transition between the screens.

#ifndef SCREEN_H
#define SCREEN_H

#ifdef WIN32
#pragma warning(disable:4786)
#endif

//Values needed here, but can't place them in a final enumeration yet (i.e.,
//there will be more screens used in the app).
//Give these values to the corresponding names in the final enumeration.
namespace SCREENLIB {
   enum CURSORTYPE {
	   CUR_Select = 0,
	   CUR_Wait = 1
   };
}

#include "EventHandlerWidget.h"
#include "DialogWidget.h"
#include "EffectList.h"
#include "FileDialogWidget.h"
#include "ScreenManager.h"

#include <BackEndLib/MessageIDs.h>
#include <BackEndLib/Wchar.h>

#include <string>

//Message dialog widget tags.
const DWORD TAG_YES	= 9001;
const DWORD TAG_NO = 9002;
const DWORD TAG_TEXT = 9003;
const DWORD TAG_FRAME = 9004;
const DWORD	TAG_CANCEL_ = 9005;
const DWORD	TAG_TEXTBOX = 9006;
const DWORD	TAG_TEXTBOX2D = 9007;

//*****************************************************************************
class CScreen : public CEventHandlerWidget
{
friend class CDialogWidget;

public:
	static int		CX_SCREEN, CY_SCREEN;	//physical screen dimensions

protected:
	friend class CScreenManager;
   friend class CEffectList;
	CScreen(const UINT eSetType);
	virtual ~CScreen();

	void				AllocFocusArray();
	bool				ExportSelectFile(const MESSAGE_ID messageID,
			WSTRING &wstrExportFile, const FileExtensionType extensionType);
	UINT				GetScreenType() const {return this->eType;}
	UINT				GetDestScreenType() const {return this->eDestScreenType;}
	void				GoToScreen(const UINT eNextScreen);
	void				HideCursor();
	void				HideStatusMessage();
	MESSAGE_ID		Import(const FileExtensionType extensionType, DWORD& dwImportedID);
	bool				IsCursorVisible() const;
	bool				IsFullScreen() const;
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
	virtual void	OnMouseDown(const DWORD dwTagNo,
			const SDL_MouseButtonEvent &Button);
	virtual void	OnMouseMotion(const DWORD dwTagNo,
			const SDL_MouseMotionEvent &MotionEvent);
	virtual bool	OnQuit();
	virtual void	Paint(bool bUpdateRect=true);
   void           RemoveToolTip();
   void           RequestToolTip(const MESSAGE_ID messageID);
	DWORD				SelectFile(WSTRING &filePath, WSTRING &fileName,
			const MESSAGE_ID messagePromptID, const bool bWrite,
			const FileExtensionType extensionType);
   void				SetCursor(const UINT cursorType=SCREENLIB::CUR_Select);
	void				SetDestScreenType(const UINT eSet) {this->eDestScreenType = eSet;}
	void				SetFullScreen(const bool bSetFull);
	virtual bool		SetForActivate();
	void				SetScreenType(const UINT eSetType) {this->eType = eSetType;}
	void				ShowCursor();
	DWORD				ShowOkMessage(const MESSAGE_ID dwMessageID);
	DWORD				ShowYesNoMessage(const MESSAGE_ID dwMessageID);
	void				ShowStatusMessage(const MESSAGE_ID dwMessageID);
	DWORD				ShowTextInputMessage(const MESSAGE_ID dwMessageID,
			WSTRING &wstrUserInput, const bool bMultiLineText=false,
         const bool bMustEnterText=true);
	DWORD				ShowTextInputMessage(const WCHAR* pwczMessage,
			WSTRING &wstrUserInput, const bool bMultiLineText=false,
         const bool bMustEnterText=true);
	void				ToggleScreenSize();
	virtual void	UpdateRect() const;
	void				UpdateRect(const SDL_Rect &rect) const;

	virtual void	OnBetweenEvents();

	Uint32			dwLastMouseMove;
	Uint16			wLastMouseX, wLastMouseY;

   //Effects drawn on top of screen
   CEffectList *  pEffects;
	bool			   bShowTip, bShowingTip;	//time to show tool tip / tool tip displaying

   CFileDialogWidget * pFileBox;		//choose from files on disk

private:
	DWORD				ShowMessage(const MESSAGE_ID dwMessageID);

	UINT					eType;
	UINT					eDestScreenType;
	CDialogWidget	*	pStatusDialog;
	CDialogWidget	*	pMessageDialog;
	CDialogWidget	*	pInputTextDialog;
};

#endif //...#ifndef SCREEN_H

// $Log: Screen.h,v $
// Revision 1.13  2003/10/01 15:57:30  mrimer
// Fixed UpdateRect() method inheritance from CWidget to CScreen.
//
// Revision 1.12  2003/08/16 03:58:18  mrimer
// Added RemoveToolTip().
//
// Revision 1.11  2003/08/16 00:44:22  mrimer
// Added effect list and tool tip support.
//
// Revision 1.10  2003/07/19 02:11:00  mrimer
// Added optional use of multi-line text editor dialog.
//
// Revision 1.9  2003/07/12 01:12:00  mrimer
// Revised screen/cursor naming in FrontEndLib.
//
// Revision 1.8  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.7  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/07/07 23:39:59  mrimer
// Made CDialogWidget a friend to CScreen.
//
// Revision 1.5  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/17 23:12:36  mrimer
// Changed ShowTextInputMessage() to require text entry by default.
//
// Revision 1.3  2003/06/10 01:21:49  mrimer
// Revised some code.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.47  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.46  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.45  2003/05/03 23:32:22  mrimer
// Added optional enforcement of entering some text on a text input dialog.
//
// Revision 1.44  2003/04/29 11:09:27  mrimer
// Added import/export file extension specifications.
//
// Revision 1.43  2003/04/28 22:57:19  mrimer
// Added ExportSelectFile().
//
// Revision 1.42  2003/04/28 22:20:40  mrimer
// Added a parameter to SelectFile().
//
// Revision 1.41  2003/04/26 17:14:47  mrimer
// Added a extra parameter to SelectFile().
//
// Revision 1.40  2003/04/15 14:53:41  mrimer
// Added support for multiple cursor graphics.
//
// Revision 1.39  2003/02/16 20:32:19  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.38  2003/01/08 00:52:54  mrimer
// Changed SelectLevelID() return type to bool.
//
// Revision 1.37  2003/01/03 09:07:50  erikh2000
// Added documentation for CScreen.
//
// Revision 1.36  2002/12/22 02:20:05  mrimer
// Added Import, SelectFile, SelectLevelID interfaces.  Added ShowStatusMessage().
//
// Revision 1.35  2002/11/22 02:13:47  mrimer
// Made wLastMouseX/Y protected.
//
// Revision 1.34  2002/11/15 02:26:01  mrimer
// Added a ShowTextInputMessage(const MESSAGE_ID) method.  Made some parameters const.
//
// Revision 1.33  2002/10/22 21:24:05  mrimer
// Added OnMouseDown(), which makes cursor appear.
// Removed unneeded includes.
//
// Revision 1.32  2002/10/16 23:03:11  mrimer
// Added wLastMouseX/Y to preserve mouse cursor position when hiding.
// Made wait time longer before hiding cursor, and movement offset minimum smaller for making it reappear.
//
// Revision 1.31  2002/10/11 17:36:13  mrimer
// Modified OnMouseMotion() to only show cursor if substantial mouse movement is made.
//
// Revision 1.30  2002/10/04 17:55:39  mrimer
// Changed OnQuit() to return a bool.
//
// Revision 1.29  2002/10/03 22:43:51  mrimer
// Added code to show mouse cursor when moved, then hide again after some non-activity.
//
// Revision 1.28  2002/09/24 21:20:31  mrimer
// Removed TAG_OK.
//
// Revision 1.27  2002/09/04 20:40:15  mrimer
// Moved UpdateRect(rect) from CWidget to CScreen.
//
// Revision 1.26  2002/07/09 23:12:28  mrimer
// Tweaking.
//
// Revision 1.25  2002/07/05 10:34:12  erikh2000
// Many changes to work with new event-handling scheme.
//
// Revision 1.24  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.23  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.22  2002/06/14 22:21:18  mrimer
// Fixed includes.
//
// Revision 1.21  2002/06/14 02:32:40  erikh2000
// Changed dialog tag constants so that they don't conflict with tags from other screens.
//
// Revision 1.20  2002/06/14 01:03:32  erikh2000
// Changed a WCHAR buffer parameter to safer wstring.
//
// Revision 1.19  2002/06/13 21:43:54  mrimer
// Refactored more event code from CScreen to CEventHandlerWidget.
//
// Revision 1.18  2002/06/11 22:45:10  mrimer
// Refactored focus and key repeat handling into CEventHandlerWidget.
// Condensed SetWindowed() into SetFullScreen(bool).
//
// Revision 1.17  2002/06/11 17:42:43  mrimer
// KeyRepeat() now returns tag of widget handling key repeat.
//
// Revision 1.16  2002/06/05 19:49:02  mrimer
// Inserted key repeat handling.
//
// Revision 1.15  2002/06/05 03:09:07  mrimer
// Added focus graphic.  Added widget focusability and keyboard control.
//
// Revision 1.14  2002/06/03 22:51:55  mrimer
// Added CEventHandlerWidget base class.
//
// Revision 1.13  2002/05/31 23:45:42  mrimer
// Added hotkey support.
//
// Revision 1.12  2002/05/15 01:28:57  erikh2000
// Added methods to get and set screen type.
//
// Revision 1.11  2002/05/12 03:20:44  erikh2000
// Added OnKeydown() method.
//
// Revision 1.10  2002/05/10 22:41:15  erikh2000
// Moved mouse cursor show/hide code from CGameScreen.
// Moved set windowed/fullscreen code from CWidget.
//
// Revision 1.9  2002/04/29 00:19:01  erikh2000
// Added list box event-handling.
//
// Revision 1.8  2002/04/25 18:19:13  mrimer
// Added ToggleScreenSize function.
//
// Revision 1.7  2002/04/24 08:15:23  erikh2000
// Added mouse handling for sliders and option buttons.
//
// Revision 1.6  2002/04/19 22:02:42  erikh2000
// Screen now handles painting buttons in response to mouse events with overridable methods.
//
// Revision 1.5  2002/04/16 10:47:19  erikh2000
// Two new methods to show message dialogs added--one for "Okay" messages, the other for "Yes/No" messages.
//
// Revision 1.4  2002/04/13 19:35:27  erikh2000
// Added methods to show message boxes on screen.
//
// Revision 1.3  2002/04/09 22:15:16  erikh2000
// Added SetWindowed() and SetFullscreen() stubs.
//
// Revision 1.2  2002/04/09 10:05:39  erikh2000
// Fixed revision log macro.
//
