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

//DbXML.cpp
//Implementation of CDbXML.
//
//Used for importing/exporting records from the DB to XML files.

#ifdef WIN32
#	include <windows.h> //Should be first include.
#endif

#include "DbXML.h"
#include "CurrentGame.h"
#include "GameConstants.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/StretchyBuffer.h>
#include <BackEndLib/Ports.h>

#include <zlib.h>

#include <stdio.h>

//Static vars
CImportInfo CDbXML::info;

vector <CDbBase*> CDbXML::dbRecordStack;
vector <CDbBase*> CDbXML::dbImportedRecords;
vector <VIEWTYPE> CDbXML::dbRecordTypes;
vector <bool>  CDbXML::SaveRecord;
vector <VIEWPROPTYPE> CDbXML::vpCurrentType;

//Local vars
bool bImportComplete = false;

static UCHAR *decodedBuf = NULL;
static char *buf = NULL;
static ULONG decodedSize = 0;

//
//CDbXML private methods.
//

//*****************************************************************************
CDbBase* CDbXML::GetNewRecord(
//Class factory for classes derived from CDbBase.  New record will need its
//members set before it is used.
//
//Params:
	const VIEWTYPE vType)		//(in)	One of the DB record views.
//
//Returns:
//Pointer to a new instance of class derived from CDbBase.
{
	switch (vType)
	{
		//These are the only classes we are considering.
		case V_Demos:
			return g_pTheDB->Demos.GetNew();
		case V_Holds:
			return g_pTheDB->Holds.GetNew();
		case V_Levels:
			return g_pTheDB->Levels.GetNew();
		case V_Players:
			return g_pTheDB->Players.GetNew();
		case V_Rooms:
			return g_pTheDB->Rooms.GetNew();
		case V_SavedGames:
			return g_pTheDB->SavedGames.GetNew();

		default:
			ASSERTP(false, "Unexpected view type.");
			return NULL;
	}
}

//*****************************************************************************
VIEWTYPE CDbXML::ParseViewType(const char *str)
//Returns: enumeration corresponding to string
{
	for (int vType=V_First; vType<V_Count; vType++)
		if (!strcmp(str, viewTypeStr[vType]))
			return (VIEWTYPE)vType;

	return V_Invalid;
}

//*****************************************************************************
VIEWPROPTYPE CDbXML::ParseViewpropType(const char *str)
//Returns: enumeration corresponding to string
{
	for (int vpType=VP_First; vpType<VP_Count; vpType++)
		if (!strcmp(str, viewpropTypeStr[vpType]))
			return (VIEWPROPTYPE)vpType;

	return VP_Invalid;
}

//*****************************************************************************
PROPTYPE CDbXML::ParsePropType(const char *str)
//Returns: enumeration corresponding to string
{
	for (int pType=P_First; pType<P_Count; pType++)
		if (!strcmp(str, propTypeStr[pType]))
			return (PROPTYPE)pType;

	return P_Invalid;
}

//*****************************************************************************
bool CDbXML::WasImportSuccessful()
//Returns: whether the import procedure has only parsed good (correct) data
//that might be added to the database
{
   return
      info.ImportStatus == MID_ImportSuccessful ||
//      info.ImportStatus == MID_DemoIgnored ||    //no data will be added to the DB for these
//      info.ImportStatus == MID_HoldIgnored ||
//      info.ImportStatus == MID_PlayerIgnored ||
      info.ImportStatus == MID_PlayerSavesIgnored ||
      info.ImportStatus == MID_OverwriteHoldPrompt ||
      info.ImportStatus == MID_OverwriteHoldPrompt2;
}

//*****************************************************************************
bool CDbXML::ContinueImport(const MESSAGE_ID status)
//Returns: whether the import procedure should continue the current pass.
//
//It should not continue if the user is being prompted for input
//or if the data to import is being ignored.
{
   //If a diagonstic/error message has been passed in, record it.
   if (status != MID_ImportSuccessful)
      info.ImportStatus = status;

   return
      info.ImportStatus == MID_ImportSuccessful ||
      info.ImportStatus == MID_PlayerSavesIgnored;
}

//*****************************************************************************
void CDbXML::StartElement(
//Expat callback function
//
//Process XML start tag, and attributes.
//
//Params:
   void * /*userData*/, const char *name, const char **atts)
{
	CDbBase *pDbBase;
	int i;

	if (!ContinueImport()) return;  //ignore the rest

   //Get view type.
	const VIEWTYPE vType = ParseViewType(name);
	if (vType != V_Invalid)
	{
		//Create new object (record) to insert into DB.
		pDbBase = GetNewRecord(vType);
		bool bSaveRecord = true;
      MESSAGE_ID status;

		//Set members.
		for (i = 0; atts[i]; i += 2) {
			const PROPTYPE pType = ParsePropType(atts[i]);
			if (pType == P_Invalid)
			{
				//Invalid tag -- Fail import
				info.ImportStatus = MID_FileCorrupted;
            delete pDbBase;
				return;
			}
			status = pDbBase->SetProperty(pType, (char* const)atts[i + 1], info, bSaveRecord);
			if (!ContinueImport(status))
         {
            delete pDbBase;
				return;
         }
		}

		SaveRecord.push_back(bSaveRecord);
		dbRecordStack.push_back(pDbBase);
	} else {
		//Create object member (sub-record).
		const VIEWPROPTYPE vpType = ParseViewpropType(name);
		switch (vpType)
		{
			case VP_Monsters:
			case VP_OrbAgents:
			case VP_Orbs:
			case VP_Scrolls:
			case VP_Exits:
         {
				//Notify sub-view that members will be set.
				MESSAGE_ID status = (dbRecordStack.back())->SetProperty(vpType,
                  P_Start, NULL, info);
				if (!ContinueImport(status))
					return;

				//Set members.
				for (i = 0; atts[i]; i += 2) {
					const PROPTYPE pType = ParsePropType(atts[i]);
					status = (dbRecordStack.back())->SetProperty(vpType, pType,
							(char* const)atts[i + 1], info);
					if (!ContinueImport(status))
						return;
				}
         }
				break;

			case VP_Invalid:
				if (!strcmp(szDROD,name))  //ignore top-level header
				{
					//If version number follows the DROD header, it means this
					//file is from version 2.0 or later, and can't be imported.
					if (atts[0])
						info.ImportStatus = MID_CantImportLaterVersion;
					return;
				}
				//no break
			default:
				//Invalid tag -- Fail import
				info.ImportStatus = MID_FileCorrupted;
				return;
		}
		vpCurrentType.push_back(vpType);
	}
}

//*****************************************************************************
void CDbXML::InElement(
//Expat callback function.
//
//Process text between XML tags.
//
//Params:
   void* /*userData*/, const XML_Char* /*s*/, int /*len*/)
{
   //nothing to handle here
}

//*****************************************************************************
void CDbXML::EndElement(
//Expat callback function
//
//Process XML end tag.
//
//Params:
   void* /*userData*/, const char* name)
{
	if (!ContinueImport()) return;  //ignore the rest

   //Verify matching tags.
	if (vpCurrentType.size() > 0)
	{
		//End of sub-view.
		const VIEWPROPTYPE vpType = ParseViewpropType(name);
		if (vpType != vpCurrentType.back())
		{
			//Mismatched tags -- Fail import
			info.ImportStatus = MID_FileCorrupted;
			return;
		}

		//Finish viewprop handling.
		switch (vpType)
		{
			case VP_Monsters:
			case VP_OrbAgents:
			case VP_Orbs:
			case VP_Scrolls:
			case VP_Exits:
			{
				const MESSAGE_ID status =
						(dbRecordStack.back())->SetProperty(vpType, P_End, NULL, info);
				if (!ContinueImport(status))
					return;
				break;
			}
			default:
				//Invalid tag -- Fail import
				info.ImportStatus = MID_FileCorrupted;
				return;
		}
		vpCurrentType.pop_back();
	} else {
		//End of record.
		const VIEWTYPE vType = ParseViewType(name);
		if (vType == V_Invalid)
		{
			if (!strcmp(szDROD,name))	//ignore top-level header
         {
            bImportComplete = true;
				return;
         }
			//Bad tag -- Fail import
			info.ImportStatus = MID_FileCorrupted;
			return;
		} else {
			//Remove record object from the stack.
			CDbBase *pDbBase = dbRecordStack.back();
			dbRecordStack.pop_back();
			if (!pDbBase)
			{
				//Unmatched tags -- Fail import
				info.ImportStatus = MID_FileCorrupted;
				return;
			} else {
				if (SaveRecord.back())
				{
					//Save for post-processing and final DB update.
					dbImportedRecords.push_back(pDbBase);
					dbRecordTypes.push_back(vType);
				} else {
					delete pDbBase;
				}
			}
			SaveRecord.pop_back();
		}
	}
}

//
//CDbXML private methods.
//

//*****************************************************************************
void CDbXML::UpdateLocalIDs()
//For some imported records, old local IDs couldn't be resolved.
//Once all records have been read in, this method is called to transform
//remaining old local IDs to new local IDs.
{
	CDbBase *pDbBase;
	PrimaryKeyMap::iterator localID;

   CDb::FreezeTimeStamps(true);

   while (dbImportedRecords.size() > 0)
	{
		//Process one record.
		pDbBase = dbImportedRecords.back();
		const VIEWTYPE vType = dbRecordTypes.back();
		dbImportedRecords.pop_back();
		dbRecordTypes.pop_back();

		switch (vType)
		{
			case V_Holds:
			{
 				CDbHold *pHold = DYN_CAST(CDbHold *, CDbBase *, pDbBase);
				ASSERT(pHold);

				if (0 == (time_t)pHold->LastUpdated)
				{
					//Implies this is only a hold reference -- it should already
					//exist in the DB and shouldn't be updated.
					delete pDbBase;
					continue;
				}

				//Update first level ID.
				localID = info.LevelIDMap.find(pHold->dwLevelID);
				if (localID == info.LevelIDMap.end())
				{
					//Entrance Level ID not found -- repair hold after levels are handled.
               pHold->dwLevelID = 0;
					info.dwRepairHoldID = pHold->dwHoldID;
            } else {
				   pHold->dwLevelID = localID->second;
            }
				break;
			}
			case V_Levels:
			{
 				CDbLevel *pLevel = DYN_CAST(CDbLevel *, CDbBase *, pDbBase);
				ASSERT(pLevel);

				if (!pLevel->dwPlayerID)
				{
					//Implies this is only a level reference -- it should already
					//exist in the DB and shouldn't be updated.
					delete pDbBase;
					continue;
				}

				//Update first room ID.
				localID = info.RoomIDMap.find(pLevel->dwRoomID);
				if (localID == info.RoomIDMap.end())
				{
					//ID not found -- Fail import
					info.ImportStatus = MID_FileCorrupted;
					return;
				}
				pLevel->dwRoomID = localID->second;
				break;
			}

			case V_Players:
			{
            //Update highlight demo IDs in player settings.
 				CDbPlayer *pPlayer = DYN_CAST(CDbPlayer *, CDbBase *, pDbBase);
				ASSERT(pPlayer);
            UpdateHighlightDemoIDs(pPlayer);
            break;
         }

			case V_Rooms:
			{
 				CDbRoom *pRoom = DYN_CAST(CDbRoom *, CDbBase *, pDbBase);
				ASSERT(pRoom);

				if (!pRoom->pszOSquares)
				{
					//Implies this is only a room reference -- it should already
					//exist in the DB and shouldn't be updated.
					delete pDbBase;
					continue;
				}

				//Update exit IDs.
				const UINT wCount = pRoom->Exits.size();
				for (UINT wIndex=0; wIndex<wCount; ++wIndex)
				{
               if (pRoom->Exits[wIndex]->dwLevelID != 0) //ignore "Finish Hold" exits
               {
					   localID = info.LevelIDMap.find(pRoom->Exits[wIndex]->dwLevelID);
					   if (localID == info.LevelIDMap.end())
					   {
						   //ID not found -- Fail import
						   info.ImportStatus = MID_LevelNotFound;
						   return;
					   }
					   pRoom->Exits[wIndex]->dwLevelID = localID->second;
               }
				}
				break;
			}

			case V_SavedGames:
			{
 				CDbSavedGame *pSavedGame = DYN_CAST(CDbSavedGame *, CDbBase *, pDbBase);
				ASSERT(pSavedGame);

				//Update IDs for ExploredRooms and ConqueredRooms.
				IDNODE *pIDNode = pSavedGame->ExploredRooms.Get(0);
				while (pIDNode)
				{
					localID = info.RoomIDMap.find(pIDNode->dwID);
					if (localID == info.RoomIDMap.end())
					{
						//ID not found -- Fail import
						info.ImportStatus = MID_FileCorrupted;
						return;
					}
					pIDNode->dwID = localID->second;
					pIDNode = pIDNode->pNext;
				}
				pIDNode = pSavedGame->ConqueredRooms.Get(0);
				while (pIDNode)
				{
					localID = info.RoomIDMap.find(pIDNode->dwID);
					if (localID == info.RoomIDMap.end())
					{
						//ID not found -- Fail import
						info.ImportStatus = MID_FileCorrupted;
						return;
					}
					pIDNode->dwID = localID->second;
					pIDNode = pIDNode->pNext;
				}
				break;
			}

         case V_Demos:
				break;	//other types need no fix-ups

         default:
            ASSERTP(false, "Unexpected view type (2).");
            break;
		}

		//Save any changes.
		pDbBase->Update();
		delete pDbBase;
	}

   //Attempt to repair corrupted data.
   //(Might be needed for data from older versions.)
   if (info.dwRepairHoldID)
   {
      CDbHold *pHold = g_pTheDB->Holds.GetByID(info.dwRepairHoldID);
      ASSERT(pHold);
      if (pHold->Repair())
      {
         pHold->Update();
      } else {
         info.ImportStatus = MID_LevelNotFound;
         return;
      }
   }

   CDb::FreezeTimeStamps(false);
}

//*****************************************************************************
void CDbXML::UpdateHighlightDemoIDs(
//The player settings contain a set of local highlight demo IDs required
//to show scenes in the game end sequence of Dugan's Dungeon.
//Since imported demos have been assigned new IDs, the stored IDs must be updated.
//
//Params:
   CDbPlayer *pPlayer)  //(in) player to update highlight demo IDs for
{
	PrimaryKeyMap::iterator localID;
   const DWORD dwNumRooms = info.highlightRoomIDs.GetSize();
   for (DWORD dwIndex=0; dwIndex<dwNumRooms; ++dwIndex)
   {
	   //Retrieve old local demo ID from player settings for each room.
      const DWORD dwRoomID = info.highlightRoomIDs.Get(dwIndex)->dwID;
		char szRoomID[11];
		string strVarName = "hd";
		strVarName += _ltoa(dwRoomID, szRoomID, 10);

	   const DWORD dwDemoID = pPlayer->Settings.GetVar(strVarName.c_str(), (DWORD)0L);
      if (dwDemoID)
      {
         //A highlight demo has been saved for this room.  Update its ID.
			localID = info.DemoIDMap.find(dwDemoID);
			if (localID == info.DemoIDMap.end())
			{
            //!!ID not found --
            //should we fail the import if the IDs in the player settings are corrupted?
         } else {
            pPlayer->Settings.SetVar(strVarName.c_str(), localID->second);
         }
      }
   }
}

//*****************************************************************************
void CDbXML::CreateLevelStartSaves(
//Create a level start saved game for each player-level listed.
//This is done on import when a hold has being overwritten and saved games have
//been erased.  This will re-create all the LevelStart saves that players
//had for the hold based on the new hold version.  Other saved positions are not
//saved as it cannot be guaranteed that they are still valid for the modified hold.
//
//Params:
	CImportInfo &info)	   //(in) Import info
{
   const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();

   CCueEvents Ignored;
   for (PlayerLevelIDList::iterator playerIter=info.playerLevelIDs.begin();
         playerIter!=info.playerLevelIDs.end(); ++playerIter)
   {
      g_pTheDB->SetPlayerID((*playerIter)->dwPlayerID);
      PrimaryKeyMap::iterator localHoldID = info.HoldIDMap.find((*playerIter)->dwHoldID);
		if (localHoldID == info.HoldIDMap.end()) continue;
      CDbHold *pHold = g_pTheDB->Holds.GetByID(localHoldID->second);
      ASSERT(pHold); //this hold was just re-imported
      if (!pHold) continue;
      DWORD dwLevelID = pHold->dwLevelID;
      for (vector<DWORD>::iterator iter=(*playerIter)->levelIndices.begin();
            iter!=(*playerIter)->levelIndices.end(); ++iter)
      {
         dwLevelID = CDbHolds::GetLevelIDAtIndex(*iter, pHold->dwHoldID);
			if (dwLevelID != 0)
         {
            //This level still remains in the new hold version.
            //Re-create the LevelStart saved game for this player.
   	      CCurrentGame *pCCG = g_pTheDB->GetImportCurrentGame();
            pCCG->LoadFromLevel(dwLevelID, Ignored);
            delete pCCG;
         }
      }

      //Re-create EndHold save.
      if ((*playerIter)->bEndHoldSave)
      {
   	   CCurrentGame *pCCG = g_pTheDB->GetImportCurrentGame();
         pCCG->LoadFromLevel(dwLevelID, Ignored);
	      pCCG->eType = ST_EndHold;
	      pCCG->bIsHidden = true;
	      pCCG->Update();
         delete pCCG;
      }
      delete pHold;
   }

   g_pTheDB->SetPlayerID(dwCurrentPlayerID);
}

//*****************************************************************************
UINT CDbXML::GetLevelStartIDs(
//Compile a list of players, each with a list of level GUIDs for all levels in
//the hold for which they have level start saved games recorded.  Also record
//whether they have an EndHold saved game.
//
//This is done on import when a hold is being overwritten and saved games are
//being erased.  This is the simplest way to save as much progress as possible
//without the import process becoming too complicated.
//
//Returns:  0 = no players have any level start saved games
//          1 = only the current player has  " "
//          2 = other players have           " "
//
//Params:
   const DWORD dwHoldID,   //(in) hold to find GUIDs for
   const DWORD dwMapHoldID,//(in) hold ID as listed in import
	CImportInfo &info)	   //(out) Import info
{
   bool bCurrentPlayerAffected = false, bOtherPlayersAffected = false;
   const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();

   //Compile list of all level start room IDs in hold.
   CDb db;
   CIDList LevelStartRoomIDs;
	CDbHold *pHold = g_pTheDB->Holds.GetByID(dwHoldID);
   ASSERT(pHold);
	CDbLevel *pLevel = pHold->Levels.GetFirst();
	while (pLevel)
	{
		LevelStartRoomIDs.Add(pLevel->dwRoomID);

		delete pLevel;
		pLevel = pHold->Levels.GetNext();
	}
	delete pHold;

   //Compile list of all players.
   CIDList localPlayerIDs;
   db.Players.FilterByLocal();
   db.Players.GetIDs(localPlayerIDs);

   c4_View SavedGamesView = GetView(ViewTypeStr(V_SavedGames));
	c4_View RoomsView = GetView(ViewTypeStr(V_Rooms));
	c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	const DWORD dwSavedGamesCount = SavedGamesView.GetSize();
	SAVETYPE eSaveType;
	DWORD dwPlayerID, dwRoomID;
   IDNODE *pPlayerID = localPlayerIDs.Get(0);
   while (pPlayerID)
   {
      //For each player, find all the level start saved games in this hold.
      bool bFound = false;
      PlayerLevelIDs *pPlayerIDs = new PlayerLevelIDs;
      pPlayerIDs->bEndHoldSave = false;
	   for (DWORD dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	   {
		   eSaveType = (SAVETYPE) (int) p_Type( SavedGamesView[dwSavedGamesI] );
		   dwPlayerID = (DWORD) p_PlayerID( SavedGamesView[dwSavedGamesI] );
		   dwRoomID = (DWORD) p_RoomID( SavedGamesView[dwSavedGamesI] );
		   if (eSaveType == ST_LevelBegin && dwPlayerID == pPlayerID->dwID &&
               LevelStartRoomIDs.IsIDInList(dwRoomID))
		   {
            //Found one.
            bFound = true;
		      const DWORD dwRoomI = LookupRowByPrimaryKey(dwRoomID, p_RoomID,
				      RoomsView);
            if (dwRoomI == ROW_NO_MATCH) {ASSERTP(false, "Room lookup failed."); continue;}
            const DWORD dwLevelID = (DWORD) p_LevelID( RoomsView[dwRoomI] );
		      const DWORD dwLevelI = LookupRowByPrimaryKey(dwLevelID, p_LevelID,
				      LevelsView);
            if (dwLevelI == ROW_NO_MATCH) {ASSERTP(false, "Level lookup failed."); continue;}
            const DWORD dwLevelIndex = (DWORD) p_GID_LevelIndex( LevelsView[dwLevelI] );
            pPlayerIDs->levelIndices.push_back(dwLevelIndex);
         } else if (eSaveType == ST_EndHold && dwPlayerID == pPlayerID->dwID)
         {
            //If this EndHold save is for this hold, retain it.
		      const DWORD dwRoomI = LookupRowByPrimaryKey(dwRoomID, p_RoomID,
				      RoomsView);
            if (dwRoomI == ROW_NO_MATCH) {ASSERTP(false, "Room lookup failed.(2)"); continue;}
            const DWORD dwLevelID = (DWORD) p_LevelID( RoomsView[dwRoomI] );
		      const DWORD dwLevelI = LookupRowByPrimaryKey(dwLevelID, p_LevelID,
				      LevelsView);
            if (dwLevelI == ROW_NO_MATCH) {ASSERTP(false, "Level lookup failed.(2)"); continue;}
            const DWORD dwLevelHoldID = (DWORD) p_HoldID( LevelsView[dwLevelI] );
            if (dwLevelHoldID == dwHoldID)
            {
               //Retain EndHold saved game.
               bFound = true;
               pPlayerIDs->bEndHoldSave = true;
            }
         }
      }
      if (bFound)
      {
         pPlayerIDs->dwPlayerID = pPlayerID->dwID;
         pPlayerIDs->dwHoldID = dwMapHoldID;
         info.playerLevelIDs.push_back(pPlayerIDs);
         if (dwCurrentPlayerID == pPlayerID->dwID)
            bCurrentPlayerAffected = true;
         else
            bOtherPlayersAffected = true;
      }
      else
         delete pPlayerIDs;

      pPlayerID = pPlayerID->pNext;
   }

   if (bOtherPlayersAffected) return 2;
   if (bCurrentPlayerAffected) return 1;
   return 0;
}

//*****************************************************************************
void CDbXML::ModifyLevelStartSaves(
//Remove the imported player's demos and saved games for specified holds,
//retaining only updated versions of their LevelStart and EndHold saved games.
//
//Params:
	CImportInfo &info)	   //(out) Import info
{
   CDb db;
   CCueEvents Ignored;

   //Compile a list of all levels in the holds in question for which the
   //imported player has a level start saved game.
   ASSERT(info.dwPlayerImportedID);
   const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
   g_pTheDB->SetPlayerID(info.dwPlayerImportedID);

   db.Demos.FilterByPlayer(info.dwPlayerImportedID);
   db.SavedGames.FilterByPlayer(info.dwPlayerImportedID);
   db.Demos.FindHiddens(true);
   db.SavedGames.FindHiddens(true);

   CIDList levelIDs, savedGameIDs, retainLevelIDs;
   for (vector<DWORD>::iterator iter=info.localHoldIDs.begin();
         iter!=info.localHoldIDs.end(); ++iter)
   {
      bool bEndHold = false;
      db.Levels.FilterBy(*iter);
      db.Levels.GetIDs(levelIDs);
      IDNODE *pLevelID = levelIDs.Get(0);
      while (pLevelID)
      {
         db.SavedGames.FilterByLevel(pLevelID->dwID);
         db.SavedGames.GetIDs(savedGameIDs);
         IDNODE *pSavedGameID = savedGameIDs.Get(0);
         while (pSavedGameID)
         {
            CDbSavedGame *pSavedGame = db.SavedGames.GetByID(pSavedGameID->dwID);
            if (pSavedGame->eType == ST_LevelBegin)
               retainLevelIDs.Add(pLevelID->dwID);
            else if (pSavedGame->eType == ST_EndHold)
               bEndHold = true;
            delete pSavedGame;
            pSavedGameID = pSavedGameID->pNext;
         }
         pLevelID = pLevelID->pNext;
      }

      //Delete all player's demos and saved games for this hold.
      CIDList demoIDs;
      db.Demos.FilterByHold(*iter);
      db.Demos.GetIDs(demoIDs);
      IDNODE *pSeekID = demoIDs.Get(0);
      while (pSeekID)
      {
         db.Demos.Delete(pSeekID->dwID);
         pSeekID = pSeekID->pNext;
      }

      db.SavedGames.FilterByHold(*iter);
      db.SavedGames.GetIDs(savedGameIDs);
      pSeekID = savedGameIDs.Get(0);
      while (pSeekID)
      {
         db.SavedGames.Delete(pSeekID->dwID);
         pSeekID = pSeekID->pNext;
      }

      //Create new instances of LevelStart saves.
      pSeekID = retainLevelIDs.Get(0);
      const DWORD dwFirstLevelID = pSeekID->dwID;
      while (pSeekID)
      {
   	   CCurrentGame *pCCG = g_pTheDB->GetImportCurrentGame();
         pCCG->LoadFromLevel(pSeekID->dwID, Ignored);
         delete pCCG;
         pSeekID = pSeekID->pNext;
      }

      //Create EndHold save.
      if (bEndHold)
      {
   	   CCurrentGame *pCCG = g_pTheDB->GetImportCurrentGame();
         pCCG->LoadFromLevel(dwFirstLevelID, Ignored);
	      pCCG->eType = ST_EndHold;
	      pCCG->bIsHidden = true;
	      pCCG->Update();
         delete pCCG;
      }
   }

   g_pTheDB->SetPlayerID(dwCurrentPlayerID);
}

//*****************************************************************************
void CDbXML::CleanUp()
{
	delete[] decodedBuf;
   decodedBuf = NULL;
   buf = NULL;
   decodedSize = 0;
}

//*****************************************************************************
MESSAGE_ID CDbXML::ImportXML(
//Import an XML file into one or more tables.
//
//Params:
	const WCHAR *wszFilename)	//(in)	XML filename, not including extension or path.
               //A NULL filename indicates a file has already been read in and
               //this previous file must be reparsed.
//
//Returns:
//Message ID giving the concluding status of the operation, i.e.
//	MID_ImportSuccessful (etc) if it worked,
//	MID_* (giving the reason for failure) if not.
//If not successful, then no changes are made to the database.
{
	//Metakit API docs are at:
	//  http://www.equi4.com/metakit/api/

	//IMPORT XML FORMAT
	//
	//The XML file will be legit XML 1.0 data.  Expat is being used to parse it.
	//Details of the expected format are determined by ExportXML().
	//See derived classes of CDbBase for reference.
	//
	//Note that the XML file may contain records for more than one table.  The
	//top-level tag in the hierarchy (under DROD) will indicate which table to
	//insert records into.  For example, the following XML would indicate to
	//insert one record into Players, two records into Saved Games, and
	//two records into Demos (some fields have been left out for brevity):
	//
	//	<?xml version="1.0" encoding="ISO8859-1" ?>
	//	<DROD>
	//  <Players PlayerID='1' Name='Bubba X' />
	//
	//  <SavedGames SavedGameID='12' Commands='CQAAAExhbmd1Y...' />
	//
	//  <SavedGames SavedGameID='13' Commands='TXVzaWMABAAAAAEAAAA...' />
	//
	//  <Demos DemoID='1' PlayerID='1' SavedGameID='12' />
	//
	//  <Demos DemoID='2' PlayerID='1' SavedGameID='13' />
	//	</DROD>
	//
	//The second-level tags, like "PlayerID" and "Name" specify the names of
	//fields that will receive values in the inserted record.  If an
	//XML-specified table (AKA "viewdef") does not exist in the database, or
	//any XML-specified field does not exist in the table, then the import
	//should fail.

	//FAILING THE IMPORT
	//
	//When the import fails, all changes to data should roll back, leaving the
	//database in its initial state.

	//REMAPPING PRIMARY AND FOREIGN KEYS
	//
	//Look back at the previous XML example, and you may see that there is a
	//potential problem with importing the data as-is.  The database probably
	//already has a Players record with a PlayerID of 1.  Same goes for demo IDs
	//and saved game IDs.
	//
	//The new Players record will need a new PlayerID value that is not already
	//used by any record currently in the Players table.  The Demos records need
	//to have their PlayerID values updated to match the new PlayerID value.
	//
	//Terminology:
	//Primary key - A primary key is a field value that uniquely identifies a record
	//              within a table.  Within a table, each record's primary key must be
	//              unique against all the other records.  "PlayerID" is a primary
	//				key within the "Players" table.  "DemoID" is a primary key within
	//              the "Demos" table.
	//Foreign key - A foreign key is a field value that identifies a record from a
	//              foreign table (a table other than the one that the foreign key
	//              appears within).  "PlayerID" is a foreign key within the "Demos"
	//              table.  In the above XML example, it indicates that the player with
	//				PlayerID of 1 is the author of the demos with demo IDs of 1 and 2.
	//				"PlayerID" is a foreign key within the "Demos" table (but not the
	//				"Players" table).  "SavedGameID" is also a foreign key within the
	//				"Demos" table.
	//
	//Metakit has no internal designation for primary or foreign keys.  You can
	//determine primary and foreign keys at run-time using the following rules:
	//
	//1. If the field name ends in "ID" and is the first field in a table, that field
	//   is the primary key for the table.
	//2. If the field name ends in "ID" and is the second or later field in a table,
	//   that field is a foreign key for the table.
	//
	//A table may not have more than one primary key (implied from above rules).  A
	//table does not necessarily have a primary or foreign keys.
	//
	//You can always expect an XML file to contain a complete
	//set of records for the import.  The above XML example is a complete set.
	//It would not be a complete set if the two SavedGames records were left
	//out, because the Demos records would have two meaningless "SavedGameID"
	//foreign keys that are left without corresponding SavedGames records.
	//
	//To construct GUIDs, fields are used to uniquely identify records on import.
	//If a record's GUID matches the GUID of a local record of that type,
	//the data will either be merged (for Players) or replace currently existing
	//data (for Holds, Levels, and Rooms).

   //If an import is interrupted in the middle by a request for user input,
   //the XML buffer read in from a file will be retained so it can be resumed
   //later without having to read in the file and decompress the data again.
   if (wszFilename)
   {
      ASSERT(!decodedBuf);
      ASSERT(!buf);
      ASSERT(!decodedSize);

	   // Read the compressed data stream
      CStretchyBuffer buffer;
      if (!CFiles::ReadFileIntoBuffer(wszFilename, buffer))
		   return MID_FileNotFound;
	   buffer.Decode();

		// Uncompress the data stream.
      const DWORD fileSize = buffer.Size();
		decodedSize = fileSize * 15;	//about as big as the uncompressed data will ever be
		decodedBuf = new UCHAR[decodedSize+1]; //allow null-termination
		int res;
		do {
			res = uncompress(decodedBuf, &decodedSize, (BYTE*)buffer, fileSize);
			switch (res)
			{
				case Z_BUF_ERROR:
					//This wasn't enough memory to decode the data to,
					//so double the buffer size.
					decodedSize *= 2;
					delete[] decodedBuf;
					decodedBuf = new UCHAR[decodedSize+1]; //allow null-termination
					break;

				case Z_DATA_ERROR:
					delete[] decodedBuf;
					return MID_FileCorrupted;

				case Z_MEM_ERROR:
					delete[] decodedBuf;
					return MID_OutOfMemory;
			}
		} while (res != Z_OK);	//success
		//Upon successful completion, decodedSize is the actual size of the uncompressed buffer.

      //Null-terminate text-string.
	   buf = (char*)decodedBuf;
	   buf[decodedSize] = '\0';
   }
   ASSERT(decodedBuf);
   ASSERT(buf);
   ASSERT(decodedSize);

   info.ImportStatus = ImportXML(buf, decodedSize);

   //Clean up.
   if (bImportComplete)
      CleanUp();

	return info.ImportStatus;
}

//*****************************************************************************
MESSAGE_ID CDbXML::ImportXML(
//Import an XML file into one or more tables (see above method for notes).
//
//Params:
	char *buf, const ULONG size)	//(in) buffer of XML text
{
	//Ensure everything is reset.
	ASSERT(dbRecordStack.size() == 0);
	ASSERT(dbImportedRecords.size() == 0);
	ASSERT(dbRecordTypes.size() == 0);
	ASSERT(vpCurrentType.size() == 0);

   //Import init.
   info.ImportStatus = MID_ImportSuccessful;
   info.dwPlayerImportedID = info.dwHoldImportedID = 0;
   bImportComplete = false;

   //Parser init.
   XML_Parser parser = XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, CDbXML::StartElement, CDbXML::EndElement);
	XML_SetCharacterDataHandler(parser, CDbXML::InElement);

   //Parse the XML.
	if (XML_Parse(parser, buf, size, true) == XML_STATUS_ERROR)
   {
		//Some problem occured.
		char errorStr[256];
		sprintf(errorStr,
				"Import Parse Error: %s at line %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser));
		CFiles Files;
      Files.AppendErrorLog((char *)errorStr);

      //Invalidate import only if data was corrupted somewhere before the end tag.
      //If something afterwards happens to be wrong, just ignore it.
      if (!bImportComplete)
		   info.ImportStatus=MID_FileCorrupted;
	}
   XML_ParserFree(parser);

   //Confirm something was actually imported.  If not, mention this.
	if (ContinueImport())
      switch (info.typeBeingImported)
      {
         case CImportInfo::Demo: break;
         case CImportInfo::Hold:
            if (info.dwHoldImportedID == 0)
               ContinueImport(MID_HoldIgnored);
            break;
         case CImportInfo::Player:
            if (info.dwPlayerImportedID == 0)
               ContinueImport(MID_PlayerIgnored);
            break;
         default: break;
      }

	if (WasImportSuccessful())
	{
      //Only save data if we're not exiting the import to prompt the user
      //on what to do.
      if (ContinueImport())
      {
	      //Now that all the records have been read through, we can
	      //update all remaining old local keys (IDs) to new local keys.
		   UpdateLocalIDs();
         if (info.playerLevelIDs.size() > 0)
            CreateLevelStartSaves(info); //performs internal Commits
         if (info.localHoldIDs.size() > 0)
            ModifyLevelStartSaves(info); //performs internal Commits
		   Commit();
      }
      g_pTheDB->ResetMembership();
	} else {
      bImportComplete = true; //Import is impossible or ignored -- won't perform another pass
		g_pTheDB->Rollback();

		//Free any remaining DB objects.
		while (dbRecordStack.size() > 0)
		{
			CDbBase *pDbBase = dbRecordStack.back();
			delete pDbBase;
			dbRecordStack.pop_back();
		}
		while (dbImportedRecords.size() > 0)
		{
			CDbBase *pDbBase = dbImportedRecords.back();
			delete pDbBase;
			dbImportedRecords.pop_back();
			dbRecordTypes.pop_back();
		}
		while (vpCurrentType.size() > 0)
			vpCurrentType.pop_back();
	}

   //Clean up.
	ASSERT(dbRecordStack.size() == 0);
	ASSERT(dbImportedRecords.size() == 0);
	ASSERT(dbRecordTypes.size() == 0);
	ASSERT(vpCurrentType.size() == 0);

	info.Clear(!bImportComplete);

   return info.ImportStatus;
}

//*****************************************************************************
bool CDbXML::ExportXML(
//Export a table to an XML file.
//
//Params:
	const char *pszTableName,	//(in)	Table to export.
	c4_IntProp &propID,			//(in)	Reference to the primary key field.
	const DWORD dwPrimaryKey,	//(in)	Key to look up in that table.
	const WCHAR *wszFilename)	//(in)	XML filename, not including extension or path.
								//		File will be found at
								//		"{data path}\Export\{filename}.xml".
//
//Returns:
//True if export was successful, false if not.  If false, the export file will not
//be present.
{
	//Ensure view record exists with primary key.
	c4_View DBView = GetView(pszTableName);
	const DWORD dwIndex = LookupRowByPrimaryKey(dwPrimaryKey, propID, DBView);
   if (dwIndex == ROW_NO_MATCH) {ASSERTP(false, "Row lookup failed."); return false;}

	//Prepare refs list
	CDbRefs dbRefs;

	string text, element;

	//XML header.
	text += "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\r\n";

	//Provide dummy top-level record.
	text += "<";
   text += szDROD;
   text += ">\r\n";

	//All record types that can be exported independently are here.
	const VIEWTYPE vType = ParseViewType(pszTableName);
	switch (vType)
	{
		case V_Demos:
			element = g_pTheDB->Demos.ExportXML(dwPrimaryKey, dbRefs);
			break;
		case V_Holds:
			element = g_pTheDB->Holds.ExportXML(dwPrimaryKey, dbRefs);
			break;
		case V_Players:
			element = g_pTheDB->Players.ExportXML(dwPrimaryKey, dbRefs);
			break;
		default:
			ASSERTP(false, "Unexpected view type.(3)");
			return false;
	}

	if (element.empty()) return false;
	text += element;
	text += "</";
   text += szDROD;
   text += ">\r\n";

	// Compress the data.
	const ULONG srcLen = (uLong)(text.size() * sizeof(char));
	ULONG destLen = (ULONG) (1.01 * srcLen) + 13;
	UCHAR *dest = new UCHAR[destLen];
	const int res = compress(dest, &destLen, (const UCHAR*)text.c_str(), srcLen);
	ASSERT(res == Z_OK);
	CStretchyBuffer buffer(dest, destLen);	//no terminating null
	buffer.Encode();
	CFiles::WriteBufferToFile(wszFilename,buffer);
	delete dest;

	return true;
}

// $Log: DbXML.cpp,v $
// Revision 1.40  2005/03/15 21:45:13  mrimer
// Added import version checking.
//
// Revision 1.39  2003/10/20 17:49:03  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.38  2003/10/10 15:27:44  mrimer
// Fixed possible rare memory bug.
//
// Revision 1.37  2003/10/08 17:30:53  mrimer
// Removed assertion that's not always true.
//
// Revision 1.36  2003/10/07 21:10:35  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.35  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.34  2003/08/09 18:09:48  mrimer
// Fixed bug: Replaced assertions with more robust code catching DB row lookup failures to prevent crashes with corrutped data.
//
// Revision 1.33  2003/08/09 00:38:53  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.32  2003/08/06 01:15:42  mrimer
// Changed interface to GetLevelIDAtIndex().
//
// Revision 1.31  2003/08/01 17:26:05  mrimer
// Fixed bug: can't confirm user import choices.  Added ClearUp().
//
// Revision 1.30  2003/07/29 13:34:01  mrimer
// Added player import fix: UpdateHighlightDemoIDs().
//
// Revision 1.29  2003/07/25 22:46:43  mrimer
// If a hold or player doesn't actually get imported (i.e. is ignored), mention this fact and roll back.
//
// Revision 1.28  2003/07/25 00:06:22  mrimer
// Fixed retention of level start/end hold saved games when hold is upgraded.
// Fixed new bug: now allows importing stairs w/ destID of 0.
//
// Revision 1.27  2003/07/24 01:30:06  mrimer
// Changed room exits data structure to vector.
//
// Revision 1.26  2003/07/23 19:04:39  mrimer
// Removed a (no longer needed) kludge.
//
// Revision 1.25  2003/07/19 02:39:30  mrimer
// Made DB view access more robust and maintainable.
//
// Revision 1.24  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.23  2003/07/13 06:46:48  mrimer
// Fixed memory leak bugs in import routine.  Removed unused parameter from Export().
//
// Revision 1.22  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.21  2003/07/09 21:27:42  mrimer
// Revised import routines to be more robust: added GetLevelStartIDs(), CreateLevelStartSaves(), ModifyLevelStartSaves(), and ContinueImport().
// Attemps to repair corrupted hold records.
// Optimized import for multiple pass parsing.
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.20  2003/07/07 23:33:05  mrimer
// Added more import diagnostic messages and logic.
//
// Revision 1.19  2003/07/05 02:34:59  mrimer
// Ignore bad(?) xml after closing </drod> tag.
//
// Revision 1.18  2003/06/26 17:29:30  mrimer
// Added short-circuit handling of import process on fail.  Deleted some undeleted DB objects.
//
// Revision 1.17  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.16  2003/06/17 22:21:08  mrimer
// Modified lone demo export to reset show number (to make demo visible to importing player).
//
// Revision 1.15  2003/06/16 18:57:38  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.14  2003/06/12 21:43:05  mrimer
// Fixed some bugs.
//
// Revision 1.13  2003/05/28 23:05:10  erikh2000
// CFiles::GetDatPath() is called differently.
//
// Revision 1.12  2003/05/25 22:46:28  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.11  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.10  2003/05/22 23:39:02  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.9  2003/05/08 22:01:08  mrimer
// Replaced local CDb instances with a pointer to global instance.
//
// Revision 1.8  2003/05/06 17:42:38  mrimer
// Removed lib pragma.
//
// Revision 1.7  2003/05/04 00:22:34  mrimer
// Finished file protection.
//
// Revision 1.6  2003/04/29 11:07:07  mrimer
// Fixed various import bugs.
//
// Revision 1.5  2003/04/21 21:19:31  mrimer
// Added ZLIB implementation.
//
// Revision 1.4  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.3  2003/02/24 17:06:34  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.2  2003/01/09 22:46:11  erikh2000
// Changed absolute expat file references to relative.  Add include and lib directories in your VC options.
//
// Revision 1.1  2002/12/22 01:47:46  mrimer
// Initial check-in.
//
