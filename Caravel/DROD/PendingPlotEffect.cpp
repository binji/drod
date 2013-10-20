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

#include "PendingPlotEffect.h"
#include "EditRoomWidget.h"
#include <BackEndLib/Assert.h>

const unsigned char MIN_OPACITY = 64;
const unsigned char MAX_OPACITY = 192;

unsigned char CPendingPlotEffect::nOpacity = MIN_OPACITY;
bool CPendingPlotEffect::bRising = true;

const SURFACECOLOR Red = {255, 0, 0};

//*****************************************************************************
CPendingPlotEffect::CPendingPlotEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,				//(in)	Should be a room widget.
	const UINT wObjectNo,			//(in)	Object to place.
	const UINT* pwTileImageNo,		//(in)	Tile(s) to display.
	const UINT wXSize, const UINT wYSize)	//(in) Dimensions of object being displayed
											//(default: 1x1)
	: CEffect(pSetWidget, EPENDINGPLOT)
	, pwTileImageNo(pwTileImageNo)
	, wObjectNo(wObjectNo)
	, wXSize(wXSize), wYSize(wYSize)
{
	ASSERT(pSetWidget->GetType() == WT_Room);
	for (UINT wIndex=wXSize*wYSize; wIndex--; )
		ASSERT(pwTileImageNo[wIndex] < TI_COUNT);	//valid tiles

	//Calculate area of effect.
	CEditRoomWidget *pRoomWidget = DYN_CAST(CEditRoomWidget *, CWidget *, pSetWidget);
   ASSERT(pRoomWidget);
	this->rAreaOfEffect.x = pSetWidget->GetX() + pRoomWidget->wMidX * CBitmapManager::CX_TILE;
	this->rAreaOfEffect.y = pSetWidget->GetY() + pRoomWidget->wMidY * CBitmapManager::CY_TILE;
	this->rAreaOfEffect.w = (pRoomWidget->wEndX - pRoomWidget->wMidX + 1) * CBitmapManager::CX_TILE;
	this->rAreaOfEffect.h = (pRoomWidget->wEndY - pRoomWidget->wMidY + 1) * CBitmapManager::CY_TILE;
}

//*****************************************************************************
bool CPendingPlotEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
   if (!pDestSurface)
	   pDestSurface = GetDestSurface();

   //Change level of transparency.
	if (this->bRising)
	{
		this->nOpacity += 3;
		if (this->nOpacity > MAX_OPACITY)
			this->bRising = false;
	} else {
		this->nOpacity -= 3;
		if (this->nOpacity < MIN_OPACITY)
			this->bRising = true;
	}

	//Draw transparent tiles in region specified in the room widget.
	CEditRoomWidget *pRoomWidget = DYN_CAST(CEditRoomWidget *, CWidget *, this->pOwnerWidget);
   ASSERT(pRoomWidget);
	const UINT wStartX = pRoomWidget->wMidX;
	const UINT wStartY = pRoomWidget->wMidY;
	UINT wEndX = pRoomWidget->wEndX;
	UINT wEndY = pRoomWidget->wEndY;
	if (wEndX - pRoomWidget->wMidX + 1 < this->wXSize)	//show at least one full object
	{
		wEndX = pRoomWidget->wMidX + this->wXSize - 1;
		this->rAreaOfEffect.w = (wEndX - pRoomWidget->wMidX + 1) * CBitmapManager::CX_TILE;
	}
	if (wEndY - pRoomWidget->wMidY + 1< this->wYSize)
	{
		wEndY = pRoomWidget->wMidY + this->wYSize - 1;
		this->rAreaOfEffect.h = (wEndY - pRoomWidget->wMidY + 1) * CBitmapManager::CY_TILE;
	}

	//Stairs have a special appearance.
	if (this->wObjectNo == T_STAIRS)
		PlotStaircase(wStartX,wStartY,wEndX,wEndY, pDestSurface);
	else
	{
		SDL_Rect OwnerRect;
		this->pOwnerWidget->GetRect(OwnerRect);

		UINT wX, wY, wTileNo;
		for (wY=wStartY; wY<=wEndY; wY++)
			for (wX=wStartX; wX<=wEndX; wX++)
			{
				if (pRoomWidget->IsSafePlacement(this->wObjectNo,wX,wY))
				{
					wTileNo = this->pwTileImageNo[
							((wY - wStartY) % this->wYSize) * this->wXSize +
							(wX - wStartX) % this->wXSize];
					g_pTheBM->BlitTileImage(wTileNo, OwnerRect.x + wX * CBitmapManager::CX_TILE,
							OwnerRect.y + wY * CBitmapManager::CY_TILE, pDestSurface, this->nOpacity);
				} else {
					if (wX * CBitmapManager::CX_TILE < OwnerRect.w && wY *
							CBitmapManager::CY_TILE < OwnerRect.h)
						g_pTheBM->ShadeTile(OwnerRect.x + wX * CBitmapManager::CX_TILE,
								OwnerRect.y + wY * CBitmapManager::CY_TILE, Red, pDestSurface);
				}
			}
	}

	//Continue effect.
	return true;
}

//*****************************************************************************
void CPendingPlotEffect::PlotStaircase(
//Staircase is drawn with a wall around the left, bottom and right sides.
//(And nothing on top.)
//
//Params:
	const UINT wStartX, const UINT wStartY, const UINT wEndX, const UINT wEndY,
   SDL_Surface* pDestSurface) //(in)
{
	CEditRoomWidget *pRoomWidget = DYN_CAST(CEditRoomWidget *, CWidget *, this->pOwnerWidget);
   ASSERT(pRoomWidget);
   ASSERT(pDestSurface);
	SDL_Rect OwnerRect;
	this->pOwnerWidget->GetRect(OwnerRect);

	UINT wX, wY, wObjectNo, wTileNo;
	for (wY=wStartY; wY<=wEndY; wY++)
		for (wX=wStartX; wX<=wEndX; wX++)
		{
			//Determine what part of staircase is being plotted at this square.
			if (wX == wStartX || wX == wEndX || wY == wEndY)
				wObjectNo = T_WALL;
			else if (wY == wStartY)
				continue;	//nothing will be plotted here
			else
				wObjectNo = T_STAIRS;

			if (pRoomWidget->IsSafePlacement(wObjectNo,wX,wY))
			{
				wTileNo = (wObjectNo == T_WALL ? TI_WALL : TI_STAIRS_1);
				g_pTheBM->BlitTileImage(wTileNo, OwnerRect.x + wX * CBitmapManager::CX_TILE,
						OwnerRect.y + wY * CBitmapManager::CY_TILE, pDestSurface, this->nOpacity);
			} else {
				if (wX * CBitmapManager::CX_TILE < OwnerRect.w && wY *
						CBitmapManager::CY_TILE < OwnerRect.h)
					g_pTheBM->ShadeTile(OwnerRect.x + wX * CBitmapManager::CX_TILE,
							OwnerRect.y + wY * CBitmapManager::CY_TILE, Red, pDestSurface);
			}
		}
}

// $Log: PendingPlotEffect.cpp,v $
// Revision 1.8  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.7  2003/07/01 20:30:31  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.6  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.4  2003/05/23 21:43:04  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.3  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.2  2002/11/22 02:16:16  mrimer
// Added PlotStaircase() to perform special handling for staircases.  Made static method vars into class vars.
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
