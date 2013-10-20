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
 * Contributor(s): mrimer
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef BLOODEFFECT_H
#define BLOODEFFECT_H

#include "ParticleExplosionEffect.h"

//****************************************************************************************
class CBloodEffect : public CParticleExplosionEffect
{
public:
	CBloodEffect(CWidget *pSetWidget, const CMoveCoord &MoveCoord, const UINT wParticles=PARTICLES_PER_EXPLOSION);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);
};

#endif //...#ifndef BLOODEFFECT_H

// $Log: BloodEffect.h,v $
// Revision 1.5  2003/07/01 20:30:31  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.4  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2002/05/16 18:20:44  mrimer
// Modified constructor to allow specification of quantity of particles in BloodEffect.
//
// Revision 1.2  2002/04/22 21:45:32  mrimer
// Implemented CBloodEffect::Draw().
//
// Revision 1.1  2002/04/12 05:15:10  erikh2000
// Initial check-in.
//
