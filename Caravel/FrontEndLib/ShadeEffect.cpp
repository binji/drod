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

#include "ShadeEffect.h"

//********************************************************************************
CShadeEffect::CShadeEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,			//(in)	Should be a room widget.
	const CCoord &SetCoord,		//(in)	Location of checkpoint.
	const SURFACECOLOR &Color)	//(in)	Color to shade with.
	: CAnimatedTileEffect(pSetWidget,SetCoord,EFFECTLIB::ESHADE)
	, Color(Color)
{
}

//********************************************************************************
bool CShadeEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.  Never ends.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
   if (!pDestSurface)
      pDestSurface = GetDestSurface();

	//Add shading to tile.
	ShadeTile(this->Color, pDestSurface);

	//Continue effect.
	return true;
}

// $Log: ShadeEffect.cpp,v $
// Revision 1.3  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.2  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.2  2002/11/15 03:01:21  mrimer
// Added effect type to constructor.
//
// Revision 1.1  2002/09/24 21:24:26  mrimer
// Initial check-in.
//
