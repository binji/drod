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

#include "TextBoxWidget.h"
#include "BitmapManager.h"
#include "EventHandlerWidget.h"
#include "FontManager.h"
#include "Inset.h"

#include <BackEndLib/Clipboard.h>
#include <BackEndLib/Types.h>
#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

#define ABS(a)	((a) > 0 ? (a) : -(a))
#define EDGE_OFFSET	(3)

//
//Public methods.
//

//******************************************************************************
CTextBoxWidget::CTextBoxWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,					//(in)	Required params for CWidget 
	const int nSetX, const int nSetY,	//		constructor.
	const UINT wSetW, const UINT wSetH,	//
	const UINT wMaxTextLength,				//(in)
	const bool bEnterSendsOK)				//(in) whether hitting Enter will return
													//		a TAG_OK event (default = true)
	: CFocusWidget(WT_TextBox, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, wTextDisplayIndex(0)
	, wMaxTextLength(wMaxTextLength)
	, wCursorIndex(0)
	, fontType(FONTLIB::F_Message)
	, bDrawCursor(true)
	, dwLastCursorDraw(0L)
	, bDigitsOnly(false)
   , bFilenameSafe(false)
   , wMarkStart((UINT)-1)
   , wMarkEnd((UINT)-1)
{
	this->pwczText = new WCHAR[this->wMaxTextLength+1];
	WCv(this->pwczText[0]) = '\0';

	if (bEnterSendsOK)
	{
		//Enter key acts as hotkey to perform OK command in parent widget.
		AddHotkey(SDLK_RETURN,TAG_OK);
		AddHotkey(SDLK_KP_ENTER,TAG_OK);
	}
}

//******************************************************************************
CTextBoxWidget::~CTextBoxWidget()
{
	delete[] this->pwczText;
}

//******************************************************************************
void CTextBoxWidget::Paint(
//Paint widget area.
//
//Params:
	bool bUpdateRect)	//(in)	If true (default) and destination
						//		surface is the screen, the screen
						//		will be immediately updated in
						//		the widget's rect.
{
	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	//Draw inset over entire widget area.
	SDL_Surface *pScreenSurface = GetDestSurface();
	DrawInset(this->x + nOffsetX, this->y + nOffsetY, this->w, this->h,
			this->pPartsSurface, pScreenSurface);

	DrawText(nOffsetX, nOffsetY);

	//Draw cursor if widget has focus.
	if (IsSelected())
		DrawCursor(nOffsetX, nOffsetY);

	PaintChildren(false);

	if (bUpdateRect) UpdateRect();
}

//******************************************************************************
void CTextBoxWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the text box.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
{
	const SDLKey key = KeyboardEvent.keysym.sym;
	switch (key) {
	case SDLK_BACKSPACE:
      if (HasSelection())
         DeleteSelected();
      else if (MoveCursorLeft())
		   DeleteAtCursor();
      break;
	case SDLK_DELETE:
   case SDLK_KP_PERIOD:
      if (!HasSelection())
		   DeleteAtCursor();
      else
         DeleteSelected();
		break;
	case SDLK_LEFT:
	case SDLK_KP4:
      if (KeyboardEvent.keysym.mod & KMOD_CTRL)
         MoveCursorLeftWord();
		else
         MoveCursorLeft();
		break;
	case SDLK_RIGHT:
	case SDLK_KP6:
      if (KeyboardEvent.keysym.mod & KMOD_CTRL)
         MoveCursorRightWord();
		else
         MoveCursorRight();
		break;
	case SDLK_HOME:
	case SDLK_PAGEUP:
		SetCursorIndex(0);
		break;
	case SDLK_END:
	case SDLK_PAGEDOWN:
		SetCursorIndex(WCSlen(this->pwczText));
		break;
/*COPYPASTE  COPYING AND PASTING - UNCOMMENT THIS AFTER 1.6 IS RELEASED - THIS ISN'T TESTED ENOUGH FOR 1.6
   case SDLK_v: // Paste
		if (KeyboardEvent.keysym.mod & KMOD_CTRL)
		{
          WSTRING insert;
          if (CClipboard::GetString(insert)) {
             for (WSTRING::const_iterator i = insert.begin(); i != insert.end(); i++) {
                InsertAtCursor(*i);
             }
          }
      }
      else
      {
		   const WCHAR wc = W_t(KeyboardEvent.keysym.unicode);
		   InsertAtCursor(wc);
      }
      break;

   case SDLK_c:  // Copy
		if (KeyboardEvent.keysym.mod & KMOD_CTRL)
		{
         if (!HasSelection()) break;
         WSTRING wstr = this->pwczText;
         UINT wStart, wEnd;

         GetSelection(wStart, wEnd);
         ASSERT(wEnd > wStart && wstr.size() >= (wEnd-wStart+1));

         CClipboard::SetString(wstr.substr(wStart, wEnd-wStart+1));
      }
      else
      {
		   const WCHAR wc = W_t(KeyboardEvent.keysym.unicode);
		   InsertAtCursor(wc);
      }
      break;

   case SDLK_x:  // Cut
		if (KeyboardEvent.keysym.mod & KMOD_CTRL)
		{
         if (!HasSelection()) break;
         WSTRING wstr = this->pwczText;
         UINT wStart, wEnd;

         GetSelection(wStart, wEnd);
         ASSERT(wEnd > wStart && wstr.size() >= (wEnd-wStart+1));

         CClipboard::SetString(wstr.substr(wStart, wEnd-wStart+1));
         DeleteSelected();
      } 
      else
      {
		   const WCHAR wc = W_t(KeyboardEvent.keysym.unicode);
		   InsertAtCursor(wc);
      }
      break;
*/
   
	default:
      //Only printable characters.
		if (key < SDLK_SPACE || key > SDLK_z ||
            (KeyboardEvent.keysym.mod & KMOD_CTRL))
         return;

      //When set, only accept digits.
		if (this->bDigitsOnly && (key < SDLK_0 || key > SDLK_9 ||
				(KeyboardEvent.keysym.mod & KMOD_SHIFT)))
			return;

      //When set, only accept filename-safe chars.
		const WCHAR wc = W_t(KeyboardEvent.keysym.unicode);
		if (this->bFilenameSafe && !charFilenameSafe(wc))
			return;

		InsertAtCursor(wc);
		break;
	}

	Paint();
}

//******************************************************************************
void CTextBoxWidget::HandleMouseDown(
//Processes a mouse event within the scope of the text box.
//
//Params:
	const SDL_MouseButtonEvent &MouseButtonEvent)	//(in) Event to handle.
{
	SetCursorByPos(MouseButtonEvent.x - this->x, MouseButtonEvent.y - this->y);
   if (MouseButtonEvent.state == SDL_PRESSED)
   {
      wMarkStart = wCursorIndex;
      wMarkEnd = (UINT)-1;
   }
	Paint();
}

//******************************************************************************
void CTextBoxWidget::HandleMouseUp(
//Processes a mouse event within the scope of the text box.
//
//Params:
	const SDL_MouseButtonEvent &MouseButtonEvent)	//(in) Event to handle.
{
	SetCursorByPos(MouseButtonEvent.x - this->x, MouseButtonEvent.y - this->y);
   if (MouseButtonEvent.state == SDL_RELEASED)
   {
      wMarkEnd = wCursorIndex;
   }
	Paint();
}

//******************************************************************************
void CTextBoxWidget::SetCursorByPos(
//Sets cursor to index of character rendered at xPos
//
//Params:
	int xPos, int yPos)	//(in) cursor's new position (in pixels)
{
	xPos -= EDGE_OFFSET;
	yPos -= EDGE_OFFSET;

	if (yPos < 0 || yPos >= static_cast<int>(this->h) || xPos >= static_cast<int>(this->w)) return;	//bounds checking

   const UINT wLength = WCSlen(this->pwczText);
	if (xPos <= 0 || wLength == 0)
	{
      //Place cursor at beginning of display.
		SetCursorIndex(this->wTextDisplayIndex);
	} else {
		//Calculate where to place cursor.
	   UINT wIndex = 0, wWidth = 0, wLastWidth = 0;
		UINT wTextW, wTextH;
		WCHAR *wStr = new WCHAR[wLength+2];
		WCv(wStr[0]) = '\0';

		while (wIndex <= wLength && static_cast<int>(wWidth) < xPos)
		{
			//Calculate width of string up to the nth character.
			wStr[wIndex] = this->pwczText[wIndex+this->wTextDisplayIndex];
			WCv(wStr[wIndex+1]) = '\0';
			g_pTheFM->GetTextWidthHeight(this->fontType, wStr, wTextW, wTextH);
			wLastWidth = wWidth;
			wWidth = wTextW;

			++wIndex;
		}
		delete[] wStr;

		if (wIndex > wLength)
			SetCursorIndex(wLength);
		else
			//Decide which side of the character that cursor falls on to place cursor.
			SetCursorIndex(ABS(wLastWidth - xPos) <= ABS(wWidth - xPos) ?
				wIndex+this->wTextDisplayIndex-1 : wIndex+this->wTextDisplayIndex);
	}
}

//******************************************************************************
void CTextBoxWidget::SetCursorIndex(
//Sets cursor to index.  Updates displayed text region.
//
//Params:
	const UINT wIndex)	//(in) cursor's new position
{
	this->wCursorIndex = wIndex;

	//Set which part of string to display in box.
	if (wIndex < this->wTextDisplayIndex)
		this->wTextDisplayIndex = wIndex;
	else
	{
		//Calculate what part of string to show.
		UINT wTextW, wTextH;
		int nOffsetX, nOffsetY;
		WCHAR *wStr = new WCHAR[WCSlen(this->pwczText)+1];
		GetScrollOffset(nOffsetX, nOffsetY);
		do
		{
			WCSncpy(wStr, this->pwczText + this->wTextDisplayIndex,
					this->wCursorIndex - this->wTextDisplayIndex);
			WCv(wStr[this->wCursorIndex - this->wTextDisplayIndex]) = '\0';
			g_pTheFM->GetTextWidthHeight(this->fontType, wStr, wTextW, wTextH);
			//Move right until the cursor is showing,
			//or left while there is extra space on the right.
			if (wTextW > this->w - (nOffsetX + EDGE_OFFSET * 2))
				++this->wTextDisplayIndex;
			else if (wTextW < this->w*3/4 - (nOffsetX + EDGE_OFFSET * 2))
			{
				if (this->wTextDisplayIndex > 0)
					--this->wTextDisplayIndex;
				else
					break;
			}
			else
				break;
		} while (true);
		delete[] wStr;
	}
}

//*****************************************************************************
bool CTextBoxWidget::CharIsWordbreak(const WCHAR wc) const
//Returns: whether character to right of cursor is whitespace,
//a dash (word break), or NULL
{
   return iswspace(wc) != 0 || wc == '-' || wc == 0;
}

//*****************************************************************************
bool CTextBoxWidget::CursorOnWhitespace() const
//Returns: whether character to right of cursor is whitespace,
//a dash (word break), or NULL
{
   return CharIsWordbreak(this->pwczText[this->wCursorIndex]);
}

//*****************************************************************************
bool CTextBoxWidget::MoveCursorLeft()
//Moves cursor left one character.
//Returns: whether operation was successful (possible)
{
	if (this->wCursorIndex == 0)
      return false;

   SetCursorIndex(this->wCursorIndex-1);
   return true;
}

//******************************************************************************
bool CTextBoxWidget::MoveCursorLeftWord()
//Moves cursor left to start of word.
//Returns: whether operation was successful (possible)
{
	if (this->wCursorIndex == 0)
      return false;

   //Go left at least one space.
   SetCursorIndex(this->wCursorIndex-1);

   //Pass whitespace.
   while (this->wCursorIndex > 0 && CursorOnWhitespace())
      SetCursorIndex(this->wCursorIndex-1);
   //Go to beginning of word.
   bool bFoundWord = false;
   while (this->wCursorIndex > 0 && !CursorOnWhitespace())
   {
      SetCursorIndex(this->wCursorIndex-1);
      bFoundWord = true;
   }

   //Passed beginning of word -- go back.
   if (bFoundWord && CursorOnWhitespace())
      SetCursorIndex(this->wCursorIndex+1);

   return true;
}

//******************************************************************************
bool CTextBoxWidget::MoveCursorRight()
//Moves cursor right one character.
//Returns: whether operation was successful (possible)
{
   ASSERT(this->wCursorIndex <= WCSlen(this->pwczText));
	if (this->wCursorIndex == WCSlen(this->pwczText))
      return false;

   SetCursorIndex(this->wCursorIndex+1);
   return true;
}

//******************************************************************************
bool CTextBoxWidget::MoveCursorRightWord()
//Moves cursor right to start of word.
//Returns: whether operation was successful (possible)
{
   const UINT wLength = WCSlen(this->pwczText);
   ASSERT(this->wCursorIndex <= wLength);
	if (this->wCursorIndex == wLength)
      return false;

   //Pass word.
   while (this->wCursorIndex < wLength && !CursorOnWhitespace())
      SetCursorIndex(this->wCursorIndex+1);

   //Pass whitespace.
   while (this->wCursorIndex < wLength && CursorOnWhitespace())
      SetCursorIndex(this->wCursorIndex+1);

   return true;
}

//******************************************************************************
void CTextBoxWidget::SetText(
//Sets text and puts cursor at end.
//
//Params:
	const WCHAR* text)	//(in)
{
	const UINT wTextLen = (text ? WCSlen(text) : 0);
   ASSERT(wTextLen <= this->wMaxTextLength);
	if (wTextLen > this->wMaxTextLength)
	{
      //Truncate text to max limit.  (Sub-optimal behavior, but could avoid further bugs.)
		WCSncpy(this->pwczText, text, this->wMaxTextLength);
		WCv(this->pwczText[this->wMaxTextLength]) = '\0';
	}
	else
		WCScpy(this->pwczText,(text ? text : wszEmpty));

	SetCursorIndex(wTextLen);
}

//******************************************************************************
bool CTextBoxWidget::InsertAtCursor(
//Inserts a character at the current cursor position.
//
//Returns: true if character was inserted, else false
//
//Params:
	const WCHAR c)	//(in)
{
	UINT wCursor = WCSlen(this->pwczText) + 1;

	if (wCursor > this->wMaxTextLength)
		return false;

	//move over everything that comes after
	while (wCursor > this->wCursorIndex)
	{
		this->pwczText[wCursor] = this->pwczText[wCursor - 1];
		--wCursor;
	}

	this->pwczText[wCursor] = c;

	SetCursorIndex(this->wCursorIndex+1);

	//Call OnSelectChange() notifier.
	CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
	if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());

	return true;
}

//******************************************************************************
void CTextBoxWidget::DeleteAtCursor()
//Deletes the character at the current cursor position.
{
	UINT wCursor = this->wCursorIndex;

	if (this->pwczText[wCursor] != L'\0')
	{
		//Call OnSelectChange() notifier.
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());
	}

	while (this->pwczText[wCursor] != L'\0')
	{
		this->pwczText[wCursor] = this->pwczText[wCursor + 1];
		++wCursor;
	}

   //In case graphical cursor position needs updating (due to word wrap, etc.)
   SetCursorIndex(this->wCursorIndex);
}

//
//Protected methods.
//

//******************************************************************************
bool CTextBoxWidget::DeleteSelected()
//Deletes the selected characters.
//Returns: true if successful, false otherwise (no selection?)
{
   if (!HasSelection()) return false;

   UINT wStart;
   UINT wEnd;
   GetSelection(wStart, wEnd);

   UINT wOffset = wEnd-wStart+1;
   UINT wCursor = wStart;

   while (this->pwczText[wCursor+wOffset-1] != L'\0')
   {
      this->pwczText[wCursor] = this->pwczText[wCursor+wOffset];
      wCursor++;
   }
   
   this->wCursorIndex = wStart;
   SetCursorIndex(this->wCursorIndex);

   this->wMarkStart = (UINT)-1;
   this->wMarkEnd = (UINT)-1;
   return true;
}

//******************************************************************************
bool CTextBoxWidget::HasSelection()
//Determines if text is currently selected.
//Returns: true if there is a selection
{
   //COPYPASTE Remove this return
   return false;

   if (this->wMarkStart == (UINT)-1 && this->wMarkEnd == (UINT)-1) return false;
   else return true;
}


//******************************************************************************
bool CTextBoxWidget::GetSelection(UINT &wStart, UINT &wEnd)
//Gets the start and end of the current selection.
//Returns: true if successful, false if there is no selection
{
   if (!HasSelection()) return false;
   wStart = (this->wMarkStart == (UINT)-1) ? 0 : this->wMarkStart;
   wEnd = (this->wMarkEnd == (UINT)-1) ? WCSlen(this->pwczText) : this->wMarkEnd;

   return true;
}


//******************************************************************************
void CTextBoxWidget::HandleAnimate()
//Draw blinking cursor if widget has focus (once per second).
{
	if (IsSelected())
	{
		const Uint32 dwNow = SDL_GetTicks();
		if (dwNow - this->dwLastCursorDraw > 500)
		{
			this->dwLastCursorDraw = dwNow;
			this->bDrawCursor = !this->bDrawCursor;
			Paint();
		}
	}
}

//
//Private methods.
//

//******************************************************************************
void CTextBoxWidget::DrawText(
//Draw text, starting from wTextDisplayIndex.
//
//Params:
	const int nOffsetX, const int nOffsetY)	//(in) Drawing offsets.
{
   g_pTheFM->DrawTextXY(this->fontType,
         this->pwczText + this->wTextDisplayIndex, GetDestSurface(),
         this->x + EDGE_OFFSET + nOffsetX, this->y + nOffsetY,
         this->w - (nOffsetX + EDGE_OFFSET) * 2, this->h - nOffsetY * 2);
}

//******************************************************************************
void CTextBoxWidget::DrawCursor(
//Draw cursor.
//
//Params:
	const int nOffsetX, const int nOffsetY)	//(in) Drawing offsets.
{
	if (!this->bDrawCursor)
      return;

   //Calculate where to place cursor.
   ASSERT(this->wCursorIndex >= this->wTextDisplayIndex);
	WCHAR *wStr = new WCHAR[this->wCursorIndex+1];
	WCSncpy(wStr, this->pwczText + this->wTextDisplayIndex,
			this->wCursorIndex - this->wTextDisplayIndex);
	WCv(wStr[this->wCursorIndex - this->wTextDisplayIndex]) = '\0';
	UINT wTextW, wTextH;
	g_pTheFM->GetTextWidthHeight(this->fontType, wStr, wTextW, wTextH);
	delete[] wStr;

	//Draw cursor
	static const SURFACECOLOR Black = {0,0,0};
	static const int nBorder = 2;	//1 + edge
	DrawCol(this->x + EDGE_OFFSET + wTextW + nOffsetX,
			this->y + nBorder + nOffsetY, this->h - 2 * nBorder,Black);
}

// $Log: TextBoxWidget.cpp,v $
// Revision 1.14  2004/01/02 01:11:32  mrimer
// Added an assertion.
//
// Revision 1.13  2003/08/15 17:53:49  schik
// Fixed a couple of bugs.
// Added support for cut/copy/paste, but it's currently commented out, and should stay so until after 1.6 is out - needs testing.
//
// Revision 1.12  2003/08/01 04:19:20  mrimer
// Fixed bug: cursor position not updating in multi-line textbox when char is deleted.
//
// Revision 1.11  2003/07/22 19:36:59  mrimer
// Linux port fixes (committted on behalf of Gerry JJ).
//
// Revision 1.10  2003/07/19 02:13:59  mrimer
// Added more editor commands.  Fixed memory leaks.  Cleaned up the code.
//
// Revision 1.9  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.8  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/07/07 23:39:18  mrimer
// Fixed backspace command to work correctly.
//
// Revision 1.6  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/12 21:45:20  mrimer
// Removed duplicated code.
//
// Revision 1.4  2003/06/10 01:23:27  mrimer
// Added an optional character filter for entering filenames.
//
// Revision 1.3  2003/06/09 19:22:16  mrimer
// Added optional max width/height params to FM::DrawTextXY().
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:31  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.30  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.29  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.28  2003/01/08 00:55:26  mrimer
// Improved showing of long text, or when text is shortened, to fill more of widget space.
//
// Revision 1.27  2002/12/22 02:17:56  mrimer
// Revised constructor to make adding ENTER hotkey optional.
//
// Revision 1.26  2002/11/15 02:22:33  mrimer
// Enhanced to support scrolling text when string is longer than widget.
// Modified to optionally accept only digits.
// Made some vars and parameters const.
//
// Revision 1.25  2002/10/21 20:20:25  mrimer
// Added unicode character support.  Removed MapKeysym().
//
// Revision 1.24  2002/09/24 21:30:39  mrimer
// Added Enter hotkey to activate OK command.
//
// Revision 1.23  2002/09/03 21:30:34  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.22  2002/07/22 18:46:05  mrimer
// Made cursor blink.
//
// Revision 1.21  2002/07/19 20:38:37  mrimer
// Revised HandleKeyDown() to return no type.
//
// Revision 1.20  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.19  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.18  2002/06/14 22:19:30  mrimer
// Updated cursor graphic.
//
// Revision 1.17  2002/06/14 18:33:41  mrimer
// Fixed some event handling.
//
// Revision 1.16  2002/06/14 17:43:15  mrimer
// Added bounds checking for moving the cursor on mouse clicks.
//
// Revision 1.15  2002/06/13 22:12:06  mrimer
// Refined SetText().
//
// Revision 1.14  2002/06/13 21:10:35  erikh2000
// Removed all implicit calls to Paint().
//
// Revision 1.13  2002/06/11 22:49:59  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.12  2002/06/09 06:46:08  erikh2000
// Added a GetText() method.
// Changed SetText() so that it does not modify the input parameter.
//
// Revision 1.11  2002/06/07 18:20:59  mrimer
// Reordered includes.
//
// Revision 1.10  2002/06/07 17:51:05  mrimer
// Tweaking.
//
// Revision 1.9  2002/06/05 03:13:26  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.8  2002/06/03 22:58:07  mrimer
// Added/refined focusability.
//
// Revision 1.7  2002/05/24 14:13:08  mrimer
// Now inherits from CFocusWidget.
//
// Revision 1.6  2002/05/24 13:51:51  mrimer
// Added Select() method.
//
// Revision 1.5  2002/05/23 21:08:23  mrimer
// Added non-alpha char shift-handling (hard-coded).
//
// Revision 1.4  2002/05/21 21:42:16  mrimer
// Implemented TextBoxWidget.
//
// Revision 1.3  2002/04/29 00:11:31  erikh2000
// Fixed a dumb little error.
//
// Revision 1.2  2002/04/29 00:05:30  erikh2000
// Paint() will draw an inset area to screen.
//
// Revision 1.1  2002/04/26 20:47:05  erikh2000
// Initial check-in.
//
