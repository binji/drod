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
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef TEXTBOXWIDGET_H
#define TEXTBOXWIDGET_H

#include "FocusWidget.h"
#include "FontManager.h"
#include <BackEndLib/Types.h>
#include <BackEndLib/Ports.h>

//Height of standard single-line text box.
const UINT CY_STANDARD_TBOX = 23;

//******************************************************************************
class CTextBoxWidget : public CFocusWidget
{
	public:
		CTextBoxWidget(const DWORD dwSetTagNo, const int nSetX, const int nSetY,
				const UINT wSetW, const UINT wSetH, const UINT wMaxTextLength=255,
				const bool bEnterSendsOK=true);
		virtual ~CTextBoxWidget();

		virtual bool   AcceptsTextEntry() {return true;}
		const WCHAR *		GetText() const {return this->pwczText;}
		void				DeleteAtCursor();
		virtual void   HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);
		virtual void   HandleMouseDown(const SDL_MouseButtonEvent &MouseButtonEvent);
      virtual	void	HandleMouseUp(const SDL_MouseButtonEvent &MouseButtonEvent);
		bool				InsertAtCursor(const WCHAR c);
		virtual void	Paint(bool bUpdateRect = true);
		void				SetDigitsOnly(const bool bAccept = true)
				{this->bDigitsOnly = bAccept;}
      void           SetFilenameSafe(const bool bFlag = true)
				{this->bFilenameSafe = bFlag;}
		virtual void	SetCursorByPos(int xPos, int yPos);
		void				SetText(const WCHAR* text);

	protected:
		virtual bool   IsAnimated() const {return true;}
		virtual void   HandleAnimate();

      bool           CharIsWordbreak(const WCHAR wc) const;
      bool           CursorOnWhitespace() const;

		virtual void	DrawText(const int nOffsetX, const int nOffsetY);
		virtual void	DrawCursor(const int nOffsetX, const int nOffsetY);

      bool				MoveCursorLeft();
		bool				MoveCursorLeftWord();
		bool				MoveCursorRight();
		bool				MoveCursorRightWord();

      bool           DeleteSelected();
      bool           HasSelection();
      bool           GetSelection(UINT &wStart, UINT &wEnd);

		virtual void	SetCursorIndex(const UINT wIndex);

		WCHAR			*	pwczText;
		UINT				wTextDisplayIndex;	//index of first character in display
		UINT				wMaxTextLength;
		UINT				wCursorIndex;		//cursor position in text
		UINT				fontType;
		bool				bDrawCursor;

   private:
		DWORD				dwLastCursorDraw;	//cursor animation
		bool				bDigitsOnly;	//only allow digits to be input
      bool           bFilenameSafe; //only allow certain chars to be input

      UINT           wMarkStart, wMarkEnd; //for marking text to cut/copy
};

#endif //#ifndef TEXTBOXWIDGET_H

// $Log: TextBoxWidget.h,v $
// Revision 1.8  2003/08/15 17:53:49  schik
// Fixed a couple of bugs.
// Added support for cut/copy/paste, but it's currently commented out, and should stay so until after 1.6 is out - needs testing.
//
// Revision 1.7  2003/07/22 19:36:59  mrimer
// Linux port fixes (committted on behalf of Gerry JJ).
//
// Revision 1.6  2003/07/19 02:13:59  mrimer
// Added more editor commands.  Fixed memory leaks.  Cleaned up the code.
//
// Revision 1.5  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.4  2003/07/07 23:39:19  mrimer
// Fixed backspace command to work correctly.
//
// Revision 1.3  2003/06/10 01:23:27  mrimer
// Added an optional character filter for entering filenames.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.20  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.19  2002/12/22 02:17:56  mrimer
// Revised constructor to make adding ENTER hotkey optional.
//
// Revision 1.18  2002/11/15 02:22:33  mrimer
// Enhanced to support scrolling text when string is longer than widget.
// Modified to optionally accept only digits.
// Made some vars and parameters const.
//
// Revision 1.17  2002/10/21 20:20:25  mrimer
// Added unicode character support.  Removed MapKeysym().
//
// Revision 1.16  2002/09/24 21:30:39  mrimer
// Added Enter hotkey to activate OK command.
//
// Revision 1.15  2002/09/03 21:30:34  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.14  2002/07/22 18:46:05  mrimer
// Made cursor blink.
//
// Revision 1.13  2002/07/22 01:00:29  erikh2000
// Made destructor declaration virtual just so that the virtual status is more obvious.
//
// Revision 1.12  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.11  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.10  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.9  2002/06/14 18:33:15  mrimer
// Changed SetCursorByPos() prototype.
//
// Revision 1.8  2002/06/09 06:46:08  erikh2000
// Added a GetText() method.
// Changed SetText() so that it does not modify the input parameter.
//
// Revision 1.7  2002/06/05 03:07:44  mrimer
// Added IsActive().
//
// Revision 1.6  2002/06/03 22:56:29  mrimer
// Added widget focusability.
//
// Revision 1.5  2002/05/24 14:13:08  mrimer
// Now inherits from CFocusWidget.
//
// Revision 1.4  2002/05/24 13:51:51  mrimer
// Added Select() method.
//
// Revision 1.3  2002/05/23 21:08:23  mrimer
// Added non-alpha char shift-handling (hard-coded).
//
// Revision 1.2  2002/05/21 21:42:16  mrimer
// Implemented TextBoxWidget.
//
// Revision 1.1  2002/04/26 20:47:06  erikh2000
// Initial check-in.
//
