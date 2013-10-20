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
 * Michael Welsh Duggan (md5i), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//RoachQueen.cpp
//Implementation of CRoach.

#include "RoachQueen.h"

//
//Public methods.
//

//*****************************************************************************************
void CRoachQueen::Process(
//Process a roach queen for movement.
//
//Params:
	const int nLastCommand,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	//Get movement offsets.
	int dxFirst, dyFirst, dx, dy;
	if (this->pCurrentGame->pRoom->IsBrainPresent())
		GetBrainDirectedMovement(dxFirst, dyFirst, dx, dy);
	else
	{
		if (CanFindSwordsman())
		{
			// Run away!
			dx = dxFirst = -sgn(this->pCurrentGame->swordsman.wX - this->wX);
			dy = dyFirst = -sgn(this->pCurrentGame->swordsman.wY - this->wY);
			GetBestMove(dx, dy);
		}
		else return;	//no change -- and don't lay eggs
	}

	// Save previous location
	const UINT oX = this->wX;
	const UINT oY = this->wY;

	//Move roach queen to new destination square.
	MakeStandardMove(CueEvents,dx,dy);
	SetOrientation(dxFirst, dyFirst);
		
	//Shall we lay some eggs?
	//The criteria for laying an egg in a square should be:
	//1. Square does not contain a monster (including mimic).
	//2. Square does not contain a swordsman or sword.
	//3. Square does not contain a mimic sword.
	//4. T-square is T_EMPTY.
	//5. O-square is T_FLOOR.
	if ((this->pCurrentGame->wSpawnCycleCount % TURNS_PER_CYCLE == 0) &&
		CanFindSwordsman())
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				const UINT ex = this->wX + x;
				const UINT ey = this->wY + y;
				if (//Valid
					this->pCurrentGame->pRoom->IsValidColRow(ex, ey) &&
					// Not current queen position
					!(ex == this->wX && ey == this->wY) &&
					// Not old queen position (compat)
					!(ex == oX && ey == oY) &&
					// Not swordsman
					!(ex == this->pCurrentGame->swordsman.wX && 
					  ey == this->pCurrentGame->swordsman.wY) &&
					// Not monster or mimic or sword
					!DoesSquareContainObstacle(ex, ey) &&
					//And t-square is empty (compat).
					this->pCurrentGame->pRoom->GetTSquare(ex, ey) == T_EMPTY &&
					 //And o-square is floor (compat).                                   
					this->pCurrentGame->pRoom->GetOSquare(ex, ey) == T_FLOOR		
					)
				{
					// Place an egg
					CMonster *m = this->pCurrentGame->pRoom->AddNewMonster(M_REGG,
							ex,ey);
					m->SetCurrentGame(this->pCurrentGame);
					m->bIsFirstTurn = true;
				}
			}
		}
	}
}
//******************************************************************************************
void CRoachQueen::GetBrainDirectedMovement(
// Special-cased for roach queens.
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
	SQUARE square;
	this->pCurrentGame->pRoom->pPathMap[this->eMovement]->GetSquare(this->wX,
			this->wY, square);
	if (square.eState == ok && square.eDirection != none)
	{
		//Run away from swordsman, based on the direction of the pathmap.
		dx = -nGetOX(square.eDirection);
		dy = -nGetOY(square.eDirection);
		//Try to move diagonally toward middle of the room on non-primary axis.
		if (dx == 0)
			dx = (wX < (this->pCurrentGame->pRoom->wRoomCols / 2)) 
				 ? 1: -1;
		else if (dy == 0)
			dy = (wY < (this->pCurrentGame->pRoom->wRoomRows / 2))
				 ? 1: -1;
		dxFirst = dx;
		dyFirst = dy;
	} else {
		//With a brain, even invisible swordsman is sensed.
		dx = dxFirst = -sgn(this->pCurrentGame->swordsman.wX - this->wX);
		dy = dyFirst = -sgn(this->pCurrentGame->swordsman.wY - this->wY);
	}
	GetBestMove(dx, dy);
}

// $Log: RoachQueen.cpp,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.22  2002/12/22 01:08:02  mrimer
// Revised swordsman vars.
//
// Revision 1.21  2002/11/22 01:58:04  mrimer
// Cleaned up logic.  Fixed a potential bug (with invisible player).
//
// Revision 1.20  2002/11/18 18:38:38  mrimer
// Changed pathmap calling convention.
//
// Revision 1.19  2002/09/19 17:51:08  mrimer
// Added player invisibility rule for roach spawning.
//
// Revision 1.18  2002/09/13 22:41:46  mrimer
// Refactored general monster code into base class.
//
// Revision 1.17  2002/09/01 00:04:55  erikh2000
// Changed references to "wTurnsTaken" to "wSpawnCycleCount".
//
// Revision 1.16  2002/08/29 22:00:40  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.15  2002/06/21 04:46:58  mrimer
// Revised includes.
// Added movement only on non-zero delta.
// Other minor revisions.
//
// Revision 1.14  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.13  2002/03/28 21:56:42  erikh2000
// Fixed egg-laying rules for compatibility with Webfoot DROD.  (Committed on behalf of mrimer.)
//
// Revision 1.12  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.11  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.10  2002/02/20 16:27:59  md5i
// Reversed loop variabled for egg laying. (Fixes task 48155)
//
// Revision 1.9  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.8  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.7  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.6  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.5  2001/11/12 02:57:56  erikh2000
// Changed roach-laying conditions to fix 1.11c compatibility problem.
//
// Revision 1.4  2001/10/27 20:26:37  erikh2000
// Removed CueEvents.Clear calls and fixed some kind of EOL translation causing double-spacing.
//
// Revision 1.3  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.2  2001/10/24 02:18:13  md5i
// Fixed egg tiles.  Used bIsFirstTurn status.
//
// Revision 1.1  2001/10/24 01:41:38  md5i
// Added Roach Queens and Eggs.
//
