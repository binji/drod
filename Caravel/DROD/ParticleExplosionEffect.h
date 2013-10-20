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

//SUMMARY
//
//CParticleExplosionEffect is a base class for particle explosion effects drawn on top of widgets.
//Effects are temporary animations drawn by the owner widget.  The screen surface 
//update is performed by the owner widget.
//

#ifndef CPARTICLEEXPLOSIONEFFECT_H
#define CPARTICLEEXPLOSIONEFFECT_H

#include "DrodEffect.h"
#include "../DRODLib/CurrentGame.h"

#define ROUND(x)	(int)((x) + 0.5)

const UINT PARTICLES_PER_EXPLOSION=25;

struct PARTICLE
{
	float x, y;    //position
	float mx, my;	//momentum
	int type;      //tile #
	UINT wDurationLeft;
	bool bActive;
};

//******************************************************************************
class CParticleExplosionEffect : public CEffect
{
public:
	CParticleExplosionEffect(CWidget *pSetWidget, const CMoveCoord &MoveCoord,
         const UINT wMaxParticleSize, const UINT wParticles=PARTICLES_PER_EXPLOSION);
	virtual ~CParticleExplosionEffect();

	//virtual bool	Draw() = 0;

protected:
	bool MoveParticles();

	PARTICLE *parrParticles;
	UINT wParticleCount;
   const UINT wMaxParticleSize;

private:
	bool OutOfBounds(const PARTICLE &particle) const;
	bool HitsObstacle(const PARTICLE &particle) const;
	void ReflectParticle(PARTICLE &particle) const;

	SDL_Rect screenRect;
};

#endif	//...#ifndef CPARTICLEEXPLOSIONEFFECT_H

// $Log: ParticleExplosionEffect.h,v $
// Revision 1.12  2003/09/16 20:40:24  mrimer
// Converted int momentums to floats for more realism.
//
// Revision 1.11  2003/08/05 01:39:47  mrimer
// Moved DROD-specific effects from FrontEndLib.
//
// Revision 1.10  2003/07/09 21:09:21  mrimer
// Fixed bug: particles going off edge of room widget.
//
// Revision 1.9  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.8  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.7  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.6  2002/12/22 02:23:05  mrimer
// Moved dwTimeOfLastMove from CParticleExplosionEffect to CEffect.
//
// Revision 1.5  2002/11/18 18:34:06  mrimer
// Added support for time-based, not frame-based, animation.
//
// Revision 1.4  2002/07/22 01:00:29  erikh2000
// Made destructor declaration virtual just so that the virtual status is more obvious.
//
// Revision 1.3  2002/06/21 04:57:09  mrimer
// Revised includes.
//
// Revision 1.2  2002/05/16 18:20:44  mrimer
// Modified constructor to allow specification of quantity of particles in BloodEffect.
//
// Revision 1.1  2002/04/22 21:44:07  mrimer
// Added base class to handle all particle explosions.
//
