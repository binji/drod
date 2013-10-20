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
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996, 
 * 1997, 2000, 2001, 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Michael Welsh Duggan (md5i)
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//WraithWing.cpp
//Implementation of CWraithWing.

#include "Wraithwing.h"

//
//Public methods.
//

//*****************************************************************************************
void CWraithWing::Process(
//Process a wraith-wing for movement.
//
//Params:
	const int /*nLastCommand*/,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	int dxFirst, dyFirst, dx, dy;

	const int distance = nDist(this->wX, this->wY, 
			     this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY);

	// Get directed movement toward swordsman
	if (!GetDirectMovement(dxFirst, dyFirst, dx, dy))
		return;

	//Wraith-wings are cowards and only attack in numbers.
	if (distance <= 5)
	{
		// Let's look for another wraith-wing, ready to pounce.
		bool runaway = true;
		for (CMonster *pSeek = this->pCurrentGame->pRoom->pFirstMonster;
		     pSeek != NULL; pSeek = pSeek->pNext)
		{
			if (pSeek->wType != M_WWING ||
			    (pSeek->wX == this->wX && pSeek->wY == this->wY))
				continue;
			const int dist2 = nDist(pSeek->wX, pSeek->wY, 
					this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY);
			//  We want a wraith-wing who is within 2 of the same
			//  distance we are from the swordsman, but who is at least
			//  3 away from us.  (Based on old code.)
			if (abs (dist2 - distance) < 3 &&
			    nDist(pSeek->wX, pSeek->wY, this->wX, this->wY) > 2)
			{
				// Another wraith-wing is able to collaborate
				runaway = false;
				break;
			}
		}
		if (runaway)
			if (distance == 5) 
				dx = dy = 0;	// Don't get involved
			else
			{
				// Flee!
				dxFirst = dx = -dxFirst;
				dyFirst = dy = -dyFirst;
				GetBestMove(dx, dy);
			}
	}

	//Move wraith-wing to new destination square.
	MakeStandardMove(CueEvents,dx,dy);
	SetOrientation(dxFirst, dyFirst);
}

// $Log: Wraithwing.cpp,v $
// Revision 1.15  2003/06/19 01:53:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.14  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.13  2002/12/22 01:49:41  mrimer
// Revised swordsman vars.
//
// Revision 1.12  2002/09/13 22:41:46  mrimer
// Refactored general monster code into base class.
//
// Revision 1.11  2002/08/29 22:00:40  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.10  2002/06/21 04:51:48  mrimer
// Revised includes.
// Added brain-directed movement.
// Added movement only on non-zero delta.
//
// Revision 1.9  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.8  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.7  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.6  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.5  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.4  2001/10/27 20:26:37  erikh2000
// Removed CueEvents.Clear calls and fixed some kind of EOL translation causing double-spacing.
//
// Revision 1.3  2001/10/26 00:49:55  md5i
// Wraith-wings added.
//
// Revision 1.2  2001/10/20 05:42:43  erikh2000
// Removed tile image references.
//
// Revision 1.1.1.1  2001/10/01 22:20:33  erikh2000
// Initial check-in.
//
