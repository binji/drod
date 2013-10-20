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

#include "BumpObstacleEffect.h"
#include "BitmapManager.h"
#include <BackEndLib/Assert.h>

#include <SDL.h>

//Get relative horizontal/vertical position from orientation#.
#define nGetOX(o)                                 ( ((o) % 3) - 1 )
#define nGetOY(o)                                 ( ((o) / 3) - 1 )

//********************************************************************************
CBumpObstacleEffect::CBumpObstacleEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,		//(in)	Should be a room widget.
	UINT wSetCol, UINT wSetRow, //(in)	Square containing obstacle.
	UINT wSetBumpO)				//(in)	Direction to bump square.
	: CEffect(pSetWidget,EFFECTLIB::EBUMPOBSTACLE)
{
	ASSERT(pSetWidget->GetType() == WT_Room);

	//Get width and height of bump blit area.
	const int dx = nGetOX(wSetBumpO);
	const int dy = nGetOY(wSetBumpO);
	this->src.w = this->dest.w = CBitmapManager::CX_TILE;
	this->src.h = this->dest.h = CBitmapManager::CY_TILE;
	
	//Set source and dest rects for bump blit.
	SDL_Rect rectWidget;
	this->pOwnerWidget->GetRect(rectWidget);
	this->src.x = this->dest.x = rectWidget.x + (wSetCol * CBitmapManager::CX_TILE);
	this->src.y = this->dest.y = rectWidget.y + (wSetRow * CBitmapManager::CY_TILE);
	
	this->dest.x += dx;
	this->dest.y += dy;

	//Area covered by effect.
	this->rAreaOfEffect = this->dest;
}

//********************************************************************************
bool CBumpObstacleEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	if (GetFrameNo() >= 6)
		//Effect is done.
		return false;

	//Blit the bumped area.
	//Make sure it doesn't go out of bounds.
   if (!pDestSurface)
	   pDestSurface = GetDestSurface();
	SDL_Rect ClipRect;
	this->pOwnerWidget->GetRect(ClipRect);
	SDL_SetClipRect(pDestSurface, &ClipRect);
	SDL_BlitSurface(pDestSurface, &this->src, pDestSurface, &this->dest);
	SDL_SetClipRect(pDestSurface, NULL);
	
	//Effect continues.
	return true;
}

// $Log: BumpObstacleEffect.cpp,v $
// Revision 1.5  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.4  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.3  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.2  2003/05/24 03:03:40  mrimer
// Modified some remaining DROD-specific code.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.10  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.9  2002/10/17 16:46:09  mrimer
// Fixed bug: painting past room edge.
//
// Revision 1.8  2002/10/10 21:15:20  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.7  2002/09/14 21:40:25  mrimer
// Added EBUMPOBSTACLE tag.
//
// Revision 1.6  2002/07/09 22:21:57  mrimer
// Revised #includes.
//
// Revision 1.5  2002/06/14 00:43:25  erikh2000
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
//
// Revision 1.4  2002/06/11 22:57:45  mrimer
// Changed "screens" to "destination surfaces".
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
