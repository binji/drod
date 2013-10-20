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

#include "ToolTipEffect.h"
#include "BitmapManager.h"

#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

//Background tool tip color.
const SURFACECOLOR BGColor = {205, 250, 255};	//pale yellow
const SURFACECOLOR Border = {0, 0, 0};	//black

const UINT	wBorder = 2;
//
//Public methods.
//

//*****************************************************************************
CToolTipEffect::CToolTipEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget, 					//(in)	Required params for CEffect 
	const CCoord &SetCoord,					//(in)	Desired location of text.
	const WCHAR *pwczSetText,				//(in)	Text that label will display.
	const Uint32 dwDisplayDuration,		//(in)	Display (default = 5000ms)
	const UINT eSetFontType)
	: CEffect(pSetWidget,EFFECTLIB::ETOOLTIP)
	, eFontType(eSetFontType)
   , pToolTipSurface(NULL)
	, dwWhenEnabled(SDL_GetTicks())
	, dwDuration(dwDisplayDuration)
{
	this->wstrText = pwczSetText ? pwczSetText : wszEmpty;

	SDL_Rect OwnerRect;
	this->pOwnerWidget->GetRect(OwnerRect);

   UINT wW, wH;
	this->w = OwnerRect.w;
	GetTextWidthHeight(wW,wH);
	this->w = wW + wBorder*2 + 2;
	this->h = wH + wBorder*2 + 2;

	this->x = SetCoord.wCol;
	this->y = SetCoord.wRow;
   if (static_cast<Sint16>(this->y) - static_cast<Sint16>(this->h) >= OwnerRect.y)
      this->y -= this->h;  //show tool tip just above mouse cursor
   else
      this->y += this->h;  //no room -- show tool tip just below mouse cursor

	//Move away from edge of parent widget, if necessary.
	if (this->x + this->w >= static_cast<UINT>(OwnerRect.x + OwnerRect.w))
		this->x = OwnerRect.x + OwnerRect.w - this->w;
	if (this->y + this->h >= static_cast<UINT>(OwnerRect.y + OwnerRect.h))
		this->y = OwnerRect.y + OwnerRect.h - this->h;

	//Area of effect.
	this->rAreaOfEffect.x = this->x;
	this->rAreaOfEffect.y = this->y;
	this->rAreaOfEffect.w = this->w;
	this->rAreaOfEffect.h = this->h;

   //Render tool tip to internal surface (to avoid re-rendering each frame).
   this->pToolTipSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			this->w, this->h, 24, 0, 0, 0, 0);
   ASSERT(this->pToolTipSurface);
	SDL_Rect TextRect = {1, 1, this->w-2, this->h-2};
   SDL_Rect BorderRect = {0, 0, this->w, this->h};
	//Fill rectangle and draw border.
	this->pOwnerWidget->DrawFilledRect(BorderRect, BGColor, pToolTipSurface);
	this->pOwnerWidget->DrawRect(BorderRect, Border, pToolTipSurface);
	g_pTheFM->DrawTextToRect(this->eFontType, this->wstrText.c_str(), 
			TextRect.x+wBorder, TextRect.y, TextRect.w-wBorder,
			TextRect.h-wBorder, pToolTipSurface);
}

//*****************************************************************************
CToolTipEffect::~CToolTipEffect()
{
   SDL_FreeSurface(this->pToolTipSurface);
}

//*****************************************************************************
void CToolTipEffect::GetTextWidthHeight(
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
bool CToolTipEffect::Draw(SDL_Surface* pDestSurface)
{
	const Uint32 dwNow = SDL_GetTicks();
	if (dwNow - this->dwWhenEnabled > this->dwDuration)
		return false;

	if (this->wstrText.size()==0) return false;

   if (!pDestSurface)
      pDestSurface = GetDestSurface();

	SDL_Rect ToolTipRect = {0, 0, this->w, this->h};
	SDL_Rect ScreenRect = {this->x, this->y, this->w, this->h};
	SDL_BlitSurface(this->pToolTipSurface, &ToolTipRect, pDestSurface, &ScreenRect);

   return true;
}

// $Log: ToolTipEffect.cpp,v $
// Revision 1.9  2003/09/22 16:57:38  mrimer
// Fixed and improved bounds checking.
//
// Revision 1.8  2003/08/19 20:09:08  mrimer
// Linux maintenance (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/08/16 00:45:01  mrimer
// Optimized tool tip effect.  Fixed a bug.
//
// Revision 1.6  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.5  2003/07/19 02:13:00  mrimer
// Modified API for a method in CFontManager.
//
// Revision 1.4  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.3  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.3  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.2  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.1  2002/11/22 02:26:30  mrimer
// Initial check-in.
//
