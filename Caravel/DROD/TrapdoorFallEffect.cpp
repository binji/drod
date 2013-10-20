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

#include "TrapdoorFallEffect.h"
#include "TileImageConstants.h"
#include "RoomWidget.h"

//********************************************************************************
CTrapdoorFallEffect::CTrapdoorFallEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,		//(in)	Should be a room widget.
	const CCoord &SetCoord)		//(in)	Location of falling trapdoor.
	: CEffect(pSetWidget)
	, wCol(SetCoord.wCol)
	, wRow(SetCoord.wRow)
{
	//It will save some time to do this just at construction.
	this->pRoom = DYN_CAST(CRoomWidget*, CWidget*, this->pOwnerWidget)->GetCurrentGame()->pRoom;

	//Calc coords to three trapdoor animation squares.
	SDL_Rect OwnerRect;
	this->pOwnerWidget->GetRect(OwnerRect);
	this->xTrapdoor = OwnerRect.x + (SetCoord.wCol * CBitmapManager::CX_TILE);
	this->yTrapdoor = OwnerRect.y + (SetCoord.wRow * CBitmapManager::CY_TILE);
	this->yTrapdoorB = this->yTrapdoor + CBitmapManager::CY_TILE;
	this->yTrapdoorC = this->yTrapdoorB + CBitmapManager::CY_TILE;

	//Specify area of effect.
	this->rAreaOfEffect.x = this->xTrapdoor;
	this->rAreaOfEffect.y = this->yTrapdoor;
	this->rAreaOfEffect.w = this->rAreaOfEffect.h = 0;
}

//********************************************************************************
bool CTrapdoorFallEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	const UINT wFrameNo = GetFrameNo();
	UINT wTI, wTIB, wTIC;

	this->rAreaOfEffect.w = CBitmapManager::CX_TILE;
	if (wFrameNo < 4)
	{
		wTI = TI_TDOORFALL_1;
		wTIB = TI_TDOORFALL_1B;
		wTIC = TI_TDOORFALL_1C;
	}
	else if (wFrameNo < 8)
	{
		wTI = TI_TDOORFALL_2;
		wTIB = TI_TDOORFALL_2B;
		wTIC = TI_TDOORFALL_2C;
	}
	else if (wFrameNo < 12)
	{
		wTI = TI_TDOORFALL_3;
		wTIB = TI_TDOORFALL_3B;
		wTIC = TI_TDOORFALL_3C;
	}
	else if (wFrameNo < 16)
	{
		wTI = TI_TDOORFALL_4;
		wTIB = TI_TDOORFALL_4B;
		wTIC = TI_TDOORFALL_4C;
	}
	else
	{
		//Effect is done.
		this->rAreaOfEffect.h = CBitmapManager::CY_TILE * 3;	//max possible
		return false;
	}

   if (!pDestSurface)
      pDestSurface = GetDestSurface();

	//Draw top tile which is always visible.
	g_pTheBM->BlitTileImage(wTI, xTrapdoor, yTrapdoor, pDestSurface);
	
	//Check square beneath to see if next tile should be drawn.
	if (this->wRow == this->pRoom->wRoomRows - 1 || 
			this->pRoom->GetOSquare(this->wCol, this->wRow + 1)!=T_PIT)
	{
		this->rAreaOfEffect.h = CBitmapManager::CY_TILE;
		return true;
	}
	g_pTheBM->BlitTileImage(wTIB, xTrapdoor, yTrapdoorB, pDestSurface);

	//Check two squares beneath to see if next tile should be drawn.
	if (this->wRow == this->pRoom->wRoomRows - 2 ||
			this->pRoom->GetOSquare(this->wCol, this->wRow + 2)!=T_PIT)
	{
		this->rAreaOfEffect.h = CBitmapManager::CY_TILE * 2;
		return true;
	}
	g_pTheBM->BlitTileImage(wTIC, xTrapdoor, yTrapdoorC, pDestSurface);
	this->rAreaOfEffect.h = CBitmapManager::CY_TILE * 3;

	//Continue effect.
	return true;
}

// $Log: TrapdoorFallEffect.cpp,v $
// Revision 1.14  2003/07/22 19:00:27  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.13  2003/07/01 20:30:32  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.12  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.11  2002/10/10 21:15:20  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.10  2002/08/28 20:30:54  mrimer
// Fixed cut-and-paste bug.
//
// Revision 1.9  2002/08/25 19:06:00  erikh2000
// Trapdoors are now a three square animation.
//
// Revision 1.8  2002/07/26 18:28:25  mrimer
// Refactored tile blitting code into base class.
//
// Revision 1.7  2002/07/25 18:56:55  mrimer
// Refactored stuff into new CAnimatedTileEffect class.
//
// Revision 1.6  2002/06/21 05:25:32  mrimer
// Revised includes.
//
// Revision 1.5  2002/06/11 22:51:44  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.4  2002/04/29 00:23:22  erikh2000
// Revised #includes.
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
