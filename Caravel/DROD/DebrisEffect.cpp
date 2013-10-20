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

#include "TileImageConstants.h"
#include "DrodBitmapManager.h"
#include "DebrisEffect.h"
#include "../DRODLib/GameConstants.h"
#include <BackEndLib/Assert.h>

//********************************************************************************
CDebrisEffect::CDebrisEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,			//(in)	Should be a room widget.
	const CMoveCoord &MoveCoord)	//(in)	Location of debris and direction of	its movement.
	: CParticleExplosionEffect(pSetWidget, MoveCoord, 8)
{
}

//********************************************************************************
bool CDebrisEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	if (!MoveParticles())
		return false;

   if (!pDestSurface)
	   pDestSurface = GetDestSurface();

   for (int nIndex=wParticleCount; nIndex--; )
		//If particle is still active, plot to display.
		if (parrParticles[nIndex].bActive)
			if (parrParticles[nIndex].type)
				g_pTheBM->BlitTileImagePart(TI_DEBRIS_2, ROUND(parrParticles[nIndex].x),
						ROUND(parrParticles[nIndex].y), 4, 5, pDestSurface);
			else
				g_pTheBM->BlitTileImagePart(TI_DEBRIS_1, ROUND(parrParticles[nIndex].x),
						ROUND(parrParticles[nIndex].y), 7, 8, pDestSurface);

	return true;
}

// $Log: DebrisEffect.cpp,v $
// Revision 1.14  2003/07/09 21:10:47  mrimer
// Now pass in size of particles to base class.
//
// Revision 1.13  2003/07/01 20:30:31  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.12  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.11  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.10  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.9  2002/12/06 19:52:25  mrimer
// Fixed tile graphic, and revised a partial tile dimension.
//
// Revision 1.8  2002/11/18 17:29:54  mrimer
// Added: rounding the now real-valued particle position.
//
// Revision 1.7  2002/10/07 20:12:16  mrimer
// Optimized painting by changing BlitTileImage calls to BlitTileImagePart.
//
// Revision 1.6  2002/06/21 05:00:15  mrimer
// Revised includes.
//
// Revision 1.5  2002/06/11 22:57:45  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.4  2002/04/22 21:46:06  mrimer
// Implemented CDebrisEffect::Draw().
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
