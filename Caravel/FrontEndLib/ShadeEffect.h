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

#ifndef SHADEEFFECT_H
#define SHADEEFFECT_H

#include "AnimatedTileEffect.h"

//****************************************************************************************
class CShadeEffect : public CAnimatedTileEffect
{
public:
	CShadeEffect(CWidget *pSetWidget, const CCoord &SetCoord,
			const SURFACECOLOR &Color);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);

protected:
	SURFACECOLOR Color;
};

#endif //...#ifndef SHADEEFFECT_H

// $Log: ShadeEffect.h,v $
// Revision 1.3  2003/08/08 17:35:01  mrimer
// Added FlashShadeEffect.  Some code maintenance.
//
// Revision 1.2  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.1  2002/09/24 21:24:26  mrimer
// Initial check-in.
//
