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

#ifndef COLORS_H
#define COLORS_H

#include <SDL.h>

struct SURFACECOLOR 
{
	Uint8 byt1;
	Uint8 byt2; 
	Uint8 byt3;
};

SURFACECOLOR	GetSurfaceColor(const SDL_Surface *pSurface, Uint8 bytRed, 
		Uint8 bytGreen, Uint8 bytBlue);

#endif //#ifndef COLORS_H

// $Log: Colors.h,v $
// Revision 1.2  2003/08/08 17:35:01  mrimer
// Added FlashShadeEffect.  Some code maintenance.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.3  2002/06/03 22:55:42  mrimer
// Inserted SDL files.
//
// Revision 1.2  2002/05/21 21:31:05  erikh2000
// Changed GetSurfaceColor() to return surface color value.
//
// Revision 1.1  2002/04/14 00:06:46  erikh2000
// Initial check-in.
//
