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

#include "Base64.h"
#include "Db.h"
#include "DbProps.h"
#include "GameConstants.h"

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

	c4_View HoldsView = GetView("Holds");
	const DWORD dwHoldRowI = LookupRowByPrimaryKey(dwHoldID, p_HoldID, HoldsView);
	if (dwHoldRowI == ROW_NO_MATCH) {ASSERT(false); return;} //Bad hold ID.

	//Delete all levels in hold (and their rooms, saved games and demos.)
	CDb db;
	CIDList LevelIDs;
	IDNODE *pSeek;

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
	if (!dwNameMID) {ASSERT(false); return;}
	DeleteMessage(dwNameMID);
	const DWORD dwDescriptionMID = p_DescriptionMessageID( HoldsView[dwHoldRowI] );
	if (!dwDescriptionMID) {ASSERT(false); return;}
	DeleteMessage(dwDescriptionMID);

	//Delete the hold.
	HoldsView.RemoveAt(dwHoldRowI);

	//After hold object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//*****************************************************************************
string CDbHolds::ExportXML(
//Returns: string containing XML text describing hold with this ID
//				AND all levels having this HoldID
//
//Pre-condition: dwHoldID is valid
//
//Params:
	const DWORD dwHoldID,	//(in)
	CDbRefs &dbRefs,			//(in/out)
	const bool bRef)			//(in) Only export GUID reference (default = false)
{
	string str;

	if (!dbRefs.IsSet(V_Holds,dwHoldID))
	{
		dbRefs.Set(V_Holds,dwHoldID);

		char dummy[32];
		CDbHold *pHold = GetByID(dwHoldID);
		ASSERT(pHold);

		//Include corresponding GID player ref.
		CDb db;
		str += db.Players.ExportXML(pHold->dwPlayerID, dbRefs, true);

		str += "<Holds GID_Created='";
		str += _ltoa((time_t)pHold->Created, dummy, 10);
		str += "' GID_PlayerID='";
		str += _ltoa(pHold->dwPlayerID, dummy, 10);
		str += "' LastUpdated='";
		if (bRef)
		{
			//Don't need any further information for a hold reference.
			str += "0";	//0 value ensures hold won't overwrite another
		} else {
			//Prepare data.
			WSTRING const wNameStr = pHold->NameText;
			WSTRING const wDescStr = pHold->DescriptionText;

			str += _ltoa((time_t)pHold->LastUpdated, dummy, 10);
			str += "' NameMessage='";
			str += Base64::encode(wNameStr);
			str += "' DescriptionMessage='";
			str += Base64::encode(wDescStr);
			str += "' LevelID='";
			str += _ltoa(pHold->dwLevelID, dummy, 10);
			str += "' RequiredDRODVersion='";
			str += _ltoa(pHold->dwRequiredDRODVersion, dummy, 10);
			str += "' GID_NewLevelIndex='";
			str += _ltoa(pHold->dwNewLevelIndex, dummy, 10);
		}
		//Put primary key last, so all message fields have been set by the
		//time Update() is called during import.
		str += "' HoldID='";
		str += _ltoa(pHold->dwHoldID, dummy, 10);

		if (bRef)
			str += "'/>\r\n";
		else
		{
			str += "'>\r\n";

			//Export all levels in hold.
			CIDList LevelIDs;
			db.Levels.FilterBy(dwHoldID);
			db.Levels.GetIDs(LevelIDs);
			const DWORD dwNumLevels = LevelIDs.GetSize();
			for (DWORD dwIndex=0; dwIndex<dwNumLevels; ++dwIndex)
			{
				str += db.Levels.ExportXML(LevelIDs.Get(dwIndex)->dwID, dbRefs);
			}

			//Export all display demos in hold.
			CIDList DemoIDs;
			db.Demos.FilterByShow();
			db.Demos.GetIDs(DemoIDs);
			const DWORD dwNumDemos = DemoIDs.GetSize();
			for (dwIndex=0; dwIndex<dwNumDemos; ++dwIndex)
			{
				str += db.Demos.ExportXML(DemoIDs.Get(dwIndex)->dwID, dbRefs);
			}

			str += "</Holds>\r\n";
		}

		delete pHold;
	}

	return str;
}

//*****************************************************************************
CDbHold * CDbHolds::GetByID(
//Get a hold by its HoldID.
//
//Params:
	const DWORD dwHoldID)	//(in)
//
//Returns:
//Pointer to loaded hold which caller must delete, or NULL if no matching hold 
//was found.
{
	CDbHold *pHold = new CDbHold();
	if (pHold)
	{
		if (!pHold->Load(dwHoldID))
		{
			delete pHold;
			pHold=NULL;
		}
	}
	return pHold;
}

//*****************************************************************************
CDbHold * CDbHolds::GetFirst(void)
//Gets first hold.  A subsequent call to GetNext() will retrieve the second hold.
//
//Returns:
//Pointer to loaded hold which caller must delete, or NULL if no matching hold
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//Set current row to first.
	this->pCurrentRow = this->MembershipIDs.Get(0);
	
	//If there are no rows, then just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load hold.
	CDbHold *pHold = GetByID(this->pCurrentRow->dwID);
	
	//Advance row to next.
	this->pCurrentRow = this->pCurrentRow->pNext;

	return pHold;
}

//*****************************************************************************
CDbHold * CDbHolds::GetNext(void)
//Gets next hold.
//
//Returns:
//Pointer to loaded hold which caller must delete, or NULL if no matching hold
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//If at end of rows, just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load hold.
	CDbHold *pHold = GetByID(pCurrentRow->dwID);
	
	//Advance row to next.
	pCurrentRow = pCurrentRow->pNext;

	return pHold;
}

//*****************************************************************************
DWORD	CDbHolds::GetNewLevelIndex(
//Returns: the NewLevelIndex for the hold with this HoldID,
//				or 0 if hold with this ID is not found.
//
//Increments hold's NewLevelIndex.
//
//Params:
	const DWORD dwHoldID)	//(in) 
{
	CDbHold *pHold = GetByID(dwHoldID);
	if (!pHold) return 0;

	const DWORD dwNextLevelIndex = pHold->dwNewLevelIndex++;
	pHold->Update();
	delete pHold;

	return dwNextLevelIndex;
}

//*****************************************************************************
CDbHold * CDbHolds::GetNew(void)
//Get a new hold object that will be added to database when it is updated.
//
//Returns:
//Pointer to new hold.
{
	//After hold object is updated, membership changes, so reset the flag.
	this->bIsMembershipLoaded = false;

	//Return new hold object.
	return new CDbHold;	
}

//
//CDbHolds private methods.
//

//*****************************************************************************
void CDbHolds::LoadMembership(void)
//Load the membership list with all hold IDs.
{
	ASSERT(CDbBase::IsOpen());
	c4_View HoldsView = CDbBase::GetView("Holds");
	const DWORD dwHoldCount = HoldsView.GetSize();

	//Each iteration gets a hold ID and puts in membership list.
	this->MembershipIDs.Clear();
	for (DWORD dwHoldI = 0; dwHoldI < dwHoldCount; ++dwHoldI)
	{
		this->MembershipIDs.Add(p_HoldID(HoldsView[dwHoldI]));
	}
	this->pCurrentRow = this->MembershipIDs.Get(0);
	this->bIsMembershipLoaded = true;
}

//
//CDbHold protected methods.
//

//*****************************************************************************
CDbHold::CDbHold(void)
//Constructor.
{
	Clear();
}

//
//CDbHold public methods.
//

//*****************************************************************************
CDbHold::~CDbHold(void)
//Destructor.
{
	Clear();
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
	ASSERT(CDbBase::IsOpen());
	c4_View HoldsView = CDbBase::GetView("Holds");

	//Find record with matching hold ID.
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
	this->dwRequiredDRODVersion = (DWORD) (p_RequiredDRODVersion(HoldsView[dwHoldI]));
	this->dwNewLevelIndex = (DWORD) (p_GID_NewLevelIndex(HoldsView[dwHoldI]));

	//Set filter for Levels to show only levels in this hold.
	this->Levels.FilterBy(this->dwHoldID);

	//Set filter for SavedGames to show only saved games in this hold.
	this->SavedGames.FilterByHold(this->dwHoldID);
	
Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*****************************************************************************
CDbLevel * CDbHold::GetStartingLevel(void)
//Gets starting level for this hold.
//
//Returns:
//Pointer to new loaded level object which caller must delete, or NULL if level
//could not be loaded.
const
{
	if (this->dwLevelID==0) return NULL;

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
MESSAGE_ID CDbHold::SetProp(
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
		case P_HoldID:
		{
			this->dwHoldID = static_cast<DWORD>(atol(str));
			if (!this->dwHoldID)
				return MID_FileCorrupted;	//corrupt file

			//Look up local ID.
			localID = Maps.HoldIDMap.find(this->dwHoldID);
			if (localID != Maps.HoldIDMap.end())
				//Error - this hold should not have been imported already.
				return MID_FileCorrupted;

			//Look up hold in the DB.
			const DWORD dwLocalHoldID = GetLocalID();
			if (dwLocalHoldID)
			{
				//Hold found in DB.
				Maps.HoldIDMap[this->dwHoldID] = dwLocalHoldID;
				this->dwHoldID = dwLocalHoldID;

				//Don't load this hold if it's an older version.
				c4_View HoldsView = CDbBase::GetView("Holds");
				const DWORD dwHoldI = CDbBase::LookupRowByPrimaryKey(
						dwLocalHoldID, p_HoldID, HoldsView);
				if (this->LastUpdated <= (time_t)p_LastUpdated(
						HoldsView[dwHoldI]))
					bSaveRecord = false;
				else
				{
					//Remove old hold from the DB.  This new one replaces it.
					CDb db;
					db.Holds.Delete(dwLocalHoldID);
				}
			} else {
				//Hold not found -- add a new record to the DB.
				const DWORD dwOldLocalID = this->dwHoldID;
				this->dwHoldID = 0L;
				Update();
				Maps.HoldIDMap[dwOldLocalID] = this->dwHoldID;
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
			localID = Maps.PlayerIDMap.find(this->dwPlayerID);
			if (localID == Maps.PlayerIDMap.end())
				return MID_FileCorrupted;	//record should have been loaded already
			this->dwPlayerID = (*localID).second;
			break;
		case P_RequiredDRODVersion:
			this->dwRequiredDRODVersion = static_cast<DWORD>(atol(str));
			break;
		case P_GID_NewLevelIndex:
			this->dwNewLevelIndex = static_cast<DWORD>(atol(str));
			if (!this->dwNewLevelIndex)
				return MID_FileCorrupted;	//corrupt file
			break;
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbHold::Update(void)
//Updates database with hold.
//
//Returns: true if successful, else false.
{
	bool bSuccess=true;

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
DWORD CDbHold::GetLocalID(void) const
//Compares this object's GID fields against those of the records in the DB.
//ASSUME: dwPlayerID has already been set to the local record ID
//
//Returns: local ID if a record in the DB matches this object's GUID, else 0
{
	ASSERT(CDbBase::IsOpen());
	c4_View HoldsView = CDbBase::GetView("Holds");
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
bool CDbHold::UpdateNew(void)
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
	ASSERT(dwNameID);
	ASSERT(dwDescID);

	//Write new Hold record.
	c4_View HoldsView = GetView("Holds");
	HoldsView.Add(
			p_HoldID[ this->dwHoldID ] +
			p_NameMessageID[ dwNameID ] +
			p_DescriptionMessageID[ dwDescID ] +
			p_LevelID[ this->dwLevelID ] +
			p_GID_Created[ this->Created ] +
			p_LastUpdated[ this->LastUpdated ] +
			p_GID_PlayerID[ this->dwPlayerID ] +
			p_RequiredDRODVersion[ dwCurrentDRODVersion ] +
			p_GID_NewLevelIndex[ 1 ]);

	return true;
}

//*****************************************************************************
bool CDbHold::UpdateExisting(void)
//Update an existing Holds record in database.
{
	ASSERT(this->dwHoldID != 0);
	ASSERT(IsOpen());

	//Lookup Holds record.
	c4_View HoldsView = GetView("Holds");
	const DWORD dwHoldID = LookupRowByPrimaryKey(this->dwHoldID,
			p_HoldID, HoldsView);
	if (dwHoldID == ROW_NO_MATCH)
	{
		ASSERT(false); //The caller probably passed a bad PKID.
		return false;
	}

	//Prepare props.
	this->LastUpdated.SetToNow();
	const DWORD dwNameID = this->NameText.Flush();
	const DWORD dwDescID = this->DescriptionText.Flush();
	ASSERT(dwNameID);
	ASSERT(dwDescID);

	//Update Holds record.
	p_HoldID( HoldsView[ dwHoldID ] ) = this->dwHoldID;
	p_LevelID( HoldsView[ dwHoldID ] ) = this->dwLevelID;
	p_LastUpdated( HoldsView[ dwHoldID ] ) = this->LastUpdated;
	p_RequiredDRODVersion( HoldsView[ dwHoldID ] ) = this->dwRequiredDRODVersion;
	p_GID_NewLevelIndex( HoldsView[ dwHoldID ] ) = this->dwNewLevelIndex;

	return true;
}

//*****************************************************************************
void CDbHold::Clear(void)
//Frees resources associated with this object and resets member vars.
{
	this->dwLevelID = this->dwHoldID = this->dwPlayerID = 0L;

	this->NameText.Clear();
	this->DescriptionText.Clear();
	this->Created = this->LastUpdated = 0L;

	this->dwRequiredDRODVersion = 0L;
	this->dwNewLevelIndex = 0L;
}

// $Log: DbHolds.cpp,v $
// Revision 1.1  2003/02/25 00:01:29  erikh2000
// Initial check-in.
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
