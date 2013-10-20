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

#include "Base64.h"
#include "Db.h"
#include "DbProps.h"

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
	c4_View LevelsView = GetView("Levels");
	const DWORD dwLevelRowI = LookupRowByPrimaryKey(dwLevelID, p_LevelID, LevelsView);
	if (dwLevelRowI == ROW_NO_MATCH) {ASSERT(false); return;} //Bad level ID.

	//Delete all rooms in level (and their scrolls, saved games and demos).
	CDb db;
	CIDList RoomIDs;
	IDNODE *pSeek;

	db.Rooms.FilterBy(dwLevelID);
	db.Rooms.GetIDs(RoomIDs);
	pSeek = RoomIDs.Get(0);
	while (pSeek)
	{
		db.Rooms.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}

	//Delete name and description message texts.
	const DWORD dwNameMID = p_NameMessageID( LevelsView[dwLevelRowI] );
	if (!dwNameMID) {ASSERT(false); return;}
	DeleteMessage(dwNameMID);
	const DWORD dwDescriptionMID = p_DescriptionMessageID( LevelsView[dwLevelRowI] );
	if (!dwDescriptionMID) {ASSERT(false); return;}
	DeleteMessage(dwDescriptionMID);

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
	string str;

	if (!dbRefs.IsSet(V_Levels,dwLevelID))
	{
		dbRefs.Set(V_Levels,dwLevelID);

		char dummy[32];
		CDbLevel *pLevel = GetByID(dwLevelID);
		ASSERT(pLevel);

		//Include corresponding hold ref.
		CDb db;
		str += db.Holds.ExportXML(pLevel->dwHoldID, dbRefs, true);
		if (!bRef)
		{
			//Include corresponding player ref.
			str += db.Players.ExportXML(pLevel->dwPlayerID, dbRefs, true);
		}

		str += "<Levels HoldID='";
		str += _ltoa(pLevel->dwHoldID, dummy, 10);
		str += "' GID_LevelIndex='";
		str += _ltoa(pLevel->dwLevelIndex, dummy, 10);
		if (!bRef)
		{
			//Prepare data.
			WSTRING const wNameStr = pLevel->NameText;
			WSTRING const wDescStr = pLevel->DescriptionText;

			str += "' PlayerID='";
			str += _ltoa(pLevel->dwPlayerID, dummy, 10);
			str += "' NameMessage='";
			str += Base64::encode(wNameStr);
			str += "' DescriptionMessage='";
			str += Base64::encode(wDescStr);
			str += "' RoomID='";
			str += _ltoa(pLevel->dwRoomID, dummy, 10);
			str += "' X='";
			str += _ltoa(pLevel->wX, dummy, 10);
			str += "' Y='";
			str += _ltoa(pLevel->wY, dummy, 10);
			str += "' O='";
			str += _ltoa(pLevel->wO, dummy, 10);
			str += "' Created='";
			str += _ltoa((time_t)pLevel->Created, dummy, 10);
			str += "' LastUpdated='";
			str += _ltoa((time_t)pLevel->LastUpdated, dummy, 10);
			str += "' RoomsNeededToComplete='";
			str += _ltoa(pLevel->wRoomsNeededToComplete, dummy, 10);
		}
		//Put primary key last, so all message fields have been set by the
		//time Update() is called during import.
		str += "' LevelID='";
		str += _ltoa(pLevel->dwLevelID, dummy, 10);
		if (bRef)
		{
			//Don't need any room information.
			str += "'/>\r\n";
		} else {
			str += "'>\r\n";

			//Export all rooms in level.
			CIDList RoomIDs;
			db.Rooms.FilterBy(dwLevelID);
			db.Rooms.GetIDs(RoomIDs);
			const DWORD dwNumRooms = RoomIDs.GetSize();
			for (DWORD dwIndex=0; dwIndex<dwNumRooms; ++dwIndex)
			{
				str += db.Rooms.ExportXML(RoomIDs.Get(dwIndex)->dwID, dbRefs);
			}

			str += "</Levels>\r\n";
		}

		delete pLevel;
	}

	return str;
}

//*****************************************************************************
void CDbLevels::GetIDs(
//Gets level IDs in membership.
//
//Params:
	CIDList &IDs)	//(out) Receives copy of object's membership list.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(this->bIsMembershipLoaded);

	IDs = this->MembershipIDs;
}

//*****************************************************************************
CDbLevel * CDbLevels::GetByID(
//Get a level by its LevelID.
//
//Params:
	const DWORD dwLevelID)	//(in)
//
//Returns:
//Pointer to loaded level which caller must delete, or NULL if no matching level 
//was found.
{
	CDbLevel *pLevel = new CDbLevel();
	if (pLevel)
	{
		if (!pLevel->Load(dwLevelID))
		{
			delete pLevel;
			pLevel=NULL;
		}
	}
	return pLevel;
}

//*****************************************************************************
CDbLevel * CDbLevels::GetFirst(void)
//Gets first level.  A subsequent call to GetNext() will retrieve the second level.
//
//Returns:
//Pointer to loaded level which caller must delete, or NULL if no matching level
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//Set current row to first.
	this->pCurrentRow = this->MembershipIDs.Get(0);
	
	//If there are no rows, then just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load level.
	CDbLevel *pLevel = GetByID(this->pCurrentRow->dwID);
	
	//Advance row to next.
	this->pCurrentRow = this->pCurrentRow->pNext;

	return pLevel;
}

//*****************************************************************************
CDbLevel * CDbLevels::GetNext(void)
//Gets next level.
//
//Returns:
//Pointer to loaded level which caller must delete, or NULL if no matching level
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//If at end of rows, just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load level.
	CDbLevel *pLevel = GetByID(pCurrentRow->dwID);
	
	//Advance row to next.
	pCurrentRow = pCurrentRow->pNext;

	return pLevel;
}

//*****************************************************************************
CDbLevel * CDbLevels::GetNew(void)
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
	ASSERT(this->dwFilterByHoldID);
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

//*****************************************************************************
void CDbLevels::RemoveFromHold(
//Removes references to this level's ID from the DB.
//Specifically, this replaces 'dwLevelID' in all levels' room Exits lists
//that have it to 'newLevelID'.
//NOTE: RemoveFromHold() should generally be called following a call to Delete().
//
//Params:
	const DWORD dwLevelID, const DWORD dwNewLevelID)	//(in)
{
	ASSERT(dwLevelID);
	ASSERT(IsOpen());

	//If this level is the first level in a hold, update hold's first level ID.
	c4_View HoldsView = GetView("Holds");
	const DWORD dwHoldCount = HoldsView.GetSize();
	for (DWORD dwHoldI = 0; dwHoldI < dwHoldCount; ++dwHoldI)
	{
		const DWORD dwFirstLevelID = (DWORD) p_LevelID(HoldsView[dwHoldI]);
		if (dwFirstLevelID == dwLevelID)
		{
			//Update the hold's first level ID.
			p_LevelID( HoldsView[dwHoldI] ) = dwNewLevelID;
		}
	}

	//Update Exits subrecord in any rooms leading to this level.
	c4_View RoomsView = GetView("Rooms");
	const DWORD dwRoomCount = RoomsView.GetSize();
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
	{
		//For a room, if this level's ID was in its Exits list,
		//replace it with dwNewLevelID.
		c4_View ExitsView = p_Exits(RoomsView[dwRoomI]);
		const DWORD dwExitCount = ExitsView.GetSize();
		for (DWORD dwNextLevelI = 0; dwNextLevelI < dwExitCount; dwNextLevelI++)
		{
			const DWORD dwNextLevelID = (DWORD) p_LevelID(ExitsView[dwNextLevelI]);
			if (dwNextLevelID == dwLevelID)
				p_LevelID( ExitsView[dwNextLevelI] ) = dwNewLevelID;
		}
	}
}

//
//CDbLevels private methods.
//

//*****************************************************************************
void CDbLevels::LoadMembership(void)
//Load the membership list with all level IDs.
{
	ASSERT(CDbBase::IsOpen());
	c4_View LevelsView = CDbBase::GetView("Levels");
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
CDbLevel::CDbLevel(void)
//Constructor.
{	
	Clear();
}


//
//CDbLevel public methods.
//

//*****************************************************************************
CDbLevel::~CDbLevel(void)
//Destructor.
{
	Clear();
}

//*****************************************************************************
const WCHAR * CDbLevel::GetAuthorText(void) const
//Returns author of the level or NULL if not found.
{
	//Look up NameMessageID from associated Players record.
	c4_View PlayersView = CDbBase::GetView("Players");
	const DWORD dwPlayersRowI = LookupRowByPrimaryKey(this->dwPlayerID,
			p_PlayerID, PlayersView);
	if (dwPlayersRowI == ROW_NO_MATCH) {ASSERT(false); return NULL;}
	const DWORD dwNameMessageID = p_NameMessageID( PlayersView[dwPlayersRowI] );

	//Look up message text.
	CDb db;
	return db.GetMessageText(dwNameMessageID);
}

//*****************************************************************************
DWORD CDbLevel::GetDefaultNextLevelID(void)
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
	c4_View RoomsView = GetView("Rooms");
	const DWORD dwRoomCount = RoomsView.GetSize();

	//Look for a room with one staircase in it.
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
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
	ASSERT(CDbBase::IsOpen());
	LevelsView = CDbBase::GetView("Levels");
	
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
	this->wRoomsNeededToComplete = p_RoomsNeededToComplete(LevelsView[dwLevelI]);
	this->Created = (time_t) (p_Created(LevelsView[dwLevelI]));
	this->LastUpdated = (time_t) (p_LastUpdated(LevelsView[dwLevelI]));
	this->DescriptionText.Bind((DWORD) p_DescriptionMessageID(LevelsView[dwLevelI]));
	this->NameText.Bind((DWORD) p_NameMessageID(LevelsView[dwLevelI]));
	this->dwLevelIndex = (DWORD) (p_GID_LevelIndex(LevelsView[dwLevelI]));
	
	//Filter Rooms member to only show rooms from this level.
	this->Rooms.FilterBy(this->dwLevelID);

	//Filter SavedGames member to only show saved games from this level.
	this->SavedGames.FilterByLevel(this->dwLevelID);
	
Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
CDbHold * CDbLevel::GetHold(void)
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
CDbRoom * CDbLevel::GetStartingRoom(void)
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
		ASSERT(CDbBase::IsOpen());
		c4_View RoomsView = CDbBase::GetView("Rooms");
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
MESSAGE_ID CDbLevel::SetProp(
//Used during XML data import.							 
//According to pType, convert string to proper datatype and member
//
//Returns: whether operation was successful
//
//Params:
	const PROPTYPE pType,	//(in) property (data member) to set
	char* const str,			//(in) string representation of value
	PrimaryKeyMaps &Maps,	//(in/out) ID map
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
			localID = Maps.LevelIDMap.find(this->dwLevelID);
			if (localID != Maps.LevelIDMap.end())
				//Error - this level should not have been imported already.
				return MID_FileCorrupted;

			//Look up level in the DB.
			const DWORD dwLocalLevelID = GetLocalID();
			if (dwLocalLevelID)
			{
				//Level found in DB.
				Maps.LevelIDMap[this->dwLevelID] = dwLocalLevelID;
				this->dwLevelID = dwLocalLevelID;
				bSaveRecord = false;
			} else {
				//Add a new record to the DB.
				const DWORD dwOldLocalID = this->dwLevelID;
				this->dwLevelID = 0L;
				Update();
				Maps.LevelIDMap[dwOldLocalID] = this->dwLevelID;
			}
			break;
		}
		case P_HoldID:
		{
			this->dwHoldID = static_cast<DWORD>(atol(str));
			if (!this->dwHoldID)
				return MID_FileCorrupted;	//corrupt data (can't load a level w/o its hold)

			//Set to local ID.
			localID = Maps.HoldIDMap.find(this->dwHoldID);
			if (localID == Maps.HoldIDMap.end())
				return MID_HoldNotFound;	//record should have been loaded already
			this->dwHoldID = (*localID).second;
			break;
		}
		case P_PlayerID:
		{
			this->dwPlayerID = static_cast<DWORD>(atol(str));
			if (!this->dwPlayerID)
				return MID_FileCorrupted;	//corrupt file (must have an author)

			//Set to local ID.
			localID = Maps.PlayerIDMap.find(this->dwPlayerID);
			if (localID == Maps.PlayerIDMap.end())
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
		case P_RoomsNeededToComplete:
			this->wRoomsNeededToComplete = static_cast<UINT>(atoi(str));
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
bool CDbLevel::Update(void)
//Updates database with level.
//
//Returns: true if successful, else false.
{
	bool bSuccess=true;

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
DWORD CDbLevel::GetLocalID(void) const
//Compares this object's GID fields against those of the records in the DB.
//ASSUME: dwHoldID has already been set to the local record ID
//
//Returns: local ID if a record in the DB matches this object's GUID, else 0
{
	ASSERT(CDbBase::IsOpen());
	c4_View LevelsView = CDbBase::GetView("Levels");
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
void CDbLevel::InsertIntoHold(
//Inserts level into its hold.
//Update rooms' Exits list as needed.
//If dwLevelSupplantedID was the ID of the hold's first level, the new
//level becomes the hold's first level.
//
//Params:
	const DWORD dwLevelSupplantedID)	//(in) Level this one is placed in front of
{
	ASSERT(this->dwHoldID);
	ASSERT(IsOpen());

	c4_View HoldsView = GetView("Holds");
	const DWORD dwHoldKey = LookupRowByPrimaryKey(this->dwHoldID, p_HoldID, HoldsView);
	if (dwHoldKey == ROW_NO_MATCH) {ASSERT(false); return;} //Bad hold ID.

	//Give level a (GID) value in the hold.
	ASSERT(this->dwLevelIndex == 0);
	this->dwLevelIndex = CDbHolds::GetNewLevelIndex(this->dwHoldID);
	ASSERT(this->dwLevelIndex);

	//If new level supplants hold's first level, update hold's first level ID. 
	const DWORD dwFirstLevelID = (DWORD) p_LevelID( HoldsView[ dwHoldKey ] );
	if (dwLevelSupplantedID == dwFirstLevelID)
		p_LevelID( HoldsView[ dwHoldKey ] ) = this->dwLevelID;

	//Update room Exits lists.
	c4_View RoomsView = GetView("Rooms");
	const DWORD dwRoomCount = RoomsView.GetSize();
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
	{
		const DWORD dwLevelID = (DWORD) p_LevelID(RoomsView[dwRoomI]);
		c4_View ExitsView = p_Exits(RoomsView[dwRoomI]);
		const DWORD dwExitCount = ExitsView.GetSize();

		for (DWORD dwNextLevelI = 0; dwNextLevelI < dwExitCount; dwNextLevelI++)
		{
			if (dwLevelID == this->dwLevelID)
			{
				//Exits in this level now go to the level supplanted.
				p_LevelID( ExitsView[dwNextLevelI] ) = dwLevelSupplantedID;
			} else {
				//Update Exits field in rooms for other levels:
				//if they went to the supplanted level, then now to go to this one.
				const DWORD dwNextLevelID =
						(DWORD) p_LevelID(ExitsView[dwNextLevelI]);
				if (dwNextLevelID == dwLevelSupplantedID)
					p_LevelID( ExitsView[dwNextLevelI] ) = dwLevelID;
			}
		}
	}
}

//*****************************************************************************
bool CDbLevel::UpdateNew(void)
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
	if (this->dwLevelIndex == 0)
		InsertIntoHold(0L);

	//Write out message texts.
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwDescID = this->DescriptionText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwDescID);

	//Write Level record.
	c4_View LevelsView = GetView("Levels");
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
			p_RoomsNeededToComplete[ this->wRoomsNeededToComplete ] +
			p_GID_LevelIndex[ this->dwLevelIndex ]);

	return true;
}

//*****************************************************************************
bool CDbLevel::UpdateExisting(void)
//Update an existing Levels record in database.
{
	ASSERT(this->dwLevelID != 0);
	ASSERT(IsOpen());

	//Lookup Levels record.
	c4_View LevelsView = GetView("Levels");
	const DWORD dwLevelID = LookupRowByPrimaryKey(this->dwLevelID,
			p_LevelID, LevelsView);
	if (dwLevelID == ROW_NO_MATCH)
	{
		ASSERT(false); //The caller probably passed a bad PKID.
		return false;
	}

	if (this->dwLevelIndex == 0)
		InsertIntoHold(0L);

	//Update Levels record.
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
	p_RoomsNeededToComplete( LevelsView[ dwLevelID ] ) = this->wRoomsNeededToComplete;
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

	ASSERT(CDbBase::IsOpen());
	RoomsView = CDbBase::GetView("Rooms");
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
void CDbLevel::Clear(void)
//Frees resources associated with this object and resets member vars.
{	
	this->dwLevelID = this->dwHoldID = this->dwPlayerID = this->dwRoomID = 
			this->dwStartingRoomX = this->dwStartingRoomY = 0L;
	this->wX = this->wY = this->wO = this->wRoomsNeededToComplete = 0;
	this->Created = this->LastUpdated = 0L;
	this->dwLevelIndex = 0L;
	this->bGotStartingRoomCoords = false;

	this->NameText.Clear();
	this->DescriptionText.Clear();
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
	this->wRoomsNeededToComplete = Src.wRoomsNeededToComplete;
	this->bGotStartingRoomCoords = false;	//reset this

	//object members
	this->NameText = Src.NameText;	//make new message texts
	this->DescriptionText = Src.DescriptionText;
	this->Created = Src.Created;
	this->LastUpdated = Src.LastUpdated;
	this->Rooms.FilterBy(this->dwLevelID);
	this->SavedGames.FilterByLevel(this->dwLevelID);

	return true;
}

// $Log: DbLevels.cpp,v $
// Revision 1.1  2003/02/25 00:01:29  erikh2000
// Initial check-in.
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
