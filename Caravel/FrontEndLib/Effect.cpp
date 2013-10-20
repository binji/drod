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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "Effect.h"
#include <BackEndLib/Assert.h>

//*********************************************************************************
CEffect::CEffect(CWidget *pSetOwnerWidget, const UINT eType)
	: pOwnerWidget(pSetOwnerWidget)
	, dwTimeStarted(SDL_GetTicks()), dwTimeOfLastMove(SDL_GetTicks())
	, eEffectType(eType)
//Constructor.
{
	ASSERT(pSetOwnerWidget);
	this->rAreaOfEffect.x = this->rAreaOfEffect.y =
			this->rAreaOfEffect.w = this->rAreaOfEffect.h = 0;
}

// $Log: Effect.cpp,v $
// Revision 1.3  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.2  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.6  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.5  2002/12/22 02:30:48  mrimer
// Moved dwTimeOfLastMove from CParticleExplosionEffect to CEffect.
//
// Revision 1.4  2002/10/10 21:09:48  mrimer
// Added rAreaOfEffect and GetAreaOfEffect().
//
// Revision 1.3  2002/09/14 21:20:15  mrimer
// Added EffectType tag.
//
// Revision 1.2  2002/06/21 05:03:55  mrimer
// Revised includes.
//
// Revision 1.1  2002/04/12 05:15:10  erikh2000
// Initial check-in.
//
