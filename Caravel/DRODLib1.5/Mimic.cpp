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
 *
 * ***** END LICENSE BLOCK ***** */

#include "Mimic.h"

//
//Public methods.
//

//*****************************************************************************************
CMimic::CMimic(
//Constructor mimic.
//
//Params:
	CCurrentGame *pSetCurrentGame) 	//(in)	If NULL (default) then 
									//		class can only be used for 
									//		accessing data, and not 
									//		for game processing.
: CMonster(M_MIMIC, pSetCurrentGame, GROUND, 100)
{ }

//*****************************************************************************************
void CMimic::Process(
//Process a mimic for movement.
//
//Params:
	const int nLastCommand,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	int dx=0, dy=0;

	switch (nLastCommand)
	{
		case CMD_C:
		{
			this->wO = nNextCO(this->wO);
			break;
		}
		case CMD_CC:
		{
			this->wO = nNextCCO(this->wO);
			break;
		}
		case CMD_NW: dx = dy = -1; break;
		case CMD_N: dy = -1; break;
		case CMD_NE: dx = 1; dy = -1; break;
		case CMD_W: dx = -1; break;
		case CMD_E: dx = 1; break;
		case CMD_SW: dx = -1; dy = 1; break;
		case CMD_S: dx = 0; dy = 1; break;
		case CMD_SE: dx = dy = 1; break;

	}

	// Did the nLastCommand successfully move the swordsman.
	// If not, then set dx/dy to zero so mimics don't move
	if (this->pCurrentGame->swordsman.wX==this->pCurrentGame->swordsman.wPrevX &&
		this->pCurrentGame->swordsman.wY==this->pCurrentGame->swordsman.wPrevY)
	{
		dx=0; dy=0;
	} else {
		GetBestMove(dx, dy);

		//Check for obstacles in destination square.
		if (dx || dy)
		{
			const UINT wMoveO = nGetO(dx, dy);

			//Before he moves, remember important square contents.
			const bool bWasOnTrapdoor = (this->pCurrentGame->pRoom->GetOSquare(
					this->wX, this->wY)==T_TRAPDOOR);

			//Move mimic to new destination square.
			Move(this->wX + dx, this->wY + dy);
			ASSERT(this->wO == this->pCurrentGame->swordsman.wO);
			ASSERT(IsValidOrientation(this->wO));

			//Check for movement off of a trapdoor.
			ASSERT(dx || dy);
			if (bWasOnTrapdoor)
			{
				this->pCurrentGame->pRoom->DestroyTrapdoor(this->wX - dx,
						this->wY - dy, CueEvents);
			}
		}
	}
}

//******************************************************************************************
bool CMimic::DoesSquareContainObstacle(
//Override for mimics.
//
//Params:
	const UINT wCol, const UINT wRow) //(in) Coords of square to evaluate.  Must be valid.
const
{
	//Most of the checks done in base method.
	if (CMonster::DoesSquareContainObstacle(wCol, wRow)) return true;

	//Check for swordsman.
	return (wCol == this->pCurrentGame->swordsman.wX &&
			wRow == this->pCurrentGame->swordsman.wY);
}

//******************************************************************************************
bool CMimic::IsTileObstacle(
//Override for mimics.
//
//Params:
	const UINT wLookTileNo)	//(in)	Tile to evaluate.  Note each tile# will always be
						//		found on the same layer of squares.
//
//Returns:
//True if tile is an obstacle, false if not.
const
{
	return (
			//All the things a mimic can step onto.
			wLookTileNo!=T_POTION_I &&
			wLookTileNo!=T_POTION_K &&
			CMonster::IsTileObstacle(wLookTileNo)
			);
}

// $Log: Mimic.cpp,v $
// Revision 1.1  2003/02/25 00:01:35  erikh2000
// Initial check-in.
//
// Revision 1.22  2002/12/22 01:52:45  mrimer
// Revised swordsman vars.  Cleaned up movement logic.
//
// Revision 1.21  2002/11/15 01:34:37  mrimer
// Made some parameters const.
//
// Revision 1.20  2002/09/19 17:50:18  mrimer
// Removed old commented-out code.
//
// Revision 1.19  2002/08/29 22:00:39  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.18  2002/08/28 04:30:33  erikh2000
// Fixed bug with mimic stepping onto swordsman.
//
// Revision 1.17  2002/07/05 10:43:47  erikh2000
// Fixed a few bugs involving mimics not treating the swordsman as an obstacle.
//
// Revision 1.16  2002/06/22 05:45:06  erikh2000
// Replaced wSwordX and wSwordY members with GetSwordX() and GetSwordY() methods to fix a problem.
//
// Revision 1.15  2002/06/21 04:39:03  mrimer
// Revised includes.
// Condensed obstacle check code.
//
// Revision 1.14  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.13  2002/03/04 22:23:47  erikh2000
// Changed obstacle logic to evaluate T_CHECKPOINT as clear.
//
// Revision 1.12  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.11  2002/02/24 03:45:39  erikh2000
// Removed sword check from Process().
// Removed CheckSwordHit().
// Added IsTileObstacle() override since mimics can step on potions.
//
// Revision 1.10  2002/02/08 23:18:37  erikh2000
// Changed CDbRoom::DeleteMonster() call to KillMonster().
//
// Revision 1.9  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.8  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.7  2001/11/19 09:21:50  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
// Removed some duplicate code from CMimic constructor.
// Added code in CheckSwordHit() to make mimics able to kill the swordsman.
//
// Revision 1.6  2001/11/18 04:10:37  md5i
// A few mimic fixes.
//
// Revision 1.5  2001/11/17 23:08:27  erikh2000
// Wrote code to make mimics work.  (Committed on behalf of j_wicks.)
//
// Revision 1.4  2001/10/20 05:46:23  erikh2000
// Removed dead code.
//
// Revision 1.3  2001/10/16 00:02:31  erikh2000
// Wrote code to display current room and handle basic swordsman movement.
//
// Revision 1.2  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.1.1.1  2001/10/01 22:20:17  erikh2000
// Initial check-in.
//
