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

#include "TransTileEffect.h"
#include "BitmapManager.h"

#include <BackEndLib/Assert.h>

UINT CTransTileEffect::wInstances = 0;

//********************************************************************************
CTransTileEffect::CTransTileEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,		//(in)	Should be a room widget.
	const CCoord &SetCoord,		//(in)	Location of checkpoint.
	const UINT wTileImageNo)	//(in)	Tile to display.
	: CAnimatedTileEffect(pSetWidget,SetCoord,EFFECTLIB::ETRANSTILE)
	, wTileImageNo(wTileImageNo)
{
	this->bFirst = (wInstances == 0);
	++wInstances;
}

//********************************************************************************
CTransTileEffect::~CTransTileEffect()
//Destructor.
{
	--wInstances;
}

//********************************************************************************
bool CTransTileEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	//Change level of transparency.
	static const unsigned char MIN_OPACITY = 32;
	static const unsigned char MAX_OPACITY = 192;
	static unsigned char nOpacity = MIN_OPACITY;	//If we have multiple effects, we'll
	static bool bRising = true;	//want them synched, so we'll maintain a static var.
	if (this->bFirst)
	{
		//Value is modified by only one of the instances.
		if (bRising)
		{
			nOpacity += 3;
			if (nOpacity > MAX_OPACITY)
				bRising = false;
		} else {
			nOpacity -= 3;
			if (nOpacity < MIN_OPACITY)
				bRising = true;
		}
	}

   if (!pDestSurface)
      pDestSurface = GetDestSurface();

	//Draw tile.
	DrawTile(this->wTileImageNo, pDestSurface, nOpacity);

	//Continue effect.
	return true;
}

// $Log: TransTileEffect.cpp,v $
// Revision 1.5  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.4  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.3  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.1  2002/11/15 02:18:00  mrimer
// Initial check-in.
//
