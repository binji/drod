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

#include "FrameWidget.h"
#include "BitmapManager.h"
#include "FontManager.h"
#include <BackEndLib/Assert.h>

//
//Public methods.
//

//******************************************************************************
CFrameWidget::CFrameWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	const WCHAR *pwczSetCaption)			//(in)	Caption text for frame. If 
											//		NULL, frame will not be 
											//		drawn with a caption.
	: CWidget(WT_Frame, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
{
	this->pwczCaption = NULL;
	if (pwczSetCaption) SetCaption(pwczSetCaption);
}

//******************************************************************************
CFrameWidget::~CFrameWidget()
//Destructor.
//
{
	delete[] this->pwczCaption;
}

//******************************************************************************
void CFrameWidget::SetCaption(
//Sets caption that appears at top of frame.
//
//Params:
	const WCHAR *pwczSetCaption) //(in) Text for caption.  NULL to remove caption.
{
	delete [] this->pwczCaption;
	this->pwczCaption = NULL;

	if (pwczSetCaption)
	{
		const UINT dwSetCaptionLen = WCSlen(pwczSetCaption);
		if (dwSetCaptionLen)
		{
			this->pwczCaption = new WCHAR[dwSetCaptionLen + 1];
			WCScpy(this->pwczCaption, pwczSetCaption);
		}
	}
}

//******************************************************************************
void CFrameWidget::Paint(
//Paint widget area.
//
//Params:
	bool bUpdateRect)				//(in)	If true (default) and destination
									//		surface is the screen, the screen
									//		will be immediately updated in
									//		the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

	ASSERT(this->w >= 4);
	ASSERT(this->h >= 4);

	//Calc surface colors if needed.
	SDL_Surface *pDestSurface = GetDestSurface();
	const SURFACECOLOR Light = GetSurfaceColor(pDestSurface, 225, 214, 192);
	const SURFACECOLOR Dark = GetSurfaceColor(pDestSurface, 145, 132, 109);

	//Here's a picture to see what it looks like:
	//
	//		   CCCCCCC
	//		###CCCCCCC####### 	# = dark
	//		#..CCCCCCC..... .	. = light
	//		#. CCCCCCC     #.	C = caption
	//		#.             #.
	//		#.             #.
	//		#.             #.
	//		#.             #.
	//		#.             #.
	//		#.             #.
	//		# ##############.
	//		 ................

	const UINT CX_INDENT = 2, CX_CORNER = 2, CX_CAPTION_SPACE = 2;
	UINT wCaptionW = 0, wCaptionH = 0;
	bool bDrawCaption = (this->pwczCaption != NULL);
	if (bDrawCaption)
	{
		//Get dimensions of caption.
		g_pTheFM->GetTextWidthHeight(FONTLIB::F_FrameCaption, this->pwczCaption,
				wCaptionW, wCaptionH);

		//If too large to fit, don't draw it.
		if (this->w < (CX_CORNER + CX_INDENT + CX_CAPTION_SPACE +
				CX_CAPTION_SPACE + CX_CORNER))
			bDrawCaption = false;
	}

	//Get direct access to pixels.
	LockDestSurface();

	if (bDrawCaption)
	{
		//Draw caption text.
		int xCaption = this->x + CX_CORNER + CX_INDENT + CX_CAPTION_SPACE;
		int yCaption = this->y - (wCaptionH / 2);
		g_pTheFM->DrawTextXY(FONTLIB::F_FrameCaption, this->pwczCaption,
            GetDestSurface(), xCaption, yCaption);

		//Draw dark outer top row left of the caption.
		DrawRow(this->x, this->y, CX_CORNER + CX_INDENT, Dark);

		//Draw dark outer top row right of the caption.
		int xRightOfCaption = xCaption + wCaptionW + CX_CAPTION_SPACE;
		DrawRow(xRightOfCaption, this->y,
			this->w - (xRightOfCaption - this->x) - 1, Dark);

		//Draw light inner top row left of the caption.
		DrawRow(this->x + 1, this->y + 1, CX_CORNER + CX_INDENT + 1, Light);

		//Draw light inner top row right of the caption.
		DrawRow(xRightOfCaption, this->y + 1,
				this->w - (xRightOfCaption - this->x) - 3, Light);
	}
	else	//Draw top rows without caption.
	{
		//Draw dark outer top row.
		DrawRow(this->x, this->y, this->w - 1, Dark);

		//Draw light inner top row.
		DrawRow(this->x + 1, this->y + 1, this->w - 3, Light);
	}

	//Draw dark left column.
	DrawCol(this->x, this->y + 1, this->h - 2, Dark);

	//Draw light inner left column.
	DrawCol(this->x + 1, this->y + 2, this->h - 4, Light);

	//Draw light outer bottom row.
	DrawRow(this->x + 1, this->y + this->h - 1, this->w - 1, Light);

	//Draw light outer right column.
	DrawCol(this->x + this->w - 1, this->y + 1, this->h - 2, Light);

	//Draw dark inner bottom row.
	DrawRow(this->x + 2, this->y + this->h - 2, this->w - 3, Dark);

	//Draw dark inner right column.
	DrawCol(this-> x + this->w - 2, this->y + 2, this->h - 4, Dark);

	UnlockDestSurface();

	PaintChildren();
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CFrameWidget::PaintClipped(
   const int /*nX*/, const int /*nY*/, const UINT /*wW*/, const UINT /*wH*/, const bool /*bUpdateRect*/)
{
	//Paint() uses direct access to pixels, so it can't be clipped with
	//SDL_SetClipRect().  Either you need to write PaintClipped() or change
	//the situation which causes this widget to be clipped.
	ASSERTP(false, "Can't paint clipped.");
}

// $Log: FrameWidget.cpp,v $
// Revision 1.7  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.6  2003/09/16 19:03:11  schik
// Fixed memory leak
//
// Revision 1.5  2003/07/09 11:56:50  schik
// Made PaintClipped() arguments consistent among derived classes (VS.NET warnings)
//
// Revision 1.4  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/06/09 19:22:16  mrimer
// Added optional max width/height params to FM::DrawTextXY().
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.12  2003/05/04 00:04:02  mrimer
// Fixed some var types.
//
// Revision 1.11  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.10  2002/11/15 03:14:06  mrimer
// Removed unneeded includes.  Removed superfluous code.
//
// Revision 1.9  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.8  2002/06/21 05:08:21  mrimer
// Renamed DrawText to DrawTextXY.
//
// Revision 1.7  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.6  2002/06/14 00:49:47  erikh2000
// Changed call to obsolete GetScreenSurfaceColor() method so that it gets color from dest surface instead.
// Changed static surfacecolor declarations to const to prevent errors.
//
// Revision 1.5  2002/06/11 22:55:52  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.4  2002/06/07 23:01:30  mrimer
// Comment tweaking.
//
// Revision 1.3  2002/04/19 21:52:47  erikh2000
// Frames can now display a text caption in their topleft corner.
//
// Revision 1.2  2002/04/14 00:30:00  erikh2000
// Wrote Paint().
//
// Revision 1.1  2002/04/13 19:28:51  erikh2000
// Initial check-in.
//
