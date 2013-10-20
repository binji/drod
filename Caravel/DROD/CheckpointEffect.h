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

#ifndef CHECKPOINTEFFECT_H
#define CHECKPOINTEFFECT_H

#include <FrontEndLib/AnimatedTileEffect.h>

//****************************************************************************************
class CCheckpointEffect : public CAnimatedTileEffect
{
public:
	CCheckpointEffect(CWidget *pSetWidget, const CCoord &SetCoord);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);
};

#endif //...#ifndef CHECKPOINTEFFECT_H

// $Log: CheckpointEffect.h,v $
// Revision 1.5  2003/07/01 20:30:31  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.4  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/05/23 21:43:02  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.2  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.1  2002/07/25 18:55:54  mrimer
// Initial check-in.
//
