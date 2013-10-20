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

#include "RoomEffectList.h"
#include "RoomWidget.h"
#include <BackEndLib/Assert.h>

//*****************************************************************************
void CRoomEffectList::Clear(
//Clears all effects from the effect list.
//
//Params:
	const bool bRepaint)	//(in)	Touch up affected room areas before deleting
								//(default = false)
{
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		ASSERT(*iSeek);
		if (bRepaint)
			DirtyTilesForEffect(*iSeek);
		delete *iSeek;
	}
	this->Effects.clear();
}

//*****************************************************************************
void CRoomEffectList::DirtyTiles() const
//Dirties room tiles within the effects' area of effect.
{
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
			DirtyTilesForEffect(*iSeek);
}

//*****************************************************************************
void CRoomEffectList::DirtyTilesForEffect(
//Dirties room tiles within the effects' area of effect.
//
//Params:
	CEffect *pEffect)
const
{
	UINT xStart, yStart, xEnd, yEnd;
	SDL_Rect rect;

	rect = pEffect->GetAreaOfEffect();
	if (rect.w || rect.h)
	{
		//Non-zero effect area -- dirty the tiles this effect covers.

		//Remove offset to calculate tiles.
		rect.x -= this->pOwnerWidget->GetX();
		rect.y -= this->pOwnerWidget->GetY();
		//Ignore parts out of bounds.
		if (rect.x < 0) {rect.w += rect.x; rect.x = 0;}
		if (rect.y < 0) {rect.h += rect.y; rect.y = 0;}
		if ((rect.x + (Sint16)rect.w < 0) || (rect.y + (Sint16)rect.h < 0) ||
				(rect.x >= (int)this->pOwnerWidget->GetW()) ||
				(rect.y >= (int)this->pOwnerWidget->GetH())) return;
		if (rect.x + rect.w >= static_cast<int>(this->pOwnerWidget->GetW()))
			{rect.w = this->pOwnerWidget->GetW() - rect.x-1;}
		if (rect.y + rect.h >= static_cast<int>(this->pOwnerWidget->GetH()))
			{rect.h = this->pOwnerWidget->GetH() - rect.y-1;}

		//Calculate tiles covered.
		xStart = rect.x / CBitmapManager::CX_TILE;
		yStart = rect.y / CBitmapManager::CY_TILE;
		xEnd = (rect.x+rect.w-1) / CBitmapManager::CX_TILE;
		yEnd = (rect.y+rect.h-1) / CBitmapManager::CY_TILE;
		ASSERT(xEnd>=xStart);
		ASSERT(yEnd>=yStart);
		//Dirty these tiles.  Overlapping regions are fine.
		DirtyTilesInRect(xStart,yStart,xEnd,yEnd);
	}
}

//*****************************************************************************
void CRoomEffectList::DirtyTilesInRect(
//Mark all tiles in bounding box as dirty.
//
//Params:
	const UINT xStart, const UINT yStart, const UINT xEnd, const UINT yEnd)	//(in)
const
{
	const UINT wCols = this->pOwnerWidget->pRoom->wRoomCols;
	UINT xIndex, yIndex;
	TILEINFO *pTInfo;
	for (yIndex=yStart; yIndex<=yEnd; yIndex++)
	{
		pTInfo = this->pOwnerWidget->pTileInfo + (yIndex * wCols + xStart);
		for (xIndex=xStart; xIndex<=xEnd; ++xIndex)
			(pTInfo++)->dirty = 1;
	}
}

//*****************************************************************************
void CRoomEffectList::RemoveEffectsOfType(
//Removes all effects of given type from the list.
//
//Params:
	const UINT eEffectType)	//(in)	Type of effect to remove.
{
	//Clear list of given effect type.
	list<CEffect *>::const_iterator iSeek = this->Effects.begin();
	while (iSeek != this->Effects.end())
	{
		if (eEffectType == (*iSeek)->GetEffectType())
		{
			//Remove from list.
			CEffect *pDelete = *iSeek;
			DirtyTilesForEffect(pDelete);	//touch up area before deleting
			++iSeek;
			this->Effects.remove(pDelete);
			delete pDelete;
		}
		else
			++iSeek;
	}
}

// $Log: RoomEffectList.cpp,v $
// Revision 1.1  2003/08/16 01:19:35  mrimer
// Refactored code from EffectList.
//
// Revision 1.12  2003/07/01 20:34:22  mrimer
// Added optional destination surface parameter to draw routine.
//
// Revision 1.11  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.9  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.8  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.7  2002/12/22 02:41:24  mrimer
// Fixed bug introduced when some effects were changed from frame counting to real-time animation during a pause.
//
// Revision 1.6  2002/11/15 02:45:26  mrimer
// Modified Clear().
//
// Revision 1.5  2002/10/14 17:25:27  mrimer
// Revised DirtyTilesInRect() to use revised data structures in CRoomWidget.
//
// Revision 1.4  2002/10/11 15:30:43  mrimer
// Fixed display bugs.
//
// Revision 1.3  2002/10/10 21:11:39  mrimer
// Added DirtyTiles(), DirtyTilesForEffect(), DirtyTilesInRect() to support optimized room drawing.
//
// Revision 1.2  2002/10/03 19:03:29  mrimer
// Added temporarily freezing events.
//
// Revision 1.1  2002/09/14 21:23:27  mrimer
// Initial check-in.
//
