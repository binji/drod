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
 * Contributor(s): mrimer
 *
 * ***** END LICENSE BLOCK ***** */

#include "TarStabEffect.h"
#include "TileImageConstants.h"
#include "DrodBitmapManager.h"
#include "../DRODLib/GameConstants.h"
#include <BackEndLib/Assert.h>

//********************************************************************************
CTarStabEffect::CTarStabEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,			//(in)	Should be a room widget.
	const CMoveCoord &MoveCoord)	//(in)	Location of debris and direction of	its movement.
	: CParticleExplosionEffect(pSetWidget, MoveCoord, 5)
{
}

//********************************************************************************
bool CTarStabEffect::Draw(SDL_Surface* pDestSurface)
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
				g_pTheBM->BlitTileImagePart(TI_TARBLOOD_2, ROUND(parrParticles[nIndex].x),
						ROUND(parrParticles[nIndex].y), 5, 5, pDestSurface);
			else
				g_pTheBM->BlitTileImagePart(TI_TARBLOOD_1, ROUND(parrParticles[nIndex].x),
						ROUND(parrParticles[nIndex].y), 4, 4, pDestSurface);

	return true;
}

// $Log: TarStabEffect.cpp,v $
// Revision 1.11  2003/07/09 21:10:47  mrimer
// Now pass in size of particles to base class.
//
// Revision 1.10  2003/07/01 20:30:32  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.9  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.8  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.7  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.6  2002/11/18 17:29:54  mrimer
// Added: rounding the now real-valued particle position.
//
// Revision 1.5  2002/10/07 20:12:17  mrimer
// Optimized painting by changing BlitTileImage calls to BlitTileImagePart.
//
// Revision 1.4  2002/06/21 05:24:26  mrimer
// Revised includes.
//
// Revision 1.3  2002/06/11 22:51:44  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.2  2002/04/23 03:46:57  erikh2000
// Changed TI_BLOOD_* to TI_TARBLOOD_*, so it draws the tar blood.
//
// Revision 1.1  2002/04/22 21:58:02  mrimer
// Initial check-in.
// Implemented CTarStabEffect::Draw().
//
