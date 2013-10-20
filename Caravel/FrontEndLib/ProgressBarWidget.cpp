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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#include "ProgressBarWidget.h"
#include "BitmapManager.h"
#include "Inset.h"
#include <BackEndLib/Assert.h>

//
//Public methods.
//

//*********************************************************************************************************
CProgressBarWidget::CProgressBarWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	BYTE bytSetValue)						//(in)	0 = left, 255 = right.
	: CWidget(WT_ProgressBar, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, bytValue(bytSetValue), bytLastDrawnValue(bytSetValue - 1)
{ }

//*********************************************************************************************************
void CProgressBarWidget::Paint(
//Paint widget area.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
    //Draw inset covering entire widget area.
    SDL_Rect rectInset;
    GetRect(rectInset);
	DrawInset(rectInset.x, rectInset.y, rectInset.w, rectInset.h, this->pPartsSurface, GetDestSurface());

    const UINT CX_BAR_SPACE = 1;
    const UINT CY_BAR_SPACE = 1;
    const UINT CX_MIN_INSET = (CX_BAR_SPACE + CX_INSET_BORDER) * 2 + 3; //3 = left/right bar border + red center.
    const UINT CY_MIN_INSET = (CY_BAR_SPACE + CY_INSET_BORDER) * 2 + 3; //3 = top/bottom bar border + red center.
    if (rectInset.w >= CX_MIN_INSET && rectInset.h >= CY_MIN_INSET)  //Inset must be big enough to display bar.
    {
        SURFACECOLOR Black;
        SURFACECOLOR Red;
        GetDestSurfaceColor(0, 0, 0, Black);
        GetDestSurfaceColor(255, 0, 0, Red);
        
        //Calc bar's rect.
        SDL_Rect rectBar;
        rectBar.x = rectInset.x + CX_INSET_BORDER + CX_BAR_SPACE;
        rectBar.y = rectInset.y + CY_INSET_BORDER + CY_BAR_SPACE;
        rectBar.h = rectInset.h - (CY_INSET_BORDER + CY_BAR_SPACE) * 2;
        UINT wPercent = bytValue * 100 / 255;
        UINT wAvailableWidth = rectInset.w - (CX_INSET_BORDER + CX_BAR_SPACE) * 2;
        rectBar.w = wAvailableWidth * wPercent / 100;
        if (rectBar.w < 3) rectBar.w = 3;

        //Draw the bar.
        DrawFilledRect(rectBar, Red);
        DrawRect(rectBar, Black);
    }

    this->bytLastDrawnValue = this->bytValue;
	if (bUpdateRect) UpdateRect();
}

//*********************************************************************************************************
void CProgressBarWidget::HandleAnimate(void) 
{ 
    if (this->bytValue != this->bytLastDrawnValue) Paint();
}

//*********************************************************************************************************
void CProgressBarWidget::SetValue(
//Set value of progress bar, which affects its position.
//
//Params:
	const BYTE bytSetValue)	//(in)	New value.
{
	this->bytValue = bytSetValue;
}

// $Log: ProgressBarWidget.cpp,v $
// Revision 1.3  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.2  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.1  2003/05/28 22:58:15  erikh2000
// Initial commit.
//
