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

#include "Base64.h"
#include "DbPlayers.h"
#include "DbProps.h"

#ifndef WIN32
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

	c4_View PlayersView = GetView("Players");
	const DWORD dwPlayerRowI = LookupRowByPrimaryKey(dwPlayerID, p_PlayerID, PlayersView);
	if (dwPlayerRowI == ROW_NO_MATCH) {ASSERT(false); return;} //Bad player ID.

	if (!bRetainRef)
	{
		//Delete name and email message texts.
		const DWORD dwNameMID = p_NameMessageID( PlayersView[dwPlayerRowI] );
		if (!dwNameMID) {ASSERT(false); return;}
		DeleteMessage(dwNameMID);
		const DWORD dwEMailMID = p_EMailMessageID( PlayersView[dwPlayerRowI] );
		if (!dwEMailMID) {ASSERT(false); return;}
		DeleteMessage(dwEMailMID);
		const DWORD dwOrigNameMID = p_GID_OriginalNameMessageID( PlayersView[dwPlayerRowI] );
		if (!dwOrigNameMID) {ASSERT(false); return;}
		DeleteMessage(dwOrigNameMID);
	}

	//Delete player's demos first and then saved games.
	CIDList DemoIDs, SavedGameIDs;
	IDNODE *pSeek;

	CDb db;
	db.Demos.FilterByPlayer(dwPlayerID);
	db.Demos.GetIDs(DemoIDs);
	pSeek = DemoIDs.Get(0);
	while (pSeek)
	{
		db.Demos.Delete(pSeek->dwID);
		pSeek=pSeek->pNext;
	}
	db.SavedGames.FilterByPlayer(dwPlayerID);
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
	string str;

	if (!dbRefs.IsSet(V_Players,dwPlayerID))
	{
		dbRefs.Set(V_Players,dwPlayerID);

		CDbPlayer *pPlayer = GetByID(dwPlayerID);
		ASSERT(pPlayer);

		//Prepare data.
		WSTRING const wNameStr = pPlayer->NameText;
		WSTRING const wEMailStr = pPlayer->EMailText;
		WSTRING const wOrigNameStr = pPlayer->OriginalNameText;
		char dummy[32];

		str += "<Players GID_OriginalNameMessage='";
		str += Base64::encode(wOrigNameStr);
		str += "' GID_Created='";
		str += _ltoa((time_t)pPlayer->Created, dummy, 10);
		str += "' LastUpdated='";
		if (bRef)
			str += "0";	//player reference -- don't overwrite anything
		else
			str += _ltoa((time_t)pPlayer->LastUpdated, dummy, 10);
		str += "' NameMessage='";
		str += Base64::encode(wNameStr);
		str += "' EMailMessage='";
		str += Base64::encode(wEMailStr);
		str += "' IsLocal='";
		if (bRef)
			str += "0";	//player reference  -- don't display 
		else
			str += _ltoa((long)pPlayer->bIsLocal, dummy, 10);
		//Put primary key last, so all message fields have been set by the
		//time Update() is called during import.
		str += "' PlayerID='";
		str += _ltoa(pPlayer->dwPlayerID, dummy, 10);
		if (bRef)
		{
			//Don't need any further information for a player reference.
			str += "'/>\r\n";
		} else {

			{
				DWORD dwBufSize;
				BYTE *pSettings = pPlayer->Settings.GetPackedBuffer(dwBufSize);
				if (dwBufSize > 4)
				{
					str += "' Settings='";
					str += Base64::encode(pSettings,dwBufSize-4);	//remove null UINT
				}
				delete pSettings;
			}

			str += "'>\r\n";

			CDb db;
			//Export player's demos.
			CIDList DemoIDs;
			db.Demos.FilterByPlayer(dwPlayerID);
			db.Demos.GetIDs(DemoIDs);
			const DWORD dwNumDemos = DemoIDs.GetSize();
			for (DWORD dwIndex=0; dwIndex<dwNumDemos; ++dwIndex)
			{
				str += db.Demos.ExportXML(DemoIDs.Get(dwIndex)->dwID, dbRefs);
			}

			//Export player's saved games (not attached to demos).
			CIDList SavedGameIDs;
			db.SavedGames.FilterByPlayer(dwPlayerID);
			db.SavedGames.GetIDs(SavedGameIDs);
			const DWORD dwNumSavedGames = SavedGameIDs.GetSize();
			for (dwIndex=0; dwIndex<dwNumSavedGames; ++dwIndex)
			{
				str += db.SavedGames.ExportXML(SavedGameIDs.Get(dwIndex)->dwID, dbRefs);
			}

			str += "</Players>\r\n";
		}

		delete pPlayer;
	}

	return str;
}

//*****************************************************************************
void CDbPlayers::FilterByLocal(void)
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
	ASSERT(CDbBase::IsOpen());
	c4_View PlayersView = GetView("Players");
	const DWORD dwPlayerCount = PlayersView.GetSize();
	
	//Each iteration checks a player name from one record.
	for (DWORD dwPlayerI = 0; dwPlayerI < dwPlayerCount; ++dwPlayerI)
	{
		CDbMessageText NameText;
		NameText.Bind( p_NameMessageID(PlayersView[dwPlayerI]) );
		if (wcscmp(pwczName, (const WCHAR *) NameText)==0)
			return (DWORD) p_PlayerID(PlayersView[dwPlayerI]); //Found it.
	}
	
	//No match.
	return 0L;
}

//*****************************************************************************
CDbPlayer * CDbPlayers::GetNew(void)
//Get a new player object that will be added to database when it is updated.
//
//Returns:
//Pointer to new player.
{
	//After player object is updated, membership changes, so reset the flag.
	this->bIsMembershipLoaded = false;

	//Return new player object.
	return new CDbPlayer;	
}

//*****************************************************************************
CDbPlayer * CDbPlayers::GetByID(
//Get a player by its PlayerID.
//
//Params:
	const DWORD dwPlayerID)	//(in)
//
//Returns:
//Pointer to loaded player which caller must delete, or NULL if no matching
//player was found.
{
	CDbPlayer *pPlayer = new CDbPlayer();
	if (pPlayer)
	{
		if (!pPlayer->Load(dwPlayerID))
		{
			delete pPlayer;
			pPlayer=NULL;
		}
	}
	return pPlayer;
}

//*****************************************************************************
CDbPlayer * CDbPlayers::GetFirst(void)
//Gets first player.  A subsequent call to GetNext() will retrieve the second
//player.
//
//Returns:
//Pointer to loaded player which caller must delete, or NULL if no matching
//player was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//Set current row to first.
	this->pCurrentRow = this->MembershipIDs.Get(0);
	
	//If there are no rows, then just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load player.
	CDbPlayer *pPlayer = GetByID(this->pCurrentRow->dwID);
	
	//Advance row to next.
	this->pCurrentRow = this->pCurrentRow->pNext;

	return pPlayer;
}

//*****************************************************************************
CDbPlayer * CDbPlayers::GetNext(void)
//Gets next player.
//
//Returns:
//Pointer to loaded player which caller must delete, or NULL if no matching
//player was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//If at end of rows, just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load player.
	CDbPlayer *pPlayer = GetByID(pCurrentRow->dwID);
	
	//Advance row to next.
	pCurrentRow = pCurrentRow->pNext;

	return pPlayer;
}

//
//CDbPlayers private methods.
//

//*****************************************************************************
void CDbPlayers::LoadMembership(void)
//Load the membership list with all player IDs.
{
	ASSERT(CDbBase::IsOpen());
	c4_View PlayersView = CDbBase::GetView("Players");
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

	ASSERT(CDbBase::IsOpen());

	{
		//Open Players view.
		c4_View PlayersView = CDbBase::GetView("Players");
		const DWORD dwPlayerCount = PlayersView.GetSize();

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
MESSAGE_ID CDbPlayer::SetProp(
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
		case P_PlayerID:
		{
			this->dwPlayerID = static_cast<DWORD>(atol(str));
			if (!this->dwPlayerID)
				return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = Maps.PlayerIDMap.find(this->dwPlayerID);
			if (localID != Maps.PlayerIDMap.end())
				//Error - this player should not have been imported yet
				return MID_FileCorrupted;

			//Look up player in the DB.
			const DWORD dwLocalPlayerID = GetLocalID();
			if (dwLocalPlayerID)
			{
				//Player found in DB.
				Maps.PlayerIDMap[this->dwPlayerID] = dwLocalPlayerID;
				this->dwPlayerID = dwLocalPlayerID;

				//Don't update player settings if this one's older.
				c4_View PlayersView = CDbBase::GetView("Players");
				const DWORD dwPlayerI = CDbBase::LookupRowByPrimaryKey(
						dwLocalPlayerID, p_PlayerID, PlayersView);
				if (this->LastUpdated <= (time_t)p_LastUpdated(
						PlayersView[dwPlayerI]))
					bSaveRecord = false;
			} else {
				//Player not found -- add a new record to the DB.
				const DWORD dwOldLocalID = this->dwPlayerID;
				this->dwPlayerID = 0L;
				Update();
				Maps.PlayerIDMap[dwOldLocalID] = this->dwPlayerID;
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
			delete data;
			break;
		}
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbPlayer::Update(void)
//Updates database with player.
{
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
void CDbPlayer::Clear(void)
//Clears members of object.
{
	this->dwPlayerID = 0L;
	this->bIsLocal = true;
	this->NameText.Clear();
	this->EMailText.Clear();
	this->OriginalNameText.Clear();
	this->Created = 0L;
	this->Settings.Clear();
}

//*****************************************************************************
DWORD CDbPlayer::GetLocalID(void)
//Compares this object's GID fields against those of the records in the DB.
//
//Returns: local ID if a record in the DB matches this object's GUID, else 0
{
	ASSERT(CDbBase::IsOpen());
	c4_View PlayersView = CDbBase::GetView("Players");
	const DWORD dwPlayerCount = PlayersView.GetSize();

	//Each iteration checks a hold's GIDs.
	for (DWORD dwPlayerI = 0; dwPlayerI < dwPlayerCount; ++dwPlayerI)
	{
		//Check time of creation.
		time_t Created = (time_t)p_GID_Created(PlayersView[dwPlayerI]);
		if (this->Created == Created)
		{
			//Check original name.
			const WCHAR *pwczText = GetMessageText((DWORD)
					(p_GID_OriginalNameMessageID(PlayersView[dwPlayerI])));
			if (!wcscmp(pwczText,(const WCHAR*)this->OriginalNameText))
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
bool CDbPlayer::UpdateExisting(void)
//Update existing Players record in database.
{
	ASSERT(this->dwPlayerID != 0L);
	ASSERT(CDbBase::IsOpen());

	//Lookup Players record.
	c4_View PlayersView = GetView("Players");
	const DWORD dwPlayerI = CDbBase::LookupRowByPrimaryKey(this->dwPlayerID,
			p_PlayerID, PlayersView);
	if (dwPlayerI == ROW_NO_MATCH)
	{
		ASSERT(false); //The caller probably passed a bad PKID.
		return false;
	}

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
bool CDbPlayer::UpdateNew(void)
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
	c4_View PlayersView = GetView("Players");
	PlayersView.Add(
		p_PlayerID[ this->dwPlayerID ] +
		p_IsLocal[ this->bIsLocal ] +
		p_NameMessageID[ dwNameID ] +
		p_EMailMessageID[ dwEMailID ] +
		p_GID_OriginalNameMessageID[ dwOrigNameID ] +
		p_GID_Created[ (time_t) this->Created ] +
		p_Settings[ SettingsBytes ] );

	delete [] pbytSettingsBytes;

	return true;
}

// $Log: DbPlayers.cpp,v $
// Revision 1.1  2003/02/25 00:01:30  erikh2000
// Initial check-in.
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
