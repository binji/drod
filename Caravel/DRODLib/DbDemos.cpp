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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DbDemos.cpp
//Implementation of CDbDemos and CDbDemo.

#include "DbDemos.h"

#include "CurrentGame.h"
#include "Db.h"
#include "DBProps.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Base64.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Ports.h>
#include <BackEndLib/Wchar.h>

UINT CDbDemos::wSavedShowSequenceNo = 0;

typedef struct tagCommandScore
{
	DWORD	dwTime;
	int		nScore;
} COMMANDSCORE;

//Prototypes.
bool	GetDemoStatBool(const CIDList &DemoStats, const DWORD dwDSID);
UINT	GetDemoStatUint(const CIDList &DemoStats, const DWORD dwDSID);

//
//CDbDemos public methods.
//

//*****************************************************************************
string CDbDemos::ExportXML(
//Returns: string containing XML text describing demo with this ID
//
//Pre-condition: dwDemoID is valid
//
//Params:
	const DWORD dwDemoID,	//(in)
	CDbRefs &dbRefs,			//(in/out)
	const bool /*bRef*/)			//(in)
{
#define STARTTAG(vType,pType) "<"; str += ViewTypeStr(vType); str += " "; str += PropTypeStr(pType); str += "='"
#define PROPTAG(pType) "' "; str += PropTypeStr(pType); str += "='"
#define ENDTAG(vType) "</"; str += ViewTypeStr(vType); str += ">\n"
#define CLOSETAG "'/>\n"
#define LONGTOSTR(val) _ltoa((val), dummy, 10)

   string str;

	if (!dbRefs.IsSet(V_Demos,dwDemoID))
	{
		dbRefs.Set(V_Demos,dwDemoID);

		CDbDemo *pDemo = GetByID(dwDemoID);
		ASSERT(pDemo);
      if (!pDemo) return str; //shouldn't happen, but just in case

		if (pDemo->dwNextDemoID)
		{
         //Temporary kludge for bug before Build 47: demos could be deleted,
         //leaving a "dangling ID" from the previous demo in a sequence.
         //This will check whether such the next demo actually exists,
         //and reset pDemo->dwNextDemoID if not.
         CDbDemo *pNextDemo = GetByID(pDemo->dwNextDemoID);
         if (!pNextDemo)
         {
            pDemo->dwNextDemoID = 0;
            pDemo->Update();
         } else {
            delete pNextDemo;
            //End of kludge for Build 46.

            //Include next demo first (so ID is accessable to this one on import).
			   str += g_pTheDB->Demos.ExportXML(pDemo->dwNextDemoID, dbRefs);
         }
		}

		//Include corresponding saved game with demo.
		str += g_pTheDB->SavedGames.ExportXML(pDemo->dwSavedGameID, dbRefs);

		//Prepare data.
		WSTRING const wDescStr = (WSTRING)pDemo->DescriptionText;
		char dummy[32];

      str += STARTTAG(V_Demos, P_SavedGameID);
		str += LONGTOSTR(pDemo->dwSavedGameID);
		str += PROPTAG(P_IsHidden);
		str += LONGTOSTR((long)pDemo->bIsHidden);
		str += PROPTAG(P_DescriptionMessage);
		str += Base64::encode(wDescStr);
		str += PROPTAG(P_ShowSequenceNo);
      str += LONGTOSTR(pDemo->wShowSequenceNo);
		str += PROPTAG(P_BeginTurnNo);
		str += LONGTOSTR(pDemo->wBeginTurnNo);
		str += PROPTAG(P_EndTurnNo);
		str += LONGTOSTR(pDemo->wEndTurnNo);
		str += PROPTAG(P_NextDemoID);
		str += LONGTOSTR(pDemo->dwNextDemoID);
		str += PROPTAG(P_Checksum);
		str += LONGTOSTR(pDemo->dwChecksum);
		//Put primary key last, so all message fields have been set by the
		//time Update() is called during import.
		str += PROPTAG(P_DemoID);
		str += LONGTOSTR(pDemo->dwDemoID);
		str += CLOSETAG;

		delete pDemo;
	}

	return str;

#undef STARTTAG
#undef PROPTAG
#undef ENDTAG
#undef CLOSETAG
#undef LONGTOSTR
}

//*****************************************************************************
DWORD CDbDemos::FindByLatest()
//Finds the latest demo.
//
//Returns:
//DemoID of the found demo, or 0 if no match found.
{
	ASSERT(CDbBase::IsOpen());

	//For now, it is a valid to just return the last ID in the membership.
	//If it becomes possible for a demo record stored in front of a second
	//demo record to be created later, then this will no longer work.
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);

	DWORD dwSize = this->MembershipIDs.GetSize();
	if (dwSize)
		return this->MembershipIDs.GetID(dwSize - 1);
	else
		return 0L;
}

//*******************************************************************************
void CDbDemos::FindHiddens(const bool bFlag)
//Changes filter so that hidden demos will also be returned (or not).
//Other filters remain the same.
{
   if (bFlag != this->bLoadHidden)
   {
 		//Membership is invalid.
		this->bIsMembershipLoaded = false;
      this->bLoadHidden = bFlag;
   }
}

//*****************************************************************************
DWORD	CDbDemos::GetHoldIDofDemo(
//Get the ID of the hold a demo belongs to.
//
//Params:
	const DWORD dwDemoID)	//(in)
const
{
	CDbDemo *pDemo = GetByID(dwDemoID);
	if (!pDemo) return 0L;
	CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetByID(pDemo->dwSavedGameID);
	delete pDemo;
	if (!pSavedGame) return 0;
	CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(pSavedGame->dwRoomID);
	delete pSavedGame;
   ASSERT(pRoom);
   if (!pRoom) return 0;   //Corrupted saved game record.
	CDbLevel *pLevel = g_pTheDB->Levels.GetByID(pRoom->dwLevelID);
	delete pRoom;
   ASSERT(pLevel);
   if (!pLevel) return 0;   //Corrupted room record.

   const DWORD dwHoldID = pLevel->dwHoldID;
	delete pLevel;

	return dwHoldID;
}

//*****************************************************************************
void CDbDemos::FilterByHold(
//Changes filter so that GetFirst() and GetNext() will return demos
//for a specified hold (and player, if specified).
//
//Params:
	const DWORD dwSetFilterByHoldID)	//(in)	Hold ID to filter by.  Set to 0
								//		for all holds.
{
	if (dwSetFilterByHoldID != this->dwFilterByHoldID ||
		this->dwFilterByRoomID != 0L)
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByRoomID = 0L;
   //Don't zero filterByShow or filterByPlayer
	this->dwFilterByHoldID = dwSetFilterByHoldID;
}

//*****************************************************************************
void CDbDemos::FilterByPlayer(
//Changes filter so that GetFirst() and GetNext() will return saved games
//for a specified player.  Other demo filters stay in effect.
//
//Params:
	const DWORD dwSetFilterByPlayerID)	//(in)	Player ID to filter by.
								// Set to 0 for all demos.
								// (Other filters remain in effect.)
{
	if (dwSetFilterByPlayerID != this->dwFilterByPlayerID)
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	//Don't zero other filter types.
	this->dwFilterByPlayerID = dwSetFilterByPlayerID;
}

//*****************************************************************************
void CDbDemos::FilterByRoom(
//Changes filter so that GetFirst() and GetNext() will return demos for a
//specified room.
//
//Params:
	const DWORD dwSetFilterByRoomID)	//(in)	Room ID to filter by.  Set to 0 for all rooms.
{
	if (dwSetFilterByRoomID != this->dwFilterByRoomID)
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByHoldID = 0L;
	this->dwFilterByRoomID = dwSetFilterByRoomID;
   //Don't zero filterByPlayer
	this->bFilterByShow = false;
}

//*****************************************************************************
void CDbDemos::FilterByShow()
//Changes filter so that GetFirst() and GetNext() will return demos in the
//show sequence.
{
	if (!this->bFilterByShow)
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

   //Don't zero filterByHold
	this->dwFilterByPlayerID = this->dwFilterByRoomID = 0L;
	this->bFilterByShow = true;
}

//*****************************************************************************
void CDbDemos::Delete(
//Deletes records for a demo and associated saved game.
//
//Params:
	const DWORD dwDemoID)	//(in)	Demo to delete.
{
	ASSERT(dwDemoID);

	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoRowI = LookupRowByPrimaryKey(dwDemoID, p_DemoID, DemosView);
	if (dwDemoRowI == ROW_NO_MATCH) {ASSERTP(false, "Bad demo ID."); return;}

	//Get saved game ID associated with demo.
	const DWORD dwSavedGameID = p_SavedGameID( DemosView[dwDemoRowI] );
	if (!dwSavedGameID) {ASSERTP(false, "Corrupted demo record."); return;}

	//Delete description message text(s).
	const DWORD dwDescriptionMID = p_DescriptionMessageID( DemosView[dwDemoRowI] );
	if (!dwDescriptionMID) {ASSERTP(false, "Bad message ID."); return;}
	DeleteMessage((MESSAGE_ID)dwDescriptionMID);

	//Delete the saved game.
	g_pTheDB->SavedGames.Delete(dwSavedGameID);

   //Remove demo from the show sequence, if needed.
   const UINT wShowSequenceNo = p_ShowSequenceNo( DemosView[dwDemoRowI] );
   if (wShowSequenceNo)
	   RemoveShowSequenceNo(wShowSequenceNo);

	//Delete the demo.
	DemosView.RemoveAt(dwDemoRowI);

   //Update any record that had this one as its next demo.
	const DWORD dwDemoCount = DemosView.GetSize();
 	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; dwDemoI++)
   {
      if (dwDemoID == (DWORD) p_NextDemoID(DemosView[dwDemoI]))
         p_NextDemoID( DemosView[ dwDemoI ] ) = 0;
   }

	//After demo object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//
//CDbDemos private methods.
//

//*****************************************************************************
void CDbDemos::LoadMembership()
//Load the membership list with all demo IDs.
//Player filtering is done internal to room filtering.
{
	ASSERT(CDbBase::IsOpen());
	this->MembershipIDs.Clear();

	if (this->bFilterByShow)
		LoadMembership_ByShow();
	else if (this->dwFilterByHoldID)
		LoadMembership_ByHold();
	else if (this->dwFilterByRoomID)
		LoadMembership_ByRoom();
	else if (this->dwFilterByPlayerID)
		LoadMembership_ByPlayer();
	else
		LoadMembership_All();

	this->pCurrentRow = this->MembershipIDs.Get(0);
	this->bIsMembershipLoaded = true;
}

//*****************************************************************************
void CDbDemos::LoadMembership_All()
//Loads membership list from all saved games,
{
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();

	//Each iteration gets a demo ID and puts in it membership list.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		if (this->bLoadHidden || p_IsHidden(DemosView[dwDemoI]) == 0)
			this->MembershipIDs.Add(p_DemoID(DemosView[dwDemoI]));
	}
}

//*****************************************************************************
void CDbDemos::LoadMembership_ByHold()
//Loads membership list from all saved games in a specified hold,
//and for specified player, if any.
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();

	//Each iteration gets a demo ID and puts in it membership list.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		if (this->bLoadHidden || p_IsHidden(DemosView[dwDemoI]) == 0)
		{
         //Does hold ID match filter?
         const DWORD dwDemoID = p_DemoID(DemosView[dwDemoI]);
         const DWORD dwHoldID = GetHoldIDofDemo(dwDemoID);  //!!can optimize
			if (dwHoldID == this->dwFilterByHoldID)
         {
			   //Does player ID match filter?
            if (!this->dwFilterByPlayerID)
               this->MembershipIDs.Add(dwDemoID);
            else
            {
			      //Look up saved game for this demo.
			      const DWORD dwSavedGameID = p_SavedGameID(DemosView[dwDemoI]);
			      const DWORD dwSavedGameI = LookupRowByPrimaryKey(dwSavedGameID,
					      p_SavedGameID, SavedGamesView);
			      if (dwSavedGameI == ROW_NO_MATCH)
               {
				      ASSERTP(false, "SavedGameID is foreign key to nowhere.");
                  continue;
               }

			      const DWORD dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
			      if (dwPlayerID == this->dwFilterByPlayerID)
				      this->MembershipIDs.Add(dwDemoID);
            }
         }
		}
	}
}

//*****************************************************************************
void CDbDemos::LoadMembership_ByRoom()
//Loads membership list from saved games in a specified room,
//and for specified player, if any.
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();
	DWORD dwRoomID, dwPlayerID;

	//Each iteration gets a demo ID and maybe puts in membership list.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		if (this->bLoadHidden || p_IsHidden(DemosView[dwDemoI]) == 0)
		{
			//Look up room ID for this demo.
			const DWORD dwSavedGameID = p_SavedGameID(DemosView[dwDemoI]);
			const DWORD dwSavedGameI = LookupRowByPrimaryKey(dwSavedGameID,
					p_SavedGameID, SavedGamesView);
			if (dwSavedGameI != ROW_NO_MATCH)
				dwRoomID = p_RoomID(SavedGamesView[dwSavedGameI]);
			else
			{
				ASSERTP(false, "SavedGameID is foreign key to nowhere.");
				dwRoomID = 0L;
			}

			//Does room ID match filter?
			if (dwRoomID == this->dwFilterByRoomID)
			{
				//Does player ID match filter?
				if (!this->dwFilterByPlayerID)
					this->MembershipIDs.Add(p_DemoID(DemosView[dwDemoI]));
				else
				{
					dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
					if (dwPlayerID == this->dwFilterByPlayerID)
						this->MembershipIDs.Add(p_DemoID(DemosView[dwDemoI]));
				}
			}
		}
	}
}

//*****************************************************************************
void CDbDemos::LoadMembership_ByPlayer()
//Loads membership list from saved games for a specified player,
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();
	DWORD dwPlayerID;

	//Each iteration gets a demo ID and maybe puts in membership list.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		if (this->bLoadHidden || p_IsHidden(DemosView[dwDemoI]) == 0)
		{
			//Look up saved game for this demo.
			const DWORD dwSavedGameID = p_SavedGameID(DemosView[dwDemoI]);
			const DWORD dwSavedGameI = LookupRowByPrimaryKey(dwSavedGameID,
					p_SavedGameID, SavedGamesView);
			if (dwSavedGameI == ROW_NO_MATCH)
         {
				ASSERTP(false, "SavedGameID is foreign key to nowhere.");
            continue;
         }

			//Does player ID match filter?
			dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
			if (dwPlayerID == this->dwFilterByPlayerID)
				this->MembershipIDs.Add(p_DemoID(DemosView[dwDemoI]));
		}
	}
}

//*****************************************************************************
void CDbDemos::LoadMembership_ByShow()
//Load the membership list with all demo IDs that are in the show sequence.
//The IDs will be ordered by values of the "ShowSequenceNo" field.
//
//Note: this filter ignores hold, room, and player filters
{
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();

	//This will hold unordered demo IDs.
	CIDList UnorderedIDs;

	//Each iteration gets a demo ID and maybe puts in membership list.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		//A ShowSequenceNo value greater than zero indicates that demo is in
		//the show sequence.
		const UINT wShowSequenceNo = p_ShowSequenceNo(DemosView[dwDemoI]);
		if (wShowSequenceNo)
		{
			UnorderedIDs.Add((DWORD) p_DemoID(DemosView[dwDemoI]),
					new CAttachableWrapper<UINT>(wShowSequenceNo), true);
		}
	}

	const UINT wIDCount = UnorderedIDs.GetSize();
	if (!wIDCount) {this->bIsMembershipLoaded=true; return;}

	//If assertians fire below, consider the following:  The values of the ShowSequenceNo
	//field go from 1 to n without gaps or duplicate values.  n equals the number of
	//demos in the show sequence.

	//Create an array which will hold all of the demo IDs in order.
	DWORD *pdwOrderedIDs = new DWORD[wIDCount];
	if (!pdwOrderedIDs) return;
	memset(pdwOrderedIDs, 0, sizeof(DWORD)*wIDCount);

	//Put all the IDs into an array element indicated by the associated show sequence.
	IDNODE *pID = UnorderedIDs.Get(0);
	while (pID)
	{
		const UINT wShowSequenceNo = *(static_cast<const CAttachableWrapper<UINT> *>(pID->pvPrivate));
		if (wShowSequenceNo <= wIDCount)
			pdwOrderedIDs[wShowSequenceNo - 1] = pID->dwID;
		else
            //There are gaps between the ShowSequenceNo values, i.e "1,2,4" instead of "1,2,3,4".
			ASSERTP(false, "Gaps between show sequence values.");
		pID = pID->pNext;
	}

	//Put ordered IDs into membership.
	for (UINT wIDI = 0; wIDI < wIDCount; ++wIDI)
	{
		if (pdwOrderedIDs[wIDI]) this->MembershipIDs.Add(pdwOrderedIDs[wIDI]);
	}

	//If below assertian fires, there are duplicate ShowSequenceNo values, i.e. "1,2,3,3".
	ASSERT(this->MembershipIDs.GetSize() == wIDCount);

	delete [] pdwOrderedIDs;

	this->bIsMembershipLoaded = true;
}

//
//CDbDemo public methods.
//

//*****************************************************************************
void CDbDemo::Clear()
//Frees resources and zeros members.
{
	this->DescriptionText.Clear();
	this->dwDemoID = this->dwSavedGameID = this->dwNextDemoID = this->dwChecksum = 0L;
	this->wBeginTurnNo = this->wEndTurnNo = this->wShowSequenceNo = 0;
	this->bIsHidden = false;
}

//*****************************************************************************
bool CDbDemo::Load(
//Loads a demo from database into this object.
//
//Params:
	const DWORD dwLoadDemoID)	//(in) DemoID of demo to load.
//
//Returns:
//True if successful, false if not.
{
	//Open demos view.
	ASSERT(CDbBase::IsOpen());
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));

	//Find record with matching saved game ID.
	const DWORD dwDemoI = LookupRowByPrimaryKey(dwLoadDemoID, p_DemoID, DemosView);
	if (dwDemoI == ROW_NO_MATCH) return false;

	//Load in props from Demos record.
	this->dwDemoID = (DWORD) (p_DemoID(DemosView[dwDemoI]));
	this->dwSavedGameID = (DWORD) (p_SavedGameID(DemosView[dwDemoI]));
	this->DescriptionText.Bind((DWORD) p_DescriptionMessageID(DemosView[dwDemoI]));
	this->bIsHidden = ((UINT) (p_IsHidden(DemosView[dwDemoI])) != 0);
	this->wShowSequenceNo = (UINT) (p_ShowSequenceNo(DemosView[dwDemoI]));
	this->wBeginTurnNo = (UINT) (p_BeginTurnNo(DemosView[dwDemoI]));
	this->wEndTurnNo = (UINT) (p_EndTurnNo(DemosView[dwDemoI]));
	this->dwNextDemoID = (DWORD) (p_NextDemoID(DemosView[dwDemoI]));
	this->dwChecksum = (DWORD) (p_Checksum(DemosView[dwDemoI]));

	return true;
}

//*****************************************************************************
CCurrentGame * CDbDemo::GetCurrentGame()
//Get a current game that is loaded from the demo's saved game with turn set to
//first turn in recorded demo.
//
//Returns:
//Current game or NULL if there was a compatibility error playing back commands.
//Other errors will return NULL, but also fire an assertian to indicate they should
//be debugged.
const
{
	ASSERT(this->dwSavedGameID);

	//Get the game.
	CCurrentGame *pGame;
	CCueEvents Ignored;
	{
		pGame = g_pTheDB->GetSavedCurrentGame(this->dwSavedGameID, Ignored, true);
		ASSERT(pGame);
		if (!pGame) return NULL;
	}

	//Demo current games should be used for playback purposes, in which case no
	//automatic game saving should occur during playback.
	pGame->SetAutoSaveOptions(ASO_NONE);
	pGame->FreezeCommands();

	//Play commands to beginning turn# of demo.
	ASSERT(this->wBeginTurnNo < pGame->Commands.GetSize());
	if (this->wBeginTurnNo) pGame->SetTurn(this->wBeginTurnNo, Ignored);

	if (pGame->wTurnNo != this->wBeginTurnNo)
	{
		//Wasn't able to play back all the turns.
		delete pGame;
		return NULL;
	}
	else
		return pGame;
}

//*****************************************************************************
UINT CDbDemos::GetNextSequenceNo() const
//Returns: next available ShowSequenceNo
{
   //Iterate the next available number each time method is called.
   if (CDbDemos::wSavedShowSequenceNo)
      return ++CDbDemos::wSavedShowSequenceNo;

   //Look up
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();
	UINT wMaxSequenceNo = 0;

	//Each iteration gets a record's sequence number.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		const UINT wShowSequenceNo = p_ShowSequenceNo(DemosView[dwDemoI]);
		if (wShowSequenceNo > wMaxSequenceNo)
			wMaxSequenceNo = wShowSequenceNo;
	}

   CDbDemos::wSavedShowSequenceNo = wMaxSequenceNo + 1;
	return CDbDemos::wSavedShowSequenceNo;
}

//*****************************************************************************
void CDbDemos::RemoveShowSequenceNo(
//Removes sequence number and decrements all greater sequence numbers.
//
//Params:
	const UINT wSequenceNo)	//(in) Sequence number to remove
{
	ASSERT(wSequenceNo);

	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();

	//Each iteration checks a record's show sequence number.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		const UINT wShowSequenceNo = p_ShowSequenceNo(DemosView[dwDemoI]);
		if (wShowSequenceNo == wSequenceNo)
			p_ShowSequenceNo( DemosView[ dwDemoI ] ) = 0;
		else if (wShowSequenceNo > wSequenceNo)
			p_ShowSequenceNo( DemosView[ dwDemoI ] ) = wShowSequenceNo - 1;
	}
}

//*****************************************************************************
bool CDbDemos::ShowSequenceNoOccupied(
//Returns: whether sequence number is being used by some record
//
//Params:
	const UINT wSequenceNo)	//(in)
{
   //Some no's might have been taken but not yet committed.
   if (CDbDemos::wSavedShowSequenceNo >= wSequenceNo)
      return true;

   c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();

	//Each iteration checks a record's show sequence number.
	for (DWORD dwDemoI = 0; dwDemoI < dwDemoCount; ++dwDemoI)
	{
		const UINT wShowSequenceNo = p_ShowSequenceNo(DemosView[dwDemoI]);
		if (wShowSequenceNo == wSequenceNo)
			return true;
	}

	//No match.
	return false;
}

//*****************************************************************************
const WCHAR * CDbDemo::GetAuthorText() const
//Returns author of the demo or NULL if not found.
{
	//Look up PlayerID from associated SavedGames record.
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGamesRowI = LookupRowByPrimaryKey(this->dwSavedGameID,
			p_SavedGameID,	SavedGamesView);
	if (dwSavedGamesRowI == ROW_NO_MATCH) {ASSERTP(false, "Saved game row missing."); return NULL;}
	const DWORD dwPlayerID = p_PlayerID( SavedGamesView[dwSavedGamesRowI] );

	//Look up NameMessageID from associated Players record.
	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayersRowI = LookupRowByPrimaryKey(dwPlayerID, p_PlayerID,
			PlayersView);
	if (dwPlayersRowI == ROW_NO_MATCH) {ASSERTP(false, "Player row missing."); return NULL;}
	const DWORD dwNameMessageID = p_NameMessageID( PlayersView[dwPlayersRowI] );

	//Look up message text.
	return g_pTheDB->GetMessageText((MESSAGE_ID)dwNameMessageID);
}

//*****************************************************************************
bool CDbDemo::GetTimeElapsed(WSTRING &wstrText) const
//Append to string argument the time duration covered by the demo,
//in hours, minutes and seconds.
{
   CCueEvents Ignored;
	CCurrentGame *pGame = g_pTheDB->GetSavedCurrentGame(this->dwSavedGameID, Ignored, true);
	if (!pGame)
	{
		ASSERTP(false, "Saved game for demo couldn't be loaded.");
		return false;
	}
   if (pGame->Commands.GetSize() < this->wEndTurnNo + 1)
   {
      delete pGame;
		ASSERTP(false, "Not enough commands stored or the ending turn# is wrong.");
		return false;
   }

   DWORD dwTimeElapsed = 0;

	COMMANDNODE *pCommand = pGame->Commands.GetFirst();
	UINT wTurnNo;
	for (wTurnNo = this->wBeginTurnNo; wTurnNo <= this->wEndTurnNo; ++wTurnNo)
	{
		if (!pCommand)
		{
         //(Should be handled by the GetSize() check above.)
			delete pGame;
			ASSERTP(false, "Not enough commands stored or the ending turn# is wrong.");
			return false;
		}
      dwTimeElapsed += pCommand->byt10msElapsedSinceLast;
		pCommand = pGame->Commands.GetNext();
	}
	delete pGame;

   dwTimeElapsed /= 100;  //convert: 10ms --> s
   const DWORD dwHours = dwTimeElapsed / 3600;
   const DWORD dwMinutes = (dwTimeElapsed / 60) % 60;
   const DWORD dwSeconds = dwTimeElapsed % 60;
   WCHAR dummy[20];
   WSTRING wTmp;
   if (dwHours)
   {
	   wstrText += _itoW(dwHours,dummy,10);
      wstrText += wszColon;
	   if (dwMinutes < 10)
	   {
		   AsciiToUnicode("0", wTmp);
		   wstrText += wTmp;
	   }
   }
	wstrText += _itoW(dwMinutes,dummy,10);
   wstrText += wszColon;
	if (dwSeconds < 10)
	{
		AsciiToUnicode("0", wTmp);
		wstrText += wTmp;
	}
	wstrText += _itoW(dwSeconds,dummy,10);

   return true;
}

//*****************************************************************************
bool CDbDemo::Update()
//Updates database with demo.
{
   g_pTheDB->Demos.ResetMembership();
   g_pTheDB->Demos.ResetSavedShowSequenceNo();
	if (this->dwDemoID == 0)
	{
		//Insert a new demo.
		return UpdateNew();
	}
	else
	{
		//Update existing demo.
		return UpdateExisting();
	}
}

//*****************************************************************************
void CDbDemo::GetSceneFromBeginTurnNo(
//Get consecutive sequence of turns (scene) in the demo beginning at a specified
//turn and ending after a specified duration.
//
//Params:
	const DWORD dwMaxTime,		//(in)	Length of time in milliseconds that the
								//		scene should match.  If there are not enough
								//		turns after the beginning turn to reach this
								//		length, then the scene will have a shorter
								//		duration.
	CDemoScene &Scene,			//(out)	Receives scene information.
	const UINT wEvalBeginTurnNo)//(in)	First turn to evaluate.  If -1 (default), the first
								//		turn in the demo will be used.
{
	ASSERT(dwMaxTime);

	//Evaluation turn range should not be outside turn range for this demo
	ASSERT(wEvalBeginTurnNo == (UINT)-1 || wEvalBeginTurnNo >= this->wBeginTurnNo);

	//Load current game from saved game with option to restore from beginning
	//of room without playing commands.
	CCueEvents CueEvents;
	CCurrentGame *pGame = g_pTheDB->GetSavedCurrentGame(this->dwSavedGameID, CueEvents, true);
	if (!pGame)
	{
		ASSERTP(false, "Saved game for demo couldn't be loaded.");
		return;
	}
	pGame->SetAutoSaveOptions(ASO_NONE); //No auto-saving for playback.

	//Set up vars used to track scores, times, and scenes from the demo.
	DWORD dwTime = 0L;
	Scene.wBeginTurnNo =
		(wEvalBeginTurnNo == (UINT)-1) ? this->wBeginTurnNo : wEvalBeginTurnNo;
	Scene.wEndTurnNo = Scene.wBeginTurnNo;

	//Set game to first turn to be evaluated.
	if (Scene.wBeginTurnNo) pGame->SetTurn(Scene.wBeginTurnNo, CueEvents);
	COMMANDNODE *pCommand = pGame->Commands.Get(Scene.wBeginTurnNo);
	if (!pCommand) {ASSERTP(false, "Corrupted saved game probably."); delete pGame; return;}

	pGame->FreezeCommands();

	//Each iteration adds time for one command.
	while (Scene.wEndTurnNo < this->wEndTurnNo-1)
	{
		if (!pCommand) {ASSERTP(false, "Corrupted saved game probably."); break;}

		//Add time for this command and see if total time is past the max.
		dwTime += (pCommand->byt10msElapsedSinceLast * 10);
		if (dwTime > dwMaxTime)
			//The scene will have ending turn# as last turn# that wasn't over the max.
			break;

		//Increment turn# and get next command.
		++Scene.wEndTurnNo;
		pCommand = pGame->Commands.GetNext();
	}

	pGame->UnfreezeCommands();
	delete pGame;

	//Scene param was set in above loop.
}

//*****************************************************************************
void CDbDemo::GetMostInterestingScene(
//Get the most interesting consecutive sequence of turns (scene) in the demo.
//"Interesting" is evaluated by a scoring system.
//
//Params:
	const DWORD dwMaxTime,		//(in)	Maximum length of time in milliseconds for
								//		the selected scene.
	CDemoScene &Scene,			//(out)	Receives the scene information.
	const UINT wEvalBeginTurnNo,//(in)	First turn to evaluate.  If -1 (default), the first
								//		turn in the demo will be used.
	const UINT wEvalEndTurnNo)	//(in)	Last turn to evaluate.  If -1 (default), the last
								//		turn in the demo will be used.
{
	ASSERT(dwMaxTime);

	//Evaluation turn range should not be outside turn range for this demo
	ASSERT(wEvalBeginTurnNo == (UINT)-1 || wEvalBeginTurnNo >= this->wBeginTurnNo);
	ASSERT(wEvalEndTurnNo == (UINT)-1 || wEvalEndTurnNo <= this->wEndTurnNo);

	//Load current game from saved game with option to restore from beginning
	//of room without playing commands.
	CCueEvents CueEvents;
	CCurrentGame *pGame = g_pTheDB->GetSavedCurrentGame(this->dwSavedGameID,
			CueEvents, true);
	if (!pGame)
	{
		ASSERTP(false, "Saved game for demo couldn't be loaded.");
		return;
	}
	pGame->SetAutoSaveOptions(ASO_NONE); //No auto-saving for playback.

	CCoordIndex VisitedSquares;
	if (!VisitedSquares.Init(pGame->pRoom->wRoomCols, pGame->pRoom->wRoomRows))
	{
		delete pGame;
		return;
	}

	//Set up vars used to track scores, times, and scenes from the demo.
	list<COMMANDSCORE> CommandScoreList;
	COMMANDSCORE Head;
	COMMANDSCORE Tail;
	int nHighestScore = -9999, nScore = 0;
	DWORD dwTime = 0L;
	CDemoScene CurrentScene;
	CurrentScene.wBeginTurnNo =
		(wEvalBeginTurnNo == (UINT)-1) ? this->wBeginTurnNo : wEvalBeginTurnNo;
	CurrentScene.wEndTurnNo = CurrentScene.wBeginTurnNo;
	UINT wLastTurnNo = (wEvalEndTurnNo == (UINT)-1) ? this->wEndTurnNo : wEvalEndTurnNo;

	//Set game to first turn to be evaluated.
	if (CurrentScene.wBeginTurnNo) pGame->SetTurn(CurrentScene.wBeginTurnNo, CueEvents);
	COMMANDNODE *pCommand = pGame->Commands.Get(CurrentScene.wBeginTurnNo);
	if (!pCommand) {ASSERTP(false, "Corrupted saved game probably."); delete pGame; return;}

	pGame->FreezeCommands();

	//Each iteration scores one turn and updates vars used for tracking scores
	//for different scenes.
	while (CurrentScene.wEndTurnNo <= wLastTurnNo)
	{
		if (!pCommand) {ASSERTP(false, "Corrupted saved game probably."); break;}

		//Process and score the next command.
		pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);
      if (CueEvents.HasOccurred(CID_MonsterKilledPlayer))
         break;   //Saved game is broken/corrupted.  So stop checking here.
		Head.nScore = ScoreGameCommand(CueEvents,
				VisitedSquares.Exists(pGame->swordsman.wX, pGame->swordsman.wY));
		Head.dwTime = (pCommand->byt10msElapsedSinceLast * 10);
		VisitedSquares.Add(pGame->swordsman.wX, pGame->swordsman.wY);

		//Add command score to list.
		CommandScoreList.push_front(Head);
		nScore += Head.nScore;
		dwTime += Head.dwTime;

		//Remove one or more tail entries if time is now over the max.
		while (dwTime > dwMaxTime)
		{
			ASSERT(CommandScoreList.size());

			Tail = CommandScoreList.back();
			CommandScoreList.pop_back();
			nScore -= Tail.nScore;
			dwTime -= Tail.dwTime;
			++(CurrentScene.wBeginTurnNo);
			ASSERT(CurrentScene.wBeginTurnNo <= CurrentScene.wEndTurnNo);
		}

		//If the score for all commands in the list is the highest, set vars
		//for new highest-scoring scene.
		if (nScore > nHighestScore)
		{
			Scene = CurrentScene; //Use return param to hold highest-scoring scene.
			nHighestScore = nScore;
		}

		//Eval next command.
		++(CurrentScene.wEndTurnNo);
		pCommand = pGame->Commands.GetNext();
	}

	pGame->UnfreezeCommands();
	delete pGame;

	//Scene param will hold the highest scoring scene.
}

//*****************************************************************************
void CDbDemo::GetNarrationText(
//Gets a narration of what happened in the demo.
//
//Params:
	WSTRING &wstrText)	//(out)	Narration text.
const
{
	CIDList DemoStats;
	const bool bDemoCorrect = Test(DemoStats);

	GetNarrationText_English(DemoStats, wstrText);

   if (!bDemoCorrect)
   {
      //Demo was corrupted or couldn't be played through because of version
      //changes.  Tack on this explanation to the description.
      wstrText += wszLeftParen;
      wstrText += g_pTheDB->GetMessageText(MID_DemoBroken);
      wstrText += wszRightParen;
   }
}

//*****************************************************************************
MESSAGE_ID CDbDemo::SetProperty(
//Used during XML data import.
//According to pType, convert string to proper datatype and member
//
//Returns: whether operation was successful
//
//Params:
	const PROPTYPE pType,	//(in) property (data member) to set
	char* const str,			//(in) string representation of value
	CImportInfo &info,	   //(in/out) Import info
	bool &bSaveRecord)		//(out) whether record should be saved
{
	static PrimaryKeyMap::iterator localID;
	switch (pType)
	{
		case P_DemoID:
 		   this->dwDemoID = static_cast<DWORD>(atol(str));
		   if (!this->dwDemoID)
			   return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = info.DemoIDMap.find(this->dwDemoID);
			if (localID != info.DemoIDMap.end())
				//Error - this demo should not have been imported already.
				return MID_FileCorrupted;

         if (bSaveRecord)
         {
			   //Add a new demo (ignore old local ID).
            const DWORD dwOldLocalID = this->dwDemoID;
			   this->dwDemoID = 0L;
            Update();
            info.DemoIDMap[dwOldLocalID] = this->dwDemoID;
         } else {
            //This demo is being ignored.
            //(It belongs to a non-existant hold/level/room/saved game.)
            info.DemoIDMap[this->dwDemoID] = 0;   //skip records with refs to this demo ID
            if (info.typeBeingImported == CImportInfo::Demo)
               return MID_DemoIgnored;
         }
			break;
		case P_SavedGameID:
			this->dwSavedGameID = static_cast<DWORD>(atol(str));
			if (!this->dwSavedGameID)
				return MID_FileCorrupted;

			//Look up local ID.
			localID = info.SavedGameIDMap.find(this->dwSavedGameID);
			if (localID == info.SavedGameIDMap.end())
				return MID_FileCorrupted;	//should have been loaded already
			this->dwSavedGameID = localID->second;
         if (!this->dwSavedGameID)
         {
            //Records for this saved game are being ignored.  Don't save this demo.
	         c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	         const DWORD dwDemoRowI = LookupRowByPrimaryKey(this->dwDemoID,
			         p_DemoID, DemosView);
	         if (dwDemoRowI != ROW_NO_MATCH)
            {
               //This is a pre-build 34/35 demo with an older XML export format.
               //It was saved to the DB before confirming whether it belonged
               //to a saved game that was actually being imported.
               //This one belongs to a saved game for a room not in the current
               //data files and must be taken back out of the DB.
	            DemosView.RemoveAt(dwDemoRowI);
            }
            bSaveRecord = false;
         }
			break;
		case P_IsHidden:
			this->bIsHidden = (atoi(str) != 0);
			break;
		case P_DescriptionMessage:
		{
			WSTRING data;
			Base64::decode(str,data);
			this->DescriptionText = data.c_str();
			break;
		}
		case P_ShowSequenceNo:
			this->wShowSequenceNo = static_cast<UINT>(atoi(str));
         //When importing a demo, allow viewing by any player.
         if (info.typeBeingImported == CImportInfo::Demo)
         {
            this->bIsHidden = false;
            this->wShowSequenceNo = 0;
         }
         if (this->wShowSequenceNo &&
               bSaveRecord)  //don't grab a sequence number unless it will be used
			{
				//Find next available show sequence index.
            this->wShowSequenceNo = 1;
				if (g_pTheDB->Demos.ShowSequenceNoOccupied(this->wShowSequenceNo))
					this->wShowSequenceNo = g_pTheDB->Demos.GetNextSequenceNo();
			}
			break;
		case P_BeginTurnNo:
			this->wBeginTurnNo = static_cast<UINT>(atoi(str));
			break;
		case P_EndTurnNo:
			this->wEndTurnNo = static_cast<UINT>(atoi(str));
			break;
		case P_NextDemoID:
			this->dwNextDemoID = static_cast<DWORD>(atol(str));
			if (!this->dwNextDemoID) break;	//no next demo

			//Look up local ID.
			localID = info.DemoIDMap.find(this->dwNextDemoID);
			if (localID == info.DemoIDMap.end())
				return MID_FileCorrupted;	//next demo should have been loaded already
			this->dwNextDemoID = localID->second;
			break;
		case P_Checksum:
			this->dwChecksum = static_cast<DWORD>(atol(str));
			break;
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbDemo::Test(
//Starts a temporary current game and tests this demo against it, running
//all the commands without UI interaction. Statistics are collected along the way so that
//the results of the demo can be evaluated.
//
//Params:
	CIDList &DemoStats) //(out) Returned containing statistics about things that happened.
//
//Returns:
//True if demo was able to be completely processed, false if not.
const
{
	bool bSuccess = true;

	ASSERT(CDbBase::IsOpen());
	ASSERT(this->dwSavedGameID);

	//Load current game from saved game with option to restore from beginning
	//of room without playing commands.
	CCueEvents CueEvents;
	CCurrentGame *pGame = g_pTheDB->GetSavedCurrentGame(this->dwSavedGameID, CueEvents, true);
	if (!pGame)
	{
		ASSERTP(false, "Saved game for demo couldn't be loaded.");
		return false;
	}
   if (pGame->Commands.GetSize() < this->wEndTurnNo + 1)
   {
      delete pGame;
		ASSERTP(false, "Not enough commands stored or the ending turn# is wrong.");
		return false;
   }
	pGame->SetAutoSaveOptions(ASO_NONE); //No auto-saving for playback.

	//Check for room conquered cue event which could have occurred on first
	//step into room.
	bool bWasRoomConquered = CueEvents.HasOccurred(CID_ConquerRoom);

	//While processing the command list, I don't want to take any actions that
	//will modify the command list.
	pGame->FreezeCommands();

	//Play from turn 0 to ending turn of demo.
	bool bDidPlayerDie = false;
	bool bDidPlayerLeaveRoom = false, bDidPlayerExitLevel = false;
	COMMANDNODE *pCommand = pGame->Commands.GetFirst();
	UINT wTurnNo;
	for (wTurnNo = 0; wTurnNo <= this->wEndTurnNo; ++wTurnNo)
	{
		if (!pCommand)
		{
         //(Should be handled by the GetSize() check above.)
			ASSERTP(false, "Not enough commands stored or the ending turn# is wrong.");
			bSuccess = false;
			break;
		}
		pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);

		//Collect statistics.
		ASSERT(!bDidPlayerDie);
		bDidPlayerDie = CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied);
		if (!bWasRoomConquered) bWasRoomConquered = CueEvents.HasOccurred(CID_ConquerRoom);

		//Check for player leaving room or game inactive.
		bDidPlayerLeaveRoom = CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom),
				CIDA_PlayerLeftRoom);
		if (bDidPlayerLeaveRoom || !pGame->bIsGameActive)
		{
			if (wTurnNo < this->wEndTurnNo) bSuccess = false;
			if (bWasRoomConquered)
				//Checksums won't match if this recently conquered room is
				//taken into account.
				pGame->ConqueredRooms.Remove(pGame->pRoom->dwRoomID);
         if (CueEvents.HasOccurred(CID_ExitLevelPending) || CueEvents.HasOccurred(CID_WinGame))
            bDidPlayerExitLevel = true;
			break;
		}

		pCommand = pGame->Commands.GetNext();
	}

	//Fix turn# so it indicates last command processed.
	if (wTurnNo > this->wEndTurnNo) wTurnNo = this->wEndTurnNo;

	//Populate demostats return param.
	DWORD dwDemoChecksum = pGame->GetChecksum();
	UINT wProcessedTurnCount = (wTurnNo >= this->wBeginTurnNo) ?
			wTurnNo - this->wBeginTurnNo + 1 : 0;
	DemoStats.Add(DS_FinalChecksum, new CAttachableWrapper<DWORD>(dwDemoChecksum), true);
	DemoStats.Add(DS_WasRoomConquered, new CAttachableWrapper<bool>(bWasRoomConquered), true);
	DemoStats.Add(DS_DidPlayerDie, new CAttachableWrapper<bool>(bDidPlayerDie), true);
	DemoStats.Add(DS_ProcessedTurnCount, new CAttachableWrapper<UINT>(wProcessedTurnCount), true);
	DemoStats.Add(DS_DidPlayerLeaveRoom, new CAttachableWrapper<bool>(bDidPlayerLeaveRoom), true);
	DemoStats.Add(DS_MonsterCount, new CAttachableWrapper<UINT>(pGame->pRoom->wMonsterCount), true);
	DemoStats.Add(DS_MonsterKills, new CAttachableWrapper<UINT>(pGame->wMonsterKills), true);
	DemoStats.Add(DS_DidPlayerExitLevel, new CAttachableWrapper<bool>(bDidPlayerExitLevel), true);

	//Compare checksum if it was specified.
	if (this->dwChecksum)
	{
		if (dwDemoChecksum != this->dwChecksum &&
            !bDidPlayerExitLevel)   //causes occasional checksum problems
			//Probably the demo no longer produces the same results as it did when
			//it was recorded due to changes in DRODLib.  But maybe data corruption
			//or other problems are to blame.
			bSuccess = false;
	}

	pGame->UnfreezeCommands();
	delete pGame;
	return bSuccess;
}

//
//CDbDemo private methods.
//

//*****************************************************************************
int CDbDemo::ScoreGameCommand(
//How interesting is one game command?
//
//Params:
	const CCueEvents &CueEvents,	//(in)	Cue events generated by processing command.
	const bool bPrevVisited)		//(in)	Has this square been visited by player already?
//
//Returns:
//A score indicating how interesting the command was.  Can be negative.
const
{
	int nScore = 0;

	//Note: If you wish to change this method to perform other types of scoring,
	//consider if it is better to put the evaluation code somewhere else that will
	//generate a cue event, and simply add the cue event to the list below.

	//Score for cue events.
#	define SCORE_FOR_CUEEVENT(cid, score) \
		nScore += (CueEvents.GetOccurrenceCount((cid)) * (score))

	SCORE_FOR_CUEEVENT( CID_MonsterKilledPlayer,				100);
	SCORE_FOR_CUEEVENT( CID_MonsterSpoke,						50);
	SCORE_FOR_CUEEVENT( CID_NeatherExitsRoom,					20);
	SCORE_FOR_CUEEVENT( CID_SwordsmanAfraid,					20);
	SCORE_FOR_CUEEVENT( CID_AllMonstersKilled,					10);
	SCORE_FOR_CUEEVENT( CID_SnakeDiedFromTruncation,			10);
	SCORE_FOR_CUEEVENT( CID_TarGrew,							10);
	SCORE_FOR_CUEEVENT( CID_OrbActivated,						4);
	SCORE_FOR_CUEEVENT( CID_OrbActivatedByMimic,				2);
	SCORE_FOR_CUEEVENT( CID_MonsterDiedFromStab,				2);
	SCORE_FOR_CUEEVENT( CID_EggHatched,							2);
	SCORE_FOR_CUEEVENT( CID_CrumblyWallDestroyed,				2);
	SCORE_FOR_CUEEVENT( CID_TarDestroyed,						2);
	SCORE_FOR_CUEEVENT( CID_TrapDoorRemoved,					2);
	SCORE_FOR_CUEEVENT( CID_DrankPotion,						2);
	SCORE_FOR_CUEEVENT( CID_MimicPlaced,						2);
	SCORE_FOR_CUEEVENT( CID_TarBabyFormed,						2);
	SCORE_FOR_CUEEVENT( CID_EvilEyeWoke,						2);

#	undef SCORE_FOR_CUEEVENT

	//Add to score if swordsman is on a new square.
	if (!bPrevVisited) ++nScore;

	return nScore;
}

//*****************************************************************************
bool CDbDemo::UpdateNew()
//Insert a new Demos record.
{
	ASSERT(this->dwDemoID == 0);

	//Open demos view.
	ASSERT(IsOpen());
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));

	//Save text.
	const DWORD dwDescriptionMID = this->DescriptionText.Flush();
   //ASSERT(dwDescriptionMID);   //will be 0 on import of pre-build 34 data

	//Add record.
	this->dwDemoID = GetIncrementedID(p_DemoID);
	DemosView.Add(
			p_DemoID[ this->dwDemoID ] +
			p_SavedGameID[ this->dwSavedGameID ] +
			p_DescriptionMessageID[ dwDescriptionMID ] +
			p_IsHidden[ this->bIsHidden ] +
			p_ShowSequenceNo[ this->wShowSequenceNo ] +
			p_BeginTurnNo[ this->wBeginTurnNo ] +
			p_EndTurnNo[ this->wEndTurnNo ] +
			p_NextDemoID[ this->dwNextDemoID ] +
			p_Checksum[ this->dwChecksum ]);

	return true;
}

//*****************************************************************************
bool CDbDemo::UpdateExisting()
//Update an existing Demos record.
{
	ASSERT(this->dwDemoID != 0);
	ASSERT(CDbBase::IsOpen());

	//Lookup demos record.
	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoI = LookupRowByPrimaryKey(this->dwDemoID,
			p_DemoID, DemosView);
	if (dwDemoI == ROW_NO_MATCH)
	{
		ASSERTP(false, "The caller probably passed a bad PKID.");
		return false;
	}

	//Save text.
	const DWORD dwDescriptionMID = this->DescriptionText.Flush();
	ASSERT(dwDescriptionMID);

	//Update record.
	p_DemoID( DemosView[ dwDemoI ] ) = this->dwDemoID;
	p_SavedGameID( DemosView[ dwDemoI ] ) = this->dwSavedGameID;
	p_IsHidden( DemosView[ dwDemoI ] ) = this->bIsHidden;
	p_DescriptionMessageID( DemosView[ dwDemoI ] ) = dwDescriptionMID;
	p_ShowSequenceNo( DemosView[ dwDemoI ] ) = this->wShowSequenceNo;
	p_BeginTurnNo( DemosView[ dwDemoI ] ) = this->wBeginTurnNo;
	p_EndTurnNo( DemosView[ dwDemoI ] ) = this->wEndTurnNo;
	p_NextDemoID( DemosView[ dwDemoI ] ) = this->dwNextDemoID;
	p_Checksum( DemosView[ dwDemoI ] ) = this->dwChecksum;

	return true;
}

//*****************************************************************************
void CDbDemo::GetNarrationText_English(
//Gets narration in English.
//
//Params:
	const CIDList &DemoStats,	//(in)	Statistics to use for narration.
	WSTRING &wstrText)			//(out) Narration text.
const
{
	//Get relevant demo statistics.
	//const bool bWasRoomConquered =	GetDemoStatBool(DemoStats, DS_WasRoomConquered);   //currently unused
	const bool bDidPlayerDie =			GetDemoStatBool(DemoStats, DS_DidPlayerDie);
	const UINT wProcessedTurnCount =	GetDemoStatUint(DemoStats, DS_ProcessedTurnCount);
	const bool bDidPlayerLeaveRoom =	GetDemoStatBool(DemoStats, DS_DidPlayerLeaveRoom);
	const UINT wMonsterCount =			GetDemoStatUint(DemoStats, DS_MonsterCount);
	const UINT wMonsterKills =			GetDemoStatUint(DemoStats, DS_MonsterKills);
	const bool bDidPlayerExitLevel =	GetDemoStatBool(DemoStats, DS_DidPlayerExitLevel);

	//Get the initial state of the room.
	CCueEvents Ignored;
	CCurrentGame *pBeforeGame = g_pTheDB->GetSavedCurrentGame(
			this->dwSavedGameID, Ignored, true);
	const UINT wBeforeMonsterKills = pBeforeGame->wMonsterKills;
   const UINT wMonstersAtStart = pBeforeGame->pRoom->wMonsterCount;
	delete pBeforeGame;

	//Monsters killed during the demo.
	const UINT wMonstersSlain = wMonsterKills - wBeforeMonsterKills;

	//Beethro dies.
	if (bDidPlayerDie)
	{
		if (wProcessedTurnCount < 50)
			wstrText += g_pTheDB->GetMessageText(MID_DemoDescKilled1);
		else
		{
			if (wMonstersSlain < 1)
			   wstrText += g_pTheDB->GetMessageText(MID_DemoDescKilled2);
			else if (wMonstersSlain == 1)
			   wstrText += g_pTheDB->GetMessageText(MID_DemoDescKilled3);
			else
			   wstrText += g_pTheDB->GetMessageText(MID_DemoDescKilled4);
		}
      wstrText += wszSpace;
      wstrText += wszSpace;
		return;
	}

	//
	//Beethro didn't die.
	//

	//Beethro conquered the room.
	if (wMonstersAtStart && wMonstersSlain && wMonsterCount == 0)
	{
		if (wMonstersSlain == 1)
		{
		   wstrText += g_pTheDB->GetMessageText(MID_DemoDescConquers1);
			if (wProcessedTurnCount < 200)
			{
            wstrText += wszComma;
            wstrText += wszSpace;
		      wstrText += g_pTheDB->GetMessageText(MID_DemoDescConquers2);
			}
			else if (wProcessedTurnCount > 500)
			{
            wstrText += wszSpace;
		      wstrText += g_pTheDB->GetMessageText(MID_DemoDescConquers3);
			}
			else
            wstrText += wszPeriod;
		}
		else if (wMonstersSlain < 10)
		   wstrText += g_pTheDB->GetMessageText(MID_DemoDescConquers4);
		else if (wMonstersSlain < 20)
		   wstrText += g_pTheDB->GetMessageText(MID_DemoDescConquers5);
		else
		   wstrText += g_pTheDB->GetMessageText(MID_DemoDescConquers6);
      wstrText += wszSpace;
      wstrText += wszSpace;

		if (bDidPlayerLeaveRoom)
      {
         if (bDidPlayerExitLevel)
		      wstrText += g_pTheDB->GetMessageText(MID_DemoDescExits1);
         else
		      wstrText += g_pTheDB->GetMessageText(MID_DemoDescLeaves1);
         wstrText += wszSpace;
         wstrText += wszSpace;
      }

		return;
	}

	//
	//Beethro has not conquered the room (or the room started empty).
	//

	//Beethro left the room.
	if (bDidPlayerLeaveRoom)
	{
		if (wMonsterCount == 0 || wMonstersAtStart == 0)
      {
         if (bDidPlayerExitLevel)
		      wstrText += g_pTheDB->GetMessageText(MID_DemoDescExits2);
         else
		      wstrText += g_pTheDB->GetMessageText(MID_DemoDescLeaves2);
      } else {
         if (bDidPlayerExitLevel)
            wstrText += g_pTheDB->GetMessageText(MID_DemoDescExits3);
         else
		      wstrText += g_pTheDB->GetMessageText(MID_DemoDescLeaves3);
      }
      wstrText += wszSpace;
      wstrText += wszSpace;
		return;
	}

	//Beethro didn't leave the room or conquer it.
	if (wProcessedTurnCount < 200)
	   wstrText += g_pTheDB->GetMessageText(MID_DemoDescLoiters1);
	else
	   wstrText += g_pTheDB->GetMessageText(MID_DemoDescLoiters2);
   wstrText += wszSpace;
   wstrText += wszSpace;
}

//Helper functions.

//*****************************************************************************
bool GetDemoStatBool(const CIDList &DemoStats, const DWORD dwDSID)
{
	IDNODE *pNode = DemoStats.GetByID(dwDSID);
	if (!pNode) return false;

	const CAttachableWrapper<bool> *pBool = static_cast<CAttachableWrapper<bool> *>
			(pNode->pvPrivate);

	return *pBool;
}

//*****************************************************************************
UINT GetDemoStatUint(const CIDList &DemoStats, const DWORD dwDSID)
{
	IDNODE *pNode = DemoStats.GetByID(dwDSID);
	if (!pNode) return 0;

	const CAttachableWrapper<UINT> *pUint = static_cast<CAttachableWrapper<UINT> *>
			(pNode->pvPrivate);

	return *pUint;
}

// $Log: DbDemos.cpp,v $
// Revision 1.78  2004/07/18 14:06:39  gjj
// Fixed some annoying warnings, and updated the config for fmod 3.7.3
//
// Revision 1.77  2004/06/27 16:22:59  mrimer
// Made data handling more robust.
//
// Revision 1.76  2003/11/09 05:24:48  mrimer
// Fixed bug: deleting demo in middle of demo sequence causes crash.
// Added check and repair for this condition on export.
//
// Revision 1.75  2003/10/20 17:49:03  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.74  2003/10/07 21:10:34  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.73  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.72  2003/09/12 17:26:34  mrimer
// Fixed demo enumeration and description problems.
//
// Revision 1.71  2003/08/19 18:37:13  mrimer
// Fixed bug: demo for unknown hold terminates player import.
//
// Revision 1.70  2003/08/19 04:40:27  mrimer
// Fixed a bug: counting past the end of a demo (or to the end, where the room is exited -- also bad).
//
// Revision 1.69  2003/08/09 19:34:03  mrimer
// Fixed bug: empty demo records (of old demo export format) being imported into DB.
//
// Revision 1.68  2003/08/09 17:18:00  mrimer
// Fixed bug: corrupted data causes export crash.
//
// Revision 1.67  2003/07/31 21:29:39  mrimer
// Added GetTimeElapsed().
//
// Revision 1.66  2003/07/29 13:38:36  mrimer
// Revised order of demo tag fields in export.  Fixed bugs in demo import (not using demo map; mapping next demo ID).
//
// Revision 1.65  2003/07/23 19:06:15  mrimer
// Fixed bug in demo export/import.
//
// Revision 1.64  2003/07/22 18:37:33  mrimer
// Changed reinterpret_casts to static_casts.
//
// Revision 1.63  2003/07/22 18:26:58  mrimer
// Added text explanation when a demo playback is broken.
// Moved hardcoded texts to Texts/DemosMessages.uni.
//
// Revision 1.62  2003/07/19 02:32:32  mrimer
// Made export code more robust and maintainable.
//
// Revision 1.61  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.60  2003/07/09 21:20:19  mrimer
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.59  2003/07/07 23:44:37  mrimer
// Added more import handling logic.
//
// Revision 1.58  2003/07/03 21:38:56  mrimer
// Port warning fix (committed on behalf of Gerry JJ).
//
// Revision 1.57  2003/06/26 17:41:22  mrimer
// Fixed bug: show sequence #s not being updated on deleting show sequence demo.
// Made DB access code more robust.
// Ignore importing records for non-existant holds.
//
// Revision 1.56  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.55  2003/06/20 00:43:54  mrimer
// Added robust handling of player death due to broken saved game.
//
// Revision 1.54  2003/06/19 05:45:17  mrimer
// Fixed bug: show sequence numbers duplicated on input.
//
// Revision 1.53  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.52  2003/06/17 22:16:51  mrimer
// Added filtering by hold support.
//
// Revision 1.51  2003/06/12 21:44:11  mrimer
// Added another saved game/demo validity check.
//
// Revision 1.50  2003/06/03 06:21:20  mrimer
// Now calls ResetMembership() on DB update.
//
// Revision 1.49  2003/05/25 22:46:25  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.48  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.47  2003/05/22 23:39:01  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.46  2003/05/20 18:12:41  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.45  2003/05/08 23:21:01  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.44  2003/05/08 22:01:07  mrimer
// Replaced local CDb instances with a pointer to global instance.
//
// Revision 1.43  2003/04/13 02:06:49  mrimer
// Added GetHoldIDofDemo(dwDemoID).
//
// Revision 1.42  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.41  2003/02/24 17:03:03  erikh2000
// Replaced all calls to obsolete CDbBase::GetStorage() to use CDbBase::GetView().
//
// Revision 1.40  2003/02/16 20:29:31  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.39  2003/01/08 00:50:15  mrimer
// Added an include.
//
// Revision 1.38  2002/12/22 02:07:42  mrimer
// Added XML import/export support.  Removed temporary export routines.
// Enhanced membership filters to support multiple players.
//
// Revision 1.37  2002/11/22 22:00:18  mrimer
// Fixed a subtle bug with membership loading.
//
// Revision 1.36  2002/11/22 02:05:12  mrimer
// Added GetNew().
//
// Revision 1.35  2002/11/14 19:04:22  mrimer
// Added methods to set and reset a demo's show sequence number.
//
// Revision 1.34  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.33  2002/10/17 16:49:32  mrimer
// Fixed calculation of wMonstersSlain to take spawning (etc) into account.
//
// Revision 1.32  2002/10/10 03:15:02  erikh2000
// Fixed several problems with demo narration.
//
// Revision 1.31  2002/10/01 22:37:04  erikh2000
// Wrote method to get a current game from a demo.
// Wrote method to get a scene of specified maximum length.
//
// Revision 1.30  2002/08/30 00:22:43  erikh2000
// Finished demo import code.
//
// Revision 1.29  2002/08/29 17:28:11  mrimer
// Added tentative implementation of CDbDemo::LoadFromFile() and CDbDemos::GetFromFile().
//
// Revision 1.28  2002/08/29 09:19:11  erikh2000
// Added method to save a demo to a file.
//
// Revision 1.27  2002/07/19 20:24:35  mrimer
// Modified CCueEvents to work with CAttachableObject.
//
// Revision 1.26  2002/07/17 20:07:12  erikh2000
// Added method to find more interesting range of a demo.
// Revised #includes.
//
// Revision 1.25  2002/06/21 03:29:32  erikh2000
// Renamed GetAllocNarrationText_English() for consistency.
//
// Revision 1.24  2002/06/21 03:11:55  erikh2000
// Fixed a logic error in CDbDemo::GetNarration().
//
// Revision 1.23  2002/06/20 04:05:58  erikh2000
// CDbDemos can now filter to only include demos in the show sequence.
//
// Revision 1.22  2002/06/16 22:03:53  erikh2000
// Fixed a couple of errors.
//
// Revision 1.21  2002/06/16 06:21:29  erikh2000
// Wrote method to delete a demo.
//
// Revision 1.20  2002/06/15 18:26:40  erikh2000
// Changed code so that storage is not committed after each database write.
//
// Revision 1.19  2002/06/09 06:12:32  erikh2000
// Added IsHidden field handling and hidden demo filtering.
// Changed code to use new table structure.
// Added CDbDemo::GetAuthorText().
//
// Revision 1.18  2002/06/05 23:59:10  mrimer
// Rearranged includes for robustness.
//
// Revision 1.17  2002/05/21 19:05:30  erikh2000
// Added narrative description generator for demos.
//
// Revision 1.16  2002/05/15 01:20:08  erikh2000
// Added CDbDemos method that finds latest recorded demo.
//
// Revision 1.15  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.14  2002/03/30 21:54:28  erikh2000
// Fixed an error in CDbDemo::Test().
//
// Revision 1.13  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.12  2002/02/26 11:46:52  erikh2000
// Slight rearrangement of game state checks in CDbDemo::Test().
// Changed CDbDemo::Update() to call routines to update for new or existing demos record.
// Fixed problem with incorrect DS_ProcessedTurnCount demo stat returned from CDbDemo::Test().
//
// Revision 1.11  2002/02/24 01:25:32  erikh2000
// Fixed calls to CCurrentGame methods to use new cue events param.
//
// Revision 1.10  2002/02/23 04:58:34  erikh2000
// Added call to CCurrentGame::FreezeCommands() in CDbDemo::Test().
// Added DS_MonsterCount demo stat.
//
// Revision 1.9  2002/02/15 02:46:04  erikh2000
// Changed call to CDb::GetSavedCurrentGame() to restore at beginning of room.
// Added checks for player leaving room to CDbDemo::Test().
//
// Revision 1.8  2002/02/07 22:33:06  erikh2000
// Wrote CDbDemo::Update().
// Changed CDbDemo::Test() so it returns statistics when testing fails and fixed bad cleanup.
//
// Revision 1.7  2001/12/16 02:08:40  erikh2000
// Added filter by room functionality to CDbDemos.
// Wrote CDbDemo::Load().
// Wrote CDbDemo::Test().
//
// Revision 1.6  2001/12/08 03:18:49  erikh2000
// Added comment.
//
// Revision 1.5  2001/12/08 02:37:51  erikh2000
// Fixed a comment typo.
//
// Revision 1.4  2001/12/08 01:41:08  erikh2000
// Added GetFirst() and GetNext() methods.
//
// Revision 1.3  2001/11/23 01:08:10  erikh2000
// Added CDbDemo::Test() stub.
//
// Revision 1.2  2001/11/20 00:53:39  erikh2000
// Added field members to CDbDemo.
// Added CDbDemo::Update() stub.
// Made constructor protected.
//
// Revision 1.1.1.1  2001/10/01 22:20:08  erikh2000
// Initial check-in.
//
