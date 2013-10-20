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

#include "BloodEffect.h"
#include "TileImageConstants.h"
#include "DrodBitmapManager.h"
#include "../DRODLib/GameConstants.h"
#include <BackEndLib/Assert.h>

//********************************************************************************
CBloodEffect::CBloodEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,			//(in)	Should be a room widget.
	const CMoveCoord &MoveCoord,	//(in)	Location of blood and direction of its movement.
	const UINT wParticles)	//(in) number of particles to generate
	: CParticleExplosionEffect(pSetWidget, MoveCoord, 4, wParticles)
{
}

//********************************************************************************
bool CBloodEffect::Draw(SDL_Surface* pDestSurface)
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
				g_pTheBM->BlitTileImagePart(TI_BLOOD_2, ROUND(parrParticles[nIndex].x),
						ROUND(parrParticles[nIndex].y), 4, 4, pDestSurface);
			else
				g_pTheBM->BlitTileImagePart(TI_BLOOD_1, ROUND(parrParticles[nIndex].x),
						ROUND(parrParticles[nIndex].y), 3, 3, pDestSurface);

	return true;
}

// $Log: BloodEffect.cpp,v $
// Revision 1.14  2003/07/09 21:10:46  mrimer
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
// Revision 1.9  2002/11/18 17:29:54  mrimer
// Added: rounding the now real-valued particle position.
//
// Revision 1.8  2002/10/07 20:12:12  mrimer
// Optimized painting by changing BlitTileImage calls to BlitTileImagePart.
//
// Revision 1.7  2002/06/21 05:00:15  mrimer
// Revised includes.
//
// Revision 1.6  2002/06/11 22:59:00  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.5  2002/05/16 18:20:44  mrimer
// Modified constructor to allow specification of quantity of particles in BloodEffect.
//
// Revision 1.4  2002/04/22 21:45:32  mrimer
// Implemented CBloodEffect::Draw().
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
