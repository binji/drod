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
 *
 * ***** END LICENSE BLOCK ***** */

#include "LabelWidget.h"

#include <BackEndLib/Wchar.h>

//
//Public methods.
//

//*****************************************************************************
CLabelWidget::CLabelWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	const UINT eSetFontType,				//(in)	Font to use for text.
	const WCHAR *pwczSetText,				//(in)	Text that label will display.
	bool bResizeToFit)						//(in)	If true, widget height will 
											//		change to match height of 
											//		rendered text.  Default is
											//		false.  See comments in
											//		SetText().
	: CWidget(WT_Label, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, eFontType(eSetFontType)
	, eTextAlign(TA_Left)
{
	SetText(pwczSetText, bResizeToFit);
}

//*****************************************************************************
void CLabelWidget::GetTextWidthHeight(
//Gets width and height of text as it is drawn within label.
//
//Params:
	UINT &wW, UINT &wH)	//(out) Width and height of text.
const
{
	if (this->wstrText.size()==0) 
	{
		wW = wH = 0;
		return;
	}

	//Ask font manager about it.
	g_pTheFM->GetTextRectHeight(this->eFontType, this->wstrText.c_str(), 
			this->w, wW, wH);
}

//*****************************************************************************
void CLabelWidget::SetText(
//Sets text of widget.
//
//Params:
	const WCHAR *pwczSetText,	//(in)	New text.
	bool bResizeToFit)			//(in)	If true, then height will be increased
								//		or reduced to fit the text exactly.  
								//		Default is false.  If your label text
								//		is constant, it is better for performance
								//		to not resize to fit.
{
	this->wstrText = (pwczSetText != NULL) ? pwczSetText : wszEmpty;

	if (bResizeToFit)
	{
		//Get text height from the font manager.
		UINT wTextW, wTextH;
		g_pTheFM->GetTextRectHeight(this->eFontType, this->wstrText.c_str(), 
			this->w, wTextW, wTextH);

		//Resize height to fit text.
		Resize(this->w, wTextH);
	}
}

//*****************************************************************************
void CLabelWidget::Paint(
//Paint text inside of the widget area.
//
//Params:
	bool bUpdateRect)			//(in)	If true (default) and destination
								//		surface is the screen, the screen
								//		will be immediately updated in
								//		the widget's rect.
{
	if (this->wstrText.size()==0) return;

	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	UINT wLineOffsetX = 0;

	//Get drawing X coord for centered text.
	if (this->eTextAlign == TA_CenterGroup)
	{
		UINT wLongestLineW, wH;
		g_pTheFM->GetTextRectHeight(this->eFontType, this->wstrText.c_str(), 
				this->w, wLongestLineW, wH);
		wLineOffsetX = (this->w - wLongestLineW) / 2;
	}
	
	g_pTheFM->DrawTextToRect(this->eFontType, this->wstrText.c_str(), 
			this->x + wLineOffsetX + nOffsetX, this->y + nOffsetY,
			this->w, this->h, GetDestSurface());

	if (bUpdateRect) UpdateRect();
}

// $Log: LabelWidget.cpp,v $
// Revision 1.3  2003/07/19 02:13:00  mrimer
// Modified API for a method in CFontManager.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.12  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.11  2002/11/22 02:33:20  mrimer
// Removed some includes.  Changed some begin()'s to c_str()'s.
//
// Revision 1.10  2002/09/03 21:37:34  erikh2000
// Made the label widget scrollable.
// Added a resize-to-fit flag to constructor and SetText() that will make the label vertically fit the text assigned to it.
//
// Revision 1.9  2002/07/09 23:15:45  mrimer
// Revised #includes.
//
// Revision 1.8  2002/06/23 10:53:23  erikh2000
// Text alignment for labels can now be either left-justified or centered.
// Added method to get dimensions of text within label.
//
// Revision 1.7  2002/06/21 01:27:29  erikh2000
// Changed internal storage of label text from WCHAR array to wstring.
//
// Revision 1.6  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.5  2002/06/11 22:55:52  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.4  2002/06/05 03:13:26  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.3  2002/04/16 10:44:30  erikh2000
// Label widget now paints text using the font manager.
//
// Revision 1.2  2002/04/13 19:44:24  erikh2000
// Added new type parameter to CWidget construction call.
//
// Revision 1.1  2002/04/09 10:48:32  erikh2000
// Initial check-in.
//
