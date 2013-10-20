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

#ifndef DRODEFFECT_H
#define DRODEFFECT_H

#include <FrontEndLib/Effect.h>

//Set of all DROD effects that need to be kept track of (for deleting early, for example).
//Add other effects here as needed.
enum EffectType
{
   EGENERIC=EFFECTLIB::EGENERIC,			//generic effect
   ESHADE=EFFECTLIB::ESHADE,				//square highlighting
   ETRANSTILE=EFFECTLIB::ETRANSTILE,		//transparent tiles
   EFRAMERATE=EFFECTLIB::EFRAMERATE,		//frame rate effect
   ETOOLTIP=EFFECTLIB::ETOOLTIP,			//tool tip
   EBUMPOBSTACLE=EFFECTLIB::EBUMPOBSTACLE,	//player bumps into obstacle
   ENEATHERHITORB,	//'Neather strikes an orb
   EPENDINGPLOT,		//selecting room area for plot in editor
   EORBHIT           //strike orb effect
};

#endif //...#ifndef DRODEFFECT_H

// $Log: DrodEffect.h,v $
// Revision 1.1  2003/08/05 01:38:53  mrimer
// Moved DROD-specific effects from FrontEndLib.
//
