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

#include "DbXML.h"
#include "Files.h"
#include "StretchyBuffer.h"

//#include "zlib.h"

#include <stdio.h>

//Static vars
vector <CDbBase*> CDbXML::dbRecordStack;
vector <CDbBase*> CDbXML::dbImportedRecords;
vector <VIEWTYPE> CDbXML::dbRecordTypes;
vector <bool>  CDbXML::SaveRecord;
vector <VIEWPROPTYPE> CDbXML::vpCurrentType;
MESSAGE_ID CDbXML::ImportStatus=MID_ImportSuccessful;

//Local vars
PrimaryKeyMaps IDMaps;

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
	CDb db;

	switch (vType)
	{
		//These are the only classes we are considering.
		case V_Demos:
			return db.Demos.GetNew();
		case V_Holds:
			return db.Holds.GetNew();
		case V_Levels:
			return db.Levels.GetNew();
		case V_Players:
			return db.Players.GetNew();
		case V_Rooms:
			return db.Rooms.GetNew();
		case V_SavedGames:
			return db.SavedGames.GetNew();

		default:
			ASSERT(false);
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
void CDbXML::StartElement(void *userData, const char *name, const char **atts)
//Expat callback function
//
//Process XML start tag, and attributes.
{
	CDbBase *pDbBase;
	int i;

	//Get view type.
	const VIEWTYPE vType = ParseViewType(name);
	if (vType != V_Invalid)
	{
		//Create new object (record) to insert into DB.
		pDbBase = GetNewRecord(vType);
		SaveRecord.push_back(true);

		//Set members.
		for (i = 0; atts[i]; i += 2) {
			const PROPTYPE pType = ParsePropType(atts[i]);
			if (pType == P_Invalid)
			{
				//Invalid tag -- Fail import
				ImportStatus = MID_FileCorrupted;
				return;
			}
			ImportStatus = pDbBase->SetProp(pType, (char* const)atts[i + 1], IDMaps,
					SaveRecord.back());
			if (ImportStatus != MID_ImportSuccessful)
				return;
		}

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
				//Notify sub-view that members will be set.
				ImportStatus = (dbRecordStack.back())->SetProp(vpType, P_Start, NULL,
						IDMaps);

				//Set members.
				for (i = 0; atts[i]; i += 2) {
					const PROPTYPE pType = ParsePropType(atts[i]);
					ImportStatus = (dbRecordStack.back())->SetProp(vpType, pType,
							(char* const)atts[i + 1], IDMaps);
					if (ImportStatus != MID_ImportSuccessful)
						return;
				}
				break;

			case VP_Invalid:
				if (!strcmp("DROD",name))	//ignore top-level header
					return;
				//no break
			default:
				//Invalid tag -- Fail import
				ImportStatus = MID_FileCorrupted;
				return;
		}
		vpCurrentType.push_back(vpType);
	}
}

//*****************************************************************************
void CDbXML::InElement(void *userData, const XML_Char *s, int len)
//Expat callback function.
//
//Process text between XML tags.
{
	if (vpCurrentType.size() == 0) return;	//nothing to handle here
}

//*****************************************************************************
void CDbXML::EndElement(void *userData, const char *name)
//Expat callback function
//
//Process XML end tag.
{
	//Verify matching tags.
	if (vpCurrentType.size() > 0)
	{
		//End of sub-view.
		const VIEWPROPTYPE vpType = ParseViewpropType(name);
		if (vpType != vpCurrentType.back())
		{
			//Mismatched tags -- Fail import
			ImportStatus = MID_FileCorrupted;
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
				ImportStatus =
						(dbRecordStack.back())->SetProp(vpType, P_End, NULL, IDMaps);
				if (ImportStatus != MID_ImportSuccessful)
					return;
				break;
			}
			default:
				//Invalid tag -- Fail import
				ImportStatus = MID_FileCorrupted;
				return;
		}
		vpCurrentType.pop_back();
	} else {
		//End of record.
		const VIEWTYPE vType = ParseViewType(name);
		if (vType == V_Invalid)
		{
			if (!strcmp("DROD",name))	//ignore top-level header
				return;
			//Bad tag -- Fail import
			ImportStatus = MID_FileCorrupted;
			return;
		} else {
			//Remove record object from the stack.
			CDbBase *pDbBase = dbRecordStack.back();
			dbRecordStack.pop_back();
			if (!pDbBase)
			{
				//Unmatched tags -- Fail import
				ImportStatus = MID_FileCorrupted;
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

//*****************************************************************************
void CDbXML::UpdateLocalIDs()
//For some imported records, old local IDs couldn't be resolved.
//Once all records have been read in, this method is called to transform
//remaining old local IDs to new local IDs.
{
	CDbBase *pDbBase;
	PrimaryKeyMap::iterator localID;

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
 				CDbHold *pHold = dynamic_cast<CDbHold *>(pDbBase);
				ASSERT(pHold);

				//Update first level ID.
				localID = IDMaps.LevelIDMap.find(pHold->dwLevelID);
				if (localID ==	IDMaps.LevelIDMap.end())
				{
					//ID not found -- Fail import
					ImportStatus = MID_LevelNotFound;
					return;
				}
				pHold->dwLevelID = (*localID).second;
				break;
			}
			case V_Levels:
			{
 				CDbLevel *pLevel = dynamic_cast<CDbLevel *>(pDbBase);
				ASSERT(pLevel);

				//Update first room ID.
				localID = IDMaps.RoomIDMap.find(pLevel->dwRoomID);
				if (localID ==	IDMaps.RoomIDMap.end())
				{
					//ID not found -- Fail import
					ImportStatus = MID_FileCorrupted;
					return;
				}
				pLevel->dwRoomID = (*localID).second;
				break;
			}

			case V_Rooms:
			{
 				CDbRoom *pRoom = dynamic_cast<CDbRoom *>(pDbBase);
				ASSERT(pRoom);

				//Update exit IDs.
				const UINT wCount = pRoom->wExitCount;
				for (UINT wIndex=0; wIndex<wCount; ++wIndex)
				{
					localID = IDMaps.LevelIDMap.find(pRoom->
							parrExits[wIndex].dwLevelID);
					if (localID ==	IDMaps.LevelIDMap.end())
					{
						//ID not found -- Fail import
						ImportStatus = MID_LevelNotFound;
						return;
					}
					pRoom->parrExits[wIndex].dwLevelID = (*localID).second;
				}
				break;
			}

			case V_SavedGames:
			{
 				CDbSavedGame *pSavedGame = dynamic_cast<CDbSavedGame *>(pDbBase);
				ASSERT(pSavedGame);

				//Update IDs for ExploredRooms and ConqueredRooms.
				IDNODE *pIDNode = pSavedGame->ExploredRooms.Get(0);
				while (pIDNode)
				{
					localID = IDMaps.RoomIDMap.find(pIDNode->dwID);
					if (localID ==	IDMaps.RoomIDMap.end())
					{
						//ID not found -- Fail import
						ImportStatus = MID_FileCorrupted;
						return;
					}
					pIDNode->dwID = (*localID).second;
					pIDNode = pIDNode->pNext;
				}
				pIDNode = pSavedGame->ConqueredRooms.Get(0);
				while (pIDNode)
				{
					localID = IDMaps.RoomIDMap.find(pIDNode->dwID);
					if (localID ==	IDMaps.RoomIDMap.end())
					{
						//ID not found -- Fail import
						ImportStatus = MID_FileCorrupted;
						return;
					}
					pIDNode->dwID = (*localID).second;
					pIDNode = pIDNode->pNext;
				}
				break;
			}

			default:
				break;	//other types need no fix-ups
		}

		//Save any changes.
		pDbBase->Update();
		delete pDbBase;
	}
}

//
//CDbXML private methods.
//

//*****************************************************************************
MESSAGE_ID CDbXML::ImportXML(
//Import an XML file into one or more tables.
//
//Params:
	const char *pszFilename)	//(in)	XML filename, not including extension or path.
								//		File will be found at 
								//		"{data path}\Import\{filename}.xml".
//
//Returns:
//Message ID giving the concluding status of the operation, i.e.
//	MID_ImportSuccessful if it worked,
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

	FILE *fp = fopen(pszFilename,"r");
	if (!fp)
		return MID_FileNotFound;

//ZEXTERN int ZEXPORT uncompress OF((Bytef *dest,   uLongf *destLen,
//                                   const Bytef *source, uLong sourceLen));
/*
     Decompresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer. Upon entry, destLen is the total
   size of the destination buffer, which must be large enough to hold the
   entire uncompressed data. (The size of the uncompressed data must have
   been saved previously by the compressor and transmitted to the decompressor
   by some mechanism outside the scope of this compression library.)
   Upon exit, destLen is the actual size of the compressed buffer.
     This function can be used to decompress a whole file at once if the
   input file is mmap'ed.

     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer, or Z_DATA_ERROR if the input data was corrupted.
*/

	char buf[BUFSIZ];
	int done;
	XML_Parser parser = XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, CDbXML::StartElement, CDbXML::EndElement);
	XML_SetCharacterDataHandler(parser, CDbXML::InElement);

	//Ensure everything is reset.
	ASSERT(dbRecordStack.size() == 0);
	ASSERT(dbImportedRecords.size() == 0);
	ASSERT(dbRecordTypes.size() == 0);
	ASSERT(vpCurrentType.size() == 0);

	ImportStatus = MID_ImportSuccessful;
	do {
		//IMPORT A FILE
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = (len < sizeof(buf) ? 1 : 0);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			//Some problem occured.
			char errorStr[256];
			sprintf(errorStr,
					"%s at line %d\n",
					XML_ErrorString(XML_GetErrorCode(parser)),
					XML_GetCurrentLineNumber(parser));
			CFiles::AppendErrorLog((char *)errorStr);

			ImportStatus=MID_FileCorrupted;
			goto Cleanup;
		}
	} while (!done && ImportStatus==MID_ImportSuccessful);
	XML_ParserFree(parser);

Cleanup:
	fclose(fp);

	//Now that all the records have been read through, we can
	//update all remaining old local keys (IDs) to new local keys.
	if (ImportStatus==MID_ImportSuccessful)
		UpdateLocalIDs();

	if (ImportStatus==MID_ImportSuccessful)
	{
		Commit();
	} else {
		Rollback();

		//Free any remaining DB objects.
		while (dbRecordStack.size() > 0)
		{
			CDbBase *pDbBase = dbRecordStack.back();
			delete pDbBase;
			dbRecordStack.pop_back();
			vpCurrentType.pop_back();
		}
		while (dbImportedRecords.size() > 0)
		{
			CDbBase *pDbBase = dbImportedRecords.back();
			delete pDbBase;
			dbImportedRecords.pop_back();
			dbRecordTypes.pop_back();
		}
	}
	ASSERT(dbRecordStack.size() == 0);
	ASSERT(dbImportedRecords.size() == 0);
	ASSERT(dbRecordTypes.size() == 0);
	ASSERT(vpCurrentType.size() == 0);
	IDMaps.DemoIDMap.clear();
	IDMaps.HoldIDMap.clear();
	IDMaps.LevelIDMap.clear();
	IDMaps.PlayerIDMap.clear();
	IDMaps.RoomIDMap.clear();
	IDMaps.SavedGameIDMap.clear();

	return ImportStatus;
}

//*****************************************************************************
bool CDbXML::ExportXML(
//Export a table to an XML file.
//
//Params:
	const char *pszTableName,	//(in)	Table to export.
	c4_IntProp &propID,			//(in)	Reference to the primary key field.
	const DWORD dwPrimaryKey,	//(in)	Key to look up in that table.
	const char *pszFilename,	//(in)	XML filename, not including extension or path.
								//		File will be found at 
								//		"{data path}\Export\{filename}.xml".
	const bool bAppendFile)			//(in)	If true, an existing file will be 
								//		appended to if found.  If false (default), an 
								//		existing file will be overwritten if found.
								//		If no existing file, a new one will be created
								//		in either case.
//
//Returns:
//True if export was successful, false if not.  If false, the export file will not
//be present.
{
	//Ensure view record exists with primary key.
	c4_View DBView = GetView(pszTableName);
	const DWORD dwIndex = LookupRowByPrimaryKey(dwPrimaryKey, propID, DBView);
	if (dwIndex == ROW_NO_MATCH) return false;

	//Prepare refs list
	CDbRefs dbRefs;

	string text, element;

	//XML header.
	text += "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\r\n";

	//Provide dummy top-level record.
	text += "<DROD>\r\n";

	//All record types that can be exported independently are here.
	const VIEWTYPE vType = ParseViewType(pszTableName);
	CDb db;
	switch (vType)
	{
		case V_Demos:
			element = db.Demos.ExportXML(dwPrimaryKey, dbRefs);
			break;
		case V_Holds:
			element = db.Holds.ExportXML(dwPrimaryKey, dbRefs);
			break;
		case V_Players:
			element = db.Players.ExportXML(dwPrimaryKey, dbRefs);
			break;
		default:
			ASSERT(false);
			return NULL;
	}

	if (element.empty()) return false;
	text += element;
	text += "</DROD>\r\n";

/*
	uLong srcLen = (uLong)(text.size() * sizeof(char));
	uLong destLen = srcLen + srcLen/1000 + 12;
	Bytef *dest = new Bytef[destLen];
	const int result = compress (dest, &destLen,
			(const Bytef*)text.c_str(), srcLen);
	CStretchyBuffer buffer(dest, destLen);	//no terminating null
*/

	CStretchyBuffer buffer((const unsigned char*)text.c_str(), text.size());	//no terminating null
	CFiles::WriteBufferToFile(pszFilename,buffer);	//!!add flag: bAppendFile

	return true;
}

// $Log: DbXML.cpp,v $
// Revision 1.1  2003/02/25 00:01:33  erikh2000
// Initial check-in.
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
