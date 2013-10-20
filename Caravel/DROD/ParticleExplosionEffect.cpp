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

#include "ParticleExplosionEffect.h"
#include "RoomWidget.h"
#include "TileImageConstants.h"
#include "DrodBitmapManager.h"
#include "../DRODLib/GameConstants.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/DbRooms.h"

const UINT EXPLOSION_SPEED = 2;
const UINT EXPLOSION_DURATION = 7;

//*****************************************************************************
CParticleExplosionEffect::CParticleExplosionEffect(
//Constructor.
//
//Adds a bunch of particles to the display that will continue to move
//in a specified direction with animation updates.
//
//Params:
	CWidget *pSetWidget,			   //(in) Should be a room widget.
	const CMoveCoord &MoveCoord,	//(in) Location of explosion and direction of its movement.
   const UINT wMaxParticleSize,  //(in) Max dimensions of particle sprite
	const UINT wParticles)	      //(in) Number of particles to generate
	: CEffect(pSetWidget)
   , wMaxParticleSize(wMaxParticleSize)
{
	ASSERT(pSetWidget->GetType() == WT_Room);

	pSetWidget->GetRect(this->screenRect);

	//Determine explosion point of origin (center of tile).
	const int x = MoveCoord.wCol*CBitmapManager::CX_TILE + CBitmapManager::CX_TILE/2;
	const int y = MoveCoord.wRow*CBitmapManager::CY_TILE + CBitmapManager::CY_TILE/2;

	//Define ranges for explosion directions.
	int nXRange, nXOffset, nYRange, nYOffset;
	switch (MoveCoord.wO) {
	case NW: case W: case SW: nXRange=EXPLOSION_DURATION+1; nXOffset=-EXPLOSION_SPEED; break;
	case NE: case E: case SE: nXRange=EXPLOSION_DURATION+1; nXOffset=EXPLOSION_SPEED; break;
	case N: case S: case NO_ORIENTATION: nXRange=(int)(EXPLOSION_DURATION*1.414)+1; nXOffset=0; break;
	}
	switch (MoveCoord.wO) {
	case NW: case N: case NE: nYRange=EXPLOSION_DURATION+1; nYOffset=-EXPLOSION_SPEED; break;
	case SW: case S: case SE: nYRange=EXPLOSION_DURATION+1; nYOffset=EXPLOSION_SPEED; break;
	case W: case E: case NO_ORIENTATION: nYRange=(int)(EXPLOSION_DURATION*1.414)+1; nYOffset=0; break;
	}

	//Add random explosion particles.
	this->wParticleCount = wParticles + RAND(wParticles/2);
	this->parrParticles = new PARTICLE[this->wParticleCount];
	for (UINT nIndex=this->wParticleCount; nIndex--; )
	{
		this->parrParticles[nIndex].x = screenRect.x + x;
		this->parrParticles[nIndex].y = screenRect.y + y;
		this->parrParticles[nIndex].mx = nXOffset + fRAND_MID(nXRange*0.5);
		this->parrParticles[nIndex].my = nYOffset + fRAND_MID(nYRange*0.5);
		this->parrParticles[nIndex].wDurationLeft = EXPLOSION_DURATION;
		this->parrParticles[nIndex].type = (RAND(3)==0 ? 1 : 0);	//one of two styles
		this->parrParticles[nIndex].bActive= true;
	}

	MoveParticles();
}

//*****************************************************************************
CParticleExplosionEffect::~CParticleExplosionEffect()
//Destructor.
{
	delete [] this->parrParticles;
}

//*****************************************************************************
bool CParticleExplosionEffect::MoveParticles()
//Updates positions of all the particles.
//Invalidates particles having left the valid display area.
//
//Returns: whether any particles are still active
{
	UINT xMax=0, yMax=0;	//bottom edge of bounding box
	bool bActiveParticles = false;	//whether any particles are still active

	//Reset bounding box.
	this->rAreaOfEffect.x = this->screenRect.x + this->screenRect.w;
	this->rAreaOfEffect.y = this->screenRect.y + this->screenRect.h;
	this->rAreaOfEffect.w = this->rAreaOfEffect.h = 0;

	const Uint32 dwNow = SDL_GetTicks();
   const Uint32 dwTimeElapsed = this->dwTimeOfLastMove >= dwNow ? 1 :
         dwNow - this->dwTimeOfLastMove;
   const float fMultiplier = dwTimeElapsed / 33.0;
	const int nDecay = ROUND(2.0f / fMultiplier);
	for (UINT nIndex=wParticleCount; nIndex--; )
	{
		//See if particle should still be active and moved.
		if (this->parrParticles[nIndex].bActive)
		{
			//Update real position in real time.
			this->parrParticles[nIndex].x += this->parrParticles[nIndex].mx * fMultiplier;
			this->parrParticles[nIndex].y += this->parrParticles[nIndex].my * fMultiplier;

			//If particle is going to go out of bounds, kill it.
			if (OutOfBounds(this->parrParticles[nIndex]))
			{
				this->parrParticles[nIndex].bActive=false;
				continue;
			}

         //Does particle runs into an obstacle?
         if (HitsObstacle(this->parrParticles[nIndex]))
				ReflectParticle(this->parrParticles[nIndex]);

			//Exponential particle decay.
			if (RAND(nDecay) == 0)
				if (--this->parrParticles[nIndex].wDurationLeft == 0)	//display time is over
				{
					this->parrParticles[nIndex].bActive=false;
					continue;
				}
			bActiveParticles = true;

			//Update bounding box of area of effect.
			const int x = ROUND(this->parrParticles[nIndex].x);
			const int y = ROUND(this->parrParticles[nIndex].y);
			const int x2 = x + this->wMaxParticleSize;
			const int y2 = y + this->wMaxParticleSize;
			if (x < this->rAreaOfEffect.x)
				this->rAreaOfEffect.x = x;
			if (y < this->rAreaOfEffect.y)
				this->rAreaOfEffect.y = y;
			if (x2 > static_cast<int>(xMax))
				xMax = x2;
			if (y2 > static_cast<int>(yMax))
				yMax = y2;
		}
	}
	if (bActiveParticles)
	{
		this->rAreaOfEffect.w = xMax - this->rAreaOfEffect.x;
		this->rAreaOfEffect.h = yMax - this->rAreaOfEffect.y;
	}

	this->dwTimeOfLastMove = SDL_GetTicks();

	return bActiveParticles;
}

//*****************************************************************************
inline bool CParticleExplosionEffect::OutOfBounds(const PARTICLE &particle) const
{
	return (particle.x < screenRect.x || particle.y < screenRect.y ||
				particle.x >= screenRect.x + screenRect.w - this->wMaxParticleSize ||
				particle.y >= screenRect.y + screenRect.h - this->wMaxParticleSize);
}

//*****************************************************************************
inline bool CParticleExplosionEffect::HitsObstacle(const PARTICLE &particle) const
//O-square obstacle?
{
	const CDbRoom *pRoom = DYN_CAST(CRoomWidget*, CWidget*, this->pOwnerWidget)->GetCurrentGame()->pRoom;
	const UINT wOTileNo = (UINT)(pRoom->pszOSquares[
		((((Sint16)particle.y - screenRect.y) / CBitmapManager::CY_TILE) * pRoom->wRoomCols) +
		(((Sint16)particle.x - screenRect.x) / CBitmapManager::CX_TILE)]);
	switch (wOTileNo) {
		case T_FLOOR:
		case T_DOOR_YO:
		case T_PIT:
		case T_CHECKPOINT:
		case T_TRAPDOOR: return false;	//particle can go through these things
		default: return true;
	}
}

//*****************************************************************************
inline void CParticleExplosionEffect::ReflectParticle(PARTICLE &particle) const
{
	particle.x -= particle.mx;	//offset the move just done the other way
	particle.y -= particle.my;

	//Randomly reflect particle trajectory.
	if (RAND(2) == 0)
	{
		particle.mx = -particle.mx;
		particle.x += particle.mx;
	} else {
		particle.my = -particle.my;
		particle.y += particle.my;
	}
}

// $Log: ParticleExplosionEffect.cpp,v $
// Revision 1.22  2003/09/16 20:40:24  mrimer
// Converted int momentums to floats for more realism.
//
// Revision 1.21  2003/07/22 19:00:24  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.20  2003/07/09 21:09:21  mrimer
// Fixed bug: particles going off edge of room widget.
//
// Revision 1.19  2003/07/01 20:28:21  mrimer
// Fixed a bug causing effects created while action is frozen to not appear.
//
// Revision 1.18  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.17  2003/06/21 05:08:51  schik
// Fixed particles not being shown if on an edge tile
//
// Revision 1.16  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.15  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.14  2002/12/22 02:30:48  mrimer
// Moved dwTimeOfLastMove from CParticleExplosionEffect to CEffect.
//
// Revision 1.13  2002/11/18 17:33:57  mrimer
// Converted animation from frame-based to time-based.
//
// Revision 1.12  2002/10/16 22:32:57  mrimer
// Revised room boundary cases (add max particle dimensions).
//
// Revision 1.11  2002/10/14 20:57:03  mrimer
// Fixed bug: stray pixels left behind with CDebrisEffect.
//
// Revision 1.10  2002/10/10 21:15:20  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.9  2002/09/13 22:26:46  mrimer
// Fixed bug: particles not drawing when originating at right and bottom edges of room.
//
// Revision 1.8  2002/06/21 05:18:35  mrimer
// Revised includes.
//
// Revision 1.7  2002/05/16 23:06:12  mrimer
// Slowed the particle explosions down.
//
// Revision 1.6  2002/05/16 18:20:44  mrimer
// Modified constructor to allow specification of quantity of particles in BloodEffect.
//
// Revision 1.5  2002/05/16 16:59:09  mrimer
// Fixed particle explosion skew.
//
// Revision 1.4  2002/05/14 19:06:16  mrimer
// Added monster animation.
//
// Revision 1.3  2002/04/29 00:15:22  erikh2000
// Revised #includes.
//
// Revision 1.2  2002/04/25 19:04:17  mrimer
// Refined bounds check to keep particles from blitting out of the room bounds.
//
// Revision 1.1  2002/04/22 21:44:07  mrimer
// Added base class to handle all particle explosions.
//
