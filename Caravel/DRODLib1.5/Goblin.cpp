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
 *
 * ***** END LICENSE BLOCK ***** */

//Goblin.cpp
//Implementation of CGoblin.

#include "Goblin.h"

#include <math.h>

//
//Public methods.
//

//*****************************************************************************************
void CGoblin::Process(
//Process a goblin for movement.
//
//Params:
	const int nLastCommand,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	int dxFirst, dyFirst, dx, dy;

	if (this->pCurrentGame->pRoom->IsBrainPresent() || CanFindSwordsman())
		GetNormalMovement(dxFirst, dyFirst, dx, dy);
	else
		return;

	//Move goblin to new destination square.
	MakeStandardMove(CueEvents,dx,dy);

	SetOrientation(dxFirst, dyFirst);
}

//
//Private methods.
//

//*****************************************************************************************
float CGoblin::DistanceToSwordsman(
// Figures out distance to the swordsman from a square.
//
//Params:
	const UINT x, const UINT y)	//(in)  Coords to check
const
{
 	if (this->pCurrentGame->pRoom->IsBrainPresent())
	{
		SQUARE square;
		this->pCurrentGame->pRoom->pPathMap[this->eMovement]->GetSquare(this->wX,
				this->wY, square);
		if (square.eState == ok && square.wTargetDist > 2)
		{
			//Brain-directed goblin movement.
			this->pCurrentGame->pRoom->pPathMap[this->eMovement]->GetSquare(
					x, y, square);
			//Discourage diagonal movements.
			const bool diagonal = ((this->wX - x) && (this->wY - y));
			return (float)(square.wTargetDist * 2 + (diagonal ? 0.5 : 0));
		}
	}

	//Calculate Euclidean distance.
	const int xd = x - this->pCurrentGame->swordsman.wX;
	const int yd = y - this->pCurrentGame->swordsman.wY;
	return (float) sqrt (xd * xd + yd * yd);
}

//*****************************************************************************************
void CGoblin::GetNormalMovement(
// Figures out the normal movement of the goblin when not affected by a brain.
//
//Params:
	int &dxFirst,	//(out) Horizontal delta for where the monster would
					//		go if there weren't obstacles.
	int &dyFirst,	//(out) Vertical delta for same.
	int &dx,		//(out) Horizontal delta (-1, 0, or 1) for where monster
					//		can go, taking into account obstacles.
	int &dy)		//(out) Vertical delta (-1, 0, or 1) for same.
const
{
	dx = dy = 0;
	dxFirst = nGetOX(this->wO);
	dyFirst = nGetOY(this->wO);

	DWORD bestscore = 1L;
	//Choose move yielding highest score.
	for (int dir = 0; dir < ORIENTATION_COUNT; dir++)
	{
		const int ndx = nGetOX(dir);
		const int ndy = nGetOY(dir);
		const int x = this->wX + ndx;
		const int y = this->wY + ndy;
		DWORD score = 0L;	//assume: it can't move here
		if (this->pCurrentGame->pRoom->IsValidColRow(x, y) &&
			(dir == NO_ORIENTATION ||
			 !DoesSquareContainObstacle(x, y) &&
			 !DoesArrowPreventMovement(ndx, ndy)))
		{
			if ((UINT)x == this->pCurrentGame->swordsman.wX &&
					(UINT)y == this->pCurrentGame->swordsman.wY)
				score = 50000L;	//Kill player -- high score
			else if (abs(x - this->pCurrentGame->swordsman.wSwordX) <= 1 &&
					abs(y - this->pCurrentGame->swordsman.wSwordY) <= 1)
				score = 5000L;	//Near sword -- low score
			else
			{
				//Closer is better
				score = 50000L - (DWORD)(100.0f * DistanceToSwordsman(x, y));
				ASSERT(score < 50000L);
			}
		}
		if (score > bestscore)
		{
			bestscore = score;
			dx = ndx;
			dy = ndy;
		}
	}
	if (dx || dy)
	{
		dxFirst = dx;
		dyFirst = dy;
	}
}


// $Log: Goblin.cpp,v $
// Revision 1.1  2003/02/25 00:01:34  erikh2000
// Initial check-in.
//
// Revision 1.21  2002/12/22 01:53:07  mrimer
// Revised swordsman vars.
//
// Revision 1.20  2002/11/23 00:13:29  mrimer
// Fixed some bugs.
//
// Revision 1.19  2002/11/22 02:00:03  mrimer
// Revised calls to CPathMap.
//
// Revision 1.18  2002/11/18 18:38:59  mrimer
// Changed pathmap calling convention.
//
// Revision 1.17  2002/10/21 20:11:51  mrimer
// Fixed goblin behavior.
//
// Revision 1.16  2002/10/04 18:56:01  mrimer
// Added this-> to member vars.
//
// Revision 1.15  2002/10/03 22:49:49  erikh2000
// Fixed bug with move scoring that causes different movement between release and debug builds.
//
// Revision 1.14  2002/09/19 16:24:53  mrimer
// Made some parameters const.  Added some documentation.
//
// Revision 1.13  2002/09/13 22:41:46  mrimer
// Refactored general monster code into base class.
//
// Revision 1.12  2002/08/30 21:45:24  mrimer
// Fixed potential movement bug.
//
// Revision 1.11  2002/08/29 22:00:38  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.10  2002/06/21 04:37:06  mrimer
// Revised includes.
// Added movement only on non-zero delta.
// Added pathmap specification.
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
// Revision 1.4  2001/11/18 05:02:21  md5i
// Goblin with pathmap fix.
//
// Revision 1.3  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.2  2001/11/14 00:52:12  md5i
// Goblins added.
//
