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

#include "Fade.h"

#include <BackEndLib/Assert.h>

#ifndef __native_client__
#include <memory.h>
#endif

//*****************************************************************************
void CFade::InitFade(
//Initialize vars for fade between two surfaces.
//Called upon construction.
//
//Params:
	SDL_Surface* pOldSurface,	//(in)	Surface that contains starting image and 
								//		will be destination surface that ends
								//		up faded to new surface when routine exits.
	SDL_Surface* pNewSurface)	//(in)	Image that destination surface will change to.
{
	//Set NULL pointers to temporary black screens
	//(for simple fade-in/out effects).
	if (!pOldSurface)
	{
		bOldNull = true;
		if (!pNewSurface)
		{
			bNewNull = true;
			return;	//nothing to do
		}
		pOldFadeSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			pNewSurface->w, pNewSurface->h, 24, 0, 0, 0, 0);
      if (!pOldFadeSurface) return;
      SDL_FillRect(pOldFadeSurface,NULL,0);  //make black screen
	} else {
		pOldFadeSurface = pOldSurface;	//keep track of surfaces
	}
	if (!pNewSurface)
	{
		ASSERT(pOldSurface);
		bNewNull = true;
		pNewFadeSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			pOldFadeSurface->w, pOldFadeSurface->h, 24, 0, 0, 0, 0);
      if (!pNewFadeSurface) return;
		SDL_FillRect(pNewFadeSurface,NULL,0);  //make black screen
	} else {
		pNewFadeSurface = pNewSurface;
	}

	if ( SDL_MUSTLOCK(pOldFadeSurface) )
		if ( SDL_LockSurface(pOldFadeSurface) < 0 )
			return;

	//The new surface shouldn't need a lock unless it is somehow a screen surface.
	ASSERT(!SDL_MUSTLOCK(pNewFadeSurface));

	//The dimensions and format of the old and new surface must match exactly.
	ASSERT(pOldFadeSurface->pitch == pNewFadeSurface->pitch);
	ASSERT(pOldFadeSurface->w == pNewFadeSurface->w);
	ASSERT(pOldFadeSurface->h == pNewFadeSurface->h);
	ASSERT(pOldFadeSurface->format->Rmask == pNewFadeSurface->format->Rmask);
	ASSERT(pOldFadeSurface->format->Rshift == pNewFadeSurface->format->Rshift);
	ASSERT(pOldFadeSurface->format->Rloss == pNewFadeSurface->format->Rloss);
	ASSERT(pOldFadeSurface->format->Gmask == pNewFadeSurface->format->Gmask);
	ASSERT(pOldFadeSurface->format->Gshift == pNewFadeSurface->format->Gshift);
	ASSERT(pOldFadeSurface->format->Gloss == pNewFadeSurface->format->Gloss);
	ASSERT(pOldFadeSurface->format->Bmask == pNewFadeSurface->format->Bmask);
	ASSERT(pOldFadeSurface->format->Bshift == pNewFadeSurface->format->Bshift);
	ASSERT(pOldFadeSurface->format->Bloss == pNewFadeSurface->format->Bloss);
	ASSERT(pOldFadeSurface->format->Rshift == pNewFadeSurface->format->Rshift);
	ASSERT(pOldFadeSurface->format->BytesPerPixel == pNewFadeSurface->format->BytesPerPixel);

	//Extract RGB pixel values from each image.
	ASSERT(pNewFadeSurface->format->BytesPerPixel==3);	//24-bit color only supported

	const Uint32 size = pOldFadeSurface->pitch * pOldFadeSurface->h;
	fadeFromRGB = new Uint8[size];
	fadeToRGB = new Uint8[size];

	Uint8 *prf = (Uint8 *)pOldFadeSurface->pixels;
	Uint8 *prt = (Uint8 *)pNewFadeSurface->pixels;
	memcpy(fadeFromRGB, prf, size);
	memcpy(fadeToRGB, prt, size);

   if ( SDL_MUSTLOCK(pOldFadeSurface) ) SDL_UnlockSurface(pOldFadeSurface);
}

//*****************************************************************************
void CFade::ExitFade()
//Complete fade and clean up vars.
{
	if (bOldNull && bNewNull) return;	//no fade to do

	if ( SDL_MUSTLOCK(pOldFadeSurface) )
		SDL_UnlockSurface(pOldFadeSurface);

	if (fadeFromRGB && fadeToRGB && pOldFadeSurface && pNewFadeSurface)
	{
		//Show new surface entirely.
		SDL_BlitSurface(pNewFadeSurface, NULL, pOldFadeSurface, NULL);
		SDL_UpdateRect(pOldFadeSurface,0,0,pOldFadeSurface->w, pOldFadeSurface->h);
	}

	//Clean up.
	delete [] fadeFromRGB;
	delete [] fadeToRGB;
	fadeFromRGB = fadeToRGB = NULL;

	if (bOldNull)
		SDL_FreeSurface(pOldFadeSurface);
	if (bNewNull)
		SDL_FreeSurface(pNewFadeSurface);
	pOldFadeSurface = pNewFadeSurface = NULL;
}

//*****************************************************************************
void CFade::IncrementFade(
//Show transition between two surfaces at 'ratio' between them.
//
//'pOldFadeSurface' is the surface the fade is applied to,
//'fadeFromRGB' is a copy of what the old screen looks like, and
//'fadeToRGB' is a copy of what the new screen looks like,
// neither faded in or out, but just normal.
//NOTE: fadeFrom, fadeTo, and pOldFadeSurface must be the same size and dimensions.
//
//Params:
	float fRatio)	//(in) mixing ratio [0,1].
{
	if (bOldNull && bNewNull) return;	//no fade to do

   if (fRatio < 0.0) fRatio = 0.0;
   if (fRatio > 1.0) fRatio = 1.0;

	const Uint8 amount = (Uint8)(fRatio * 255);
	const Uint8 nOldAmt = 255 - amount;

	const Uint8 *pFrom = fadeFromRGB;
	const Uint8 *pTo = fadeToRGB;
	
	//Mix pixels in "from" and "to" images by 'amount'.
	Uint8 *pw = (Uint8 *)pOldFadeSurface->pixels;
	const Uint32 size = pOldFadeSurface->pitch * pOldFadeSurface->h;
	Uint8 *const pStop = pw + size;

   if ( SDL_MUSTLOCK(pOldFadeSurface) )
		if ( SDL_LockSurface(pOldFadeSurface) < 0 )
			return;

   do {
      //dividing by 256 instead of 255 provides huge optimization
	   *pw = (nOldAmt * *(pFrom++) + amount * *(pTo++)) / 256;
   } while (++pw != pStop);

   if ( SDL_MUSTLOCK(pOldFadeSurface) ) SDL_UnlockSurface(pOldFadeSurface);

	SDL_UpdateRect(pOldFadeSurface, 0, 0, pOldFadeSurface->w, pOldFadeSurface->h);
}

//*****************************************************************************
void CFade::FadeBetween(
//Fade between two surfaces of the same dimensions.
//Effect duration is independent of machine speed.
//
//Params:
	const UINT wFadeDuration)
{
	ASSERT(wFadeDuration > 0);

	if ((bOldNull && bNewNull) || wFadeDuration == 0) return;	//no fade to do

	//Fade from old to new surface.  Effect takes constant time.
	DWORD
		dwFirstPaint = SDL_GetTicks(),
		dwNow = dwFirstPaint;
	do
	{
		//The +50 is to allow first frame to show some change.
		IncrementFade((dwNow - dwFirstPaint + 50)	/ (float)wFadeDuration);
		dwNow = SDL_GetTicks();
	} while (dwNow - dwFirstPaint + 50 < wFadeDuration);	// constant-time effect
}

//Much of Fade.cpp/h's code was cut-and-pasted from ScreenManager.cpp, and contains 
//contributions outside of those contained in erikh2000's commits.  Before the 
//revision history shown in the log for fade.cpp/h, mrimer wrote the original 
//fading routines.  Optimizations were then made by erikh2000.  See history of 
//ScreenManager.cpp/h if interested.

// $Log: Fade.cpp,v $
// Revision 1.7  2004/01/02 01:12:28  mrimer
// Changed a comment.
//
// Revision 1.6  2003/09/16 16:07:55  mrimer
// Removed test code.  Optimized fade loop slightly.  Fixed some comments.
//
// Revision 1.5  2003/09/16 02:23:45  erikh2000
// Fixed incorrect lock/unlock pairs for surfaces.  UpdateRect shouldn't be called while surface is locked.  OldSurface was not getting unlocked on exit fade.
//
// Revision 1.4  2003/09/11 02:01:43  mrimer
// Optimized fade loop (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/06/18 03:35:59  mrimer
// Fixed a bug.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.4  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.3  2002/10/21 09:35:26  erikh2000
// Added a safeguard against division by zero error.
//
// Revision 1.2  2002/10/02 21:40:17  mrimer
// Put fade routines into class CFade.
// Added InitFade(), IncrementFade(), and ExitFade() routines.
// Generalized routines to perform fades on any surface, not just full screens.
// Parameterized fade duration.
//
// Revision 1.1  2002/10/01 22:38:57  erikh2000
// Initial check-in.
//
