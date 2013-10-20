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
 * Michael Welsh Duggan (md5i), John Wm. Wicks (j_wicks), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//Monster.cpp
//Implementation of CMonster.

#include "Monster.h"
#include "MonsterFactory.h"
#include "Mimic.h"

//
//CMonster methods.
//

//*****************************************************************************
CMonster::CMonster(
   const UINT wSetType, const CCurrentGame *pSetCurrentGame,
   const MovementType eMovement,
	const UINT wSetProcessSequence)
	: CAttachableObject()
   , wType(wSetType)
	, wX(0), wY(0), wO(0)
   , wPrevX(0), wPrevY(0)
	, wProcessSequence(wSetProcessSequence)
	, bIsFirstTurn(false)
	, eMovement(eMovement)
	, pNext(NULL), pPrevious(NULL)
	, pCurrentGame(pSetCurrentGame)
{	}

//*****************************************************************************
CMonster::~CMonster() 
{
	Clear();
}

//*****************************************************************************
void CMonster::Clear()
//Frees resources and zeroes members.
{	
	this->pPrevious=this->pNext=NULL;
	this->wType=this->wX=this->wY=this->wO=this->wProcessSequence=0;
   this->wPrevX = this->wPrevY = 0;
	this->bIsFirstTurn=false;
	this->ExtraVars.Clear();
}

//*****************************************************************************
void CMonster::ResetCurrentGame()
//Resets the current game pointer.  (Called only from the room editor.)
{
	this->pCurrentGame = NULL;
}

//*****************************************************************************
void CMonster::SetCurrentGame(
//Sets current game pointer for monster.  This is necessary for many methods of 
//the monster class to work.
//
//Params:
	const CCurrentGame *pSetCurrentGame) //(in)
{
	ASSERT(pSetCurrentGame);
	ASSERT(!this->pCurrentGame);
	this->pCurrentGame = pSetCurrentGame;
}

//*****************************************************************************
void CMonster::SetOrientation(
//Sets monster orientation based on direction vectors.
//
//Params:
	const int& dxFirst, const int& dyFirst)	//(in) Direction to face.
{
   ASSERT(abs((int)dxFirst) <= 1);
	ASSERT(abs((int)dyFirst) <= 1);
   this->wO = nGetO(dxFirst, dyFirst);
	ASSERT(IsValidOrientation(this->wO));
}

//*****************************************************************************
void CMonster::Process(
//Overridable method to process a monster for movement after swordsman moves.  If derived
//class uses this method, then the monster will do nothing.
//
//Params:
	const int /*nLastCommand*/,	//(in) Last swordsman command.
	CCueEvents &/*CueEvents*/)	//(in) Accepts pointer to an IDList object that will be populated
							//codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	//Monster does nothing and nothing happens.
}

//*****************************************************************************
bool CMonster::OnStabbed(
//Overridable method to process the effects of a monster being stabbed.  If derived class
//uses this method, then a cue event of monster being killed will be returned.
//
//Params:
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
//
//Returns:
//True if any cue events were returned, false if not.
{
	//Monster dies.
	CueEvents.Add(CID_MonsterDiedFromStab, this);
	return true;
}

//*****************************************************************************
bool CMonster::DoesArrowPreventMovement(
//Does an arrow prevent movement of this monster to a destination square?
//The current square and destination square are both checked for arrows.
//
//Params:
	const int dx, const int dy)	//(in)	Offsets that indicate direction
											//		of movement from current square.
//
//Returns:
//True if an arrow prevents movement, false if not.
const
{
	//Check for obstacle arrow in current square.
	const int nO = nGetO(sgn(dx), sgn(dy));	//allow for dx/dy > 1
	UINT wLTileNo = this->pCurrentGame->pRoom->GetTSquare(this->wX, this->wY);
	if (bIsArrow(wLTileNo))
	{
		//Is it an obstacle.
		if (bIsArrowObstacle(wLTileNo, nO)) return true;
	}

	//Check for obstacle arrow in destination square.
	wLTileNo = this->pCurrentGame->pRoom->GetTSquare(this->wX + dx, 
			this->wY + dy);
	if (!bIsArrow(wLTileNo)) return false;
	
	//Check if arrow in destination square is an obstacle.
	return bIsArrowObstacle(wLTileNo, nO);
}

//*****************************************************************************
bool CMonster::DoesSquareContainObstacle(
//Determines if a square contains an obstacle for this monster.
//
//Params:
	const UINT wCol, const UINT wRow) //(in) Coords of square to evaluate.
const
{
	//Routine is not written to check the square on which this monster is 
	//standing.
	ASSERT(wCol != this->wX || wRow != this->wY);

	//Check t-square for obstacle.
	CDbRoom *pRoom = this->pCurrentGame->pRoom;
	if (!pRoom->IsValidColRow(wCol,wRow))
	{
		ASSERTP(false, "(wCol,wRow) not valid.");
		return true;
	}
	UINT wLookTileNo = pRoom->GetTSquare(wCol, wRow);
	if (wLookTileNo != T_EMPTY)
	{
		if (IsTileObstacle(wLookTileNo)) return true;
	}

	//Check o-square obstacle.
	wLookTileNo = pRoom->GetOSquare(wCol, wRow);
	if (IsTileObstacle(wLookTileNo)) return true;

	//Check for monster at square.
	if (pRoom->GetMonsterAtSquare(wCol, wRow)) return true;

	//Check for sword at square.
	if (wCol == this->pCurrentGame->swordsman.wSwordX && 
			wRow == this->pCurrentGame->swordsman.wSwordY) return true;

	//Check for mimic sword at square.
	//Note difference from CDbRoom::DoesSquareContainMimicSword().
	CMonster *pMonster = this->pCurrentGame->pRoom->pFirstMonster;
	while (pMonster)
	{
		if (pMonster->wType == M_MIMIC && 
				pMonster != this) //Because it's okay for mimics to walk into their own sword square.
		{
			CMimic *pMimic = (CMimic*) pMonster;
			if (wCol == pMimic->GetSwordX() && wRow == pMimic->GetSwordY())
				return true;
		}
		pMonster = pMonster->pNext;
	}

	//No obstacle.
	return false;
}

//*****************************************************************************
bool CMonster::IsSwordsmanWithin(
//Returns: whether swordsman is within this many squares of monster.
//
//Params:
	const UINT wSquares)
const
{
	return nDist(this->wX, this->wY, this->pCurrentGame->swordsman.wX,
			this->pCurrentGame->swordsman.wY) <= wSquares;
}

//*****************************************************************************
bool CMonster::IsSwordWithin(
//Returns: whether a sword is within this many squares of monster.
//
//Params:
	const UINT wSquares)
const
{
	//Check mimic swords.
	CMonster *pSeek = this->pCurrentGame->pRoom->pFirstMonster;
	while (pSeek)
	{
		if (pSeek->wType == M_MIMIC)
		{
			CMimic *pMimic = DYN_CAST(CMimic*, CMonster*, pSeek);
			if (nDist(this->wX, this->wY, pMimic->GetSwordX(),
					pMimic->GetSwordY()) <= wSquares)
				return true;
		}
		pSeek = pSeek->pNext;
	}

	//Check player's sword.
	return nDist(this->wX, this->wY, this->pCurrentGame->swordsman.wSwordX,
			this->pCurrentGame->swordsman.wSwordY) <= wSquares;
}

//*****************************************************************************
bool CMonster::IsTileObstacle(
//Overridable method to determine if a tile is an obstacle for this monster.
//This method and any overrides, should not evaluate game state or anything
//else besides the tile# to determine if the tile is an obstacle.  If a tile
//is sometimes an obstacle, but not always, IsTileObstacle() should return true,
//and the caller can use extra context to figure it out.  An example of this would
//be arrows, which can be an obstacle to a monster depending on direction of 
//movement.
//
//Params:
	const UINT wLookTileNo)	//(in)	Tile to evaluate.  Note each tile# will always be
						//		found on the same layer of squares.
//
//Returns:
//True if tile is an obstacle, false if not.
const
{
	return ( !(
			
			//All the things a monster can step onto.
			wLookTileNo==T_EMPTY ||
			wLookTileNo==T_FLOOR || 
			wLookTileNo==T_CHECKPOINT ||
			wLookTileNo==T_DOOR_YO || 
			wLookTileNo==T_TRAPDOOR || 
			wLookTileNo==T_SCROLL ||
			bIsArrow(wLookTileNo) ||
			(this->eMovement == AIR && wLookTileNo == T_PIT)

			) );
}

//*****************************************************************************
void CMonster::GetBestMove(
//Given a direction, chooses best possible movement for monster taking
//obstacles into account.
//
//Params:
	int &dx,		//(out) Horizontal delta (-1, 0, or 1) for where monster
					//		can go, taking into account obstacles.
	int &dy)		//(out) Vertical delta (-1, 0, or 1) for same.
const
{
	//Check for out of bounds movement.
	if (this->wX + dx >= this->pCurrentGame->pRoom->wRoomCols ||
	    (int)this->wX + dx < 0) dx = 0;
	if (this->wY + dy >= this->pCurrentGame->pRoom->wRoomRows ||
	    (int)this->wY + dy < 0) dy = 0;
	if (dx == 0 && dy == 0)
	  return;

	//See if directly moving to square will work.
	bool bFoundDir = !(
			DoesSquareContainObstacle(this->wX + dx, this->wY + dy) ||
			DoesArrowPreventMovement(dx, dy));

	//See if moving in just the vertical direction will work.
	if (!bFoundDir && dy)
	{
		bFoundDir = !(
			DoesSquareContainObstacle(this->wX, this->wY + dy) ||
			DoesArrowPreventMovement(0, dy));
		if (bFoundDir) dx = 0;
	}

	//See if moving in just the horizontal direction will work.
	if (!bFoundDir && dx)
	{
		bFoundDir = !(
			DoesSquareContainObstacle(this->wX + dx, this->wY) ||
			DoesArrowPreventMovement(dx, 0));
		if (bFoundDir) dy = 0;
	}
    	
	//If no clear direction has been found, set movement deltas to zero.
	if (!bFoundDir) dx=dy=0;
}

//*****************************************************************************
void CMonster::GetBeelineMovement(
//Gets offsets for a beeline movement of this monster to the swordsman,
//taking obstacles into account.
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
	dx = dxFirst = sgn(this->pCurrentGame->swordsman.wX - this->wX);
	dy = dyFirst = sgn(this->pCurrentGame->swordsman.wY - this->wY);

	GetBestMove(dx, dy);
}

//*****************************************************************************
void CMonster::GetBrainDirectedMovement(
//Gets offsets for a brain-directed movement of this monster to the swordsman,
//taking obstacles into account.
//
//Brain-directed movement is:
//	Follow shortest path to swordsman, even if he is invisible
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
	POINT paths[9];
	UINT num_paths;
	this->pCurrentGame->pRoom->pPathMap[this->eMovement]->
			GetRecPaths(this->wX, this->wY, paths, num_paths);
	for (UINT i = 0; i < num_paths; i++)
		if ((UINT)paths[i].x == this->wX && (UINT)paths[i].y == this->wY)
			break;	//no advantageous brain-directed path found -- beeline
		else
		{
			const int tdx = paths[i].x - this->wX;
			const int tdy = paths[i].y - this->wY;
         ASSERT(abs((int)tdx) <= 1);
			ASSERT(abs((int)tdy) <= 1);
			if (!DoesSquareContainObstacle(paths[i].x, paths[i].y) &&
				!DoesArrowPreventMovement(tdx, tdy))
			{
				dx = dxFirst = tdx;
				dy = dyFirst = tdy;
				return;
			}
		}

	//Default
	GetBeelineMovement(dxFirst, dyFirst, dx, dy);
}

//*****************************************************************************
bool CMonster::GetDirectMovement(
//Gets offsets for standard monster movement to the swordsman, taking
//obstacles into account.
//
//Returns: true if some movement was planned, else false
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
	if (this->pCurrentGame->bBrainSensesSwordsman)
		GetBrainDirectedMovement(dxFirst, dyFirst, dx, dy);
	else if (CanFindSwordsman()) 
		GetBeelineMovement(dxFirst, dyFirst, dx, dy);
	else
		return false;

	return true;
}

//*****************************************************************************
bool CMonster::IsOnSwordsman(void)
//Is the monster in the same square as the swordsman?
const
{
	return (this->wX == this->pCurrentGame->swordsman.wX &&
			this->wY == this->pCurrentGame->swordsman.wY);
}

//*****************************************************************************
bool CMonster::CanFindSwordsman(void)
//Overridable method for determining if a monster can find the swordsman.  
//Currently used by movement routines to see if monster will attempt to move
//towards the swordsman or not.
//
//Returns:
//True if monster can find the swordsman, false if not.
const
{
	//If swordsman is visible, monster can see him.
	if (this->pCurrentGame->swordsman.bIsVisible) return true;

	//Otherwise, monster can smell him if within range.
	return CanSmellObjectAt(this->pCurrentGame->swordsman.wX,
			this->pCurrentGame->swordsman.wY);
}

//*****************************************************************************
bool CMonster::CanSmellObjectAt(
//Returns: whether (wX,wY) is within smelling range.
//
//Params:
	const UINT wX,	const UINT wY)	//(in) Coordinates of object.
const
{
	 return (nDist(wX, wY, this->wX, this->wY) <= DEFAULT_SMELL_RANGE);
}

//*****************************************************************************
bool CMonster::OnAnswer(
//Overridable method for responding to an answer given by player to a question asked by the
//monster.
//
//Params:
	int nCommand,			//(in)	CMD_YES or CMD_NO.
	CCueEvents &/*CueEvents*/)	//(out)	Add cue events if appropriate.
//
//Returns:
//True if any cue events were added, false if not.
{
	ASSERT(nCommand == CMD_YES || nCommand == CMD_NO);

	return false;
}

//*****************************************************************************
void CMonster::Say(
//Create a cue event that makes the monster "say" something.
//
//Params:
	MESSAGE_ID eMessageID,		//(in)	Message to say.  One of the MID_* constants.
	CCueEvents &CueEvents)    //(out)	One new CID_MonsterSpoke cue event will be added.
const
{
	ASSERT(eMessageID > (MESSAGE_ID)0);

	//Create the monster message.
	CMonsterMessage *pNewMessage = 
			new CMonsterMessage(MMT_OK, eMessageID, const_cast<CMonster *>(this));
	
	//Add cue event with attached monster message.
	CueEvents.Add(CID_MonsterSpoke, pNewMessage, true);
}

//*****************************************************************************
void CMonster::AskYesNo(
//Create a cue event that makes the monster "ask" something.
//
//Params:
	MESSAGE_ID eMessageID,    //(in)	Message for question to ask.  One of the MID_* constants.
	CCueEvents &CueEvents)    //(out)	One new CID_MonsterSpoke cue event will be added.
const
{
	ASSERT(eMessageID > (MESSAGE_ID)0);

	//Create the monster message.
	CMonsterMessage *pNewMessage = 
		new CMonsterMessage(MMT_YESNO, eMessageID, const_cast<CMonster *>(this));
		
	//Add cue event with attached monster message.
	CueEvents.Add(CID_MonsterSpoke, pNewMessage, true);
}

//*****************************************************************************
void CMonster::MakeStandardMove(
//Move monster and check whether it killed the player.
//
//Params:
	CCueEvents &CueEvents,	//(out)	Add cue events if appropriate.
	const int dx, const int dy)	//(in)	Movement offset.
{
	if (dx || dy)
	{
		Move(this->wX + dx, this->wY + dy);

		//If on the swordsman then kill him.
		if (IsOnSwordsman()) CueEvents.Add(CID_MonsterKilledPlayer, this);
   } else {
      //Remain stationary.
      this->wPrevX = this->wX;
      this->wPrevY = this->wY;
   }
}

//*****************************************************************************
void CMonster::Move(
//Moves the monster to a new square in the room.  Can be overridden for things like
//monsters that take up more than one square.
//
//Params:
	const UINT wDestX, const UINT wDestY)	//(in)	Destination to move to.
{
	//Update monster array.
	this->pCurrentGame->pRoom->MoveMonster(this,wDestX,wDestY);

   //Save old coords.
   this->wPrevX = this->wX;
   this->wPrevY = this->wY;

   //Set new coords.
	this->wX = wDestX;
	this->wY = wDestY;
}

// $Log: Monster.cpp,v $
// Revision 1.45  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.44  2003/07/22 18:37:07  mrimer
// Changed reinterpret_casts to DYN_CAST.
//
// Revision 1.43  2003/06/20 23:48:03  mrimer
// Brains can now only sense invisible player when within smelling range.
//
// Revision 1.42  2003/06/20 20:45:01  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.41  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.40  2003/06/16 23:36:38  mrimer
// Added prev position vars.  Fixed serpent error.
//
// Revision 1.39  2003/05/24 02:07:21  mrimer
// Fixes for APPLE portability (committed on behalf of Ross Jones).
//
// Revision 1.38  2003/05/13 01:10:25  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.37  2003/05/09 02:43:20  mrimer
// Tweaked some ASSERTS to compile as __assume's for Release build.
//
// Revision 1.36  2003/05/08 23:22:38  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.35  2003/01/08 00:51:14  mrimer
// Added an include.
//
// Revision 1.34  2002/12/22 01:51:41  mrimer
// Revised swordsman vars.
// Changed handling of some dxFirst/dyFirst parameters.
//
// Revision 1.33  2002/11/22 01:54:41  mrimer
// Added ResetCurrentGame().
//
// Revision 1.32  2002/11/18 18:39:00  mrimer
// Changed pathmap calling convention.
//
// Revision 1.31  2002/11/15 01:33:34  mrimer
// Added IsSwordsmanWithin() and IsSwordWithin().
//
// Revision 1.30  2002/09/27 17:49:48  mrimer
// Fixed usage of DEFAULT_SMELL_RANGE.
//
// Revision 1.29  2002/09/24 21:13:19  mrimer
// Added CanSmellObjectAt().  Moved DEFAULT_SMELL_RANGE to .h file.
//
// Revision 1.28  2002/09/19 16:24:53  mrimer
// Made some parameters const.  Added some documentation.
//
// Revision 1.27  2002/09/13 22:40:04  mrimer
// Refactored general monster code from derived classes into GetDirectMovement(), MakeStandardMove(), and SetOrientation().
//
// Revision 1.26  2002/08/30 21:46:43  mrimer
// Documented that brain-directed movement will track player when invisible.
//
// Revision 1.25  2002/08/29 22:00:39  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.24  2002/07/22 00:55:00  erikh2000
// Changed "new CMonsterMessage" calls to use member-setting constructor.
//
// Revision 1.23  2002/06/22 05:51:54  erikh2000
// Changed code to use new CMimic methods for getting sword coords.
//
// Revision 1.22  2002/06/21 04:40:42  mrimer
// Revised includes.
// Added multiple pathmap support.
// Optimized monster lookup.
//
// Revision 1.21  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.20  2002/03/04 22:23:47  erikh2000
// Changed obstacle logic to evaluate T_CHECKPOINT as clear.
//
// Revision 1.19  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.18  2002/02/09 00:57:49  erikh2000
// Changed current game pointers to const.
//
// Revision 1.17  2001/12/16 02:15:37  erikh2000
// Added CMonster::AskYesNo(), CMonster::Say(), and CMonster::OnAnswer() methods.
//
// Revision 1.16  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.15  2001/11/19 20:43:29  erikh2000
// Added code to handle non-obstacle case of mimic moving onto his own sword square in CMonster::DoesSquareContainObstacle().
//
// Revision 1.14  2001/11/19 20:36:21  erikh2000
// Added check for mimic swords in CMonster::DoesSquareContainObstacle().  (Committed on behalf of j_wicks.)
//
// Revision 1.13  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.12  2001/11/05 23:15:25  erikh2000
// Changed CID_MonsterDiedFromStab cue event add to receive CMonster * private data.
//
// Revision 1.11  2001/10/27 20:26:37  erikh2000
// Removed CueEvents.Clear calls and fixed some kind of EOL translation causing double-spacing.
//
// Revision 1.10  2001/10/26 19:35:59  md5i
// Out-of-bounds checks should work for <0 case as well.
//
// Revision 1.9  2001/10/26 19:30:37  md5i
// Out-of-bounds check could make for lack of movement (theoretically).
// Add null-movement check.
//
// Revision 1.8  2001/10/26 06:23:52  erikh2000
// Added out-of-bounds square checking to CMonster::GetBestMove().
//
// Revision 1.7  2001/10/25 05:43:24  md5i
// Fixed arrow problem for monsters.
//
// Revision 1.6  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.5  2001/10/23 03:28:28  md5i
// Add GetBestMove routine.
//
// Revision 1.4  2001/10/21 00:29:32  erikh2000
// Fixed bugs in GetBeelineMovement().
// Added checks for monster and sword in DoesSquareContainObstacle().
//
// Revision 1.3  2001/10/20 05:42:43  erikh2000
// Removed tile image references.
//
// Revision 1.2  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.1.1.1  2001/10/01 22:20:18  erikh2000
// Initial check-in.
//
