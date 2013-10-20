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

#ifndef BUMPOBSTACLEEFFECT_H
#define BUMPOBSTACLEEFFECT_H

#include "Effect.h"
#include "Widget.h"
#include <BackEndLib/Types.h>

//****************************************************************************************
class CBumpObstacleEffect : public CEffect
{
public:
	CBumpObstacleEffect(CWidget *pSetWidget, UINT wSetCol, UINT wSetRow, UINT wSetBumpO);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);

private:
	SDL_Rect src, dest;
};

#endif //...#ifndef BUMPOBSTACLEEFFECT_H

// $Log: BumpObstacleEffect.h,v $
// Revision 1.3  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.2  2003/05/25 22:44:35  erikh2000
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
// Revision 1.2  2002/07/09 22:21:57  mrimer
// Revised #includes.
//
// Revision 1.1  2002/04/12 05:15:10  erikh2000
// Initial check-in.
//
