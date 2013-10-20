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

//DbHolds.cpp
//Implementation of CDbHolds and CDbHold.

#include "DbHolds.h"

#include "Db.h"
#include "DBProps.h"
#include "DbXML.h"
#include "GameConstants.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Base64.h>
#include <BackEndLib/Ports.h>

//
//CDbHolds public methods.
//

//*****************************************************************************
void CDbHolds::Delete(
//Deletes records for a hold with the given ID.
//Also deletes all levels in the hold (and, by association, all rooms,
//saved games and demos).
//
//Params:
	const DWORD dwHoldID)	//(in)	ID of hold(s) to delete.
{
	ASSERT(dwHoldID);

	c4_View HoldsView = GetView(ViewTypeStr(V_Holds));
	const DWORD dwHoldRowI = LookupRowByPrimaryKey(dwHoldID, p_HoldID, HoldsView);
	if (dwHoldRowI == ROW_NO_MATCH) {ASSERTP(false, "Bad hold ID."); return;}

	//Delete all levels in hold (and their rooms, saved games and demos.)
	CIDList LevelIDs;
	IDNODE *pSeek;
   CDb db;
	db.Levels.FilterBy(dwHoldID);
	db.Levels.GetIDs(LevelIDs);
	pSeek = LevelIDs.Get(0);
	while (pSeek)
	{
		db.Levels.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}

	//Delete name and description message texts.
	const DWORD dwNameMID = p_NameMessageID( HoldsView[dwHoldRowI] );
	if (!dwNameMID) {ASSERTP(false, "Bad MID for name."); return;}
	DeleteMessage(static_cast<MESSAGE_ID>(dwNameMID));
	const DWORD dwDescriptionMID = p_DescriptionMessageID( HoldsView[dwHoldRowI] );
	if (!dwDescriptionMID) {ASSERTP(false, "Bad MID for description."); return;}
	DeleteMessage(static_cast<MESSAGE_ID>(dwDescriptionMID));

	//Delete the hold.
	HoldsView.RemoveAt(dwHoldRowI);

	//After hold object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//*****************************************************************************
string CDbHolds::ExportXML(
//Returns: string containing XML text describing hold with this ID
//				AND all levels having this HoldID
//          AND all show demos in this hold
//
//Pre-condition: dwHoldID is valid
//
//Params:
	const DWORD dwHoldID,	//(in)
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

	if (!dbRefs.IsSet(V_Holds,dwHoldID))
	{
		dbRefs.Set(V_Holds,dwHoldID);

		char dummy[32];
		CDbHold *pHold = GetByID(dwHoldID);
		ASSERT(pHold);
      if (!pHold) return str; //shouldn't happen, but just in case

		//Include corresponding GID player ref.
		str += g_pTheDB->Players.ExportXML(pHold->dwPlayerID, dbRefs, true);

		str += STARTTAG(V_Holds, P_GID_Created);
		str += LONGTOSTR((time_t)pHold->Created);
		str += PROPTAG(P_GID_PlayerID);
		str += LONGTOSTR(pHold->dwPlayerID);
		str += PROPTAG(P_LastUpdated);
		str += LONGTOSTR((time_t)pHold->LastUpdated);  //timestamp provides hold "version" info
		if (!bRef)
		{
			//Don't need any further information for a hold reference.

         //Prepare data.
			WSTRING const wNameStr = (WSTRING)pHold->NameText;
			WSTRING const wDescStr = (WSTRING)pHold->DescriptionText;
			WSTRING const wEndHoldStr = (WSTRING)pHold->EndHoldText;

			str += PROPTAG(P_NameMessage);
			str += Base64::encode(wNameStr);
			str += PROPTAG(P_DescriptionMessage);
			str += Base64::encode(wDescStr);
			str += PROPTAG(P_LevelID);
			str += LONGTOSTR(pHold->dwLevelID);
			str += PROPTAG(P_GID_NewLevelIndex);
         //if there is a level in the hold, the newLevelIndex must be > 0
         ASSERT(!pHold->dwLevelID || pHold->dwNewLevelIndex);
			str += LONGTOSTR(pHold->dwNewLevelIndex);
			str += PROPTAG(P_EditingPrivileges);
			str += LONGTOSTR((long)pHold->editingPrivileges);
			str += PROPTAG(P_EndHoldMessage);
			str += Base64::encode(wEndHoldStr);
		}
		//Put primary key last, so all message fields have been set by the
		//time Update() is called during import.
		str += PROPTAG(P_HoldID);
		str += LONGTOSTR(pHold->dwHoldID);

		if (bRef)
			str += CLOSETAG;
		else
		{
			str += CLOSESTARTTAG;

			//Export all levels in hold.
         CDb db;
			CIDList LevelIDs;
			db.Levels.FilterBy(dwHoldID);
			db.Levels.GetIDs(LevelIDs);
			const DWORD dwNumLevels = LevelIDs.GetSize();
			DWORD dwIndex;
			for (dwIndex=0; dwIndex<dwNumLevels; ++dwIndex)
			{
				str += db.Levels.ExportXML(LevelIDs.Get(dwIndex)->dwID, dbRefs);
			}

			//Export all display demos in hold.
			CIDList DemoIDs;
			db.Demos.FilterByShow();
			db.Demos.GetIDs(DemoIDs);
			const DWORD dwNumDemos = DemoIDs.GetSize();
         DWORD dwDemoID, dwDemoHoldID;
			for (dwIndex=0; dwIndex<dwNumDemos; ++dwIndex)
			{
		      dwDemoID = DemoIDs.Get(dwIndex)->dwID;
        		dwDemoHoldID = db.Demos.GetHoldIDofDemo(dwDemoID);
            if (dwDemoHoldID == dwHoldID)
				   str += db.Demos.ExportXML(dwDemoID, dbRefs);
			}

			str += ENDTAG(V_Holds);
		}

		delete pHold;
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
bool CDbHolds::EditableHoldExists() const
//Returns: whether there exists a hold the current player can edit.
{
	//Get holds in DB.
   CIDList holdList;
	g_pTheDB->Holds.GetIDs(holdList);

   for (UINT nIndex=holdList.GetSize(); nIndex--; )
   {
      const DWORD dwHoldID = holdList.Get(nIndex)->dwID;
		if (g_pTheDB->Holds.PlayerCanEditHold(dwHoldID))
         return true;
	}

   return false;
}

//*****************************************************************************
DWORD CDbHolds::GetLevelIDAtIndex(
//Find level at GID index in this hold
//
//Returns:
//LevelID of found level, or 0 if no match.
//
//Params:
   const DWORD dwIndex, const DWORD dwHoldID)  //(in) level index for hold
{
	ASSERT(IsOpen());

   c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	const DWORD dwLevelCount = LevelsView.GetSize();

	//Scan through all the levels to find a match.
	for (DWORD dwLevelI = 0; dwLevelI < dwLevelCount; ++dwLevelI)
	{
      const DWORD dwLevelsHoldID = (DWORD) p_HoldID(LevelsView[dwLevelI]);
		const DWORD dwLevelIndex = (DWORD) p_GID_LevelIndex(LevelsView[dwLevelI]);
		if (dwLevelsHoldID == dwHoldID && dwLevelIndex == dwIndex)
			return (DWORD) p_LevelID(LevelsView[dwLevelI]); //Found it.
	}
	return 0; //Didn't find it.
}

//*****************************************************************************
bool CDbHolds::PlayerCanEditHold(
//Returns: whether current player can view and edit the given hold
//(True if player is the author of the hold, or has completed it, or
//hold has write privileges set for everyone.)
//
//Params:
	const DWORD dwHoldID)	//(in)
const
{
   if (!dwHoldID) return false;

	CDbHold *pHold = g_pTheDB->Holds.GetByID(dwHoldID);
	ASSERT(pHold);
	if (!pHold) return false;
	const DWORD dwAuthorID = pHold->dwPlayerID;
   const CDbHold::EditAccess editAccess = pHold->editingPrivileges;
	delete pHold;

   switch (editAccess)
   {
      case CDbHold::Anyone: return true;

      case CDbHold::OnlyYou: return g_pTheDB->GetPlayerID() == dwAuthorID;

      case CDbHold::YouAndConquerors:
      {
	      if (g_pTheDB->GetPlayerID() == dwAuthorID)
		      return true;
	      const DWORD dwEndHoldID = g_pTheDB->SavedGames.FindByEndHold(dwHoldID);
	      return dwEndHoldID != 0;
      }

      default: ASSERTP(false, "Unexpected editAccess."); return false;
   }
}

//
//CDbHold protected methods.
//

//*****************************************************************************
CDbHold::CDbHold()
//Constructor.
{
	Clear();
}

//
//CDbHold public methods.
//

//*****************************************************************************
CDbHold::~CDbHold()
//Destructor.
{
	Clear();
}

//*****************************************************************************
bool CDbHold::ChangeAuthor(
//Change hold author.  Revise hold name and description to reflect this.
//
//Returns: whether author was changed
//
//Params:
   const DWORD dwNewAuthorID)   //(in) new hold author-player
{
   ASSERT(dwNewAuthorID);
   if (this->dwPlayerID == dwNewAuthorID)
      return false;  //this player is already the hold author

	CDbPlayer *pPlayer = g_pTheDB->Players.GetByID(dwNewAuthorID);
   ASSERT(pPlayer);
	WSTRING holdName = (WSTRING)pPlayer->NameText;
	delete pPlayer;

   static const WCHAR wszApostrophe[] = { W_t('\''), W_t('s'), W_t(' '), W_t(0) };
	holdName += wszApostrophe;
	holdName += (WSTRING)this->NameText;
	this->NameText = holdName.c_str();

	WSTRING holdDesc = (WSTRING)this->DescriptionText;
	CDbPlayer *pOrigHoldAuthor = g_pTheDB->Players.GetByID(this->dwPlayerID);
   if (pOrigHoldAuthor)
   {
		holdDesc += wszSpace;
		holdDesc += wszLeftParen;
		holdDesc += g_pTheDB->GetMessageText(MID_HoldOriginallyAuthoredBy);
		holdDesc += wszSpace;
		holdDesc += (WSTRING)pOrigHoldAuthor->NameText;
		holdDesc += wszRightParen;
		delete pOrigHoldAuthor;
   }
	this->DescriptionText = holdDesc.c_str();

   this->dwPlayerID = dwNewAuthorID;
  	return Update();
}

//*****************************************************************************
bool CDbHold::Load(
//Loads a hold from database into this object.
//
//Params:
	const DWORD dwLoadHoldID)	//(in) HoldID of hold to load.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;

	Clear();

	//Open holds view.
	ASSERT(IsOpen());
	c4_View HoldsView = GetView(ViewTypeStr(V_Holds));

	//Find record with matching hold ID.
   DWORD dwEndHoldMessageID;
	const DWORD dwHoldI = LookupRowByPrimaryKey(dwLoadHoldID, p_HoldID, HoldsView);
	if (dwHoldI == ROW_NO_MATCH) {bSuccess=false; goto Cleanup;}

	//Load in props from Holds record.
	this->dwHoldID = (DWORD) p_HoldID(HoldsView[dwHoldI]);
	this->NameText.Bind((DWORD) p_NameMessageID(HoldsView[dwHoldI]));
	this->DescriptionText.Bind((DWORD) p_DescriptionMessageID(HoldsView[dwHoldI]));
	this->dwLevelID = (DWORD) p_LevelID(HoldsView[dwHoldI]);
	this->Created = (time_t) (p_GID_Created(HoldsView[dwHoldI]));
	this->LastUpdated = (time_t) (p_LastUpdated(HoldsView[dwHoldI]));
	this->dwPlayerID = (DWORD) p_GID_PlayerID(HoldsView[dwHoldI]);
	this->dwNewLevelIndex = (DWORD) (p_GID_NewLevelIndex(HoldsView[dwHoldI]));
   this->editingPrivileges = (CDbHold::EditAccess)(DWORD)(p_EditingPrivileges(HoldsView[dwHoldI]));
   dwEndHoldMessageID = (DWORD) p_EndHoldMessageID(HoldsView[dwHoldI]);
   if (dwEndHoldMessageID)
      this->EndHoldText.Bind(dwEndHoldMessageID);

	//Set filter for Levels to show only levels in this hold.
	this->Levels.FilterBy(this->dwHoldID);

	//Set filter for SavedGames to show only saved games in this hold.
	this->SavedGames.FilterByHold(this->dwHoldID);
	
Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
CDbLevel* CDbHold::GetStartingLevel()
//Gets starting level for this hold.
//
//Returns:
//Pointer to new loaded level object which caller must delete, or NULL if level
//was not defined or could not be loaded.
const
{
	if (this->dwLevelID==0)
      return NULL;

	CDbLevel *pLevel = new CDbLevel();
	if (pLevel)
	{
		if (!pLevel->Load(this->dwLevelID))
		{
			delete pLevel;
			pLevel = NULL;
		}
	}
	return pLevel;
}

//*****************************************************************************
void CDbHold::InsertLevel(
//Inserts level into this hold.
//Update rooms' Exits list as needed.
//If dwLevelSupplantedID was the ID of the hold's first level, the new
//level becomes the hold's first level.
//
//Params:
   CDbLevel *pLevel,                //(in) Level being inserted
	const DWORD dwLevelSupplantedID)	//(in) Level this one is placed in front of
{
	ASSERT(IsOpen());
	ASSERT(pLevel->dwHoldID);
   ASSERT(pLevel->dwHoldID == this->dwHoldID);

	//Give level a (GID) value in the hold.
	ASSERT(pLevel->dwLevelIndex == 0);  //level is not current a member of a hold
	pLevel->dwLevelIndex = ++this->dwNewLevelIndex;
	ASSERT(pLevel->dwLevelIndex);

	//If new level supplants hold's first level, or hold doesn't have a first level,
   //update hold's first level ID.
	if (!this->dwLevelID || dwLevelSupplantedID == this->dwLevelID)
   {
      ASSERT(pLevel->dwLevelID);   //otherwise, the hold will not work
		this->dwLevelID = pLevel->dwLevelID;
   }

	//Update room Exits lists.
   //NOTE: Any existing room objects are to be discarded following this operation,
   //otherwise Updating them will revert their destination exits to the old values!
	c4_View RoomsView = GetView(ViewTypeStr(V_Rooms));
	const DWORD dwRoomCount = RoomsView.GetSize();
	for (DWORD dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
	{
		const DWORD dwLevelID = (DWORD) p_LevelID(RoomsView[dwRoomI]);
		c4_View ExitsView = p_Exits(RoomsView[dwRoomI]);
		const DWORD dwExitCount = ExitsView.GetSize();

		for (DWORD dwNextLevelI = 0; dwNextLevelI < dwExitCount; ++dwNextLevelI)
		{
			if (dwLevelID == pLevel->dwLevelID)
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

   //Must save hold's modified dwNewLevelIndex, etc.
   Update();
}

//*****************************************************************************
void CDbHold::RemoveLevel(
//Removes references to this level's ID from the hold.
//Specifically, this replaces 'dwLevelID' in all levels' room Exits lists
//that have it to 'newLevelID'.
//NOTE: RemoveLevel() should generally be called following a call to CDbLevel::Delete().
//
//Params:
	const DWORD dwLevelID, const DWORD dwNewLevelID)	//(in)
{
	ASSERT(dwLevelID);
	ASSERT(IsOpen());

	//If this level is the first level in the hold, update hold's first level ID.
	if (dwLevelID == this->dwLevelID)
   {
      if (dwNewLevelID)
         this->dwLevelID = dwNewLevelID;
      else {
         //Get ID of first remaining level, else 0.
         this->Levels.ResetMembership();
         CIDList LevelIDs;
         this->Levels.GetIDs(LevelIDs);
         this->dwLevelID = LevelIDs.GetSize() > 0 ? LevelIDs.GetID(0) : 0;
      }
   }

	//Update Exits sub-record in any rooms leading to this level.
   c4_View ExitsView;
	c4_View RoomsView = GetView(ViewTypeStr(V_Rooms));
	const UINT wRoomCount = RoomsView.GetSize();
	for (UINT wRoomI = 0; wRoomI < wRoomCount; ++wRoomI)
	{
      ExitsView = p_Exits(RoomsView[wRoomI]);
		const UINT wExitCount = ExitsView.GetSize();
		for (UINT wNextLevelI = 0; wNextLevelI < wExitCount; ++wNextLevelI)
		{
			const DWORD dwNextLevelID = (DWORD) p_LevelID(ExitsView[wNextLevelI]);
			if (dwNextLevelID == dwLevelID)
				p_LevelID( ExitsView[wNextLevelI] ) = dwNewLevelID;
		}
	}
}

//*****************************************************************************
CDbHold* CDbHold::MakeCopy()
//Creates a copy of the entire hold, saving it to the DB.
//
//Returns: pointer to new hold
{
   CDbHold *pNewHold = g_pTheDB->Holds.GetNew();
   if (!pNewHold) return NULL;

   *pNewHold = *this;         //must make new message texts
	pNewHold->dwHoldID = 0L;   //so this hold gets added to DB as new hold
   pNewHold->Created = 0;     //update timestamps
	pNewHold->dwLevelID = 0L;	//set below
	pNewHold->Update();

   //Make a copy of all the levels in hold.
   const DWORD dwEntranceLevelID = SaveCopyOfLevels(pNewHold->dwHoldID);

	//Set new hold's entrance level ID.
	pNewHold->dwLevelID = dwEntranceLevelID;
	pNewHold->Update();

   //2nd pass to update exit level IDs to point to level copies.
   CDb db;
   db.Levels.FilterBy(pNewHold->dwHoldID);
   CIDList levelIDs;
   db.Levels.GetIDs(levelIDs);
   IDNODE *pLevelID = levelIDs.Get(0);
   while (pLevelID)
   {
      db.Levels.UpdateExitIDs(pLevelID->dwID, pNewHold->dwHoldID, false);
      pLevelID = pLevelID->pNext;
   }

   return pNewHold;
}

//*****************************************************************************
bool CDbHold::Repair()
//Repair corrupted data, if possible (might be needed for data from older versions).
//
//Returns: whether bad data was updated
{
   CIDList levelIDs;
   if (this->dwLevelID)
   {
      this->Levels.GetIDs(levelIDs);
      if (!levelIDs.IsIDInList(this->dwLevelID))
      {
         //Entrance level ID is bogus -- update it.
         this->dwLevelID = levelIDs.GetSize() == 0 ? 0 : levelIDs.GetID(0);
         return true;
      }
   } else {
      this->Levels.GetIDs(levelIDs);
      if (levelIDs.GetSize() > 0)
      {
         //There should be an entrance level ID -- set it.
         this->dwLevelID = levelIDs.GetID(0);
         return true;
      }
   }
   return false;
}

//*****************************************************************************
DWORD CDbHold::SaveCopyOfLevels(
//Make copies of all levels in hold in the DB.
//
//Returns: ID of new entrance level (0 if no levels exist)
//
//Params:
   const DWORD dwNewHoldID)  //(in) hold new levels belong to
{
	BEGIN_DBREFCOUNT_CHECK;

   DWORD dwEntranceLevelID = 0L;
	this->Levels.FilterBy(this->dwHoldID);
	CDbLevel *pLevel = this->Levels.GetFirst();
	while (pLevel)
	{
		const bool bEntranceLevel = pLevel->dwLevelID == this->dwLevelID;
		CDbLevel *pNewLevel = pLevel->MakeCopy(dwNewHoldID);
	   pNewLevel->dwHoldID = dwNewHoldID;
      pNewLevel->Update();

      if (bEntranceLevel)
		{
			//Get hold's new entrance level ID.
			ASSERT(dwEntranceLevelID == 0L);	//there should only be one
			dwEntranceLevelID = pNewLevel->dwLevelID;
		}
      delete pNewLevel;
		delete pLevel;
		pLevel = this->Levels.GetNext();
	}

   END_DBREFCOUNT_CHECK;

   return dwEntranceLevelID;
}

//*****************************************************************************
bool CDbHold::SetMembers(
//For copy constructor and assignment operator.
//
//Params:
	const CDbHold &Src)
{
	//primitive types
	this->dwHoldID = Src.dwHoldID;
	this->dwLevelID = Src.dwLevelID;
	this->dwPlayerID = Src.dwPlayerID;
   this->editingPrivileges = Src.editingPrivileges;
   this->dwNewLevelIndex = Src.dwNewLevelIndex;

   //object members
	this->NameText = Src.NameText;	//make new message texts
	this->DescriptionText = Src.DescriptionText;
	this->Created = Src.Created;
	this->LastUpdated = Src.LastUpdated;
   this->EndHoldText = Src.EndHoldText;

   this->Levels.ResetMembership();
	this->Levels.FilterBy(this->dwHoldID);
   this->SavedGames.ResetMembership();
	this->SavedGames.FilterByHold(this->dwHoldID);

	return true;
}

//*****************************************************************************
MESSAGE_ID CDbHold::SetProperty(
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
		case P_HoldID:
		{
			this->dwHoldID = static_cast<DWORD>(atol(str));
			if (!this->dwHoldID)
				return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = info.HoldIDMap.find(this->dwHoldID);
			if (localID != info.HoldIDMap.end())
				//Error - this hold should not have been imported already.
				return MID_FileCorrupted;

			//Look up hold in the DB.
			const DWORD dwLocalHoldID = GetLocalID();
			if (dwLocalHoldID)
			{
			   //Hold found in DB.
            //Compare hold versions.
				c4_View HoldsView = GetView(ViewTypeStr(V_Holds));
				const DWORD dwHoldI = LookupRowByPrimaryKey(
						dwLocalHoldID, p_HoldID, HoldsView);
	         if (dwHoldI == ROW_NO_MATCH) return MID_FileCorrupted;
            const time_t lastUpdated = (time_t)p_LastUpdated(HoldsView[dwHoldI]);
            switch (info.typeBeingImported)
            {
            case CImportInfo::Hold:
            {
               if (this->NameText.IsEmpty())
               {
                  //This is just a hold reference.  Don't need to check its version.
				      info.HoldIDMap[this->dwHoldID] = dwLocalHoldID;
                  this->dwHoldID = dwLocalHoldID;
                  bSaveRecord = false;
               } else {
                  //This hold is being imported.
				      if (lastUpdated >= this->LastUpdated)
                  {
        				   //Don't import this hold since it's an older version of an existing one.
				         bSaveRecord = false;
				         info.HoldIDMap[this->dwHoldID] = dwLocalHoldID;
				         this->dwHoldID = dwLocalHoldID;
                     return MID_HoldIgnored;
                  }

				      const DWORD dwOldLocalID = this->dwHoldID;

                  //Compile GUID list of levels with ST_LevelStart saves.
                  const UINT wPlayersAffected = CDbXML::GetLevelStartIDs(dwLocalHoldID, dwOldLocalID, info);
                  //Prompt the player that saved games for the older hold will be erased.
                  if (!info.bReplaceOldHolds)
                  {
                     if (wPlayersAffected == 1) //means you will lose saved games
                     {
				            bSaveRecord = false;
                        return MID_OverwriteHoldPrompt;
                     }
                     if (wPlayersAffected == 2) //means other players will lose saved games
                     {
     				         bSaveRecord = false;
                        return MID_OverwriteHoldPrompt2;
                     }
                     //Otherwise there are no saved games to lose for this hold,
                     //so confirmation can be skipped.
                  }

                  //Remove old hold from the DB.  This new one replaces it.
				      g_pTheDB->Holds.Delete(dwLocalHoldID);
                  this->dwHoldID = 0;
                  Update();
				      info.HoldIDMap[dwOldLocalID] = this->dwHoldID;
                  info.dwHoldImportedID = this->dwHoldID;  //keep track of which hold was imported
               }
               break;
            }
            case CImportInfo::Player:
            {
               //A player with records for this hold is being imported.
			      bSaveRecord = false;
				   info.HoldIDMap[this->dwHoldID] = dwLocalHoldID;
				   this->dwHoldID = dwLocalHoldID;

               //!!Needed to correctly import players exported from versions of
               //Dugan's Dungeon hold with a buggy timestamp (before build 29).
               if (dwLocalHoldID == 1 && this->LastUpdated >= lastUpdated)
                  break;

               if (lastUpdated != this->LastUpdated &&
                     info.ImportStatus == MID_ImportSuccessful)   //player record is actually being imported
               {
                  //Hold versions don't match.
                  //Only Level Start saved games will be retained for this hold.
                  info.localHoldIDs.push_back(dwLocalHoldID);
                  return MID_PlayerSavesIgnored;
               }
               break;
				}
            case CImportInfo::Demo:
            {
               //A demo in this hold is being imported.
               //No hold version checking is done.
			      bSaveRecord = false;
				   info.HoldIDMap[this->dwHoldID] = dwLocalHoldID;
				   this->dwHoldID = dwLocalHoldID;
               break;
            }
            default: break;
            }  //switch
			} else {
				//Hold not found -- add a new record to the DB.
            if (this->NameText.IsEmpty())
            {
				   //This means this record is a GUID reference for a hold not
               //present.  Ignore importing records related to this hold.
               info.HoldIDMap[this->dwHoldID] = 0;   //skip records with refs to this hold ID
               bSaveRecord = false;
            } else {
               //Import this hold into the DB.
               const DWORD dwOldLocalID = this->dwHoldID;
				   this->dwHoldID = 0L;
				   Update();
				   info.HoldIDMap[dwOldLocalID] = this->dwHoldID;
               info.dwHoldImportedID = this->dwHoldID;  //keep track of which hold was imported
            }
			}
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
		case P_LevelID:
			this->dwLevelID = static_cast<DWORD>(atol(str));
			//Level hasn't been read in yet and local ID must be set later
			break;
		case P_GID_Created:
			this->Created = (time_t)(atol(str));
			break;
		case P_LastUpdated:
			this->LastUpdated = (time_t)(atol(str));
			break;
		case P_GID_PlayerID:
			this->dwPlayerID = static_cast<DWORD>(atol(str));
			if (!this->dwPlayerID)
				return MID_FileCorrupted;	//corrupt file (must have an author)

			//Set to local ID.
			localID = info.PlayerIDMap.find(this->dwPlayerID);
			if (localID == info.PlayerIDMap.end())
				return MID_FileCorrupted;	//record should have been loaded already
			this->dwPlayerID = (*localID).second;
			break;
		case P_GID_NewLevelIndex:
			this->dwNewLevelIndex = static_cast<DWORD>(atol(str));
			if (this->dwLevelID && !this->dwNewLevelIndex)
				return MID_FileCorrupted;	//bad data -- can't have a level when dwNewLevelIndex == 0
			break;
		case P_EditingPrivileges:
         this->editingPrivileges = static_cast<EditAccess>(atol(str));
			break;
		case P_EndHoldMessage:
		{
			WSTRING data;
			Base64::decode(str,data);
			this->EndHoldText = data.c_str();
			break;
		}
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbHold::Update()
//Updates database with hold.
//
//Returns: true if successful, else false.
{
	bool bSuccess=true;

   g_pTheDB->Holds.ResetMembership();
	if (this->dwHoldID == 0)
	{
		//Insert a new hold.
		bSuccess = UpdateNew();
	}
	else
	{
		//Update existing hold.
		bSuccess = UpdateExisting();
	}

	if (!bSuccess) return false;

	//Set filter for Levels to show only levels in this hold.
	this->Levels.FilterBy(this->dwHoldID);

	//Set filter for SavedGames to show only saved games in this hold.
	this->SavedGames.FilterByHold(this->dwHoldID);

	return bSuccess;
}

//
//CDbHold private methods.
//

//*****************************************************************************
DWORD CDbHold::GetLocalID() const
//Compares this object's GID fields against those of the records in the DB.
//ASSUME: dwPlayerID has already been set to the local record ID
//
//Returns: local ID if a record in the DB matches this object's GUID, else 0
{
	ASSERT(IsOpen());
	c4_View HoldsView = GetView(ViewTypeStr(V_Holds));
	const DWORD dwHoldCount = HoldsView.GetSize();

	//Each iteration checks a hold's GIDs.
	for (DWORD dwHoldI = 0; dwHoldI < dwHoldCount; ++dwHoldI)
	{
		//Check time of creation.
		const time_t Created = (time_t)p_GID_Created(HoldsView[dwHoldI]);
		if (this->Created == Created)
		{
			//Check author.
			const DWORD dwPlayerID = (DWORD)p_GID_PlayerID(HoldsView[dwHoldI]);
			if (this->dwPlayerID == dwPlayerID)
			{
				//GUIDs match.  Return this record's local ID.
				return (DWORD) p_HoldID(HoldsView[dwHoldI]);
			}
		}
	}

	//No match.
	return 0L;
}

//*****************************************************************************
bool CDbHold::UpdateNew()
//Add new Holds record to database.
{
	ASSERT(this->dwHoldID == 0);
	ASSERT(IsOpen());

	//Prepare props.
	this->dwHoldID = GetIncrementedID(p_HoldID);
	if ((time_t)this->Created == 0)
	{
		this->Created.SetToNow();
		this->LastUpdated.SetToNow();
	}

	//Write out message texts.
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwDescID = this->DescriptionText.Flush();
	const DWORD dwEndHoldID = this->EndHoldText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwDescID);
   //ASSERT(dwEndHoldID);  //might be 0

	//Write new Hold record.
	c4_View HoldsView = GetView(ViewTypeStr(V_Holds));
	HoldsView.Add(
			p_HoldID[ this->dwHoldID ] +
			p_NameMessageID[ dwNameID ] +
			p_DescriptionMessageID[ dwDescID ] +
			p_LevelID[ this->dwLevelID ] +
			p_GID_Created[ this->Created ] +
			p_LastUpdated[ this->LastUpdated ] +
			p_GID_PlayerID[ this->dwPlayerID ] +
			p_GID_NewLevelIndex[ 0 ] +
			p_EditingPrivileges[ this->editingPrivileges ] +
			p_EndHoldMessageID[ dwEndHoldID ]);

	return true;
}

//*****************************************************************************
bool CDbHold::UpdateExisting()
//Update an existing Holds record in database.
{
	ASSERT(this->dwHoldID != 0);
	ASSERT(IsOpen());

	//Lookup Holds record.
	c4_View HoldsView = GetView(ViewTypeStr(V_Holds));
	const DWORD dwHoldID = LookupRowByPrimaryKey(this->dwHoldID,
			p_HoldID, HoldsView);
	if (dwHoldID == ROW_NO_MATCH)
	{
		ASSERTP(false, "The caller probably passed a bad PKID.");
		return false;
	}

	//Prepare props.
   if (!CDb::FreezingTimeStamps())
	   this->LastUpdated.SetToNow();
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwDescID = this->DescriptionText.Flush();
	const DWORD dwEndHoldID = this->EndHoldText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwDescID);

	//Update Holds record.
	p_HoldID( HoldsView[ dwHoldID ] ) = this->dwHoldID;
	p_LevelID( HoldsView[ dwHoldID ] ) = this->dwLevelID;
	p_LastUpdated( HoldsView[ dwHoldID ] ) = this->LastUpdated;
	p_GID_PlayerID( HoldsView[ dwHoldID ] ) = this->dwPlayerID;
	p_GID_NewLevelIndex( HoldsView[ dwHoldID ] ) = this->dwNewLevelIndex;
	p_EditingPrivileges( HoldsView[ dwHoldID ] ) = this->editingPrivileges;
   p_EndHoldMessageID( HoldsView[ dwHoldID ] ) = dwEndHoldID;

	return true;
}

//*****************************************************************************
void CDbHold::Clear()
//Frees resources associated with this object and resets member vars.
{
	this->dwLevelID = this->dwHoldID = this->dwPlayerID = 0L;

	this->NameText.Clear();
	this->DescriptionText.Clear();
	this->EndHoldText.Clear();
	this->Created = this->LastUpdated = 0L;

	this->dwNewLevelIndex = 0L;
	this->editingPrivileges = OnlyYou;
}

//*****************************************************************************
bool CDbHold::VerifySolvability()
//Checks whether the hold is "solvable".
//Currently, this means check for a currently-valid victory demo
//in each required room in the hold.
{
	CIDList LevelIDs, RoomIDs;
	IDNODE *pLevelNode;
   CDbRoom *pRoom;

	//Get list of all levels in hold.
	this->Levels.GetIDs(LevelIDs);

	pLevelNode = LevelIDs.Get(0);
   CDb db;
	while (pLevelNode)
	{
		//Get list of all rooms in level.
		db.Rooms.FilterBy(pLevelNode->dwID);
      pRoom = db.Rooms.GetFirst();
		while (pRoom)
		{
         //Only check for victory demo if conquering room is required to beat level.
         if (pRoom->bIsRequired)
         {
			   bool bVictoryDemoFound = false;

			   //Get all demos in room.
			   db.Demos.FilterByRoom(pRoom->dwRoomID);
			   CDbDemo *pDemo = db.Demos.GetFirst();

			   while (pDemo)
			   {
				   //Check for a victory demo in the room.
				   CIDList DemoStats;
				   const bool bIntact = pDemo->Test(DemoStats);
				   delete pDemo;
				   if (bIntact && DemoStats.IsIDInList(DS_WasRoomConquered) &&
						   DemoStats.IsIDInList(DS_DidPlayerLeaveRoom))
				   {
					   bVictoryDemoFound = true;
					   break;
				   }
				   pDemo = db.Demos.GetNext();
			   }
			   if (!bVictoryDemoFound)
            {
               delete pRoom;
				   return false;
            }
         }

         delete pRoom;
			pRoom = db.Rooms.GetNext();
		}

		pLevelNode=pLevelNode->pNext;
	}

	return true;
}

// $Log: DbHolds.cpp,v $
// Revision 1.56  2004/01/02 01:04:54  mrimer
// Fixed bug: hold end text sometimes not retained in copied hold.
//
// Revision 1.55  2003/11/09 05:25:26  mrimer
// Made export release code more robust.
//
// Revision 1.54  2003/10/20 17:49:03  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.53  2003/10/07 21:10:34  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.52  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.51  2003/08/18 21:04:11  erikh2000
// "Who can edit?" now defaults to "only you" for new holds.
//
// Revision 1.50  2003/08/12 18:50:00  mrimer
// Moved CDbLevelS::RemoveFromHold() to CDbHold::RemoveLevel().
//
// Revision 1.49  2003/08/09 18:09:48  mrimer
// Fixed bug: Replaced assertions with more robust code catching DB row lookup failures to prevent crashes with corrutped data.
//
// Revision 1.48  2003/08/09 00:38:53  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.47  2003/08/06 01:18:00  mrimer
// Fixed bug: level exit IDs not being updated on hold copy.
//
// Revision 1.46  2003/07/26 22:59:24  mrimer
// Fixed a memory leak.
//
// Revision 1.45  2003/07/25 22:45:48  mrimer
// Now allow importing a hold if its description field is empty.
//
// Revision 1.44  2003/07/25 00:03:09  mrimer
// Changed call to GetLevelStartIDs().
//
// Revision 1.43  2003/07/23 19:05:30  mrimer
// Fixed hold solvability check to only consider "required" rooms.
//
// Revision 1.42  2003/07/21 22:03:10  mrimer
// Added ChangeAuthor().  Fixed bug in SetMembers().
//
// Revision 1.41  2003/07/19 21:19:34  mrimer
// Fixed bug: imported demo's hold not being found.
//
// Revision 1.40  2003/07/19 02:38:36  mrimer
// Added handling of new EndHoldMessage(ID) member.
// Made export code more robust and maintainable.
//
// Revision 1.39  2003/07/16 07:40:12  mrimer
// Fixed 3-4 level indexing bugs.  Rewrote CDbLevel::InsertIntoHold() as CDbHold::InsertLevel().
//
// Revision 1.38  2003/07/15 00:28:02  mrimer
// Added MakeCopy().  Fixed hold copying bugs and two import bugs (all due to new features in build 28).
//
// Revision 1.37  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.36  2003/07/09 21:22:24  mrimer
// Added Repair().  Revised export and import routines to be more robust.
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.35  2003/07/07 23:35:30  mrimer
// Added new methods to make a copy of an entire hold in the DB.
//
// Revision 1.34  2003/07/01 20:22:44  mrimer
// Made error handling more robust.
//
// Revision 1.33  2003/06/27 17:27:32  mrimer
// Added hold editing privilege data.
//
// Revision 1.32  2003/06/27 00:37:04  mrimer
// Fixed bug: PlayerID not getting written in CDbHold::UpdateExisting().
//
// Revision 1.31  2003/06/26 17:35:52  mrimer
// Ignore importing records for non-existant holds.
//
// Revision 1.30  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.29  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.28  2003/06/17 22:15:52  mrimer
// Fixed ExportXML() to only export show demos in the hold being exported.
//
// Revision 1.27  2003/06/09 19:26:27  mrimer
// Added methods for checking whether a hold is editable by a player.
//
// Revision 1.26  2003/06/03 06:21:20  mrimer
// Now calls ResetMembership() on DB update.
//
// Revision 1.25  2003/05/25 22:46:25  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.24  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.23  2003/05/22 23:39:01  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.22  2003/05/20 18:12:41  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.21  2003/05/08 23:19:45  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.20  2003/05/08 22:01:07  mrimer
// Replaced local CDb instances with a pointer to global instance.
//
// Revision 1.19  2003/04/29 11:05:53  mrimer
// Moved CDb::verifyHoldSolvability() to CDbHolds::VerifySolvability().
//
// Revision 1.18  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.17  2003/02/24 17:06:23  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.16  2003/02/16 20:29:32  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.15  2002/12/22 02:05:24  mrimer
// Added XML import/export support.  (Added GUID support.)
//
// Revision 1.14  2002/11/22 22:00:19  mrimer
// Fixed a subtle bug with membership loading.
//
// Revision 1.13  2002/11/14 19:06:45  mrimer
// Added methods GetNew(), Delete(), and Update(), etc.
// Made GetByID() static.
// Moved member initialization into constructor initialization list.
//
// Revision 1.12  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.11  2002/06/15 18:27:28  erikh2000
// Changed code so that storage is not committed after each database write.
//
// Revision 1.10  2002/06/09 06:14:14  erikh2000
// Changed code to use new message text class.
//
// Revision 1.9  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.8  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.7  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.6  2002/02/08 23:20:27  erikh2000
// Added #include <list> to fix compile error.
//
// Revision 1.5  2001/12/16 02:11:56  erikh2000
// Simplified calls to CIDList::Get().
//
// Revision 1.4  2001/12/08 03:18:31  erikh2000
// Added Levels member to CDbHold.
//
// Revision 1.3  2001/12/08 02:37:03  erikh2000
// Wrote CDbHolds::GetFirst() and CDbHolds::GetNext().
//
// Revision 1.2  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.1.1.1  2001/10/01 22:20:09  erikh2000
// Initial check-in.
//
