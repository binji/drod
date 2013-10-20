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

#include "StrikeOrbEffect.h"
#include "RoomWidget.h"
#include "TileImageConstants.h"
#include "DrodBitmapManager.h"
#include <FrontEndLib/Bolt.h>
#include "../DRODLib/GameConstants.h"
#include <BackEndLib/Assert.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//********************************************************************************
CStrikeOrbEffect::CStrikeOrbEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,			//(in)	Should be a room widget.
	const COrbData &SetOrbData,		//(in)	Orb being struck.
	SDL_Surface *pSetPartsSurface,	//(in)	Preloaded surface containing bolt parts.
   bool bSetDrawOrb)                //(in)   True if the orb should be drawn
	: CEffect(pSetWidget, EORBHIT)
{
	ASSERT(pSetWidget->GetType() == WT_Room);
	ASSERT(pSetPartsSurface);

	this->pPartsSurface = pSetPartsSurface;
   this->bDrawOrb = bSetDrawOrb;

	SDL_Rect OwnerRect;
	this->pOwnerWidget->GetRect(OwnerRect);
	this->wOrbX = OwnerRect.x + (SetOrbData.wX * CBitmapManager::CX_TILE);
	this->wOrbY = OwnerRect.y + (SetOrbData.wY * CBitmapManager::CY_TILE);

	//Create bolt array.
	this->wBoltCount = SetOrbData.wAgentCount;
	if (!this->wBoltCount)
		this->parrBolts = NULL;
	else
	{
		static const UINT CX_TILE_HALF = CBitmapManager::CX_TILE / 2;
		static const UINT CY_TILE_HALF = CBitmapManager::CY_TILE / 2;
		const int xOrbCenter = this->wOrbX + CX_TILE_HALF;
		const int yOrbCenter = this->wOrbY + CY_TILE_HALF;
		this->parrBolts = new BOLT[this->wBoltCount];

		//Calc start and end coords for each bolt.
		for (UINT wBoltI = 0; wBoltI < this->wBoltCount; ++wBoltI)
		{
			COrbAgentData &Agent = SetOrbData.parrAgents[wBoltI];

			this->parrBolts[wBoltI].xBegin = xOrbCenter;
			this->parrBolts[wBoltI].yBegin = yOrbCenter;
			this->parrBolts[wBoltI].xEnd = OwnerRect.x + 
					Agent.wX * CBitmapManager::CX_TILE + CX_TILE_HALF;
			this->parrBolts[wBoltI].yEnd = OwnerRect.y + 
					Agent.wY * CBitmapManager::CY_TILE + CY_TILE_HALF;
		}
	}
}

//********************************************************************************
CStrikeOrbEffect::~CStrikeOrbEffect(void)
//Destructor.
{
	delete [] this->parrBolts;
}

//********************************************************************************
bool CStrikeOrbEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	static const UINT wDuration = 7;
	static const int CX_TILE = CBitmapManager::CX_TILE;
	static const int CY_TILE = CBitmapManager::CY_TILE;
	const UINT wFrameNo = GetFrameNo();
	if (wFrameNo >= wDuration)
		return false;	//Effect is done.

   if (!pDestSurface)
      pDestSurface = GetDestSurface();
   
	//Draw activated orb tile.
   if (bDrawOrb)
   	g_pTheBM->BlitTileImage(TI_ORB_L, this->wOrbX, this->wOrbY, pDestSurface);

	//Clip screen surface to widget because bolt segments will go all over the place.
	SDL_Rect OwnerRect;
	this->pOwnerWidget->GetRect(OwnerRect);
	SDL_SetClipRect(pDestSurface, &OwnerRect);

	//Set transparency level: bolt fades out over life of effect.
	const Uint8 nOpacity = (Uint8)(255.0 * (wDuration - wFrameNo) / (float)wDuration);
	SDL_SetAlpha(this->pPartsSurface, SDL_SRCALPHA, nOpacity);

	//Reset bounding box.
	this->rAreaOfEffect.x = OwnerRect.x + OwnerRect.w;
	this->rAreaOfEffect.y = OwnerRect.y + OwnerRect.h;
	int xMax=0, yMax=0;

	//Draw energy bolts.
	for (UINT wBoltI = 0; wBoltI < wBoltCount; ++wBoltI)
	{
		const int x1 = this->parrBolts[wBoltI].xBegin;
		const int y1 = this->parrBolts[wBoltI].yBegin;
		const int x2 = this->parrBolts[wBoltI].xEnd;
		const int y2 = this->parrBolts[wBoltI].yEnd;

      DrawBolt(x1, y1, x2, y2, CDrodBitmapManager::DISPLAY_COLS,
            this->pPartsSurface, pDestSurface);

		//Update bounding box of area of effect.
		int xMin = min(x1,x2) - CX_TILE;	//provide some buffer for random bolt
		int xSize = max(x1,x2) + CX_TILE;
		int yMin = min(y1,y2) - CY_TILE;
		int ySize = max(y1,y2) + CY_TILE;
		//Make buffer of sufficient size for all cases.
		while (xSize - xMin < 6 * CX_TILE)
		{
			xMin -= CX_TILE;
			xSize += CX_TILE;
		}
		while (ySize - yMin < 6 * CY_TILE)
		{
			yMin -= CY_TILE;
			ySize += CY_TILE;
		}
		if (xMin < this->rAreaOfEffect.x)
			this->rAreaOfEffect.x = xMin;
		if (yMin < this->rAreaOfEffect.y)
			this->rAreaOfEffect.y = yMin;
		if (xSize > xMax)
			xMax = xSize;
		if (ySize > yMax)
			yMax = ySize;
	}

	//Remove transparency level.
	SDL_SetAlpha(this->pPartsSurface, 0, 0);

	if (this->rAreaOfEffect.x < OwnerRect.x)
		this->rAreaOfEffect.x = OwnerRect.x;
	if (this->rAreaOfEffect.y < OwnerRect.y)
		this->rAreaOfEffect.y = OwnerRect.y;
	this->rAreaOfEffect.w = xMax - this->rAreaOfEffect.x;
	this->rAreaOfEffect.h = yMax - this->rAreaOfEffect.y;

	//Unclip screen surface.
	SDL_SetClipRect(pDestSurface, NULL);

	//Continue effect.
	return true;
}

// $Log: StrikeOrbEffect.cpp,v $
// Revision 1.18  2003/07/01 20:30:32  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.17  2003/06/21 04:10:59  schik
// Orbs affecting a door are now colored according to the toggle/open/close action
//
// Revision 1.16  2003/06/16 21:53:04  mrimer
// Added effect type.
//
// Revision 1.15  2003/05/29 20:47:08  mrimer
// Cleaned up some code.
//
// Revision 1.14  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.13  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.12  2003/05/08 21:00:20  mrimer
// Bolts now fade out over life of effect.
//
// Revision 1.11  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.10  2002/10/11 15:30:43  mrimer
// Fixed display bugs.
//
// Revision 1.9  2002/10/10 21:15:20  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.8  2002/06/21 05:24:26  mrimer
// Revised includes.
//
// Revision 1.7  2002/06/11 22:49:59  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.6  2002/04/22 09:44:11  erikh2000
// Moved bolt-related code to Bolt.h/cpp.
//
// Revision 1.5  2002/04/22 09:41:17  erikh2000
// Initial check-in.
//
// Revision 1.4  2002/04/22 02:55:08  erikh2000
// Strike-orb effect now draws energy bolts from orb to destinations.
//
// Revision 1.3  2002/04/19 21:43:49  erikh2000
// Removed references to ScreenConstants.h.
//
// Revision 1.2  2002/04/13 19:37:54  erikh2000
// Added ASSERT to check that owner widget is a CRoomWidget.  Effect is not appropriate for display in other widgets.
//
// Revision 1.1  2002/04/12 05:15:10  erikh2000
// Initial check-in.
//
