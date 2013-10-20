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

#ifndef CNEATHERSTRIKESORBEFFECT_H
#define CNEATHERSTRIKESORBEFFECT_H

#include <FrontEndLib/AnimatedTileEffect.h>
#include "DrodEffect.h"

//*****************************************************************************
class CNeatherStrikesOrbEffect : public CAnimatedTileEffect
{
public:
	CNeatherStrikesOrbEffect(CWidget *pSetOwnerWidget, const CCoord &SetCoord)
		: CAnimatedTileEffect(pSetOwnerWidget,SetCoord,ENEATHERHITORB)
		, wCol(SetCoord.wCol) , wRow(SetCoord.wRow)
			{this->rAreaOfEffect.h = CBitmapManager::CY_TILE * 2;}
	virtual bool Draw(SDL_Surface* pDestSurface=NULL)
	{
		if (GetFrameNo() > 15) return false;	//duration: 500 ms
      if (!pDestSurface)
         pDestSurface = GetDestSurface();

		DrawTile(TI_NTHR_SS, pDestSurface);
		SDL_Rect OwnerRect;
		this->pOwnerWidget->GetRect(OwnerRect);
		g_pTheBM->BlitTileImage(TI_ORB_N, OwnerRect.x + this->wCol *
				CBitmapManager::CX_TILE, OwnerRect.y + (this->wRow+1) *
				CBitmapManager::CY_TILE, pDestSurface);
		return true;	//Continue effect.
	}
private:
	UINT		wCol, wRow;
};

#endif	//...#ifndef CNEATHERSTRIKESORBEFFECT_H

// $Log: NeatherStrikesOrbEffect.h,v $
// Revision 1.6  2003/08/16 01:52:46  mrimer
// Included DrodEffects.h
//
// Revision 1.5  2003/07/01 20:30:31  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.4  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.3  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.2  2002/10/10 21:12:11  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.1  2002/09/14 21:23:27  mrimer
// Initial check-in.
//
