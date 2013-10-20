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

#ifndef FADE_H
#define FADE_H

#include <BackEndLib/Types.h>

#include <SDL.h>

//Fade between two SDL_Surfaces.
class CFade
{
public:
	CFade(SDL_Surface* pOldSurface, SDL_Surface* pNewSurface)
		: pOldFadeSurface(NULL), pNewFadeSurface(NULL)
		, fadeFromRGB(NULL), fadeToRGB(NULL)
		, bOldNull(false), bNewNull(false)
			{InitFade(pOldSurface,pNewSurface);}

	~CFade() {ExitFade();}

	//Performs a step of the fade.
	void IncrementFade(float fRatio);

	//Performs entire fade.
	void FadeBetween(const UINT wFadeDuration=400);	//How long to fade, in milliseconds

private:
	SDL_Surface *pOldFadeSurface, *pNewFadeSurface;
	Uint8 *fadeFromRGB, *fadeToRGB;	//surface pixels
	bool bOldNull, bNewNull;			//whether a surface is NULL

	void InitFade(SDL_Surface* pOldSurface, SDL_Surface* pNewSurface);
	void ExitFade(void);
};

#endif //...#ifndef FADE_H

//Much of Fade.cpp/h's code was cut-and-pasted from ScreenManager.cpp, and contains 
//contributions outside of those contained in erikh2000's commits.  Before the 
//revision history shown in the log for fade.cpp/h, mrimer wrote the original 
//fading routines.  Optimizations were then made by erikh2000.  See history of 
//ScreenManager.cpp/h if interested.

// $Log: Fade.h,v $
// Revision 1.3  2003/06/18 03:35:59  mrimer
// Fixed a bug.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.3  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
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
