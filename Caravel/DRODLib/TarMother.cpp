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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Michael Welsh Duggan (md5i)
 *
 * ***** END LICENSE BLOCK ***** */

//TarMother.cpp
//Implementation of CTarMother.

#include "TarMother.h"

//
//Public methods.
//

//*****************************************************************************
void CTarMother::Process(
//Process a tar mother for movement.
//
//Params:
	const int /*nLastCommand*/,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	if (!this->bEyeSet)
	{
		//Determine whether this is a left or right eye.
		this->bLeftEye = (this->wO == NO_ORIENTATION || this->wO == W);
		this->bEyeSet = true;
	}

	//Reverse eyes if either eye in pair is scared (Beethro's sword is close).
	static const UINT DIST = 1;
	const UINT wMinX = this->wX - (this->bLeftEye ? DIST : DIST+1);
	const UINT wMaxX = this->wX + (this->bLeftEye ? DIST+1 : DIST);
	const bool bScared = this->pCurrentGame->pRoom->IsSwordWithinRect(
			wMinX,this->wY - DIST,wMaxX,this->wY + DIST);

	if (pCurrentGame->wSpawnCycleCount % TURNS_PER_CYCLE == TURNS_PER_CYCLE-1)
		this->wO = (this->bLeftEye != bScared ? W : SW);	//closed eye
	else
		this->wO = (this->bLeftEye != bScared ? NO_ORIENTATION : S);	//open eye

	//Grow tar if swordsman is sensed.
	if ( this->pCurrentGame->wSpawnCycleCount % TURNS_PER_CYCLE == 0 && 
         !CueEvents.HasOccurred(CID_TarGrew) &&
		   (CanFindSwordsman() || this->pCurrentGame->bBrainSensesSwordsman) )        
      CueEvents.Add(CID_TarGrew);
}

// $Log: TarMother.cpp,v $
// Revision 1.19  2003/08/13 21:17:39  mrimer
// Fixed tar mother to spawn one move later the first time, making it every 30th move.
// Extended the roach egg hatching cycle by one turn, from four turns to five.
//
// Revision 1.18  2003/08/06 15:44:32  erikh2000
// Will now spawn if invisible Beethro is in sight of brain.
//
// Revision 1.17  2003/08/05 15:43:51  erikh2000
// Tar mothers won't spawn when tar babies see Beethro anymore.
//
// Revision 1.16  2003/06/19 01:53:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.15  2002/11/15 01:28:46  mrimer
// Added logic to show left and right eyes, and whether tar mother is scared.
//
// Revision 1.14  2002/09/19 17:47:30  mrimer
// Added player invisibility rule for tar growth.
//
// Revision 1.13  2002/09/01 00:04:55  erikh2000
// Changed references to "wTurnsTaken" to "wSpawnCycleCount".
//
// Revision 1.12  2002/08/29 22:00:40  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.11  2002/07/17 20:38:10  erikh2000
// Removed commented out code.
//
// Revision 1.10  2002/06/21 04:50:56  mrimer
// Revised includes.
// Added TURNS_PER_CYCLE.
//
// Revision 1.9  2002/05/14 17:22:50  mrimer
// Now basing particle explosion direction on sword movement (not orientation).
// Changed tar babies' stab appearance to TarStabEffect.
//
// Revision 1.8  2002/05/14 15:54:33  mrimer
// Fixed bug: Tar baby formed from tar mother stab.
//
// Revision 1.7  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.6  2002/03/14 03:52:20  erikh2000
// Replaced T_EMPTY plot in OnStabbed() with call to new CDbRoom::RemoveStabbedTar().
//
// Revision 1.5  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.4  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.3  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.2  2001/11/13 05:35:54  md5i
// Added TarMother and growing tar.
//
