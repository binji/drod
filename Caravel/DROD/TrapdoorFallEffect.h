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

#ifndef TRAPDOORFALLEFFECT_H
#define TRAPDOORFALLEFFECT_H

#include "DrodEffect.h"

#include "../DRODLib/DbRooms.h"
#include "../DRODLib/CurrentGame.h"

//****************************************************************************************
class CTrapdoorFallEffect : public CEffect
{
public:
	CTrapdoorFallEffect(CWidget *pSetWidget, const CCoord &SetCoord);

	virtual bool	Draw(SDL_Surface* pDestSurface);
	virtual long	GetDrawSequence(void) const {return 1L + this->wRow;}

private:
	CDbRoom *	pRoom;
	int			xTrapdoor, yTrapdoor;
	int			yTrapdoorB, yTrapdoorC;
	UINT		wCol, wRow;
};

#endif //...#ifndef TRAPDOORFALLEFFECT_H

// $Log: TrapdoorFallEffect.h,v $
// Revision 1.10  2003/08/05 01:39:47  mrimer
// Moved DROD-specific effects from FrontEndLib.
//
// Revision 1.9  2003/07/01 20:30:32  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.8  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.7  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.6  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.5  2002/08/25 19:06:01  erikh2000
// Trapdoors are now a three square animation.
//
// Revision 1.4  2002/07/25 18:54:59  mrimer
// Refactored stuff into new CAnimatedTileEffect class.
//
// Revision 1.3  2002/06/21 04:57:09  mrimer
// Revised includes.
//
// Revision 1.2  2002/04/29 00:23:22  erikh2000
// Revised #includes.
//
// Revision 1.1  2002/04/12 05:15:10  erikh2000
// Initial check-in.
//
