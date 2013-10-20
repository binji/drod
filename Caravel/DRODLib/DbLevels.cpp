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

//DbLevels.cpp
//Implementation of CDbLevels and CDbLevel.

#include "DbLevels.h"

#include "Db.h"
#include "DBProps.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Base64.h>
#include <BackEndLib/Ports.h>

//
//CDbLevels public methods.
//

//*****************************************************************************
void CDbLevels::Delete(
//Deletes records for a level with the given ID.
//Also deletes all rooms in the level (and their scrolls, saved games and demos).
//NOTE: Does not remove the level (i.e. references to its ID) from its hold,
//	since this might need to be handled in a specific way.
//	RemoveFromHold() should be called following a call to Delete().
//
//Params:
	const DWORD dwLevelID)	//(in)	ID of level to delete.
{
	ASSERT(dwLevelID);

	//Get index in DB.
	c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	const DWORD dwLevelRowI = LookupRowByPrimaryKey(dwLevelID, p_LevelID, LevelsView);
	if (dwLevelRowI == ROW_NO_MATCH) {ASSERTP(false, "Bad level ID."); return;}

	//Delete all rooms in level (and their scrolls, saved games and demos).
   CDb db;
	CIDList RoomIDs;
	db.Rooms.FilterBy(dwLevelID);
	db.Rooms.GetIDs(RoomIDs);
	IDNODE *pSeek = RoomIDs.Get(0);
	while (pSeek)
	{
		db.Rooms.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}

	//Delete name and description message texts.
	const DWORD dwNameMID = p_NameMessageID( LevelsView[dwLevelRowI] );
	if (!dwNameMID) {ASSERTP(false, "Bad MID for name"); return;}
	DeleteMessage(static_cast<MESSAGE_ID>(dwNameMID));
	const DWORD dwDescriptionMID = p_DescriptionMessageID( LevelsView[dwLevelRowI] );
	if (!dwDescriptionMID) {ASSERTP(false, "Bad MID for description."); return;}
	DeleteMessage(static_cast<MESSAGE_ID>(dwDescriptionMID));

	//Delete the level.
	LevelsView.RemoveAt(dwLevelRowI);

	//After level object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//*****************************************************************************
string CDbLevels::ExportXML(
//Returns: string containing XML text describing level with this ID
//				AND all rooms having this LevelID
//
//Pre-condition: dwLevelID is valid
//
//Params:
	const DWORD dwLevelID,	//(in)
	CDbRefs &dbRefs,			//(in/out)
	const bool bRef)			//(in) Only export GUID reference (default = false)
{
#define STARTTAG(vType,pType) "<"; str += ViewTypeStr(vType); str += " "; str += PropTypeStr(pType); str += "='"
#define PROPTAG(pType) "' "; str += PropTypeStr(pType); str += "='"
#define ENDTAG(vType) "</"; str += ViewTypeStr(vType); str += ">\n"
#define CLOSETAG "'/>\n"
#define CLOSESTARTTAG "'>\n"
#define LONGTOSTR(val) _ltoa((val), dummy, 10)

   string str;

	if (!dbRefs.IsSet(V_Levels,dwLevelID))
	{
		dbRefs.Set(V_Levels,dwLevelID);

		//Prepare data.
		char dummy[32];
		CDbLevel *pLevel = GetByID(dwLevelID);
		ASSERT(pLevel);
      if (!pLevel) return str; //shouldn't happen, but just in case

		//Include corresponding hold ref.
		str += g_pTheDB->Holds.ExportXML(pLevel->dwHoldID, dbRefs, true);
		if (!bRef)
		{
			//Include corresponding player ref.
			str += g_pTheDB->Players.ExportXML(pLevel->dwPlayerID, dbRefs, true);
		}

		str += STARTTAG(V_Levels, P_HoldID);
		str += LONGTOSTR(pLevel->dwHoldID);
		str += PROPTAG(P_GID_LevelIndex);
		str += LONGTOSTR(pLevel->dwLevelIndex);
		if (!bRef)
		{
			//Prepare data.
			WSTRING const wNameStr = (WSTRING)pLevel->NameText;
			WSTRING const wDescStr = (WSTRING)pLevel->DescriptionText;

			str += PROPTAG(P_PlayerID);
			str += LONGTOSTR(pLevel->dwPlayerID);
			str += PROPTAG(P_NameMessage);
			str += Base64::encode(wNameStr);
			str += PROPTAG(P_DescriptionMessage);
			str += Base64::encode(wDescStr);
			str += PROPTAG(P_RoomID);
			str += LONGTOSTR(pLevel->dwRoomID);
			str += PROPTAG(P_X);
			str += LONGTOSTR(pLevel->wX);
			str += PROPTAG(P_Y);
			str += LONGTOSTR(pLevel->wY);
			str += PROPTAG(P_O);
			str += LONGTOSTR(pLevel->wO);
			str += PROPTAG(P_Created);
			str += LONGTOSTR((time_t)pLevel->Created);
			str += PROPTAG(P_LastUpdated);
			str += LONGTOSTR((time_t)pLevel->LastUpdated);
		}
		//Put primary key last, so all message fields have been set by the
		//time Update() is called during import.
		str += PROPTAG(P_LevelID);
		str += LONGTOSTR(pLevel->dwLevelID);
		if (bRef)
		{
			//Don't need any room information.
			str += CLOSETAG;
		} else {
			str += CLOSESTARTTAG;

			//Export all rooms in level.
         CDb db;
			CIDList RoomIDs;
			db.Rooms.FilterBy(dwLevelID);
			db.Rooms.GetIDs(RoomIDs);
			const DWORD dwNumRooms = RoomIDs.GetSize();
			for (DWORD dwIndex=0; dwIndex<dwNumRooms; ++dwIndex)
			{
				str += db.Rooms.ExportXML(RoomIDs.Get(dwIndex)->dwID, dbRefs);
			}

			str += ENDTAG(V_Levels);
		}

		delete pLevel;
	}

	return str;

#undef STARTTAG
#undef PROPTAG
#undef ENDTAG
#undef CLOSETAG
#undef CLOSESTARTTAG
#undef LONGTOSTR
}

//*****************************************************************************
CDbLevel * CDbLevels::GetNew()
//Get a new level object that will be added to database when it is updated.
//
//Returns:
//Pointer to new level.
{
	//After level object is updated, membership changes, so reset the flag.
	this->bIsMembershipLoaded = false;

	//Create new level object.
	CDbLevel *pLevel = new CDbLevel;

	//Put level in specified hold.
	pLevel->dwHoldID = this->dwFilterByHoldID;

	return pLevel;
}

//*****************************************************************************
void CDbLevels::FilterBy(
//Changes filter so that GetFirst() and GetNext() will return levels for a specified hold.
//
//Params:
	const DWORD dwSetFilterByHoldID)	//(in)	Hold ID to filter by.  Set to 0 for all levels.
{
	if (dwSetFilterByHoldID != this->dwFilterByHoldID && this->bIsMembershipLoaded)
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByHoldID = dwSetFilterByHoldID;
}

//
//CDbLevels private methods.
//

//*****************************************************************************
void CDbLevels::LoadMembership()
//Load the membership list with all level IDs.
{
	ASSERT(IsOpen());
	c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	const DWORD dwLevelCount = LevelsView.GetSize();

	//Each iteration gets a level ID and puts in membership list.
	this->MembershipIDs.Clear();
	for (DWORD dwLevelI = 0; dwLevelI < dwLevelCount; ++dwLevelI)
	{
		const DWORD dwHoldID = p_HoldID(LevelsView[dwLevelI]);
		if (this->dwFilterByHoldID == 0 || dwHoldID == this->dwFilterByHoldID)
			this->MembershipIDs.Add(p_LevelID(LevelsView[dwLevelI]));
	}
	this->pCurrentRow = this->MembershipIDs.Get(0);
	this->bIsMembershipLoaded = true;
}

//
//CDbLevel protected methods.
//

//*****************************************************************************
CDbLevel::CDbLevel()
//Constructor.
{	
	Clear();
}


//
//CDbLevel public methods.
//

//*****************************************************************************
CDbLevel::~CDbLevel()
//Destructor.
{
	Clear();
}

//*****************************************************************************
const WCHAR * CDbLevel::GetAuthorText() const
//Returns author of the level or NULL if not found.
{
	//Look up NameMessageID from associated Players record.
	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayersRowI = LookupRowByPrimaryKey(this->dwPlayerID,
			p_PlayerID, PlayersView);
	if (dwPlayersRowI == ROW_NO_MATCH) {ASSERTP(false, "Bad player row."); return NULL;}
	const DWORD dwNameMessageID = p_NameMessageID( PlayersView[dwPlayersRowI] );

	//Look up message text.
	return g_pTheDB->GetMessageText(static_cast<MESSAGE_ID>(dwNameMessageID));
}

//*****************************************************************************
DWORD CDbLevel::GetDefaultNextLevelID()
//Gets the ID for level that comes after this one.  This is accomplished by
//finding the room in the level with only one staircase in it.
//If there are no rooms with a single staircase, the first staircase found
//will be used.
//If there are no staircases, return 0.
//
//Returns:
//Level ID, or 0L if no level comes after this one.
const
{
	c4_View RoomsView = GetView(ViewTypeStr(V_Rooms));
	const DWORD dwRoomCount = RoomsView.GetSize();

	//Look for a room with one staircase in it.
	DWORD dwRoomI;
	for (dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
	{
		//Get rooms in this level.
		const DWORD dwLevelID = (DWORD) p_LevelID(RoomsView[dwRoomI]);
		if (dwLevelID != this->dwLevelID)
			continue;

		c4_View ExitsView = p_Exits(RoomsView[dwRoomI]);
		const DWORD dwExitCount = ExitsView.GetSize();
		if (dwExitCount == 1)
			return (DWORD) p_LevelID(ExitsView[0]);
	}

	//Look for a room with stairs in it.
	for (dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
	{
		//Get rooms in this level.
		const DWORD dwLevelID = (DWORD) p_LevelID(RoomsView[dwRoomI]);
		if (dwLevelID != this->dwLevelID)
			continue;

		c4_View ExitsView = p_Exits(RoomsView[dwRoomI]);
		const DWORD dwExitCount = ExitsView.GetSize();
		if (dwExitCount > 0)
			return (DWORD) p_LevelID(ExitsView[0]);
	}

	return 0L;
}


//*****************************************************************************
bool CDbLevel::Load(
//Loads a level from database into this object.
//
//Params:
	const DWORD dwLoadLevelID)	//(in) LevelID of level to load.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;
	c4_View LevelsView;

	Clear();

	//Open levels view.
	ASSERT(IsOpen());
	LevelsView = GetView(ViewTypeStr(V_Levels));

	//Find record with matching level ID.
	const DWORD dwLevelI = LookupRowByPrimaryKey(dwLoadLevelID, p_LevelID, LevelsView);
	if (dwLevelI == ROW_NO_MATCH) {bSuccess=false; goto Cleanup;}

	//Load in props from Levels record.
	this->dwLevelID = (DWORD) (p_LevelID(LevelsView[dwLevelI]));
	this->dwHoldID = (DWORD) (p_HoldID(LevelsView[dwLevelI]));
	this->dwRoomID = (DWORD) (p_RoomID(LevelsView[dwLevelI]));
	this->dwPlayerID = (DWORD) (p_PlayerID(LevelsView[dwLevelI]));
	this->wX = p_X(LevelsView[dwLevelI]);
	this->wY = p_Y(LevelsView[dwLevelI]);
	this->wO = p_O(LevelsView[dwLevelI]);
	this->Created = (time_t) (p_Created(LevelsView[dwLevelI]));
	this->LastUpdated = (time_t) (p_LastUpdated(LevelsView[dwLevelI]));
	this->DescriptionText.Bind((DWORD) p_DescriptionMessageID(LevelsView[dwLevelI]));
	this->NameText.Bind((DWORD) p_NameMessageID(LevelsView[dwLevelI]));
	this->dwLevelIndex = (DWORD) (p_GID_LevelIndex(LevelsView[dwLevelI]));

	//Filter Rooms member to only show rooms from this level.
	this->Rooms.FilterBy(this->dwLevelID);

	//Filter SavedGames member to only show saved games from this level.
   //AND
   //for only the current player.
	this->SavedGames.FilterByLevel(this->dwLevelID);
   this->SavedGames.FilterByPlayer(g_pTheDB->GetPlayerID());

Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
CDbHold * CDbLevel::GetHold()
//Get hold associated with this level.
//
//Returns:
//Pointer to loaded hold which caller must delete, or NULL if no matching hold 
//was found.
const
{
	CDbHold *pHold = new CDbHold();
	if (pHold)
	{
		if (!pHold->Load(this->dwHoldID))
		{
			delete pHold;
			pHold=NULL;
		}
	}
	return pHold;
}

//*****************************************************************************
void CDbLevel::GetRequiredRooms(
//Gets list of rooms in level required to conquer in order to complete level.
//
//Params:
	CIDList& requiredRooms)	//(out)
{
	CIDList roomsInLevel;
	this->Rooms.GetIDs(roomsInLevel);
	IDNODE *pRoomNode = roomsInLevel.Get(0);
	while (pRoomNode)
	{
		if (CDbRoom::IsRequired(pRoomNode->dwID))
			requiredRooms.Add(pRoomNode->dwID);
		pRoomNode = pRoomNode->pNext;
	}
}

//*****************************************************************************
CDbRoom * CDbLevel::GetStartingRoom()
//Gets starting room associated with this level.
//
//Returns:
//Pointer to a new loaded room object which caller must delete or NULL if could 
//not load room.
{
	if (!this->dwRoomID) return NULL;

	CDbRoom *pRoom = new CDbRoom();
	if (pRoom)
	{
		if (pRoom->Load(this->dwRoomID))
		{
			//Remember starting room coords.
			if (!this->bGotStartingRoomCoords)
			{
				this->dwStartingRoomX = pRoom->dwRoomX;
				this->dwStartingRoomY = pRoom->dwRoomY;
				this->bGotStartingRoomCoords = true;
			}
		}
		else
		{
			delete pRoom;
			pRoom = NULL;
		}
	}
	return pRoom;
}

//*****************************************************************************
void CDbLevel::GetStartingRoomCoords(
//Gets coordinates of starting room for level.
//Edited levels might not have a starting room, so return (0,0) in this case.
//
//Params:
	DWORD &dwRoomX, DWORD &dwRoomY)	//(out) The starting room coords.
{
	//Do I already have starting room coordinates.
	if (!this->bGotStartingRoomCoords)
	{
		//No--Look up the coords.
		ASSERT(IsOpen());
		c4_View RoomsView = GetView(ViewTypeStr(V_Rooms));
		const DWORD dwRoomI = CDbBase::LookupRowByPrimaryKey(this->dwRoomID,
				p_RoomID, RoomsView);
		if (dwRoomI == ROW_NO_MATCH) {dwRoomX = 0L; dwRoomY = 0L; return;}
		this->dwStartingRoomX = p_RoomX( RoomsView[dwRoomI] );
		this->dwStartingRoomY = p_RoomY( RoomsView[dwRoomI] );
		this->bGotStartingRoomCoords = true;
	}

	//Return coords.
	dwRoomX = this->dwStartingRoomX;
	dwRoomY = this->dwStartingRoomY;
}

//*****************************************************************************
CDbRoom * CDbLevel::GetRoomAtCoords(
//Gets a room at specified coords in this level.
//
//Params:
	const DWORD dwRoomX,	const DWORD dwRoomY)	//(in)	Coords of room to get.
//
//Returns:
//Pointer to a new loaded room object which caller must delete or NULL if could 
//not load room.
{
	CDbRoom *pRoom = NULL;

	//Find matching room ID.
	const DWORD dwLoadRoomID = FindRoomIDAtCoords(dwRoomX, dwRoomY);
	if (!dwLoadRoomID) return NULL;

	//Load room.
	pRoom = new CDbRoom();
	if (pRoom)
	{
		if (!pRoom->Load(dwLoadRoomID))
		{
			delete pRoom;
			pRoom = NULL;
		}
		
		//Remember starting room coords if this is the starting room.
		if (dwLoadRoomID == this->dwRoomID && !this->bGotStartingRoomCoords)
		{
			this->dwStartingRoomX = pRoom->dwRoomX;
			this->dwStartingRoomY = pRoom->dwRoomY;
			this->bGotStartingRoomCoords = true;
		}
	}
	return pRoom;
}

//*****************************************************************************
CDbLevel* CDbLevel::MakeCopy(
//Creates a copy of the entire level, saving it to the DB.
//
//Returns: pointer to new level
//
//Params:
   const DWORD dwNewHoldID)  //(in) hold new level belongs to
{
	CDbLevel *pNewLevel = g_pTheDB->Levels.GetNew();
   if (!pNewLevel) return NULL;

	*pNewLevel = *this;        //must make new message texts
	pNewLevel->dwLevelID = 0L;	//so this level gets added to DB as a new level
   pNewLevel->Created = 0;    //update timestamps
	pNewLevel->dwHoldID = dwNewHoldID;
	pNewLevel->Update();

   //Make a copy of all the rooms in level.
   const DWORD dwEntranceRoomID = SaveCopyOfRooms(pNewLevel->dwLevelID);

   //Set new level's entrance room ID.
	ASSERT(dwEntranceRoomID);
	pNewLevel->dwRoomID = dwEntranceRoomID;
	pNewLevel->Update();

   return pNewLevel;
}

//*****************************************************************************
MESSAGE_ID CDbLevel::SetProperty(
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
		case P_LevelID:
		{
			this->dwLevelID = static_cast<DWORD>(atol(str));
			if (!this->dwLevelID)
				return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = info.LevelIDMap.find(this->dwLevelID);
			if (localID != info.LevelIDMap.end())
				//Error - this level should not have been imported already.
				return MID_FileCorrupted;

			//Look up level in the DB.
         DWORD dwLocalLevelID;

         //!!Fixes bug from pre-build 30(?) builds: corrupted LevelIndex.
         //Explanation: If this level being imported has the same local ID
         //as another level that was just imported, this level's LevelIndex
         //is corrupted and should be updated.
         //NOTE: This code will probably have to be revised if ever multiple
         //holds can be imported simultaneously with levels pointing to each other.
         while (true)
         {
            dwLocalLevelID = GetLocalID();
            if (!dwLocalLevelID)
               break;

            //Finding the local level ID in the map means a level w/ this GUID
            //was just imported, implying one of them is corrupted.
            localID = info.LevelIDMap.begin();
            while (localID != info.LevelIDMap.end())
            {
               if (localID->second == dwLocalLevelID)
                  break;
               ++localID;
            }
		      if (localID == info.LevelIDMap.end())
               break;   //didn't find another level being imported w/ same GUID
            //Update this level's index and try again.
            ++this->dwLevelIndex;
         }

			if (dwLocalLevelID)
			{
				//Level found in DB, but was not just imported.
				info.LevelIDMap[this->dwLevelID] = dwLocalLevelID;
				this->dwLevelID = dwLocalLevelID;
				bSaveRecord = false;
			} else {
            if (bSaveRecord)
            {
				   //Add a new record to the DB.
				   const DWORD dwOldLocalID = this->dwLevelID;
				   this->dwLevelID = 0L;
				   Update();
				   info.LevelIDMap[dwOldLocalID] = this->dwLevelID;
            } else {
               //This level is being ignored.
               //(It's probably a GUID reference to a non-existant hold.)
               info.LevelIDMap[this->dwLevelID] = 0;   //skip records with refs to this level ID
            }
			}
			break;
		}
		case P_HoldID:
		{
			this->dwHoldID = static_cast<DWORD>(atol(str));
			if (!this->dwHoldID)
				return MID_FileCorrupted;	//corrupt data (can't load a level w/o its hold)

			//Set to local ID.
			localID = info.HoldIDMap.find(this->dwHoldID);
			if (localID == info.HoldIDMap.end())
				return MID_HoldNotFound;	//record should have been loaded already
			this->dwHoldID = (*localID).second;
         if (!this->dwHoldID)
         {
            //Records for this hold are being ignored.  Don't save this level.
            bSaveRecord = false;
         }
			break;
		}
		case P_PlayerID:
		{
			this->dwPlayerID = static_cast<DWORD>(atol(str));
			if (!this->dwPlayerID)
				return MID_FileCorrupted;	//corrupt file (must have an author)

			//Set to local ID.
			localID = info.PlayerIDMap.find(this->dwPlayerID);
			if (localID == info.PlayerIDMap.end())
				return MID_FileCorrupted;	//record should have been loaded already
			this->dwPlayerID = (*localID).second;
			break;
		}
		case P_NameMessage:
		{
			WSTRING data;
			Base64::decode(str,data);
			this->NameText = data.c_str();
			break;
		}
		case P_DescriptionMessage:
		{
			WSTRING data;
			Base64::decode(str,data);
			this->DescriptionText = data.c_str();
			break;
		}
		case P_RoomID:
			this->dwRoomID = static_cast<DWORD>(atol(str));
			//Room hasn't been read in yet and local ID must be set later
			break;
		case P_X:
			this->wX = static_cast<UINT>(atoi(str));
			break;
		case P_Y:
			this->wY = static_cast<UINT>(atoi(str));
			break;
		case P_O:
			this->wO = static_cast<UINT>(atoi(str));
			break;
		case P_Created:
			this->Created = (time_t)(atol(str));
			break;
		case P_LastUpdated:
			this->LastUpdated = (time_t)(atol(str));
			break;
		case P_GID_LevelIndex:
			this->dwLevelIndex = static_cast<DWORD>(atol(str));
			if (!this->dwLevelIndex)
				return MID_FileCorrupted;	//corrupt file
         break;
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbLevel::Update()
//Updates database with level.
//
//Returns: true if successful, else false.
{
	bool bSuccess=true;

   g_pTheDB->Levels.ResetMembership();
	if (this->dwLevelID == 0)
	{
		//Insert a new level.
		bSuccess = UpdateNew();
	}
	else
	{
		//Update existing level.
		bSuccess = UpdateExisting();
	}

	if (!bSuccess) return false;

	//Filter Rooms member to only show rooms from this level.
	this->Rooms.FilterBy(this->dwLevelID);

	//Filter SavedGames member to only show saved games from this level.
	this->SavedGames.FilterByLevel(this->dwLevelID);

	return bSuccess;
}

//
//CDbLevel private methods.
//

//*****************************************************************************
DWORD CDbLevel::GetLocalID() const
//Compares this object's GID fields against those of the records in the DB.
//ASSUME: dwHoldID has already been set to the local record ID
//
//Returns: local ID if a record in the DB matches this object's GUID, else 0
{
	ASSERT(IsOpen());
	c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	const DWORD dwLevelCount = LevelsView.GetSize();

	//Each iteration checks a level's GIDs.
	for (DWORD dwLevelI = 0; dwLevelI < dwLevelCount; ++dwLevelI)
	{
		//Check hold.
		const DWORD dwHoldID = (DWORD)p_HoldID(LevelsView[dwLevelI]);
		if (this->dwHoldID == dwHoldID)
		{
			//Check level index.
			const DWORD dwLevelIndex = (DWORD)p_GID_LevelIndex(LevelsView[dwLevelI]);
			if (this->dwLevelIndex == dwLevelIndex)
			{
				//GUIDs match.  Return this record's local ID.
				return (DWORD) p_LevelID(LevelsView[dwLevelI]);
			}
		}
	}

	//No match.
	return 0L;
}

//*****************************************************************************
bool CDbLevel::UpdateNew()
//Add new Levels record to database.
{
	ASSERT(this->dwLevelID == 0);
	ASSERT(IsOpen());

	//Prepare props.
	this->dwLevelID = GetIncrementedID(p_LevelID);
	if ((time_t)this->Created == 0)
	{
		this->Created.SetToNow();
		this->LastUpdated.SetToNow();
	}
	//ASSERT(this->dwLevelIndex); //can't assume this: it will be 0 if it hasn't
   //been inserted into a hold yet, since only a level with a real dwLevelID
   //can be inserted into a hold.

	//Write out message texts.
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwDescID = this->DescriptionText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwDescID);

	//Write Level record.
	c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	LevelsView.Add(
			p_LevelID[ this->dwLevelID ] +
			p_HoldID[ this->dwHoldID ] +
			p_PlayerID[ this->dwPlayerID ] +
			p_NameMessageID[ dwNameID ] +
			p_DescriptionMessageID[ dwDescID ] +
			p_RoomID[ this->dwRoomID ] +
			p_X[ this->wX ] +
			p_Y[ this->wY ] +
			p_O[ this->wO ] +
			p_Created[ this->Created ] +
			p_LastUpdated[ this->LastUpdated ] +
			p_GID_LevelIndex[ this->dwLevelIndex ]);

	return true;
}

//*****************************************************************************
bool CDbLevel::UpdateExisting()
//Update an existing Levels record in database.
{
	ASSERT(this->dwLevelID != 0);
	ASSERT(IsOpen());

	//Lookup Levels record.
	c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	const DWORD dwLevelID = LookupRowByPrimaryKey(this->dwLevelID,
			p_LevelID, LevelsView);
	if (dwLevelID == ROW_NO_MATCH)
	{
		ASSERTP(false, "The caller probably passed a bad PKID.");
		return false;
	}

   ASSERT(this->dwLevelIndex);

	//Update Levels record.
   if (!CDb::FreezingTimeStamps())
	   this->LastUpdated.SetToNow();
	p_LevelID( LevelsView[ dwLevelID ] ) = this->dwLevelID;
	p_HoldID( LevelsView[ dwLevelID ] ) = this->dwHoldID;
	p_PlayerID( LevelsView[ dwLevelID ] ) = this->dwPlayerID;
	p_RoomID( LevelsView[ dwLevelID ] ) = this->dwRoomID;
	p_X( LevelsView[ dwLevelID ] ) = this->wX;
	p_Y( LevelsView[ dwLevelID ] ) = this->wY;
	p_O( LevelsView[ dwLevelID ] ) = this->wO;
	p_Created( LevelsView[ dwLevelID ] ) = this->Created;
	p_LastUpdated( LevelsView[ dwLevelID ] ) = this->LastUpdated;
	p_GID_LevelIndex( LevelsView[ dwLevelID ] ) = this->dwLevelIndex;

	//Write out message texts.
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwDescID = this->DescriptionText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwDescID);

	return true;
}

//*****************************************************************************
DWORD CDbLevel::FindRoomIDAtCoords(
//Finds room at specified coords in this level.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY) //(in) Coords of room to find.
//
//Returns:
//RoomID of found room, or 0 if no match.
const
{
	c4_View RoomsView;

	ASSERT(IsOpen());
	RoomsView = GetView(ViewTypeStr(V_Rooms));
	const DWORD dwRoomCount = RoomsView.GetSize();

	//Scan through all the rooms to find a match.
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; dwRoomI++)
	{
		const DWORD dwEvalRoomX = (DWORD) p_RoomX(RoomsView[dwRoomI]);
		const DWORD dwEvalRoomY = (DWORD) p_RoomY(RoomsView[dwRoomI]);
		const DWORD dwEvalLevelID = (DWORD) p_LevelID(RoomsView[dwRoomI]);
		
		if (dwEvalLevelID == this->dwLevelID &&
			dwEvalRoomY == dwRoomY &&
			dwEvalRoomX == dwRoomX)
		{
			return (DWORD) p_RoomID(RoomsView[dwRoomI]); //Found it.
		}
	}
	return 0; //Didn't find it.
}

//*****************************************************************************
void CDbLevel::Clear()
//Frees resources associated with this object and resets member vars.
{	
	this->dwLevelID = this->dwHoldID = this->dwPlayerID = this->dwRoomID = 
			this->dwStartingRoomX = this->dwStartingRoomY = 0L;
	this->wX = this->wY = this->wO = 0;
	this->Created = this->LastUpdated = 0L;
	this->dwLevelIndex = 0L;
	this->bGotStartingRoomCoords = false;

	this->NameText.Clear();
	this->DescriptionText.Clear();
}

//*****************************************************************************
DWORD CDbLevel::SaveCopyOfRooms(
//Make copies of all rooms in level in the DB.
//
//Returns: ID of new entrance room
//
//Params:
   const DWORD dwNewLevelID)  //(in) level new rooms belong to
{
	DWORD dwEntranceRoomID = 0L;
	this->Rooms.FilterBy(this->dwLevelID);
	CDbRoom *pRoom = this->Rooms.GetFirst();
	while (pRoom)
	{
		const bool bEntranceRoom = pRoom->dwRoomID == this->dwRoomID;
		CDbRoom *pRoomCopy = pRoom->MakeCopy(); //must make new message texts
		pRoomCopy->dwLevelID = dwNewLevelID;
      //keep room (x,y) coords synched with local levelID
      pRoomCopy->dwRoomY = (dwNewLevelID * 100) + (pRoomCopy->dwRoomY % 100);
		pRoomCopy->dwRoomID = 0L;	//so this room gets added to DB as a new room
		pRoomCopy->Update();
		if (bEntranceRoom)
		{
			//Get level's new entrance room ID.
			ASSERT(dwEntranceRoomID == 0L);	//there should only be one
			dwEntranceRoomID = pRoomCopy->dwRoomID;
		}
		delete pRoom;
		delete pRoomCopy;
		pRoom = this->Rooms.GetNext();
	}

   ASSERT(dwEntranceRoomID);  //there should always be an entrance room
   return dwEntranceRoomID;
}

//*****************************************************************************
bool CDbLevel::SetMembers(
//For copy constructor and assignment operator.
//
//Params:
	const CDbLevel &Src)
{
	//primitive types
	this->dwLevelID = Src.dwLevelID;
	this->dwHoldID = Src.dwHoldID;
	this->dwPlayerID = Src.dwPlayerID;
	this->dwRoomID = Src.dwRoomID;
	this->wX = Src.wX;
	this->wY = Src.wY;
	this->wO = Src.wO;
	this->bGotStartingRoomCoords = false;	//reset this (takes care of dwStartingRoomX/Y)
   this->dwLevelIndex = Src.dwLevelIndex;

	//object members
	this->NameText = Src.NameText;	//make new message texts
	this->DescriptionText = Src.DescriptionText;
	this->Created = Src.Created;
	this->LastUpdated = Src.LastUpdated;

   this->Rooms.ResetMembership();
	this->Rooms.FilterBy(this->dwLevelID);
   this->SavedGames.ResetMembership();
	this->SavedGames.FilterByLevel(this->dwLevelID);

	return true;
}

//*****************************************************************************
void CDbLevels::UpdateExitIDs(
//When a level is copied from an old hold to a new hold, then the stairs'
//destination level IDs must be updated.  If an individual level is copied, the
//only reliable thing to do is reset all the IDs.  If an entire hold is copied,
//then the IDs must be re-linked to the IDs of the newly-created levels.
//
//Params:
   const DWORD dwLevelID, const DWORD dwNewHoldID, //(in)
   const bool bResetIDs)   //(in) if true, set all exit IDs to 0,
                           //otherwise, link to current hold [default = true]
{
   CDb db;
   db.Rooms.FilterBy(dwLevelID);
   CDbRoom *pRoom = db.Rooms.GetFirst();
   while (pRoom)
   {
      pRoom->UpdateExitIDs(dwNewHoldID, bResetIDs);
      delete pRoom;
      pRoom = db.Rooms.GetNext();
   }
}

// $Log: DbLevels.cpp,v $
// Revision 1.59  2005/03/15 21:46:54  mrimer
// Fixed bug: scrolls corrupted when level is copied.
//
// Revision 1.58  2003/11/09 05:23:22  mrimer
// Fixed bug: stair destinations not changed when room is moved to new hold.
//
// Revision 1.57  2003/10/20 17:49:03  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.56  2003/10/07 21:10:34  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.55  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.54  2003/08/12 18:50:00  mrimer
// Moved CDbLevelS::RemoveFromHold() to CDbHold::RemoveLevel().
//
// Revision 1.53  2003/08/06 01:19:37  mrimer
// Fixed bug: level exit IDs not being updated on level copy.
//
// Revision 1.52  2003/07/31 21:29:00  mrimer
// Fixed bug: room Y coord not updated when level is copied.
//
// Revision 1.51  2003/07/25 22:45:11  mrimer
// Tweaking.
//
// Revision 1.50  2003/07/22 18:25:27  mrimer
// Fixed bug: level copying doesn't copy room texts correctly.
// Revised level import process to repair corrupted GUIDs (the LevelIndex member).
//
// Revision 1.49  2003/07/19 02:35:19  mrimer
// Made export code more robust and maintainable.
//
// Revision 1.48  2003/07/16 07:40:12  mrimer
// Fixed 3-4 level indexing bugs.  Rewrote CDbLevel::InsertIntoHold() as CDbHold::InsertLevel().
//
// Revision 1.47  2003/07/15 00:26:31  mrimer
// Added MakeCopy().
//
// Revision 1.46  2003/07/11 17:26:14  mrimer
// Now filtering level's saved game list by player also.
//
// Revision 1.45  2003/07/09 21:22:57  mrimer
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.44  2003/07/07 23:33:51  mrimer
// Added SaveCopyOfRooms().
//
// Revision 1.43  2003/07/02 02:37:16  mrimer
// Added an assertion.
//
// Revision 1.42  2003/07/02 02:03:04  mrimer
// Fixed bug: hold without levels doesn't get first level ID set when level is pasted.
//
// Revision 1.41  2003/06/26 17:35:52  mrimer
// Ignore importing records for non-existant holds.
//
// Revision 1.40  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.39  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.38  2003/06/03 06:21:20  mrimer
// Now calls ResetMembership() on DB update.
//
// Revision 1.37  2003/05/25 22:46:25  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.36  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.35  2003/05/22 23:39:01  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.34  2003/05/20 18:12:41  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.33  2003/05/08 23:21:01  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.32  2003/05/08 22:01:07  mrimer
// Replaced local CDb instances with a pointer to global instance.
//
// Revision 1.31  2003/04/29 11:07:40  mrimer
// Removed an assertion.
//
// Revision 1.30  2003/04/17 20:58:46  mrimer
// Removed wRoomsNeededToComplete and added GetRequiredRooms().
//
// Revision 1.29  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.28  2003/02/24 17:06:24  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.27  2003/02/16 20:29:32  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.26  2003/01/08 00:48:44  mrimer
// Fixed RemoveFromHold() usage.
//
// Revision 1.25  2003/01/04 23:06:55  mrimer
// Fixed a bug.
//
// Revision 1.24  2002/12/22 02:04:31  mrimer
// Added XML import/export support.  (Added GID support, relative to hold.)
// Removed NextLevels field (level exits now supported solely by room exits).
// Added InsertIntoHold() and RemoveFromHold().
// Implemented object assignment and copy construction.
//
// Revision 1.23  2002/11/22 22:00:20  mrimer
// Fixed a subtle bug with membership loading.
//
// Revision 1.22  2002/11/22 02:04:27  mrimer
// Added AddNextLevelID().  Revised Delete() and Update() to update NextLevels fields.
//
// Revision 1.21  2002/11/14 19:09:13  mrimer
// Added GetNew(), GetByID() and Update() methods.
// Moved member initialization into constructor initialization list.
// Made some parameters const.
//
// Revision 1.20  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.19  2002/07/10 03:56:20  erikh2000
// Added CDbLevels method to return IDs contained in membership.
//
// Revision 1.18  2002/06/21 22:27:24  erikh2000
// Added method to lookup message text for author of a level.
//
// Revision 1.17  2002/06/15 18:27:28  erikh2000
// Changed code so that storage is not committed after each database write.
//
// Revision 1.16  2002/06/09 06:15:21  erikh2000
// Changed code to use new message text class.
// Added PlayerID field.
//
// Revision 1.15  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.14  2002/04/28 23:47:10  erikh2000
// Added member to CDbLevel that can be used to access all saved games for a level.
//
// Revision 1.13  2002/03/17 03:08:51  erikh2000
// Fixed logic error in CDbLevel::GetStartingRoom().
//
// Revision 1.12  2002/03/14 23:42:27  erikh2000
// Added CDbLevel::GetStartingRoomCoords() and code in other methods to remember coords of starting room.
//
// Revision 1.11  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.10  2002/02/08 23:20:27  erikh2000
// Added #include <list> to fix compile error.
//
// Revision 1.9  2001/12/16 02:09:42  erikh2000
// Simplified calls to CIDList::Get().
//
// Revision 1.8  2001/12/08 05:16:49  erikh2000
// Added CDbRooms member to CDbLevel.
//
// Revision 1.7  2001/12/08 03:19:10  erikh2000
// Added CDbLevels::GetFirst() and CDbLevel::GetNext() methods.
//
// Revision 1.6  2001/11/03 20:16:57  erikh2000
// Removed OnPlot() and OnLoad() references.
//
// Revision 1.5  2001/10/27 04:41:59  erikh2000
// Added SetOnLoad calls.
//
// Revision 1.4  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.3  2001/10/22 23:56:04  erikh2000
// Changed GetDefaultNextLevel() to GetDefaultNextLevelID().
//
// Revision 1.2  2001/10/21 00:31:29  erikh2000
// Twiddling.
//
// Revision 1.1.1.1  2001/10/01 22:20:09  erikh2000
// Initial check-in.
//
