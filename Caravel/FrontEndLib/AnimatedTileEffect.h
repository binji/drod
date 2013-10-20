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

#ifndef ANIMATEDTILEEFFECT_H
#define ANIMATEDTILEEFFECT_H

#include "Effect.h"
#include <BackEndLib/Coord.h>

//****************************************************************************************
class CAnimatedTileEffect : public CEffect
{
public:
	CAnimatedTileEffect(CWidget *pSetWidget, const CCoord &SetCoord,
      const UINT eType=EFFECTLIB::EGENERIC);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL)=0;

protected:
	void DrawTile(const UINT wTileImageNo, SDL_Surface* pDestSurface, const Uint8 nOpacity=255);
	void ShadeTile(const SURFACECOLOR &Color, SDL_Surface* pDestSurface);

private:
	UINT		wX, wY;
};

#endif //...#ifndef ANIMATEDTILEEFFECT_H

// $Log: AnimatedTileEffect.h,v $
// Revision 1.4  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.3  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.2  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.6  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.5  2002/11/15 02:49:29  mrimer
// Added optional opacity parameter to DrawTile().
//
// Revision 1.4  2002/09/24 21:15:55  mrimer
// Added ShadeTile().
//
// Revision 1.3  2002/09/14 21:20:15  mrimer
// Added EffectType tag.
//
// Revision 1.2  2002/07/26 18:23:49  mrimer
// Added DrawTile().
//
// Revision 1.1  2002/07/25 18:55:54  mrimer
// Initial check-in.
//
