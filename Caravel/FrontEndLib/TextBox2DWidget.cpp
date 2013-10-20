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

#include "TextBox2DWidget.h"
#include "BitmapManager.h"
#include "EventHandlerWidget.h"
#include "FontManager.h"
#include "Inset.h"

#include <BackEndLib/Types.h>
#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>
#include <BackEndLib/Clipboard.h>

#define ABS(a)	((a) > 0 ? (a) : -(a))
#define EDGE_OFFSET	(3)

const UINT BORDER = 2;	//1 + edge

//
//Public methods.
//

//******************************************************************************
CTextBox2DWidget::CTextBox2DWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,					//(in)	Required params for CWidget 
	const int nSetX, const int nSetY,	//		constructor.
	const UINT wSetW, const UINT wSetH,	//
	const UINT nMaxTextLength)				//(in)
	: CTextBoxWidget(dwSetTagNo, nSetX, nSetY, wSetW, wSetH, nMaxTextLength, false)
   , wCursorX(0), wCursorY(0)
{
}

//******************************************************************************
void CTextBox2DWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the text box.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
{
	const SDLKey key = KeyboardEvent.keysym.sym;
	switch (key)
   {
	   case SDLK_UP:
	   case SDLK_KP8:
         MoveCursorUp();
		   break;
	   case SDLK_DOWN:
	   case SDLK_KP2:
         MoveCursorDown();
		   break;
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
      {
     		const WCHAR wc = W_t(KeyboardEvent.keysym.unicode);
     		InsertAtCursor(wc);
         break;
      }
      case SDLK_HOME:
      case SDLK_KP7:
         MoveCursorToLineStart();
         break;
      case SDLK_END:
      case SDLK_KP1:
         MoveCursorToLineEnd();
         break;
      case SDLK_PAGEDOWN:
      case SDLK_KP3:
         MoveCursorDownPage();
         break;
      case SDLK_PAGEUP:
      case SDLK_KP9:
         MoveCursorUpPage();
         break;

      case SDLK_LEFT:
      case SDLK_KP4:
         CTextBoxWidget::HandleKeyDown(KeyboardEvent);
         if (KeyboardEvent.keysym.mod & KMOD_SHIFT) {
            // Add this to the current copy buffer.
         }
         break;
      
      case SDLK_RIGHT:
      case SDLK_KP6:
         CTextBoxWidget::HandleKeyDown(KeyboardEvent);
         if (KeyboardEvent.keysym.mod & KMOD_SHIFT) {
            // Add this to the current copy buffer.
         }
         break;

      default:
         CTextBoxWidget::HandleKeyDown(KeyboardEvent);
         return;  //don't Paint
	}
   Paint();
}

//*****************************************************************************
bool CTextBox2DWidget::MoveCursorUp(UINT wNumLines)
//Moves cursor up one line.
//Returns: whether operation was successful (possible)
{
   for (UINT wLine = 0; wLine < wNumLines; wLine++) {
	   if (this->wCursorIndex == 0)
         return false;

      const UINT wLineOfTextH = g_pTheFM->GetFontHeight(this->fontType);
      if (wLineOfTextH > this->wCursorY)
      {
         const UINT wCursorX = this->wCursorX, wCursorY = this->wCursorY;
         MoveViewUp();
         SetCursorByPos(wCursorX, wCursorY);
      } else
         SetCursorByPos(this->wCursorX, this->wCursorY - wLineOfTextH-3);
   }
   return true;
}

//******************************************************************************
bool CTextBox2DWidget::MoveCursorDown(UINT wNumLines)
//Moves cursor down one line.
//Returns: whether operation was successful (possible)
{
   for (UINT wLine = 0; wLine < wNumLines; wLine++) {
      ASSERT(this->wCursorIndex <= WCSlen(this->pwczText));
	   if (this->wCursorIndex == WCSlen(this->pwczText))
         return false;

      const UINT wLineOfTextH = g_pTheFM->GetFontLineHeight(this->fontType) + 1;
      if (this->wCursorY + wLineOfTextH >= this->h)
      {
         const UINT wCursorX = this->wCursorX, wCursorY = this->wCursorY;
         MoveViewDown();
         SetCursorByPos(wCursorX, wCursorY);
      } else
         SetCursorByPos(this->wCursorX, this->wCursorY + wLineOfTextH+3);
   }
   return true;
}

//******************************************************************************
bool CTextBox2DWidget::MoveCursorToLineStart()
//Moves cursor to beginning of line.
//Returns: true
{
   SetCursorByPos(0, this->wCursorY);
   return true;
}

//******************************************************************************
bool CTextBox2DWidget::MoveCursorToLineEnd()
//Moves cursor to end of line.
//Returns: true
{
   SetCursorByPos(GetW()-1, this->wCursorY);
   return true;
}

//******************************************************************************
bool CTextBox2DWidget::MoveCursorUpPage()
//Moves cursor down one page (widget height).
//Returns: Whether or not the operation was successful (possible)
{
   ASSERT(this->wCursorIndex <= WCSlen(this->pwczText));
	if (this->wCursorIndex == 0)
      return false;

   const UINT wCursorX = this->wCursorX, wCursorY = this->wCursorY;
   const UINT wLineOfTextH = g_pTheFM->GetFontLineHeight(this->fontType) + 1;
   const UINT wPageLines = GetH() / wLineOfTextH;
   UINT wNumMoved = MoveViewUp(wPageLines);
   if (wNumMoved < wPageLines) MoveCursorUp(wPageLines - wNumMoved);
   else {
      SetCursorByPos(wCursorX, wCursorY);
   }

   return true;
}

//******************************************************************************
bool CTextBox2DWidget::MoveCursorDownPage()
//Moves cursor up one page (widget height).
//Returns: Whether or not the operation was successful (possible)
{
   ASSERT(this->wCursorIndex <= WCSlen(this->pwczText));
	if (this->wCursorIndex == WCSlen(this->pwczText))
      return false;

   const UINT wCursorX = this->wCursorX, wCursorY = this->wCursorY;
   const UINT wLineOfTextH = g_pTheFM->GetFontLineHeight(this->fontType) + 1;
   const UINT wPageLines = GetH() / wLineOfTextH;
   UINT wNumMoved = MoveViewDown(wPageLines);
   if (wNumMoved < wPageLines) MoveCursorDown(wPageLines - wNumMoved);
   else {
      SetCursorByPos(wCursorX, wCursorY);
   }

   return true;
}

//******************************************************************************
void CTextBox2DWidget::SetCursorByPos(
//Sets cursor to index of character rendered at (xPos,yPos)
//
//Params:
	int xPos, int yPos)	//(in) cursor's new position (in pixels)
{
	if (yPos >= static_cast<int>(this->h) ||
         xPos >= static_cast<int>(this->w)) return;	//bounds checking

   const UINT wLength = WCSlen(this->pwczText);
	if ((xPos <= 0 && yPos <= 0) || wLength == 0)
	{
      //Place cursor at beginning of display.
		SetCursorIndex(this->wTextDisplayIndex);
	} else {
		//Calculate where to place cursor.
      if (xPos < 0) xPos = 0;
      if (yPos < 0) yPos = 0;
      const UINT wLineOfTextH = g_pTheFM->GetFontHeight(this->fontType);
	   UINT wIndex = this->wTextDisplayIndex, wWidth = 0, wHeight = 0, wLastHeight = (UINT)-1;
      //Move down to correct line.
      GetCursorPosition(wIndex, wWidth, wHeight);
		while (wIndex <= wLength && static_cast<int>(wHeight + wLineOfTextH) <= yPos)
		{
			//Calculate cursor position at the nth character.
         GetCursorPosition(wIndex, wWidth, wHeight);
         ++wIndex;
		}
      wLastHeight = wHeight;
      //Move over to correct character.
      while (wIndex <= wLength && static_cast<int>(wWidth) < xPos)
      {
         GetCursorPosition(wIndex, wWidth, wHeight);
         if (wHeight > wLastHeight)
         {
            //We were already on the correct line, and have just moved to
            //the next one -- stop cursor at the end of the correct line.
            break;
         }
         wLastHeight = wHeight;
         if (static_cast<int>(wWidth) <= xPos)
            ++wIndex;
      }
		if (wIndex > wLength)
			SetCursorIndex(wLength);
		else
         SetCursorIndex(wIndex > this->wTextDisplayIndex ? wIndex-1 : wIndex);

      GetCursorPosition(wCursorIndex, wWidth, wHeight);
	}
}

//******************************************************************************
void CTextBox2DWidget::SetCursorIndex(
//Sets cursor to index.  Updates displayed text region.
//
//Params:
	const UINT wIndex)	//(in) cursor's new position
{
   ASSERT(wIndex <= WCSlen(this->pwczText));
   //if (wIndex == this->wCursorIndex) return;  //Don't need: in case text was changed.

   //If cursor has moved outside viewing area, update it.
   if (wIndex == 0)
      this->wTextDisplayIndex = 0;  //view from beginning
   else if (wIndex < this->wTextDisplayIndex)
   {
      if (this->wTextDisplayIndex > WCSlen(this->pwczText))
         this->wTextDisplayIndex = WCSlen(this->pwczText);
      //Move view up until cursor comes into view.
      do {
         MoveViewUp();
      } while (wIndex < this->wTextDisplayIndex);
   } else {
      const UINT wLineOfTextH = g_pTheFM->GetFontHeight(this->fontType);
      UINT wCursorX, wCursorY;
	   int nOffsetX, nOffsetY;
	   GetScrollOffset(nOffsetX, nOffsetY);
      GetCursorPosition(wIndex, wCursorX, wCursorY);
      //Move viewing index down line by line until cursor is in viewing area.
      while (wCursorY + wLineOfTextH > this->h - BORDER)
      {
         MoveViewDown();
         GetCursorPosition(wIndex, wCursorX, wCursorY);
      }
   }

   //Calculate where to place cursor.
   //(This is processor intensive, so we'll store the value.)
   GetCursorPosition(wIndex, this->wCursorX, this->wCursorY);
   this->wCursorIndex = wIndex;
}

//
//Private methods.
//

//******************************************************************************
void CTextBox2DWidget::DrawText(
//Draw text, starting from wTextDisplayIndex.
//
//Params:
	const int nOffsetX, const int nOffsetY)	//(in) Drawing offsets.
{
   g_pTheFM->DrawTextToRect(this->fontType,
         this->pwczText + this->wTextDisplayIndex,
         this->x + EDGE_OFFSET + nOffsetX, this->y + nOffsetY,
         this->w - (nOffsetX + EDGE_OFFSET) * 2, this->h - nOffsetY * 2,
         GetDestSurface());
}

//******************************************************************************
void CTextBox2DWidget::DrawCursor(
//Draw cursor.
//
//Params:
	const int nOffsetX, const int nOffsetY)	//(in) Drawing offsets.
{
	if (!this->bDrawCursor)
      return;

	//Draw cursor.
	static const SURFACECOLOR Black = {0,0,0};
   const int yPos = this->y + this->wCursorY + nOffsetY;
   const UINT wLineOfTextH = g_pTheFM->GetFontHeight(this->fontType);
	DrawCol(this->x + EDGE_OFFSET + this->wCursorX + nOffsetX, yPos,
         wLineOfTextH - 2 * BORDER,Black);
}

//******************************************************************************
void CTextBox2DWidget::GetCursorPosition(
//Get top of cursor's location (x,y).
//
//Params:
	const UINT wCursorIndex, UINT &wCursorX, UINT &wCursorY)	//(out) Cursor location.
const
{
   ASSERT(wCursorIndex >= this->wTextDisplayIndex);
   const UINT wLength = WCSlen(this->pwczText);
	WCHAR *wStr = new WCHAR[wLength+1];
	UINT wTextW, wTextH;
   UINT wCursorI = wCursorIndex;
   bool bCursorOnWord = false;
   if (!CharIsWordbreak(this->pwczText[wCursorIndex]))
   {
      //When cursor is on a word, we need to find out whether it will be pushed
      //to the next line:
      //1. Find beginning of word cursor is on.
      while (wCursorI > 0 && !CharIsWordbreak(this->pwczText[wCursorI-1]))
         --wCursorI;
      if (wCursorI < this->wTextDisplayIndex)  //stay within displayed text
         wCursorI = this->wTextDisplayIndex;
      bCursorOnWord = true;
   }
   //Get dimensions of text up to this spot.
	WCSncpy(wStr, this->pwczText + this->wTextDisplayIndex,
			 wCursorI - this->wTextDisplayIndex);
   ASSERT(wCursorI - this->wTextDisplayIndex <= wLength);
	WCv(wStr[wCursorI - this->wTextDisplayIndex]) = '\0';
   const UINT wMaxWidth = this->w - 2 * EDGE_OFFSET;
	wCursorX = g_pTheFM->GetTextRectHeight(this->fontType, wStr,
         wMaxWidth, wTextW, wTextH);
   const UINT wLineOfTextH = g_pTheFM->GetFontHeight(this->fontType);
   wCursorY = wTextH - wLineOfTextH;
   if (bCursorOnWord)
   {
      //2. Get dimensions of next word.
      UINT wWordStartIndex = wCursorI;
      ++wCursorI;
      while (wCursorI < wLength && !CharIsWordbreak(this->pwczText[wCursorI]))
         ++wCursorI;
      UINT wCharsNotDrawn;
      do {
         const UINT wWordLength = wCursorI - wWordStartIndex;
	      WCSncpy(wStr, this->pwczText + wWordStartIndex, wWordLength);
	      WCv(wStr[wWordLength]) = '\0';
         g_pTheFM->CalcPartialWord(this->fontType, wStr,
               wWordLength, wMaxWidth-wCursorX, wCharsNotDrawn);
         if (wCharsNotDrawn)
         {
            //The word can't fit on this line...
            UINT wCharsRendered = wWordLength - wCharsNotDrawn;
            g_pTheFM->CalcPartialWord(this->fontType, wStr,
                  wWordLength, wMaxWidth, wCharsNotDrawn);
            if (wCharsNotDrawn)
            {
               if (wWordStartIndex + wCharsRendered > wCursorIndex)
               {
                  //...but part of it is rendered on this line, and the cursor is
                  //within this part.  Find out where in Step 3.
                  break;
               }
            } else {
               //...but it can fit on the next line, so it will all be
               //rendered there.  Find out how far cursor is over in Step 3.
               wCharsRendered = 0;
            }
            //...else the cursor won't be rendered on this line.
            wCursorX = 0;
            wCursorY += g_pTheFM->GetFontLineHeight(this->fontType);
            wWordStartIndex += wCharsRendered;
         }
      } while (wCharsNotDrawn);
      //3. Find cursor's position relative to the start of the word it's on
      //(or the piece of the segmented word on a lower line).
	   WCSncpy(wStr, this->pwczText + wWordStartIndex, wCursorIndex - wWordStartIndex);
	   WCv(wStr[wCursorIndex - wWordStartIndex]) = '\0';
	   UINT wWordW, wWordH;
	   g_pTheFM->GetTextRectHeight(this->fontType, wStr, wMaxWidth, wWordW, wWordH);
      wCursorX += wWordW;
   }
	delete[] wStr;
}

//******************************************************************************
UINT CTextBox2DWidget::MoveViewDown(const UINT wNumLines)
//Move viewing area down a line.
//Returns: Number of lines actually moved down
{
   UINT numMoved = 0;
   for (UINT line = 0; line < wNumLines; line++) {
      UINT wIndex = this->wTextDisplayIndex;
      const UINT wLength = WCSlen(this->pwczText);
      if (wIndex > wLength) break;
      UINT wCursorX, wCursorY, wCursorNewY;
      GetCursorPosition(wIndex, wCursorX, wCursorY);
      //Find position of first character on next line.
      do
      {
         GetCursorPosition(++wIndex, wCursorX, wCursorNewY);
      } while (wIndex < wLength && wCursorNewY <= wCursorY);
      this->wTextDisplayIndex = wIndex;
      numMoved++;
   }
   return numMoved;
}

//******************************************************************************
UINT CTextBox2DWidget::MoveViewUp(const UINT wNumLines)
//Move viewing area up a line.
//Returns: Number of lines actually moved up
{
   UINT wNumMoved = 0;
   for (UINT wLine = 0; wLine < wNumLines; wLine++) {
      UINT wIndex = this->wTextDisplayIndex;
      if (wIndex == 0) break;
      UINT wCursorX, wCursorY;
      //Find position of first character on previous line.
      const UINT wLineOfTextH = g_pTheFM->GetFontHeight(this->fontType);
      do
      {
         --this->wTextDisplayIndex;
         GetCursorPosition(wIndex, wCursorX, wCursorY);
      } while (this->wTextDisplayIndex > 0 && wCursorY < wLineOfTextH);
      //Keep moving back while more will fit on the previous line.
      while (this->wTextDisplayIndex > 0 && wCursorY < 2 * wLineOfTextH && wCursorX == 0)
      {
         --this->wTextDisplayIndex;
         GetCursorPosition(wIndex, wCursorX, wCursorY);
      }
      if (this->wTextDisplayIndex == 0)
         break;
      //Moved back one too far.
      if (wCursorY >= 2 * wLineOfTextH || wCursorX > 0)
         ++this->wTextDisplayIndex;
      wNumMoved++;
   }
   return wNumMoved;
}

// $Log: TextBox2DWidget.cpp,v $
// Revision 1.6  2004/05/20 17:39:23  mrimer
// Fixed bug: incorrect text view when text is changed by external call.
//
// Revision 1.5  2004/01/02 01:11:32  mrimer
// Added an assertion.
//
// Revision 1.4  2003/08/15 17:53:49  schik
// Fixed a couple of bugs.
// Added support for cut/copy/paste, but it's currently commented out, and should stay so until after 1.6 is out - needs testing.
//
// Revision 1.3  2003/08/14 05:10:55  schik
// HOME and END now work as the user would expect.
//
// Revision 1.2  2003/08/01 04:20:06  mrimer
// Fixed bug: cursor not drawn in correct position when word wraps to next line.
//
// Revision 1.1  2003/07/19 02:10:03  mrimer
// Initial check-in.
//
