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

//SUMMARY
//
//CEffect is an abstract base class for graphical effects drawn on top of widgets.
//Effects are temporary animations drawn by the owner widget.  The screen surface 
//update is performed by the owner widget.
//
//USAGE
//
//The widget must have code to add, draw, and clear effects before an effect 
//can be used with it.  In CRoomWidget, a call to AddTLayerEffect() or 
//AddLastLayerEffect() is all that is needed for room widget to handle the effect.
//
//DESIGNING AN EFFECT
//
//1. Create a new class derived from CEffect.  
//2. Your constructor should take params for everything needed to draw the effect 
//   over its lifetime.  For speed, try to perform calculations here instead
//   of in Draw().  In other words, it is better to perform a calc one time in
//   the constructor, instead of every time Draw() is called from the i/o loop.
//   Avoid keeping pointers passed from cue event private data or current game--
//   they can become invalid during the lifetime of your effect.
//3. Override the Draw() pure virtual method in your class.  Draw() should do
//   the following:
//		a. Return false without drawing if the timespan that the effect can 
//			be drawn in has past.  For example if you have an explosion that lasts
//			60 frames and GetFrameNo() returns 62, then the effect is no longer
//			active and you should return false.
//		b. Decide what to draw based on the # of frames since construction
//			returned from GetFrameNo(), the class members set by the constructor,
//			and data from the owner widget.
//		c. Draw to the screen surface.
//		d. Specify the bounding box the effect covers (in absolute coordinates).
//		e. Return true to specify the effect is still continuing.
//			(If the effect has completed, return false before drawing anything.)

#ifndef EFFECT_H
#define EFFECT_H

#include "Widget.h"
#include <BackEndLib/Types.h>

//****************************************************************************************

//Generic effects needed, but can't place them in a final enumeration yet (i.e.,
//there will probably be more effects used in the app).
//Give these values to the corresponding names in the final enumeration.
namespace EFFECTLIB
{
   enum EFFECTTYPE
   {
	   EGENERIC=0,			//generic effect
	   ESHADE,				//square highlighting
	   ETRANSTILE,			//transparent tiles
	   EFRAMERATE,			//frame rate effect
	   ETOOLTIP,			//tool tip
	   EBUMPOBSTACLE,		//something bumps into obstacle
      EFLASHSHADE       //flashing square highlighting
   };
};

//****************************************************************************************
class CEffectList;
class CEffect
{
friend class CEffectList;	//to access dwTimeStarted

public:
   CEffect(CWidget *pSetOwnerWidget, const UINT eType=EFFECTLIB::EGENERIC);
	virtual ~CEffect() { }

	virtual bool	Draw(SDL_Surface* pDestSurface=NULL) = 0;
	virtual long	GetDrawSequence() const {return 0L;}
	UINT           GetEffectType() const {return this->eEffectType;}
	const SDL_Rect GetAreaOfEffect() const {return this->rAreaOfEffect;}
	
protected:
	UINT			GetFrameNo() const {return (SDL_GetTicks() - this->dwTimeStarted) / 33;}
	SDL_Surface *	GetDestSurface() {return this->pOwnerWidget->GetDestSurface();}

	CWidget *		pOwnerWidget;

	SDL_Rect			rAreaOfEffect;	//bounding box covered by effect this frame
	Uint32			dwTimeStarted;
	Uint32			dwTimeOfLastMove;	//used by CParticleExplosionEffect

	UINT		      eEffectType;   //can't finalize as enumeration yet
};

#endif //...#ifndef EFFECT_H

// $Log: Effect.h,v $
// Revision 1.6  2003/08/08 17:35:01  mrimer
// Added FlashShadeEffect.  Some code maintenance.
//
// Revision 1.5  2003/08/05 01:36:14  mrimer
// Moved DROD-specific effects out of FrontEndLib.
//
// Revision 1.4  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.3  2003/06/16 21:55:09  mrimer
// Added EORBHIT effect type.
//
// Revision 1.2  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.15  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.14  2002/12/22 02:23:05  mrimer
// Moved dwTimeOfLastMove from CParticleExplosionEffect to CEffect.
//
// Revision 1.13  2002/11/22 02:17:53  mrimer
// Added ETOOLTIP.
//
// Revision 1.12  2002/11/18 18:34:04  mrimer
// Added support for time-based, not frame-based, animation.
//
// Revision 1.11  2002/11/15 02:46:05  mrimer
// Added more effect types.
//
// Revision 1.10  2002/10/11 15:28:27  mrimer
// Modified a comment.
//
// Revision 1.9  2002/10/10 21:09:48  mrimer
// Added rAreaOfEffect and GetAreaOfEffect().
//
// Revision 1.8  2002/10/03 19:01:50  mrimer
// Made CEffectList a friend.
//
// Revision 1.7  2002/09/14 21:39:38  mrimer
// Added EBUMPOBSTACLE tag.
//
// Revision 1.6  2002/09/14 21:20:15  mrimer
// Added EffectType tag.
//
// Revision 1.5  2002/08/25 18:59:05  erikh2000
// Added method that determines order that effects are drawn in.
//
// Revision 1.4  2002/07/22 00:57:25  erikh2000
// Made destructor virtual.
//
// Revision 1.3  2002/06/21 04:53:28  mrimer
// Revised includes.
//
// Revision 1.2  2002/06/11 22:41:37  mrimer
// Changed GetScreenSurface to GetDestSurface.
//
// Revision 1.1  2002/04/12 05:15:10  erikh2000
// Initial check-in.
//
