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
 *
 * ***** END LICENSE BLOCK ***** */

//Serpent.cpp
//Implementation of CSerpent.

#include "Serpent.h"

//
//Public methods.
//

//*****************************************************************************************
CSerpent::CSerpent(CCurrentGame *pSetCurrentGame)
	: CMonster(M_SERPENT, pSetCurrentGame)
	, foundTail(false)
{
}

//*****************************************************************************************
void CSerpent::Process(
//Process a serpent for movement.
//
//Params:
	const int nLastCommand,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	if (!this->foundTail)
		FindTail();

	int dxFirst, dyFirst, dx, dy;
	if (!GetSerpentMovement(dxFirst, dyFirst, dx, dy))
		return;

	// Move according to direction chosen.
	LengthenHead(dx,dy,nGetOX(wO),nGetOY(wO));
	if (ShortenTail(CueEvents))
		return;	//snake died

	if (IsOnSwordsman())
		CueEvents.Add(CID_MonsterKilledPlayer, this);
}

//******************************************************************************************
bool CSerpent::GetSerpentMovement(
//Gets offsets for serpent movement to the swordsman, taking
//obstacles into account.
//
//Always plans some movement even if player is invisible.
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
	if (pCurrentGame->pRoom->IsBrainPresent())
		GetBrainDirectedMovement(dxFirst, dyFirst, dx, dy);
	else if (CanFindSwordsman())
		GetNormalMovement(dx, dy);
	else
		return false;

	return true;
}

//*****************************************************************************************
bool CSerpent::LengthenHead(
//Move head out one square (extending body by one).
//
//Returns: whether a move can be made
//
//Params:
	const int dx, const int dy,	//(in)	Offsets that indicate direction
											//		of movement from current square.
	const int oX, const int oY)	//(in)	Current orientation of head.
{
	if (dx || dy)
	{
		ASSERT ((dx == 0) != (dy == 0));	//always moving, no diagonal steps

		UINT tile;
		if (dx != 0) 
		{
			switch (nGetO(dx, oY))
			{
			case E: case W:	tile = T_SNK_EW; break;
			case NE: tile = T_SNK_NW; break;
			case SE: tile = T_SNK_SW; break;
			case NW: tile = T_SNK_NE; break;
			case SW: tile = T_SNK_SE; break;
			default: ASSERT(false);
			}
		} 
		else
		{
			switch(nGetO(oX, dy))
			{
			case N: case S:	tile = T_SNK_NS; break;
			case NE: tile = T_SNK_SE; break;
			case SE: tile = T_SNK_NE; break;
			case NW: tile = T_SNK_SW; break;
			case SW: tile = T_SNK_NW; break;
			default: ASSERT(false);
			}
		}
		Move(this->wX + dx, this->wY + dy);
		//add segment to the old spot (wX and wY were just updated)
		this->pCurrentGame->pRoom->Plot(this->wX - dx, this->wY - dy, tile, this);

		wO = nGetO(dx, dy);
		return true;
	}
	return false;
}

//*****************************************************************************************
bool CSerpent::ShortenTail(
//Shrink tail by one square (shortening body by one).
//
//Returns: whether snake died from truncation
//
//Params:
	CCueEvents &CueEvents)	//(in/out)
{
	int dx = nGetOX(this->tailO);
	int dy = nGetOY(this->tailO);
	ASSERT((dx==0) != (dy==0));	//always moving, no diagonals
	
	this->pCurrentGame->pRoom->Plot(this->tailX, this->tailY, T_EMPTY);
	this->tailX += dx;
	this->tailY += dy;
	if (this->tailX == this->wX && this->tailY == this->wY) 
	{
		CueEvents.Add(CID_SnakeDiedFromTruncation, this);
		return true;
	}

	UINT tile = this->pCurrentGame->pRoom->GetTSquare(
			this->tailX, this->tailY);
	int t;
	switch (tile)
	{
		case T_SNK_NS: case T_SNK_EW: break;
		case T_SNK_NW: case T_SNK_SE: t = dx; dx = -dy; dy = -t; break;
		case T_SNK_NE: case T_SNK_SW: t = dx; dx = dy; dy = t; break;
		default: ASSERT(false);
	}
	switch (nGetO(dx, dy))
	{
		case N: tile = T_SNKT_S; this->tailO = N; break;
		case S: tile = T_SNKT_N; this->tailO = S; break;
		case E: tile = T_SNKT_W; this->tailO = E; break;
		case W: tile = T_SNKT_E; this->tailO = W; break;
		default: ASSERT(false);
	}
	this->pCurrentGame->pRoom->Plot(this->tailX, this->tailY, tile, this);

	return false;
}

//*****************************************************************************************
bool CSerpent::IsTileObstacle(
// Override the normal IsTileObstacle for serpents.
// 
//Params:
	const UINT wLookTileNo)	//(in)	Tile to evaluate.  Note each tile# will
						//		always be found on the same layer of squares.
//
//Returns:
//True if tile is an obstacle, false if not.
const
{
	return (!(
			wLookTileNo == T_EMPTY ||
		  wLookTileNo == T_FLOOR ||
		  wLookTileNo == T_CHECKPOINT ||
		  wLookTileNo == T_TRAPDOOR ||
		  wLookTileNo == T_DOOR_YO
			//should have T_SCROLL also, but it was left out of Webfoot DROD
			));
}

//*****************************************************************************************
void CSerpent::GetBrainDirectedMovement(
// Special-cased for serpents.
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
	this->pCurrentGame->pRoom->pPathMap[this->eMovement]->GetRecPaths(this->wX,
			this->wY, paths, num_paths);
	
	for (UINT i = 0; i < num_paths; i++)
	{
		if ((UINT)paths[i].x == this->wX && (UINT)paths[i].y == this->wY)
			continue;
		else if (abs(this->wX - paths[i].x) != abs(this->wY - paths[i].y) &&
				 !DoesSquareContainObstacle(paths[i].x, paths[i].y))
		{
			dx = dxFirst = paths[i].x - this->wX;
			ASSERT(abs(dx) <= 1);
			dy = dyFirst = paths[i].y - this->wY;
			ASSERT(abs(dy) <= 1);
			return;
		}
	}

	GetNormalMovement(dx, dy);
}

//******************************************************************************************
bool CSerpent::DoesSquareContainObstacle(
//Determines if a square contains an obstacle for a serpent.  Unlike other monsters, the
//serpent CAN move onto a sword square, and CAN NOT move onto an arrow.
//
//Params:
	UINT wCol, UINT wRow) //(in) Coords of square to evaluate.  Must be valid.
const
{
	//Routine is not written to check the square on which this monster is 
	//standing.
	ASSERT(wCol != this->wX || wRow != this->wY);
	
	//Check t-square for obstacle.
	CDbRoom *pRoom = this->pCurrentGame->pRoom;
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

	//No obstacle.
	return false;
}

//*****************************************************************************
void CSerpent::GetTail(
//Returns location of the tail.  (Called by the room editor.)
//
//Params:
	UINT &wTailX, UINT &wTailY)	//(out) Coords of tail
{
	FindTail();	//always look for the tail
	wTailX = this->tailX;
	wTailY = this->tailY;
}

//
//Private methods.
//

//*****************************************************************************
void CSerpent::GetNormalMovement(
// Figures out the normal movement of the serpent when not affected by a brain.
//
//Params:
	int &dx,		//(out) Horizontal delta (-1, 0, or 1) for where monster
					//		can go, taking into account obstacles.
	int &dy)		//(out) Vertical delta (-1, 0, or 1) for same.
const
{
	// If swordsman is ahead or behind the serpent, keep moving toward him.
	// Otherwise, switch between favoring horizonal or vertical movement
	// every five turns.  
	const bool horizontal = ((this->pCurrentGame->wSpawnCycleCount % 10) < 5);

	const int oX = nGetOX(wO);
	const int oY = nGetOY(wO);

	if (CanFindSwordsman()) 
	{
		//Is swordsman in front of or behind serpent?
		if (!oX)
		{
			//serpent is moving vertically
			if (this->pCurrentGame->swordsman.wX == this->wX)
			{
				//Yes.  Keep moving this direction.
				dy = oY;
				dx = 0;
				if (CanMoveTo(this->wX + dx, this->wY + dy))
					return;
			}
		} else {
			//serpent moving horizontally
			if (this->pCurrentGame->swordsman.wY == this->wY)
			{
				dx = oX;
				dy = 0;
				if (CanMoveTo(this->wX + dx, this->wY + dy))
					return;
			}
		}

		// Move towards swordsman.
		if (horizontal)
		{
			dx = sgn(this->pCurrentGame->swordsman.wX - this->wX);
			if (dx == 0)
				dy = sgn(this->pCurrentGame->swordsman.wY - this->wY);
			else
				dy = 0;
		}
		else
		{
			dy = sgn(this->pCurrentGame->swordsman.wY - this->wY);
			if (dy == 0)
				dx = sgn(this->pCurrentGame->swordsman.wX - this->wX);
			else
				dx = 0;
		}

		// Check the coordinates
		if (CanMoveTo(this->wX + dx, this->wY + dy))
			return;	//move here
	}

	// We can't move towards the swordsman in the desired manner.
	// Try the four cardinal directions
	static const int directions[] = {N, E, S, W};
	bool found = false;
	for (int i = 0; !found && i < 4; i++) 
	{
		dx = nGetOX(directions[i]);
		dy = nGetOY(directions[i]);
		// Don't backtrack
		if (dx == -oX && dy == -oY)
			continue;
		if (CanMoveTo(this->wX + dx, this->wY + dy))
			found = true;
	}
	if (!found)
		dx = dy = 0;	//stuck
}

//*****************************************************************************************
bool CSerpent::CanMoveTo(
	const int x,
	const int y) const
{
	return (!(
		  (UINT)x >= pCurrentGame->pRoom->wRoomCols ||
		  (UINT)y >= pCurrentGame->pRoom->wRoomRows ||
		  DoesSquareContainObstacle((UINT)x, (UINT)y)));
}

//*****************************************************************************************
void CSerpent::FindTail(void)
// Starting from head, traverse room tiles to find position of its tail.
// Assumes a valid serpent.
{
	int dx = -(int)nGetOX(this->wO);
	int dy = -(int)nGetOY(this->wO);
	ASSERT ((dx == 0) != (dy == 0));
	int x = this->wX, y = this->wY;
	bool done = false;
	UINT tile;
	while (!done)
	{
		int t;
		x += dx;
		y += dy;
		tile = pCurrentGame->pRoom->GetTSquare(x, y);
		ASSERT(bIsSerpent(tile));
		switch (tile) 
		{
		case T_SNK_EW: case T_SNK_NS: break;
		case T_SNK_NW: case T_SNK_SE: t = dx; dx = -dy; dy = -t; break;
		case T_SNK_NE: case T_SNK_SW: t = dx; dx = dy; dy = t; break;
		case T_SNKT_S: case T_SNKT_W: case T_SNKT_N: case T_SNKT_E:
			//tail tiles
			done = true;
			break;
		default: ASSERT(false);
		}
	}
	this->tailX = x;
	this->tailY = y;
	this->tailO = nGetO(-dx, -dy);
	this->foundTail = true;
}

// $Log: Serpent.cpp,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.21  2002/12/22 01:50:19  mrimer
// Revised swordsman vars.
//
// Revision 1.20  2002/11/22 01:56:34  mrimer
// Fixed a bug (for room editor).
//
// Revision 1.19  2002/11/18 18:35:52  mrimer
// Added GetTail().
//
// Revision 1.18  2002/11/15 01:31:13  mrimer
// Fixed serpent movement logic to be compatible with Webfoot DROD.
// Revised code to work with CDbRoom's new monster array logic (for large monsters).
//
// Revision 1.17  2002/10/03 21:16:40  mrimer
// Cleaned up code.
//
// Revision 1.16  2002/09/19 17:49:28  mrimer
// Cleaned up serpent code.  Explicitly declared vitual methods.
// Added player invisibility rule for serpents.
//
// Revision 1.15  2002/09/10 19:24:43  mrimer
// Tweaking.
//
// Revision 1.14  2002/09/01 00:04:55  erikh2000
// Changed references to "wTurnsTaken" to "wSpawnCycleCount".
//
// Revision 1.13  2002/08/29 22:00:40  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.12  2002/06/21 04:47:48  mrimer
// Revised includes.
//
// Revision 1.11  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.10  2002/03/04 22:23:24  erikh2000
// Changed obstacle logic to evaluate T_CHECKPOINT as clear.
//
// Revision 1.9  2002/02/25 03:41:02  erikh2000
// Made read-only methods const.
// Changed GetBrainDirectedMovement() to make serpent look at all of its brain options before using normal movement.
//
// Revision 1.8  2002/02/08 23:19:43  erikh2000
// Changed CID_SnakeDiedFromTruncation event add to have this pointer for private data.
//
// Revision 1.7  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.6  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.5  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.4  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.3  2001/11/12 01:27:25  erikh2000
// Made serpents not die when stabbed.
// Made serpents traverse squares containing swords.
//
// Revision 1.2  2001/11/11 17:19:35  md5i
// Fixed some roaches left in the comments.
//
// Revision 1.1  2001/11/11 05:01:16  md5i
// Added serpents.
//
