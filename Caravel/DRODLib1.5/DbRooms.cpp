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
 * Michael Welsh Duggan (md5i), JP Burford (jpburford), John Wm. Wicks (j_wicks),
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DbRooms.cpp
//Implementation of CDbRooms, CDbRoom, and other classes related to rooms.

#include "DbRooms.h"

#include "Base64.h"
#include "CurrentGame.h"
#include "Db.h"
#include "DbProps.h"
#include "Mimic.h"

//Macros.
//Uniform way of accessing 2D information in 1D array (column-major).
#define ARRAYINDEX(x,y)	(((y) * this->wRoomCols) + (x))

//
//CDbRooms public methods.
//

//*****************************************************************************
void CDbRooms::Delete(
//Deletes records for a room with the given ID.
//Deletes scroll messages, demos and saved games associated with this room.
//
//Params:
	const DWORD dwRoomID)	//(in)	ID of room(s) to delete.
{
	ASSERT(dwRoomID);

	c4_View RoomsView = GetView("Rooms");
	const DWORD dwRoomRowI = LookupRowByPrimaryKey(dwRoomID, p_RoomID, RoomsView);
	if (dwRoomRowI == ROW_NO_MATCH) {ASSERT(false); return;} //Bad room ID.

	//Delete all scroll messages in room.
	c4_View ScrollsView = p_Scrolls(RoomsView[dwRoomRowI]);
	for (UINT wScrollI = ScrollsView.GetSize(); wScrollI--; )
		DeleteMessage(p_MessageID(ScrollsView[wScrollI]));

	CDb db;
	CIDList DemoIDs, SavedGameIDs;
	IDNODE *pSeek;

	//Delete all demos in room (and their associated saved game).
	db.Demos.FilterByRoom(dwRoomID);
	db.Demos.GetIDs(DemoIDs);
	pSeek = DemoIDs.Get(0);
	while (pSeek)
	{
		db.Demos.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}

	//Delete all (remaining) saved games in room.
	db.SavedGames.FilterByRoom(dwRoomID);
	db.SavedGames.GetIDs(SavedGameIDs);
	pSeek = SavedGameIDs.Get(0);
	while (pSeek)
	{
		db.SavedGames.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}

	//Delete the room.
	RoomsView.RemoveAt(dwRoomRowI);

	//After room object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//*****************************************************************************
string CDbRooms::ExportXML(
//Returns: string containing XML text describing room with this ID
//
//Pre-condition: dwRoomID is valid
//
//Params:
	const DWORD dwRoomID,	//(in)
	CDbRefs &dbRefs,			//(in/out)
	const bool bRef)			//(in) Only export GUID reference (default = false)
{
	string str;

	if (!dbRefs.IsSet(V_Rooms,dwRoomID))
	{
		dbRefs.Set(V_Rooms,dwRoomID);

		char dummy[32];
		CDbRoom *pRoom = GetByID(dwRoomID);
		ASSERT(pRoom);

		//Include corresponding level ref.
		CDb db;
		str += db.Levels.ExportXML(pRoom->dwLevelID, dbRefs, true);

		str += "<Rooms LevelID='";
		str += _ltoa(pRoom->dwLevelID, dummy, 10);
		str += "' RoomX='";
		str += _ltoa(pRoom->dwRoomX, dummy, 10);
		str += "' RoomY='";
		str += _ltoa(pRoom->dwRoomY, dummy, 10);
		str += "' RoomID='";
		str += _ltoa(pRoom->dwRoomID, dummy, 10);
		if (bRef)
		{
			//Don't need any further information for a room reference.
			str += "'/>\r\n";
		} else {
			str += "' RoomCols='";
			str += _ltoa(pRoom->wRoomCols, dummy, 10);
			str += "' RoomRows='";
			str += _ltoa(pRoom->wRoomRows, dummy, 10);
			str += "' Style='";
			str += _ltoa(pRoom->wStyle, dummy, 10);
			str += "' Squares='";

			//Process squares data.
			DWORD dwSize;
			{
				c4_Bytes *c4Squares = pRoom->PackSquares();
				const BYTE *pSquares = c4Squares->Contents();
				dwSize = c4Squares->Size();

				str += Base64::encode(pSquares,dwSize);

				delete c4Squares;
			}

			str += "'>\r\n";

			//Process room data lists.
			DWORD dwIndex, dwAgentI, dwNumAgents;

			//Orbs
			COrbData *pOrb;
			COrbAgentData *pOrbAgent;
			dwSize = pRoom->wOrbCount;
			for (dwIndex=0; dwIndex<dwSize; ++dwIndex)
			{
				pOrb = pRoom->parrOrbs + dwIndex;
				str += "<Orbs X='";
				str += _ltoa(pOrb->wX, dummy, 10);
				str += "' Y='";
				str += _ltoa(pOrb->wY, dummy, 10);
				str += "'>\r\n";

				dwNumAgents = pOrb->wAgentCount;
				for (dwAgentI=0; dwAgentI<dwNumAgents; ++dwAgentI)
				{
					pOrbAgent = pOrb->parrAgents + dwAgentI;
					str += "<OrbAgents Type='";
					str += _ltoa(pOrbAgent->wAction, dummy, 10);
					str += "' X='";
					str += _ltoa(pOrbAgent->wX, dummy, 10);
					str += "' Y='";
					str += _ltoa(pOrbAgent->wY, dummy, 10);
					str += "' />\r\n";
				}

				str += "</Orbs>\r\n";
			}

			//Monsters
			CMonster *pMonster;
			pMonster = pRoom->pFirstMonster;
			BYTE *pExtraVars;
			DWORD dwBufferSize;
			while (pMonster)
			{
				str += "<Monsters Type='";
				str += _ltoa(pMonster->wType, dummy, 10);
				str += "' X='";
				str += _ltoa(pMonster->wX, dummy, 10);
				str += "' Y='";
				str += _ltoa(pMonster->wY, dummy, 10);
				str += "' O='";
				str += _ltoa(pMonster->wO, dummy, 10);
				if (pMonster->bIsFirstTurn)
				{
					str += "' IsFirstTurn='";
					str += _ltoa((long)pMonster->bIsFirstTurn, dummy, 10);
				}
				if (pMonster->wProcessSequence != DEFAULT_PROCESS_SEQUENCE)
				{
					str += "' ProcessSequence='";
					str += _ltoa(pMonster->wProcessSequence, dummy, 10);
				}
				pExtraVars = pMonster->ExtraVars.GetPackedBuffer(dwBufferSize);
				if (dwBufferSize > 4)	//null buffer
				{
					str += "' ExtraVars='";
					str += Base64::encode(pExtraVars,dwBufferSize-4);	//strip null UINT
				}
				delete pExtraVars;
				str += "' />\r\n";
				pMonster = pMonster->pNext;
			}

			//Scrolls
			CScrollData *pScroll;
			dwSize = pRoom->wScrollCount;
			for (dwIndex=0; dwIndex<dwSize; ++dwIndex)
			{
				pScroll = pRoom->parrScrolls + dwIndex;
				WSTRING const wMessage = pScroll->ScrollText;

				str += "<Scrolls X='";
				str += _ltoa(pScroll->wX, dummy, 10);
				str += "' Y='";
				str += _ltoa(pScroll->wY, dummy, 10);
				str += "' Message='";
				str += Base64::encode(wMessage);
				str += "' />\r\n";
			}

			//Exits.
			CExitData *pExit;
			CDbLevel *pLevel;
			DWORD dwHoldID;
			dwSize = pRoom->wExitCount;
			if (dwSize)
			{
				pLevel = db.Levels.GetByID(pRoom->dwLevelID);
				ASSERT(pLevel);
				dwHoldID = pLevel->dwHoldID;
				delete pLevel;
			}
			for (dwIndex=0; dwIndex<dwSize; ++dwIndex)
			{
				pExit = pRoom->parrExits + dwIndex;

				//Include corresponding level ref if level is not in this hold.
				pLevel = db.Levels.GetByID(pExit->dwLevelID);
				ASSERT(pLevel);
				if (pLevel->dwHoldID != dwHoldID)
					str += db.Levels.ExportXML(pExit->dwLevelID, dbRefs, true);
				delete pLevel;

				str += "<Exits LevelID='";
				str += _ltoa(pExit->dwLevelID, dummy, 10);
				str += "' Left='";
				str += _ltoa(pExit->wLeft, dummy, 10);
				str += "' Right='";
				str += _ltoa(pExit->wRight, dummy, 10);
				str += "' Top='";
				str += _ltoa(pExit->wTop, dummy, 10);
				str += "' Bottom='";
				str += _ltoa(pExit->wBottom, dummy, 10);
				str += "' />\r\n";
			}

			str += "</Rooms>\r\n";
		}

		delete pRoom;
	}

	return str;
}

//*****************************************************************************
void CDbRooms::GetIDs(
//Gets room IDs in membership.
//
//Params:
	CIDList &IDs)	//(out) Receives copy of object's membership list.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(this->bIsMembershipLoaded);

	IDs = this->MembershipIDs;
}

//*****************************************************************************
CDbRoom * CDbRooms::GetByID(
//Get a room by its RoomID.
//
//Params:
	const DWORD dwRoomID)	//(in)
//
//Returns:
//Pointer to loaded room which caller must delete, or NULL if no matching room 
//was found.
{
	CDbRoom *pRoom = new CDbRoom();
	if (pRoom)
	{
		if (!pRoom->Load(dwRoomID))
		{
			delete pRoom;
			pRoom=NULL;
		}
	}
	return pRoom;
}

//*****************************************************************************
CDbRoom * CDbRooms::GetByCoords(
//Get a room by its coordinates.
//
//Params:
	const DWORD dwLevelID,				//(in)	Level containing room.
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of room to get.
//
//Returns:
//Pointer to loaded room which caller must delete, or NULL if no matching room 
//was found.
{
	const DWORD dwRoomID = FindIDAtCoords(dwLevelID, dwRoomX, dwRoomY);
	if (dwRoomID)
		return GetByID(dwRoomID);
	else
		return NULL;
}

//*****************************************************************************
DWORD CDbRooms::FindIDAtCoords(
//Finds a room at specified coordinates.
//
//Params:
	const DWORD dwLevelID,				//(in)	Level containing room.
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in)	Coords of room to find.
//
//Returns:
//RoomID of matching room, or 0 if not matching room was found.
{
	//Open rooms view.
	ASSERT(CDbBase::IsOpen());
	c4_View RoomsView = CDbBase::GetView("Rooms");
	DWORD dwRoomCount = RoomsView.GetSize();

	//Scan through rooms to find matching room.
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; dwRoomI++)
	{
		if (dwLevelID == (DWORD) p_LevelID(RoomsView[dwRoomI]))
		{
			if (dwRoomY == (DWORD) p_RoomY(RoomsView[dwRoomI]))
			{
				if (dwRoomX == (DWORD) p_RoomX(RoomsView[dwRoomI]))
				{
					//Found it.
					return p_RoomID(RoomsView[dwRoomI]);
				}
			}
		}
	}

	//No match.
	return 0;
}

//*****************************************************************************
CDbRoom * CDbRooms::GetFirst(void)
//Gets first room.  A subsequent call to GetNext() will retrieve the second room.
//
//Returns:
//Pointer to loaded room which caller must delete, or NULL if no matching room
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//Set current row to first.
	this->pCurrentRow = this->MembershipIDs.Get(0);
	
	//If there are no rows, then just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load room.
	CDbRoom *pRoom = GetByID(this->pCurrentRow->dwID);
	
	//Advance row to next.
	this->pCurrentRow = this->pCurrentRow->pNext;

	return pRoom;
}

//*****************************************************************************
CDbRoom * CDbRooms::GetNext(void)
//Gets next room.
//
//Returns:
//Pointer to loaded room which caller must delete, or NULL if no matching room
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//If at end of rows, just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load room.
	CDbRoom *pRoom = GetByID(pCurrentRow->dwID);
	
	//Advance row to next.
	pCurrentRow = pCurrentRow->pNext;

	return pRoom;
}

//*****************************************************************************
CDbRoom * CDbRooms::GetNew(void)
//Get a new room object that will be added to database when it is updated.
//
//Returns:
//Pointer to new room.
{
	//After room object is updated, membership changes, so reset the flag.
	this->bIsMembershipLoaded = false;

	//Return new room object.
	CDbRoom *pRoom = new CDbRoom;
	pRoom->dwLevelID = this->dwFilterByLevelID;	//set to current level
	return pRoom;
}

//*****************************************************************************
void CDbRooms::FilterBy(
//Changes filter so that GetFirst() and GetNext() will return rooms for a
//specified level.
//
//Params:
	const DWORD dwSetFilterByLevelID)	//(in)	Level ID to filter by.
													//Set to 0 for all rooms.
{
	if (dwSetFilterByLevelID != this->dwFilterByLevelID && this->bIsMembershipLoaded)
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByLevelID = dwSetFilterByLevelID;
}

//
//CDbRooms private methods.
//

//*****************************************************************************
void CDbRooms::LoadMembership(void)
//Load the membership list with all room IDs.
{
	ASSERT(CDbBase::IsOpen());
	c4_View RoomsView = CDbBase::GetView("Rooms");
	const DWORD dwRoomCount = RoomsView.GetSize();

	//Each iteration gets a room ID and puts in membership list.
	this->MembershipIDs.Clear();
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
	{
		const DWORD dwLevelID = p_LevelID(RoomsView[dwRoomI]);
		if (this->dwFilterByLevelID == 0 || dwLevelID == this->dwFilterByLevelID)
			this->MembershipIDs.Add(p_RoomID(RoomsView[dwRoomI]));
	}
	this->pCurrentRow = this->MembershipIDs.Get(0);
	this->bIsMembershipLoaded = true;
}

//
//CDbRoom protected methods.
//

//*****************************************************************************
CDbRoom::CDbRoom(void)
	//Set pointers to NULL so Clear() won't try to delete them.
	: pszOSquares(NULL), pszTSquares(NULL)
	, parrOrbs(NULL)
	, pFirstMonster(NULL), pLastMonster(NULL)
	, pMonsterSquares(NULL)
	, parrScrolls(NULL)
	, parrExits(NULL)
	, pCurrentGame(NULL)
	, pImportMonster(NULL), pImportOrb(NULL), pImportOrbAgent(NULL)
	, pImportScroll(NULL), pImportExit(NULL)
//Constructor.
{
	for (int n=0; n<NumMovementTypes; n++)
		this->pPathMap[n]=NULL;

	Clear();
}

//*****************************************************************************
void CDbRoom::ClearPlotHistory(void)
//Resets flag stating that a plot(s) were made to the room.
{
	this->bPlotsMade = false;
}

//
//CDbRoom public methods.
//

//*****************************************************************************
CDbRoom::~CDbRoom(void)
//Destructor.
{
	Clear();
}

//*****************************************************************************
CDbLevel * CDbRoom::GetLevel(void)
//Get level associated with this room.
//
//Returns:
//Pointer to loaded level which caller must delete, or NULL if no matching level 
//was found.
const
{
	CDbLevel *pLevel = new CDbLevel();
	if (pLevel)
	{
		if (!pLevel->Load(this->dwLevelID))
		{
			delete pLevel;
			pLevel=NULL;
		}
	}
	return pLevel;
}

//*****************************************************************************
bool CDbRoom::Load(
//Loads a room from database into this object.
//
//Params:
	const DWORD dwLoadRoomID)	//(in) RoomID of room to load.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;
	c4_View RoomsView;
	c4_View OrbsView;
	c4_View MonstersView;
	c4_View ScrollsView;
	c4_View ExitsView;
	c4_Bytes SquaresBytes;
	DWORD dwSquareCount, dwRoomI, dwRoomCount;

	Clear();

	//Open rooms view.
	ASSERT(CDbBase::IsOpen());
	RoomsView = CDbBase::GetView("Rooms");
	dwRoomCount = RoomsView.GetSize();

	//Find record with matching room ID.
	dwRoomI = LookupRowByPrimaryKey(dwLoadRoomID, p_RoomID, RoomsView);
	if (dwRoomI == ROW_NO_MATCH) {bSuccess=false; goto Cleanup;}

	//Load in props from Rooms record.
	this->dwRoomID = (DWORD) (p_RoomID(RoomsView[dwRoomI]));
	this->dwLevelID = (DWORD) (p_LevelID(RoomsView[dwRoomI]));
	this->dwRoomX = (DWORD) (p_RoomX(RoomsView[dwRoomI]));
	this->dwRoomY = (DWORD) (p_RoomY(RoomsView[dwRoomI]));
	this->wRoomCols = (UINT) (p_RoomCols(RoomsView[dwRoomI]));
	this->wRoomRows = (UINT) (p_RoomRows(RoomsView[dwRoomI]));
	this->wStyle = (UINT) (p_Style(RoomsView[dwRoomI]));
	
	//Load squares for this room.
	dwSquareCount = this->wRoomCols * this->wRoomRows;
	this->pszOSquares = new char[dwSquareCount + 1];
	if (!this->pszOSquares) {bSuccess=false; goto Cleanup;}
	this->pszTSquares = new char[dwSquareCount + 1];
	if (!this->pszTSquares) {bSuccess=false; goto Cleanup;}
	this->pMonsterSquares = new CMonster*[dwSquareCount];
	if (!this->pMonsterSquares) {bSuccess=false; goto Cleanup;}
	SquaresBytes = p_Squares(RoomsView[dwRoomI]);
	if (!UnpackSquares(SquaresBytes.Contents(), SquaresBytes.Size(), 
			dwSquareCount, this->pszOSquares, this->pszTSquares))
		{bSuccess=false; goto Cleanup;}

	//Load orbs for this room.
	OrbsView = p_Orbs(RoomsView[dwRoomI]);
	if (!LoadOrbs(OrbsView)) {bSuccess=false; goto Cleanup;}
	
	//Load monsters for this room
	MonstersView = p_Monsters(RoomsView[dwRoomI]);
	if (!LoadMonsters(MonstersView)) {bSuccess=false; goto Cleanup;}

	//Load scrolls for this room
	ScrollsView = p_Scrolls(RoomsView[dwRoomI]);
	if (!LoadScrolls(ScrollsView)) {bSuccess=false; goto Cleanup;}

	//Load exits for this room.
	ExitsView = p_Exits(RoomsView[dwRoomI]);
	if (!LoadExits(ExitsView)) {bSuccess=false; goto Cleanup;}

	//Filter demos to show demos for the current room only.
	this->Demos.FilterByRoom(this->dwRoomID);

	//Filter saved games to show saved games for the current room only.
	this->SavedGames.FilterByRoom(this->dwRoomID);

Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
MESSAGE_ID CDbRoom::SetProp(
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
		case P_RoomID:
		{
			this->dwRoomID = static_cast<DWORD>(atol(str));
			if (!this->dwRoomID)
				return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = Maps.RoomIDMap.find(this->dwRoomID);
			if (localID != Maps.RoomIDMap.end())
				//Error - this room should not have been imported yet
				return MID_FileCorrupted;

			//Look up room in the DB.
			const DWORD dwLocalRoomID = GetLocalID();
			if (dwLocalRoomID)
			{
				//Room found in DB.
				Maps.RoomIDMap[this->dwRoomID] = dwLocalRoomID;
				this->dwRoomID = dwLocalRoomID;
				bSaveRecord = false;
			} else {
				//Add a new record to the DB.
				const DWORD dwOldLocalID = this->dwRoomID;
				this->dwRoomID = 0L;
				Update();
				Maps.RoomIDMap[dwOldLocalID] = this->dwRoomID;
			}
			break;
		}
		case P_LevelID:
			this->dwLevelID = static_cast<DWORD>(atol(str));
			if (!this->dwLevelID)
				return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = Maps.LevelIDMap.find(this->dwLevelID);
			if (localID == Maps.LevelIDMap.end())
				return MID_LevelNotFound;	//can't load a room w/o its level
			this->dwLevelID = (*localID).second;
			break;
		case P_RoomX:
			this->dwRoomX = static_cast<DWORD>(atol(str));
			break;
		case P_RoomY:
			this->dwRoomY = static_cast<DWORD>(atol(str));
			break;
		case P_RoomCols:
			this->wRoomCols = static_cast<UINT>(atoi(str));
			break;
		case P_RoomRows:
			this->wRoomRows = static_cast<UINT>(atoi(str));
			break;
		case P_Style:
			this->wStyle = static_cast<UINT>(atoi(str));
			break;
		case P_Squares:
		{
			//Must be read in following P_RoomCols and P_RoomRows.
			const DWORD dwSquareCount = this->wRoomCols * this->wRoomRows;

			//Allocate memory.
			this->pszOSquares = new char[dwSquareCount + 1];
			if (!this->pszOSquares)	return false;
			this->pszTSquares = new char[dwSquareCount + 1];
			if (!this->pszOSquares)	return false;
			this->pMonsterSquares = new CMonster*[dwSquareCount];
			if (!this->pMonsterSquares) return false;
			memset(this->pMonsterSquares,NULL,dwSquareCount	* sizeof(CMonster*));

			const string sstr = str;
			BYTE *data;
			const DWORD size = Base64::decode(sstr,data);
			UnpackSquares((const BYTE*)data, size, dwSquareCount,
					this->pszOSquares, this->pszTSquares);
			delete data;
			break;
		}
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
MESSAGE_ID CDbRoom::SetProp(
//Used during XML data import.							 
//According to vpType, convert string to proper datatype and member
//
//Returns: whether operation was successful
//
//Params:
	const VIEWPROPTYPE vpType,	//(in) sub-view to modify
	const PROPTYPE pType,	//(in) property (data member) in sub-view to set
	char* const str,			//(in) string representation of value
	PrimaryKeyMaps &Maps)	//(in/out) ID map
{
	switch (vpType)
	{
		case VP_OrbAgents:
			ASSERT(this->pImportOrb);
			if (pType != P_Start)
				ASSERT(this->pImportOrbAgent);
			switch (pType)
			{
				case P_Start:
					ASSERT(!this->pImportOrbAgent);
					this->pImportOrbAgent = new COrbAgentData;
					break;
				case P_Type:
					this->pImportOrbAgent->wAction = static_cast<UINT>(atoi(str));
					break;
				case P_X:
					this->pImportOrbAgent->wX = static_cast<UINT>(atoi(str));
					if (this->pImportOrbAgent->wX >= this->wRoomCols)
						return MID_FileCorrupted;
					break;
				case P_Y:
					this->pImportOrbAgent->wY = static_cast<UINT>(atoi(str));
					if (this->pImportOrbAgent->wY >= this->wRoomRows)
						return MID_FileCorrupted;
					break;
				case P_End:
					//Finish processing
					this->pImportOrb->AddAgent(this->pImportOrbAgent);
					delete this->pImportOrbAgent;
					this->pImportOrbAgent = NULL;
					break;
				default:
					return MID_FileCorrupted;
			}
			break;
		case VP_Orbs:
			if (pType != P_Start)
				ASSERT(!this->pImportOrbAgent);
			switch (pType)
			{
				case P_Start:
					ASSERT(!this->pImportOrb);
					this->pImportOrb = new COrbData;
					break;
				case P_X:
					this->pImportOrb->wX = static_cast<UINT>(atoi(str));
					if (this->pImportOrb->wX >= this->wRoomCols)
						return MID_FileCorrupted;
					break;
				case P_Y:
					this->pImportOrb->wY = static_cast<UINT>(atoi(str));
					if (this->pImportOrb->wY >= this->wRoomRows)
						return MID_FileCorrupted;
					break;
				case P_End:
					//Finish processing
					AddOrb(this->pImportOrb);
					delete this->pImportOrb;
					this->pImportOrb = NULL;
					break;
				default:
					return MID_FileCorrupted;
			}
			break;
		case VP_Monsters:
			if (pType != P_Start)
				ASSERT(this->pImportMonster);
			switch (pType)
			{
				case P_Start:
				{
					CMonsterFactory mf;
					ASSERT(!this->pImportMonster);
					this->pImportMonster = mf.GetNewMonster(M_MIMIC);
					break;
				}
				case P_Type:
					this->pImportMonster->wType = static_cast<UINT>(atoi(str));
					break;
				case P_X:
					this->pImportMonster->wX = static_cast<UINT>(atoi(str));
					if (this->pImportMonster->wX >= this->wRoomCols)
						return MID_FileCorrupted;
					break;
				case P_Y:
					this->pImportMonster->wY = static_cast<UINT>(atoi(str));
					if (this->pImportMonster->wY >= this->wRoomRows)
						return MID_FileCorrupted;
					break;
				case P_O:
					this->pImportMonster->wO = static_cast<UINT>(atoi(str));
					if (this->pImportMonster->wO >= ORIENTATION_COUNT)
						return MID_FileCorrupted;
					break;
				case P_IsFirstTurn:
					this->pImportMonster->bIsFirstTurn = (static_cast<UINT>(atoi(str)) != 0);
					break;
				case P_ProcessSequence:
					this->pImportMonster->wProcessSequence = static_cast<UINT>(atoi(str));
					break;
				case P_ExtraVars:
				{
					const string sstr = str;
					BYTE *data;
					Base64::decode(sstr,data);
					this->pImportMonster->ExtraVars = (const BYTE*)data;
					delete data;
					break;
				}
				case P_End:
					//Finish processing
					LinkMonster(this->pImportMonster);
					this->pImportMonster = NULL;
					break;
				default:
					return MID_FileCorrupted;
			}
			break;
		case VP_Scrolls:
			if (pType != P_Start)
				ASSERT(this->pImportScroll);
			switch (pType)
			{
				case P_Start:
					ASSERT(!this->pImportScroll);
					this->pImportScroll = new CScrollData;
					break;
				case P_X:
					this->pImportScroll->wX = static_cast<UINT>(atoi(str));
					if (this->pImportScroll->wX >= this->wRoomCols)
						return MID_FileCorrupted;
					break;
				case P_Y:
					this->pImportScroll->wY = static_cast<UINT>(atoi(str));
					if (this->pImportScroll->wY >= this->wRoomRows)
						return MID_FileCorrupted;
					break;
				case P_Message:
				{
					WSTRING data;
					Base64::decode(str,data);
					this->pImportScroll->ScrollText = data.c_str();
					break;
				}
				case P_End:
					//Finish processing
					AddScroll(this->pImportScroll);
					delete this->pImportScroll;
					this->pImportScroll = NULL;
					break;
				default:
					return MID_FileCorrupted;
			}
			break;
		case VP_Exits:
			if (pType != P_Start)
				ASSERT(this->pImportExit);
			switch (pType)
			{
				case P_Start:
					ASSERT(!this->pImportExit);
					this->pImportExit = new CExitData;
					break;
				case P_ExitLevelID:
					//IDs will be matched to local ones on later completion of import
					this->pImportExit->dwLevelID = static_cast<DWORD>(atol(str));
					break;
				case P_Left:
					this->pImportExit->wLeft = static_cast<UINT>(atoi(str));
					if (this->pImportExit->wLeft >= this->wRoomCols)
						return MID_FileCorrupted;
					break;
				case P_Right:
					this->pImportExit->wRight = static_cast<UINT>(atoi(str));
					if (this->pImportExit->wRight >= this->wRoomCols)
						return MID_FileCorrupted;
					break;
				case P_Top:
					this->pImportExit->wTop = static_cast<UINT>(atoi(str));
					if (this->pImportExit->wTop >= this->wRoomRows)
						return MID_FileCorrupted;
					break;
				case P_Bottom:
					this->pImportExit->wBottom = static_cast<UINT>(atoi(str));
					if (this->pImportExit->wBottom >= this->wRoomRows)
						return MID_FileCorrupted;
					break;
				case P_End:
					//Finish processing
					AddExit(this->pImportExit);
					delete this->pImportExit;
					this->pImportExit = NULL;
					break;
				default:
					return MID_FileCorrupted;
			}
			break;
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbRoom::Update(void)
//Updates database with room.
//
//Returns: true if successful, else false.
{
	bool bSuccess=true;

	if (this->dwRoomID == 0)
	{
		//Insert a new room.
		bSuccess = UpdateNew();
	}
	else
	{
		//Update existing room.
		bSuccess = UpdateExisting();
	}

	if (!bSuccess) return false;

	//Filter demos to show demos for the current room only.
	this->Demos.FilterByRoom(this->dwRoomID);

	//Filter saved games to show saved games for the current room only.
	this->SavedGames.FilterByRoom(this->dwRoomID);

	return bSuccess;
}

//*****************************************************************************
void CDbRoom::Reload(void)
//Reloads a currently loaded room.
{
	ASSERT(this->dwRoomID);
	VERIFY(Load(this->dwRoomID));
}

//*****************************************************************************
void CDbRoom::ResetMonsterFirstTurnFlags(void)
//Sets first turn flag to false for all monsters.  When the first turn flag is
//set to true, the monster is not processed in CCurrentGame::ProcessMonsters().
{
	for (CMonster *pSeek = this->pFirstMonster; pSeek != NULL; pSeek=pSeek->pNext)
		pSeek->bIsFirstTurn = false;
}

//*****************************************************************************
bool CDbRoom::IsValidColRow(const UINT wX, const UINT wY) const
//Our square coords valid for this room dimensions.
{
	return (wX < wRoomCols && wY < wRoomRows);
}

//*****************************************************************************
bool CDbRoom::DoesSquareContainSwordsmanObstacle(
//Does a square contain an obstacle to swordsman movement?
//
//Params:
		const UINT wX, const UINT wY,		//(in)	Destination square to check.
		const UINT wO)				//(in)	Direction of movement onto squre.
//
//Returns:
//True if it does, false if not.
const
{
	ASSERT(IsValidColRow(wX, wY));

	//Look for t-square obstacle.
	UINT wTileNo = GetTSquare(wX, wY);
	if (bIsArrowObstacle(wTileNo, wO)) return true;
	if ( !(wTileNo == T_EMPTY || wTileNo==T_SCROLL || wTileNo==T_POTION_I ||
		wTileNo==T_POTION_K || bIsArrow(wTileNo)) ) return true;

	//Look for o-square obstacle.
	wTileNo = GetOSquare(wX, wY);
	if ( !(wTileNo == T_FLOOR || wTileNo == T_CHECKPOINT ||
			wTileNo == T_DOOR_YO || wTileNo==T_TRAPDOOR || wTileNo == T_STAIRS) )
		return true;

	//Is there a monster in the square?
	if (GetMonsterAtSquare(wX, wY) != NULL) return true;

	//Is there a mimic sword in the square?
	if (DoesSquareContainMimicSword(wX, wY)) return true;

	//No obstacle.
	return false;
}

//*****************************************************************************
bool CDbRoom::DoesSquareContainMimicPlacementObstacle(
//Does a square contain an obstacle to mimic placement?
//
//Params:
	const UINT wX, const UINT wY)		//(in)	Destination square to check.
//
//Returns:
//True if it does, false if not.
const
{
	ASSERT(IsValidColRow(wX, wY));

	//Look for t-square obstacle.
	UINT wTileNo = GetTSquare(wX, wY);
	if ( wTileNo != T_EMPTY ) return true;

	//Look for o-square obstacle.
	wTileNo = GetOSquare(wX, wY);
	if ( wTileNo != T_FLOOR && wTileNo != T_CHECKPOINT ) return true;

	//Is there a monster in the square?
	if (GetMonsterAtSquare(wX, wY) != NULL) return true;

	//Is the swordsman in the square?
	if (this->pCurrentGame->swordsman.wX == wX &&
			this->pCurrentGame->swordsman.wY == wY) return true;

	//Is the swordsman's sword in the square?
	if (this->pCurrentGame->swordsman.wSwordX == wX &&
			this->pCurrentGame->swordsman.wSwordY == wY)
		return true;

	//Is a mimic sword in the square?
	if (DoesSquareContainMimicSword(wX, wY)) return true;

	//No obstacle.
	return false;
}

//*****************************************************************************
void CDbRoom::KillMonster(
//Kills a monster.
//
//Params:
	CMonster *pMonster,		//(in)	Monster already in list of monsters for this room.
	CCueEvents &CueEvents)	//(out)	Adds cue events as appropriate.
{
	ASSERT(pMonster);

	//Remove monster from the array and list and fix up links.
	this->pMonsterSquares[ARRAYINDEX(pMonster->wX,pMonster->wY)] = NULL;
	if (pMonster->pPrevious) pMonster->pPrevious->pNext = pMonster->pNext;
	if (pMonster->pNext) pMonster->pNext->pPrevious = pMonster->pPrevious;
	if (pMonster == this->pLastMonster) this->pLastMonster = pMonster->pPrevious;
	if (pMonster == this->pFirstMonster) this->pFirstMonster = pMonster->pNext;
	
	if (pMonster->wType != M_MIMIC)
	{
		DecMonsterCount(CueEvents);

		//Decrement brain count.
		if (pMonster->wType == M_BRAIN)
		{
			ASSERT(this->wBrainCount != 0); //Count should never decrement past 0.
			--wBrainCount;
			if (wBrainCount == 0)
				DeletePathMaps();
		}

		//Remove left-over long monster pieces.
		if (pMonster->wType == M_SERPENT)
			RemoveLongMonsterPieces(pMonster);
	}

	//Put monster in dead monster list.
	//The pointer will be valid until this CDbRoom is destroyed.
	this->DeadMonsters.push_back(pMonster);
}

//*****************************************************************************
bool CDbRoom::KillMonsterAtSquare(
//Kills a monster in a specified square.
//Supports monsters occupying multiple squares.
//
//Params:
	const UINT wX, const UINT wY,	//(in) Indicates square containing monster to kill.
	CCueEvents &CueEvents)	//(out) Adds cue events as appropriate.
//
//Returns:
//True if a monster was found in square, false if not.
{
	CMonster *pMonster = GetMonsterAtSquare(wX, wY);
	if (!pMonster) return false;
	
	KillMonster(pMonster, CueEvents);

	ASSERT(GetMonsterAtSquare(wX, wY)==NULL); //Data was saved with multiple monsters in one square.

	return true;
}

//*****************************************************************************
bool CDbRoom::MonsterHeadIsAt(
//Returns: whether a monster's head is on a square.
//	That is, if a monster occupies multiple squares, then its head is here.
//	If the monster covers only one square, should always return true.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Position
const
{
	CMonster *pMonster = GetMonsterAtSquare(wX,wY);
	if (!pMonster)
		return false;	//no monster there
	return (pMonster->wX == wX && pMonster->wY == wY);
}

//*****************************************************************************
void CDbRoom::DecMonsterCount(
//Decrement room monster count.
//
//Params:
	CCueEvents &CueEvents)	//(out)	Adds cue events as appropriate.
{
	ASSERT(this->wMonsterCount != 0); //Count should never decrement past 0.
	--this->wMonsterCount;

	//Did room just get cleared?
	if (!this->wMonsterCount)
		if (this->pCurrentGame)	//editor
			if (!(this->pCurrentGame->IsCurrentRoomConquered()))	//and was it not already cleared?
				CueEvents.Add(CID_AllMonstersKilled);
}

//*****************************************************************************
void CDbRoom::MoveMonster(
//Moves a monster in the monster array.
//
//Params:
	CMonster* const pMonster,	//(in) Monster moving.
	const UINT wDestX, const UINT wDestY)	//(in)	Destination to move to.
{
	ASSERT(this->pMonsterSquares[ARRAYINDEX(pMonster->wX,pMonster->wY)]==pMonster);
	ASSERT(!this->pMonsterSquares[ARRAYINDEX(wDestX,wDestY)]);
	std::swap(
		this->pMonsterSquares[ARRAYINDEX(pMonster->wX,pMonster->wY)],
		this->pMonsterSquares[ARRAYINDEX(wDestX,wDestY)]);
}

//*****************************************************************************
CMonster * CDbRoom::AddNewMonster(
//Creates a new monster and returns a pointer to it.
//Also sets its room position and adds to monster list and array.
//
//Params:
	const UINT wMonsterType,		//(in)	One of M_* constants indicating
							//		type of monster to create.
	const UINT wX, const UINT wY)	//(in) position of monster
//
//Returns:
//Pointer to new monster object.
{
	//Set up a new monster.
	const MONSTERTYPE eMonsterType = (const MONSTERTYPE)wMonsterType;
	CMonsterFactory mf;
	CMonster *pNew = mf.GetNewMonster(eMonsterType);
	
	//Set monster position.
	pNew->wX = wX;
	pNew->wY = wY;

	//Update room stats.
	if (eMonsterType != M_MIMIC)
	{
		++this->wMonsterCount;
		if (eMonsterType == M_BRAIN)
			++this->wBrainCount;
	}

	LinkMonster(pNew);

	//Return pointer to the new monster.
	return pNew;
}

//*****************************************************************************
void CDbRoom::LinkMonster(
//Sets monster's room position and adds to monster list and array.
//
//Params:
	CMonster *pMonster)	//(in) Monster to add
{
	//Find location in list to put monster at.  List is sorted by process sequence.
	CMonster *pSeek = this->pFirstMonster, *pLastPrecedingMonster=NULL;
	while (pSeek)
	{
		if (pSeek->wProcessSequence > pMonster->wProcessSequence) break;
		pLastPrecedingMonster = pSeek;
		pSeek = pSeek->pNext;
	}
	
	//Add monster to list.
	if (pLastPrecedingMonster) //New monster goes at middle or end of list.
	{
		pMonster->pNext = pLastPrecedingMonster->pNext;
		pMonster->pPrevious = pLastPrecedingMonster;
		pLastPrecedingMonster->pNext = pMonster;
		if (pMonster->pNext) //Adding at middle of list.
		{
			pMonster->pNext->pPrevious = pMonster;
		}
		else //Adding at end of list.
		{
			this->pLastMonster = pMonster;
		}
	}
	else	//New monster goes at beginning of list.
	{
		pMonster->pPrevious = NULL;
		if (this->pFirstMonster) //The list has nodes.
		{
			pMonster->pNext = this->pFirstMonster;
			this->pFirstMonster->pPrevious = pMonster;
			this->pFirstMonster = pMonster;
		}
		else	//Empty list.
		{
			this->pFirstMonster = this->pLastMonster = pMonster;
			pMonster->pNext = NULL;
		}
	}

	//Set monster array pointer.
	ASSERT(!this->pMonsterSquares[ARRAYINDEX(pMonster->wX,pMonster->wY)]);
	this->pMonsterSquares[ARRAYINDEX(pMonster->wX,pMonster->wY)] = pMonster;
}

//*****************************************************************************
CMonster * CDbRoom::GetMonsterAtSquare(
//Gets a monster located within the room at specified coordinates.
//Supports monsters covering multiple squares.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Coords to look in.
//
//Returns:
//Pointer to monster at (wX,wY) or NULL if none.
const
{
	//Quickly look up pointer to a monster in a specified square.
	ASSERT(this->pMonsterSquares);
	ASSERT(IsValidColRow(wX,wY));
	return this->pMonsterSquares[ARRAYINDEX(wX,wY)];
}

//*****************************************************************************
bool CDbRoom::AreMonstersInRect(
//Determines if any monsters are in a rectangular area of the room.
//Considers any part of a monster covering multiple squares.
//
//Params:
	const UINT wLeft, const UINT wTop,		//(in)	Rect to find monsters in.
	const UINT wRight, const UINT wBottom)	//		 Boundaries are inclusive.
//
//Returns:
//True if there is one or more in the rect, false otherwise.
const
{
	ASSERT(wLeft <= wRight && wTop <= wBottom);

	//Decide which search method is faster.
	if (this->wMonsterCount < (wRight-wLeft+1)*(wBottom-wTop+1))
	{
		//Probably quicker to traverse list.
		CMonster *pSeek = this->pFirstMonster;

		while (pSeek)
		{
			if (pSeek->wX >= wLeft && pSeek->wX <= wRight &&
					pSeek->wY >= wTop && pSeek->wY <= wBottom)
				return true;
			pSeek = pSeek->pNext;
		}
	} else {
		//Probably quicker to check squares.
		CMonster **pMonsters;

		ASSERT(wRight < this->wRoomCols && wBottom < this->wRoomRows);
		for (int y=wTop; y<=wBottom; y++)
		{
			pMonsters = &(this->pMonsterSquares[ARRAYINDEX(wLeft,y)]);
			for (int x=wLeft; x<=wRight; x++)
			{
				if (*pMonsters)
					return true;
				++pMonsters;
			}
		}
	}
	
	//No monsters in rect.
	return false;
}

//*****************************************************************************
bool CDbRoom::IsMonsterWithin(
//Returns: Whether a monster is within 'wSquares' of (wX,wY)?
//
//Params:
	const UINT wX, const UINT wY,		//(in)	Square to check from.
	const UINT wSquares)	//(in)	Radius to check around center square (inclusive).
const
{
	//bounds checking
	const UINT wLeft = (wX > wSquares ? wX - wSquares : 0);
	const UINT wTop = (wY > wSquares ? wY - wSquares : 0);
	const UINT wRight = (wX < this->wRoomCols-wSquares ?
			wX + wSquares : this->wRoomCols-1);
	const UINT wBottom = (wY < this->wRoomRows-wSquares ?
			wY + wSquares : this->wRoomRows-1);

	return AreMonstersInRect(wLeft,wTop,wRight,wBottom);
}

//*****************************************************************************
bool CDbRoom::IsSwordWithinRect(
//Returns: Whether a sword is within this rectanglular region
//
//Params:
	const UINT wMinX, const UINT wMinY, const UINT wMaxX, const UINT wMaxY)
const
{
	//Check mimic swords.
	UINT wSX, wSY;
	CMonster *pSeek = this->pFirstMonster;
	while (pSeek)
	{
		if (pSeek->wType == M_MIMIC)
		{
			CMimic *pMimic = reinterpret_cast<CMimic *>(pSeek);
			wSX = pMimic->GetSwordX();
			wSY = pMimic->GetSwordY();
			if (wMinX <= wSX && wSX <= wMaxX && wMinY <= wSY && wSY <= wMaxY)
				return true;
		}
		pSeek = pSeek->pNext;
	}

	//Check player's sword.
	wSX = this->pCurrentGame->swordsman.wSwordX;
	wSY = this->pCurrentGame->swordsman.wSwordY;
	return (wMinX <= wSX && wSX <= wMaxX && wMinY <= wSY && wSY <= wMaxY);
}

//*****************************************************************************
bool CDbRoom::SomeMonsterCanSmellSwordsman(void) const
//Returns: whether a monster can smell the player.
{
	return IsMonsterWithin(this->pCurrentGame->swordsman.wX,
			this->pCurrentGame->swordsman.wY, DEFAULT_SMELL_RANGE);
}

//*****************************************************************************
COrbData * CDbRoom::GetOrbAtCoords(
//Gets an orb located within room at specified coords.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Square to look for orb at.
//
//Returns:
//Pointer to found orb or NULL if no match.
const
{
	for (UINT wOrbI=0; wOrbI < this->wOrbCount; wOrbI++)
	{
		if (this->parrOrbs[wOrbI].wX == wX &&
				this->parrOrbs[wOrbI].wY == wY)
			return &(this->parrOrbs[wOrbI]);
	}
	return NULL;
}

//*****************************************************************************
const WCHAR *	CDbRoom::GetScrollTextAtSquare(
//Gets scroll text of a scroll located within room at specified coords.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Square to look for scroll at.
//
//Returns:
//Pointer to text of scroll or NULL if no match.
const
{
	for (UINT wScrollI=0; wScrollI < this->wScrollCount; wScrollI++)
	{
		if (this->parrScrolls[wScrollI].wX == wX &&
				this->parrScrolls[wScrollI].wY == wY)
			return (const WCHAR*) this->parrScrolls[wScrollI].ScrollText;
	}
	return NULL;
}

//*****************************************************************************
UINT CDbRoom::GetOSquare(
//Get tile# for a square on the opaque layer.
//
//Params:
	const UINT wX, const UINT wY) //(in) 
const
{
	ASSERT(wX < this->wRoomCols);
	ASSERT(wY < this->wRoomRows);

	return (UINT) (unsigned char) (this->pszOSquares[ARRAYINDEX(wX,wY)]);
}

//*****************************************************************************
UINT CDbRoom::GetTSquare(
//Get tile# for a square on the transparent layer.
//
//Params:
	const UINT wX, const UINT wY) //(in) 
const
{
	ASSERT(wX < this->wRoomCols);
	ASSERT(wY < this->wRoomRows);

	return (UINT) (unsigned char) (this->pszTSquares[ARRAYINDEX(wX,wY)]);
}

//*****************************************************************************
void CDbRoom::SetCurrentGame(
//Sets the current game pointer for anything associated with this room.
//
//Params:
	const CCurrentGame *pSetCurrentGame)
{
	ASSERT(pSetCurrentGame);
	this->pCurrentGame = pSetCurrentGame;
	SetCurrentGameForMonsters(pSetCurrentGame);
}

//*****************************************************************************
DWORD CDbRoom::GetExitLevelIDAt(
//Returns: the levelID for the exit at (wX,wY), else 0L if none.
//
//Params:
	const UINT wX, const UINT wY)
const
{
	for (UINT i=0; i<this->wExitCount; i++)
	{
		const CExitData &stairs = this->parrExits[i];
		if (wX >= stairs.wLeft && wX <= stairs.wRight &&
				wY >= stairs.wTop && wY <= stairs.wBottom) 
		{
			return stairs.dwLevelID;
		}
	}

	return 0L;
}

//*****************************************************************************
void CDbRoom::SetExit(
//Sets the exit at (wX,wY) to dwLevelID.
//ASSUME: there's only one exit at that coordinate.
//
//Params:
	const UINT wX, const UINT wY, const DWORD dwLevelID)
{
	ASSERT(GetOSquare(wX,wY) == T_STAIRS);

	for (UINT i=0; i<this->wExitCount; i++)
	{
		CExitData &stairs = this->parrExits[i];
		if (wX >= stairs.wLeft && wX <= stairs.wRight &&
				wY >= stairs.wTop && wY <= stairs.wBottom) 
		{
			//Modify existing exit's value.
			stairs.dwLevelID = dwLevelID;
			break;
		}
	}

	//Add new exit.
	//Find the extent of the staircase, starting from (wX,wY).
	//ASSUME: the staircase is rectangular.
	UINT wMinX = wX, wMaxX = wX, wMinY = wY, wMaxY = wY;
	while (wMinX > 0 && GetOSquare(wMinX-1,wY) == T_STAIRS)
		--wMinX;
	while (wMaxX < this->wRoomCols-1 && GetOSquare(wMaxX+1,wY) == T_STAIRS)
		++wMaxX;
	while (wMinY > 0 && GetOSquare(wX,wMinY-1) == T_STAIRS)
		--wMinY;
	while (wMaxY < this->wRoomRows-1 && GetOSquare(wX,wMaxY+1) == T_STAIRS)
		++wMaxY;

	CExitData *pNewExit, *pNewExits = new CExitData[this->wExitCount+1];
	for (i=0; i<this->wExitCount; i++)
		pNewExits[i] = this->parrExits[i];	//copy existing ones
	delete this->parrExits;
	this->parrExits = pNewExits;

	pNewExit = pNewExits + this->wExitCount;
	pNewExit->dwLevelID = dwLevelID;
	pNewExit->wLeft = wMinX;
	pNewExit->wRight = wMaxX;
	pNewExit->wTop = wMinY;
	pNewExit->wBottom = wMaxY;
	++this->wExitCount;
}

//*****************************************************************************
void CDbRoom::GetLevelPositionDescription(
//Gets text description of this room's position within level.
//
//Params:
	WSTRING &wstrDescription)	//(in/out)	Accepts empty or non-empty value.
								//			Returns with description appended.
const
{
	//Call language-specific version of method.
	GetLevelPositionDescription_English(wstrDescription);
}

//
//CDbRoom private methods.
//

//*****************************************************************************
CMonster* CDbRoom::FindLongMonster(
//From the current square, searches along the length of a long monster
//until a pointer to the monster object is found.  (Recursive call.)
//
//Returns: pointer to the monster, if it was found, else NULL
//
//Params:
	const UINT wX, const UINT wY,	//(in) Search from this position.
	const UINT wFromDirection)	//(in) Where last search move was from
										//(default = 10 (an invalid orientation) at start
const
{
	if (!IsValidColRow(wX,wY))
		return NULL;

	CMonster *pMonster = GetMonsterAtSquare(wX,wY);
	if (pMonster)
		return pMonster;	//found the head

	const UINT tile = GetTSquare(wX, wY);
	ASSERT(bIsSerpent(tile));	//only long monster
	switch (tile)
	{
		//Check in one direction.
		case T_SNK_EW:
		case T_SNK_NW:
		case T_SNK_SW:
		case T_SNKT_W:
			if (wFromDirection != E) pMonster = FindLongMonster(wX+1,wY,W);
			break;

		case T_SNK_NS:
		case T_SNK_SE:
		case T_SNKT_S:
			if (wFromDirection != N) pMonster = FindLongMonster(wX,wY-1,S);
			break;

		case T_SNK_NE:
		case T_SNKT_N:
			if (wFromDirection != S) pMonster = FindLongMonster(wX,wY+1,N);
			break;

		case T_SNKT_E:
			if (wFromDirection != W) pMonster = FindLongMonster(wX-1,wY,E);
			break;
		default: ASSERT(false); return NULL;
	}

	if (!pMonster)
		switch (tile)
		{
			//Check in other direction.
			case T_SNK_EW:
			case T_SNK_SE:
			case T_SNK_NE:
				if (wFromDirection != W) pMonster = FindLongMonster(wX-1,wY,E);
				break;

			case T_SNK_NW:
			case T_SNK_NS:
				if (wFromDirection != S) pMonster = FindLongMonster(wX,wY+1,N);
				break;

			case T_SNK_SW:
				if (wFromDirection != N) pMonster = FindLongMonster(wX,wY-1,S);
				break;
		}

	//Return monster, or NULL if a pointer to it wasn't found along this way.
	return pMonster;
}

//*****************************************************************************
DWORD CDbRoom::GetLocalID(void) const
//Compares this object's GID fields against those of the records in the DB.
//ASSUME: dwLevelID has already been set to the local record ID
//
//Returns: local ID if a record in the DB matches this object's GUID, else 0
{
	ASSERT(CDbBase::IsOpen());
	c4_View RoomsView = CDbBase::GetView("Rooms");
	const DWORD dwRoomCount = RoomsView.GetSize();

	//Each iteration checks a room's GIDs.
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
	{
		//Check level.
		const DWORD dwLevelID = (DWORD)p_LevelID(RoomsView[dwRoomI]);
		if (this->dwLevelID == dwLevelID)
		{
			//Check room coords.
			const DWORD dwRoomX = (DWORD)p_RoomX(RoomsView[dwRoomI]);
			const DWORD dwRoomY = (DWORD)p_RoomY(RoomsView[dwRoomI]);
			if (this->dwRoomX == dwRoomX && this->dwRoomY == dwRoomY)
			{
				//GUIDs match.  Return this record's local ID.
				return (DWORD) p_RoomID(RoomsView[dwRoomI]);
			}
		}
	}

	//No match.
	return 0L;
}

//*****************************************************************************
void CDbRoom::GetNumber_English(
//Writes a number in English.  Should only be called by
//GetLevelPositionDescription_English().
//
//Params:
	const DWORD num,	//(in)	decimal number
	WCHAR *str)			//(out)	English description of number
const
{
	const WCHAR Once[] =	{'O','n','c','e',0},
			Twice[] =		{'T','w','i','c','e',0},
			Thrice[] =		{'T','h','r','i','c','e',0},
			Four[] =		{'F','o','u','r',0},
			Five[] =		{'F','i','v','e',0},
			Six[] =			{'S','i','x',0},
			Seven[] =		{'S','e','v','e','n',0},
			Eight[] =		{'E','i','g','h','t',0},
			Nine[] =		{'N','i','n','e',0},
			Ten[] =			{'T','e','n',0},
			Eleven[] =		{'E','l','e','v','e','n',0},
			Twelve[] =		{'T','w','e','l','v','e',0},
			Thirteen[] =	{'T','h','i','r','t','e','e','n',0},
			Fourteen[] =	{'F','o','u','r','t','e','e','n',0},
			Fifteen[] =		{'F','i','f','t','e','e','n',0},
			Sixteen[] =		{'S','i','x','t','e','e','n',0},
			Seventeen[] =	{'S','e','v','e','n','t','e','e','n',0},
			Eighteen[] =	{'E','i','g','h','t','e','e','n',0},
			Nineteen[] =	{'N','i','n','e','t','e','e','n',0};

	const WCHAR *numberText[19] = {Once, Twice, Thrice, Four, Five, Six, Seven, Eight, Nine,
			Ten, Eleven, Twelve, Thirteen, Fourteen, Fifteen, Sixteen, Seventeen, Eighteen, Nineteen};
	const WCHAR wszTimes[] = {' ','T','i','m','e','s',0};

	ASSERT(num);

	if (num < 20)
		wcscpy(str,numberText[num-1]);
	else
		_itow(num,str,10);
	if (num > 3)
		wcscat(str,wszTimes);
}

//*****************************************************************************
void CDbRoom::GetLevelPositionDescription_English(
//Gets English text description of player's room position within level.
//Should only be called by GetLevelPositionDescription().
//
//Params:
	WSTRING &wstrDescription)	//(in/out)	Accepts empty or non-empty value.
								//			Returns with description appended.
const
{
	WCHAR temp[20]; //Hold "once", "twice", "seventeen times" etc.
	DWORD dwRoomX, dwRoomY;	//level starting room coords
	
	//calculate how far from entrance
	{
		CDbLevel *pLevel = GetLevel();
		pLevel->GetStartingRoomCoords(dwRoomX, dwRoomY);
		delete pLevel;
	}
	const int dX = this->dwRoomX - dwRoomX;	//offset from starting room
	const int dY = this->dwRoomY - dwRoomY;

	if (dX == 0 && dY == 0)
	{
		const WCHAR wszTheEntrance[] = {'T','h','e',' ','E','n','t','r','a','n','c','e',0};
		wstrDescription += wszTheEntrance;
	} else {
		if (dY)
		{
			const WCHAR wszNorth[] = {' ','N','o','r','t','h',0};
			const WCHAR wszSouth[] = {' ','S','o','u','t','h',0};
			const WCHAR wszComma[] = {',',' ',0};

			GetNumber_English(abs(dY),temp);
			wstrDescription += temp;
			if (dY > 0) wstrDescription += wszSouth;
				else wstrDescription += wszNorth;
			if (dX) wstrDescription += wszComma;
		}
		if (dX)
		{
			const WCHAR wszEast[] = {' ','E','a','s','t',0};
			const WCHAR wszWest[] = {' ','W','e','s','t',0};

			GetNumber_English(abs(dX),temp);
			wstrDescription += temp;
			if (dX > 0) wstrDescription += wszEast;
				else wstrDescription += wszWest;
		}
	}
}

//*****************************************************************************
void CDbRoom::DeletePathMaps(void)
//Deletes all existing PathMaps.
{
	for (int n=0; n<NumMovementTypes; n++)
	{
		delete pPathMap[n];
		pPathMap[n] = NULL;
	}
}

//*****************************************************************************
bool CDbRoom::DoesSquareContainPathMapObstacle(
//Does a square contain an obstacle for the pathmap.  The CPathMap class can
//use different obstacle rules; this routine just defines the obstacle rules
//for the CDbRoom's pathmap member.
//
//Params:
	const UINT wX, const UINT wY,	//(in)	Square to evaluate.
	const MovementType eMovement)	//(in)  Type of movement ability to consider
//
//Returns:
//True if it does for the given movement ability type, false if not.
const
{
	//O-square obstacle?
	switch (GetOSquare(wX, wY))
	{
		case T_FLOOR:
		case T_DOOR_YO:
		case T_CHECKPOINT:
		case T_TRAPDOOR:
			break;	//check below for what's on the square
		case T_PIT:
			if (eMovement != AIR)
				return true;	//pits are obstacles to ground-movers
			break;
		default:
			return true;	//all others are automatically obstacles
	}

	//T-square obstacle?
	if (GetTSquare(wX, wY) != T_EMPTY) return true;

	//A mimic?
	CMonster *pMonster = GetMonsterAtSquare(wX, wY);
	if (pMonster && pMonster->wType == M_MIMIC) return true;

	return false;
}

//*****************************************************************************
void CDbRoom::OpenYellowDoor(
//Opens a yellow door.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Coords of any square of the door to open.
{
	if (GetOSquare(wX, wY)==T_DOOR_Y) 
		FloodPlot(wX, wY, T_DOOR_YO);
}

//*****************************************************************************
void CDbRoom::CloseYellowDoor(
//Closes a yellow door.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Coords of any square of the door to close.
{
	if (GetOSquare(wX, wY)==T_DOOR_YO) 
		FloodPlot(wX, wY, T_DOOR_Y);
}

//*****************************************************************************
void CDbRoom::ToggleYellowDoor(
//Toggles a yellow door either open or shut.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Coords of any square of the door to toggle.
{
	const UINT wTileNo = GetOSquare(wX, wY);
	if (wTileNo == T_DOOR_YO) 
		FloodPlot(wX, wY, T_DOOR_Y);
	else if (wTileNo == T_DOOR_Y)
		FloodPlot(wX, wY, T_DOOR_YO);
}

//*****************************************************************************
void CDbRoom::Clear(void)
//Frees resources associated with this object and resets member vars.
{
	ClearPlotHistory();

	this->dwRoomID = this->dwLevelID = this->dwRoomX = this->dwRoomY = 0;
	this->wRoomCols = this->wRoomRows = this->wStyle = this->wOrbCount = 
	this->wMonsterCount = this->wScrollCount = this->wBrainCount =
	this->wExitCount=0;
	this->wTrapDoorsLeft=0;

	delete [] this->pszOSquares;
	this->pszOSquares = NULL;

	delete [] this->pszTSquares;
	this->pszTSquares = NULL;

	delete [] this->pMonsterSquares;
	this->pMonsterSquares = NULL;

	delete [] this->parrOrbs;
	this->parrOrbs = NULL;

	delete [] this->parrScrolls;
	this->parrScrolls = NULL;

	delete [] this->parrExits;
	this->parrExits = NULL;

	DeletePathMaps();

	ClearMonsters();
	ClearDeadMonsters();
}

//*****************************************************************************
bool CDbRoom::UnpackSquares(
//Unpacks squares from database into a format that the game will use.  The data 
//format supports a larger tileset and number of layers than the current DROD
//game engine.  This routine will fail if the data is incompatible with the
//game engine.
//
//Params:
	const BYTE *pSrc,	//(in) Buffer of square data from database.
	const DWORD dwSrcSize,	//(in) Size of buffer.
	const DWORD dwSquareCount,//(in) Number of squares in room.
	char *pszOSQuares,	//(in/out) Accepts buffer allocated to room dwSquareCount + 1.
						//Returns buffer filled with opaque-layer squares.
	char *pszTSQuares)	//(in/out) Same as pszOSquares but buffer is filled with
						//transparent-layer squares.
//
//Returns:
//True if successful, false if not.
const
{
	const BYTE *pRead = pSrc, *pStopReading = pRead + dwSrcSize;
	char *pWriteO = pszOSquares, *pWriteT = pszTSquares;
	char *pStopOWriting = pWriteO + dwSquareCount;
	UINT *pwTrapDoorCount=(UINT * )&wTrapDoorsLeft;

	UINT wLayerCount, wOpaqueTileNo, wTransparentTileNo;
	while (pRead < pStopReading)
	{
		//Don't write past end of buffer.
		if (pWriteO >= pStopOWriting) return false;

		//Get number of layers stored in this square.
		wLayerCount = *(pRead++);
		if (pRead >= pStopReading) return false;	//There should be square data next.
		if (wLayerCount < 1 || wLayerCount > 2) return false; //DROD only supports 2 layers right now.

		//Write opaque square.
		wOpaqueTileNo = *((USHORT *) pRead);
		if (wOpaqueTileNo > 255) return false; //DROD only supports 256 tiles right now.

		//Gather special room stats.
		if (wOpaqueTileNo == T_TRAPDOOR) (*pwTrapDoorCount)++;

		*(pWriteO++) = (char) wOpaqueTileNo;
		pRead += 2;

		//Write transparent square.
		if (wLayerCount == 1)
		{
			wTransparentTileNo = T_EMPTY;
		}
		else
		{
			wTransparentTileNo  = *((USHORT *) pRead);
			if (wTransparentTileNo  > 255) return false; //DROD only supports 256 tiles right now.
			pRead += 2;
		}
		*(pWriteT++) = (char) wTransparentTileNo;
	}

	//Source buffer should contain data for exactly the number of squares in 
	//the room.
	if (pWriteO != pStopOWriting) return false;

	//Add terminating zeros.
	*pWriteO = '\0';
	*pWriteT = '\0';

	return true;
}

//*****************************************************************************
bool CDbRoom::LoadOrbs(
//Loads orbs from database into member vars of object.
//
//Params:
	c4_View &OrbsView)		//(in) Open view containing 0 or more orbs.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = true;

	ASSERT(this->parrOrbs == NULL);

	this->wOrbCount = OrbsView.GetSize();
	UINT wOrbI, wOrbAgentI;
	if (this->wOrbCount > 0)
	{
		this->parrOrbs = new COrbData[this->wOrbCount];
		if (!this->parrOrbs) {bSuccess=false; goto Cleanup;}
		for (wOrbI=0; wOrbI < this->wOrbCount; wOrbI++)
		{
			c4_View OrbAgentsView = p_OrbAgents(OrbsView[wOrbI]);
			this->parrOrbs[wOrbI].wX = p_X(OrbsView[wOrbI]);
			this->parrOrbs[wOrbI].wY = p_Y(OrbsView[wOrbI]);
			this->parrOrbs[wOrbI].wAgentCount = OrbAgentsView.GetSize();
			if (this->parrOrbs[wOrbI].wAgentCount > 0)
			{
				this->parrOrbs[wOrbI].parrAgents = 
						new COrbAgentData[this->parrOrbs[wOrbI].wAgentCount];
				if (!this->parrOrbs[wOrbI].parrAgents) {bSuccess=false; goto Cleanup;}
				for (wOrbAgentI=0; wOrbAgentI < this->parrOrbs[wOrbI].wAgentCount; wOrbAgentI++)
				{
					this->parrOrbs[wOrbI].parrAgents[wOrbAgentI].wAction = 
							p_Type(OrbAgentsView[wOrbAgentI]);
					this->parrOrbs[wOrbI].parrAgents[wOrbAgentI].wX = 
							p_X(OrbAgentsView[wOrbAgentI]);
					this->parrOrbs[wOrbI].parrAgents[wOrbAgentI].wY = 
							p_Y(OrbAgentsView[wOrbAgentI]);
				}
			}
		}
	}

Cleanup:
	if (!bSuccess)
	{
		delete [] this->parrOrbs;
		this->parrOrbs = NULL;
	}
	return bSuccess;
}

//*****************************************************************************
bool CDbRoom::LoadMonsters(
//Loads monsters from database into member vars of object.
//
//Params:
	c4_View &MonstersView)		//(in) Open view containing 0 or more monsters.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;

	memset(this->pMonsterSquares,NULL,this->wRoomRows * this->wRoomCols
			* sizeof(CMonster*));

	UINT wMonsterI, wMonsterCount = MonstersView.GetSize();
	if (wMonsterCount > 0)
	{
		for (wMonsterI = 0; wMonsterI < wMonsterCount; wMonsterI++)
		{
			//Note: To test a monster which is not in any of the available rooms,
			//it is useful to change the line below to replace the monster type
			//with the one you'd like to see.
			const UINT wMonsterType = p_Type(MonstersView[wMonsterI]);

			CMonster *pNew = AddNewMonster((MONSTERTYPE)wMonsterType,
					p_X(MonstersView[wMonsterI]),p_Y(MonstersView[wMonsterI]));
			if (!pNew) {bSuccess=false; goto Cleanup;}
			pNew->bIsFirstTurn = (p_IsFirstTurn(MonstersView[wMonsterI]) == 1);
			pNew->ExtraVars = p_ExtraVars(MonstersView[wMonsterI]);
			pNew->wO = p_O(MonstersView[wMonsterI]);
			pNew->SetMembersFromExtraVars();
		}
	}

	//Link long monster segments to monster object.
	CMonster *pMonster;
	UINT wX, wY;
	for (wY=0; wY<this->wRoomRows; wY++)
		for (wX=0; wX<this->wRoomCols; wX++)
			if (bIsSerpent(GetTSquare(wX,wY)))
			{
				pMonster = FindLongMonster(wX,wY);
				ASSERT(!this->pMonsterSquares[ARRAYINDEX(wX,wY)]);
				this->pMonsterSquares[ARRAYINDEX(wX,wY)] = pMonster;
			}

Cleanup:
	if (!bSuccess) ClearMonsters();
	return bSuccess;
}

//*****************************************************************************
void CDbRoom::ClearMonsters(void)
//Puts room in a monster-free state.
{
	CMonster *pDelete, *pSeek = this->pFirstMonster;
	while (pSeek)
	{
		pDelete = pSeek;
		pSeek = pSeek->pNext;
		delete pDelete;
	}
	this->pFirstMonster = this->pLastMonster = NULL;
	this->wMonsterCount = this->wBrainCount = 0;

	memset(this->pMonsterSquares,NULL,this->wRoomRows * this->wRoomCols
			* sizeof(CMonster*));
}

//*****************************************************************************
void CDbRoom::ClearDeadMonsters(void)
//Frees memory and resets members for dead monster list.
{
	list<CMonster *>::const_iterator iSeek = this->DeadMonsters.begin(), 
			iStop = this->DeadMonsters.end();
	while (iSeek != iStop)
	{
		CMonster *pDelete = *iSeek;
		ASSERT(pDelete);
		delete pDelete;
		++iSeek;
	}
	this->DeadMonsters.clear();
}

//*****************************************************************************
bool CDbRoom::LoadScrolls(
//Loads scrolls from database into member vars of object.
//
//Params:
	c4_View &ScrollsView)		//(in) Open view containing 0 or more scrolls.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;

	UINT wScrollI;
	this->wScrollCount = ScrollsView.GetSize();
	if (this->wScrollCount > 0)
	{
		this->parrScrolls = new CScrollData[this->wScrollCount];
		if (!this->parrScrolls) {bSuccess=false; goto Cleanup;}
		for (wScrollI = 0; wScrollI < this->wScrollCount; wScrollI++)
		{
			this->parrScrolls[wScrollI].wX = p_X(ScrollsView[wScrollI]);
			this->parrScrolls[wScrollI].wY = p_Y(ScrollsView[wScrollI]);
			this->parrScrolls[wScrollI].ScrollText.Bind(
					(DWORD) (p_MessageID(ScrollsView[wScrollI])) );
		}
	}

Cleanup:
	if (!bSuccess)
	{
		delete [] this->parrScrolls;
		this->parrScrolls = NULL;
	}
	return bSuccess;
}

//*****************************************************************************
void CDbRoom::AddOrb(
//Adds orb to room object.
//
//Params:
	COrbData *pOrb)	//(in) orb object
{
	if (GetTSquare(pOrb->wX,pOrb->wY) != T_ORB)
		return;	//no orb actually here

	//Add new orb.  Reallocate orb array.
	COrbData *pNewOrbs = new COrbData[this->wOrbCount+1];
	COrbData *pNewOrb = pNewOrbs + this->wOrbCount;
	for (UINT wOrbI = 0; wOrbI < this->wOrbCount; wOrbI++)
		pNewOrbs[wOrbI] = this->parrOrbs[wOrbI];
	pNewOrbs[this->wOrbCount] = *pOrb;
	++this->wOrbCount;

	delete[] this->parrOrbs;
	this->parrOrbs = pNewOrbs;
}

//*****************************************************************************
COrbData* CDbRoom::AddOrbToSquare(
//Adds orb at coords in room object.
//
//Returns: pointer to new orb data
//
//Params:
	const UINT wX, const UINT wY)	//(in) Square with orb
{
	ASSERT(GetTSquare(wX,wY) == T_ORB);

	//Add new orb.  Reallocate orb array.
	COrbData *pNewOrbs = new COrbData[this->wOrbCount+1];
	COrbData *pNewOrb = pNewOrbs + this->wOrbCount;
	for (UINT wOrbI = 0; wOrbI < this->wOrbCount; wOrbI++)
		pNewOrbs[wOrbI] = this->parrOrbs[wOrbI];
	pNewOrb->wAgentCount = 0;
	pNewOrb->parrAgents = NULL;
	pNewOrb->wX = wX;
	pNewOrb->wY = wY;
	++this->wOrbCount;

	delete[] this->parrOrbs;
	this->parrOrbs = pNewOrbs;

	return pNewOrb;
}

//*****************************************************************************
void CDbRoom::AddScroll(
//Adds scroll to room object.
//
//Params:
	CScrollData *pScroll)	//(in) scroll object
{
	if (GetTSquare(pScroll->wX,pScroll->wY) != T_SCROLL)
		return;	//no scroll actually here

	//Add new text.  Reallocate scroll array.
	CScrollData *pNewScrolls = new CScrollData[this->wScrollCount+1];
	for (UINT wScrollI = 0; wScrollI < this->wScrollCount; wScrollI++)
		pNewScrolls[wScrollI] = this->parrScrolls[wScrollI];
	pNewScrolls[this->wScrollCount] = *pScroll;
	++this->wScrollCount;

	delete[] this->parrScrolls;
	this->parrScrolls = pNewScrolls;
}

//*****************************************************************************
void CDbRoom::AddExit(
//Adds an Exit object to the room
//
//Params:
	CExitData *pExit)
{
	if (GetOSquare(pExit->wLeft,pExit->wTop) != T_STAIRS)
		return;	//no exit actually here

	CExitData *pNewExits = new CExitData[this->wExitCount+1];
	for (UINT i=0; i<this->wExitCount; i++)
		pNewExits[i] = this->parrExits[i];	//copy existing ones
	pNewExits[this->wExitCount] = *pExit;
	++this->wExitCount;

	delete this->parrExits;
	this->parrExits = pNewExits;
}

//*****************************************************************************
void CDbRoom::DeleteOrbAtSquare(
//Removes orb data at coord from room object.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Square with orb
{
	ASSERT(GetTSquare(wX,wY) == T_ORB);

	for (UINT wOrbI=0; wOrbI < this->wOrbCount; wOrbI++)
	{
		if (this->parrOrbs[wOrbI].wX == wX &&
				this->parrOrbs[wOrbI].wY == wY)
		{
			//Found it.
			break;
		}
	}

	//Reorganize orbs array (move last one forward).
	this->parrOrbs[wOrbI] = this->parrOrbs[--this->wOrbCount];
}

//*****************************************************************************
void CDbRoom::DeleteScrollTextAtSquare(
//Removes scroll text at coord from room object.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Square with scroll
{
	ASSERT(GetTSquare(wX,wY) == T_SCROLL);

	//Search for scroll in room data.
	for (UINT wScrollI=0; wScrollI < this->wScrollCount; wScrollI++)
	{
		if (this->parrScrolls[wScrollI].wX == wX &&
				this->parrScrolls[wScrollI].wY == wY)
		{
			//Found it.  Remove scroll text from DB
			this->parrScrolls[wScrollI].ScrollText.Delete();
			break;
		}
	}

	//Reorganize scrolls array (move later ones forward).
	while (wScrollI < this->wScrollCount-1)
	{
		this->parrScrolls[wScrollI] = this->parrScrolls[wScrollI+1];
		++wScrollI;
	}
	--this->wScrollCount;
}

//*****************************************************************************
bool CDbRoom::LoadExits(
//Loads exits from database into member vars of object.
//
//Params:
	c4_View &ExitsView)	//(in) Open view containing 0 or more exits.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;

	UINT wExitI;
	this->wExitCount = ExitsView.GetSize();
	if (this->wExitCount > 0)
	{
		this->parrExits = new CExitData[this->wExitCount];
		if (!this->parrExits) {bSuccess=false; goto Cleanup;}
		for (wExitI = 0; wExitI < this->wExitCount; wExitI++)
		{
			this->parrExits[wExitI].wBottom = 
					p_Bottom(ExitsView[wExitI]);
			this->parrExits[wExitI].wLeft = 
					p_Left(ExitsView[wExitI]);
			this->parrExits[wExitI].wTop = 
					p_Top(ExitsView[wExitI]);
			this->parrExits[wExitI].wRight = 
					p_Right(ExitsView[wExitI]);
			this->parrExits[wExitI].dwLevelID = 
					p_LevelID(ExitsView[wExitI]);
		}
	}

Cleanup:
	if (!bSuccess)
	{
		delete [] this->parrExits;
		this->parrExits = NULL;
	}
	return bSuccess;
}

//*****************************************************************************
void CDbRoom::SetCurrentGameForMonsters(
//Sets the current game pointer for all monsters in the room.  The monsters
//can't be used for current game operations until this is done.
//
//Params:
	const CCurrentGame *pSetCurrentGame)
{
	ASSERT(pSetCurrentGame);

	CMonster *pSeek = this->pFirstMonster;
	while (pSeek)
	{
		pSeek->SetCurrentGame(pSetCurrentGame);
		pSeek = pSeek->pNext;
	}
}

//*****************************************************************************
void CDbRoom::SetScrollTextAtSquare(
//Updates scroll text at coord in room object.
//If there is no scroll text there, add it.
//
//Params:
	const UINT wX, const UINT wY,	//(in) Square with scroll
	WCHAR *pwczScrollText)			//(in) Text to assign
{
	ASSERT(GetTSquare(wX,wY) == T_SCROLL);

	//Search for scroll in room data.
	for (UINT wScrollI=0; wScrollI < this->wScrollCount; wScrollI++)
	{
		if (this->parrScrolls[wScrollI].wX == wX &&
				this->parrScrolls[wScrollI].wY == wY)
		{
			//Update text of existing scroll.
			this->parrScrolls[wScrollI].ScrollText = pwczScrollText;
			return;
		}
	}

	//Add new text.  Reallocate scroll array.
	CScrollData *pNewScrolls = new CScrollData[this->wScrollCount+1];
	CScrollData *pNewScroll = pNewScrolls + this->wScrollCount;
	for (wScrollI = 0; wScrollI < this->wScrollCount; wScrollI++)
		pNewScrolls[wScrollI] = this->parrScrolls[wScrollI];
	pNewScroll->ScrollText = pwczScrollText;
	pNewScroll->wX = wX;
	pNewScroll->wY = wY;
	++this->wScrollCount;

	delete[] this->parrScrolls;
	this->parrScrolls = pNewScrolls;
}

//*****************************************************************************
inline bool CDbRoom::NewTarWouldBeStable(
//Determines whether new tar could be placed at this spot (as opposed to a tar baby)
//according to rule that a minimum of a 2x2 square of tar can exist.
//
//Params:
	tartype *added_tar,	//(in) where tar is located in room
	const UINT tx, const UINT ty)	//(in) square where tar is growing
//
//Returns:
//True if should go here, false if tar baby.
{
	bool tar[3][3] = {{false}};
	UINT x, y;

	//mark where tar is
	for (x=tx-1; x!=tx+2; x++)
		if (x < wRoomCols)
			for (y=ty-1; y!=ty+2; y++)
				if (y < wRoomRows)
					tar[x-tx+1][y-ty+1] = (added_tar[y * wRoomCols + x] != notar);

	return (
		(tar[0][0] && tar[0][1] && tar[1][0]) ||	//upper-left corner
		(tar[0][2] && tar[0][1] && tar[1][2]) ||	//lower-left corner
		(tar[2][0] && tar[2][1] && tar[1][0]) ||	//upper-right corner
		(tar[2][2] && tar[2][1] && tar[1][2]));		//lower-right corner
}

//*****************************************************************************
void CDbRoom::GrowTar(
//Grows the tar and creates tarbabies.
//
//Params:
	CCueEvents &CueEvents)	//(out)	May receive some new cue events.
{
	//This operation requires room to be attached to a current game.
	ASSERT(this->pCurrentGame);

	//Assign to local vars for speed and brevity.
	const UINT wSManX = this->pCurrentGame->swordsman.wX;
	const UINT wSManY = this->pCurrentGame->swordsman.wY;

	//Get coord index with all swords for quick eval.
	CCoordIndex SwordCoords;
	GetSwordCoords(SwordCoords);

	//calculate where tar might grow
	tartype *added_tar = new tartype[wRoomCols * wRoomRows];
	CCoordStack *possible = new CCoordStack();
	UINT x, y, pos;
	for (x = 0; x < wRoomCols; x++)
		for (y = 0; y < wRoomRows; y++) 
		{
			pos = y * wRoomCols + x;	//shorthand
			//Determine where old tar is.
			CMonster *m = GetMonsterAtSquare(x, y);
			if (m != NULL && m->wType == M_TARMOTHER) 
				Plot(x, y, T_TAR);
			if (GetTSquare(x, y) == T_TAR) 
			{
				added_tar[pos] = oldtar;
				continue;
			} else {
				added_tar[pos] = notar;
			}
			//Determine whether new tar can go here.
			const UINT tile = GetOSquare(x, y);
			if ((tile == T_FLOOR || tile == T_DOOR_YO || 
			     tile == T_CHECKPOINT || tile == T_TRAPDOOR) &&
			    GetTSquare(x, y) == T_EMPTY &&
				!(x == wSManX && y == wSManY) && !m)
				for (int o = 0; o < ORIENTATION_COUNT; o++)
				{
					if (o == NO_ORIENTATION) continue;
					const int nx = x + nGetOX(o);
					const int ny = y + nGetOY(o);
					if (IsValidColRow(nx, ny) &&
						GetTSquare(nx, ny) == T_TAR)
					{
						//Tar is adjacent to this square, so tar might grow here.
						added_tar[pos] = newtar;
						possible->Push(x, y);
						break;
					}
				}
		}

	//calculate whether tar or tar babies are placed where tar grows
	bool done = false;
	while (!done)
	{
		done = true;
		CCoordStack *newPossible = new CCoordStack();
		while (possible->Pop(x, y))
		{
			pos = y * wRoomCols + x;	//shorthand
			if (added_tar[pos] == newtar)	//going to grow here
				if (NewTarWouldBeStable(added_tar,x,y))
				{
					//tar might grow here, but wait until all tar babies have spawned, etc.
					newPossible->Push(x, y);
				}
				else if (!SwordCoords.Exists(x,y))
				{
					added_tar[pos] = notar;
					CMonster *m = AddNewMonster(M_TARBABY,x,y);
					CueEvents.Add(CID_TarBabyFormed, m);
					m->SetCurrentGame(this->pCurrentGame);
					m->bIsFirstTurn = false;
					done = false;
				}
				else {	//tar baby can't grow under sword
					added_tar[pos] = notar;
					done = false;
				}
		}
		
		delete possible;
		possible = newPossible;
	} 
	
	while (possible->Pop(x, y)) 
		Plot(x, y, T_TAR);

	delete possible;	
	delete added_tar;
}

//*****************************************************************************
bool CDbRoom::StabTar(
//Update game for results of stabbing a square containing tar.
//
//Params:
	const UINT wX, const UINT wY,	//(in)		Tar square coords.
	CCueEvents &CueEvents,	//(in/out)	If tar is destroyed, a cue event will be added.
	const bool removeTarNow)	//(in) If true, automatically stab tar.
														//		If false, only determine whether effect of stabbing tar would destroy tar or not
//
//Returns:
//True if tar will be removed, else false.
{
	//Operation should only be performed when room is attached to a current game.
	ASSERT(this->pCurrentGame);
	
	if (removeTarNow) {
		//Remove tar (effect of possibly simultaneous hits).
		//In this case, checking for vulnerability of this spot should have been
		//performed previously.
		if (GetTSquare(wX, wY)==T_TAR)
		{
			RemoveStabbedTar(wX, wY, CueEvents);
			CueEvents.Add(CID_TarDestroyed, new CMoveCoord(wX, wY,
					this->pCurrentGame->GetSwordMovement()), true);
			return true;
		}
		return false;
	} else {
		//If the tar is vulnerable, flag to remove it.
		return IsTarVulnerableToStab(wX, wY);
	}
}

//*****************************************************************************
bool CDbRoom::IsTarStableAt(
//Determines whether tar placed at this square would not turn into a tar baby,
//according to the rule that a minimum of a 2x2 square of tar can exist.
//
//Params:
	const UINT wX, const UINT wY)	//(in) square being considered
//
//Returns:
//True if tar would be stable here, false if it should be a tar baby.
const
{
	bool tar[3][3] = {{false}};	//center position is square being considered
	UINT x, y;

	//mark where tar is
	for (x=wX-1; x!=wX+2; x++)
		if (x < wRoomCols)
			for (y=wY-1; y!=wY+2; y++)
				if (y < wRoomRows)
					tar[x-wX+1][y-wY+1] =
							(this->pszTSquares[ARRAYINDEX(x,y)] == T_TAR);

	return (
		(tar[0][0] && tar[0][1] && tar[1][0]) ||	//upper-left corner
		(tar[0][2] && tar[0][1] && tar[1][2]) ||	//lower-left corner
		(tar[2][0] && tar[2][1] && tar[1][0]) ||	//upper-right corner
		(tar[2][2] && tar[2][1] && tar[1][2]));	//lower-right corner
}

//*****************************************************************************
bool CDbRoom::IsTarVulnerableToStab(
//Checks that a square of tar is vulnerable to a sword stab, either from the 
//swordsman or a mimic.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Square to check.
//
//Returns:
//True if square is vulnerable, false if not.
const
{
	ASSERT(GetTSquare(wX, wY)==T_TAR);

	// Tar can be destroyed if it has a flat edge.  Tar has a flat edge if:
	// a) there is a non-tar space orthogonally adjacent to the tar, and 
	// b) the 6 squares on the other side of the empty space are all tar.
	// For example:
	//                        (T = tar)
	//  TTT  TT.  .n.  .TT    (x = stabbed tar)
	//  TxT  Txn  TxT  nxT    (n = non-tar)
	//  .n.  TT.  TTT  .TT    (. = any)

	//
	//Checking condition A.
	//

	//For each orthogonal neighbor of the stabbed square, check for a square
	//which does not contain tar.
	static const int orthogonal[] = {N, E, S, W};
	UINT empty = NO_ORIENTATION;
	for (int i = 0; i < 4; i++)
	{
		const UINT tx = wX + nGetOX(orthogonal[i]);
		const UINT ty = wY + nGetOY(orthogonal[i]);
		if (!IsValidColRow(tx, ty) || GetTSquare(tx, ty) != T_TAR)
		{
			empty = orthogonal[i];
			break;
		}
	}

	//If no non-tar square is orthogonally adjacent to stab, the stab does not
	//destroy tar.
	if (empty == NO_ORIENTATION) return false;
	
	//
	//Checking condition B.
	//

	//For each of the six squares on the other side of the empty space, check
	//that it contains tar.
	const int dx = nGetOX(empty);
	const int dy = nGetOY(empty);
	for (i = 0; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
		{
			const UINT tx = wX + dy * j - dx * i;
			const UINT ty = wY + dx * j - dy * i;
			if (!IsValidColRow(tx, ty))
				return false;
			if (GetTSquare(tx, ty) != T_TAR)
				return false;
		}

	return true;
}

//*****************************************************************************
void CDbRoom::RemoveStabbedTar(
//Removes tar found in a square in response to the tar being stabbed.  Adjacent
//tar squares may change into tar babies as a result.  Does not check that stab
//is directed at a vulnerable tar square.
//
//Params:
	const UINT wX, const UINT wY,	//(in)	Square containing tar to remove.
	CCueEvents &CueEvents)			//(out)	May receive cue events.
{
	//Operation should only be performed when room is attached to a current game.
	ASSERT(this->pCurrentGame);
	
	ASSERT(GetTSquare(wX, wY)==T_TAR);

	//Easy part--remove the tar.
	Plot(wX, wY, T_EMPTY);	
	
	//Tar baby formation rules: If a square contains tar, but does not
	//have three adjacent neighbors at n/nw/w, n/ne/e, s/sw/s, or s/se/e
	//tar is removed from that square (T_TAR -> T_EMPTY) and a tar baby 
	//monster is created in that square.

 recompute:
	//For each of the eight squares adjacent to the stabbed square, check
	//for a tar baby formation in that square.
	for (UINT j = wY - 1; j != wY + 2; j++)
	{
		for (UINT i = wX - 1; i != wX + 2; i++)
		{
			if (!IsValidColRow(i,j) ||
					//Skip over the square that got stabbed or...
					(i == wX && j == wY) ||

					//...a square that doesn't contain tar.
					GetTSquare(i, j) != T_TAR)
				continue;

			//Get the orthogonal squares once for speed.
			const bool bIsNorthTar = (j > 0) ? (GetTSquare(i, j - 1)==T_TAR) : false;
			const bool bIsSouthTar = (j < this->wRoomRows - 1) ? (GetTSquare(i, j + 1)==T_TAR) : false;
			const bool bIsWestTar = (i > 0) ? (GetTSquare(i - 1, j)==T_TAR) : false;
			const bool bIsEastTar = (i < this->wRoomCols - 1) ? (GetTSquare(i + 1, j)==T_TAR) : false;

			//Check the four corners.
			if (
					//Check northwest corner.
					(i > 0 && j > 0 &&
					GetTSquare(i - 1, j - 1)==T_TAR &&
					bIsNorthTar && bIsWestTar) ||

					//Check northeast corner.
					(i < this->wRoomCols - 1 && j > 0 &&
					GetTSquare(i + 1, j - 1)==T_TAR &&
					bIsNorthTar && bIsEastTar) ||

					//Check southwest corner.
					(i > 0 && j < this->wRoomRows - 1 &&
					GetTSquare(i - 1, j + 1)==T_TAR &&
					bIsSouthTar && bIsWestTar) ||

					//Check southeast corner.
					(i < this->wRoomCols - 1 && j < this->wRoomRows - 1 &&
					GetTSquare(i + 1, j + 1)==T_TAR &&
					bIsSouthTar && bIsEastTar)
				)
				//Stable tar--skip to next square.
				continue;
			
			//If there is not a tar mother, spawn a tarbaby.
			if (GetMonsterAtSquare(i, j)==NULL)
			{
				//Tar disappears.
				Plot(i, j, T_EMPTY);

				//Spawn a tarbaby.
				CMonster *m = AddNewMonster(M_TARBABY,i,j);
				CueEvents.Add(CID_TarBabyFormed, m);
				m->SetCurrentGame(this->pCurrentGame);
				m->bIsFirstTurn = false;
				
				// This tar changing into a tarbaby may invalidate a
				// previous decision, so recompute the others.
				goto recompute;
			}
		} //...for(i=...
	} //...for(j=...
}

//*****************************************************************************
void CDbRoom::GetMimicSwordCoords(
//Gets a coord index containing coords of all mimic swords.  
//If swords are out of the room boundaries, don't add them.
//
//For efficiency, use this method when you need to check several squares for a 
//mimic sword.  Use DoesSquareContainMimicSword() when you have only need to
//check one or two.
//
//Params:
	CCoordIndex &MimicSwordCoords)	//(out) Uninitialized.
const
{
	MimicSwordCoords.Init(this->wRoomCols, this->wRoomRows);
	CMonster *pSeek = this->pFirstMonster;
	while (pSeek)
	{
		if (pSeek->wType == M_MIMIC)
		{
			CMimic *pMimic = reinterpret_cast<CMimic *>(pSeek);
			const UINT wSX = pMimic->GetSwordX(), wSY = pMimic->GetSwordY();
			if (IsValidColRow(wSX, wSY))
				MimicSwordCoords.Add(wSX, wSY);
		}
		pSeek=pSeek->pNext;
	}
}

//*****************************************************************************
void CDbRoom::GetSwordCoords(
//Gets a coord index containing coords of all swords (player & mimics).
//If swords are out of the room boundaries, don't add them.
//
//Params:
	CCoordIndex &SwordCoords)	//(out) Uninitialized.
const
{
	GetMimicSwordCoords(SwordCoords);
	if (IsValidColRow(this->pCurrentGame->swordsman.wSwordX,
			this->pCurrentGame->swordsman.wSwordY))
		SwordCoords.Add(this->pCurrentGame->swordsman.wSwordX,
				this->pCurrentGame->swordsman.wSwordY);
}

//*****************************************************************************
bool CDbRoom::DoesSquareContainMimicSword(
//Determines if a square contains a mimic sword.
//
//For efficiency, use this GetMimicSwordCoords() when you need to check several 
//squares for a mimic sword.  Use this method when you have only need to check 
//one or two.
//
//Params:
	const UINT wX, const UINT wY)	//(in)	Square to check.
//
//Returns:
//True if it does, false if not.
const
{
	CMonster *pSeek = this->pFirstMonster;
	while (pSeek)
	{
		if (pSeek->wType == M_MIMIC)
		{
			CMimic *pMimic = reinterpret_cast<CMimic *>(pSeek);
			if (pMimic->GetSwordX()==wX && pMimic->GetSwordY()==wY) return true;
		}
		pSeek=pSeek->pNext;
	}
	return false;
}

//*****************************************************************************
void CDbRoom::CreatePathMap(
//Creates a PathMap for the current room.  If a PathMap has already
//been created, it will reset the PathMap.
//
//Params:
	const UINT wX, const UINT wY, // (in) Position of target (swordsman)
	const MovementType eMovement)	// (in) Type of movement path reflects
{
	POINT p = {wX, wY};
	if (!this->pPathMap[eMovement])
		this->pPathMap[eMovement] = new CPathMap(this->wRoomCols, this->wRoomRows, p);
	else
		this->pPathMap[eMovement]->SetTarget(p);
	for (UINT x = 0; x < this->wRoomCols; x++)
		for (UINT y = 0; y < this->wRoomRows; y++)
		{
			const bool obstacle = DoesSquareContainPathMapObstacle(x, y, eMovement);
			this->pPathMap[eMovement]->SetSquare(x, y, obstacle);
		}
}

//*****************************************************************************
bool bIsArrowObstacle(
//Determines whether a specified arrow tile would be an obstacle for the
//swordsman approaching it from a specified angle or whether the swordsman
//could leave the tile if he were currently standing on it.
//
//Accepts:
  const int nArrowTile, 
  const int nO) //Orientation of swordsman approach.
//
//Returns:
//true/false.
{
	switch (nArrowTile)
   {
	case T_ARROW_N:
		return (nO==S || nO==SW || nO==SE);
	case T_ARROW_S:
		return (nO==N || nO==NW || nO==NE);
	case T_ARROW_W:
		return (nO==E || nO==SE || nO==NE);
	case T_ARROW_E:
		return (nO==W || nO==SW || nO==NW);
	case T_ARROW_NW:
		return (nO==S || nO==E || nO==SE);
	case T_ARROW_SW:
		return (nO==N || nO==E || nO==NE);
	case T_ARROW_NE:
		return (nO==S || nO==W || nO==SW);
	case T_ARROW_SE:
		return (nO==N || nO==W || nO==NW);
	default:
		return false;
	}
}

//*****************************************************************************
void CDbRoom::ActivateOrb(
//Activates an orb by releasing any associated agents into the current room
//to perform tasks.
//
//Accepts:
	const UINT wX, const UINT wY,		//(in) Orb location.
	CCueEvents &CueEvents,		//(in/out) Appends cue events to list.
	CMimic *pMimic)				//(in) If !NULL, then a mimic hit the orb
										//(default: NULL)
{
	ASSERT(GetTSquare(wX, wY) == T_ORB);	//No orb at location.

	//Get the orb and its agent instructions.
	COrbData *pOrb = GetOrbAtCoords(wX, wY);

	//Add cue event even if there's no info for this orb in the DB.
	if (pMimic)
		CueEvents.Add(CID_OrbActivatedByMimic, pOrb);
	else 
		CueEvents.Add(CID_OrbActivated, pOrb);

	if (!pOrb) return;  //No orb information -- nothing to do.

	//For each agent in orb...
	COrbAgentData *pSeek = pOrb->parrAgents, *pStop = pSeek + pOrb->wAgentCount;
	while (pSeek != pStop)
	{
		switch(pSeek->wAction)
		{
			case OA_TOGGLE:
				ToggleYellowDoor(pSeek->wX, pSeek->wY);
			break;

			case OA_OPEN:
				OpenYellowDoor(pSeek->wX, pSeek->wY);
			break;

			case OA_CLOSE:
				CloseYellowDoor(pSeek->wX, pSeek->wY);
			break;
			
			default:
				ASSERT(false);
			break;
		}
		++pSeek;
	}
}

//*****************************************************************************
void CDbRoom::FloodPlot(
//Flood fills all squares from a starting square with a new tile.  Only adjacent
//squares from the starting square with the same tile will be flooded.
//
//Params:
	const UINT wX, const UINT wY,	//(in)	Starting square coords.
	const UINT wTileNo)		//(in)	Tile to flood with.
{
	ASSERT(IsValidColRow(wX, wY));
	ASSERT(IsValidTileNo(wTileNo));

	//Layer of new tile determines if flood will occur on t- or o-layer.
	const bool bUseO = (TILE_LAYER[wTileNo] == 0);

	//Get original tile.
	UINT wOriginalTileNo = bUseO ? GetOSquare(wX, wY) : GetTSquare(wX, wY);
	
	//This will contain coords to evaluate.
	CCoordStack EvalCoords;
	EvalCoords.Push(wX, wY);

	//Each iteration evaluates one pair of coordinates for plotting,
	//and possibly add adjacent coords.
	//Exits when there are no more coords in stack to evaluate.
	UINT wEvalX, wEvalY;
	while (EvalCoords.Pop(wEvalX, wEvalY))
	{
		ASSERT(IsValidColRow(wEvalX, wEvalY));

		//Is tile here same as original tile?
		const UINT wEvalTileNo = bUseO ? GetOSquare(wEvalX, wEvalY) : 
				GetTSquare(wEvalX, wEvalY);
		if (wEvalTileNo == wOriginalTileNo) //Yes.
		{
			//Plot new tile.
			Plot(wEvalX, wEvalY, wTileNo);
			
			//Add adjacent coords to eval stack.
			if (wEvalX != 0) 
			{
				EvalCoords.Push(wEvalX - 1, wEvalY);
				if (wEvalY != 0) EvalCoords.Push(wEvalX - 1, wEvalY - 1);
				if (wEvalY != wRoomRows - 1) EvalCoords.Push(wEvalX - 1, wEvalY + 1);
			}
			if (wEvalX != wRoomCols - 1)
			{
				EvalCoords.Push(wEvalX + 1, wEvalY);
				if (wEvalY != 0) EvalCoords.Push(wEvalX + 1, wEvalY - 1);
				if (wEvalY != wRoomRows - 1) EvalCoords.Push(wEvalX + 1, wEvalY + 1);
			}
			if (wEvalY != 0)
				EvalCoords.Push(wEvalX, wEvalY - 1);
			if (wEvalY != wRoomRows - 1)
				EvalCoords.Push(wEvalX, wEvalY + 1);
		}
	}
}

//******************************************************************************
void CDbRoom::DestroyTrapdoor(
//Plots a pit to a trapdoor square.
//If there is a scroll there also, remove it.
//Updates red doors as needed.
//
//Params:
	const UINT wX, const UINT wY,		//(in)
	CCueEvents &CueEvents)		//(in/out)
{
	ASSERT(GetOSquare(wX,wY)==T_TRAPDOOR);
	Plot(wX, wY, T_PIT);
	//Remove scroll on trapdoor, if exists.
	if (GetTSquare(wX,wY)==T_SCROLL)
		Plot(wX, wY, T_EMPTY);
	CueEvents.Add(CID_TrapDoorRemoved, new CCoord(wX, wY), true); //Add CueEvent to handle effect

	ASSERT(this->wTrapDoorsLeft);	//This function should never be called with 0 Trap Doors
	this->wTrapDoorsLeft--;
	if (this->wTrapDoorsLeft == 0) 
	{
		if (RemoveRedDoors())
			CueEvents.Add(CID_RedDoorsOpened);
	}
}

//*****************************************************************************
void CDbRoom::DestroyCrumblyWall(
//Plots a floor to a crumbly wall square.
//
//Params:
	const UINT wX, const UINT wY,	const UINT wO,	//(in) Crumbly wall square and direction hit from.
	CCueEvents &CueEvents)			//(in/out)
{
	ASSERT(GetOSquare(wX,wY)==T_WALL_B);
	Plot(wX, wY, T_FLOOR);
	CueEvents.Add(CID_CrumblyWallDestroyed, new CMoveCoord(wX, wY,
			this->pCurrentGame->GetSwordMovement()), true); //Add CueEvent to handle effect
}

//*****************************************************************************
bool CDbRoom::RemoveBlueDoors(void)
//Removes all blue doors in the room.
//
//Returns:
//True if any blue doors were found, false if not.
{
	return (ChangeTiles(T_DOOR_C, T_FLOOR));
}

//*****************************************************************************
bool CDbRoom::RemoveGreenDoors(void)
//Removes all green doors in the room.
//
//Returns:
//True if any green doors were found, false if not.
{
	return (ChangeTiles(T_DOOR_M, T_FLOOR));
}

//*****************************************************************************
bool CDbRoom::RemoveRedDoors(void)
//Removes all red doors in the room.
//
//Returns:
//True if any red doors were found, false if not.
{
	return (ChangeTiles(T_DOOR_R, T_FLOOR));
}

//*****************************************************************************
bool CDbRoom::RemoveLongMonsterPieces(
//Removes all the pieces of a long monster after the head.
//
//Returns: whether monster was successfully removed
//
//Params:
	CMonster *pMonster)	//(in) Long monster
{
	ASSERT(pMonster->wType == M_SERPENT);	//only long monster

	UINT wX = pMonster->wX, wY = pMonster->wY;
	int dx = -(int)nGetOX(pMonster->wO);
	int dy = -(int)nGetOY(pMonster->wO);
	UINT tile;
	int t;
	if (!bIsSerpent(GetTSquare(wX+dx, wY+dy))) return true;
	while (true)
	{
		//Remove each piece along the monster.
		ASSERT ((dx == 0) != (dy == 0));	//no diagonals
		wX += dx;
		wY += dy;
		tile = GetTSquare(wX, wY);
		ASSERT(bIsSerpent(tile));
		Plot(wX, wY, T_EMPTY);
		//Go to next piece.
		switch (tile)
		{
			case T_SNK_EW: case T_SNK_NS: break;
			case T_SNK_NW: case T_SNK_SE: t = dx; dx = -dy; dy = -t; break;
			case T_SNK_NE: case T_SNK_SW: t = dx; dx = dy; dy = t; break;
			case T_SNKT_S: case T_SNKT_W: case T_SNKT_N: case T_SNKT_E:
				//tail tiles -- done
				return true;
			default: ASSERT(false); return false;
		}
	}
}

//*****************************************************************************
bool CDbRoom::RemoveSerpents(void)
//Remove leftover serpent segments in the room.
//
//Returns:
//True if any serpent segments were found, false if not.
{
	bool bChangedTiles=false;
	for (UINT unY=0; unY < wRoomRows; ++unY)
		for (UINT unX=0; unX < wRoomCols; ++unX)
			if (bIsSerpent(GetTSquare(unX, unY)))
			{
				Plot(unX, unY, T_EMPTY);
				bChangedTiles = true;
			}
	return bChangedTiles;
}

//*****************************************************************************
bool CDbRoom::ChangeTiles(const UINT unOldTile, const UINT unNewTile)
//Swaps any occurence of unOldTile in the room with the Tile unNewTile
//
//Retuns:
//True if any tiles were found
{
	bool bChangedTiles=false;
	for (UINT unY=0; unY < wRoomRows; ++unY)
		for (UINT unX=0; unX < wRoomCols; ++unX)
			if (GetOSquare(unX,unY) == unOldTile)
			{
				Plot(unX, unY, unNewTile);
				bChangedTiles=true;
			}
	return bChangedTiles;
}

//*****************************************************************************
void CDbRoom::Plot(
//Plots a tile value to a square in the room.  All game operations that involve
//changing tiles should use Plot() as opposed to directly modifying square
//data.
//
//Params:
	const UINT wX, const UINT wY,	//(in)	Coords for square to plot.
	const UINT wTileNo,		//(in)	New tile.  The tile# will also determine
						//		which layer the tile is plotted to.
	CMonster *pMonster)	//(in) default=NULL
{
	ASSERT(IsValidTileNo(wTileNo));
	ASSERT(IsValidColRow(wX, wY));

	//Consider updates to each type of movement path.
	bool bWasPathMapObstacle[NumMovementTypes];
	for (int eMovement=0; eMovement<NumMovementTypes; eMovement++)
		if (this->pPathMap[eMovement])
			bWasPathMapObstacle[eMovement] =
					DoesSquareContainPathMapObstacle(wX, wY, (MovementType)eMovement);

	const UINT wSquareIndex = ARRAYINDEX(wX,wY);
	bool bLongMonsterWasHere, bLongMonsterNowHere;
	switch(TILE_LAYER[wTileNo])
	{
		case 0: //Opaque layer.
			this->pszOSquares[wSquareIndex] = static_cast<unsigned char>(wTileNo);
		break;

		case 1: //Transparent layer.
			bLongMonsterWasHere = bIsSerpent(this->pszTSquares[wSquareIndex]);
			bLongMonsterNowHere = bIsSerpent(wTileNo);
			if (bLongMonsterWasHere && !bLongMonsterNowHere)
			{
				//Serpent no longer occupies square -- remove link to it.
				ASSERT(this->pMonsterSquares[wSquareIndex]);
				this->pMonsterSquares[wSquareIndex] = NULL;
			}
			if (!bLongMonsterWasHere && bLongMonsterNowHere)
			{
				//Serpent now occupies square -- add link to it.
				ASSERT(!this->pMonsterSquares[wSquareIndex]);
				ASSERT(pMonster);
				this->pMonsterSquares[wSquareIndex] = pMonster;
			}
			this->pszTSquares[wSquareIndex] = static_cast<unsigned char>(wTileNo);
		break;
	}

	for (eMovement=0; eMovement<NumMovementTypes; eMovement++)
		if (this->pPathMap[eMovement])
		{
			const bool bIsPathMapObstacle = DoesSquareContainPathMapObstacle(wX,
					wY, (MovementType)eMovement);
			if (bWasPathMapObstacle[eMovement] != bIsPathMapObstacle)
				this->pPathMap[eMovement]->SetSquare(wX, wY, bIsPathMapObstacle);
		}

	this->bPlotsMade = true;
}

//*****************************************************************************
bool CDbRoom::IsDoorOpen(
//Determines if a door is open or not.
//
//Accepts:
	const int nCol, const int nRow) //Coords to a door.
//
//Returns:
//true if it is, false if it isn't.
{
	ASSERT(IsValidColRow((UINT)nCol, (UINT)nRow));
	const UINT unTile=GetOSquare((UINT)nCol,(UINT)nRow);
	switch (unTile)
	{
		case T_DOOR_YO: return true;
		case T_DOOR_Y: return false;
		default: ASSERT(false);	return false; //checked a tile that wasn't a door
	}
}

//*****************************************************************************
c4_Bytes* CDbRoom::PackSquares() const
//Saves room squares from member vars of object into database.
//
//Returns: pointer to record to be saved into database (must be deleted).
{
	UINT wSquareLayerCount;
	const DWORD dwSquareCount = this->wRoomCols * this->wRoomRows;
	char *pSquares = new char[(dwSquareCount * 5)];	//max possible size required
	char *pWrite = pSquares;

	for (UINT dwSquareI = 0; dwSquareI < dwSquareCount; dwSquareI++)
	{
		//Write number of layers this square will have.
		ASSERT(this->pszOSquares[dwSquareI] != T_EMPTY);
		wSquareLayerCount = (this->pszTSquares[dwSquareI] == T_EMPTY) ? 1 : 2;
		*(pWrite++) = wSquareLayerCount;

		//Write opaque square.
		*(pWrite++) = this->pszOSquares[dwSquareI];
		*(pWrite++) = 0;	//Extra 0 is placeholder for >256 tile#s.
								//DROD only supports 256 tiles right now.

		//Write transparent square if it is not empty.
		if (wSquareLayerCount == 2)
		{
			*(pWrite++) = this->pszTSquares[dwSquareI];
			*(pWrite++) = 0;	//Extra 0 is placeholder for >256 tile#s.
									//DROD only supports 256 tiles right now.
		}
	}

	const DWORD dwSquaresLen = (DWORD) (pWrite - pSquares);
	return new c4_Bytes(pSquares, dwSquaresLen);
}

//*****************************************************************************
void CDbRoom::SaveOrbs(
//Saves orbs from member vars of object into database.
//
//Params:
	c4_View &OrbsView)		//(in) Open view to fill.
const
{
	if (!this->parrOrbs) return;

	UINT wOrbI, wOrbAgentI;
	for (wOrbI=0; wOrbI < this->wOrbCount; wOrbI++)
	{
		//Save orb.  Don't save orbs without any agents.
		if (this->parrOrbs[wOrbI].wAgentCount)
		{
			c4_View OrbAgentsView;
			for (wOrbAgentI=0; wOrbAgentI < this->parrOrbs[wOrbI].wAgentCount; wOrbAgentI++)
			{
				OrbAgentsView.Add(
						p_Type[ this->parrOrbs[wOrbI].parrAgents[wOrbAgentI].wAction ] +
						p_X[ this->parrOrbs[wOrbI].parrAgents[wOrbAgentI].wX ] +
						p_Y[ this->parrOrbs[wOrbI].parrAgents[wOrbAgentI].wY ]);
			}
			OrbsView.Add(
					p_X[ this->parrOrbs[wOrbI].wX ] +
					p_Y[ this->parrOrbs[wOrbI].wY ] +
					p_OrbAgents[ OrbAgentsView ]);
		}
	}
}

//*****************************************************************************
void CDbRoom::SaveMonsters(
//Saves monsters from member vars of object into database.
//
//Params:
	c4_View &MonstersView)		//(in) Open view to fill.
const
{
	CMonster *pSeek = this->pFirstMonster;
	DWORD dwSettingsSize;
	BYTE *pbytSettingsBytes;
	while (pSeek)
	{
		pbytSettingsBytes = pSeek->ExtraVars.GetPackedBuffer(dwSettingsSize);
		ASSERT(pbytSettingsBytes);
		c4_Bytes SettingsBytes(pbytSettingsBytes, dwSettingsSize);

		MonstersView.Add(
			p_Type[ pSeek->wType ] +
			p_X[ pSeek->wX ] +
			p_Y[ pSeek->wY ] +
			p_O[ pSeek->wO ] +
			p_IsFirstTurn[ pSeek->bIsFirstTurn ] +
			p_ProcessSequence[ pSeek->wProcessSequence ] +
			p_ExtraVars[ SettingsBytes ]);

		pSeek = pSeek->pNext;
	}
}

//*****************************************************************************
void CDbRoom::SaveScrolls(
//Saves scrolls from member vars of object into database.
//
//Params:
	c4_View &ScrollsView)		//(in) Open view to fill.
const
{
	for (UINT wScrollI=0; wScrollI < this->wScrollCount; wScrollI++)
	{
		ScrollsView.Add(
				p_X[ this->parrScrolls[wScrollI].wX ] +
				p_Y[ this->parrScrolls[wScrollI].wY ] +
				p_MessageID[ this->parrScrolls[wScrollI].ScrollText.Flush() ]);
	}
}

//*****************************************************************************
void CDbRoom::SaveExits(
//Saves exits from member vars of object into database.
//Don't save any exits with a LevelID of 0 (these exits end the hold).
//
//Params:
	c4_View &ExitsView)		//(in) Open view to fill.
const
{
	for (UINT wExitI=0; wExitI < this->wExitCount; wExitI++)
	{
		if (this->parrExits[wExitI].dwLevelID)
			ExitsView.Add(
					p_LevelID[ this->parrExits[wExitI].dwLevelID ] +
					p_Left[ this->parrExits[wExitI].wLeft ] +
					p_Right[ this->parrExits[wExitI].wRight ] +
					p_Top[ this->parrExits[wExitI].wTop ] +
					p_Bottom[ this->parrExits[wExitI].wBottom ]);
	}
}

//*****************************************************************************
bool CDbRoom::UpdateExisting(void)
//Update an existing Rooms record in database.
{
	ASSERT(this->dwRoomID != 0);
	ASSERT(IsOpen());

	//Lookup Rooms record.
	c4_View RoomsView = GetView("Rooms");
	const DWORD dwRoomID = LookupRowByPrimaryKey(this->dwRoomID,
			p_RoomID, RoomsView);
	if (dwRoomID == ROW_NO_MATCH)
	{
		ASSERT(false); //The caller probably passed a bad PKID.
		return false;
	}

	//Prepare props.
	c4_Bytes *pSquaresBytes = PackSquares();

	c4_View OrbsView;
	SaveOrbs(OrbsView);

	c4_View MonstersView;
	SaveMonsters(MonstersView);

	c4_View ScrollsView;
	SaveScrolls(ScrollsView);

	c4_View ExitsView;
	SaveExits(ExitsView);

	//Update Rooms record.
	p_RoomID( RoomsView[ dwRoomID ] ) = this->dwRoomID;
	p_LevelID( RoomsView[ dwRoomID ] ) = this->dwLevelID;
	p_RoomX( RoomsView[ dwRoomID ] ) = this->dwRoomX;
	p_RoomY( RoomsView[ dwRoomID ] ) = this->dwRoomY;
	p_RoomCols( RoomsView[ dwRoomID ] ) = this->wRoomCols;
	p_RoomRows( RoomsView[ dwRoomID ] ) = this->wRoomRows;
	p_Style( RoomsView[ dwRoomID ] ) = this->wStyle;
	p_Squares( RoomsView[ dwRoomID ] ) = *pSquaresBytes;
	p_Orbs( RoomsView[ dwRoomID ] ) = OrbsView;
	p_Monsters( RoomsView[ dwRoomID ] ) = MonstersView;
	p_Scrolls( RoomsView[ dwRoomID ] ) = ScrollsView;
	p_Exits( RoomsView[ dwRoomID ] ) = ExitsView;

	delete pSquaresBytes;

	return true;
}

//*****************************************************************************
bool CDbRoom::UpdateNew(void)
//Add new Rooms record to database.
//Note: this doesn't add the room to its corresponding level.
{
	ASSERT(this->dwRoomID == 0);
	ASSERT(IsOpen());

	//Prepare props.
	this->dwRoomID = GetIncrementedID(p_RoomID);

	c4_Bytes *pSquaresBytes = PackSquares();

	c4_View OrbsView;
	SaveOrbs(OrbsView);

	c4_View MonstersView;
	SaveMonsters(MonstersView);

	c4_View ScrollsView;
	SaveScrolls(ScrollsView);

	c4_View ExitsView;
	SaveExits(ExitsView);

	//Write Rooms record.
	c4_View RoomsView = GetView("Rooms");
	RoomsView.Add(
			p_RoomID[ this->dwRoomID ] +
			p_LevelID[ this->dwLevelID ] +
			p_RoomX[ this->dwRoomX ] +
			p_RoomY[ this->dwRoomY ] +
			p_RoomCols[ this->wRoomCols ] +
			p_RoomRows[ this->wRoomRows ] +
			p_Style[ this->wStyle ] +
			p_Squares[ *pSquaresBytes ] +
			p_Orbs[ OrbsView ] +
			p_Monsters[ MonstersView ] +
			p_Scrolls[ ScrollsView ] +
			p_Exits[ ExitsView ]);

	delete pSquaresBytes;

	return true;
}

//*****************************************************************************
bool CDbRoom::SetMembers(
//For copy constructor and assignment operator.
//
//Params:
	const CDbRoom &Src)
{
	bool bSuccess = true;
	UINT wIndex;

	//primitive types
	this->dwRoomID = Src.dwRoomID;
	this->dwLevelID = Src.dwLevelID;
	this->dwRoomX = Src.dwRoomX;
	this->dwRoomY = Src.dwRoomY;
	this->wRoomCols = Src.wRoomCols;
	this->wRoomRows = Src.wRoomRows;
	this->wStyle = Src.wStyle;
	this->wOrbCount = Src.wOrbCount;
	this->wMonsterCount = Src.wMonsterCount;
	this->wBrainCount = Src.wBrainCount;
	this->wScrollCount = Src.wScrollCount;
	this->wExitCount = Src.wExitCount;
	this->wTrapDoorsLeft = Src.wTrapDoorsLeft;
	this->bPlotsMade = Src.bPlotsMade;

	//Room squares
	const DWORD dwSquareCount = this->wRoomCols * this->wRoomRows;
	this->pszOSquares = new char[dwSquareCount + 1];
	if (!this->pszOSquares) {bSuccess=false; goto Cleanup;}
	memcpy(this->pszOSquares, Src.pszOSquares, dwSquareCount * sizeof(char));
	this->pszTSquares = new char[dwSquareCount + 1];
	if (!this->pszTSquares) {bSuccess=false; goto Cleanup;}
	memcpy(this->pszTSquares, Src.pszTSquares, dwSquareCount * sizeof(char));

	//Special room data
	this->parrOrbs = (this->wOrbCount ? new COrbData[this->wOrbCount] : NULL);
	for (wIndex=this->wOrbCount; wIndex--; )
		this->parrOrbs[wIndex] = Src.parrOrbs[wIndex];
	this->parrScrolls = (this->wScrollCount ? new CScrollData[this->wScrollCount] : NULL);
	for (wIndex=this->wScrollCount; wIndex--; )
		this->parrScrolls[wIndex] = Src.parrScrolls[wIndex];
	this->parrExits = (this->wExitCount ? new CExitData[this->wExitCount] : NULL);
	for (wIndex=this->wExitCount; wIndex--; )
		this->parrExits[wIndex] = Src.parrExits[wIndex];

	//Monster data
	this->pFirstMonster = this->pLastMonster = NULL;
	this->pMonsterSquares = new CMonster*[dwSquareCount];
	if (!this->pMonsterSquares) {bSuccess=false; goto Cleanup;}
	memset(this->pMonsterSquares, NULL, dwSquareCount * sizeof(CMonster*));
	CMonster *pMonster, *pTrav;
	pTrav = Src.pFirstMonster;
	while (pTrav)
	{
		pMonster = pTrav->Clone();
		LinkMonster(pMonster);
		pTrav = pTrav->pNext;
	}
	//Don't need a copy of DeadMonsters list
	this->DeadMonsters.clear();

	this->pCurrentGame = Src.pCurrentGame;

	//Path maps -- don't copy
	for (wIndex=NumMovementTypes; wIndex--; )
		this->pPathMap[wIndex] = NULL;

Cleanup:
	if (!bSuccess)
		Clear();
	return bSuccess;
}

// $Log: DbRooms.cpp,v $
// Revision 1.1  2003/02/25 00:01:30  erikh2000
// Initial check-in.
//
// Revision 1.75  2003/02/24 17:06:28  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.74  2003/02/17 03:27:01  erikh2000
// Removed L" string literals.
//
// Revision 1.73  2003/02/16 20:29:32  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.72  2003/01/08 00:48:10  mrimer
// Changed monster type to enumeration.
//
// Revision 1.71  2002/12/22 01:59:44  mrimer
// Added XML import/export support.  Added AddOrb(), AddScroll(), AddExit().
// Revised swordsman vars.  Renamed *SpecialExit* to *Exit*.
// Revised OrbActived* cues to fire even if no orb record exists in DB for square.
//
// Revision 1.70  2002/11/23 00:13:29  mrimer
// Fixed some bugs.
//
// Revision 1.69  2002/11/22 22:00:21  mrimer
// Fixed a subtle bug with membership loading.
//
// Revision 1.68  2002/11/22 02:01:55  mrimer
// Revised calls to CPathMap.  Fixed a minor bug.
//
// Revision 1.67  2002/11/15 02:06:07  mrimer
// Implemented Update() and Delete() methods.
// Added SetMembers(), FindLongMonster(), RemoveLongMonsterPieces(), IsSwordWithinRect(), adding and deleting orbs, scrolls, and special exits.
// Revised Plot() to handle large monsters in monster array.  Added MonsterHeadIsAt().
// Modified ActivateOrb() to receive whether a mimic activated it to produce a new cue event.
// Refactored AddNewMonster() code into new LinkMonster method.
// Removed plot history stuff.
// Changed GetSwordCoords() to only gather swords that are inside the room area.
// Made some parameters and vars const.
// Moved member initialization in constructor's initialization list.
//
// Revision 1.66  2002/10/22 05:28:08  mrimer
// Revised includes.
//
// Revision 1.65  2002/10/18 23:04:20  mrimer
// Added: remove scroll when trapdoor falls.
//
// Revision 1.64  2002/09/27 17:49:48  mrimer
// Fixed usage of DEFAULT_SMELL_RANGE.
//
// Revision 1.63  2002/09/24 21:12:00  mrimer
// Added SomeMonsterCanSmellSwordsman().
//
// Revision 1.62  2002/09/13 21:25:58  mrimer
// Moved some code into DecMonsterCount().
//
// Revision 1.61  2002/09/10 18:00:40  mrimer
// Added GetSwordCoords().
//
// Revision 1.60  2002/09/03 22:38:21  mrimer
// Fixed tar growth and stabbing bugs at room edges.
//
// Revision 1.59  2002/08/28 21:36:39  erikh2000
// A cue event is now added when red doors open.
//
// Revision 1.58  2002/08/28 20:29:14  mrimer
// Added IsMonsterWithin().
// Optimized AreMonstersInRect().
//
// Revision 1.57  2002/07/25 18:52:00  mrimer
// Tweaking.
//
// Revision 1.56  2002/07/17 20:08:33  erikh2000
// Changed tar removal methods to return cue events.
//
// Revision 1.55  2002/07/05 10:43:46  erikh2000
// Fixed a few bugs involving mimics not treating the swordsman as an obstacle.
//
// Revision 1.54  2002/06/23 10:47:22  erikh2000
// Fixed a bug where monster array is not cleared on arrival to a conquered room.
//
// Revision 1.53  2002/06/22 05:51:01  erikh2000
// Changed code to use new CMimic methods for getting sword coords.
// Removed the Wraithwing/pathmap testing code.
//
// Revision 1.52  2002/06/22 01:01:02  erikh2000
// All roaches are replaced with wraithwings.  Temporary change made for testing.
//
// Revision 1.51  2002/06/21 04:33:18  mrimer
// Optimized monster lookup by adding pointer array.
// Added multiple pathmap support.
// Revised includes and added consts to several formal parameters.
//
// Revision 1.50  2002/06/20 04:07:21  erikh2000
// Changed a call to a renamed method.
//
// Revision 1.49  2002/06/15 18:29:54  erikh2000
// Changed code so that storage is not committed after each database write.
//
// Revision 1.48  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.47  2002/05/15 02:48:35  erikh2000
// Added an extra check to avoid an assertian firing during tar stabs.
//
// Revision 1.46  2002/05/14 19:48:05  mrimer
// Fixed logic for playing "Ha!" sound.
//
// Revision 1.45  2002/05/14 17:22:50  mrimer
// Now basing particle explosion direction on sword movement (not orientation).
// Changed tar babies' stab appearance to TarStabEffect.
//
// Revision 1.44  2002/04/28 23:48:54  erikh2000
// Added member to CDbRoom that can be used to access all saved games for a room.
// Added dwLevelID parameter to CDbRooms::FindAtCoords()--necessary because in the future rooms on different levels will have same room coords.
//
// Revision 1.43  2002/04/22 21:52:29  mrimer
// Augmented CID_CrumblyWallDestroyed event to include player orientation info.
//
// Revision 1.42  2002/04/12 21:43:54  mrimer
// Added code to perform simultaneous tar stabbings.
// Fixed two bugs in tar growth code.
//
// Revision 1.41  2002/03/16 11:45:20  erikh2000
// Changed CDbRoom::KillMonster() and KillMonsterAtSquare() to return new cueevents param.  (Committed on behalf of mrimer.)
// Wrote code to add cue events in response to game events.  (Committed on behalf of mrimer.)
//
// Revision 1.40  2002/03/14 03:52:53  erikh2000
// Moved most code from CDbRoom::StabTar() into new IsTarVulnerableToStab() and RemoveStabbedTar() methods, so that RemoveStabbedTar() could be called from CTarMother::OnStabbed() without checking tar vulnerability.
// Changed wCol,wRow parameters to wX,wY for consistency.
//
// Revision 1.39  2002/03/13 23:45:05  erikh2000
// Fixed problem with tar not growing under swords.  (Committed on behalf of mrimer.)
//
// Revision 1.38  2002/03/13 06:32:47  erikh2000
// Twiddling.
//
// Revision 1.37  2002/03/04 22:24:09  erikh2000
// Removed unused line of code.
//
// Revision 1.36  2002/02/27 23:52:13  erikh2000
// Created CDbRoom::ResetMonsterFirstTurnFlags() to replace some redundant code in CCurrentGame.
//
// Revision 1.35  2002/02/25 08:21:30  erikh2000
// Changed CDbRoom::GrowTar() to spawn tarbabies in more cases.
// Removed commented-out code.
//
// Revision 1.34  2002/02/25 03:37:10  erikh2000
// Added CDbRoom::DoesSquareContainPathmapObstacle().
// Changed CDbRoom::Plot() to handle all incremental changes to pathmap obstacles.
//
// Revision 1.33  2002/02/24 06:20:28  erikh2000
// Added CDbRoom::GetMimicSwordCoords() and CDbRoom::DoesSquareContainMimicSword().
// Added checks for swords to tar baby formation criteria in GrowTar().
//
// Revision 1.32  2002/02/24 03:45:20  erikh2000
// Added CDbRoom::DestroyCrumblyWall().
// Added CDbRoom::DoesSquareContainMimicPlacementObstacle().
//
// Revision 1.31  2002/02/24 02:03:30  erikh2000
// ActivateOrb() now adds a CID_OrbActivated cue event.
//
// Revision 1.30  2002/02/23 04:59:00  erikh2000
// Fixed incorrect out of bounds check in CDbRoom::StabTar().
//
// Revision 1.29  2002/02/16 02:55:40  erikh2000
// Changed CDbRoom::FloodPlot() to flood to diagonal squares as well as orthogonal.
//
// Revision 1.28  2002/02/09 00:56:17  erikh2000
// Changed current game pointers to const.
// Moved md5i's CCurrentGame::StabTar() to CDbRoom::StabTar().
// Moved md5i's CCurrentGame::GrowTar() to CDbRoom::GrowTar().
//
// Revision 1.27  2002/02/08 23:18:14  erikh2000
// Changed CDbRoom to delete monsters that have been killed during object destruction.
// CDbRoom::DeleteMonster() and ::DeleteMonsterAtSquare() renamed to ::KillMonster() and ::KillMonsterAtSquare().
//
// Revision 1.26  2002/02/07 23:28:18  erikh2000
// Added CDbRoom::IsDoorOpen().  (Committed on behalf of j_wicks.)
//
// Revision 1.25  2001/12/16 02:11:56  erikh2000
// Simplified calls to CIDList::Get().
//
// Revision 1.24  2001/12/08 05:17:09  erikh2000
// Added CDbRoom::GetFirst() and CDbRoom::GetNext() methods.
//
// Revision 1.23  2001/11/25 21:26:51  erikh2000
// Added call to CMonster::SetMembersFromExtraVars() in CDbRoom::LoadMonsters().
//
// Revision 1.22  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.21  2001/11/19 09:22:40  erikh2000
// Changed CDbRoom::AddNewMonster() to not take a process sequence parameter.
//
// Revision 1.20  2001/11/18 05:02:44  md5i
// Trapdoor with pathmap fix.
//
// Revision 1.19  2001/11/18 04:10:37  md5i
// A few mimic fixes.
//
// Revision 1.18  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.17  2001/11/11 05:01:16  md5i
// Added serpents.
//
// Revision 1.16  2001/11/06 08:49:58  erikh2000
// Wrote DestroyTrapdoor() and added code to track number of trap doors left.  (Committed on behalf of jpburford.)
//
// Revision 1.15  2001/11/03 20:17:28  erikh2000
// Removed OnPlot() and OnLoad() references.  Added plot history tracking.
//
// Revision 1.14  2001/10/29 19:31:41  erikh2000
// Added JP Burford to contributors list.
//
// Revision 1.13  2001/10/29 19:29:33  erikh2000
// Added CDbRoom::ChangeTiles() method and calls from door-removal methods.  (Committed on behalf of jpburford.)
//
// Revision 1.12  2001/10/27 04:41:07  erikh2000
// Added OnLoad callbacks.
// Changed SetOnLoad() and SetOnPlot() to protected to encourage setting callbacks with CCurrentGame methods.
//
// Revision 1.11  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.10  2001/10/25 05:37:38  md5i
// Fixed arrow problem for Swordsman.
//
// Revision 1.9  2001/10/25 05:05:30  md5i
// Fixed attributions.
//
// Revision 1.8  2001/10/24 01:43:17  md5i
// Actually add different types of monsters.
//
// Revision 1.7  2001/10/21 00:28:24  erikh2000
// Fixed bugs preventing monsters from being processed.
// Added CDbRoom::Update() stub.
// Fixed access violation caused by bad CMonster.pNext pointer.
//
// Revision 1.6  2001/10/20 20:05:12  erikh2000
// Wrote ActivateOrb().
//
// Revision 1.5  2001/10/20 05:45:36  erikh2000
// Added Plot() and SetOnPlot() methods.
// Added stubs for some routines to be written.
// Removed dead code.
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
// Revision 1.1.1.1  2001/10/01 22:20:12  erikh2000
// Initial check-in.
//
