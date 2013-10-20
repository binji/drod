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

#include "SwordsmanSwirlEffect.h"
#include "DrodBitmapManager.h"
#include "TileImageConstants.h"
#include "../DRODLib/CurrentGame.h"
#include "../DRODLib/GameConstants.h"
#include <BackEndLib/Assert.h>

#include <math.h>

#define NUM_SWIRL_OBJECTS	(10)				//Sprites in animation
#define NUM_REVOLUTIONS		(3)				//~3 seconds duration
#define FRAMES_PER_REVOLUTION	(30)	//Full rotation every x frames
#define MS_PER_REVOLUTION (FRAMES_PER_REVOLUTION*1000/30)
#define SPRITE_SIZE	(6)	//largest image, in pixels

const double pi = 3.1415926536;

const UINT wSwirlImageNo[NUM_SWIRL_OBJECTS] = {
		TI_SWIRLDOT_1,
		TI_SWIRLDOT_1,
		TI_SWIRLDOT_2,
		TI_SWIRLDOT_2,
		TI_SWIRLDOT_3,
		TI_SWIRLDOT_3,
		TI_SWIRLDOT_4,
		TI_SWIRLDOT_4,
		TI_SWIRLDOT_5,
		TI_SWIRLDOT_5};

//********************************************************************************
CSwordsmanSwirlEffect::CSwordsmanSwirlEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,		//(in)	Should be a room widget.
	CCurrentGame *pCurrentGame)	//(in) to track the swordsman's position
	: CEffect(pSetWidget)
	, pCurrentGame(pCurrentGame)
{
	ASSERT(pSetWidget->GetType() == WT_Room);
}

//********************************************************************************
bool CSwordsmanSwirlEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//Flashes circle around the swordsman at his current position.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	if (GetFrameNo() >= NUM_REVOLUTIONS * FRAMES_PER_REVOLUTION + NUM_SWIRL_OBJECTS)
		return false;

   if (!pDestSurface)
	   pDestSurface = GetDestSurface();

   const Uint32 dwNow = SDL_GetTicks();
	const Uint32 dwTimeElapsed = dwNow - this->dwTimeStarted;
	
	//Get swordsman's position.
	SDL_Rect OwnerRect;
	this->pOwnerWidget->GetRect(OwnerRect);
	const UINT wXCenter = OwnerRect.x + (this->pCurrentGame->swordsman.wX *
			CBitmapManager::CX_TILE) + CBitmapManager::CX_TILE/4;
	const UINT wYCenter = OwnerRect.y + (this->pCurrentGame->swordsman.wY *
			CBitmapManager::CY_TILE) + CBitmapManager::CY_TILE/4;
	const UINT rMaxPosition=NUM_REVOLUTIONS*MS_PER_REVOLUTION;
	UINT xMax=0, yMax=0;	//bottom edge of bounding box

	//Reset area of effect.
	this->rAreaOfEffect.x = OwnerRect.x + OwnerRect.w;
	this->rAreaOfEffect.y = OwnerRect.y + OwnerRect.h;

	//Draw swirl.
	for (UINT nIndex=0; nIndex<NUM_SWIRL_OBJECTS; nIndex++)
	{
		const UINT rPosition = dwTimeElapsed-nIndex*60;
		if (rPosition > rMaxPosition) continue;	//appear gradually
		const double radius = 4.0*(rMaxPosition-rPosition)/(rMaxPosition+1.0);
		const double theta = rPosition/(MS_PER_REVOLUTION+1.0)*2.0*pi;
		const UINT wX = static_cast<UINT>(wXCenter + sin(theta) * CBitmapManager::CX_TILE*radius);
		const UINT wY = static_cast<UINT>(wYCenter + cos(theta) * CBitmapManager::CY_TILE*radius);
		const UINT wSize = SPRITE_SIZE - nIndex/2;	//dimensions of this dot

		if (static_cast<int>(wX) >= OwnerRect.x && static_cast<int>(wY) >= OwnerRect.y &&
				wX < OwnerRect.x + OwnerRect.w - wSize &&
				wY < OwnerRect.y + OwnerRect.h - wSize)
		{
			g_pTheBM->BlitTileImagePart(wSwirlImageNo[nIndex], wX, wY,
					wSize, wSize, pDestSurface);

			//Update bounding box of area of effect.
			if (static_cast<int>(wX) < this->rAreaOfEffect.x)
            this->rAreaOfEffect.x = wX;
			if (static_cast<int>(wY) < this->rAreaOfEffect.y)
				this->rAreaOfEffect.y = wY;
			if (wX + wSize > xMax)	//Technically should also have a -1,
				xMax = wX + wSize;	//but this simpler and consistent with
			if (wY + wSize > yMax)	//the calculation below.
				yMax = wY + wSize;
		}
	}
	this->rAreaOfEffect.w = xMax - this->rAreaOfEffect.x;
	this->rAreaOfEffect.h = yMax - this->rAreaOfEffect.y;

	return true;
}

// $Log: SwordsmanSwirlEffect.cpp,v $
// Revision 1.12  2003/07/01 20:30:32  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.11  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.9  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.8  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.7  2002/12/22 02:28:53  mrimer
// Revised swordsman vars.
//
// Revision 1.6  2002/11/18 17:33:51  mrimer
// Converted animation from frame-based to time-based.
//
// Revision 1.5  2002/10/10 21:15:20  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.4  2002/10/07 20:12:17  mrimer
// Optimized painting by changing BlitTileImage calls to BlitTileImagePart.
//
// Revision 1.3  2002/07/05 21:07:56  uid78714
// Tweaking.
//
// Revision 1.2  2002/07/04 21:12:05  erikh2000
// Used new tiles and tweaked the animation a bit.
//
// Revision 1.1  2002/07/03 22:01:40  mrimer
// Initial check-in.
//
