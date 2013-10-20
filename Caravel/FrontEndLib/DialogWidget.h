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

#ifndef DIALOGWIDGET_H
#define DIALOGWIDGET_H

#include "EventHandlerWidget.h"

//SUMMARY
//
//A dialog appears on top of a screen and takes over event-handling from the
//screen.  When the user presses a button, the dialog exits.
//
//USAGE
//
//Do the following in Load() method of CScreen or CScreen-derived class, before
//the call to LoadChildren():
//  1. Instance a new CDialogWidget.
//  2. Add child widgets to it for labels, buttons, frames, etc.  Use tag#s 
//     for buttons and other widgets so that you can identify which buttons
//     have been pressed.  A button without a tag# will not close the dialog.
//  3. Call CDialogWidget::Hide() so that dialog will not be visible right away.
//  4. Call AddWidget() to add the dialog to the screen.
//
//To activate the dialog and accept user input.
//  1. Call CDialogWidget::Display(). (Note that event-handling has passed to
//     the dialog at this point.)
//  2. Respond to the returned tag#.  If it is TAG_QUIT, you are obliged to
//     close the app down, probably by calling CScreen::GoToScreen(SCR_None)
//     after confirming the exit with the user.
//  3. Repaint the area underneath the now-hidden dialog--probably with a call
//     to C*Screen::Paint().  If you want to respond to button presses without
//     closing the dialog, you could skip the underneath repainting, and 
//     call Activate() on the dialog again.
//
//Dialog and associated resources will be freed in C*Screen::Unload() when it 
//makes its call to UnloadChildren().  Don't try to delete the CDialogWidget
//pointer.
//
//If you want to handle the input differently, derive a new class
//from CDialogWidget and override the On*() event notification methods.  
//CKeypressDialogWidget is an example of this.

//******************************************************************************
class CDialogWidget : public CEventHandlerWidget
{
public:
	CDialogWidget(const DWORD dwSetTagNo, const int nSetX, const int nSetY,
			const UINT wSetW, const UINT wSetH, const bool bListBoxDoubleClickReturns=false);

	DWORD				Display();
	virtual void		Paint(bool bUpdateRect = true);

	void				SetEnterText(const DWORD dwTagNo) {this->dwReqTextField = dwTagNo;}

protected:
	void					CheckTextBox();

	virtual bool   SetForActivate();

	virtual void		OnClick(const DWORD dwTagNo);
	virtual void      OnDoubleClick(const DWORD dwTagNo);
	virtual void		OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
	virtual void		OnSelectChange(const DWORD dwTagNo);
	virtual bool		OnQuit();

	DWORD				dwDeactivateValue;

private:
	DWORD				dwReqTextField;	//when a text box is present as a child
   bool           bListBoxDoubleClickReturns;
};

#endif //#ifndef DIALOGWIDGET_H

// $Log: DialogWidget.h,v $
// Revision 1.5  2003/08/05 15:50:22  mrimer
// Fixed bug: OK button inactive when file is selected in FileDialogWidget.
//
// Revision 1.4  2003/07/24 04:01:54  mrimer
// Added optional flag allowing double-clicking a list box on the dialog to exit w/ OK return value.
//
// Revision 1.3  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.2  2003/07/12 03:04:05  mrimer
// Files can now be selected by double-clicking on them.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.12  2003/05/03 23:32:22  mrimer
// Added optional enforcement of entering some text on a text input dialog.
//
// Revision 1.11  2002/12/22 02:24:51  mrimer
// Added OnSelectChange().
//
// Revision 1.10  2002/10/04 17:55:39  mrimer
// Changed OnQuit() to return a bool.
//
// Revision 1.9  2002/09/03 21:34:13  erikh2000
// Added a "virtual" in front of a method declaration for accuracy.
//
// Revision 1.8  2002/07/05 10:34:12  erikh2000
// Many changes to work with new event-handling scheme.
//
// Revision 1.7  2002/06/21 22:29:57  mrimer
// Made Disable() virtual function.
//
// Revision 1.6  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.5  2002/06/03 22:51:55  mrimer
// Added CEventHandlerWidget base class.
//
// Revision 1.4  2002/05/21 18:11:24  mrimer
// Added changing command key mappings in settings screen.
//
// Revision 1.3  2002/04/29 00:07:36  erikh2000
// Renamed "depressed" to "pressed" for consistency.
//
// Revision 1.2  2002/04/13 19:43:18  erikh2000
// Class can now take over event loop and handle repaints to itself.
//
// Revision 1.1  2002/04/12 22:49:32  erikh2000
// Initial check-in.
//
