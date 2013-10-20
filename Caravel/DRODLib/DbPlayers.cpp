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
 * Portions created by the Initial Developer are Copyright (C) 
 * 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//DbPlayers.cpp.
//Implementation of CDbPlayers and CDbPlayer.

#include "Db.h"

#include "DbPlayers.h"
#include "DBProps.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Base64.h>
#include <BackEndLib/Ports.h>

#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
#error The byte order of several data-types may differ.
//Need to write code in GetVar() and SetVar() to convert between different
//byte-ordering schemes.
#endif

//
//CDbPlayers public methods.
//

//*****************************************************************************
void CDbPlayers::Delete(
//Deletes record for player with the given ID.
//By default, it leaves a reference to the player in the DB,
//as that player might be a hold/level author and we need to retain
//their info.
//
//Params:
	const DWORD dwPlayerID,	//(in)	ID of player(s) to delete.
	const bool bRetainRef)	//(in)	whether a player reference should be kept
									//			(default=true)
{
	ASSERT(dwPlayerID);

	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayerRowI = LookupRowByPrimaryKey(dwPlayerID, p_PlayerID, PlayersView);
	if (dwPlayerRowI == ROW_NO_MATCH) {ASSERTP(false, "Bad player ID."); return;}

	if (!bRetainRef)
	{
		//Delete name and email message texts.
		const DWORD dwNameMID = p_NameMessageID( PlayersView[dwPlayerRowI] );
		if (!dwNameMID) {ASSERTP(false, "Bad MID for name."); return;}
		DeleteMessage(static_cast<MESSAGE_ID>(dwNameMID));
		const DWORD dwEMailMID = p_EMailMessageID( PlayersView[dwPlayerRowI] );
		if (!dwEMailMID) {ASSERTP(false, "Bad MID for e-mail."); return;}
		DeleteMessage(static_cast<MESSAGE_ID>(dwEMailMID));
		const DWORD dwOrigNameMID = p_GID_OriginalNameMessageID( PlayersView[dwPlayerRowI] );
		if (!dwOrigNameMID) {ASSERTP(false, "Bad MID for original name."); return;}
		DeleteMessage(static_cast<MESSAGE_ID>(dwOrigNameMID));
	}

	//Delete player's demos first and then saved games (including hidden ones).
	CIDList DemoIDs, SavedGameIDs;
	IDNODE *pSeek;

   CDb db;
	db.Demos.FilterByPlayer(dwPlayerID);
	db.Demos.FindHiddens(true);
	db.Demos.GetIDs(DemoIDs);
	pSeek = DemoIDs.Get(0);
	while (pSeek)
	{
		db.Demos.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}
	db.SavedGames.FilterByPlayer(dwPlayerID);
	db.SavedGames.FindHiddens(true);
	db.SavedGames.GetIDs(SavedGameIDs);
	pSeek = SavedGameIDs.Get(0);
	while (pSeek)
	{
		db.SavedGames.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}

	if (!bRetainRef)
	{
		//Delete the player.
		PlayersView.RemoveAt(dwPlayerRowI);
	} else {
		//Hide the player record.
		p_IsLocal( PlayersView[dwPlayerRowI] ) = false;
	}

	//After player object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//*****************************************************************************
string CDbPlayers::ExportXML(
//Returns: string containing XML text describing player with this ID
//
//Pre-condition: dwPlayerID is valid
//
//Params:
	const DWORD dwPlayerID,	//(in)
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

	if (!dbRefs.IsSet(V_Players,dwPlayerID))
	{
		dbRefs.Set(V_Players,dwPlayerID);

		CDbPlayer *pPlayer = GetByID(dwPlayerID);
		ASSERT(pPlayer);
      if (!pPlayer) return str; //shouldn't happen, but just in case

		//Prepare data.
		WSTRING const wNameStr = (WSTRING)pPlayer->NameText;
		WSTRING const wEMailStr = (WSTRING)pPlayer->EMailText;
		WSTRING const wOrigNameStr = (WSTRING)pPlayer->OriginalNameText;
		char dummy[32];

		str += STARTTAG(V_Players, P_GID_OriginalNameMessage);
		str += Base64::encode(wOrigNameStr);
		str += PROPTAG(P_GID_Created);
		str += LONGTOSTR((time_t)pPlayer->Created);
		str += PROPTAG(P_LastUpdated);
		if (bRef)
			str += "0";	//player reference -- don't overwrite anything
		else
			str += LONGTOSTR((time_t)pPlayer->LastUpdated);
		str += PROPTAG(P_NameMessage);
		str += Base64::encode(wNameStr);
		str += PROPTAG(P_EMailMessage);
		str += Base64::encode(wEMailStr);
		str += PROPTAG(P_IsLocal);
		if (bRef)
			str += "0";	//player reference  -- don't display 
		else
			str += LONGTOSTR((long)pPlayer->bIsLocal);
		//Put primary key last, so all message fields have been set by the
		//time Update() is called during import.
		str += PROPTAG(P_PlayerID);
		str += LONGTOSTR(pPlayer->dwPlayerID);
		if (bRef)
		{
			//Don't need any further information for a player reference.
			str += CLOSETAG;
		} else {

			{
				DWORD dwBufSize;
				BYTE *pSettings = pPlayer->Settings.GetPackedBuffer(dwBufSize);
				if (dwBufSize > 4)
				{
					str += PROPTAG(P_Settings);
					str += Base64::encode(pSettings,dwBufSize-4);	//remove null UINT
				}
				delete pSettings;
			}

			str += CLOSESTARTTAG;

			//Export player's demos.
         //Include hidden demos (e.g. highlight).
         CDb db;
			CIDList DemoIDs;
			db.Demos.FilterByPlayer(dwPlayerID);
         db.Demos.FindHiddens(true);
			db.Demos.GetIDs(DemoIDs);
			const DWORD dwNumDemos = DemoIDs.GetSize();
         DWORD dwIndex;
			for (dwIndex=0; dwIndex<dwNumDemos; ++dwIndex)
			{
				str += db.Demos.ExportXML(DemoIDs.Get(dwIndex)->dwID, dbRefs);
			}

			//Export player's saved games (not attached to demos).
         //Include hidden saved games (e.g. continue, end hold).
			CIDList SavedGameIDs;
			db.SavedGames.FilterByPlayer(dwPlayerID);
         db.SavedGames.FindHiddens(true);
			db.SavedGames.GetIDs(SavedGameIDs);
			const DWORD dwNumSavedGames = SavedGameIDs.GetSize();
			for (dwIndex=0; dwIndex<dwNumSavedGames; ++dwIndex)
			{
				str += db.SavedGames.ExportXML(
						SavedGameIDs.Get(dwIndex)->dwID, dbRefs);
			}

			str += ENDTAG(V_Players);
		}

		delete pPlayer;
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
void CDbPlayers::FilterByLocal()
//Filter membership so that it only contains local players.
{
	this->bFilterByLocal = true;
	this->bIsMembershipLoaded = false;
}

//*****************************************************************************
DWORD CDbPlayers::FindByName(
//Find a player by name.
//
//Params:
	const WCHAR *pwczName)	//(in)	Case-sensitive name to look for.
//
//Returns:
//PlayerID if a match was found, 0L for no match.
{
	ASSERT(IsOpen());
	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayerCount = PlayersView.GetSize();
	
	//Each iteration checks a player name from one record.
	for (DWORD dwPlayerI = 0; dwPlayerI < dwPlayerCount; ++dwPlayerI)
	{
		CDbMessageText NameText;
		NameText.Bind( p_NameMessageID(PlayersView[dwPlayerI]) );
		if (WCScmp(pwczName, (const WCHAR *) NameText)==0)
			return (DWORD) p_PlayerID(PlayersView[dwPlayerI]); //Found it.
	}
	
	//No match.
	return 0L;
}

//
//CDbPlayers private methods.
//

//*****************************************************************************
void CDbPlayers::LoadMembership()
//Load the membership list with all player IDs.
{
	ASSERT(IsOpen());
	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayerCount = PlayersView.GetSize();
	
	//Each iteration gets a player ID and maybe puts in membership list.
	this->MembershipIDs.Clear();
	for (DWORD dwPlayerI = 0; dwPlayerI < dwPlayerCount; ++dwPlayerI)
	{
		const bool bIsLocal = ((UINT) p_IsLocal(PlayersView[dwPlayerI]) != 0);
		if (bIsLocal || !this->bFilterByLocal)
			this->MembershipIDs.Add(p_PlayerID(PlayersView[dwPlayerI]));
	}
	this->pCurrentRow = this->MembershipIDs.Get(0);
	this->bIsMembershipLoaded = true;
}

//
//CDbPlayer public methods.
//

//*****************************************************************************
bool CDbPlayer::Load(
//Loads a player from database into this object.
//
//Params:
	DWORD dwLoadPlayerID)	//(in) PlayerID of player to load.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;

	Clear();

	ASSERT(IsOpen());

	{
		//Open Players view.
		c4_View PlayersView = GetView(ViewTypeStr(V_Players));

		//Find record with matching player ID.
		const DWORD dwPlayerI = LookupRowByPrimaryKey(dwLoadPlayerID, p_PlayerID,
				PlayersView);
		if (dwPlayerI == ROW_NO_MATCH) {bSuccess=false; goto Cleanup;}

		//Load in props from Players record.
		this->dwPlayerID = (DWORD) (p_PlayerID(PlayersView[dwPlayerI]));
		this->bIsLocal = ((UINT) p_IsLocal(PlayersView[dwPlayerI]) != 0);
		this->NameText.Bind( (DWORD) (p_NameMessageID(PlayersView[dwPlayerI])) );
		this->EMailText.Bind( (DWORD) (p_EMailMessageID(PlayersView[dwPlayerI])) );
		this->OriginalNameText.Bind( (DWORD)
				(p_GID_OriginalNameMessageID(PlayersView[dwPlayerI])) );
		this->Created = (time_t) (p_GID_Created(PlayersView[dwPlayerI]));
		this->LastUpdated = (time_t) (p_LastUpdated(PlayersView[dwPlayerI]));
		this->Settings = p_Settings(PlayersView[dwPlayerI]);
	}

Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
MESSAGE_ID CDbPlayer::SetProperty(
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
		case P_PlayerID:
		{
			this->dwPlayerID = static_cast<DWORD>(atol(str));
			if (!this->dwPlayerID)
				return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = info.PlayerIDMap.find(this->dwPlayerID);
			if (localID != info.PlayerIDMap.end())
				//Error - this player should not have been imported yet
				return MID_FileCorrupted;

			//Look up player in the DB.
			const DWORD dwLocalPlayerID = GetLocalID();
			if (dwLocalPlayerID)
			{
				//Player found in DB.
				info.PlayerIDMap[this->dwPlayerID] = dwLocalPlayerID;
				this->dwPlayerID = dwLocalPlayerID;

				c4_View PlayersView = GetView(ViewTypeStr(V_Players));
				const DWORD dwPlayerI = CDbBase::LookupRowByPrimaryKey(
						dwLocalPlayerID, p_PlayerID, PlayersView);
	         if (dwPlayerI == ROW_NO_MATCH) return MID_FileCorrupted;
            if (this->bIsLocal)
            {
               //Local players are only imported when it's their data being
               //imported, i.e., this is not just a player reference for
               //something like a hold author.
               ASSERT(info.typeBeingImported == CImportInfo::Player);
               const bool bOldPlayerIsLocal = 0 != p_IsLocal(PlayersView[dwPlayerI]);
               if (bOldPlayerIsLocal)
               {
				      //Don't replace a local player record unless it's older than
                  //the one being imported.
                  const time_t lastUpdated = (time_t)p_LastUpdated(PlayersView[dwPlayerI]);
				      if (lastUpdated >= this->LastUpdated)
                  {
                     //Mention the player to import is being skipped.
					      bSaveRecord = false;
                     return MID_PlayerIgnored;
                  }

                  //Newer player version is being imported -- prompt for overwrite confirmation.
                  if (!info.bReplaceOldPlayers)
                     return MID_OverwritePlayerPrompt;

                  //Remove old player from the DB.  This new one replaces it.
				      g_pTheDB->Players.Delete(dwLocalPlayerID);
               } else {
                  //The existing player record is non-local -- it must be
                  //updated no matter what (i.e., pretend the old one wasn't there).
				      g_pTheDB->Players.Delete(dwLocalPlayerID);
               }
               info.dwPlayerImportedID = this->dwPlayerID;   //keep track of player being imported
            } else {
               //A non-local player is being imported -- don't have to update it.
				   bSaveRecord = false;
            }
			} else {
				//Player not found -- add a new record to the DB.
				const DWORD dwOldLocalID = this->dwPlayerID;
				this->dwPlayerID = 0L;
				Update();
				info.PlayerIDMap[dwOldLocalID] = this->dwPlayerID;
            info.dwPlayerImportedID = this->dwPlayerID;   //keep track of player being imported
			}
			break;
		}
		case P_IsLocal:
			this->bIsLocal = (atoi(str) != 0);
			break;
		case P_NameMessage:
		{
			WSTRING data;
			Base64::decode(str,data);
			this->NameText = data.c_str();
			break;
		}
		case P_EMailMessage:
		{
			WSTRING data;
			Base64::decode(str,data);
			this->EMailText = data.c_str();
			break;
		}
		case P_GID_OriginalNameMessage:
		{
			WSTRING data;
			Base64::decode(str,data);
			this->OriginalNameText = data.c_str();
			break;
		}
		case P_GID_Created:
			this->Created = (time_t)atol(str);

         //No exported timestamp should ever be 0.
         if (this->Created == (time_t)0)
            return MID_FileCorrupted;
			break;
		case P_LastUpdated:
			this->LastUpdated = (time_t)atol(str);
			break;
		case P_Settings:
		{
			const string sstr = str;
			BYTE *data;
			Base64::decode(sstr,data);
			this->Settings = (const BYTE*)data;
			delete[] data;
			break;
		}
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbPlayer::Update()
//Updates database with player.
{
   g_pTheDB->Players.ResetMembership();
	if (this->dwPlayerID == 0)
		//Update a new player.
		return UpdateNew();
	else
		//Update existing player.
		return UpdateExisting();
}

//
//CDbPlayer private methods.
//

//*****************************************************************************
void CDbPlayer::Clear()
//Clears members of object.
{
	this->dwPlayerID = 0L;
	this->bIsLocal = true;
	this->NameText.Clear();
	this->EMailText.Clear();
	this->OriginalNameText.Clear();
	this->Created = this->LastUpdated = 0L;
	this->Settings.Clear();
}

//*****************************************************************************
DWORD CDbPlayer::GetLocalID()
//Compares this object's GID fields against those of the records in the DB.
//
//Returns: local ID if a record in the DB matches this object's GUID, else 0
{
	ASSERT(IsOpen());
	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayerCount = PlayersView.GetSize();

	//Each iteration checks a player's GIDs.
	for (DWORD dwPlayerI = 0; dwPlayerI < dwPlayerCount; ++dwPlayerI)
	{
		//Check time of creation.
		const time_t Created = (time_t)p_GID_Created(PlayersView[dwPlayerI]);
		if (this->Created == Created)
		{
			//Check original name.
         const DWORD dwOriginalNameMessageID = p_GID_OriginalNameMessageID(PlayersView[dwPlayerI]);
         CDbMessageText FromDbOriginalNameText( static_cast<MESSAGE_ID>(dwOriginalNameMessageID) );
			if (FromDbOriginalNameText == this->OriginalNameText)
			{
				//GUIDs match.  Return this record's local ID.
				return (DWORD) p_PlayerID(PlayersView[dwPlayerI]);
			}
		}
	}

	//No match.
	return 0L;
}

//*****************************************************************************
bool CDbPlayer::UpdateExisting()
//Update existing Players record in database.
{
	ASSERT(this->dwPlayerID != 0L);
	ASSERT(IsOpen());

	//Lookup Players record.
	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayerI = CDbBase::LookupRowByPrimaryKey(this->dwPlayerID,
			p_PlayerID, PlayersView);
	if (dwPlayerI == ROW_NO_MATCH)
	{
		ASSERTP(false, "The caller probably passed a bad PKID."); 
		return false;
	}

   if (!CDb::FreezingTimeStamps())
	   this->LastUpdated.SetToNow();

	//Get settings into a buffer that can be written to db.
	DWORD dwSettingsSize;
	BYTE *pbytSettingsBytes = this->Settings.GetPackedBuffer(dwSettingsSize);
	if (!pbytSettingsBytes) return false;
	c4_Bytes SettingsBytes(pbytSettingsBytes, dwSettingsSize);

	//Update Players record.
	p_IsLocal( PlayersView[ dwPlayerI ] ) = this->bIsLocal;
	p_LastUpdated( PlayersView[ dwPlayerI ] ) = this->LastUpdated;
	p_Settings( PlayersView[ dwPlayerI ] ) = SettingsBytes;

	//Update message texts.
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwEMailID = this->EMailText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwEMailID);
	//don't have to update this->OriginalNameText

	delete [] pbytSettingsBytes;

	return true;
}

//*****************************************************************************
bool CDbPlayer::UpdateNew()
//Add new Players record to database.
{
	ASSERT(this->dwPlayerID == 0L);
	ASSERT(CDbBase::IsOpen());

	//Prepare props.
	this->dwPlayerID = GetIncrementedID(p_PlayerID);
	if (this->OriginalNameText.IsEmpty())
		this->OriginalNameText = this->NameText;	//for GUID
	if ((time_t)this->Created == 0)
	{
		this->Created.SetToNow();
		this->LastUpdated.SetToNow();
	}

	//Get settings into a buffer that can be written to db.
	DWORD dwSettingsSize;
	BYTE *pbytSettingsBytes = this->Settings.GetPackedBuffer(dwSettingsSize);
	if (!pbytSettingsBytes) return false;
	c4_Bytes SettingsBytes(pbytSettingsBytes, dwSettingsSize);

	//Write out message texts.
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwEMailID = this->EMailText.Flush();
	const DWORD dwOrigNameID = this->OriginalNameText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwEMailID);
	ASSERT(dwOrigNameID);

	//Add record.
	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	PlayersView.Add(
		p_PlayerID[ this->dwPlayerID ] +
		p_IsLocal[ this->bIsLocal ] +
		p_NameMessageID[ dwNameID ] +
		p_EMailMessageID[ dwEMailID ] +
		p_GID_OriginalNameMessageID[ dwOrigNameID ] +
		p_GID_Created[ (time_t) this->Created ] +
		p_LastUpdated[ (time_t) this->LastUpdated ] +
		p_Settings[ SettingsBytes ] );

	delete [] pbytSettingsBytes;

	return true;
}

// $Log: DbPlayers.cpp,v $
// Revision 1.39  2005/03/15 21:51:12  mrimer
// Fixed memory leaks.
//
// Revision 1.38  2003/11/09 05:25:26  mrimer
// Made export release code more robust.
//
// Revision 1.37  2003/10/20 17:49:03  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.36  2003/10/07 21:10:34  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.35  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.34  2003/08/09 18:09:48  mrimer
// Fixed bug: Replaced assertions with more robust code catching DB row lookup failures to prevent crashes with corrutped data.
//
// Revision 1.33  2003/07/29 13:35:49  mrimer
// Fixed bug: hidden player demos not being exported.
//
// Revision 1.32  2003/07/19 02:33:19  mrimer
// Made export code more robust and maintainable.
//
// Revision 1.31  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.30  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.29  2003/07/09 21:23:51  mrimer
// Revised import routine to be more robust.
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.28  2003/07/07 23:44:37  mrimer
// Added more import handling logic.
//
// Revision 1.27  2003/07/06 04:54:50  mrimer
// Tweaking.
//
// Revision 1.26  2003/06/30 16:46:49  mrimer
// Fixed a bug.
//
// Revision 1.25  2003/06/26 17:35:21  mrimer
// Added check for importing corrupted player data.
//
// Revision 1.24  2003/06/25 17:03:01  mrimer
// Fixed bug: deleted player not being re-imported.
//
// Revision 1.23  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.22  2003/06/24 20:12:10  mrimer
// Augmented filtering to optionally include hidden saved games.
// Fixed some bugs relating to this (in export and player deletion).
//
// Revision 1.21  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.20  2003/06/17 22:14:55  mrimer
// Tweaking.
//
// Revision 1.19  2003/06/03 06:21:20  mrimer
// Now calls ResetMembership() on DB update.
//
// Revision 1.18  2003/05/25 22:46:26  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.17  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.16  2003/05/22 23:39:02  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.15  2003/05/20 18:12:41  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.14  2003/05/08 23:21:01  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.13  2003/05/08 22:01:08  mrimer
// Replaced local CDb instances with a pointer to global instance.
//
// Revision 1.12  2003/04/28 22:19:33  mrimer
// Added a missing field to UpdateExisting() and Clear().
//
// Revision 1.11  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.10  2003/02/24 17:06:26  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.9  2003/02/16 20:29:32  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.8  2002/12/22 02:02:04  mrimer
// Added XML import/export support.  (Added GUID support.)
// Revised Delete() to remove demos and saved games, but retain non-local player record.
//
// Revision 1.7  2002/11/22 22:00:20  mrimer
// Fixed a subtle bug with membership loading.
//
// Revision 1.6  2002/11/22 02:03:05  mrimer
// Now adding a new player doesn't add a continue slot.
// Now deleting a player deletes all their continue slots.
//
// Revision 1.5  2002/11/14 19:10:21  mrimer
// Added Delete().
// Moved member initialization into constructor initialization list.
// Made some parameters const.
//
// Revision 1.4  2002/10/22 05:28:08  mrimer
// Revised includes.
//
// Revision 1.3  2002/08/30 00:23:43  erikh2000
// New players can be added to database now.
// Wrote method to find a player by name.
//
// Revision 1.2  2002/06/15 18:29:54  erikh2000
// Changed code so that storage is not committed after each database write.
//
// Revision 1.1  2002/06/09 05:58:56  erikh2000
// Initial check-in.
//
