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

#ifndef TEXTBOX2DWIDGET_H
#define TEXTBOX2DWIDGET_H

#include "TextBoxWidget.h"

//Height of standard multi-line text box.
const UINT CY_STANDARD_TBOX2D = 210;

//******************************************************************************
class CTextBox2DWidget : public CTextBoxWidget
{
public:
	CTextBox2DWidget(const DWORD dwSetTagNo, const int nSetX, const int nSetY,
			const UINT wSetW, const UINT wSetH, const UINT nMaxTextLength=500);
   virtual ~CTextBox2DWidget() {}

	virtual void   HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);
	virtual void   SetCursorByPos(int xPos, int yPos);

protected:
	virtual void	DrawText(const int nOffsetX, const int nOffsetY);
	virtual void	DrawCursor(const int nOffsetX, const int nOffsetY);

   bool				MoveCursorUp(UINT wNumLines = 1);
	bool				MoveCursorDown(UINT wNumLines = 1);
   bool				MoveCursorToLineStart();
	bool				MoveCursorToLineEnd();
	bool				MoveCursorUpPage();
   bool				MoveCursorDownPage();

	virtual void	SetCursorIndex(const UINT wIndex);
private:
   void           GetCursorPosition(const UINT wCursorIndex,
         UINT &wCursorX, UINT &wCursorY) const;
   UINT           MoveViewDown(const UINT wNumLines = 1);
   UINT           MoveViewUp(const UINT wNumLines = 1);

   UINT           wCursorX, wCursorY;   //cursor position
};

#endif //#ifndef TEXTBOX2DWIDGET_H

// $Log: TextBox2DWidget.h,v $
// Revision 1.4  2004/01/02 01:10:56  mrimer
// Increased default max text length.
//
// Revision 1.3  2003/08/15 17:53:49  schik
// Fixed a couple of bugs.
// Added support for cut/copy/paste, but it's currently commented out, and should stay so until after 1.6 is out - needs testing.
//
// Revision 1.2  2003/08/14 05:10:55  schik
// HOME and END now work as the user would expect.
//
// Revision 1.1  2003/07/19 02:10:03  mrimer
// Initial check-in.
//
