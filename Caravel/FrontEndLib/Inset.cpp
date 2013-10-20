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

#include <SDL.h>

#include "Inset.h"
#include <BackEndLib/Assert.h>

//********************************************************************************
void DrawInset(
//Draw inset area to a surface.
//
//Params:
	int nX, int nY,				//(in)	Dest coords.
	UINT wW, UINT wH,			//(in)	Dimension of area to draw.
	SDL_Surface *pPartsSurface,	//(in)	Contains parts need to draw inset.
	SDL_Surface *pDestSurface)	//(in)	Surface to draw to.
{
	//Source coords and dimensions within parts surface.
	const int X_LEFT_BEVEL = 151;
	const int Y_TOP_BEVEL = 0;
	const UINT CX_CENTER = 13;
	const UINT CY_CENTER = 13;
	const int X_RIGHT_BEVEL = 165;
	const int Y_BOTTOM_BEVEL = 14;
	const int X_CENTER = 152;
	const int Y_CENTER = 1;

	//Dest coords and dimensions.
	int xLeftBevel = nX;
	int xRightBevel = nX + wW - CX_INSET_BORDER;
	int yTopBevel = nY;
	int yBottomBevel = nY + wH - CY_INSET_BORDER;
	int xCenter = xLeftBevel + CX_INSET_BORDER;
	int yCenter = yTopBevel + CY_INSET_BORDER;

	//Draw top-left corner.
	SDL_Rect src = {X_LEFT_BEVEL, Y_TOP_BEVEL, CX_INSET_BORDER, CY_INSET_BORDER};
	SDL_Rect dest = {xLeftBevel, yTopBevel, CX_INSET_BORDER, CY_INSET_BORDER};
	SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);

	//Draw top-right corner.
	src.x = X_RIGHT_BEVEL;
	dest.x = xRightBevel;
	SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-right corner.
	src.y = Y_BOTTOM_BEVEL;
	dest.y = yBottomBevel;
	SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-left corner.
	src.x = X_LEFT_BEVEL;
	dest.x = xLeftBevel;
	SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom bevel.
	src.x = X_LEFT_BEVEL + CX_INSET_BORDER;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xLeftBevel + CX_INSET_BORDER; dest.x < xRightBevel; dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > (UINT)xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw top bevel.
	dest.y = yTopBevel;
	src.y = Y_TOP_BEVEL;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xLeftBevel + CX_INSET_BORDER; dest.x < xRightBevel; dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > (UINT)xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw left bevel.
	dest.x = xLeftBevel;
	src.x = X_LEFT_BEVEL;
	src.y = Y_TOP_BEVEL + CY_INSET_BORDER;
	src.w = dest.w = CX_INSET_BORDER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yTopBevel + CY_INSET_BORDER; dest.y < yBottomBevel; dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > (UINT)yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw right bevel.
	dest.x = xRightBevel;
	src.x = X_RIGHT_BEVEL;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yTopBevel + CY_INSET_BORDER; dest.y < yBottomBevel; dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > (UINT) yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw center.
	src.x = X_CENTER;
	src.y = Y_CENTER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < yBottomBevel; dest.y += CY_CENTER)
	{
		src.w = dest.w = CX_CENTER;
		for (dest.x = xCenter; dest.x < xRightBevel; dest.x += CX_CENTER)
		{
			if (dest.x + CX_CENTER > (UINT)xRightBevel)
				dest.w = src.w = xRightBevel - dest.x; //Clip the blit to remaining width.
			if (dest.y + CY_CENTER > (UINT)yBottomBevel)
				dest.h = src.h = yBottomBevel - dest.y; //Clip the blit to remaining height.
			SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
		}
	}
}

// $Log: Inset.cpp,v $
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.2  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.1  2002/04/29 00:00:52  erikh2000
// Initial check-in.
//
