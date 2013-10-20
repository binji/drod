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

#include "CheckpointEffect.h"
#include "TileImageConstants.h"

//********************************************************************************
CCheckpointEffect::CCheckpointEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,		//(in)	Should be a room widget.
	const CCoord &SetCoord)		//(in)	Location of checkpoint.
	: CAnimatedTileEffect(pSetWidget,SetCoord)
{
}

//********************************************************************************
bool CCheckpointEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	if (GetFrameNo() > 30)
		//Effect is done.
		return false;

   if (!pDestSurface)
	   pDestSurface = GetDestSurface();

   //Draw lit up checkpoint.
	DrawTile(TI_CHECKPOINT_L, pDestSurface);

	//Continue effect.
	return true;
}

// $Log: CheckpointEffect.cpp,v $
// Revision 1.4  2003/07/01 20:30:31  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.3  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.2  2002/07/26 18:28:25  mrimer
// Refactored tile blitting code into base class.
//
// Revision 1.1  2002/07/25 18:55:54  mrimer
// Initial check-in.
//
