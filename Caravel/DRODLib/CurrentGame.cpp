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
 * Michael Welsh Duggan (md5i), Richard Cookney (timeracer), John Wm. Wicks (j_wicks),
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//CurrentGame.cpp.
//Implementation of CCurrentGame,

#include "CurrentGame.h"
#include "Db.h"
#include "DBProps.h"
#include "Monster.h"
#include "CueEvents.h"
#include "GameConstants.h"
#include "TileConstants.h"
#include "MonsterFactory.h"
#include "Pathmap.h"
#include "Mimic.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/CoordStack.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

#define TAG_ESCAPE	(DWORD)(-2)

const UINT TIRED_TURN_COUNT = 40;	//# turns to check for Swordsman becoming tired

//
//Protected methods.
//

//*****************************************************************************
CCurrentGame::CCurrentGame()
//Constructor.
	: pRoom(NULL)
	, pLevel(NULL)
	, pHold(NULL)
	, pbMonstersKilled(NULL)
{
	//Zero resource members before calling Clear().
	Clear();
}

//
//Public methods.
//

//*****************************************************************************
CCurrentGame::~CCurrentGame()
//Destructor.
{
	if (this->bIsDemoRecording) 
	{
		if (!EndDemoRecording())
		{
			CFiles f;
			f.AppendErrorLog("Failed to save a demo when recording ended.\r\n");
		}
	}
	
	if (this->Commands.IsFrozen()) this->Commands.Unfreeze();

	Clear();
}

//*****************************************************************************
void CCurrentGame::Clear()
//Frees resources associated with this object and zeros members.
{
	CDbSavedGame::Clear();

	delete this->pRoom;
	this->pRoom = NULL;

	delete this->pLevel;
	this->pLevel = NULL;

	delete this->pHold;
	this->pHold=NULL;

	this->swordsman.Clear();

	this->wSpawnCycleCount = this->wTurnNo = 0;

	this->wMonsterKills = 0L;
	delete this->pbMonstersKilled;
	this->pbMonstersKilled=NULL;
	this->wMonstersKilledRecently = 0L;
	this->bLotsOfMonstersKilled = false;

	this->dwLastCheckpointSavedGameID = 0L;
	this->bOnCheckpoint = false;
   this->bBrainSensesSwordsman = false;

	this->dwAutoSaveOptions = ASO_DEFAULT;
	
	this->bIsGameActive = this->bIsDemoRecording = false;

	memset(&(this->DemoRecInfo), 0, sizeof(this->DemoRecInfo));

	this->UnansweredQuestions.clear();

	//Reset the Explored and Conquered room lists.
	ClearRoomLists();
}

//*****************************************************************************
WSTRING CCurrentGame::AbbrevRoomLocation()
   //Prepend room position to non-empty (player viewable) demo descriptions.
{
   WSTRING descText;

   //Hold name.
   WSTRING holdName = static_cast<const WCHAR *>(this->pHold->NameText);
   WSTRING abbrevHoldName;
   static const UINT MAX_HOLD_NAME = 8;
   if (holdName.size() <= MAX_HOLD_NAME)
      descText += holdName;
   else
   {
      //Try to abbreviate by taking only the first letter from each word
      abbrevHoldName = filterFirstLettersAndNumbers(holdName);
      descText += abbrevHoldName;
   }
   descText += wszColon;
   descText += wszSpace;

   //Level name.
   descText += this->pLevel->NameText;
   descText += wszColon;
   descText += wszSpace;

   //Room name.
   WSTRING abbrevRoomPosition;
   this->pRoom->GetLevelPositionDescription(abbrevRoomPosition, true);
   descText += abbrevRoomPosition;

   return descText;
}

//*****************************************************************************
void CCurrentGame::SetHighlightRoomIDs(
//Sets the rooms for which highlight demos will be automatically saved.  The 
//caller is telling CCurrentGame which rooms it is interested in.
//
//Params:
	const CIDList &SetRoomIDs)
{
	//No highlight demos will be saved unless the ASO_HIGHLIGHTDEMO flag was set.
	//If you come up with a valid use for having the room IDs set without the flag
	//set, then this assertian can be removed or changed.
	ASSERT((this->dwAutoSaveOptions & ASO_HIGHLIGHTDEMO)==ASO_HIGHLIGHTDEMO);

	this->HighlightRoomIDs = SetRoomIDs;
}

//*****************************************************************************
bool CCurrentGame::IsLevelStart()
//Is the current game at the beginning of a level.
{
	if (this->wTurnNo != 0) return false;

   //Must be in level entrance room.
	DWORD dwStartRoomX, dwStartRoomY;
	this->pLevel->GetStartingRoomCoords(dwStartRoomX, dwStartRoomY);
	if (!(this->pRoom->dwRoomX == dwStartRoomX &&
			this->pRoom->dwRoomY == dwStartRoomY)) return false;

   //Must be at level entrance position.
   return (this->swordsman.wX == this->pLevel->wX &&
         this->swordsman.wY == this->pLevel->wY);
}

//*****************************************************************************
void CCurrentGame::BeginDemoRecording(
//Begins demo recording.  Database writes to store the demo will occur when the 
//swordsman exits a room or EndDemoRecording() is called.
//
//Params:
	const WCHAR *pwczSetDescription)	//(in)  Description stored with demo record(s).
{
	ASSERT(pwczSetDescription);
	ASSERT(!this->bIsDemoRecording);
	
	//Commands should not be frozen while recording.  Recording requires commands to
	//be added for each call to ProcessCommand().  It is possible to have some carefully
	//thought-out mixture of the two states, in which case this assertian can be changed.
	ASSERT(!this->Commands.IsFrozen());

	//Set recording information.
	this->DemoRecInfo.dwDescriptionMessageID = AddMessageText(pwczSetDescription);
	this->DemoRecInfo.wBeginTurnNo = this->wTurnNo;
	this->DemoRecInfo.dwPrevDemoID = 0L;
	this->DemoRecInfo.dwFirstDemoID = 0L;

	this->bIsDemoRecording = true;
}

//*****************************************************************************
DWORD CCurrentGame::EndDemoRecording()
//Ends demo recording, which may cause database to be updated with demo information.
//
//Returns:
//DemoID of first and maybe only demo in series of demos recorded (one per room) or 
//0 if no commands recorded.
{
	ASSERT(this->bIsDemoRecording);

	//Commands should not be frozen while recording.  Recording requires commands to
	//be added for each call to ProcessCommand().  It is possible to have some carefully
	//thought-out mixture of the two states, in which case this assertian can be changed.
	ASSERT(!this->Commands.IsFrozen());

	this->bIsDemoRecording = false;

	//If no commands were recorded, no database update for the current room is needed.
	if (this->DemoRecInfo.wBeginTurnNo < this->wTurnNo)
		//Save the current room state and commands to a demo.
		WriteCurrentRoomDemo(this->DemoRecInfo, false, false);
	else return TAG_ESCAPE;

	return this->DemoRecInfo.dwFirstDemoID;
}

//*****************************************************************************
DWORD CCurrentGame::GetChecksum()
//Gets a checksum representing the current game state.  This checksum is meant to
//stay the same even if DROD's implementation changes radically, so only things that
//I'm confident will be consistently measurable in future versions are included.
//
//Returns:
//The checksum.
const
{
	const DWORD dwSum =
	 
		//Swordsman position.
		BYTE(this->swordsman.wX) * 0x00000001 +
		BYTE(this->swordsman.wY) * 0x00000010 +
		BYTE(this->swordsman.wO) * 0x00000100 +

		//Number of monsters.
		BYTE(this->pRoom->wMonsterCount) * 0x00010000 +

		//Turn count.
		BYTE(this->wTurnNo) * 0x00100000 +

		//Conquered rooms.
		BYTE(this->ConqueredRooms.GetSize()) * 0x01000000 +

		//Explored rooms.
		BYTE(this->ExploredRooms.GetSize()) * 0x10000000;

	return dwSum;
}

//*****************************************************************************
void CCurrentGame::SetSwordsmanMood(
//Determine swordsman's mood, according to relative position of monsters.
//Scared overrides aggressive, aggressive overrides tired/normal mood.
//
//Params:
	CCueEvents &CueEvents)	//(out)	List of events that can be handled by caller.
{
	const UINT wSX = this->swordsman.wSwordX, wSY = this->swordsman.wSwordY;	//shorthand
	CMonster *pMonster;
	UINT x, y;

	//Scared?
	//(whether aggressive monster is behind swordsman)
	const UINT wX = this->swordsman.wX, wY = this->swordsman.wY,	//shorthand
			wO = this->swordsman.wO;
	const int oX = nGetOX(wO);
	const int oY = nGetOY(wO);
	CCoordStack checkSquares;

	//Determine which squares to check.
	if (wX-oX < this->pRoom->wRoomCols)	//avoid assertion
		checkSquares.Push(wX-oX,wY-oY);	//check directly behind swordsman
	//check behind on left/right sides
	if (oX && oY)	//swordsman is facing diagonally
	{
		checkSquares.Push(wX-oX,wY);
		checkSquares.Push(wX,wY-oY);
	} else {	//swordsman is facing straight
		if (oX && wX-oX < this->pRoom->wRoomCols)	//avoid assertion
		{
			checkSquares.Push(wX-oX,wY-1);
			checkSquares.Push(wX-oX,wY+1);
		}
		else if (wY-oY < this->pRoom->wRoomRows)	//avoid assertion
		{	//oY
			checkSquares.Push(wX-1,wY-oY);
			checkSquares.Push(wX+1,wY-oY);
		}
	}

	//Check squares for monster.
	while (checkSquares.GetSize()) {
		checkSquares.Pop(x,y);
		if (this->pRoom->IsValidColRow(x,y))
		{
			pMonster = this->pRoom->GetMonsterAtSquare(x,y);
			if (this->pRoom->MonsterHeadIsAt(x,y) && pMonster->IsAggressive())
			{
				CueEvents.Add(CID_SwordsmanAfraid);
				while (checkSquares.GetSize())	//remove remaining squares
					checkSquares.Pop(x,y);
				return;	//don't check for other moods
			}
		}
	}

	//Aggressive?
	//(whether sword is adjacent to a non-friendly monster)
	for (x=wSX-1; x<=wSX+1; x++)
		for (y=wSY-1; y<=wSY+1; y++)
		{
			if (x==wSX && y==wSY) continue;
			if (this->pRoom->IsValidColRow(x,y))
			{
				pMonster = this->pRoom->GetMonsterAtSquare(x,y);
				if (this->pRoom->MonsterHeadIsAt(x,y) && pMonster->wType != M_MIMIC)
				{
					CueEvents.Add(CID_SwordsmanAggressive);
					return;	//don't check for other moods
				}
			}
		}

	//Tired?
	if (IsSwordsmanTired())
		CueEvents.Add(CID_SwordsmanTired);

	CueEvents.Add(CID_SwordsmanNormal);
}

//*****************************************************************************
void CCurrentGame::ProcessCommand(
//Processes a game command, causing game data and current room to be updated
//in response.  Display or sound should be handled by caller if appropriate.
//
//Params:
	const int nCommand,			//(in)	Game command.
	CCueEvents &CueEvents)	//(out)	List of events that can be handled by caller.
							//		These are things that the UI wouldn't necessarily
							//		be aware of by looking at the modified game
							//		data on return.
{
	//Caller should not be trying to process new commands after the game is
	//inactive.  Before doing so, caller will need to reload the room in some way.
	ASSERT(this->bIsGameActive);

	//Add this command to list of commands for the room.
	if (!this->Commands.IsFrozen()) this->Commands.Add(nCommand);

	//Note: Private data pointers found in CueEvents are guaranteed to be valid 
	//until the CCurrentGame instance that originally returned the CueEvents goes out
	//of scope or the ProcessCommand() method is called again on the same instance.	
	CueEvents.Clear();
	this->pRoom->ClearPlotHistory();

	//Clear dead monsters stored in room from a previous turn.  They were kept in
	//memory after the previous call to ProcessCommand() to keep private data
	//pointers valid.
	this->pRoom->ClearDeadMonsters();

	UINT wOriginalMonsterCount = this->pRoom->wMonsterCount;

	//Increment the turn#.
	this->wTurnNo++;
	
	//If there are any unanswered questions, process them.
	if (this->UnansweredQuestions.size())
		ProcessUnansweredQuestions(nCommand, this->UnansweredQuestions, CueEvents);
	else
	{
      const UINT wMonstersBeforeMove = this->pRoom->wMonsterCount;
		if (this->swordsman.bIsPlacingMimic)
			ProcessMimicPlacement(nCommand, CueEvents);
		else
		{
			//Swordsman tired logic.
			unsigned char *pbMonstersKilled = this->pbMonstersKilled + (this->wTurnNo % TIRED_TURN_COUNT);
			this->wMonstersKilledRecently -= *pbMonstersKilled;
			*pbMonstersKilled = 0;

			ProcessSwordsman(nCommand, CueEvents);

			CalcPathMaps();

			if (!CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom))
			{
				ProcessMonsters(nCommand, CueEvents);

				ProcessSimultaneousSwordHits(CueEvents);	//destroy simultaneously-stabbed tar

				//Grow the tar in response to cue event.
				if (CueEvents.HasOccurred(CID_TarGrew))
					this->pRoom->GrowTar(CueEvents);

				SetSwordsmanMood(CueEvents);
			}
		}
      if (CueEvents.HasOccurred(CID_MonsterKilledPlayer))
         this->swordsman.bIsDying = true;
      else
      {
         //Did room just get cleared?
	      if (!this->pRoom->wMonsterCount && wMonstersBeforeMove)
	         if (!IsCurrentRoomConquered())	//and was it not already cleared?
		         CueEvents.Add(CID_AllMonstersKilled);
      }
	}

	//Check for new questions that were asked.  Put them in a list of questions
	//for which answers will be expected on subsequent calls.
	AddQuestionsToList(CueEvents, this->UnansweredQuestions);

	//If any of the above killed the player, remember that the game is now
	//inactive.
	this->bIsGameActive = !(CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), 
			CIDA_PlayerDied));
	
	//End demo recording if game is now inactive.
	if (!this->bIsGameActive) 
	{
		if (IsDemoRecording())	//End a demo that is recording.
		{
			if (!EndDemoRecording()) 
			{
				CFiles f;
				f.AppendErrorLog("Failed to save a demo when recording ended.\r\n");
			}
		}
		else if ((this->dwAutoSaveOptions & ASO_DIEDEMO) == ASO_DIEDEMO)
			WriteCurrentRoomDieDemo();
	}

	//If swordsman steps onto a checkpoint and lives...
	if (pRoom->GetOSquare(this->swordsman.wX, this->swordsman.wY) == T_CHECKPOINT && this->bIsGameActive)
	{
		//Only save checkpoint when stepping onto it, not while standing on it.
		if (!this->bOnCheckpoint)
		{
			this->bOnCheckpoint = true;

			//Add CueEvent to handle effect.
			CueEvents.Add(CID_CheckpointActivated, new CCoord(this->swordsman.wX,
					this->swordsman.wY), true);
			
			//Save the game unless options have disabled it.
			if (!this->Commands.IsFrozen() && 
					(this->dwAutoSaveOptions & ASO_CHECKPOINT)==ASO_CHECKPOINT)
				SaveToCheckpoint();
		}
	} else {
		this->bOnCheckpoint = false;
	}

	//If there were monsters, but not any more, then remove green doors.
	if (wOriginalMonsterCount && !this->pRoom->wMonsterCount) 
	{
		if (this->pRoom->RemoveGreenDoors())
			CueEvents.Add(CID_GreenDoorsOpened);
	}

	//Return cue event for plots if any plots were made.  This check needs
	//to go after any code that could call pRoom->Plot().
	if (this->pRoom->bPlotsMade)
		CueEvents.Add(CID_Plots);
}

//*****************************************************************************
void CCurrentGame::SetSwordsman(
//Move swordsman to new square.
//
//Params:
	const UINT wSetX, const UINT wSetY)	//(in)	Coords of new square.
{
	ASSERT(this->pRoom->IsValidColRow(wSetX, wSetY));

	const bool moved = this->swordsman.Move(wSetX,wSetY);

	//Reset PathMaps' target.
	if (moved)
		SetPathMapsTarget(this->swordsman.wX, this->swordsman.wY);
}

//*****************************************************************************
bool CCurrentGame::SetSwordsmanToEastExit()
//Move swordsman to an exit along east column.
{
	//Look for a clear square along the east.  Search outwards from the row
	//swordsman is currently at.
	for (UINT wRowOffset = 0; ; ++wRowOffset)
	{
		bool bNorthInBounds = (this->swordsman.wY - wRowOffset < this->pRoom->wRoomRows);
		bool bSouthInBounds = (this->swordsman.wY + wRowOffset < this->pRoom->wRoomRows);
		if (!bNorthInBounds && !bSouthInBounds) return false; //No exit found.

		//Check for exit at north offset.
		if (bNorthInBounds)
		{
			if (this->pRoom->CanSetSwordsman(
				this->pRoom->wRoomCols - 1, this->swordsman.wY - wRowOffset))
			{
				SetSwordsman(this->pRoom->wRoomCols - 1, this->swordsman.wY - wRowOffset);
				return true;
			}
		}

		//Check for exit at south offset.
		if (bSouthInBounds)
		{
			if (this->pRoom->CanSetSwordsman(
				this->pRoom->wRoomCols - 1, this->swordsman.wY + wRowOffset))
			{
				SetSwordsman(this->pRoom->wRoomCols - 1, this->swordsman.wY + wRowOffset);
				return true;
			}
		}
	}
}

//*****************************************************************************
bool CCurrentGame::SetSwordsmanToWestExit()
//Move swordsman to an exit along west column.
{
	//Look for a clear square along the west.  Search outwards from the row
	//swordsman is currently at.
	for (UINT wRowOffset = 0; ; ++wRowOffset)
	{
		bool bNorthInBounds = (this->swordsman.wY - wRowOffset < this->pRoom->wRoomRows);
		bool bSouthInBounds = (this->swordsman.wY + wRowOffset < this->pRoom->wRoomRows);
		if (!bNorthInBounds && !bSouthInBounds) return false; //No exit found.

		//Check for exit at north offset.
		if (bNorthInBounds)
		{
			if (this->pRoom->CanSetSwordsman(0, 
					this->swordsman.wY - wRowOffset))
			{
				SetSwordsman(0, this->swordsman.wY - wRowOffset);
				return true;
			}
		}

		//Check for exit at south offset.
		if (bSouthInBounds)
		{
			if (this->pRoom->CanSetSwordsman(0, 
					this->swordsman.wY + wRowOffset))
			{
				SetSwordsman(0, this->swordsman.wY + wRowOffset);
				return true;
			}
		}
	}
}

//*****************************************************************************
bool CCurrentGame::SetSwordsmanToNorthExit()
//Move swordsman to an exit along north row.
{
	//Look for a clear square along the north.  Search outwards from the column
	//swordsman is currently at.
	for (UINT wColOffset = 0; ; ++wColOffset)
	{
		bool bWestInBounds = (this->swordsman.wX - wColOffset < this->pRoom->wRoomCols);
		bool bEastInBounds = (this->swordsman.wX + wColOffset < this->pRoom->wRoomCols);
		if (!bWestInBounds && !bEastInBounds) return false; //No exit found.

		//Check for exit at west offset.
		if (bWestInBounds)
		{
			if (this->pRoom->CanSetSwordsman(
				this->swordsman.wX - wColOffset, 0))
			{
				SetSwordsman(this->swordsman.wX - wColOffset, 0);
				return true;
			}
		}

		//Check for exit at east offset.
		if (bEastInBounds)
		{
			if (this->pRoom->CanSetSwordsman(
				this->swordsman.wX + wColOffset, 0))
			{
				SetSwordsman(this->swordsman.wX + wColOffset, 0);
				return true;
			}
		}
	}
}

//*****************************************************************************
bool CCurrentGame::SetSwordsmanToSouthExit()
//Move swordsman to an exit along south row.
{
	//Look for a clear square along the south.  Search outwards from the column
	//swordsman is currently at.
	for (UINT wColOffset = 0; ; ++wColOffset)
	{
		bool bWestInBounds = (this->swordsman.wX - wColOffset < this->pRoom->wRoomCols);
		bool bEastInBounds = (this->swordsman.wX + wColOffset < this->pRoom->wRoomCols);
		if (!bWestInBounds && !bEastInBounds) return false; //No exit found.

		//Check for exit at west offset.
		if (bWestInBounds)
		{
			if (this->pRoom->CanSetSwordsman(
				this->swordsman.wX - wColOffset, this->pRoom->wRoomRows - 1))
			{
				SetSwordsman(this->swordsman.wX - wColOffset, this->pRoom->wRoomRows - 1);
				return true;
			}
		}

		//Check for exit at east offset.
		if (bEastInBounds)
		{
			if (this->pRoom->CanSetSwordsman(
				this->swordsman.wX + wColOffset, this->pRoom->wRoomRows - 1))
			{
				SetSwordsman(this->swordsman.wX + wColOffset, this->pRoom->wRoomRows - 1);
				return true;
			}
		}
	}
}

//*****************************************************************************
UINT CCurrentGame::GetRoomExitDirection(
//Returns: direction swordsman should exit the current room,
//based on his move and position.
//
//Params:
	const UINT wMoveO)		//(in)	Direction swordsman is exiting.
const
{
	switch (wMoveO)
	{
		case N:
		return N;

		case S:
		return S;

		case W:
		return W;

		case E:
		return E;

		case NW:
			if (this->swordsman.wY == 0) 
				return N;
			else
				return W;

		case NE:
			if (this->swordsman.wY == 0) 
				return N;
			else
				return E;

		case SW:
			if (this->swordsman.wY == this->pRoom->wRoomRows - 1)
				return S;
			else
				return W;

		case SE:
			if (this->swordsman.wY == this->pRoom->wRoomRows - 1)
				return S;
			else
				return E;

		default:
			return NO_ORIENTATION;	//bad orientation
	}
}

//*****************************************************************************
void CCurrentGame::LoadNewRoomForExit(
//Loads room adjacent to the current room in the direction of the swordsman's
//exit of the current room.  Swordsman coords are also updated so that he wraps to
//other side in new room.
//
//Params:
   const DWORD dwSX, const DWORD dwSY,
   CDbRoom* pNewRoom,
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
{
   ASSERT(pNewRoom);
	delete this->pRoom;
   this->pRoom = pNewRoom;

	//Put swordsman at designated square.
	SetSwordsman(dwSX, dwSY);

	//Set start room members.
	SetRoomStartToSwordsman();
	SetMembersAfterRoomLoad(CueEvents);
}

//*****************************************************************************
bool CCurrentGame::LoadNewRoomForExit(
//Loads room adjacent to the current room in the direction of the swordsman's
//exit of the current room.  Swordsman coords are also updated so that he wraps to
//other side in new room.
//
//Params:
	const UINT wExitO,		//(in)	Direction swordsman is exiting.
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
{
	switch (wExitO)
	{
		case N:
		return LoadNorthRoom(CueEvents);

		case S:
		return LoadSouthRoom(CueEvents);

		case W:
		return LoadWestRoom(CueEvents);

		case E:
		return LoadEastRoom(CueEvents);

		default:
			ASSERTP(false, "Bad orientation value.");
		return false;
	}
}

//*****************************************************************************
void CCurrentGame::SaveToContinue()
//Save the current player's game to the continue slot for this hold.
{
	//It is not valid to save the current game when it is inactive.
	ASSERT(this->bIsGameActive);

	//Set saved game ID to current player's continue slot.
	DWORD dwContinueID = g_pTheDB->SavedGames.FindByContinue();
   if (!dwContinueID)
      dwContinueID = g_pTheDB->SavedGames.SaveNewContinue(g_pTheDB->GetPlayerID());
	this->dwSavedGameID = dwContinueID;
	this->eType = ST_Continue;
	this->bIsHidden = true;
	Update();

   //Update player's timestamp.
   CDbPlayer *pPlayer = g_pTheDB->GetCurrentPlayer();
   ASSERT(pPlayer);
   pPlayer->Update();
   delete pPlayer;
}

//*****************************************************************************
void CCurrentGame::SaveToLevelBegin()
//Saves the current game to the level-begin slot for this level.
{
	//It is not valid to save the current game when it is inactive.
	ASSERT(this->bIsGameActive);

	//Swordsman should be at beginning of level.
	ASSERT(this->wTurnNo == 0);
	ASSERT(this->swordsman.wX == this->wStartRoomX);
	ASSERT(this->swordsman.wY == this->wStartRoomY);
	ASSERT(this->swordsman.wO == this->wStartRoomO);
	ASSERT(this->pRoom->dwRoomID == this->dwRoomID);
	ASSERT(this->ExploredRooms.GetSize() == 1);

	this->eType = ST_LevelBegin;
	this->bIsHidden = false;

	//Is there already a saved game for this level?
	const DWORD dwExistingSavedGameID = g_pTheDB->SavedGames.FindByLevelBegin(
			this->pRoom->dwLevelID);
	if (dwExistingSavedGameID)	//Yes.
		return; //The existing saved game would be identical after an update, so exit.
			
	//No.
	this->dwSavedGameID = 0L;
	Update();
}

//*****************************************************************************
void CCurrentGame::SaveToRoomBegin()
//Saves the current game to the begin-room slot for this room.
{
	//It is not valid to save the current game when it is inactive.
	ASSERT(this->bIsGameActive);

	//Swordsman should be at beginning of room.
	ASSERT(this->wTurnNo == 0);
	ASSERT(this->swordsman.wX == this->wStartRoomX);
	ASSERT(this->swordsman.wY == this->wStartRoomY);
	ASSERT(this->swordsman.wO == this->wStartRoomO);

	this->eType = ST_RoomBegin;
	this->bIsHidden = false;

	//Is there already a saved game for this room?
	const DWORD dwExistingSavedGameID = g_pTheDB->SavedGames.FindByRoomBegin(
			this->pRoom->dwRoomID);
	if (dwExistingSavedGameID)	//Yes.
		this->dwSavedGameID = dwExistingSavedGameID;
	else						//No.
		this->dwSavedGameID = 0L;
	Update();
}

//*****************************************************************************
void CCurrentGame::SaveToCheckpoint()
//Saves the current game to the checkpoint slot for this room/checkpoint.
{
	//It is not valid to save the current game when it is inactive.
	ASSERT(this->bIsGameActive);

	//Swordsman should be standing on a checkpoint.
	ASSERT(this->pRoom->GetOSquare(this->swordsman.wX, this->swordsman.wY)==T_CHECKPOINT);

	this->eType = ST_Checkpoint;
	this->wCheckpointX = this->swordsman.wX;
	this->wCheckpointY = this->swordsman.wY;
	this->bIsHidden = false;	

	//If this is the first time a new room has been entered, make sure it will
	//also be marked this way when reloading this saved game.
	if (this->bIsNewRoom)
		this->ExploredRooms.Remove(this->pRoom->dwRoomID);

	//Is there already a saved game for this room checkpoint?
	const DWORD dwExistingSavedGameID = g_pTheDB->SavedGames.FindByCheckpoint(
			this->pRoom->dwRoomID, this->swordsman.wX, this->swordsman.wY);
	if (dwExistingSavedGameID)	//Yes.
		this->dwSavedGameID = dwExistingSavedGameID;
	else						//No.
		this->dwSavedGameID = 0L;
	Update();

	if (this->bIsNewRoom) SetCurrentRoomExplored();	//put it back the way it was

	//Mark as last checkpoint visited.
	this->dwLastCheckpointSavedGameID = this->dwSavedGameID;
	ASSERT(this->dwLastCheckpointSavedGameID);
}

//*****************************************************************************
void CCurrentGame::SaveToEndHold()
//Save the current player's game to the end hold slot for this hold.
//Only do once.
{
	//It is not valid to save the current game when it is inactive.
	ASSERT(this->bIsGameActive);

	//Set saved game ID to current player's end hold slot.
	const DWORD dwEndHoldID = g_pTheDB->SavedGames.FindByEndHold(this->pHold->dwHoldID);
	if (dwEndHoldID) return;	//one already exists -- don't need another
	this->dwSavedGameID = dwEndHoldID;
	this->eType = ST_EndHold;
	this->bIsHidden = true;
	Update();
}

//*****************************************************************************
bool CCurrentGame::IsCurrentRoomConquered() const
{
	return CDbSavedGame::ConqueredRooms.IsIDInList(this->pRoom->dwRoomID);
}

//*****************************************************************************
bool CCurrentGame::IsCurrentRoomExplored() const
{
	return CDbSavedGame::ExploredRooms.IsIDInList(this->pRoom->dwRoomID);
}

//*****************************************************************************
bool CCurrentGame::IsCurrentRoomPendingExit() const
{
    return this->pRoom->wMonsterCount == 0;
}

//*****************************************************************************
void CCurrentGame::SetRoomStatusFromAllSavedGames()
//Set explored status of rooms to include those from all saved games for the
//current player in the current level.
{
	ASSERT(CDbBase::IsOpen());

	//Get a list of all the rooms in the current level.
	CIDList AllRoomsInLevel;
	this->pLevel->Rooms.GetIDs(AllRoomsInLevel);
	
	//Each iteration looks at one saved game for the current level and adds
	//any unique explored or conquered rooms to object members.
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	c4_View ExploredRoomsView;
	const DWORD dwSavedGamesCount = SavedGamesView.GetSize();
   const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	DWORD dwLoopRoomID, dwI, dwRoomCount, dwRoomID;
	for (DWORD dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		if (p_IsHidden(SavedGamesView[dwSavedGamesI]) != 0) continue;
		if (static_cast<DWORD>(p_PlayerID(SavedGamesView[dwSavedGamesI]))
            != dwCurrentPlayerID) continue;

		dwLoopRoomID = p_RoomID(SavedGamesView[dwSavedGamesI]);
		if (AllRoomsInLevel.IsIDInList(dwLoopRoomID))
		{
			//Add any unique explored rooms from this saved game.
			ExploredRoomsView = p_ExploredRooms(SavedGamesView[dwSavedGamesI]);
			dwRoomCount = ExploredRoomsView.GetSize();
			for (dwI = 0; dwI < dwRoomCount; ++dwI)
			{
				dwRoomID = p_RoomID(ExploredRoomsView[dwI]);
				this->ExploredRooms.Add(dwRoomID);
			}
		}
	}
}

//*****************************************************************************
bool CCurrentGame::LoadFromHold(
//Loads current game from the starting level and room of a specified hold.
//
//Params:
	const DWORD dwHoldID,	//(in) Identifies hold that game will begin in.
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first
							//		step into the room.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;

	//Commit any outstanding writes now.
	Commit();

	//End and save a demo if in the middle of recording.
	if (IsDemoRecording() && !EndDemoRecording())
   {
      CFiles Files;
		Files.AppendErrorLog("Failed to save a demo when recording ended.\r\n");
   }

	//Unload an already loaded current game if there is one.
	{
		const DWORD dwAutoSaveOptionsB4 = this->dwAutoSaveOptions;
		Clear();
		this->dwAutoSaveOptions = dwAutoSaveOptionsB4;
	}

	//Load the hold.
	this->pHold = new CDbHold();
	if (!this->pHold) {bSuccess=false; goto Cleanup;}
	bSuccess = this->pHold->Load(dwHoldID);
	if (!bSuccess) goto Cleanup;

	//Load the first level of hold.
	this->pLevel = this->pHold->GetStartingLevel();
	if (!this->pLevel) {bSuccess=false; goto Cleanup;}
	this->wStartRoomO = this->pLevel->wO;
	this->wStartRoomX = this->pLevel->wX;
	this->wStartRoomY = this->pLevel->wY;
	
	//Load the first room of level.
	this->pRoom = this->pLevel->GetStartingRoom();
	if (!this->pRoom) {bSuccess=false; goto Cleanup;}

	//Set swordsman to beginning of room.
	SetSwordsmanToRoomStart();

	//Get members ready.
	SetMembersAfterRoomLoad(CueEvents);

	//Save to level-begin and room-begin slots.
	{
		if ((this->dwAutoSaveOptions & ASO_LEVELBEGIN)==ASO_LEVELBEGIN &&
				!g_pTheDB->SavedGames.FindByLevelBegin(this->pLevel->dwLevelID)) 
			SaveToLevelBegin();
		if ((this->dwAutoSaveOptions & ASO_ROOMBEGIN)==ASO_ROOMBEGIN &&
				!g_pTheDB->SavedGames.FindByRoomBegin(this->pRoom->dwRoomID)) 
			SaveToRoomBegin();
	}
			
Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
bool CCurrentGame::LoadFromLevel(
//Loads current game from starting room of a specified level.  Hold is implied
//by the specified level.
//
//Params:
	const DWORD dwLevelID,	//(in) Identifies level that game will begin in.
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first
							//		step into the room.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwLevelID != 0);

	bool bSuccess=true;

	//Commit any outstanding writes now.
	Commit();

	//End and save a demo if in the middle of recording.
   if (IsDemoRecording() && !EndDemoRecording())
   {
      CFiles Files;
      Files.AppendErrorLog("Failed to save a demo when recording ended.\r\n");
   }

	//Unload an already loaded current game if there is one.
	{
		const DWORD dwAutoSaveOptionsB4 = this->dwAutoSaveOptions;
		Clear();
		this->dwAutoSaveOptions = dwAutoSaveOptionsB4;
	}

	//Load the level.
	this->pLevel = new CDbLevel();
	if (!this->pLevel) {bSuccess=false; goto Cleanup;}
	bSuccess = this->pLevel->Load(dwLevelID);
	if (!bSuccess) goto Cleanup;
	this->wStartRoomO = this->pLevel->wO;
	this->wStartRoomX = this->pLevel->wX;
	this->wStartRoomY = this->pLevel->wY;

	//Load hold associated with level.
	this->pHold = new CDbHold();
	if (!this->pHold) {bSuccess=false; goto Cleanup;}
	bSuccess = this->pHold->Load(pLevel->dwHoldID);
	if (!bSuccess) goto Cleanup;
	
	//Load the first room of level.
	this->pRoom = this->pLevel->GetStartingRoom();
	if (!this->pRoom) {bSuccess=false; goto Cleanup;}

	//Set swordsman to beginning of room.
	SetSwordsmanToRoomStart();

	//Get members ready.
	SetMembersAfterRoomLoad(CueEvents);

	//Save to level-begin and room-begin slots.
	{
		if ((this->dwAutoSaveOptions & ASO_LEVELBEGIN)==ASO_LEVELBEGIN &&
				!g_pTheDB->SavedGames.FindByLevelBegin(this->pLevel->dwLevelID)) 
			SaveToLevelBegin();
		if ((this->dwAutoSaveOptions & ASO_ROOMBEGIN)==ASO_ROOMBEGIN &&
				!g_pTheDB->SavedGames.FindByRoomBegin(this->pRoom->dwRoomID)) 
			SaveToRoomBegin();
	}
			
Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
bool CCurrentGame::LoadFromRoom(
//Loads current game from specified room.  Hold and level are implied
//by the specified room.
//
//Params:
	const DWORD dwRoomID,	//(in) Identifies room that game will begin in.
	CCueEvents &CueEvents,	//(out)	Cue events generated by swordsman's first
							//		step into the room.
	const UINT wX, const UINT wY, const UINT wO)	//(in) Player starting position
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwRoomID);

	bool bSuccess=true;

	//Commit any outstanding writes now.
	Commit();

	//End and save a demo if in the middle of recording.
   if (IsDemoRecording() && !EndDemoRecording())
   {
      CFiles Files;
      Files.AppendErrorLog("Failed to save a demo when recording ended.\r\n");
   }

	//Unload an already loaded current game if there is one.
	Clear();

	//Only checkpoint saves enabled during testing.
	this->dwAutoSaveOptions = ASO_CHECKPOINT;

	//Load the room.
	this->pRoom = new CDbRoom;
	if (!this->pRoom) {bSuccess=false; goto Cleanup;}
	bSuccess = this->pRoom->Load(dwRoomID);
	if (!bSuccess) goto Cleanup;

	//Load level associated with room.
	this->pLevel = new CDbLevel();
	if (!this->pLevel) {bSuccess=false; goto Cleanup;}
	bSuccess = this->pLevel->Load(this->pRoom->dwLevelID);
	if (!bSuccess) goto Cleanup;
	this->wStartRoomO = wO;
	this->wStartRoomX = wX;
	this->wStartRoomY = wY;

	//Load hold associated with level.
	this->pHold = new CDbHold();
	if (!this->pHold) {bSuccess=false; goto Cleanup;}
	bSuccess = this->pHold->Load(this->pLevel->dwHoldID);
	if (!bSuccess) goto Cleanup;
	
	//Set swordsman to beginning of room.
	SetSwordsmanToRoomStart();

	//Get members ready.
	SetMembersAfterRoomLoad(CueEvents);

Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
bool CCurrentGame::LoadFromSavedGame(
//Loads current game from a saved game.
//
//Params:
	const DWORD dwSavedGameID,	//(in) Identifies saved game to load from.
	CCueEvents &CueEvents,		//(out)	Cue events generated by swordsman's last 
								//		step in the room (which may be the first step).
	bool bRestoreAtRoomStart)	//(in)	If true, current game will be loaded to
								//		beginning of room in saved game.  If false (default),
								//		current game will be loaded to the exact room
								//		state specified in the saved game.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;

	//Commit any outstanding writes now.
	Commit();

	//End and save a demo if in the middle of recording.
   if (IsDemoRecording() && !EndDemoRecording())
   {
      CFiles Files;
      Files.AppendErrorLog("Failed to save a demo when recording ended.\r\n");
   }

	//Unload an already loaded current game if there is one.
	{
		const DWORD dwAutoSaveOptionsB4 = this->dwAutoSaveOptions;
		Clear();
		this->dwAutoSaveOptions = dwAutoSaveOptionsB4;
	}

	//Load the saved game.
	bSuccess = CDbSavedGame::Load(dwSavedGameID);
	if (!bSuccess) goto Cleanup;
	this->swordsman.wX = this->swordsman.wPrevX = CDbSavedGame::wStartRoomX;
	this->swordsman.wY = this->swordsman.wPrevY = CDbSavedGame::wStartRoomY;
	this->swordsman.SetOrientation(CDbSavedGame::wStartRoomO);

	//Load the room.
	this->pRoom = CDbSavedGame::GetRoom();
	if (!this->pRoom) goto Cleanup;

	//Load the level.
	this->pLevel = this->pRoom->GetLevel();
	if (!this->pLevel) goto Cleanup;

	//Load the hold.
	this->pHold = this->pLevel->GetHold();
	if (!this->pHold) goto Cleanup;

	//Put room in correct beginning state and get cue events for the 
	//last step the swordsman takes.
	if (bRestoreAtRoomStart || this->Commands.GetSize()==0)
	{
		//Cue events come from first step into the room.
		SetMembersAfterRoomLoad(CueEvents, false);
	}
	else
	{
		//Cue events come from processing of last command below.
		//Ignore cue events from first step into the room.
		CCueEvents IgnoredCueEvents;
		SetMembersAfterRoomLoad(IgnoredCueEvents, false);

		//Play through any commands from the saved game.
		if (!PlayCommands(this->Commands.GetSize(), CueEvents))
		{
#ifdef _DEBUG
			char szMsg[80];
			sprintf(szMsg, "Commands in saved game ID#%lu could not be played back.",
					dwSavedGameID);
         CFiles Files;
         Files.AppendErrorLog(szMsg);
#endif

			//There was an error encountered during playback.
			//Just reload room in its initial state.
			this->swordsman.wX = this->swordsman.wPrevX = CDbSavedGame::wStartRoomX;
			this->swordsman.wY = this->swordsman.wPrevY = CDbSavedGame::wStartRoomY;
			this->swordsman.SetOrientation(CDbSavedGame::wStartRoomO);
			delete this->pRoom;
			this->pRoom = CDbSavedGame::GetRoom();
			SetMembersAfterRoomLoad(CueEvents);
			goto Cleanup;
		}

		//Prepare checkpoint restore, if exists.
      if (this->eType == ST_Checkpoint && this->bOnCheckpoint)
      {
         //This *is* the latest checkpoint saved game.
         this->dwLastCheckpointSavedGameID = this->dwSavedGameID;
      } else {
         //Find the latest checkpoint save.
		   const DWORD dwLatestSavedGame = g_pTheDB->SavedGames.FindByRoomLatest(
               this->pRoom->dwRoomID);
		   if (dwLatestSavedGame)
		   {
			   CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetByID(dwLatestSavedGame);
			   if (pSavedGame->eType == ST_Continue || pSavedGame->eType == ST_Checkpoint)
				   this->dwLastCheckpointSavedGameID = dwLatestSavedGame;
			   delete pSavedGame;
		   }
      }
	}

Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
void CCurrentGame::RestartRoom(
//Restarts the current room.
//
//Params:
	CCueEvents &CueEvents)		//(out)	Cue events generated by swordsman's
								//		first step into the room.
{
	//End and save a demo if in the middle of recording.
   if (IsDemoRecording() && !EndDemoRecording())
   {
      CFiles Files;
      Files.AppendErrorLog("Failed to save a demo when recording ended.\r\n");
   }

	//If this is the first time a new room has been entered, make sure it will
	//also be marked this way after reloading the room.
	if (this->bIsNewRoom)
		this->ExploredRooms.Remove(this->pRoom->dwRoomID);

	ASSERT(this->pRoom);
	this->pRoom->Reload();

	//Move the swordsman back to the beginning of the room.
	SetSwordsmanToRoomStart();
	SetMembersAfterRoomLoad(CueEvents);
}

//*****************************************************************************
void CCurrentGame::RestartRoomFromLastCheckpoint(
//Restart the current room from the saved game associated with the last checkpoint 
//touched in this room.  If no checkpoints have been touched in this room, then
//the player will restart from the beginning of the room.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
{
	const DWORD dwLastCheckpointSavedGameID_ = 
			this->dwLastCheckpointSavedGameID;	//Room load calls erase this.

	//Either reload the room from a saved game or restart the room.
	if (this->dwLastCheckpointSavedGameID)
	{
		VERIFY(LoadFromSavedGame(dwLastCheckpointSavedGameID_,CueEvents,false));
		ASSERT(this->pRoom);
		ASSERT(this->pRoom->GetOSquare(this->swordsman.wX, this->swordsman.wY)==T_CHECKPOINT);
	}
	else
		RestartRoom(CueEvents);
	
	//Restore value.
	this->dwLastCheckpointSavedGameID = 
				dwLastCheckpointSavedGameID_;	
}

//*****************************************************************************
bool CCurrentGame::IsCurrentLevelComplete() const
//For the current level, has the player conquered all required rooms for the 
//completion doors to open?
//
//Returns:
//True if so, false if not.
{
	CIDList requiredRooms;
	this->pLevel->GetRequiredRooms(requiredRooms);
	return CDbSavedGame::ConqueredRooms.ContainsList(requiredRooms);
}

//*****************************************************************************
bool CCurrentGame::IsRoomAtCoordsConquered(
//Determines if a room in the current level has been conquered in the current game.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of room to check.
//
//Returns:
//True if it has, false if it hasn't.
const
{
	const DWORD dwRoomID = this->pLevel->FindRoomIDAtCoords(dwRoomX, dwRoomY);
	return CDbSavedGame::ConqueredRooms.IsIDInList(dwRoomID);
}

//*****************************************************************************
bool CCurrentGame::IsRoomAtCoordsExplored(
//Determines if a room in the current level has been explored in the current game.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of room to check.
const
//
//Returns:
//True if it has, false if it hasn't.
{
	const DWORD dwRoomID = this->pLevel->FindRoomIDAtCoords(dwRoomX, dwRoomY);
	return CDbSavedGame::ExploredRooms.IsIDInList(dwRoomID);
}

//*****************************************************************************
void CCurrentGame::SetCurrentRoomConquered()
//Adds the current room to the list of conquered rooms.
{
	CDbSavedGame::ConqueredRooms.Add(this->pRoom->dwRoomID);
}

//*****************************************************************************
void CCurrentGame::SetCurrentRoomExplored()
//Adds the current room to the list of explored rooms.
{
	CDbSavedGame::ExploredRooms.Add(this->pRoom->dwRoomID);
}

//*****************************************************************************
void CCurrentGame::SetTurn( 
//Sets current game to a specific turn, without removing commands.
// 
//Params: 
	const UINT wTurnNo,			//(in)	Turn to which game will be set.
	CCueEvents &CueEvents)	//(out)	Cue events generated by the last command.
{ 
	ASSERT(this->pRoom);
	ASSERT(wTurnNo < this->Commands.GetSize());
	
	const DWORD dwLastCheckpointSavedGameID_ = 
			this->dwLastCheckpointSavedGameID;	//Room load calls erase this.

	//Freeze commands as a precaution--nothing below should change commands.
	FreezeCommands();

	this->pRoom->Reload();

	//Move the swordsman back to the beginning of the room.
	SetSwordsmanToRoomStart();
	SetMembersAfterRoomLoad(CueEvents, false);
	UnfreezeCommands();

	//Restore last checkpoint.
	this->dwLastCheckpointSavedGameID = dwLastCheckpointSavedGameID_;

	//Play the commands back.
	if (wTurnNo) PlayCommands(wTurnNo, CueEvents);
} 

//*****************************************************************************
bool CCurrentGame::SwordsmanIsDying() const
//Returns: whether the current state indicates the the swordsman is dying
{
	return this->swordsman.bIsDying;
}

//*****************************************************************************
void CCurrentGame::UndoCommands( 
//Undos one or more commands by restarting current room and replaying recorded moves to 
//reach the current turn minus a specified number of "undoed" commands. 
// 
//Params: 
	const UINT wUndoCount,	//(in)	Number of commands to undo.
	CCueEvents &CueEvents)	//(out)	Cue events generated by the new last command.
{ 
	ASSERT(this->pRoom);
	ASSERT(wUndoCount > 0 && wUndoCount <= this->Commands.GetSize());
	UINT wPlayCount = (this->Commands.GetSize() - wUndoCount);

	const DWORD dwLastCheckpointSavedGameID_ = 
			this->dwLastCheckpointSavedGameID;	//Room load calls erase this.

	//Freeze commands as a precaution--nothing below should change commands.
	FreezeCommands();

	//If this is the first time a new room has been entered, make sure it will
	//also be marked this way when reloading this saved game.
	if (this->bIsNewRoom)
		this->ExploredRooms.Remove(this->pRoom->dwRoomID);

	this->pRoom->Reload();

	//Move the swordsman back to the beginning of the room.
	SetSwordsmanToRoomStart();
	SetMembersAfterRoomLoad(CueEvents, false);
	UnfreezeCommands();

	//Restore last checkpoint.
	this->dwLastCheckpointSavedGameID = dwLastCheckpointSavedGameID_;

	//Play the commands back, minus undo count.
	PlayCommands(wPlayCount, CueEvents);
	this->Commands.Truncate(wPlayCount);
} 

//*****************************************************************************
void CCurrentGame::UndoCommand(
//Undoes one command.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Cue events generated by the new last command.
{
	if (this->Commands.GetSize()==0) return; //No commands to undo.
	UndoCommands(1, CueEvents);
}

//
//Private methods.
//

//*****************************************************************************
bool CCurrentGame::SetRoomAtCoords(
//Loads a room on this level and sets the current room to it.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in)	Coords to specify room to load.
//
//Returns:
//True if successful, false if not.  If a failure occurs, the current room will
//stay loaded.
{
	//Load new room.
	CDbRoom *pNewRoom = this->pLevel->GetRoomAtCoords(dwRoomX, dwRoomY);
	if (!pNewRoom)
      return false;

   //Free the old room and return successful.
	delete this->pRoom;
   this->pRoom = pNewRoom;
	return true;
}

//*****************************************************************************
void CCurrentGame::FreezeCommands()
//Disallow modification of command list, i.e. adding commands, clearing, or truncating.  
//Assertians will fire in CDbCommands if this is violated.
{
	//Commands should not be frozen while recording.  Recording requires commands to
	//be added for each call to ProcessCommand().  It is possible to have some carefully
	//thought-out mixture of the two states, in which case this assertian can be changed.
	ASSERT(!this->bIsDemoRecording);

	this->Commands.Freeze();
}

//*****************************************************************************
void CCurrentGame::UnfreezeCommands()
//Allow modification of command list after a call to FreezeCommands().
{
	//Commands should not have been frozen while recording.  Recording requires 
	//commands to be added for each call to ProcessCommand().  It is possible to have 
	//some carefully thought-out mixture of the two states, in which case this assertian 
	//can be changed.
	ASSERT(!this->bIsDemoRecording);

	this->Commands.Unfreeze();
}

//*****************************************************************************
bool CCurrentGame::WalkDownStairs()
//Move swordsman one step down stairs.
//
//Returns: whether the end of the stairs have been reached.
{
	if (this->pRoom->IsValidColRow(this->swordsman.wX, this->swordsman.wY + 1))
	{
		if (pRoom->GetOSquare(this->swordsman.wX, this->swordsman.wY + 1) == T_STAIRS)
		{
			++this->swordsman.wY;
			SetSwordsman(this->swordsman.wX, this->swordsman.wY);
			return true;
		}
	}

	return false;
}

//
// Private methods
//

//***************************************************************************************
void CCurrentGame::AddQuestionsToList(
//Adds questions from CID_MonsterSpoke event to a list of questions.
//
//Params:
	CCueEvents &CueEvents,					//(in)	May contain CID_MonsterSpoke and 
											//		associated questions.
	list<CMonsterMessage> &QuestionList)	//(out)	Questions will be added to it.
const
{
	CMonsterMessage *pMessage = DYN_CAST(CMonsterMessage*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_MonsterSpoke));
	while (pMessage)
	{
		if (pMessage->eType == MMT_YESNO)
			QuestionList.push_back(*pMessage);
		pMessage = DYN_CAST(CMonsterMessage*, CAttachableObject*,
            CueEvents.GetNextPrivateData());
	}
}

//***************************************************************************************
bool CCurrentGame::PlayCommands(
//Play back stored commands to change the game state.  Assumes that the room has been 
//freshly loaded.
//
//Params:
	UINT wCommandCount,		//(in)	Number of commands to play back.
	CCueEvents &CueEvents)	//(out)	Cue events generated by last processed command.
//
//Returns:
//True if commands were successfully played without putting the game into an 
//unexpected state, false if not.
{
	ASSERT(this->wTurnNo == 0);
	ASSERT(wCommandCount <= this->Commands.GetSize());
	
	//While processing the command list, I don't want to take any actions that
	//will modify the command list.
	FreezeCommands();

	COMMANDNODE *pCommand = this->Commands.GetFirst();
	CCueEvents IgnoredCueEvents, *pCueEvents = NULL;
	UINT wCommandI;
	for (wCommandI = 0; wCommandI < wCommandCount; ++wCommandI)
	{
		ASSERT(pCommand);
		
		int nCommand = pCommand->bytCommand;
		pCueEvents = (wCommandI == wCommandCount - 1) ?
			&CueEvents :		//Last command--remember cue events.
			&IgnoredCueEvents;	//Other commands--ignore cue events.
		ProcessCommand(nCommand, *pCueEvents);
				
		//Check for unexpected game states that indicate commands are invalid.
		//Note: a possible reason for getting these errors is that the current version
		//of the app is not compatible with a game previously saved.
		if (!this->bIsGameActive) break;
		if (pCueEvents->HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied)) break;
		if (pCueEvents->HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom)) break;
		
		pCommand = this->Commands.GetNext();
	}
   this->swordsman.wPrevX = this->swordsman.wX;
   this->swordsman.wPrevY = this->swordsman.wY;

	//Allow modification of command list again.
	UnfreezeCommands();

	//Successful return if I processed all the commands.
	return (wCommandI == wCommandCount);
}

//***************************************************************************************
void CCurrentGame::ProcessUnansweredQuestions(
//Processes unanswered questions.  Game is in a state where it is waiting for player
//to answer one of more questions posed by monsters.  If multiple questions have
//been asked, then game will take an answer for the first question asked, then the second, 
//etc.
//
//Params:
	int nCommand,								//(in)		CMD_YES or CMD_NO.
	list<CMonsterMessage> &UnansweredQuestions,	//(in/out)	Receives list of unanswered
												//			questions.  If command is
												//			a valid answer to the first,
												//			the the first question will
												//			be removed from list.
	CCueEvents &CueEvents)						//(out)		Adds cue events as appropriate.
{
	list<CMonsterMessage>::const_iterator iQuestion = UnansweredQuestions.begin();
	if (iQuestion->eType == MMT_YESNO)
	{
		if (nCommand != CMD_YES && nCommand != CMD_NO)
		{
			//This command should not have been sent.  Possible causes:
			//- the front end has been implemented incorrectly and sent a bad command.
			//- a demo has been corrupted or misrecorded.
			ASSERTP(false, "Bad command.");
			this->bIsGameActive = false;
			return;
		}

		//Send answer to question's sender.
		iQuestion->pSender->OnAnswer(nCommand, CueEvents);

		//There should be some kind of shared cue event handler between this
		//and ProcessMonsters().  I happen to know that the only relevant
		//cueevent right now is CID_MonsterDiedFromStab, but that could change.
		CMonster *pStabbedMonster = DYN_CAST(CMonster*, CAttachableObject*,
				CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab));
		while (pStabbedMonster )
		{
			this->pRoom->KillMonster(pStabbedMonster, CueEvents);
			pStabbedMonster = DYN_CAST(CMonster*, CAttachableObject*,
				CueEvents.GetNextPrivateData());
		}

		//Remove the question that was just answered from the list.
		UnansweredQuestions.pop_front();
	}
	else
	{
		ASSERTP(false, "Unexpected question type.");
	}
}

//***************************************************************************************
void CCurrentGame::SetMembersAfterRoomLoad(
//Sets members of current game object that should be changed whenever
//a new room is loaded.
//
//Params:
	CCueEvents &CueEvents,	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
   const bool bResetCommands) //(in) [default = true]
{
	ASSERT(this->pRoom);

   //Reset swordsman stats.
	this->swordsman.bIsPlacingMimic = false;
	this->swordsman.bIsDying = false;
	this->swordsman.bIsVisible = true;

	this->bIsGameActive = true;
	this->wTurnNo = 0;
	this->wSpawnCycleCount = 0;

	this->wMonsterKills = 0L;
	if (!this->pbMonstersKilled)
		this->pbMonstersKilled = new unsigned char[TIRED_TURN_COUNT];
	memset(this->pbMonstersKilled, 0, TIRED_TURN_COUNT * sizeof(unsigned char));
	this->wMonstersKilledRecently = 0L;
	this->bLotsOfMonstersKilled = false;

	this->pRoom->SetCurrentGame(this);
	const DWORD dwCurrentRoomID = this->pRoom->dwRoomID;
	this->dwRoomID = dwCurrentRoomID;

	this->dwPlayerID = g_pTheDB->GetPlayerID();

	//Update demo recording information.
	this->DemoRecInfo.wBeginTurnNo = 0;

	//If room has not been explored before, add it to explored list.
	this->bIsNewRoom = !IsCurrentRoomExplored();
	if (this->bIsNewRoom) SetCurrentRoomExplored();

	//Remove a crumbly wall or monster underneath the swordsman if it exists.
	if (this->pRoom->GetOSquare(this->swordsman.wX, this->swordsman.wY)==T_WALL_B)
		this->pRoom->DestroyCrumblyWall(this->swordsman.wX, this->swordsman.wY, CueEvents);
	if (this->pRoom->GetMonsterAtSquare(this->swordsman.wX, this->swordsman.wY))
      this->pRoom->KillMonsterAtSquare(this->swordsman.wX, this->swordsman.wY, CueEvents);

	//See if room is already conquered.
	bool bWasLevelComplete = IsCurrentLevelComplete();
	if (IsCurrentRoomConquered())
	{
		this->pRoom->RemoveGreenDoors();
		this->pRoom->RemoveSerpents();
		this->pRoom->ClearMonsters();
	}
	//Not conquered, but if no monsters in it then add to conquered list.
	else if (this->pRoom->wMonsterCount == 0)
	{
		CueEvents.Add(CID_ConquerRoom);
		this->pRoom->RemoveGreenDoors();
		SetCurrentRoomConquered();
	}
	//Clear first turn status on new room monsters.
	else
		this->pRoom->ResetMonsterFirstTurnFlags();

	//Remove blue doors if level is complete.
	if (IsCurrentLevelComplete()) 
	{
		if (!bWasLevelComplete) CueEvents.Add(CID_CompleteLevel);
		this->pRoom->RemoveBlueDoors();
	}

	//Setup PathMap for brains.
	if (this->pRoom->IsBrainPresent())
		CreatePathMaps();

	//Remove red doors if no trapdoors in the room.  This would arise from a level 
	//editing error.
	if (pRoom->wTrapDoorsLeft == 0)
		this->pRoom->RemoveRedDoors();

	//Process the swordsman's movement onto the first square.
	ProcessSwordsman(CMD_WAIT, CueEvents);
	ProcessSimultaneousSwordHits(CueEvents);	//destroy simultaneously-stabbed tar
	SetSwordsmanMood(CueEvents);
	this->pRoom->ResetMonsterFirstTurnFlags();

	//Reset checkpoints for this room.
	this->dwLastCheckpointSavedGameID = 0L;

   //Remove any monster messages left unprocessed.
	this->UnansweredQuestions.clear();

   if (bResetCommands)
      this->Commands.Clear();
}

//***************************************************************************************
void CCurrentGame::ProcessMimicPlacement(
//Processes mimic placement (period of the game between player stepping on
//a mimic potion and a mimic being placed in the room at player-specified
//position).
//
//Params:
	int nCommand,			//(in)	Game command.
	CCueEvents &CueEvents)	//(out)	List of events that can be handled by caller.
							//		These are things that the UI wouldn't necessarily
							//		be aware of by looking at the modified game
							//		data on return.
{
	ASSERT(this->swordsman.bIsPlacingMimic);

 	//Figure out how to change swordsman based on command.
 	int dx = 0, dy = 0;
 	switch (nCommand)
 	{
 		case CMD_N: dy = -1; break;
 		case CMD_NW: dx = dy = -1; break;
 		case CMD_NE: dx = 1; dy = -1; break;
 		case CMD_E: dx = 1; break;
 		case CMD_SE: dx = dy = 1; break;
 		case CMD_S: dx = 0; dy = 1; break;
 		case CMD_SW: dx = -1; dy = 1; break;
 		case CMD_W: dx = -1; break;
 	}

 	if (nCommand==CMD_WAIT) // Create a new Mimic
 	{
   		//Check for obstacles in destination square.
 		if (!pRoom->DoesSquareContainMimicPlacementObstacle(this->swordsman.wMimicCursorX, 
				this->swordsman.wMimicCursorY))
 		{
 			this->swordsman.bIsPlacingMimic=false;
 			CMimic *pMimic = DYN_CAST(CMimic *, CMonster *, pRoom->AddNewMonster(M_MIMIC,
					this->swordsman.wMimicCursorX, this->swordsman.wMimicCursorY));

 			if (pMimic) //Yes adding a mimic worked.
 			{
 				pMimic->SetCurrentGame(this);
				pMimic->wO=this->swordsman.wO;
				pMimic->Process(CMD_WAIT, CueEvents);
            if (this->pRoom->IsValidColRow(pMimic->GetSwordX(), pMimic->GetSwordY()))
				   ProcessSwordHit(pMimic->GetSwordX(), pMimic->GetSwordY(), CueEvents, pMimic);
				ProcessSimultaneousSwordHits(CueEvents);	//destroy stabbed tar now
				CueEvents.Add(CID_MimicPlaced);
 			}
 		}
 	}
	//Move mimic placement cursor.
 	else if (dx != 0 || dy != 0)
 	{
 		//Don't allow moving cursor outside of room boundaries.
 		if (!pRoom->IsValidColRow(this->swordsman.wMimicCursorX + dx, this->swordsman.wMimicCursorY + dy))
 		{
 			dx = dy = 0;
 		}
 		else
 		{
 			this->swordsman.wMimicCursorX += dx;
 			this->swordsman.wMimicCursorY += dy;
 		}
 	}
}

//***************************************************************************************
void CCurrentGame::ProcessMonsters(
//Processes all the monsters in the current room.
//
//Params:
	int nLastCommand,		//(in)		Last swordsman command.
	CCueEvents &CueEvents)	//(in/out)	List of events that can be handled by caller.
{
	CMonster *pSeek, *pNextMonster;
	UINT wX, wY;

	//Increment the spawn cycle counter.
	++this->wSpawnCycleCount;

	//Each iteration processes one monster.
	pSeek = this->pRoom->pFirstMonster;
	while (pSeek)
	{
		if (!pSeek->bIsFirstTurn)
		{
			//Recalc pathmaps if needed.
			//(Monster processing may have plotted an obstacle or removed one.)
			CalcPathMaps();

			//Process monster.
			pSeek->Process(nLastCommand, CueEvents);
			
			//Remember the next monster now, because this monster may be dead and
			//removed from the monster list in the next block.
			pNextMonster = pSeek->pNext;
			
			//Check for events stemming from the monster's behavior
			//which require modification of the monster list.
			switch (pSeek->wType)
			{
				case M_MIMIC:
					{
					//Process mimic sword hit.
					CMimic *pMimic = DYN_CAST(CMimic*, CMonster*, pSeek);
					const UINT wMSwordX = pMimic->GetSwordX(), wMSwordY = pMimic->GetSwordY();
					if (this->pRoom->IsValidColRow(wMSwordX, wMSwordY))
					{
						ProcessSwordHit(wMSwordX, wMSwordY, CueEvents, pMimic);
						
						//Update next monster pointer, because the sword hit may have removed 
						//the next monster.
						pNextMonster = pSeek->pNext;
					}
					}
					break;
				case M_NEATHER:
					if (CueEvents.HasOccurredWith(CID_NeatherExitsRoom, pSeek))
						this->pRoom->KillMonster(pSeek, CueEvents);
					break;
				case M_SERPENT:
					if (CueEvents.HasOccurredWith(CID_SnakeDiedFromTruncation, pSeek))
					{
						this->pRoom->KillMonster(pSeek, CueEvents);
						++this->wMonsterKills;	//counts as a kill
					}
					break;
				case M_REGG:
					if (CueEvents.HasOccurredWith(CID_EggHatched, pSeek))
					{
						// Spawn a roach AFTER egg has been removed.
						wX = pSeek->wX;
						wY = pSeek->wY;
						this->pRoom->KillMonster(pSeek, CueEvents);
						CMonster *m = this->pRoom->AddNewMonster(M_ROACH,wX,wY);
						m->SetCurrentGame(this);
						m->bIsFirstTurn = true;
					}
					break;
			}

		}
		else
			pNextMonster = pSeek->pNext;
		
		pSeek = pNextMonster;
	}

	//Clear first turn status of current monsters.  First turn flag indicates that
	//a monster was created and added to the monster list during processing, but is 
	//not ready to be processed yet.  Clearing it here will allow the monster to be
	//processed the next time ProcessMonsters() is called.
	this->pRoom->ResetMonsterFirstTurnFlags();
}

//***************************************************************************************
void CCurrentGame::ProcessSwordHit(
//Processes results of a sword (mimic or swordsman) entering a square.
//
//Params:
	const UINT wSX, const UINT wSY,			//(in)	Square sword is in.
	CCueEvents &CueEvents,		//(out)	List of events that can be handled by caller.
								//		These are things that the UI wouldn't necessarily
								//		be aware of by looking at the modified game
								//		data on return.
	CMimic *pMimic)				//(in)	If NULL (default) this call is checking the 
								//		swordsman's sword.  Otherwise, this will be a 
								//		pointer to a mimic and the call will be checking
								//		that mimic's sword.
{
	ASSERT(this->pRoom->IsValidColRow(wSX, wSY));

	//Did sword hit a monster?
	CMonster *pMonster = this->pRoom->GetMonsterAtSquare(wSX, wSY);
	if (pMonster) //Yes.
	{
		if (pMonster->OnStabbed(CueEvents))
		{
			if (CueEvents.HasOccurredWith(CID_MonsterDiedFromStab, pMonster))
			{
				++this->wMonsterKills;
				this->pRoom->KillMonster(pMonster, CueEvents);
				//store info about stab effect in the monster orientation
				pMonster->wO = GetSwordMovement();
				if (!pMimic)
				{
					++this->wMonstersKilledRecently;
					++this->pbMonstersKilled[this->wTurnNo % TIRED_TURN_COUNT];
				}
			}

			if (pMonster->wType == M_TARMOTHER)
				simulSwordHits.Push(wSX,wSY);	//stab hits tar mother
		}
	}

	//
	//Check for things in T-square for sword to hit.
	//
	UINT wTTileNo = this->pRoom->GetTSquare(wSX, wSY);
	switch (wTTileNo)
	{
		case T_ORB: 
			this->pRoom->ActivateOrb(wSX, wSY, CueEvents, pMimic);
		break;

		case T_TAR:
			if (this->pRoom->StabTar(wSX, wSY, CueEvents, false))	//don't remove tar yet!
				simulSwordHits.Push(wSX,wSY);	//stab hits vulnerable tar
		break;
	}

	//
	//Check for things in O-square for sword to hit.
	//

	//Did sword hit a crumbly wall?
	if (this->pRoom->GetOSquare(wSX, wSY) == T_WALL_B) 
		this->pRoom->DestroyCrumblyWall(wSX, wSY, CueEvents);

	//
	//Mimic-only checks.
	//

	if (pMimic)
	{
		//Hit the swordsman?
		if (wSX == this->swordsman.wX && wSY == this->swordsman.wY)
			CueEvents.Add(CID_MonsterKilledPlayer, pMimic);
	}
}

//***************************************************************************************
void CCurrentGame::ProcessSimultaneousSwordHits(
//Processes results of all swords (mimic and swordsman) entering squares that
//must be stabbed simultaneously.
//
//Params:
	CCueEvents &CueEvents)	//(out)	List of events that can be handled by caller.
							//		These are things that the UI wouldn't necessarily
							//		be aware of by looking at the modified game
							//		data on return.
{
	UINT wSX, wSY;			//Square sword hit is in.

	//NOTE: this is currently only relevant and in effect for tar stabbings
	while (simulSwordHits.GetSize()) {
		simulSwordHits.Pop(wSX,wSY);
		this->pRoom->StabTar(wSX, wSY, CueEvents, true);	//now remove tar
	}
}

//***************************************************************************************
void CCurrentGame::ProcessSwordsman(
//Processes the swordsman.
//
//Params:
	const int nCommand,			//(in)	Game command.
	CCueEvents &CueEvents)	//(out)	List of events that can be handled by caller.
							//		These are things that the UI wouldn't necessarily
							//		be aware of by looking at the modified game
							//		data on return.
{
	//Figure out how to change swordsman based on command.
	int dx = 0, dy = 0;
	switch (nCommand)
	{
		//Rotate swordsman.
		case CMD_C:
			this->swordsman.SetOrientation(nNextCO(this->swordsman.wO));
			CueEvents.Add(CID_SwingSword);
			break;
		case CMD_CC:
			this->swordsman.SetOrientation(nNextCCO(this->swordsman.wO));
			CueEvents.Add(CID_SwingSword);
			break;
		//Move swordsman.
		case CMD_NW: dx = dy = -1; break;
		case CMD_N: dy = -1; break;
		case CMD_NE: dx = 1; dy = -1; break;
		case CMD_W: dx = -1; break;
		case CMD_E: dx = 1; break;
		case CMD_SW: dx = -1; dy = 1; break;
		case CMD_S: dx = 0; dy = 1; break;
		case CMD_SE: dx = dy = 1; break;
	}

	//Calculate sword movement
	this->swordsman.SetSwordMovement(nCommand);

	//Look for obstacles and set dx, dy accordingly.
	if (dx != 0 || dy != 0)
	{
		const UINT wMoveO = nGetO(dx, dy);

		//Check for arrows underfoot.
		if (bIsArrowObstacle(pRoom->GetTSquare(this->swordsman.wX, this->swordsman.wY), wMoveO))
		{
			dx = dy = 0;
			CueEvents.Add(CID_HitObstacle, new CMoveCoord(this->swordsman.wX, this->swordsman.wY, wMoveO), true);
		}
		else
		{
			//Check for leaving room.
			if (!pRoom->IsValidColRow(this->swordsman.wX + dx, this->swordsman.wY + dy))
			{
				if (ProcessSwordsman_HandleLeaveRoom(wMoveO, CueEvents))
					return;

            //Probably a room that had an exit but no room adjacent
				//in that direction, or cannot enter room on that square.
            //Just treat it as an obstacle.
				dx = dy = 0;
			}

			//Check for obstacles in destination square.
			if (pRoom->DoesSquareContainSwordsmanObstacle(
					this->swordsman.wX + dx, this->swordsman.wY + dy, wMoveO))
			{
				if (pRoom->GetOSquare(this->swordsman.wX + dx, this->swordsman.wY + dy)==T_PIT)
					CueEvents.Add(CID_Scared);
				else CueEvents.Add(CID_HitObstacle, 
						new CMoveCoord(this->swordsman.wX + dx, this->swordsman.wY + dy, wMoveO), true);
				dx = dy = 0;
			}
		}
	}

	//Before he moves, remember important square contents.
	const bool bWasOnTrapdoor = (pRoom->GetOSquare(this->swordsman.wX, this->swordsman.wY)==T_TRAPDOOR);
	const bool bWasOnSameScroll = (pRoom->GetTSquare(this->swordsman.wX, this->swordsman.wY)==T_SCROLL) &&
         (dx == 0 && dy == 0);

	//Set swordsman to new location.
	SetSwordsman(this->swordsman.wX + dx, this->swordsman.wY + dy);
	
	//Check for movement off of a trapdoor.
	if (bWasOnTrapdoor && (dx != 0 || dy != 0))
	{
		pRoom->DestroyTrapdoor(this->swordsman.wX - dx, this->swordsman.wY - dy, CueEvents);
	}

	//Check for things that sword could hit.
	if (pRoom->IsValidColRow(this->swordsman.wSwordX, this->swordsman.wSwordY))
		ProcessSwordHit(this->swordsman.wSwordX, this->swordsman.wSwordY, CueEvents);

	//Check for scroll events.
	const UINT wNewTSquare = pRoom->GetTSquare(this->swordsman.wX, this->swordsman.wY);
	if (!bWasOnSameScroll && wNewTSquare == T_SCROLL)
	{
		CDbMessageText *pScrollText = new CDbMessageText();
		*pScrollText = pRoom->GetScrollTextAtSquare(this->swordsman.wX, this->swordsman.wY);
		pScrollText->Cancel();	//don't affect DB
		ASSERT((const WCHAR *)(*pScrollText)); //On assertian, room data is probably stored incorrectly.
		CueEvents.Add(CID_StepOnScroll, pScrollText, true);
	}
	
	//Check for t-square things swordsman can step onto.
	switch(wNewTSquare)
	{
		case T_POTION_K:	//Mimic potion.
 			//yes, drink potion, begin mimic placement
 			//     and init mimic cursor position
 			this->swordsman.bIsPlacingMimic=true;
 			this->swordsman.wMimicCursorX=this->swordsman.wX;
 			this->swordsman.wMimicCursorY=this->swordsman.wY;
 			pRoom->Plot(this->swordsman.wX, this->swordsman.wY, T_EMPTY);
			CueEvents.Add(CID_DrankPotion);
 		break;

		case T_POTION_I:	//Invisibility potion.
			this->swordsman.bIsVisible = !this->swordsman.bIsVisible;   //Toggle effect.
			pRoom->Plot(this->swordsman.wX, this->swordsman.wY, T_EMPTY);
			CueEvents.Add(CID_DrankPotion);
		break;

		default:        //normal step (footfalls)
			if (dx != 0 || dy != 0)
				CueEvents.Add(CID_Step);
		break;
	}

	//Check for o-square things swordsman can step onto.
	const UINT wNewOSquare = pRoom->GetOSquare(this->swordsman.wX, this->swordsman.wY);
	switch (wNewOSquare)
	{
		case T_STAIRS:			//Level exit.
			ProcessSwordsman_HandleLeaveLevel(CueEvents);
		break;
	}
}

//***************************************************************************************
bool CCurrentGame::ProcessSwordsman_HandleLeaveRoom(
//This is a hunk of code yanked out of ProcessSwordsman() for readability.
//
//Beginning state:	Swordsman is standing on the perimeter of a room and player has 
//					issued a command to move out of the room.
//Ending state:		If successful, swordsman will either be positioned in the new room,
//					or the game will be inactive and ready for a new room load.
//Side effects:		Demos may be saved.  
//					Cue events may be added.  
//					Current room may be set to conquered.
//					Game may become inactive if this call is during demo playback.
//
//Params:
	const UINT wMoveO,		//(in)	Direction of movement that leaves the room.
	CCueEvents &CueEvents)	//(out)	Events added to it.
//
//Returns:
//True if swordsman left the room successfully, false if swordsman's departure was
//unsuccessful (ProcessSwordsman() should keep swordsman in current room).
{
	const UINT wExitDirection = GetRoomExitDirection(wMoveO);
   DWORD dwNewSX, dwNewSY;
   CDbRoom *pNewRoom = NULL;
   if (!SwordsmanCanExitRoom(wExitDirection, dwNewSX, dwNewSY, pNewRoom))
   {
      delete pNewRoom;
      return false;
   }

   //Write a demo record if recording.
	if (this->bIsDemoRecording)
	{
		const DWORD dwDemoID = WriteCurrentRoomDemo(this->DemoRecInfo, false, false);

		//Set demo recording info to begin in a new room.
		this->DemoRecInfo.wBeginTurnNo = 0;
		this->DemoRecInfo.dwPrevDemoID = dwDemoID;
	}

	//Was the room conquered on this visit to the room?
	const bool bConquered = WasRoomConqueredOnThisVisit();

	//Do things for a conquered room.
	if (bConquered)
	{
		//Save a conquer demo if I need to.
		if ( (this->dwAutoSaveOptions & ASO_CONQUERDEMO)==ASO_CONQUERDEMO )
			WriteCurrentRoomConquerDemo();

		//Save a highlight demo if I need to.
		if ( (this->dwAutoSaveOptions & ASO_HIGHLIGHTDEMO)==ASO_HIGHLIGHTDEMO &&
				this->HighlightRoomIDs.IsIDInList(this->pRoom->dwRoomID) )
		{
			CHighlightDemoInfo *pHDI = new CHighlightDemoInfo(
					this->pRoom->dwRoomID, WriteCurrentRoomHighlightDemo());
			CueEvents.Add(CID_HighlightDemoSaved, pHDI, true);
		}

		//If player exited the room after killing monsters in the room, then
		//add a cue event and set room's official status to conquered.  For rooms
		//that have no monsters, this would already have happened on the player's
		//first step into the room.
        bool bWasLevelComplete = IsCurrentLevelComplete();
		if (bConquered && !IsCurrentRoomConquered())
		{
			CueEvents.Add(CID_ConquerRoom);
			SetCurrentRoomConquered();
		}

		//If all required rooms have now been conquered, then level is completed.
		if (IsCurrentLevelComplete())
		{
			if (!bWasLevelComplete) CueEvents.Add(CID_CompleteLevel);
			this->pRoom->RemoveBlueDoors();
		}
	}

	//If commands are frozen (demo playback), I can't load a new room, so add 
	//exit pending event and return.
	if (this->Commands.IsFrozen())
	{
		CueEvents.Add(CID_ExitRoomPending, new CAttachableWrapper<UINT>(wMoveO), true);
		this->bIsGameActive = false;	//A load is needed before more commands 
										//can be processed.
      delete pNewRoom;
		return true;
	}

	//Leave the room.
	LoadNewRoomForExit(dwNewSX, dwNewSY, pNewRoom, CueEvents);  //retain pNewRoom
	if ((this->dwAutoSaveOptions & ASO_ROOMBEGIN)==ASO_ROOMBEGIN)
   {
      bool bSaveRoom = false;
      const DWORD dwRoomBeginSavedGameID =
            g_pTheDB->SavedGames.FindByRoomBegin(this->pRoom->dwRoomID);
      if (!dwRoomBeginSavedGameID)   //Save game for newly explored room.
         bSaveRoom = true;
      else if (!IsCurrentRoomConquered())
      {
         //Allow resaving room if it's still not conquered
         //AND
         //has >= the # of conquered rooms as the existing room start saved game.
         CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetByID(dwRoomBeginSavedGameID);
         ASSERT(pSavedGame);
	      const DWORD dwConqueredCount = pSavedGame->ConqueredRooms.GetSize();
         delete pSavedGame;
         if (this->ConqueredRooms.GetSize() >= dwConqueredCount)
            bSaveRoom = true;
      }
      if (bSaveRoom)
         SaveToRoomBegin();
   }
	CueEvents.Add(CID_ExitRoom, new CAttachableWrapper<UINT>(wExitDirection), true);
	return true;
}

//***************************************************************************************
void CCurrentGame::ProcessSwordsman_HandleLeaveLevel(
//This is a hunk of code yanked out of ProcessSwordsman() for readability.
//
//Beginning state:	Swordsman has just moved onto a T_STAIRS square.
//Ending state:		Game will be inactive.  A cue event of either CID_WinGame or
//					CID_ExitLevelPending will have been added.
//Side effects:		Demos may be saved.
//					CID_HighlightDemoSaved could be added.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Events added to it.
{
	//Do things for a conquered room.
	if (WasRoomConqueredOnThisVisit())
	{
		//Save a conquer demo if I need to.
		if ( (this->dwAutoSaveOptions & ASO_CONQUERDEMO)==ASO_CONQUERDEMO )
			WriteCurrentRoomConquerDemo();

		//Save a highlight demo if I need to.
		if ( (this->dwAutoSaveOptions & ASO_HIGHLIGHTDEMO)==ASO_HIGHLIGHTDEMO &&
				this->HighlightRoomIDs.IsIDInList(this->pRoom->dwRoomID) )
		{
			CHighlightDemoInfo *pHDI = new CHighlightDemoInfo(
					this->pRoom->dwRoomID, WriteCurrentRoomHighlightDemo());
			CueEvents.Add(CID_HighlightDemoSaved, pHDI, true);
		}
	}

	//Check for exits.
	DWORD dwNextLevelID=0;
	for (UINT i=0; i<pRoom->Exits.size(); ++i)
	{
		const CExitData &stairs = *(pRoom->Exits[i]);
		if (this->swordsman.wX >= stairs.wLeft && this->swordsman.wX <= stairs.wRight &&
				this->swordsman.wY >= stairs.wTop && this->swordsman.wY <= stairs.wBottom) 
		{
			dwNextLevelID = stairs.dwLevelID;
			break;
		}
	}

	//If no exits, this signifies the end of the hold, i.e.,
	//If there are no levels to go to next, then the game has been won.
	if (dwNextLevelID==0)
	{
		CueEvents.Add(CID_WinGame);
		if (!this->Commands.IsFrozen())
			SaveToEndHold();
	}
	else	//Send back level to go to next.  ProcessCommand() caller
			//is charged with loading the next one if that is appropriate.
		CueEvents.Add(CID_ExitLevelPending,
				new CAttachableWrapper<DWORD>(dwNextLevelID), true);

	//A load is needed before more commands can be processed.
	this->bIsGameActive = false;
}

//***************************************************************************************
bool CCurrentGame::SwordsmanCanExitRoom(
//Returns: whether player can exit this room and enter the next room.
//
//OUT: Loads room adjacent to the current room in the direction of the swordsman's
//exit of the current room.  Swordsman coords are also updated so that he wraps
//to other side in new room.
//
//Params:
   const UINT wDirection,  //(in) direction player is leaving room from
   DWORD &dwNewSX, DWORD &dwNewSY,  //(out) destination room info
   CDbRoom* &pNewRoom)
{
   pNewRoom = NULL;

	//Swordsman should be exiting from proper edge of room.
   //Get coordinates of room being entered.
   DWORD dwNewRoomX, dwNewRoomY;
	switch (wDirection)
	{
		case N:
	      ASSERT(this->swordsman.wY == 0);
         dwNewRoomX = this->pRoom->dwRoomX;
         dwNewRoomY = this->pRoom->dwRoomY - 1;
		   break;
		case S:
      	ASSERT(this->swordsman.wY == this->pRoom->wRoomRows - 1);
         dwNewRoomX = this->pRoom->dwRoomX;
         dwNewRoomY = this->pRoom->dwRoomY + 1;
		   break;
		case W:
	      ASSERT(this->swordsman.wX == 0);
         dwNewRoomX = this->pRoom->dwRoomX - 1;
         dwNewRoomY = this->pRoom->dwRoomY;
		   break;
		case E:
      	ASSERT(this->swordsman.wX == this->pRoom->wRoomCols - 1);
         dwNewRoomX = this->pRoom->dwRoomX + 1;
         dwNewRoomY = this->pRoom->dwRoomY;		
		   break;
		default:
			ASSERTP(false, "Bad direction value.");
		return false;
	}

   //Attempt to load room.
   pNewRoom = this->pLevel->GetRoomAtCoords(dwNewRoomX, dwNewRoomY);
   if (!pNewRoom)
      return false;

   //Put swordsman at opposite edge of new room.
	switch (wDirection)
	{
		case N:
         dwNewSX = this->swordsman.wX;
         dwNewSY = pNewRoom->wRoomRows - 1;
         break;
		case S:
         dwNewSX = this->swordsman.wX;
         dwNewSY = 0;		
         break;
		case W:
         dwNewSX = this->pRoom->wRoomCols - 1;
         dwNewSY = this->swordsman.wY;		
         break;
		case E:
         dwNewSX = 0;
         dwNewSY = this->swordsman.wY;		
         break;
   }

   //Determine whether swordsman can enter room here.
	return pNewRoom->CanSetSwordsman(dwNewSX, dwNewSY, IsRoomAtCoordsConquered(dwNewRoomX, dwNewRoomY));
}

//***************************************************************************************
bool CCurrentGame::WasRoomConqueredOnThisVisit() 
//Was the current room conquered on this visit?  This should be called before the 
//player has left the room.
//
//Returns:
//True if room was conquered on this visit, false if not.
const
{
	ASSERT(this->bIsGameActive);

	if (this->pRoom->wMonsterCount) 
		return false;		//Room is still in an unconquered state.
	else	
	{						//No monsters left in the room.
		if (!IsCurrentRoomConquered() ||
				//If the room isn't already conquered, that means there were monsters 
				//in the room when the player got here, and the player has conquered 
				//the room by killing them all and leaving the room.
				
				this->bIsNewRoom)
				//If the room is already conquered and this is a new room, that means 
				//there were no monsters when the player entered the room, so it was 
				//immediately conquered upon entrance.
			
			return true;
	}
	return false;
}

//***************************************************************************************
bool CCurrentGame::LoadNorthRoom(
//Loads room north of the current room in the context of the swordsman exiting
//from the north end of the room.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
//Returns:
//True if room was successfully loaded, false if not.
{
	//Swordsman should be at north edge.
	ASSERT(this->swordsman.wY == 0);

	//Attempt to load room.
	if (!SetRoomAtCoords(this->pRoom->dwRoomX, this->pRoom->dwRoomY - 1))
		return false;

	//Put swordsman at south edge of new room.
	SetSwordsman(this->swordsman.wX, this->pRoom->wRoomRows - 1);

	//Set start room members.
	SetRoomStartToSwordsman();
	SetMembersAfterRoomLoad(CueEvents);

	return true;
}

//***************************************************************************************
bool CCurrentGame::LoadSouthRoom(
//Loads room south of the current room in the context of the swordsman exiting
//from the south end of the room.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
//Returns:
//True if room was successfully loaded, false if not.
{
	//Swordsman should be at south edge.
	ASSERT(this->swordsman.wY == this->pRoom->wRoomRows - 1);

	//Attempt to load room.
	if (!SetRoomAtCoords(this->pRoom->dwRoomX, this->pRoom->dwRoomY + 1))
		return false;

	//Put swordsman at north edge of new room.
	SetSwordsman(this->swordsman.wX, 0);

	//Set start room members.
	SetRoomStartToSwordsman();
	SetMembersAfterRoomLoad(CueEvents);

	return true;
}

//***************************************************************************************
bool CCurrentGame::LoadWestRoom(
//Loads room west of the current room in the context of the swordsman exiting
//from the west end of the room.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
//
//Returns:
//True if room was successfully loaded, false if not.
{
	//Swordsman should be at west edge.
	ASSERT(this->swordsman.wX == 0);

	//Attempt to load room.
	if (!SetRoomAtCoords(this->pRoom->dwRoomX - 1, this->pRoom->dwRoomY))
		return false;

	//Put swordsman at east edge of new room.
	SetSwordsman(this->pRoom->wRoomCols - 1, this->swordsman.wY);

	//Set start room members.
	SetRoomStartToSwordsman();
	SetMembersAfterRoomLoad(CueEvents);

	return true;
}

//***************************************************************************************
bool CCurrentGame::LoadEastRoom(
//Loads room east of the current room in the context of the swordsman exiting
//from the east end of the room.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Cue events generated by swordsman's first step 
							//		into the room.
//
//Returns:
//True if room was successfully loaded, false if not.
{
	//Swordsman should be at east edge.
	ASSERT(this->swordsman.wX == this->pRoom->wRoomCols - 1);

	//Attempt to load room.
	if (!SetRoomAtCoords(this->pRoom->dwRoomX + 1, this->pRoom->dwRoomY))
		return false;

	//Put swordsman at west edge of new room.
	SetSwordsman(0, this->swordsman.wY);

	//Set start room members.
	SetRoomStartToSwordsman();
	SetMembersAfterRoomLoad(CueEvents);

	return true;
}

//***************************************************************************************
void CCurrentGame::ClearRoomLists()
//Added function to clear the room flags when a new game starts.
{
	CDbSavedGame::ConqueredRooms.Clear();
	CDbSavedGame::ExploredRooms.Clear();
}

//***************************************************************************************
DWORD CCurrentGame::WriteCurrentRoomConquerDemo()
//Writes a demo to show this room being conquered.
//
//Returns:
//DemoID of new Demos record.
{
	//Set recording information for a conquer demo.
	DEMO_REC_INFO dri;
	dri.dwDescriptionMessageID = MID_ConquerDemoDescription;
	dri.wBeginTurnNo = 0;
	dri.dwPrevDemoID = 0L;
	dri.dwFirstDemoID = 0L;

	//This call does the real work.
	return WriteCurrentRoomDemo(dri);
}

//***************************************************************************************
DWORD CCurrentGame::WriteCurrentRoomDieDemo()
//Writes a demo to show player dieing in this room.
//
//Returns:
//DemoID of new Demos record.
{
	//Set recording information for a conquer demo.
	DEMO_REC_INFO dri;
	dri.dwDescriptionMessageID = MID_DieDemoDescription;
	dri.wBeginTurnNo = 0;
	dri.dwPrevDemoID = 0L;
	dri.dwFirstDemoID = 0L;

	//This call does the real work.
	return WriteCurrentRoomDemo(dri);
}

//***************************************************************************************
DWORD CCurrentGame::WriteCurrentRoomHighlightDemo()
//Writes a demo to show this highlighted room being conquered.
//
//Returns:
//DemoID of new Demos record.
{
	//Set recording information for a highlight demo.
	DEMO_REC_INFO dri;
	dri.dwDescriptionMessageID = MID_NoText;
	dri.wBeginTurnNo = 0;
	dri.dwPrevDemoID = 0L;
	dri.dwFirstDemoID = 0L;

	//This call does the real work.
	return WriteCurrentRoomDemo(dri, true, false);
}

//***************************************************************************************
DWORD CCurrentGame::WriteCurrentRoomDemo(
//Update database with demo information for current room.
//
//Note: Logic for updating demo descriptions in this method depends on this method
//being the only updater of demo descriptions.
//
//Params:
	DEMO_REC_INFO &dri,	//(in/out)	Receives with members set with info needed to
						//			record the demo.  Returns with with values that 
						//			can be used in a subsequent call for a multi-room 
						//			demo.
	const bool bHidden,		//(in)		Make the demo hidden?  Default is false.
   const bool bAppendRoomLocation)  //(in) Add room location to description (default = true)
//
//Returns:
//DemoID of new Demos record.
{
	ASSERT(this->wTurnNo > dri.wBeginTurnNo);
	ASSERT(dri.dwDescriptionMessageID);

	//Save the current game to a new slot.
	this->dwSavedGameID = 0L;
	this->eType = ST_Demo;
	this->bIsHidden = true;
	Update();
	ASSERT(this->dwSavedGameID);

	//Get a new demo and set its properties.
	CDbDemo *pDemo = new CDbDemo;
	pDemo->bIsHidden = bHidden;
	pDemo->dwSavedGameID = this->dwSavedGameID;
	pDemo->wBeginTurnNo = dri.wBeginTurnNo;
	pDemo->wEndTurnNo = this->wTurnNo - 1;

   //Prepend room position to non-empty (player viewable) demo descriptions.
   WCHAR *pText = g_pTheDB->GetMessageText(dri.dwDescriptionMessageID);
   WSTRING descText;
   if (bAppendRoomLocation && !dri.dwPrevDemoID)
   {
      descText = AbbrevRoomLocation();
      descText += wszColon;
      descText += wszSpace;
   }
   descText += pText;
	pDemo->DescriptionText = descText.c_str();

	if (dri.dwPrevDemoID)
	{
		WSTRING wstrNewDescription = (const WCHAR *)pDemo->DescriptionText;
		if (dri.dwPrevDemoID == dri.dwFirstDemoID) //2nd demo.
		{
			WSTRING wTmp;
			AsciiToUnicode( " (2)", wTmp );
			wstrNewDescription += wTmp;
		}
		else //3rd or later demo.
		{
			//Parse number from description text.
			WSTRING wstrNum;
			WSTRING::iterator iNumStart = wstrNewDescription.end();
			WSTRING::iterator iNumStop = wstrNewDescription.end();
			WSTRING wTmp;
			AsciiToUnicode("()", wTmp);
         WSTRING::iterator iSeek;
			for (iSeek = wstrNewDescription.end() - 1; iSeek != wstrNewDescription.begin(); --iSeek)
			{
				if ((WCHAR)*iSeek == wTmp[1]) {iNumStop = iSeek; continue;}
				if ((WCHAR)*iSeek == wTmp[0]) {iNumStart = iSeek + 1; break;}
			}
         for (iSeek = iNumStart; iSeek != iNumStop; ++iSeek)
			   wstrNum += *iSeek;
			int nNum = _Wtoi(wstrNum.c_str());

			//Parsing error or previous call didn't append "(n)".
			ASSERT(nNum != 0);
			ASSERT((iNumStart != wstrNewDescription.end()) &&
				   (iNumStop != wstrNewDescription.end()));

			//Concat new description with incremented number.
			WCHAR wczNewNum[7];
			_itoW(nNum + 1, wczNewNum, 10);
			wstrNewDescription.replace(iNumStart, iNumStop, wczNewNum);
		}
		pDemo->DescriptionText.BindNew(); //A new text will be created for this demo in Update().
		pDemo->DescriptionText = wstrNewDescription.c_str();
		dri.dwDescriptionMessageID = pDemo->DescriptionText.Flush();
	}
	pDemo->dwChecksum = GetChecksum();

	//Update the demo.
	pDemo->Update();
	ASSERT(pDemo->dwDemoID);

	//If recording began in a previous room, update the demo record for that room
	//so that its NextDemoID field will point to this new demo record.
	if (dri.dwPrevDemoID)
	{
		CDbDemo *pPrevDemo = g_pTheDB->Demos.GetByID(dri.dwPrevDemoID);
		ASSERT(pPrevDemo); //If fires, then probably a bad value in dwPrevDemoID.
		ASSERT(pPrevDemo->dwNextDemoID == 0L);
		pPrevDemo->dwNextDemoID = pDemo->dwDemoID;
		if (pPrevDemo->dwDemoID == dri.dwFirstDemoID) //First demo.
		{
			//Add "(1)" to description to indicate multi-room demo.
			WSTRING wstrNewDescription = (const WCHAR *)pPrevDemo->DescriptionText;
			WSTRING wTmp;
			AsciiToUnicode( " (1)", wTmp );
			wstrNewDescription += wTmp;
			pPrevDemo->DescriptionText = wstrNewDescription.c_str();
		}
		pPrevDemo->Update();
		delete pPrevDemo;
	}

	//If no first demo ID is stored then this is it.
	if (!dri.dwFirstDemoID)
		dri.dwFirstDemoID = pDemo->dwDemoID;

	const DWORD dwNewDemoID = pDemo->dwDemoID;
	delete pDemo;
	return dwNewDemoID;
}

//***************************************************************************************
void CCurrentGame::SetSwordsmanToRoomStart()
//Sets swordsman-related members to start of room.
{
	this->swordsman.SetOrientation(this->wStartRoomO);
	SetSwordsman(this->wStartRoomX, this->wStartRoomY);
}

//***************************************************************************************
void CCurrentGame::SetRoomStartToSwordsman()
//Opposite of SetSwordsmanToRoomStart().
{
	this->wStartRoomO = this->swordsman.wO;
	this->wStartRoomX = this->swordsman.wX;
	this->wStartRoomY = this->swordsman.wY;
}

//***************************************************************************************
void CCurrentGame::CalcPathMaps()
//Calculate PathMaps for each movement ability type.
//
//NOTE: Should only need to be done when a brain can sense the swordsman and
//provide monsters with smart movement information.
{
   this->bBrainSensesSwordsman = this->pRoom->BrainSensesSwordsman();
   if (this->bBrainSensesSwordsman)
   {
	   for (int n=0; n<NumMovementTypes; n++)
		   if (this->pRoom->pPathMap[n])
			   this->pRoom->pPathMap[n]->CalcPaths();
   }
}

//***************************************************************************************
bool CCurrentGame::MonsterWithMovementTypeExists(
//Returns: whether a monster with 'eMovement' type exists.
//
//Params:
	const MovementType eMovement)	//(in)
const
{
	CMonster *pMonster = this->pRoom->pFirstMonster;
	while (pMonster)
	{
		if (pMonster->eMovement == eMovement)
			return true;
		pMonster = pMonster->pNext;
	}
	
	return false;
}

//***************************************************************************************
void CCurrentGame::CreatePathMaps()
//Create PathMap for each movement ability type.
{
	//Always create first pathmap (as there's always GROUND monsters
	//in the room, i.e. the brain!)
	this->pRoom->CreatePathMap(this->swordsman.wX, this->swordsman.wY, (MovementType)0);

	//Generate other path maps as needed.
	for (int n=1; n<NumMovementTypes; n++)
		if (MonsterWithMovementTypeExists((MovementType)n))
			this->pRoom->CreatePathMap(this->swordsman.wX, this->swordsman.wY, (MovementType)n);
}

//***************************************************************************************
void CCurrentGame::SetPathMapsTarget(
//Set PathMap target for each movement ability type.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Target for each pathmaps
{
	POINT p = {wX, wY};
	for (int n=0; n<NumMovementTypes; n++)
		if (this->pRoom->pPathMap[n])
			this->pRoom->pPathMap[n]->SetTarget(p);
}

//***************************************************************************************
bool CCurrentGame::IsSwordsmanTired()
//Returns: Whether swordsman has just finished a long job
//and is breathing a sigh of exhausted relief. 
{
	static const float KILL_RATIO = 0.70f;
	static const UINT NO_MONSTER_RADIUS = 3;

	//Criteria:
	//1. At least KILL_RATIO % of TIRED_TURN_COUNT turns was spent
	//   killing monsters.
	if (!this->bLotsOfMonstersKilled)
	{
		if (this->wMonstersKilledRecently < TIRED_TURN_COUNT * KILL_RATIO)
			return false;
		this->bLotsOfMonstersKilled = true;
	}

	//2. At least some monsters have been killed recently.
	if (this->wMonstersKilledRecently == 0)
	{
		//Too much time has elapsed, so (1) doesn't count any more.
		this->bLotsOfMonstersKilled = false;
		return false;
	}

	//3. No monsters are now within NO_MONSTER_RADIUS squares of player.
	const bool bTired = !(this->pRoom->IsMonsterWithin(this->swordsman.wX,this->swordsman.wY,NO_MONSTER_RADIUS));

	if (bTired)
	{
		//Reset all "tired" vars to prevent sound repeating in consecutive turns.
		this->bLotsOfMonstersKilled = false;
		this->wMonstersKilledRecently = 0;
		memset(this->pbMonstersKilled,0,TIRED_TURN_COUNT * sizeof(unsigned char));
		return true;
	}
	return false;
}

// $Log: CurrentGame.cpp,v $
// Revision 1.175  2005/03/19 21:06:51  mrimer
// Fixed bug: victory demo is not recorded when restarting or restoring to a checkpoint save in an empty room.
//
// Revision 1.174  2005/03/16 02:22:36  mrimer
// Fixed bug: demos save progress to player.
//
// Revision 1.173  2005/03/15 21:50:34  mrimer
// More error checking.
//
// Revision 1.172  2003/10/20 17:49:02  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.171  2003/10/07 21:10:34  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.170  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.169  2003/09/19 23:15:08  mrimer
// Fixed bug: Beethro laughs at mimic death.
//
// Revision 1.168  2003/09/16 19:34:18  mrimer
// Fixed bug: not checking for monster removal when checking for room cleaned event.
//
// Revision 1.167  2003/09/16 16:11:02  mrimer
// Fixed bug: last monsters killed as new ones are created sends room cleared event.
//
// Revision 1.166  2003/09/12 17:26:34  mrimer
// Fixed demo enumeration and description problems.
//
// Revision 1.165  2003/09/11 17:37:45  mrimer
// Fixed bug: can't enter conquered room where serpent was on screen edge
//
// Revision 1.164  2003/08/23 19:28:10  mrimer
// Finished previous fix.
//
// Revision 1.163  2003/08/23 15:45:02  mrimer
// Fixed bug: level start screen shows when player continues in entrance room after reentering room.
//
// Revision 1.162  2003/08/23 15:25:33  mrimer
// Fixed bugs: room conquered list for saved game on restore screen gets replaced with another list; checkpoints don't work when restoring to broken checkpoint save.
//
// Revision 1.161  2003/08/20 22:13:37  mrimer
// Revised saved game handling.
//
// Revision 1.160  2003/08/07 16:08:46  mrimer
// Fixed bug: mimic stabbing player not registering as player death in SwordsmanIsDying().
//
// Revision 1.159  2003/07/28 21:21:22  erikh2000
// Fixed double exit bug.
//
// Revision 1.158  2003/07/24 21:30:35  mrimer
// Fixed a memory leak.
//
// Revision 1.157  2003/07/24 19:44:54  mrimer
// Fixed bug: duplicate room location description on multi-room demos.
//
// Revision 1.156  2003/07/24 01:30:06  mrimer
// Changed room exits data structure to vector.
//
// Revision 1.155  2003/07/23 20:24:21  mrimer
// Now output error log for demos not playing to the end only in _DEBUG builds.
//
// Revision 1.154  2003/07/22 18:37:07  mrimer
// Changed reinterpret_casts to DYN_CAST.
//
// Revision 1.153  2003/07/21 22:02:26  mrimer
// Fixed bug: stepping on bug on entrance doesn't open green doors.
//
// Revision 1.152  2003/07/19 21:21:58  mrimer
// Fixed bugs: extra room location being added to demo description; latest room warp change incorrect.
//
// Revision 1.151  2003/07/19 02:29:49  mrimer
// Added LoadNewRoomForExit() and SwordsmanCanExitRoom().  Fixed bug: exit room code executed even when room can't be left.
//
// Revision 1.150  2003/07/17 02:15:35  mrimer
// Fixed bug: mimic placement on edge of screen crashes DROD.
//
// Revision 1.149  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.148  2003/07/14 16:37:01  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.147  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.146  2003/07/07 23:46:56  mrimer
// Fixed bug: selecting wrong checkpoint to restart to.
// Restricted when room start saved games can be re-saved.
//
// Revision 1.145  2003/07/06 04:54:05  mrimer
// Update player timestamp on continue save.
//
// Revision 1.144  2003/07/03 21:41:47  mrimer
// Added AbbrevRoomLocation() to facilitate more descriptive naming of automatically saved demos.
//
// Revision 1.143  2003/07/02 01:11:05  mrimer
// Fixed bug: stabbing Neather playtesting in final room can cause crash.
//
// Revision 1.142  2003/07/01 20:23:20  mrimer
// Changed room start saving to always (re)save while room hasn't been conquered yet.
//
// Revision 1.141  2003/06/27 19:16:11  mrimer
// Enabled checkpoints during play-testing (using temporary player that gets deleted after playtesting).
//
// Revision 1.140  2003/06/26 17:39:21  mrimer
// Added: filter map rooms by current player only, and remove monster player is standing on on entering room.
//
// Revision 1.139  2003/06/20 23:49:44  mrimer
// Added bBrainSensesSwordsman and query logic.
//
// Revision 1.138  2003/06/19 20:57:03  mrimer
// Fixed bug: stepping on adjacent scrolls not recognized.
//
// Revision 1.137  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.136  2003/06/18 21:22:23  schik
// Added visual cue when room is in conquered state.
//
// Revision 1.135  2003/06/16 23:36:38  mrimer
// Added prev position vars.  Fixed serpent error.
//
// Revision 1.134  2003/05/28 23:05:10  erikh2000
// CFiles::GetDatPath() is called differently.
//
// Revision 1.133  2003/05/25 22:46:24  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.132  2003/05/23 21:30:36  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.131  2003/05/22 23:39:01  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.130  2003/05/08 22:01:07  mrimer
// Replaced local CDb instances with a pointer to global instance.
//
// Revision 1.129  2003/05/03 23:29:23  mrimer
// Added support for a new save game type: ST_EndHold, that gets saved when a
// player exits a hold, to mark that they have completed it.
//
// Revision 1.128  2003/04/17 20:59:48  mrimer
// Changed logic for recognizing when the current level is completed.
//
// Revision 1.127  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.126  2003/02/24 17:06:22  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.125  2003/02/17 00:48:11  erikh2000
// Remove L" string literals.
//
// Revision 1.124  2003/02/16 20:29:31  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.123  2003/02/12 19:58:03  mrimer
// Fixed assertion on leaving room diagonally.
//
// Revision 1.122  2003/01/08 00:50:48  mrimer
// Fixed a bug.
//
// Revision 1.121  2003/01/04 23:07:31  mrimer
// Updated swordsman interface.
//
// Revision 1.120  2002/12/22 02:14:50  mrimer
// Revised swordsman vars.
// Made error handling of replaying an invalid command list more robust.
// Fixed potential bugs in multiple player record handling.
// Revised logic for sending ExitLevelPending cue.
//
// Revision 1.119  2002/11/22 02:07:20  mrimer
// Added LoadFromRoom().  Made some vars const.  Cleaned up old code.  Revised calls to CPathMap.
//
// Revision 1.118  2002/11/15 02:12:19  mrimer
// Revised SaveToContinue() to lookup current player's continue saved game.
// Enhanced SetSwordsmanMood().
// Replaced plot history stuff with a dirty bit.
// Moved GetLevelPositionDescription_English(), etc. into CDbRoom.
// Made several vars and parameters const.
// Removed unneeded includes.
//
// Revision 1.117  2002/10/21 20:13:43  mrimer
// Added bOnCheckpoint.  Checkpoints are now saved only when stepping onto a checkpoint (not standing on it).
//
// Revision 1.116  2002/10/17 18:11:20  mrimer
// Re-inserted accidentally removed fix for not restarting at checkpoints.
//
// Revision 1.115  2002/10/17 16:46:56  mrimer
// Added wMonsterKills.
//
// Revision 1.114  2002/10/16 01:28:23  erikh2000
// CID_StepOffScroll is no longer added.
//
// Revision 1.113  2002/10/15 23:08:00  erikh2000
// Checkpoint cue event will get added when swordsman steps on checkpoint during demo playback.
//
// Revision 1.112  2002/10/15 20:34:09  erikh2000
// Fixed problems with auto-save options getting lost.  (Victory and death demos not being saved.)
//
// Revision 1.111  2002/10/14 21:14:05  mrimer
// Fixed bug: placing mimic doesn't stab tar immediately.
//
// Revision 1.110  2002/10/14 17:21:31  mrimer
// Fixed bug: not restarting at checkpoint after restoring to checkpoint save.
//
// Revision 1.109  2002/10/11 17:35:06  mrimer
// Fixed bug: not restarting to checkpoint after restoring game.
//
// Revision 1.108  2002/10/09 00:54:23  erikh2000
// Reaaranged some code for readability.
// Conquer and highlight demos may now be saved when swordsman exits a level.
//
// Revision 1.107  2002/10/03 21:43:58  mrimer
// Moved wSpawnCycleCount increment into ProcessMonsters().
//
// Revision 1.106  2002/10/01 22:34:57  erikh2000
// If current game has commands frozen when it is destroyed, that is now okay (no assertian will fire).
//
// Revision 1.105  2002/09/06 20:06:24  erikh2000
// Rerenamed "CtagHighlightDemoInfo" to "CHighlightDemoInfo".
//
// Revision 1.104  2002/09/03 22:36:56  mrimer
// Moved an assertion.
//
// Revision 1.103  2002/09/03 21:43:39  mrimer
// Fixed bug: tar not being stabbed when entering room.
//
// Revision 1.102  2002/09/01 00:04:08  erikh2000
// Split wTurnsTaken into wSpawnCycleCount and wTurnNo.  wSpawnCycleCount is incremented less often than wTurnNo, and is used for determining spawning related events.  wTurnNo is used for everything else.
//
// Revision 1.101  2002/08/29 22:00:35  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.100  2002/08/28 21:36:22  erikh2000
// A cue event is now added when green doors open.
//
// Revision 1.99  2002/08/28 20:27:53  mrimer
// Added SwordsmanTired event logic.
//
// Revision 1.98  2002/07/25 18:53:07  mrimer
// Added CID_CheckpointActivated logic.
//
// Revision 1.97  2002/07/25 17:28:34  mrimer
// Fixed bug with adding scroll text to cue event.
//
// Revision 1.96  2002/07/23 20:12:00  mrimer
// Tweaking.
//
// Revision 1.95  2002/07/22 00:52:53  erikh2000
// Removed assertian because access violation is obvious enough.
//
// Revision 1.94  2002/07/19 20:24:35  mrimer
// Modified CCueEvents to work with CAttachableObject.
//
// Revision 1.93  2002/07/17 02:13:03  erikh2000
// Fixed a call to changed CDbRoom::GrowTar() method.
//
// Revision 1.92  2002/07/10 03:55:35  erikh2000
// Moved room exiting logic into a separate function called by ProcessSwordsman().
// Room criteria for highlight demos can be set.
// Highlight demos are automatically saved.
//
// Revision 1.91  2002/07/03 21:58:49  mrimer
// Revised SetRoomStatusFromAllSavedGames() to show conquered rooms based on current room.
//
// Revision 1.90  2002/06/24 20:33:29  mrimer
// Bug fix in pathmap calculation.
//
// Revision 1.89  2002/06/23 10:46:37  erikh2000
// Added method to determine if game is at start of a level.
//
// Revision 1.88  2002/06/22 21:05:35  erikh2000
// Fixed bug where monsters are sometimes processed after they are dead.
//
// Revision 1.87  2002/06/22 05:48:08  erikh2000
// Changed code to use new CMimic methods for getting sword coords.
//
// Revision 1.86  2002/06/21 22:46:28  mrimer
// Added MonsterWithMovementTypeExists() to determine which pathmaps need to be created.
//
// Revision 1.85  2002/06/21 04:31:06  mrimer
// Added multiple pathmap support.
//
// Revision 1.84  2002/06/21 03:11:02  erikh2000
// Fixed a handful of errors checking auto-save flags.
//
// Revision 1.83  2002/06/20 04:03:25  erikh2000
// Changed methods to return wstrings instead of using WCHAR arrays.
//
// Revision 1.82  2002/06/16 22:02:46  erikh2000
// Added automatic demo-saving for player deaths and conquered rooms.
//
// Revision 1.81  2002/06/15 18:20:28  erikh2000
// Corrected demo writing code to support multiple rooms without errors.
//
// Revision 1.80  2002/06/13 21:39:51  mrimer
// Modified EndDemoRecording() to return special flag when no moves are recorded.
//
// Revision 1.79  2002/06/09 18:56:10  erikh2000
// Fixed a problem with checkpoint-saving.
//
// Revision 1.78  2002/06/09 06:04:02  erikh2000
// Changed some message-text handling code.
// Moved checkpoint saving code so that it occurs only if player doesn't die.
//
// Revision 1.77  2002/05/21 19:03:42  erikh2000
// Made changes related to demo recording.
//
// Revision 1.76  2002/05/16 17:02:30  mrimer
// Placing mimic so its sword is on player will now kill player.
//
// Revision 1.75  2002/05/15 23:43:28  mrimer
// Completed exit level sequence.
//
// Revision 1.74  2002/05/15 01:16:31  erikh2000
// Made changes related to recording demos.
//
// Revision 1.73  2002/05/14 22:05:04  mrimer
// Wrote part of animating the swordsman walking down the stairs.
//
// Revision 1.72  2002/05/14 17:22:50  mrimer
// Now basing particle explosion direction on sword movement (not orientation).
// Changed tar babies' stab appearance to TarStabEffect.
//
// Revision 1.71  2002/05/14 15:54:33  mrimer
// Fix bug: Tar baby formed from tar mother stab.
//
// Revision 1.70  2002/05/12 03:08:09  erikh2000
// Made changes in how automatic games are saved.  It is now possible to disable all or some of the automatic saving behaviour.
//
// Revision 1.69  2002/05/10 22:30:38  erikh2000
// Changed rules for saving to room-begin slot.
//
// Revision 1.68  2002/04/28 23:44:02  erikh2000
// Changed level position description routine to not include level at beginning.
// Added method to set explored/conquered rooms to a superset of statuses from all saved games.
//
// Revision 1.67  2002/04/22 21:52:29  mrimer
// Augmented CID_CrumblyWallDestroyed event to include player orientation info.
//
// Revision 1.66  2002/04/20 08:17:49  erikh2000
// Changed GetLevelPositionDescription() to format text differently and fixed a bug.
//
// Revision 1.65  2002/04/18 17:47:10  mrimer
// Updated SetSwordsmanMood() to add only one mood event at a time.
//
// Revision 1.64  2002/04/12 21:48:27  mrimer
// Added CID_SwordsmanAfraid and CID_SwordsmanAggressive events
// and code to calculate when they occur.
// Added simultaneous tar stabbings.
//
// Revision 1.63  2002/04/12 05:11:02  erikh2000
// Changed orb hit cue event adds to use CMoveCoord instead of CCoord.
//
// Revision 1.62  2002/03/30 21:53:24  erikh2000
// Made a few changes to the checkpoint save and restore functions to avoid doing unnecessary work.
//
// Revision 1.61  2002/03/30 05:10:34  erikh2000
// Wrote RestartRoomFromLastCheckpoint().  (Committed on behalf of mrimer.)
//
// Revision 1.60  2002/03/17 23:04:36  erikh2000
// Added RestartRoomFromLastCheckpoint() stub.
//
// Revision 1.59  2002/03/16 12:20:47  erikh2000
// Changes made for GetLevelPosition() to work.  (Committed on behalf of mrimer.)
//
// Revision 1.58  2002/03/16 11:44:48  erikh2000
// Wrote code in several places to add cue events in response to game events.  (Committed on behalf of mrimer.)
//
// Revision 1.57  2002/03/04 22:21:57  erikh2000
// Moved T_CHECKPOINT to opaque layer.
// Changed obstacle logic to evaluate T_CHECKPOINT as clear.
//
// Revision 1.56  2002/02/28 04:51:44  erikh2000
// Removed Save().
// Added SaveToRoomBegin(), SaveToLevelBegin(), and SaveToCheckpoint() and calls to them.
//
// Revision 1.55  2002/02/27 23:51:52  erikh2000
// Changed SetLevelToDefaultNext() to not attempt loading a level with a bad level ID.
// Added calls to CDbRoom::ResetMonsterFirstTurnFlags().
//
// Revision 1.54  2002/02/26 11:47:46  erikh2000
// Set bIsGameActive member to false for CID_ExitRoomPending event.
// Added state-checking assertians in FreezeCommands(), UnfreezeCommands(), BeginRecordingDemo(), and EndRecordingDemo().
// Changed WriteCurrentDemo() to store first demo ID in DemoRecInfo for a series.
// Changed EndDemoRecording() to return demo ID of first demo in a series.
// Changed destructor to end demo recording automatically.
// Changed placement of turns taken incrementor to keep its value correct.
// Changed SetMembersAfterRoomLoad() to add a CID_ConquerRoom and CID_CompleteLevel cue events.
//
// Revision 1.53  2002/02/25 03:37:42  erikh2000
// Wrote UndoCommand() and UndoCommands().
// Changed ProcessMonsters() to recalc pathmap as needed before each monster is processed.
//
// Revision 1.52  2002/02/24 03:44:33  erikh2000
// Wrote ProcessSwordHit() method and moved code from ProcessSwordsman() into it.
// Added call to ProcessSwordHit() for mimics in ProcessMonsters().
// Changed ProcessMimicPlacement() to use new DoesSquareContainMimicPlacementObstacle() method for obstacle check and added ProcessSwordHit() call.
//
// Revision 1.51  2002/02/24 02:02:43  erikh2000
// ProcessSwordsman() now adds a CID_Crumbly WallDestroyed cue event.
//
// Revision 1.50  2002/02/24 01:24:26  erikh2000
// Fixed problem with sprintf format string.
// Put code in SetMembersAfterRoomLoad() to handle consequences of swordsman's entrance into room.
// Removed code from ProcessSwordsman() that is now present in SetMembersAfterRoomLoad().
// Added CueEvents parameter to several CCurrentGame methods related to room loading.  Whenever a room is loaded in CCurrentGame, the swordsman is taking a step into the room that can cause things to happen.
// Removed some redundant code from SetRoomAtCoords().
//
// Revision 1.49  2002/02/23 04:59:43  erikh2000
// Renamed CID_LevelExit reference to "CID_LevelExitPending".
// Added FreezeCommands() and UnfreezeCommands() methods.
// Removed bAddCommands parameter from ProcessCommand() because Freeze/UnfreezeCommands() provide better solution.
// Rewrote PlayCommands() to use Freeze/UnfreezeCommands() and fail gracefully if commands are invalid.
//
// Revision 1.48  2002/02/16 03:17:18  erikh2000
// Correct rules for conquering rooms to require swordsman exits a room after killing all monsters.
//
// Revision 1.47  2002/02/15 02:46:27  erikh2000
// Added option to restore from beginning of room to CDb::LoadFromSavedGame().
//
// Revision 1.46  2002/02/13 01:05:26  erikh2000
// Changed CCurrentGame::EndDemoRecording() to return DemoID of new demo.
//
// Revision 1.45  2002/02/10 03:58:35  erikh2000
// Fixed logic errors in ProcessUnansweredQuestions().
//
// Revision 1.44  2002/02/10 02:34:52  erikh2000
// Wrote CCurrentGame::PlayCommands().
// Wrote CCurrentGame::SaveToContinue().
// Wrote code to play through commands when loading from saved game.
// Moved CDbSavedGame::wTurnsTaken, wX, wY, wO, bIsPlacingMimic, wMimicCursorX, wMimicCursorY, and bIsVisible to CCurrentGame.
//
// Revision 1.43  2002/02/09 02:15:12  erikh2000
// Reenabled call to Commands.Add().
//
// Revision 1.42  2002/02/09 00:58:46  erikh2000
// Moved md5i's CCurrentGame::StabTar() to CDbRoom::StabTar().
// Moved md5i's CCurrentGame::GrowTar() to CDbRoom::GrowTar().
//
// Revision 1.41  2002/02/08 23:19:16  erikh2000
// Changed CDbRoom::DeleteMonster() calls to KillMonster().
// Commented out Commands.Add() call that was trashing memory.
//
// Revision 1.40  2002/02/07 23:28:43  erikh2000
// Added CCurrentGame::ProcessMonsters() code to remove 'Neather monster.  (Committed on behalf of j_wicks.)
//
// Revision 1.39  2002/02/07 22:31:09  erikh2000
// Removed CCurrentGame::bIsFirstTurn member and references to it.
//
// Revision 1.38  2001/12/16 02:12:53  erikh2000
// Added processing for monster questions.
// Added CCurrentGame::GetChecksum().
//
// Revision 1.37  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.36  2001/11/23 01:07:53  erikh2000
// Added CCurrentGame::UndoCommands() stub.
//
// Revision 1.35  2001/11/20 00:51:52  erikh2000
// Added a bunch of demo-related stuff that doesn't work yet.  This is an early check-in to avoid later merging conflicts.
//
// Revision 1.34  2001/11/19 09:25:15  erikh2000
// Removed some code that initializes members of new CMonster--no longer necessary with changes to CMonster.
//
// Revision 1.33  2001/11/18 04:10:37  md5i
// A few mimic fixes.
//
// Revision 1.32  2001/11/18 01:31:21  erikh2000
// Fix handling of crumbly walls when swordsman moves into a new room.
// Added a red door-clearing check in SetMembersAfterRoomLoad().
//
// Revision 1.31  2001/11/17 23:08:48  erikh2000
// Added mimic cursor processing.  (Committed on behalf of j_wicks.)
//
// Revision 1.30  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.29  2001/11/14 01:43:06  md5i
// Invisibility potions added.
//
// Revision 1.28  2001/11/14 01:36:20  md5i
// Fix some tile growth calculation problems.
//
// Revision 1.27  2001/11/14 00:53:47  md5i
// Change bFirstTurn semantics slightly.
//
// Revision 1.26  2001/11/13 05:35:54  md5i
// Added TarMother and growing tar.
//
// Revision 1.25  2001/11/12 23:28:44  md5i
// Make tar work.
//
// Revision 1.24  2001/11/12 03:22:02  erikh2000
// Added StabTar() stub.
//
// Revision 1.23  2001/11/11 05:01:16  md5i
// Added serpents.
//
// Revision 1.22  2001/11/08 11:33:04  erikh2000
// Removed reference to now-deleted bIsPlacingMimic member.
//
// Revision 1.21  2001/11/06 08:47:57  erikh2000
// Removed duplicate calls to pRoom->SetCurrentGame() causing assertian errors.  (Committed on behalf of jpburford.)
//
// Revision 1.20  2001/11/05 06:17:39  erikh2000
// Added code to destroy crumbly walls.
//
// Revision 1.19  2001/11/05 05:44:01  erikh2000
// Added SetActiveGame to false code in level exit.
//
// Revision 1.18  2001/11/05 04:57:40  erikh2000
// Changed level exit code to return list of level IDs instead of level pointer.
//
// Revision 1.17  2001/11/04 17:22:22  md5i
// Level CueEvents generated.
//
// Revision 1.16  2001/11/03 20:16:19  erikh2000
// Removed OnPlot() and OnLoad() references.  Added code that creates CID_Plots cue event.  Renamed bIsSwordsmanDead member to "bIsGameActive" so that it can be used more generally.
//
// Revision 1.15  2001/10/30 02:04:30  erikh2000
// Added clearing of conquered/explored room lists to Clear() method.  (Committed on behalf of timeracer.)
//
// Revision 1.14  2001/10/27 20:25:33  erikh2000
// Added room jump commands.
// Fixed bug where swordsman is sometimes not killed when more than one monster is in the room.
//
// Revision 1.13  2001/10/27 04:42:35  erikh2000
// Added SetOnLoad() and SetOnPlot() methods.
// Replaced room ID-checking code for explored/conquered rooms and level completion with calls to class methods that do same.
//
// Revision 1.12  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.11  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.10  2001/10/24 02:18:13  md5i
// Fixed egg tiles.  Used bIsFirstTurn status.
//
// Revision 1.9  2001/10/24 01:44:33  md5i
// Add turn counter.
//
// Revision 1.8  2001/10/22 23:55:44  erikh2000
// Revised level-loading methods.
//
// Revision 1.7  2001/10/21 00:27:37  erikh2000
// Fixed bugs preventing monsters from being processed.
// Monsters can now kill the swordsman.
// Wrote Save() method that just calls CDbSavedGame::Update() stub.
// Now keeps track of which rooms have been explored or conquered, and handles effects of.
// ProcessSwordsman() now sends back cue events for stepping on/off scrolls.
//
// Revision 1.6  2001/10/20 20:05:46  erikh2000
// Updated ProcessSwordsman() to stab monsters, destroy trapdoors, and strike orbs.
// Moved most of the room-exiting logic into LoadNewRoomForExit() to improve readability of ProcessSwordsman().
//
// Revision 1.5  2001/10/20 05:42:43  erikh2000
// Removed tile image references.
//
// Revision 1.4  2001/10/16 00:02:31  erikh2000
// Wrote code to display current room and handle basic swordsman movement.
//
// Revision 1.3  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.2  2001/10/07 04:38:34  erikh2000
// General cleanup--finished removing game logic code from DROD.EXE.
// Added ProcessCommand() method to CCurrentGame to be used as interface between UI and game logic.
//
// Revision 1.1.1.1  2001/10/01 22:20:06  erikh2000
// Initial check-in.
//
