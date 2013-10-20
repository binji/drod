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
 * JP Burford (jpburford)
 *
 * ***** END LICENSE BLOCK ***** */

//DbSavedGames.cpp
//Implementation of CDbSavedGame and CDbSavedGames.

#include "DbSavedGames.h"

#include "Db.h"
#include "DBProps.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Base64.h>
#include <BackEndLib/Ports.h>

//
//CDbSavedGame protected methods.
//

//*******************************************************************************
CDbSavedGame::CDbSavedGame()
//Constructor.
{
	Clear();
}

//
//CDbSavedGame public methods.
//

//*******************************************************************************
CDbSavedGame::~CDbSavedGame()
//Destructor.
{
	Clear();
}

//*******************************************************************************
bool CDbSavedGame::Load(
//Loads a saved game from database into this object.
//
//Params:
	const DWORD dwLoadSavedGameID)	//(in) ID of saved game to load.
//
//Returns:
//True if successful, false if not.
{
	DWORD dwRoomI, dwRoomCount;
	c4_View ConqueredRoomsView;
	c4_View ExploredRoomsView;
	bool bSuccess=true;
	
	Clear();

	//Open saved games view.
	ASSERT(IsOpen());
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));

	//Find record with matching saved game ID.
	const DWORD dwSavedGameI = LookupRowByPrimaryKey(dwLoadSavedGameID,
			p_SavedGameID, SavedGamesView);
	if (dwSavedGameI == ROW_NO_MATCH) {bSuccess=false; goto Cleanup;}

	//Load in props from SavedGames record.
	this->dwSavedGameID = (DWORD) (p_SavedGameID(SavedGamesView[dwSavedGameI]));
	ASSERT(this->dwSavedGameID == dwLoadSavedGameID);
	this->dwPlayerID = (DWORD) (p_PlayerID(SavedGamesView[dwSavedGameI]));
	this->dwRoomID = (DWORD) (p_RoomID(SavedGamesView[dwSavedGameI]));
	if (!this->dwRoomID) //Placeholder record.
	{
		bSuccess = false;
		goto Cleanup;
	}
	this->eType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGameI] );
	this->wCheckpointX = (UINT) p_CheckpointX( SavedGamesView[dwSavedGameI] );
	this->wCheckpointY = (UINT) p_CheckpointY( SavedGamesView[dwSavedGameI] );
	this->bIsHidden = ( p_IsHidden( SavedGamesView[dwSavedGameI])!=0 );
	this->wStartRoomX = (UINT) p_StartRoomX( SavedGamesView[dwSavedGameI] );
	this->wStartRoomY = (UINT) p_StartRoomY( SavedGamesView[dwSavedGameI] );
	this->wStartRoomO = (UINT) p_StartRoomO( SavedGamesView[dwSavedGameI] );
	this->Created = (time_t) p_Created( SavedGamesView[dwSavedGameI] );
	this->LastUpdated = (time_t) p_LastUpdated( SavedGamesView[dwSavedGameI] );
	this->Commands = p_Commands( SavedGamesView[dwSavedGameI] );

	//Populate conquered room list.
	ConqueredRoomsView = p_ConqueredRooms( SavedGamesView[dwSavedGameI] );
	dwRoomCount = ConqueredRoomsView.GetSize();
	for (dwRoomI = 0; dwRoomI < dwRoomCount; dwRoomI++)
		this->ConqueredRooms.Add((DWORD) p_RoomID(ConqueredRoomsView[dwRoomI]));

	//Populate explored room list.
	ExploredRoomsView = p_ExploredRooms( SavedGamesView[dwSavedGameI] );
	dwRoomCount = ExploredRoomsView.GetSize();
	for (dwRoomI = 0; dwRoomI < dwRoomCount; dwRoomI++)
		this->ExploredRooms.Add((DWORD) p_RoomID(ExploredRoomsView[dwRoomI]));

Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
MESSAGE_ID CDbSavedGame::SetProperty(
//Used during XML data import.
//According to pType, convert string to proper datatype and member
//
//Returns: whether operation was successful
//
//Params:
	const PROPTYPE pType,	//(in) property (data member) to set
	char* const str,			//(in) string representation of value
	CImportInfo &Maps,	   //(in/out) Import data
	bool &bSaveRecord)		//(out) whether record should be saved
{
	static PrimaryKeyMap::iterator localID;
	switch (pType)
	{
		case P_SavedGameID:
		{
			this->dwSavedGameID = static_cast<DWORD>(atol(str));
			if (!this->dwSavedGameID)
				return MID_FileCorrupted;	//corrupt data

			//Look up local ID.
			localID = Maps.SavedGameIDMap.find(this->dwSavedGameID);
			if (localID != Maps.SavedGameIDMap.end())
				//Error - this saved game should not have been imported yet
				return MID_FileCorrupted;

         if (bSaveRecord)
         {
			   //Add a new record to the DB.
			   const DWORD dwOldLocalID = this->dwSavedGameID;
			   this->dwSavedGameID = 0L;
			   Update();
			   Maps.SavedGameIDMap[dwOldLocalID] = this->dwSavedGameID;
         } else {
            //This saved game is being ignored.
            //(It belongs to a non-existant hold/level/room.)
            Maps.SavedGameIDMap[this->dwSavedGameID] = 0;   //skip records with refs to this saved game ID
         }
			break;
		}
		case P_PlayerID:
			this->dwPlayerID = static_cast<DWORD>(atol(str));
			if (!this->dwPlayerID)
				return MID_FileCorrupted;	//corrupt data (saved game must have player)

			//Look up local ID.
			localID = Maps.PlayerIDMap.find(this->dwPlayerID);
			if (localID == Maps.PlayerIDMap.end())
				return MID_FileCorrupted;	//record should exist now
			this->dwPlayerID = localID->second;
			break;
		case P_RoomID:
			this->dwRoomID = static_cast<DWORD>(atol(str));
			if (!this->dwRoomID)
				return MID_FileCorrupted;	//corrupt data (saved game must have a room)

			//Look up local ID.
			localID = Maps.RoomIDMap.find(this->dwRoomID);
			if (localID == Maps.RoomIDMap.end())
				return MID_FileCorrupted;	//record should exist now
			this->dwRoomID = localID->second;
         if (!this->dwRoomID)
         {
            //Records for this room are being ignored.  Don't save this game.
            bSaveRecord = false;
         }
			break;
		case P_Type:
			this->eType = static_cast<SAVETYPE>(atol(str));
			break;
		case P_CheckpointX:
			this->wCheckpointX = static_cast<UINT>(atoi(str));
			break;
		case P_CheckpointY:
			this->wCheckpointY = static_cast<UINT>(atoi(str));
			break;
		case P_IsHidden:
			this->bIsHidden = (atoi(str) != 0);
			break;
		case P_LastUpdated:
			this->LastUpdated = (time_t)atol(str);
			break;
		case P_StartRoomX:
			this->wStartRoomX = static_cast<UINT>(atoi(str));
			break;
		case P_StartRoomY:
			this->wStartRoomY = static_cast<UINT>(atoi(str));
			break;
		case P_StartRoomO:
			this->wStartRoomO = static_cast<UINT>(atoi(str));
			break;
		case P_ConqueredRooms:
		{
			//Parse list of room IDs.
			char *tempstr = new char[strlen(str)+1];
			strcpy(tempstr,str);

			DWORD dwRoomID;
			char *token = strtok(tempstr, " ");
			do
			{
				//IDs will be matched to local ones later on completion of import.
				dwRoomID = static_cast<DWORD>(atol(token));
				if (!dwRoomID)
					return MID_FileCorrupted;	//corrupt file
				this->ConqueredRooms.Add(dwRoomID);
				token = strtok(NULL, " ");
			} while (token);

			delete[] tempstr;
			break;
		}
		case P_ExploredRooms:
		{
			//Parse list of room IDs.
			char *tempstr = new char[strlen(str)+1];
			strcpy(tempstr,str);

			DWORD dwRoomID;
			char *token = strtok(tempstr, " ");
			do
			{
				//IDs will be matched to local ones later on completion of import.
				dwRoomID = static_cast<DWORD>(atol(token));
				if (!dwRoomID)
					return MID_FileCorrupted;	//corrupt file
				this->ExploredRooms.Add(dwRoomID);
				token = strtok(NULL, " ");
			} while (token);

			delete[] tempstr;
			break;
		}
		case P_Created:
			this->Created = (time_t)atol(str);
			break;
		case P_Commands:
		{
			const string sstr = str;
			BYTE *data;
			Base64::decode(sstr,data);
			this->Commands = (const BYTE*)data;
			delete[] data;
			break;
		}
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbSavedGame::Update()
//Updates database with saved game.
{
   g_pTheDB->SavedGames.ResetMembership();
	if (this->dwSavedGameID == 0)
		//Insert a new saved game.
		return UpdateNew();
	else
		//Update existing saved game.
		return UpdateExisting();
}

//
//CDbSavedGame private methods.
//

//*****************************************************************************
bool CDbSavedGame::UpdateNew()
//Add new SavedGames record to database.
{
	ASSERT(this->dwSavedGameID == 0);
	ASSERT(IsOpen());
	
	//Get explored rooms subview ready.
	c4_View ExploredRoomsView;
	if (this->ExploredRooms.GetSize())
	{
		IDNODE *pSeek = this->ExploredRooms.Get(0);
		while (pSeek)
		{
			ASSERT(pSeek->dwID);
			ExploredRoomsView.Add( p_RoomID[ pSeek->dwID ] );
			pSeek = pSeek->pNext;
		}
	}

	//Get conquered rooms subview ready.
	c4_View ConqueredRoomsView;
	if (this->ConqueredRooms.GetSize())
	{
		IDNODE *pSeek = this->ConqueredRooms.Get(0);
		while (pSeek)
		{
			ASSERT(pSeek->dwID);
			ConqueredRoomsView.Add( p_RoomID[ pSeek->dwID ] );
			pSeek = pSeek->pNext;
		}
	}

	//Write SavedGames record.
	this->Created.SetToNow();
	this->LastUpdated.SetToNow();
	this->dwSavedGameID = GetIncrementedID(p_SavedGameID);
	DWORD dwCommandsSize;
	BYTE *pbytCommands = this->Commands.GetPackedBuffer(dwCommandsSize);
	c4_Bytes CommandsBytes(pbytCommands, dwCommandsSize);
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	SavedGamesView.Add(
			p_SavedGameID[ this->dwSavedGameID ] +
			p_PlayerID[ this->dwPlayerID ] +
			p_RoomID[ this->dwRoomID ] +
			p_Type[ this->eType ] +
			p_CheckpointX[ this->wCheckpointX ] +
			p_CheckpointY[ this->wCheckpointY ] +
			p_IsHidden[ this->bIsHidden ] +
			p_LastUpdated[ this->LastUpdated ] +
			p_StartRoomX[ this->wStartRoomX ] +
			p_StartRoomY[ this->wStartRoomY ] +
			p_StartRoomO[ this->wStartRoomO ] +
			p_ExploredRooms[ ExploredRoomsView ] +
			p_ConqueredRooms[ ConqueredRoomsView ] +
			p_Created[ this->Created ] +
			p_Commands[ CommandsBytes ] );
	delete[] pbytCommands;

	return true;
}

//*******************************************************************************
bool CDbSavedGame::UpdateExisting()
//Update an existing SavedGames record in database.
{
	ASSERT(this->dwSavedGameID != 0);
	ASSERT(IsOpen());

	//Lookup SavedGames record.
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameI = LookupRowByPrimaryKey(this->dwSavedGameID,
			p_SavedGameID, SavedGamesView);
	if (dwSavedGameI == ROW_NO_MATCH)
	{
		ASSERTP(false, "CDbSavedGame::UpdateExisting() -- Bad Saved Game ID.");
		return false;
	}

	//Get explored rooms subview ready.
	c4_View ExploredRoomsView;
	if (this->ExploredRooms.GetSize())
	{
		IDNODE *pSeek = this->ExploredRooms.Get(0);
		while (pSeek)
		{
			ASSERT(pSeek->dwID);
			ExploredRoomsView.Add( p_RoomID[ pSeek->dwID ] );
			pSeek = pSeek->pNext;
		}
	}

	//Get conquered rooms subview ready.
	c4_View ConqueredRoomsView;
	if (this->ConqueredRooms.GetSize())
	{
		IDNODE *pSeek = this->ConqueredRooms.Get(0);
		while (pSeek)
		{
			ASSERT(pSeek->dwID);
			ConqueredRoomsView.Add( p_RoomID[ pSeek->dwID ] );
			pSeek = pSeek->pNext;
		}
	}

	//Update SavedGames record.
   if (!CDb::FreezingTimeStamps())
	   this->LastUpdated.SetToNow();
	DWORD dwCommandsSize;
	BYTE *pbytCommands = this->Commands.GetPackedBuffer(dwCommandsSize);
	c4_Bytes CommandsBytes(pbytCommands, dwCommandsSize);
	
	p_SavedGameID( SavedGamesView[ dwSavedGameI ] ) = this->dwSavedGameID;
	p_PlayerID( SavedGamesView[ dwSavedGameI] ) = this->dwPlayerID;
	p_RoomID( SavedGamesView[ dwSavedGameI ] ) = this->dwRoomID;
	p_Type( SavedGamesView[ dwSavedGameI ] ) = this->eType;
	p_CheckpointX( SavedGamesView[ dwSavedGameI ] ) = this->wCheckpointX;
	p_CheckpointY( SavedGamesView[ dwSavedGameI ] ) = this->wCheckpointY;
	p_IsHidden( SavedGamesView[ dwSavedGameI ] ) = this->bIsHidden;
	p_LastUpdated( SavedGamesView[ dwSavedGameI ] ) = this->LastUpdated;
	p_StartRoomX( SavedGamesView[ dwSavedGameI ] ) = this->wStartRoomX;
	p_StartRoomY( SavedGamesView[ dwSavedGameI ] ) = this->wStartRoomY;
	p_StartRoomO( SavedGamesView[ dwSavedGameI ] ) = this->wStartRoomO;
	p_ExploredRooms( SavedGamesView[ dwSavedGameI ] ) = ExploredRoomsView;
	p_ConqueredRooms( SavedGamesView[ dwSavedGameI ] ) = ConqueredRoomsView;
	p_Created( SavedGamesView[ dwSavedGameI ] ) = this->Created;
	p_Commands( SavedGamesView[ dwSavedGameI ] ) = CommandsBytes;

	delete[] pbytCommands;

	return true;
}

//*******************************************************************************
void CDbSavedGame::Clear()
//Frees resources associated with object and zeroes members.
{
	this->dwRoomID=this->dwSavedGameID=0L;
	this->bIsHidden=false;
	this->wStartRoomX=this->wStartRoomY=this->wStartRoomO=
			this->wCheckpointX=this->wCheckpointY=0;
	this->eType = ST_Unknown;

	this->dwPlayerID = 0L;

	this->ConqueredRooms.Clear();
	this->ExploredRooms.Clear();
	this->Commands.Clear();
}

//
//CDbSavedGames public methods.
//

//*******************************************************************************
void CDbSavedGames::Delete(
//Deletes a saved games record.
//NOTE: Don't call this directly to delete saved games belonging to demos.
//Instead call CDbDemos::Delete() to delete the demo and associated saved game.
//
//Params:
	const DWORD dwSavedGameID)	//(in)	Saved game to delete.
{
	ASSERT(dwSavedGameID);

	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameRowI = LookupRowByPrimaryKey(dwSavedGameID,
			p_SavedGameID, SavedGamesView);
	if (dwSavedGameRowI==ROW_NO_MATCH) {ASSERTP(false, "Bad dwSavedGameID."); return;}

	SavedGamesView.RemoveAt(dwSavedGameRowI);

	//After object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//*******************************************************************************
void CDbSavedGames::DeleteForRoom(
//Deletes all saved games records (and demos) for the specified room.
//
//Params:
	const DWORD dwRoomID)	//(in)	Room's saved games to delete.
{
   //Delete demos and their saved games...
   CDb db;
   CIDList ids;
   IDNODE *pID;
   db.Demos.FilterByRoom(dwRoomID);
	db.Demos.FindHiddens(true);
   db.Demos.GetIDs(ids);
   pID = ids.Get(0);
   while (pID)
   {
      db.Demos.Delete(pID->dwID);
      pID = pID->pNext;
   }

   //...and then saved games w/o demos.
   db.SavedGames.FilterByRoom(dwRoomID);
	db.SavedGames.FindHiddens(true);
   db.SavedGames.GetIDs(ids);
   pID = ids.Get(0);
   while (pID)
   {
      db.SavedGames.Delete(pID->dwID);
      pID = pID->pNext;
   }
}

//*****************************************************************************
string CDbSavedGames::ExportXML(
//Returns: string containing XML text describing saved game with this ID
//
//Pre-condition: dwSavedGameID is valid
//
//Params:
//NOTE: Unused param names commented out to suppress warning
	const DWORD dwSavedGameID,	//(in)
	CDbRefs &dbRefs,			//(in/out)
	const bool /*bRef*/)		//(in)
{
#define STARTTAG(vType,pType) "<"; str += ViewTypeStr(vType); str += " "; str += PropTypeStr(pType); str += "='"
#define PROPTAG(pType) "' "; str += PropTypeStr(pType); str += "='"
#define ENDTAG(vType) "</"; str += ViewTypeStr(vType); str += ">\n"
#define CLOSETAG "'/>\n"
#define CLOSESTARTTAG "'>\n"
#define LONGTOSTR(val) _ltoa((val), dummy, 10)

	string str;

	if (!dbRefs.IsSet(V_SavedGames,dwSavedGameID))
	{
      dbRefs.Set(V_SavedGames,dwSavedGameID);

      CDbSavedGame *pSavedGame = GetByID(dwSavedGameID);
      if (!pSavedGame)
         return str; //placeholder record -- not needed
		DWORD dwIndex, dwSize;

		//Include corresponding player and room refs.
		str += g_pTheDB->Players.ExportXML(pSavedGame->dwPlayerID, dbRefs, true);
		str += g_pTheDB->Rooms.ExportXML(pSavedGame->dwRoomID, dbRefs, true);

		//First include refs for rooms in explored/conquered lists.
		dwSize = pSavedGame->ExploredRooms.GetSize();
		for (dwIndex=0; dwIndex<dwSize; ++dwIndex)
		{
			str += g_pTheDB->Rooms.ExportXML(
					pSavedGame->ExploredRooms.Get(dwIndex)->dwID, dbRefs, true);
		}
		dwSize = pSavedGame->ConqueredRooms.GetSize();
		for (dwIndex=0; dwIndex<dwSize; ++dwIndex)
		{
			str += g_pTheDB->Rooms.ExportXML(
					pSavedGame->ConqueredRooms.Get(dwIndex)->dwID, dbRefs, true);
		}

		//Prepare data.
		char dummy[32];
		DWORD dwBufSize;
		BYTE *const pCommands = pSavedGame->Commands.GetPackedBuffer(dwBufSize);

      str += STARTTAG(V_SavedGames, P_PlayerID);
		str += LONGTOSTR(pSavedGame->dwPlayerID);
		str += PROPTAG(P_RoomID);
		str += LONGTOSTR(pSavedGame->dwRoomID);
		str += PROPTAG(P_Type);
		str += LONGTOSTR((long)pSavedGame->eType);
		str += PROPTAG(P_SavedGameID);
		str += LONGTOSTR(pSavedGame->dwSavedGameID);
		str += PROPTAG(P_CheckpointX);
		str += LONGTOSTR(pSavedGame->wCheckpointX);
		str += PROPTAG(P_CheckpointY);
		str += LONGTOSTR(pSavedGame->wCheckpointY);
		str += PROPTAG(P_IsHidden);
		str += LONGTOSTR((long)pSavedGame->bIsHidden);
		str += PROPTAG(P_LastUpdated);
		str += LONGTOSTR((time_t)pSavedGame->LastUpdated);
		str += PROPTAG(P_StartRoomX);
		str += LONGTOSTR((long)pSavedGame->wStartRoomX);
		str += PROPTAG(P_StartRoomY);
		str += LONGTOSTR((long)pSavedGame->wStartRoomY);
		str += PROPTAG(P_StartRoomO);
		str += LONGTOSTR((long)pSavedGame->wStartRoomO);
		//Process room lists.
		dwSize = pSavedGame->ExploredRooms.GetSize();
		if (dwSize > 0)
		{
			str += PROPTAG(P_ExploredRooms);
			for (dwIndex=0; dwIndex<dwSize; ++dwIndex)
			{
				str += LONGTOSTR(pSavedGame->ExploredRooms.Get(dwIndex)->dwID);
				str += " ";
			}
		}

		dwSize = pSavedGame->ConqueredRooms.GetSize();
		if (dwSize > 0)
		{
			str += PROPTAG(P_ConqueredRooms);
			for (dwIndex=0; dwIndex<dwSize; ++dwIndex)
			{
				str += LONGTOSTR(pSavedGame->ConqueredRooms.Get(dwIndex)->dwID);
				str += " ";
			}
		}
		str += PROPTAG(P_Created);
		str += LONGTOSTR((time_t)pSavedGame->Created);
		str += PROPTAG(P_Commands);
		if (dwBufSize > sizeof(BYTE))
			str += Base64::encode(pCommands, dwBufSize-sizeof(BYTE));	//strip null BYTE
		str += CLOSETAG;

		delete[] pCommands;
		delete pSavedGame;
	}

	return str;

#undef STARTTAG
#undef PROPTAG
#undef ENDTAG
#undef CLOSETAG
#undef CLOSESTARTTAG
#undef LONGTOSTR
}

//*********************************************************************************
void CDbSavedGames::FilterByHold(
//Changes filter so that GetFirst() and GetNext() will return saved games 
//for a specified hold (and player, if specified).
//
//Params:
	const DWORD dwSetFilterByHoldID)	//(in)	Hold ID to filter by.  Set to 0
								//		for all saved games.
{
	if (this->bIsMembershipLoaded && (dwSetFilterByHoldID != this->dwFilterByHoldID || 
		this->dwFilterByLevelID != 0L || this->dwFilterByRoomID != 0L) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByLevelID = this->dwFilterByRoomID = 0L;
	this->dwFilterByHoldID = dwSetFilterByHoldID;
}

//*********************************************************************************
void CDbSavedGames::FilterByLevel(
//Changes filter so that GetFirst() and GetNext() will return saved games 
//for a specified level (and player, if specified). 
//
//Params:
	const DWORD dwSetFilterByLevelID)	//(in)	Level ID to filter by.  Set to
								//		0 for all saved games.
{
	if (this->bIsMembershipLoaded && (dwSetFilterByLevelID != this->dwFilterByLevelID || 
		this->dwFilterByHoldID != 0L || this->dwFilterByRoomID != 0L) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByHoldID = this->dwFilterByRoomID = 0L;
	this->dwFilterByLevelID = dwSetFilterByLevelID;
}

//*********************************************************************************
void CDbSavedGames::FilterByPlayer(
//Changes filter so that GetFirst() and GetNext() will return saved games 
//for a specified player.  Other saved game filters stay in effect.
//
//Params:
	const DWORD dwSetFilterByPlayerID)	//(in)	Player ID to filter by.
								// Set to 0 for all saved games.
								// (Other filters remain in effect.)
{
	if (this->bIsMembershipLoaded && (dwSetFilterByPlayerID !=
			this->dwFilterByPlayerID) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	//Don't zero other filter types.
	this->dwFilterByPlayerID = dwSetFilterByPlayerID;
}

//*********************************************************************************
void CDbSavedGames::FilterByRoom(
//Changes filter so that GetFirst() and GetNext() will return saved games
//for a specified room (and player, if specified).
//
//Params:
	const DWORD dwSetFilterByRoomID)	//(in)	Room ID to filter by.  Set to 0
								//		for all saved games.
{
	if (this->bIsMembershipLoaded && (dwSetFilterByRoomID != this->dwFilterByRoomID || 
		this->dwFilterByHoldID != 0L || this->dwFilterByLevelID != 0L) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByHoldID = this->dwFilterByLevelID = 0L;
	this->dwFilterByRoomID = dwSetFilterByRoomID;
}

//*******************************************************************************
void CDbSavedGames::FindHiddens(const bool bFlag)
//Changes filter so that hidden saved games will also be returned (or not).
//Other filters remain the same.
{
   if (bFlag != this->bLoadHidden)
   {
 		//Membership is invalid.
		this->bIsMembershipLoaded = false;
      this->bLoadHidden = bFlag;
   }
}

//*******************************************************************************
DWORD CDbSavedGames::FindByContinue()
//Finds ID of saved game that was saved to continue slot
//for the current player and hold.
//
//Returns:
//SavedGameID of the found continue saved game, or 0 if none were found.
{
	ASSERT(IsOpen());
	const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGamesCount = SavedGamesView.GetSize();

	//Each iteration looks at one saved game record for a match.
	SAVETYPE eSaveType;
	DWORD dwPlayerID, dwRoomID;
	CDbRoom *pRoom;
	CDbLevel *pLevel;
   //counting down so the possible deletion below doesn't mess up the iteration
	for (DWORD dwSavedGamesI=dwSavedGamesCount; dwSavedGamesI--; )
	{
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGamesI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGamesI] );
		if (eSaveType == ST_Continue && dwPlayerID == dwCurrentPlayerID)
		{
			//Find player's continue slot for this hold.
			dwRoomID = (DWORD) p_RoomID( SavedGamesView[dwSavedGamesI] );
			if (!dwRoomID)
			{
				//Unused saved game record -- this one can be used.
				const DWORD dwSavedGameID = p_SavedGameID( SavedGamesView[dwSavedGamesI] );
				return dwSavedGameID; //Found it.
			}
			pRoom = g_pTheDB->Rooms.GetByID(dwRoomID);
         if (pRoom)
         {
			   pLevel = g_pTheDB->Levels.GetByID(pRoom->dwLevelID);
            ASSERT(pLevel);
			   delete pRoom;
			   if (pLevel->dwHoldID == g_pTheDB->GetHoldID())
			   {
				   delete pLevel;
				   const DWORD dwSavedGameID = p_SavedGameID( SavedGamesView[dwSavedGamesI] );
				   return dwSavedGameID; //Found it.
			   }
			   delete pLevel;
         } else {
            //Saved game pointing to non-existant room -- delete it.
				const DWORD dwSavedGameID = p_SavedGameID( SavedGamesView[dwSavedGamesI] );
            g_pTheDB->SavedGames.Delete(dwSavedGameID);
         }
		}
	}

   //Didn't find one.
   return 0;
}

//*******************************************************************************
DWORD CDbSavedGames::SaveNewContinue(const DWORD dwPlayerID)
//Make new continue saved game slot for this player.
//
//Returns: new saved game ID
{
	CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetNew();
	pSavedGame->dwPlayerID = dwPlayerID;
	pSavedGame->dwRoomID = 0L;
	pSavedGame->eType = ST_Continue;
	pSavedGame->bIsHidden = true;
	pSavedGame->wStartRoomX = 0;
	pSavedGame->wStartRoomY = 0;
	pSavedGame->Update();
	const DWORD dwSavedGameID = pSavedGame->dwSavedGameID;
	delete pSavedGame;

	return dwSavedGameID;
}

//*******************************************************************************
DWORD CDbSavedGames::FindByContinueLatest(
//Finds the latest continue saved game ID for the given player.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
//
//Params:
   const DWORD dwLookupPlayerID)
{
	ASSERT(IsOpen());

	ASSERT(dwLookupPlayerID);

	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGamesCount = SavedGamesView.GetSize();

	//Each iteration looks at one saved game record.
	SAVETYPE eSaveType;
	DWORD dwPlayerID, dwRoomID;
	DWORD dwLatestSavedGameIndex = ROW_NO_MATCH;
	DWORD dwLatestTime = 0L;
	for (DWORD dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGamesI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGamesI] );
		dwRoomID = (DWORD) p_RoomID( SavedGamesView[dwSavedGamesI] );
		if (eSaveType == ST_Continue && dwPlayerID == dwLookupPlayerID && dwRoomID &&
				(DWORD) p_LastUpdated( SavedGamesView[dwSavedGamesI] ) > dwLatestTime)
		{
			//This continue saved game is the most recent one found so far.
			dwLatestSavedGameIndex = dwSavedGamesI;
			dwLatestTime = (DWORD) p_LastUpdated( SavedGamesView[dwSavedGamesI] );
		}
	}

	//No continue slot found for player.
	if (dwLatestSavedGameIndex == ROW_NO_MATCH)
		return 0L;

	//Found player's most recent continue slot.
   const DWORD dwSavedGameID = p_SavedGameID( SavedGamesView[dwLatestSavedGameIndex] );
   return dwSavedGameID;
}

//*****************************************************************************
DWORD CDbSavedGames::FindByEndHold(
//Finds the end hold saved game for the current player for the given hold.
//
//
//Params:
	const DWORD dwHoldID) //(in)	Hold to look in.
//Returns:
//ID of the found saved game, or 0 if no match found.
{
	ASSERT(IsOpen());

	const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGamesCount = SavedGamesView.GetSize();

	//Each iteration looks at one saved game record.
	SAVETYPE eSaveType;
	DWORD dwPlayerID, dwRoomID;
	CDbRoom *pRoom;
	CDbLevel *pLevel;
	for (DWORD dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGamesI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGamesI] );
		if (eSaveType == ST_EndHold && dwPlayerID == dwCurrentPlayerID)
		{
			//Find player's end hold slot for this hold.
			dwRoomID = (DWORD) p_RoomID( SavedGamesView[dwSavedGamesI] );
			if (!dwRoomID)
				continue;
			pRoom = g_pTheDB->Rooms.GetByID(dwRoomID);
			pLevel = g_pTheDB->Levels.GetByID(pRoom->dwLevelID);
			delete pRoom;
			if (pLevel->dwHoldID == dwHoldID)
			{
				delete pLevel;
				const DWORD dwSavedGameID = p_SavedGameID( SavedGamesView[dwSavedGamesI] );
				return dwSavedGameID; //Found it.
			}
			delete pLevel;
		}
	}

	//No end hold slot found for player.
	return 0L;
}

//*******************************************************************************
DWORD CDbSavedGames::FindByLevelBegin(
//Finds a saved game that was saved to a certain level-begin slot.
//
//Params:
	const DWORD dwFindLevelID) //(in)	Level to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindLevelID);
	ASSERT(IsOpen());

	const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	//Get the starting room ID for the level.
	CDbLevel *pLevel = g_pTheDB->Levels.GetByID(dwFindLevelID);
	if (!pLevel) 
	{
		ASSERTP(false, "Couldn't load level.");
		return 0L;
	}
	const DWORD dwFindRoomID = pLevel->dwRoomID;
	delete pLevel;

	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	DWORD dwSavedGamesCount = SavedGamesView.GetSize();

	//Each iteration looks at one saved game record for a match.
	SAVETYPE eSaveType;
	DWORD dwPlayerID;
	DWORD dwLoopRoomID;
	for (DWORD dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGamesI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGamesI] );
		if (eSaveType == ST_LevelBegin && dwPlayerID == dwCurrentPlayerID)
		{
			dwLoopRoomID = p_RoomID( SavedGamesView[dwSavedGamesI] );
			if (dwFindRoomID == dwLoopRoomID)
			{
				const DWORD dwSavedGameID = p_SavedGameID( SavedGamesView[dwSavedGamesI] );
				return dwSavedGameID; //Found it.
			}
		}
	}
	return 0L;	//Didn't find it.
}

//*******************************************************************************
DWORD CDbSavedGames::FindByLevelLatest(
//Finds the latest saved game on a specified level.
//
//Params:
	const DWORD dwFindLevelID) //(in)	Level to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindLevelID);
	ASSERT(IsOpen());

	const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	//Get IDs of saved games for the level.
	CDbLevel *pLevel = g_pTheDB->Levels.GetByID(dwFindLevelID);
	if (!pLevel) 
	{
		ASSERTP(false, "Couldn't load level.(2)");
		return 0L;
	}
	CIDList SavedGameIDs;
	pLevel->SavedGames.GetIDs(SavedGameIDs);
	delete pLevel;
	if (SavedGameIDs.GetSize()==0) return 0L; //No saved games on this level.

	//Find the saved game with latest date.  Each iteration looks at the date of one
	//saved game.
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	DWORD dwLatestSavedGameID = 0L;
	DWORD dwLatestTime = 0L;
	SAVETYPE eSaveType;
	DWORD dwPlayerID;
	for (DWORD dwI = 0; dwI < SavedGameIDs.GetSize(); ++dwI)
	{
		const DWORD dwSavedGameI = LookupRowByPrimaryKey(SavedGameIDs.GetID(dwI), 
				p_SavedGameID, SavedGamesView);
		if (dwSavedGameI == ROW_NO_MATCH)
      {
         ASSERTP(false, "Corrupted DB data.");
         continue;
      }
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGameI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
		if (dwPlayerID == dwCurrentPlayerID &&
				(DWORD) p_LastUpdated( SavedGamesView[dwSavedGameI] ) >
				dwLatestTime && eSaveType != ST_Demo && eSaveType != ST_Continue)
		{
			dwLatestSavedGameID = (DWORD) p_SavedGameID(
					SavedGamesView[dwSavedGameI] );
			dwLatestTime = (DWORD) p_LastUpdated( SavedGamesView[dwSavedGameI] );
		}
	}
	ASSERT(dwLatestSavedGameID);

	//Return the latest saved game.
	return dwLatestSavedGameID;
}

//*******************************************************************************
DWORD CDbSavedGames::FindByRoomBegin(
//Finds a saved game that was saved to a certain room-begin slot.
//
//Params:
	const DWORD dwFindRoomID) //(in)	Room to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindRoomID);
	ASSERT(IsOpen());

	const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	DWORD dwSavedGamesCount = SavedGamesView.GetSize();

	//Each iteration looks at one saved game record for a match.
	SAVETYPE eSaveType;
	DWORD dwLoopRoomID;
	DWORD dwPlayerID;
	for (DWORD dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGamesI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGamesI] );
		if (eSaveType == ST_RoomBegin && dwPlayerID == dwCurrentPlayerID)
		{
			dwLoopRoomID = p_RoomID( SavedGamesView[dwSavedGamesI] );
			if (dwFindRoomID == dwLoopRoomID)
			{
				const DWORD dwSavedGameID = p_SavedGameID( SavedGamesView[dwSavedGamesI] );
				return dwSavedGameID; //Found it.
			}
		}
	}
	return 0L;	//Didn't find it.
}

//*******************************************************************************
DWORD CDbSavedGames::FindByRoomLatest(
//Finds the latest saved game in a specified room.
//
//Params:
	const DWORD dwFindRoomID) //(in)	Room to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindRoomID);
	ASSERT(IsOpen());

	const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	//Get IDs of saved games for the room.
	CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(dwFindRoomID);
	if (!pRoom) 
	{
		ASSERTP(false, "Couldn't load room.(3)");
		return 0L;
	}
	CIDList SavedGameIDs;
	pRoom->SavedGames.GetIDs(SavedGameIDs);
	delete pRoom;
	if (SavedGameIDs.GetSize()==0) return 0L; //No saved games in this room.

	//Find the saved game with latest date.  Each iteration looks at the date of one
	//saved game.
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	DWORD dwLatestSavedGameID = 0L;
	DWORD dwLatestTime = 0L;
	SAVETYPE eSaveType;
	DWORD dwPlayerID;
	for (DWORD dwI = 0; dwI < SavedGameIDs.GetSize(); ++dwI)
	{
		const DWORD dwSavedGameI = LookupRowByPrimaryKey(SavedGameIDs.GetID(dwI), 
				p_SavedGameID, SavedGamesView);
		if (dwSavedGameI == ROW_NO_MATCH)
      {
         ASSERTP(false, "Corrupted DB data.(2)");
         continue;
      }
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGameI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
		if (dwPlayerID == dwCurrentPlayerID &&
				(DWORD) p_LastUpdated( SavedGamesView[dwSavedGameI] ) >
				dwLatestTime && eSaveType != ST_Demo && eSaveType != ST_Continue) 
		{
			dwLatestSavedGameID = (DWORD) p_SavedGameID( 
					SavedGamesView[dwSavedGameI] );
			dwLatestTime = (DWORD) p_LastUpdated( SavedGamesView[dwSavedGameI] );
		}
	}
	ASSERT(dwLatestSavedGameID);

	//Return the latest saved game.
	return dwLatestSavedGameID;
}

//*******************************************************************************
DWORD CDbSavedGames::FindByCheckpoint(
//Finds a saved game that was saved to a certain checkpoint slot.
//
//Params:
	const DWORD dwFindRoomID,		//(in)	Room to look for.
	const UINT wCol, const UINT wRow)	//(in)	Square containing checkpoint.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindRoomID);
	ASSERT(IsOpen());

	const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGamesCount = SavedGamesView.GetSize();

	//Each iteration looks at one saved game record for a match.
	SAVETYPE eSaveType;
	DWORD dwPlayerID;
	for (DWORD dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGamesI] );
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGamesI] );
		if (eSaveType == ST_Checkpoint && dwPlayerID == dwCurrentPlayerID)
		{
			const UINT wCheckpointX = p_CheckpointX( SavedGamesView[dwSavedGamesI] );
			const UINT wCheckpointY = p_CheckpointY( SavedGamesView[dwSavedGamesI] );
			if (wCheckpointX == wCol && wCheckpointY == wRow)
			{
				const DWORD dwSavedGameID = p_SavedGameID(
						SavedGamesView[dwSavedGamesI] );
				return dwSavedGameID;
			}
		}
	}
	return 0L;	//Didn't find it.
}

//*****************************************************************************
DWORD CDbSavedGames::GetHoldIDofSavedGame(
//Get the ID of the hold a saved game belongs to.
//
//Params:
	const DWORD dwSavedGameID)	//(in)
const
{
	CDbSavedGame *pSavedGame = GetByID(dwSavedGameID);
	if (!pSavedGame) return 0L;
	CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(pSavedGame->dwRoomID);
   delete pSavedGame;
   if (!pRoom) return 0L;  //No room was saved in the continue slot.
	CDbLevel *pLevel = g_pTheDB->Levels.GetByID(pRoom->dwLevelID);
	delete pRoom;
   if (!pLevel) return 0L;  //Corrupted DB. Fail gracefully.
	const DWORD dwHoldID = pLevel->dwHoldID;
	delete pLevel;

	return dwHoldID;
}

//*****************************************************************************
CDbRoom* CDbSavedGame::GetRoom()
//Gets room associated with a saved game.
//
//Returns:
//Pointer to loaded room which caller must delete, or NULL if no matching room
//was found.
const
{
	CDbRoom *pRoom = new CDbRoom();
	if (pRoom)
	{
		if (!pRoom->Load(this->dwRoomID))
		{
			delete pRoom;
			pRoom=NULL;
		}
	}
	return pRoom;
}

//
//CDbSavedGames private methods.
//

//*******************************************************************************
void CDbSavedGames::LoadMembership()
//Load the membership list with all saved game IDs.
//Player filtering is done internal to hold, level, and room filtering.
{
	ASSERT(IsOpen());
	this->MembershipIDs.Clear();

	//Call function load membership with appropriate filtering.
	if (this->dwFilterByHoldID)
		LoadMembership_ByHold(this->dwFilterByHoldID);
	else if (this->dwFilterByLevelID)
		LoadMembership_ByLevel(this->dwFilterByLevelID);
	else if (this->dwFilterByRoomID)
		LoadMembership_ByRoom(this->dwFilterByRoomID);
	else if (this->dwFilterByPlayerID)
		LoadMembership_ByPlayer(this->dwFilterByPlayerID);
	else
		LoadMembership_All();

	this->pCurrentRow = this->MembershipIDs.Get(0);
	this->bIsMembershipLoaded = true;
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_All()
//Loads membership list from all saved games.
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameCount = SavedGamesView.GetSize();

	//Each iteration gets a saved game ID and puts in membership list.
	for (DWORD dwSavedGameI = 0L; dwSavedGameI < dwSavedGameCount; ++dwSavedGameI)
	{
		if (this->bLoadHidden || p_IsHidden( SavedGamesView[dwSavedGameI] ) == 0)
			this->MembershipIDs.Add( p_SavedGameID(SavedGamesView[dwSavedGameI]) );
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByRoom(const DWORD dwByRoomID)
//Loads membership list from saved games in a specified room,
//and for specified player, if any.
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameCount = SavedGamesView.GetSize();

	//Each iteration gets a saved game ID and puts in membership list.
	DWORD dwRoomID, dwPlayerID;
	for (DWORD dwSavedGameI = 0L; dwSavedGameI < dwSavedGameCount; ++dwSavedGameI)
	{
		if (this->bLoadHidden || p_IsHidden( SavedGamesView[dwSavedGameI] ) == 0)
		{
			//Does room ID match filter?
			dwRoomID = (DWORD) p_RoomID( SavedGamesView[dwSavedGameI] );
			if (dwRoomID == dwByRoomID)
			{
				//Does player ID match filter?
				if (!this->dwFilterByPlayerID)
					this->MembershipIDs.Add(p_SavedGameID(SavedGamesView[dwSavedGameI]) );
				else
				{
					dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
					if (dwPlayerID == this->dwFilterByPlayerID)
						this->MembershipIDs.Add(p_SavedGameID(SavedGamesView[dwSavedGameI]) );
				}
			}
		}
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByPlayer(const DWORD dwByPlayerID)
//Loads membership list from saved games for a specified player.
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameCount = SavedGamesView.GetSize();

	//Each iteration gets a saved game ID and puts in membership list.
	DWORD dwPlayerID;
	for (DWORD dwSavedGameI = 0L; dwSavedGameI < dwSavedGameCount; ++dwSavedGameI)
	{
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
		if (dwPlayerID == dwByPlayerID && 
				(this->bLoadHidden || p_IsHidden( SavedGamesView[dwSavedGameI] ) == 0))
			this->MembershipIDs.Add(p_SavedGameID(SavedGamesView[dwSavedGameI]) );
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByLevel(const DWORD dwByLevelID)
//Loads membership list from saved games in a specified level,
//and for specified player, if any.
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameCount = SavedGamesView.GetSize();

	//Store IDs of all the rooms in specified level.
	CIDList LevelRoomIDs;
	CDbLevel *pLevel = g_pTheDB->Levels.GetByID(dwByLevelID);
	if (!pLevel) {ASSERTP(false, "Failed to retrieve level."); return;}
	pLevel->Rooms.GetIDs(LevelRoomIDs);
	delete pLevel;

	//Each iteration gets a saved game ID and puts in membership list.
	DWORD dwRoomID, dwPlayerID;
	for (DWORD dwSavedGameI = 0L; dwSavedGameI < dwSavedGameCount; ++dwSavedGameI)
	{
		dwRoomID = (DWORD) p_RoomID(SavedGamesView[dwSavedGameI]);
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
		if (LevelRoomIDs.IsIDInList(dwRoomID) && 
				(!this->dwFilterByPlayerID || dwPlayerID == this->dwFilterByPlayerID) &&
				(this->bLoadHidden || p_IsHidden( SavedGamesView[dwSavedGameI] ) == 0))
			this->MembershipIDs.Add(p_SavedGameID(SavedGamesView[dwSavedGameI]) );
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByHold(const DWORD dwByHoldID)
//Loads membership list from saved games in a specified level,
//and for specified player, if any.
{
	c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameCount = SavedGamesView.GetSize();

	//Store IDs of all the rooms in the hold levels.
	CIDList HoldRoomIDs;
	CIDList LevelRoomIDs;
	CDbHold *pHold = g_pTheDB->Holds.GetByID(dwByHoldID);
	if (!pHold) {ASSERTP(false, "Failed to retrieve hold."); return;}
	CDbLevel *pLevel = pHold->Levels.GetFirst();
	while (pLevel)
	{
		pLevel->Rooms.GetIDs(LevelRoomIDs);
		HoldRoomIDs += LevelRoomIDs;

		delete pLevel;
		pLevel = pHold->Levels.GetNext();
	}
	delete pHold;
	
	//Each iteration gets a saved game ID and puts in membership list.
	DWORD dwRoomID, dwPlayerID;
	for (DWORD dwSavedGameI = 0L; dwSavedGameI < dwSavedGameCount; ++dwSavedGameI)
	{
		dwRoomID = (DWORD) p_RoomID(SavedGamesView[dwSavedGameI]);
		dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGameI] );
		if (HoldRoomIDs.IsIDInList(dwRoomID) && 
				(!this->dwFilterByPlayerID || dwPlayerID == this->dwFilterByPlayerID) &&
				(this->bLoadHidden || p_IsHidden( SavedGamesView[dwSavedGameI] ) == 0))
			this->MembershipIDs.Add(p_SavedGameID( SavedGamesView[dwSavedGameI] ));
	}
}

// $Log: DbSavedGames.cpp,v $
// Revision 1.61  2005/03/15 21:51:12  mrimer
// Fixed memory leaks.
//
// Revision 1.60  2003/10/20 17:49:03  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.59  2003/10/07 21:10:35  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.58  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.57  2003/08/20 22:13:37  mrimer
// Revised saved game handling.
//
// Revision 1.56  2003/08/09 18:09:48  mrimer
// Fixed bug: Replaced assertions with more robust code catching DB row lookup failures to prevent crashes with corrutped data.
//
// Revision 1.55  2003/07/31 01:23:32  mrimer
// Fixed bug: possible duplicated saved games in export.
//
// Revision 1.54  2003/07/19 21:17:50  mrimer
// Fixed bug: incorrectly marking saved games on export.
//
// Revision 1.53  2003/07/19 02:32:55  mrimer
// Made export code more robust and maintainable.
//
// Revision 1.52  2003/07/11 05:44:48  mrimer
// Fixed a DB memory leak.
//
// Revision 1.51  2003/07/09 21:29:41  mrimer
// Made deletion routine more robust.
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.50  2003/07/07 23:43:38  mrimer
// Removed erroneous call to filter by the current player.
//
// Revision 1.49  2003/07/01 20:22:21  mrimer
// Fixed a DB access bug.
//
// Revision 1.48  2003/06/26 17:32:14  mrimer
// Ignore importing records for non-existant holds.  Fixed some bugs.
// FindByContinueLatest() now filters on valid room ID.  Added GetHoldIDofSavedGame().
//
// Revision 1.47  2003/06/24 20:12:10  mrimer
// Augmented filtering to optionally include hidden saved games.
// Fixed some bugs relating to this (in export and player deletion).
//
// Revision 1.46  2003/06/22 05:57:27  mrimer
// Fixed a bug.
//
// Revision 1.45  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.44  2003/06/12 21:43:42  mrimer
// Fixed a bug in command export.
//
// Revision 1.43  2003/06/03 06:21:21  mrimer
// Now calls ResetMembership() on DB update.
//
// Revision 1.42  2003/05/25 22:46:28  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.41  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.40  2003/05/22 23:39:02  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.39  2003/05/20 18:12:42  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.38  2003/05/08 22:01:08  mrimer
// Replaced local CDb instances with a pointer to global instance.
//
// Revision 1.37  2003/05/03 23:29:23  mrimer
// Added support for a new save game type: ST_EndHold, that gets saved when a
// player exits a hold, to mark that they have completed it.
//
// Revision 1.36  2003/04/28 22:17:34  mrimer
// Fixed a bug.
//
// Revision 1.35  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.34  2003/02/24 17:06:33  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.33  2002/12/22 01:55:50  mrimer
// Added XML import/export support.
// Added FindByContinueLatest().
// Revised filtering methods to support multiple players and holds.
//
// Revision 1.32  2002/11/22 22:00:21  mrimer
// Fixed a subtle bug with membership loading.
//
// Revision 1.31  2002/11/22 02:01:06  mrimer
// Added DeleteContinuesForPlayer().  Revised FindByContinue() to add continue saved game if none exists.
//
// Revision 1.30  2002/11/18 18:40:13  mrimer
// Fixed a bug.
//
// Revision 1.29  2002/11/14 19:18:53  mrimer
// Added FilterByPlayer support.
// Moved member initialization into constructor initialization list.
// Made some parameters const.
//
// Revision 1.28  2002/10/22 05:28:08  mrimer
// Revised includes.
//
// Revision 1.27  2002/06/21 03:12:39  erikh2000
// Fixed bug so that new saved games always have a fresh "Created" date.
//
// Revision 1.26  2002/06/16 22:04:43  erikh2000
// Changed membership-loading routines to not add hidden saved games.
//
// Revision 1.25  2002/06/16 06:21:52  erikh2000
// Added method to delete a saved game.
//
// Revision 1.24  2002/06/15 18:29:54  erikh2000
// Changed code so that storage is not committed after each database write.
//
// Revision 1.23  2002/06/09 06:18:03  erikh2000
// Added new PlayerID field.
//
// Revision 1.22  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.21  2002/05/17 09:17:41  erikh2000
// Saved game membership used for enumeration now won't contain demo or continue saved games.
//
// Revision 1.20  2002/05/15 01:20:58  erikh2000
// Fixed a problem causing demos to show up in saved games enumerations.
//
// Revision 1.19  2002/05/12 03:08:58  erikh2000
// Fixed some bugs in routines that find the latest saved game.
//
// Revision 1.18  2002/05/10 22:32:15  erikh2000
// Added more ways to find saved games using DbSavedGames.
//
// Revision 1.17  2002/04/28 23:50:03  erikh2000
// Class now supports enumeration saved games using GetFirst() and GetNexT() methods.  Saved games can be filtered by hold, level, and room.
//
// Revision 1.16  2002/03/30 21:52:17  erikh2000
// Fixed problem with commands not being cleared in CDbSavedGame::Clear().
//
// Revision 1.15  2002/03/17 23:03:48  erikh2000
// Changed c4_Storage::GetAs() call to View() to avoid changing the viewdef.
//
// Revision 1.14  2002/03/04 22:22:17  erikh2000
// Moved T_CHECKPOINT to opaque layer.
//
// Revision 1.13  2002/02/28 04:52:29  erikh2000
// Added CDbSavedGames::FindByRoomBegin(), CDbSavedGames::FindByLevelBegin(), and CDbSavedGames::FindByCheckpoint().
// Changed CDbSavedGame::Load(), UpdateNew(), and UpdateExisting() to access new "Type" field of "SavedGames" table.
//
// Revision 1.12  2002/02/10 02:35:58  erikh2000
// Fixed bad return value check in CDbSavedGame::UpdateExisting().
// Changed CDbSavedGame::Load() to fail for placeholder record.
// Moved CDbSavedGame::wTurnsTaken, wX, wY, wO, bIsPlacingMimic, wMimicCursorX, wMimicCursorY, and bIsVisible to CCurrentGame.
// Removed references to deleted SavedGames properties.
//
// Revision 1.11  2002/02/08 23:20:27  erikh2000
// Added #include <list> to fix compile error.
//
// Revision 1.10  2002/02/07 22:34:30  erikh2000
// Wrote CDbSavedGame::Update().
//
// Revision 1.9  2001/12/08 01:42:54  erikh2000
// Updated CDbSavedGame::Load() to read Commands field into member var.
//
// Revision 1.8  2001/11/08 11:34:07  erikh2000
// Added wMimicCursorX, wMimicCursorY, and bIsPlacingMimic members and code to load them.  (Committed on behalf of jpburford.)
//
// Revision 1.7  2001/11/03 20:16:57  erikh2000
// Removed OnPlot() and OnLoad() references.
//
// Revision 1.6  2001/10/27 04:41:59  erikh2000
// Added SetOnLoad calls.
//
// Revision 1.5  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.4  2001/10/21 00:28:53  erikh2000
// Added CDbSavedGame::Update() stub.
//
// Revision 1.3  2001/10/20 05:41:23  erikh2000
// Removed tile image references.
//
// Revision 1.2  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.1.1.1  2001/10/01 22:20:13  erikh2000
// Initial check-in.
//
