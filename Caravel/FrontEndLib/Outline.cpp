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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#include <SDL.h>

#include "Colors.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>
#include <BackEndLib/CoordIndex.h>

void AddOutline_3BPP(SDL_Surface *pSurface, SURFACECOLOR OutlineColor, 
		SURFACECOLOR BackgroundColor, UINT wWidth);
void AddOutline_1BPP(SDL_Surface *pSurface, Uint8 OutlineColor, 
		Uint8 BackgroundColor, UINT wWidth);

//************************************************************************************
void AddOutline(
//Call either 1-bpp or 3-bpp routine to add outline to pixels on a surface.  Background
//color must be set by color key on the surface.
//
//Params:
	SDL_Surface *pSurface,			//(in)	Surface to add outline to.
	SDL_Color OutlineColor,			//(in)	Color of outline pixels.
	UINT wWidth)					//(in)	Width in pixels to make the outline.  Default
									//		is 1.
{
	//Lock surface.
	if (SDL_MUSTLOCK(pSurface)) 
	{
		while ( SDL_LockSurface(pSurface) < 0 ) 
			SDL_Delay(100);
	}

	//Get surface colors and make the call.
	switch (pSurface->format->BytesPerPixel)
	{
		case 1:
		{
			Uint32 OutlineSurfaceColor = SDL_MapRGB(pSurface->format, OutlineColor.r, 
					OutlineColor.g, OutlineColor.b);
			Uint32 BackgroundSurfaceColor = pSurface->format->colorkey;
			AddOutline_1BPP(pSurface, OutlineSurfaceColor, BackgroundSurfaceColor, wWidth);
		}
		break;

		case 3:
		{
			SURFACECOLOR OutlineSurfaceColor = GetSurfaceColor(pSurface, OutlineColor.r, 
					OutlineColor.g, OutlineColor.b);			
			Uint32 ColorKey = pSurface->format->colorkey;
			Uint8 r, g, b;
			SDL_GetRGB(ColorKey, pSurface->format, &r, &g, &b);
			SURFACECOLOR BackgroundSurfaceColor = GetSurfaceColor(pSurface, r, g, b);
			AddOutline_3BPP(pSurface, OutlineSurfaceColor, BackgroundSurfaceColor, wWidth);
		}
		break;

		default:
			ASSERTP(false,"No implementation for this.");
	}

	//Unlock surface.
	if (SDL_MUSTLOCK(pSurface)) SDL_UnlockSurface(pSurface);
}

#define ISCOLOR(pPixel,SurfaceColor) \
	((pPixel)[0] == (SurfaceColor).byt1 && \
	(pPixel)[1] == (SurfaceColor).byt2 && \
	(pPixel)[3] == (SurfaceColor).byt3)
									
//************************************************************************************
void AddOutline_3BPP(
//Adds an outline around the perimeter of non-background pixels in surface.
//
//Params:
	SDL_Surface *pSurface,			//(in)	Surface to add outline to.
	SURFACECOLOR OutlineColor,		//(in)	Color of outline pixels.
	SURFACECOLOR BackgroundColor,	//(in)	Indicates which pixels are the background.
	UINT wWidth)					//(in)	Width in pixels to make the outline.
{
	//Index will hold all the outline plots to make.
	CCoordIndex OutlinePlots;
	if (!OutlinePlots.Init(pSurface->w, pSurface->h)) return;

	const BYTE *pbytIndex = OutlinePlots.GetIndex();
	const BYTE *pbytStop = pbytIndex + (pSurface->w * pSurface->h);
	const BYTE *pbytSeek;
	UINT x, y;
	Uint8 *pPixel;
	UINT wBPP = pSurface->format->BytesPerPixel;
	ASSERT(wBPP == 3);
	UINT wRowOffset = pSurface->pitch - (pSurface->w * wBPP);

	//Each iteration draws one pixel-width outline.
	for (UINT wWidthI = 0; wWidthI < wWidth; ++wWidthI)
	{
		//Each iteration finds outline pixels for one row.
		pPixel = static_cast<Uint8 *>(pSurface->pixels);
		for (y = 0; y < static_cast<UINT>(pSurface->h); ++y)
		{
			//Each iteration checks pixel to see if outline pixels go around it.
			for ( x = 0; x < static_cast<UINT>(pSurface->w); ++x)
			{
				if (!ISCOLOR(pPixel, BackgroundColor)) //Found a non-background pixel.
				{
					//Add outline pixels to plot index.
					if (x > 0 && ISCOLOR(pPixel - 1, BackgroundColor)) 
						OutlinePlots.Add(x - 1, y);
					if (x < static_cast<UINT>(pSurface->w) - 1 &&
							ISCOLOR(pPixel + 1, BackgroundColor)) 
						OutlinePlots.Add(x + 1, y);
					if (y > 0 &&
							ISCOLOR(pPixel - pSurface->pitch, BackgroundColor)) 
						OutlinePlots.Add(x, y - 1);
					if (y < static_cast<UINT>(pSurface->h) - 1 &&
							ISCOLOR(pPixel + pSurface->pitch, BackgroundColor)) 
						OutlinePlots.Add(x, y + 1);
				}
				pPixel += wBPP;
			}
			pPixel += wRowOffset;
		}

		//Plot all the outline pixels.
		pbytSeek = pbytIndex;
		while (pbytSeek != pbytStop)
		{
			if (*pbytSeek)
			{
				y = (pbytSeek - pbytIndex) / pSurface->w;
				x = (pbytSeek - pbytIndex) % pSurface->w; 
				pPixel = static_cast<Uint8 *>(pSurface->pixels) + 
						(y * pSurface->pitch) + (x * wBPP);
				pPixel[0] = OutlineColor.byt1;
				pPixel[1] = OutlineColor.byt2;
				pPixel[2] = OutlineColor.byt3;
			}
			++pbytSeek;
		}
	}
}

//************************************************************************************
void AddOutline_1BPP(
//1-bpp version of above routine.
//
//Params:
	SDL_Surface *pSurface,			//(in)	Surface to add outline to.
	Uint8 OutlineColor,				//(in)	Color of outline pixels.
	Uint8 BackgroundColor,			//(in)	Indicates which pixels are the background.
	UINT wWidth)					//(in)	Width in pixels to make the outline.
{
	//Index will hold all the outline plots to make.
	CCoordIndex OutlinePlots;
	if (!OutlinePlots.Init(pSurface->w, pSurface->h)) return;

	const BYTE *pbytIndex = OutlinePlots.GetIndex();
	const BYTE *pbytStop = pbytIndex + (pSurface->w * pSurface->h);
	const BYTE *pbytSeek;
	UINT x, y;
	Uint8 *pPixel;
	ASSERT(pSurface->format->BytesPerPixel);
	UINT wRowOffset = pSurface->pitch - pSurface->w;

	//Each iteration draws one pixel-width outline.
	for (UINT wWidthI = 0; wWidthI < wWidth; ++wWidthI)
	{
		//Each iteration finds outline pixels for one row.
		pPixel = static_cast<Uint8 *>(pSurface->pixels);
		for (y = 0; y < static_cast<UINT>(pSurface->h); ++y)
		{
			//Each iteration checks pixel to see if outline pixels go around it.
			for ( x = 0; x < static_cast<UINT>(pSurface->w); ++x)
			{
				if (*pPixel != BackgroundColor) //Found a non-background pixel.
				{
					//Add outline pixels to plot index.
					if (x > 0 && pPixel[-1] == BackgroundColor) 
						OutlinePlots.Add(x - 1, y);
					if (x < static_cast<UINT>(pSurface->w) - 1 && 
							pPixel[1] == BackgroundColor) 
						OutlinePlots.Add(x + 1, y);
					if (y > 0 && pPixel[-(pSurface->pitch)] == BackgroundColor) 
						OutlinePlots.Add(x, y - 1);
					if (y < static_cast<UINT>(pSurface->h) - 1 &&
							pPixel[pSurface->pitch] == BackgroundColor) 
						OutlinePlots.Add(x, y + 1);
				}
				++pPixel;
			}
			pPixel += wRowOffset;
		}

		//Plot all the outline pixels.
		pbytSeek = pbytIndex;
		while (pbytSeek != pbytStop)
		{
			if (*pbytSeek)
			{
				y = (pbytSeek - pbytIndex) / pSurface->w;
				x = (pbytSeek - pbytIndex) % pSurface->w; 
				pPixel = static_cast<Uint8 *>(pSurface->pixels) + 
						(y * pSurface->pitch) + x;
				*pPixel = OutlineColor;
			}
			++pbytSeek;
		}

		OutlinePlots.Clear();
	}
}

// $Log: Outline.cpp,v $
// Revision 1.3  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.3  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.2  2002/05/21 21:35:29  erikh2000
// Changed color-setting code.
//
// Revision 1.1  2002/05/17 22:53:58  erikh2000
// Initial check-in.
//
