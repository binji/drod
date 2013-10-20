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

#include "FrameRateEffect.h"
#include "FontManager.h"

//
//Public methods.
//

//*****************************************************************************
CFrameRateEffect::CFrameRateEffect(CWidget *pSetWidget)
	: CEffect(pSetWidget,EFFECTLIB::EFRAMERATE)
	, x(pOwnerWidget->GetX())
	, y(pOwnerWidget->GetY())
	, dwLastDrawTime(SDL_GetTicks())
//Constructor.
{
	this->rAreaOfEffect.x = this->x;
	this->rAreaOfEffect.y = this->y;
}

//*****************************************************************************
bool CFrameRateEffect::Draw(SDL_Surface* pDestSurface)
//Calc the frame rate and put it on the screen.
//
//Returns:
//True.
{
   if (!pDestSurface)
	   pDestSurface = GetDestSurface();

   //Calc the frame rate based on the amount of time between this draw and 
	//last.
	const DWORD dwNow = SDL_GetTicks();
	UINT wFrameRate;
	if (dwNow == this->dwLastDrawTime) //Unlikely, but gotta handle it.
		wFrameRate = 1000;
	else
		wFrameRate = 1000 / (dwNow - this->dwLastDrawTime);
	this->dwLastDrawTime = dwNow;

	//Smooth out the frame rate display a little.
	UINT wDisplayFrameRate;
	if (wFrameRate == this->wLastFrameRate)
		wDisplayFrameRate = wFrameRate;
	else
		wDisplayFrameRate = this->wLastDisplayFrameRate;
	this->wLastFrameRate = wFrameRate;
	this->wLastDisplayFrameRate = wDisplayFrameRate;

	//Display frame rate in top-left corner of widget.
	WCHAR wczFrameRate[20];
	_itoW(wDisplayFrameRate, wczFrameRate, 10);
	g_pTheFM->DrawTextXY(FONTLIB::F_FrameRate, wczFrameRate, pDestSurface,
         this->x, this->y);

	//Get area of effect.
	UINT cxDraw, cyDraw;
	g_pTheFM->GetTextWidthHeight(FONTLIB::F_FrameRate, wczFrameRate, cxDraw, cyDraw);
	this->rAreaOfEffect.w = cxDraw;
	this->rAreaOfEffect.h = cyDraw;

	//Effect will last forever.
	return true;
}

// $Log: FrameRateEffect.cpp,v $
// Revision 1.5  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.4  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.3  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/06/09 19:22:16  mrimer
// Added optional max width/height params to FM::DrawTextXY().
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.4  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.3  2002/10/10 21:15:20  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.2  2002/09/14 21:22:41  mrimer
// Added effect type tag to fix bug in CRoomWidget: hiding frame rate cancels other last-layer effects.
//
// Revision 1.1  2002/08/30 23:59:15  erikh2000
// Initial check-in.
//
