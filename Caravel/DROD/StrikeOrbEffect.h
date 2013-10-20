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

#ifndef STRIKEORBEFFECT_H
#define STRIKEORBEFFECT_H

#include "DrodEffect.h"
#include "../DRODLib/DbRooms.h"
#include <BackEndLib/Types.h>

typedef struct tagBolt
{
	int xBegin;
	int yBegin;
	int xEnd;
	int yEnd;
} BOLT;

//****************************************************************************************
class CStrikeOrbEffect : public CEffect
{
public:
	CStrikeOrbEffect(CWidget *pSetWidget, const COrbData &SetOrbData, 
			SDL_Surface *pSetPartsSurface, bool bSetDrawOrb);
	virtual ~CStrikeOrbEffect(void);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);

private:
	UINT			wOrbX, wOrbY;
	BOLT *			parrBolts;
	UINT			wBoltCount;
   bool        bDrawOrb;

	SDL_Surface	*	pPartsSurface;
};

#endif //...#ifndef STRIKEORBEFFECT_H

// $Log: StrikeOrbEffect.h,v $
// Revision 1.11  2003/08/05 01:39:47  mrimer
// Moved DROD-specific effects from FrontEndLib.
//
// Revision 1.10  2003/07/01 20:30:32  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.9  2003/06/21 04:10:59  schik
// Orbs affecting a door are now colored according to the toggle/open/close action
//
// Revision 1.8  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.7  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.6  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.5  2002/07/22 01:00:29  erikh2000
// Made destructor declaration virtual just so that the virtual status is more obvious.
//
// Revision 1.4  2002/04/22 09:44:11  erikh2000
// Moved bolt-related code to Bolt.h/cpp.
//
// Revision 1.3  2002/04/22 09:41:17  erikh2000
// Initial check-in.
//
// Revision 1.2  2002/04/22 02:55:08  erikh2000
// Strike-orb effect now draws energy bolts from orb to destinations.
//
// Revision 1.1  2002/04/12 05:15:10  erikh2000
// Initial check-in.
//
