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
 * Portions created by the Initial Developer are Copyright (C) 2003 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#ifdef WIN32 //Many things will not compile w/o WIN32 API.  Fix them if you are porting.
#	include <windows.h> //Should be first include.
#	pragma warning(disable:4786)
#endif

#include "Util1_6.h"
#include <BackEndLib/MessageIDs.h>
#include "../DRODLib/Db.h"
#include "../DRODLib/DBProps.h"
#include "../DRODLib/dbprops1_5.h"
#include "../DRODLib/DbMessageText.h"
#include "../DRODLib/GameConstants.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Date.h>
#include <BackEndLib/GameStream.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>
#include <BackEndLib/Ports.h>
#include "v1_11c.h"

#ifdef __linux__
#include <unistd.h> //unlink
#include <dirent.h> //opendir, readdir, closedir
#endif

//Hard-coded player IDs that are always the same each time a Players table is created.
const DWORD PLAYERID_ERIK = 2L;
const DWORD PLAYERID_LUCAS = 3L;
const DWORD PLAYERID_MATTHEW = 4L;

const UINT MAXLEN_NAMETAG = 200;

static void GetRoomExitsView(UINT wRoomX, UINT wRoomY, c4_View &ExitsView);

//**************************************************************************************
bool CUtil1_6::PrintDelete(
//Deletes DROD data.
//
//Params:
	const COptionList &/*Options*/)	//(in) Reserved for future use.
//
//Returns:
//True if successful, false if not.
const
{
	//Get paths to 3 storage files.
	WSTRING wstrHoldFilepath, wstrPlayerFilepath, wstrTextFilepath;
	GetHoldFilepath(wstrHoldFilepath);
	GetPlayerFilepath(wstrPlayerFilepath);
	GetTextFilepath(wstrTextFilepath);

	//Make sure at least one of the 3 files exists.
	if (!CFiles::DoesFileExist(wstrHoldFilepath.c_str()) &&
		!CFiles::DoesFileExist(wstrPlayerFilepath.c_str()) &&
		!CFiles::DoesFileExist(wstrTextFilepath.c_str()))
	{
#ifdef HAS_UNICODE
		printf("FAILED--Couldn't find data to delete at %S.\r\n", this->strPath.c_str());
#else
		char path[strPath.length()+1];
		UnicodeToAscii(strPath, path);
		printf("FAILED--Couldn't find data to delete at %s.\r\n", path);
#endif
		return false;
	}

	//Delete hold dat.
	bool bDeleteSuccess = !CFiles::DoesFileExist(wstrHoldFilepath.c_str()) || 
		DeleteDat(wstrHoldFilepath.c_str());
	if (!bDeleteSuccess) printf("Couldn't delete drod1_6.dat.\r\n");

	//Delete player dat.
	if (CFiles::DoesFileExist(wstrPlayerFilepath.c_str()))
	{
		if (!DeleteDat(wstrPlayerFilepath.c_str()))
		{
			bDeleteSuccess = false;
			printf("Couldn't delete player.dat.\r\n");
		}
	}

	//Delete text dat.
	if (CFiles::DoesFileExist(wstrTextFilepath.c_str()))
	{
		if (!DeleteDat(wstrTextFilepath.c_str()))
		{
			bDeleteSuccess = false;
			printf("Couldn't delete text.dat.\r\n");
		}
	}

	if (!bDeleteSuccess)
	{
		printf("FAILED--Couldn't delete all of the files.  Possibly an access problem.  "
				"Make sure DROD isn't running.\r\n");
		return false;
	}

	//Success.
	return true;
}

//**************************************************************************************
bool CUtil1_6::PrintCreate(
//Create storage files for 1.6 data.
//
//Params:
	const COptionList &/*Options*/)	//(in) Reserved for future use.
//
//Returns:
//True if successful, false if not.
const
{
	//Check for valid dest path.
	if (!IsPathValid(this->strPath.c_str()))
	{
#ifdef HAS_UNICODE
      printf("FAILED--Destination path (%S) is not a valid path.\r\n", this->strPath.c_str());
#else
		char path[strPath.length()+1];
		UnicodeToAscii(strPath, path);
		printf("FAILED--Destination path (%s) is not a valid path.\r\n", path);
#endif
		return false;
	}

	//Get paths to 3 storage files.
	WSTRING wstrHoldFilepath, wstrPlayerFilepath, wstrTextFilepath;
	GetHoldFilepath(wstrHoldFilepath);
	GetPlayerFilepath(wstrPlayerFilepath);
	GetTextFilepath(wstrTextFilepath);

	//If any of the 3 storage files already exit, then exit.
	if (DoesFileExist(wstrHoldFilepath.c_str()) || DoesFileExist(wstrPlayerFilepath.c_str())
			|| DoesFileExist(wstrTextFilepath.c_str()))
	{
#ifdef HAS_UNICODE
		printf("FAILED--data already exists at %S.\r\n", this->strPath.c_str());
#else
		char path[strPath.length()+1];
		UnicodeToAscii(strPath, path);
		printf("FAILED--data already exists at %s.\r\n", path);
#endif
		return false;
	}

	//Create the hold database.
	char szFilepath[MAX_PATH + 1];
	{
		UnicodeToAscii(wstrHoldFilepath, szFilepath);
		c4_Storage HoldStorage(szFilepath, true);
    if (!DoesFileExist(wstrHoldFilepath.c_str()))
	  {
#ifdef HAS_UNICODE
		  printf("FAILED--Was not able to create %S.\r\n", wstrHoldFilepath.c_str());
#else
		  char path[wstrHoldFilepath.length()+1];
		  UnicodeToAscii(wstrHoldFilepath, path);
		  printf("FAILED--Was not able to create %s.\r\n", path);
#endif
		  return false;
	  }
    c4_View IncrementedIDs = HoldStorage.GetAs(HOLD_INCREMENTEDIDS_VIEWDEF);
    c4_View Holds = HoldStorage.GetAs(HOLDS_VIEWDEF);
    c4_View Levels = HoldStorage.GetAs(LEVELS_VIEWDEF);
    c4_View Rooms = HoldStorage.GetAs(ROOMS_VIEWDEF);
    c4_View SavedGames = HoldStorage.GetAs(SAVEDGAMES_VIEWDEF);
    c4_View Demos = HoldStorage.GetAs(DEMOS_VIEWDEF);
    HoldStorage.Commit();
  }

  //Create the player database.
  {
		UnicodeToAscii(wstrPlayerFilepath, szFilepath);
		c4_Storage PlayerStorage(szFilepath, true);
    if (!DoesFileExist(wstrPlayerFilepath.c_str()))
	  {
#ifdef HAS_UNICODE
		  printf("FAILED--Was not able to create %S.\r\n", wstrPlayerFilepath.c_str());
#else
		  char path[wstrPlayerFilepath.length()+1];
		  UnicodeToAscii(wstrPlayerFilepath, path);
		  printf("FAILED--Was not able to create %s.\r\n", path);
#endif
		  return false;
	  }
    c4_View IncrementedIDs = PlayerStorage.GetAs(PLAYER_INCREMENTEDIDS_VIEWDEF);
    c4_View Players = PlayerStorage.GetAs(PLAYERS_VIEWDEF);
    PlayerStorage.Commit();
  }

  //Create the texts database.
  {
		UnicodeToAscii(wstrTextFilepath, szFilepath);
		c4_Storage TextStorage(szFilepath, true);
    if (!DoesFileExist(wstrTextFilepath.c_str()))
	  {
#ifdef HAS_UNICODE
		  printf("FAILED--Was not able to create %S.\r\n", wstrTextFilepath.c_str());
#else
		  char path[wstrTextFilepath.length()+1];
		  UnicodeToAscii(wstrTextFilepath, path);
		  printf("FAILED--Was not able to create %s.\r\n", path);
#endif
		  return false;
	  }
    c4_View IncrementedIDs = TextStorage.GetAs(TEXT_INCREMENTEDIDS_VIEWDEF);
    c4_View MessageTexts = TextStorage.GetAs(MESSAGETEXTS_VIEWDEF);
    TextStorage.Commit();
	}	

	//Success.
	return true;
}

//**************************************************************************************
bool CUtil1_6::PrintImport(
//Imports DROD data from a specified location/version.
//
//Params:
	const COptionList &Options,	//(in)	Options affecting the import.--IGNORED
	const WCHAR* pszSrcPath,		//(in)	Path to find DROD data for import.
	VERSION eSrcVersion)		    //(in)	Version of data to locate.--IGNORED
//
//Returns:
//True if successful, false if not.
const
{	
  CGameStream *pDROD1_5Stream = NULL;
  c4_Storage *pDROD1_5Storage = NULL;

  //Get options.
  static const WCHAR wh[] = {{'h'},{0}};
  static const WCHAR wd[] = {{'d'},{0}};
  static const WCHAR wp[] = {{'p'},{0}};
  static const WCHAR wt[] = {{'t'},{0}};
  bool bImportHolds = Options.GetSize()==0 || Options.Exists(wh);
  bool bImportDemos = Options.GetSize()==0 || Options.Exists(wd);
  bool bImportPlayers = Options.GetSize()==0 || Options.Exists(wp);
  bool bImportMIDs = Options.GetSize()==0 || Options.Exists(wt);

	//Make sure 1.6 path is valid.
	if (!IsPathValid(this->strPath.c_str()))
	{
#ifdef HAS_UNICODE
		printf("FAILED--Destination path (%S) is not a valid path.\r\n", this->strPath.c_str());
#else
		char path[strPath.length()+1];
		UnicodeToAscii(strPath, path);
		printf("FAILED--Destination path (%s) is not a valid path.\r\n", path);
#endif
		return false;
	}

	//Make sure source (1.5) path is valid.
	if (!IsPathValid(pszSrcPath))
	{
#ifdef HAS_UNICODE
		printf("FAILED--Source path (%S) is not a valid path.\r\n", pszSrcPath);
#else
		WSTRING tmp = pszSrcPath;
		char path[tmp.length()+1];
		UnicodeToAscii(tmp, path);
		printf("FAILED--Source path (%s) is not a valid path.\r\n", path);
#endif
		return false;
	}

	//Only v1.5 is supported.
	if (eSrcVersion != v1_5)
	{
#ifdef HAS_UNICODE
		printf("FAILED--Data version %S is not supported.\r\n", g_szarrVersions[eSrcVersion]);
#else
		WSTRING tmp = g_szarrVersions[eSrcVersion];
		char ver[tmp.length()+1];
		UnicodeToAscii(tmp, ver);
		printf("FAILED--Data version %s is not supported.\r\n", ver);
#endif
		return false;
	}

	//Check for presence of 3 destination dats.
	WSTRING wstrHoldFilepath, wstrPlayerFilepath, wstrTextFilepath;
	GetHoldFilepath(wstrHoldFilepath);
	GetPlayerFilepath(wstrPlayerFilepath);
	GetTextFilepath(wstrTextFilepath);
	if (!DoesFileExist(wstrHoldFilepath.c_str()) || !DoesFileExist(wstrPlayerFilepath.c_str()) 
			|| !DoesFileExist(wstrTextFilepath.c_str()))
	{
#ifdef HAS_UNICODE
		printf("FAILED--Storage files not found at %S.\r\n", this->strPath.c_str());
#else
		char path[strPath.length()+1];
		UnicodeToAscii(strPath, path);
		printf("FAILED--Storage files not found at %s.\r\n", path);
#endif
		return false;
	}

  //Open drod1.5.dat file.
  if (bImportHolds || bImportDemos || bImportPlayers)
  {
    //Check for presence of 1.5 dat.
	  WSTRING wstr1_5Filepath = pszSrcPath;
	  {
		  const WCHAR szDROD1_5Dat[] = {{'d'},{'r'},{'o'},{'d'},{'1'},{'_'},{'5'},{'.'},{'d'},{'a'},{'t'},{0}};
		  wstr1_5Filepath += wszSlash;
		  wstr1_5Filepath += szDROD1_5Dat;
		  if (!DoesFileExist(wstr1_5Filepath.c_str()))
		  {
#ifdef HAS_UNICODE
 			  printf("FAILED--%S not found.\r\n", wstr1_5Filepath.c_str());
#else
			  char path[wstr1_5Filepath.length()+1];
			  UnicodeToAscii(wstr1_5Filepath, path);
			  printf("FAILED--%s not found.\r\n", path);
#endif
			  return false;
		  }
	  }

    //Open it.
    printf("Opening drod1_5.dat...");
    pDROD1_5Stream = new CGameStream(wstr1_5Filepath.c_str());
    if (!pDROD1_5Stream) return false;
    pDROD1_5Storage = new c4_Storage;
    if (!pDROD1_5Storage || !pDROD1_5Storage->LoadFrom(*pDROD1_5Stream))
    {
      printf("FAILED.\r\n");
      delete pDROD1_5Storage;
      delete pDROD1_5Stream;
      return false;
    }
    printf("done.\r\n");
  }

  //This will hold IDs of messages that are needed to support imported objects.
  list<DWORD> MessagesNeeded;

  //Populate drod1_6.dat (hold dat).
  if (bImportHolds || bImportDemos)
	{
    //Open hold dat.
    printf("Opening drod1_6.dat...");
    CGameStream HoldStream(wstrHoldFilepath.c_str());
    c4_Storage HoldStorage;
    if (!HoldStorage.LoadFrom(HoldStream))
    {
      printf("FAILED.\r\n");
      return false;
    }
    printf("done.\r\n");

    //Copy some of the fields from IncrementedIDs.
    printf("Importing incremented IDs...");
    {
      c4_View SourceIncrementedIDs = pDROD1_5Storage->View("IncrementedIDs");
      c4_View DestIncrementedIDs = HoldStorage.View("IncrementedIDs");
      c4_RowRef SourceRow = SourceIncrementedIDs.GetAt(0);
      DestIncrementedIDs.Add(
          p_DemoID[            ns1_5::p_DemoID(SourceRow) ] +
          p_LevelID[           ns1_5::p_LevelID(SourceRow) ] +
          p_HoldID[            ns1_5::p_HoldID(SourceRow) ] +
          p_RoomID[            ns1_5::p_RoomID(SourceRow) ] +
          p_SavedGameID[       ns1_5::p_SavedGameID(SourceRow) ]);
    }
    printf("done.\r\n");

    if (bImportHolds)
    {
		  //Copy viewdefs for hold dat.
      printf("Importing holds...");
      if (!ImportHolds(*pDROD1_5Storage, HoldStorage, MessagesNeeded))
      {
			  printf("FAILED.\r\n");
			  return false;
		  }
      printf("done.\r\n");
      printf("Importing levels...");
		  if (!ImportLevels(*pDROD1_5Storage, HoldStorage, MessagesNeeded))
      {
			  printf("FAILED.\r\n");
			  return false;
		  }
      printf("done.\r\n");
      printf("Importing rooms:\r\n");
		  if (!ImportRooms(*pDROD1_5Storage, HoldStorage, MessagesNeeded))
      {
			  printf("FAILED.\r\n");
			  return false;
		  }
      printf("Done importing rooms.\r\n");
    }

    if (bImportDemos)
    {
      printf("Importing saved games...");
		  if (!ImportSavedGames(*pDROD1_5Storage, HoldStorage, MessagesNeeded))
      {
			  printf("FAILED.\r\n");
			  return false;
		  }
      printf("done.\r\n");
      printf("Importing demos...");
		  if (!ImportDemos(*pDROD1_5Storage, HoldStorage, MessagesNeeded))
      {
			  printf("FAILED.\r\n");
			  return false;
		  }
      printf("done.\r\n");
    }

    HoldStream.Save(&HoldStorage);
  }

  //Populate player.dat.
  if (bImportPlayers)
  {
    //Open player dat.
    printf("Opening player.dat...");
    CGameStream PlayerStream(wstrPlayerFilepath.c_str());
    c4_Storage PlayerStorage;
    if (!PlayerStorage.LoadFrom(PlayerStream))
    {
      printf("FAILED.\r\n");
      return false;
    }
    printf("done.\r\n");

    //Copy some of the fields from IncrementedIDs.
    printf("Importing incremented IDs...");
    {
      c4_View SourceIncrementedIDs = pDROD1_5Storage->View("IncrementedIDs");
      c4_View DestIncrementedIDs = PlayerStorage.View("IncrementedIDs");
      c4_RowRef SourceRow = SourceIncrementedIDs.GetAt(0);
      DestIncrementedIDs.Add(
          p_PlayerID[                ns1_5::p_PlayerID(SourceRow) ]);
    }
    printf("done.\r\n");

		//Copy viewdefs for player dat.
    printf("Importing players...");
   	if (!ImportPlayers(*pDROD1_5Storage, PlayerStorage, MessagesNeeded))
    {
			printf("FAILED.\r\n");
			return false;
		}
    printf("done.\r\n");

    PlayerStream.Save(&PlayerStorage);
  }

  //Populate text.dat with messages needed and maybe basic text.
  {
    //Open text dat.
    printf("Opening text.dat...");
    CGameStream TextStream(wstrTextFilepath.c_str());
    c4_Storage TextStorage;
    if (!TextStorage.LoadFrom(TextStream))
    {
      printf("FAILED.\r\n");
      return false;
    }
    printf("done.\r\n");

    //Copy some of the fields from IncrementedIDs.
    if (MessagesNeeded.size())
    {
      ASSERT(pDROD1_5Storage);
      printf("Importing incremented IDs...");
      {
        c4_View SourceIncrementedIDs = pDROD1_5Storage->View("IncrementedIDs");
        c4_View DestIncrementedIDs = TextStorage.View("IncrementedIDs");
        c4_RowRef SourceRow = SourceIncrementedIDs.GetAt(0);
        DestIncrementedIDs.Add(
            p_MessageTextID[            ns1_5::p_MessageTextID(SourceRow) ] +
            p_MessageID[                ns1_5::p_MessageID(SourceRow) ]);
      }
      printf("done.\r\n");

    	//Copy needed messages.
      printf("Importing message texts...");
		  if (!ImportMessageTexts(*pDROD1_5Storage, TextStorage, MessagesNeeded))
		  {
			  printf("FAILED.\r\n");
			  return false;
		  }
      printf("done.\r\n");
    }

    if (bImportMIDs)
    {
      printf("Importing basic messages...");
      if (!ImportBasicMessages(pszSrcPath, TextStorage))
      {
        printf("FAILED.\r\n");
        return false;
      }
      printf("done.\r\n");
    }

    TextStream.Save(&TextStorage);
	}

   //Now that MessageTexts is set up, update players table to have original name field.
   if (bImportPlayers)
   {
     printf("Assigning original names to players...");
  
     //Fill in any blank original name fields with copies of current name.
     if (!AssignOriginalNames())
     {
        printf("FAILED.");
        return false;
     }
     printf("done.\r\n");
   }

   //Success.
   delete pDROD1_5Storage;
   delete pDROD1_5Stream;
   return true;
}

//
//Private methods.
//

//**************************************************************************************
void CUtil1_6::GetHoldFilepath(
//Concat filepath to hold dat.
//
//Params:
	WSTRING &wstrFilepath)	//(out)  Filepath.
const
{
   const WCHAR wszDROD1_6Dat[] = {{'d'},{'r'},{'o'},{'d'},{'1'},{'_'},{'6'},{'.'},{'d'},{'a'},{'t'},{0}};
   wstrFilepath += this->strPath.c_str();
   wstrFilepath += wszSlash;
   wstrFilepath += wszDROD1_6Dat;
}

//**************************************************************************************
void CUtil1_6::GetPlayerFilepath(
//Concat filepath to player dat.
//
//Params:
	WSTRING &wstrFilepath)	//(out)  Filepath.
const
{
		const WCHAR wszPlayerDat[] = {{'p'},{'l'},{'a'},{'y'},{'e'},{'r'},{'.'},{'d'},{'a'},{'t'},{0}};
		wstrFilepath += this->strPath.c_str();
		wstrFilepath += wszSlash;
		wstrFilepath += wszPlayerDat;
}

//**************************************************************************************
void CUtil1_6::GetTextFilepath(
//Concat filepath to text dat.
//
//Params:
	WSTRING &wstrFilepath)	//(out)  Filepath.
const
{
		const WCHAR wszTextDat[] = {{'t'},{'e'},{'x'},{'t'},{'.'},{'d'},{'a'},{'t'},{0}};
		wstrFilepath += this->strPath.c_str();
		wstrFilepath += wszSlash;
		wstrFilepath += wszTextDat;
}

//**************************************************************************************
bool CUtil1_6::DeleteDat(
//Delete a dat file.
//
//Params:
	const WCHAR *pwszFilepath) //(in)	Dat to delete.
{
	char szFilepath[MAX_PATH + 1];
	UnicodeToAscii(WSTRING(pwszFilepath), szFilepath);
#ifdef WIN32
	OFSTRUCT ofs;
	memset(&ofs, 0, sizeof(ofs));
	return (OpenFile(szFilepath, &ofs, OF_DELETE)!=HFILE_ERROR);
#else
	return !unlink(szFilepath);
#endif
}

//**************************************************************************************
bool CUtil1_6::ImportHolds(
//Imports holds from 1.5 source storage to 1.6 dest storage.
//
//Params:
  c4_Storage &SourceStorage,    //(in)  drod1_5.dat
  c4_Storage &DestStorage,      //(in)  drod1_6.dat
  list<DWORD> &MessagesNeeded)  //(out) Adds IDs of messages used by imported holds.
//
//Returns:
//True if successful, false if not.
const
{
  CDate CreatedTime(1997, 1, 1);
  c4_View SourceView = SourceStorage.View(ViewTypeStr(V_Holds));
  c4_View DestView = DestStorage.View(ViewTypeStr(V_Holds));
  long lRowCount = SourceView.GetSize();
  for (long lRowNo = 0; lRowNo < lRowCount; ++lRowNo)
  {
    c4_RowRef SourceRow = SourceView.GetAt(lRowNo);
    ASSERT(ns1_5::p_HoldID(SourceRow)==HOLD_DUGANS_DUNGEON);

    DestView.Add(  
			p_HoldID[               ns1_5::p_HoldID(SourceRow) ] +
			p_NameMessageID[        ns1_5::p_NameMessageID(SourceRow) ] +
			p_DescriptionMessageID[ ns1_5::p_DescriptionMessageID(SourceRow) ] +
			p_LevelID[              ns1_5::p_LevelID(SourceRow) ] +
			p_GID_Created[          CreatedTime ] +
			p_LastUpdated[          CreatedTime ] +
			p_GID_PlayerID[         PLAYERID_ERIK ] + //Hard-coded for Dugan's Dungeon.
			p_GID_NewLevelIndex[    26 ] + //Hard-coded for Dugan's Dungeon.
         p_EditingPrivileges[    CDbHold::YouAndConquerors ] + //Hard-coded for Dugan's Dungeon.
         p_EndHoldMessageID[     0 ]); //end hold message is located in Texts/EndOfGame.uni

    MessagesNeeded.push_back(ns1_5::p_NameMessageID(SourceRow));
    MessagesNeeded.push_back(ns1_5::p_DescriptionMessageID(SourceRow));
  }
  DestStorage.Commit();

  return true;
}

//**************************************************************************************
bool CUtil1_6::ImportLevels(
//Imports levels from 1.5 source storage to 1.6 dest storage.
//
//Params:
  c4_Storage &SourceStorage,    //(in)  drod1_5.dat
  c4_Storage &DestStorage,      //(in)  drod1_6.dat
  list<DWORD> &MessagesNeeded)  //(out) Adds IDs of messages used by imported levels.
//
//Returns:
//True if successful, false if not.
const
{
  c4_View SourceView = SourceStorage.View(ViewTypeStr(V_Levels));
  c4_View DestView = DestStorage.View(ViewTypeStr(V_Levels));
  long lRowCount = SourceView.GetSize();
  for (long lRowNo = 0; lRowNo < lRowCount; ++lRowNo)
  {
    c4_RowRef SourceRow = SourceView.GetAt(lRowNo);

    DestView.Add(
			p_LevelID[              ns1_5::p_LevelID(SourceRow) ] +
			p_HoldID[               ns1_5::p_HoldID(SourceRow) ] +
			p_PlayerID[             ns1_5::p_PlayerID(SourceRow) ] +
			p_NameMessageID[        ns1_5::p_NameMessageID(SourceRow) ] +
			p_DescriptionMessageID[ ns1_5::p_DescriptionMessageID(SourceRow) ] +
			p_RoomID[               ns1_5::p_RoomID(SourceRow) ] +
			p_X[                    ns1_5::p_X(SourceRow) ] +
			p_Y[                    ns1_5::p_Y(SourceRow) ] +
			p_O[                    ns1_5::p_O(SourceRow) ] +
			p_Created[              ns1_5::p_Created(SourceRow) ] +
			p_LastUpdated[          ns1_5::p_LastUpdated(SourceRow) ] +
			p_GID_LevelIndex[       ns1_5::p_LevelID(SourceRow) ]); //Hard-coded for Dugan's Dungeon.

      MessagesNeeded.push_back(ns1_5::p_NameMessageID(SourceRow));
      MessagesNeeded.push_back(ns1_5::p_DescriptionMessageID(SourceRow));
  }
  DestStorage.Commit();

  return true;
}

//**************************************************************************************
bool CUtil1_6::ImportRooms(
//Imports rooms from 1.5 source storage to 1.6 dest storage.
//
//Params:
  c4_Storage &SourceStorage,    //(in)  drod1_5.dat
  c4_Storage &DestStorage,      //(in)  drod1_6.dat
  list<DWORD> &MessagesNeeded)  //(out) Adds IDs of messages used by imported rooms.
//
//Returns:
//True if successful, false if not.
const
{
  c4_View OrbsView;
  c4_View MonstersView;
	c4_View ScrollsView;
	c4_View SpecialExitsView;
   c4_View RoomView;

  c4_View SourceView = SourceStorage.View(ViewTypeStr(V_Rooms));
  c4_View DestView = DestStorage.View(ViewTypeStr(V_Rooms));
  long lRowCount = SourceView.GetSize();
  for (long lRowNo = 0; lRowNo < lRowCount; ++lRowNo)
  {
    printf("  Room %ld of %ld.\r\n", lRowNo + 1, lRowCount);

    static c4_RowRef SourceRow = SourceView.GetAt(0);
    SourceRow = SourceView.GetAt(lRowNo);

    OrbsView =          ns1_5::p_Orbs(SourceRow);
    MonstersView =      ns1_5::p_Monsters(SourceRow);
	 ScrollsView =       ns1_5::p_Scrolls(SourceRow);
   
   // Fix Tar mother eyes
	UINT wMonsterI, wMonsterCount = MonstersView.GetSize();
	if (wMonsterCount > 0)
	{
		for (wMonsterI = 0; wMonsterI < wMonsterCount; wMonsterI++)
		{
			const UINT wMonsterType = p_Type(MonstersView[wMonsterI]);
         if (wMonsterType == M_TARMOTHER_111c)
         {
            const int xMonster = p_X(MonstersView[wMonsterI]);
            const int yMonster = p_Y(MonstersView[wMonsterI]);
            //const int ori = p_O(MonstersView[wMonsterI]);
            const int xRoom = p_RoomX(SourceView[lRowNo]);
            const int yRoom = p_RoomY(SourceView[lRowNo]);
            int oMonster = NO_ORIENTATION;

				//Assume the eyes come in sequence.
            static bool bRightEye = false;
				if (bRightEye)
               oMonster = S;
            else
               oMonster = p_O(MonstersView[wMonsterI]);
				bRightEye = !bRightEye;

				//Fix a couple of times when this assumption is false.
				if (xRoom == 53 && yRoom == 857)
				{
					//8:7S3E (3-4,27)
					if (yMonster == 27)
					{
						if (xMonster == 3)
							oMonster = NO_ORIENTATION;
						else if (xMonster == 4)
							oMonster = S;
					}
				} else if (xRoom == 51 && yRoom == 1750)
				{
					//17:1E (8,8) and (17,22)
					if (xMonster == 8 && yMonster == 8)
						oMonster = NO_ORIENTATION;
					else if (xMonster == 17 && yMonster == 22)
						oMonster = S;
				}
            p_O(MonstersView[wMonsterI]) = oMonster;
         }
		}
	}

   //Add exits for room.  This is hard-coded for Dugan's Dungeon.
    c4_View ExitsView;
    GetRoomExitsView(ns1_5::p_RoomX(SourceRow), ns1_5::p_RoomY(SourceRow), ExitsView);

	  DestView.Add(
			p_RoomID[       ns1_5::p_RoomID(SourceRow) ] +
			p_LevelID[      ns1_5::p_LevelID(SourceRow) ] +
			p_RoomX[        ns1_5::p_RoomX(SourceRow) ] +
			p_RoomY[        ns1_5::p_RoomY(SourceRow) ] +
			p_RoomCols[     ns1_5::p_RoomCols(SourceRow) ] +
			p_RoomRows[     ns1_5::p_RoomRows(SourceRow) ] +
			p_Style[        ns1_5::p_Style(SourceRow) ] +
			p_IsRequired[   Is1_5RoomRequired(p_RoomID(SourceRow)) ] +
			p_Squares[      ns1_5::p_Squares(SourceRow) ] +
			p_Orbs[         OrbsView ] +
			p_Monsters[     MonstersView ] +
			p_Scrolls[      ScrollsView ] +
			p_Exits[        ExitsView ]);

    //Get scroll text message IDs.
    UINT wScrollCount = ScrollsView.GetSize();
    for (UINT wScrollNo = 0; wScrollNo < wScrollCount; ++wScrollNo)
    {
      DWORD dwMessageID = p_MessageID( ScrollsView[wScrollNo] );
      MessagesNeeded.push_back(dwMessageID);
    }
  }
  DestStorage.Commit();

  return true;
}

//**************************************************************************************
inline void InsertDemoCommand(CDbCommands &commands, const UINT wIndex, const BYTE command)
{
   COMMANDNODE *pCommand = commands.Get(wIndex);
   COMMANDNODE *pNewCommand = new COMMANDNODE;
   pNewCommand->bytCommand = command;
   pNewCommand->byt10msElapsedSinceLast = 0;
   pNewCommand->pNext = pCommand->pNext;
   pCommand->pNext = pNewCommand;
}

//**************************************************************************************
bool CUtil1_6::ImportSavedGames(
//Imports saved games from 1.5 source storage to 1.6 dest storage.
//
//Params:
  c4_Storage &SourceStorage,    //(in)  drod1_5.dat
  c4_Storage &DestStorage,      //(in)  drod1_6.dat
  list<DWORD> &/*MessagesNeeded*/)  //(out) Adds IDs of messages used by imported saved games.
//
//Returns:
//True if successful, false if not.
const
{
  c4_View SourceView = SourceStorage.View(ViewTypeStr(V_SavedGames));
  c4_View DestView = DestStorage.View(ViewTypeStr(V_SavedGames));
  long lRowCount = SourceView.GetSize();
  for (long lRowNo = 0; lRowNo < lRowCount; ++lRowNo)
  {
    c4_RowRef SourceRow = SourceView.GetAt(lRowNo);

    if (ns1_5::p_PlayerID(SourceRow)!=1) //A stub player continue record created by drodutil v1.5.
    {
       c4_View ExploredRooms = ns1_5::p_ExploredRooms(SourceRow);
       c4_View ConqueredRooms = ns1_5::p_ConqueredRooms(SourceRow);

         //Fix broken demos 1 and 4.
         //Also need to change endTurnNo in the corresponding demo record.
         CDbCommands commands;
         commands = ns1_5::p_Commands(SourceRow);
         switch (lRowNo)
         {
            case 1:
            {
               static const UINT wIndexNo[] = {31, 62, 92, 409, 410, 411, 412, 413, 414, 415, 416};
               static const BYTE commandNo[] = {CMD_WAIT, CMD_WAIT, CMD_WAIT, CMD_CC, CMD_CC, CMD_N, CMD_NE, CMD_C, CMD_C, CMD_SE, CMD_SE};
               for (UINT wIndex=0; wIndex<11; ++wIndex)
                  InsertDemoCommand(commands, wIndexNo[wIndex], commandNo[wIndex]);
               commands.Add(CMD_N, 0);
               break;
            }
            case 4:
            {
               InsertDemoCommand(commands, 0, CMD_WAIT);
               break;
            }
         }
	      DWORD dwCommandsSize;
	      BYTE *pbytCommands = commands.GetPackedBuffer(dwCommandsSize);
	      c4_Bytes CommandsBytes(pbytCommands, dwCommandsSize);

         DestView.Add(
			   p_SavedGameID[        ns1_5::p_SavedGameID(SourceRow) ] +
			   p_PlayerID[           ns1_5::p_PlayerID(SourceRow) ] +
			   p_RoomID[             ns1_5::p_RoomID(SourceRow) ] +
			   p_Type[               ns1_5::p_Type(SourceRow) ] +
			   p_CheckpointX[        ns1_5::p_CheckpointX(SourceRow) ] +
			   p_CheckpointY[        ns1_5::p_CheckpointY(SourceRow) ] +
			   p_IsHidden[           ns1_5::p_IsHidden(SourceRow) ] +
			   p_LastUpdated[        ns1_5::p_LastUpdated(SourceRow) ] +
			   p_StartRoomX[         ns1_5::p_StartRoomX(SourceRow) ] +
			   p_StartRoomY[         ns1_5::p_StartRoomY(SourceRow) ] +
			   p_StartRoomO[         ns1_5::p_StartRoomO(SourceRow) ] +
			   p_ExploredRooms[      ExploredRooms ] +
			   p_ConqueredRooms[     ConqueredRooms ] +
			   p_Created[            ns1_5::p_Created(SourceRow) ] +
			   p_Commands[           CommandsBytes ] );
    }
  }
  DestStorage.Commit();

  return true;
}

//**************************************************************************************
bool CUtil1_6::ImportDemos(
//Imports demos from 1.5 source storage to 1.6 dest storage.
//
//Params:
  c4_Storage &SourceStorage,      //(in)  drod1_5.dat
  c4_Storage &DestStorage,        //(in)  drod1_6.dat
  list<DWORD> &MessagesNeeded)    //(out) Adds IDs of messages used by imported demos.
//
//Returns:
//True if successful, false if not.
const
{
  c4_View SourceView = SourceStorage.View(ViewTypeStr(V_Demos));
  c4_View DestView = DestStorage.View(ViewTypeStr(V_Demos));
  long lRowCount = SourceView.GetSize();
  for (long lRowNo = 0; lRowNo < lRowCount; ++lRowNo)
  {
    c4_RowRef SourceRow = SourceView.GetAt(lRowNo);

    //Fix broken demos 1 and 4 (due to changed tar mother and roach queen spawning).
    const DWORD dwSavedGameID = ns1_5::p_SavedGameID(SourceRow);
    UINT wEndTurnNo = ns1_5::p_EndTurnNo(SourceRow);
    switch (dwSavedGameID)
    {
       case 10001:
          wEndTurnNo += 12;
          break;
       case 10004:
          ++wEndTurnNo;
          break;
    }

    DestView.Add(
			  p_DemoID[               ns1_5::p_DemoID(SourceRow) ] +
			  p_SavedGameID[          dwSavedGameID ] +
			  p_DescriptionMessageID[ ns1_5::p_DescriptionMessageID(SourceRow) ] +
			  p_IsHidden[             ns1_5::p_IsHidden(SourceRow) ] + 
			  p_ShowSequenceNo[       ns1_5::p_ShowSequenceNo(SourceRow) ] +
			  p_BeginTurnNo[          ns1_5::p_BeginTurnNo(SourceRow) ] +
			  p_EndTurnNo[            wEndTurnNo ] +
			  p_NextDemoID[           ns1_5::p_NextDemoID(SourceRow) ] +
			  p_Checksum[             ns1_5::p_Checksum(SourceRow) ]);

    MessagesNeeded.push_back(ns1_5::p_DescriptionMessageID(SourceRow));
  }
  DestStorage.Commit();

  return true;
}

//**************************************************************************************
bool CUtil1_6::ImportPlayers(
//Imports players from 1.5 source storage to 1.6 dest storage.
//
//Params:
  c4_Storage &SourceStorage,    //(in)  drod1_5.dat
  c4_Storage &DestStorage,      //(in)  player.dat
  list<DWORD> &MessagesNeeded)  //(out) Adds IDs of messages used by imported players.
//
//Returns:
//True if successful, false if not.
const
{
  CDate CreatedTime(1997, 1, 1);
  c4_View SourceView = SourceStorage.View(ViewTypeStr(V_Players));
  c4_View DestView = DestStorage.View(ViewTypeStr(V_Players));
  long lRowCount = SourceView.GetSize();
  for (long lRowNo = 0; lRowNo < lRowCount; ++lRowNo)
  {
    c4_RowRef SourceRow = SourceView.GetAt(lRowNo);

    DestView.Add(
		    p_PlayerID[                     ns1_5::p_PlayerID(SourceRow) ] +
		    p_IsLocal[                      ns1_5::p_IsLocal(SourceRow) ] +
		    p_NameMessageID[                ns1_5::p_NameMessageID(SourceRow) ] +
		    p_EMailMessageID[               ns1_5::p_EMailMessageID(SourceRow) ] +
		    p_GID_OriginalNameMessageID[    MID_NoText ] +
		    p_GID_Created[                  (time_t)(CreatedTime) + lRowNo ] +
		    p_LastUpdated[                  (time_t)(CreatedTime) + lRowNo ] +
		    p_Settings[                     ns1_5::p_Settings(SourceRow) ] );

    MessagesNeeded.push_back(ns1_5::p_NameMessageID(SourceRow));
    MessagesNeeded.push_back(ns1_5::p_EMailMessageID(SourceRow));
  }
  DestStorage.Commit();

  return true;
}

//**************************************************************************************
bool CUtil1_6::ImportMessageTexts(
//Imports message texts from 1.5 source storage to 1.6 dest storage.
//
//Params:
  c4_Storage &SourceStorage,          //(in) drod1_5.dat
  c4_Storage &DestStorage,            //(in) text.dat
  const list<DWORD> &MessagesNeeded)  //(in) IDs of messages that will be imported.
//
//Returns:
//True if successful, false if not.
const
{
  c4_View SourceView = SourceStorage.View("MessageTexts");
  c4_View DestView = DestStorage.View("MessageTexts");
  long lRowCount = SourceView.GetSize();
  for (long lRowNo = 0; lRowNo < lRowCount; ++lRowNo)
  {
    c4_RowRef SourceRow = SourceView.GetAt(lRowNo);
    
    //See if this message is needed.
    DWORD dwMessageID = ns1_5::p_MessageID(SourceRow);
    list<DWORD>::const_iterator iSeek;
    for (iSeek = MessagesNeeded.begin(); iSeek != MessagesNeeded.end(); ++iSeek)
    {
      if (*iSeek == dwMessageID) break;
    }
    if (iSeek == MessagesNeeded.end()) continue; //Not needed.
    
    //Add message.
    DestView.Add(
        p_MessageTextID[            ns1_5::p_MessageTextID(SourceRow) ] +
			  p_MessageID[                dwMessageID ] +
			  p_LanguageCode[             English ] + //Hard-coded to English.
			  p_MessageText[              ns1_5::p_MessageText(SourceRow) ]);
  }
  DestStorage.Commit();

  return true;
}

//**************************************************************************************
//Written by mrimer.  Revisions after 5/5/03 tracked in log.
bool CUtil1_6::Is1_5RoomRequired(
//One-time method for inserting whether each room in Dugan's Dungeon hold
//is required to complete the level or not.
//
//Params:
  DWORD dwRoomID) //(in)  Identifies a room from DD.
//
//Returns:
//True if it is required, false if not.
const
{
	//Non-required rooms:
	switch (dwRoomID)
	{
		//Level 1.
	case 10:
	case 12:
	case 15:
	case 16:	//the shareware demo room
		//Level 2.
	case 22:
		//Level 3.
	case 52:
		//Level 4.
	case 55:
	case 67:
		//Level 5.
	case 68:
		//Level 6.
	case 100:
		//Level 7.
	case 103:	//master intellect?
	case 116:
		//Level 8.
		//Level 9.
	case 137:
		//Level 10.
	case 146:
		//Level 11.
	case 160:
		//Level 12.
	case 168:
	case 179:
		//Level 13 (ALL rooms -- below).
		//Level 14.
	case 205:	//??why not add a way in to this room?
	case 206:
		//Level 15.
	case 226:
	case 230: case 231: case 232:
		//Level 16.
	case 242:
		//Level 17.
	case 255:
		//Level 18.
	case 266:
		//Level 19.
		//Level 20.
		//Level 21.
	case 291:
		//Level 22.
	case 303:
	case 304:
		//Level 23.
	case 323:
		//Level 24.
	case 326:
		//Level 25.
		//none
		return false;
	}
	//Level 13.
	if (180 <= dwRoomID && dwRoomID <= 204)
		return false;

	return true;
}

//*****************************************************************************
//Written by mrimer.  Revisions after 5/5/03 tracked in log.
static void GetRoomExitsView(
//Add any exits found in this room.
//
//Params:
	UINT wRoomX, UINT wRoomY,
	c4_View &ExitsView)
{
//Adds an exit to room (wX,wY) going to level 'wLevel'.
//Ensure stair tiles are at located at the corners of the specified rectangle.
#define ADDEXIT(wX, wY, wLevel, wA, wB, wC, wD)	\
	if (wRoomX == wX && wRoomY == wY)\
	{\
		ExitsView.Add(p_LevelID[wLevel] + p_Left[wA] + p_Right[wC] + p_Top[wB] + p_Bottom[wD]);\
	}\

	const DWORD dwLevelNo = wRoomY / 100;
	switch (dwLevelNo)
	{
		case 1:
			ADDEXIT(51,149, 2, 23,15,26,31);
		break;
		case 2:
			ADDEXIT(51,249, 3, 30,20,35,31);
		break;
		case 3:
			ADDEXIT(50,352, 4, 29,23,35,31);
		break;
		case 4:
			ADDEXIT(52,452, 5, 27,7,36,17);
			//Secret warp room on level 4.
			ADDEXIT(51,448, 1, 27,26,27,30);
			ADDEXIT(51,448, 6, 30,26,30,30);
			ADDEXIT(51,448, 7, 33,26,33,30);
		break;
		case 5:
			ADDEXIT(49,547, 6, 14,10,22,23);
		break;
		case 6:
			ADDEXIT(50,651, 7, 12,18,13,28);
		break;
		case 7:
			ADDEXIT(51,752, 8, 15,18,18,23);
		break;
		case 8:
			ADDEXIT(53,855, 9, 15,19,16,20);
		break;
		case 9:
			ADDEXIT(54,946, 10, 15,14,21,30);
		break;
		case 10:
			ADDEXIT(50,1047, 11, 7,9,13,24);
		break;
		case 11:
			ADDEXIT(52,1149, 12, 25,20,29,27);
		break;
		case 12:
			ADDEXIT(52,1252, 13, 36,5,36,30);
			//Secret warp room on level 12.
			ADDEXIT(50,1248, 3, 5,30,6,30);
			ADDEXIT(50,1248, 15, 8,30,9,30);
			ADDEXIT(50,1248, 2, 17,30,18,30);
			ADDEXIT(50,1248, 16, 20,30,21,30);
			ADDEXIT(50,1248, 4, 11,30,12,30);
			ADDEXIT(50,1248, 14, 14,30,15,30);
		break;
		case 13:
			ADDEXIT(48,1352, 14, 1,25,3,30);
		break;
		case 14:
			ADDEXIT(52,1447, 15, 16,17,20,19);
		break;
		case 15:
			ADDEXIT(46,1551, 16, 25,26,29,29);
		break;
		case 16:
			ADDEXIT(50,1653, 17, 9,12,11,20);
		break;
		case 17:
			ADDEXIT(52,1752, 18, 16,7,19,21);
		break;
		case 18:
			ADDEXIT(51,1851, 19, 13,3,17,19);
		break;
		case 19:
			ADDEXIT(50,1950, 20, 2,10,3,15);
		break;
		case 20:
			ADDEXIT(48,2048, 21, 36,30,36,30);
		break;
		case 21:
			ADDEXIT(50,2148, 22, 17,11,20,17);
		break;
		case 22:
			ADDEXIT(50,2248, 23, 13,15,21,21);
		break;
		case 23:
			ADDEXIT(50,2351, 24, 4,20,12,28);
		break;
		case 24:
			ADDEXIT(50,2448, 25, 27,12,34,28);
		break;
		//Level 25 is the end of the hold.
	}
#undef ADDEXIT
}

//****************************************************************************************************
bool CUtil1_6::AssignOriginalNames(void) const
//Looks at GID_OriginalName field of each player and assigns a value if it is missing.
{
  //Note that text.dat must have already been created and correctly populated.
  CDb db;
  if (!db.IsOpen()) 
  {
    if (db.Open(this->strPath.c_str()) != MID_Success) return false;
  }

  //Look through records of player.dat for missing original name.
  c4_View PlayersView = db.GetView(ViewTypeStr(V_Players));
  UINT wPlayerCount = PlayersView.GetSize();
  for (UINT wPlayerNo = 0; wPlayerNo < wPlayerCount; ++wPlayerNo)
  {
    DWORD dwOriginalNameMID = p_GID_OriginalNameMessageID( PlayersView[wPlayerNo] );
    if (dwOriginalNameMID != MID_NoText) continue;

    //Add a copy of the current name to the database.
    DWORD dwNameMessageID = p_NameMessageID( PlayersView[wPlayerNo]);
    CDbMessageText NameText( static_cast<MESSAGE_ID>(dwNameMessageID));
    if (WCSlen((const WCHAR *)NameText)==0) //A stub player record created by drodutil v1.5.
    {
        PlayersView.RemoveAt(wPlayerNo);   
        --wPlayerNo; --wPlayerCount; //Rows shuffled up by one after deletion.
        continue; 
    }
    CDbMessageText OriginalNameText;
    OriginalNameText = NameText;
    
    p_GID_OriginalNameMessageID( PlayersView[wPlayerNo] ) = OriginalNameText.Flush();
  }

  db.Commit();

  return true;
}

//****************************************************************************************************
bool CUtil1_6::ImportBasicMessages(
//Import basic messages (those that correspond to MID_* constants) from .uni files.  Any 
//existing texts will be deleted.  A header file containing MID_* constants that match message 
//records will be generated at destination path.
//
//Params:
    const WCHAR *pwzSrcPath,    //(in)  Path to .uni files.  All files matching *.uni will be imported.
    c4_Storage &TextStorage)    //(in)  text.dat
//
//Returns:
//True if successful, false if not.
const
{
    //Get text representation of date/time right now.
    char szNow[100];
    {
        CDate NowTime;
        NowTime.SetToNow();
        WSTRING wstrNow;
        NowTime.GetLocalFormattedText(DF_SHORT_DATE | DF_SHORT_TIME, wstrNow);
        UnicodeToAscii(wstrNow, szNow);
    }

    //Get map of already-assigned MIDs from existing MIDS.h.
    DWORD dwLastMessageID = 0;
    WSTRING wstrHeaderFilepath;
    ASSIGNEDMIDS AssignedMIDs;
    {
        static const WCHAR wszMIDS_H[] = {{'M'},{'I'},{'D'},{'s'},{'.'},{'h'},{0}};
        //static const char szWB[] = "wb";
        wstrHeaderFilepath = pwzSrcPath;
        wstrHeaderFilepath += wszSlash;
        wstrHeaderFilepath += wszMIDS_H;
        GetAssignedMIDs(wstrHeaderFilepath.c_str(), AssignedMIDs, dwLastMessageID);
    }

    //Concat first part of MIDS.h.  
    string strMIDs =
        "//MIDs.h\r\n"
        "//Generated by DRODUTIL at ";
    strMIDs += szNow;
    strMIDs += ".\r\n"
        "\r\n"
        "#ifndef _MIDS_H_\r\n"
        "#define _MIDS_H_\r\n"
        "\r\n"
        "enum MID_CONSTANT {\r\n"
        "  //Range of MIDs between 0 and 99 are reserved for IDs that do not correspond to\r\n"
        "  //messages stored in the database.\r\n"
        "  MID_Success = 0,\r\n"
        "  MID_DatMissing = 1,\r\n"
        "  MID_DatNoAccess = 2,\r\n"
        "  MID_DatCorrupted_NoBackup = 3,\r\n"
        "  MID_DatCorrupted_Restored = 4,\r\n"
        "  MID_CouldNotOpenDB = 5,\r\n"
        "  MID_MemPerformanceWarning = 6,\r\n"
        "  MID_MemLowWarning = 7,\r\n"
        "  MID_MemLowExitNeeded = 8,\r\n"
        "  MID_LastUnstored = 99,\r\n";
    if (dwLastMessageID < 99) dwLastMessageID = 99;

    //Delete any existing basic messages from the database.  (MIDs < 10000 == basic).
    {
        c4_View MessageTextsView = TextStorage.View("MessageTexts");
        long lMessageTextCount = MessageTextsView.GetSize();
        for (long lMessageTextNo = lMessageTextCount - 1; lMessageTextNo >= 0; --lMessageTextNo)
        {
            if (p_MessageID( MessageTextsView[lMessageTextNo] ) < 10000)
                MessageTextsView.RemoveAt(lMessageTextNo);
        }
    }

    WSTRING wstrSearchPath = pwzSrcPath;
#ifdef WIN32
    //Concat search path.
    const WCHAR wszUNI[] = {{'*'},{'.'},{'u'},{'n'},{'i'},{0}};
    wstrSearchPath += wszSlash;
    wstrSearchPath += wszUNI;
    
    //Import every *.UNI file found at the source path.
    WIN32_FIND_DATA FindData;
    HANDLE hFindUNI = FindFirstFile(wstrSearchPath.c_str(), &FindData);
    if (hFindUNI != INVALID_HANDLE_VALUE)
    {
       do
       {
          char szFilename[MAX_PATH + 1];
          WSTRING wstrUNIFilepath = pwzSrcPath;
          wstrUNIFilepath += wszSlash;
          wstrUNIFilepath += FindData.cFileName;
          UnicodeToAscii(FindData.cFileName, szFilename);
#else
	char szSearchPath[wstrSearchPath.length()];
	UnicodeToAscii(wstrSearchPath, szSearchPath);
	DIR *pdFindUNI = opendir(szSearchPath);
	if (pdFindUNI)
	{
		struct dirent *pDir;
		while ((pDir = readdir(pdFindUNI)))
		{
			const size_t len = strlen(pDir->d_name);
			if (len < 4 || strcasecmp(pDir->d_name + len - 4, ".uni"))
				continue;

			WSTRING wstrUNIFilepath = pwzSrcPath;
			wstrUNIFilepath += wszSlash;
			WSTRING wstrFilename;
			AsciiToUnicode(pDir->d_name, wstrFilename);
			wstrUNIFilepath += wstrFilename;
#endif

         strMIDs +=
         "\r\n"
         "  //Messages from ";
#ifdef WIN32
         strMIDs += szFilename;
#else
			strMIDs += pDir->d_name;
#endif
         strMIDs += ":\r\n";

			if (!ImportUNI(wstrUNIFilepath.c_str(), TextStorage, AssignedMIDs, dwLastMessageID, strMIDs))
			{
				TextStorage.Rollback();
				return false;
			}
#ifdef WIN32
      } while (FindNextFile(hFindUNI, &FindData));

		FindClose(hFindUNI);
#else
		}

		closedir(pdFindUNI);
#endif
    }

    //Finish MIDs concatenation.
    strMIDs +=
        "\r\n"
        "  MID_END_UNUSED\r\n"
        "};\r\n"
        "\r\n"
        "#endif //...#ifndef _MIDS_H_\r\n";

    //Write MIDs.h.
    {
        CGameStream HeaderStream(wstrHeaderFilepath.c_str(), "wb");
        if (!HeaderStream.Write(strMIDs.c_str(), strMIDs.size()))
            {ASSERTP(false, "Failed to write header."); TextStorage.Rollback(); return false;}
    }

    //Commit database writes.
    TextStorage.Commit();
    return true;
}

//****************************************************************************************************
bool CUtil1_6::ImportUNI(
//Imports contents of one UNI file into the text dat.
//
//Params:
  const WCHAR *pwzFilepath,         //(in)      Full path to UNI file.
  c4_Storage &TextStorage,          //(in)      text.dat
  const ASSIGNEDMIDS &AssignedMIDs, //(in)      Already assigned MIDs.
  DWORD &dwLastMessageID,           //(in/out)  Accepts last message ID used.  Returned with new value
                                    //          of last message ID used.
  string &strMIDs)                  //(in/out)  C++ MID_* enumerations appended to this.
//
//Returns:
//True if successful, false if not.
const
{
  /* Example Format 1.

  [MID_NoText]
  [English]

  [MID_Yes]
  [English]
  &Yes

  [French]
  &Oui

  [MID_PressAnyKeyToContinue]
  [English]
  Press any key to continue.

  [French]
  Appuyez n'importe quelle touche pour continuer.
  

  Example Format 2--no whitespace.

  [MID_NoText][English][MID_Yes][English]&Yes[French]&Oui[MID_PressAnyKeyToContinue][Engli
  sh]Press any key to continue.[French]Appuyez n'importe quelle touche pour continuer.  
  */

# define IS_WHITESPACE(c) \
    ((WCHAR)(c)==' ' || (WCHAR)(c)==9 || (WCHAR)(c)=='\r' || (WCHAR)(c)=='\n' || (WCHAR)(c)==0)

  //Read the whole .UNI file into a buffer.
  CStretchyBuffer Source;
  if (!CFiles::ReadFileIntoBuffer(pwzFilepath, Source)) return false;

  const WCHAR wszNameTagStart[] = {{'M'},{'I'},{'D'},{'_'},{0}};
  const WCHAR wszEnglish[] = {{'E'},{'n'},{'g'},{'l'},{'i'},{'s'},{'h'},{0}};
  const WCHAR wszFrench[] = {{'F'},{'r'},{'e'},{'n'},{'c'},{'h'},{0}};
  const WCHAR wszRussian[] = {{'R'},{'u'},{'s'},{'s'},{'i'},{'a'},{'n'},{0}};
  char szNameTag[MAXLEN_NAMETAG + 1];

  const WCHAR *pSeek = (const WCHAR *)((const BYTE *)Source);
  const WCHAR *pStop = (const WCHAR *)((const BYTE *) pSeek + Source.Size());
  const WCHAR *pTagStart;
  UINT wTagLen;
  while (pSeek <= pStop)
  {
    //Looking for a name tag.
    while (true)
    {
      while (*pSeek != '[' && pSeek < pStop) ++pSeek;
      if (pSeek + 5 >= pStop) return true;
      if (WCSncmp(pSeek + 1, wszNameTagStart, (sizeof(wszNameTagStart)-1)/2)!=0) 
        continue; //Some other tag--keep looking.
      else
        break;
    }

    //
    //Found a name tag.
    //

    //Find end of name tag.
    ASSERT(*pSeek == '['); //pSeek is at '[' in "[MID_*]".
    pTagStart = pSeek;
    while (*pSeek != ']' && pSeek < pStop) ++pSeek;
    if (pSeek >= pStop) {ASSERTP(false, "Parse name tag error."); return false;}
    wTagLen = pSeek - pTagStart + 1;
    if (wTagLen - 2 > MAXLEN_NAMETAG) {ASSERTP(false, "Parse name tag error.(2)"); return false;}
    pSeek = pTagStart + 1;
    char *pszWriteNameTag = szNameTag;
    while (*pSeek != ']')
      *(pszWriteNameTag++) = (char) pWCv(pSeek++);
    *pszWriteNameTag = 0;

    //Use previously-assigned MID or a new one?
    DWORD dwUseMessageID;
    {
        ASSIGNEDMIDS::const_iterator iMID;
        string strNameTag = szNameTag;
        iMID = AssignedMIDs.find(strNameTag);
        if (iMID == AssignedMIDs.end())
            dwUseMessageID = ++dwLastMessageID; //Use new MID.
        else    //Use existing MID.
        {
            dwUseMessageID = iMID->second;
        }
    }

    //Copy name to MIDs concatenation.
    {
        //In the format "  tagname = tagvalue,".  The explicit value assignment is not necessary, but makes 
        //for easier parsing for code that is planned later.
        static char szNumBuf[20];
        strMIDs += "  ";
        strMIDs += szNameTag;
        strMIDs += " = ";
        strMIDs += _itoa(dwUseMessageID, szNumBuf, 10);
        strMIDs += ",";
        if (dwUseMessageID == dwLastMessageID)
            strMIDs += " //New.";
        strMIDs += "\r\n";
    }

    //Look for language tags with associated text to store.
    pSeek = pTagStart + wTagLen;
    while(true)
    {
      //Find next language tag.
      while (*pSeek != '[' && pSeek < pStop) ++pSeek;
      if (pSeek >= pStop) return true;
      pTagStart = pSeek;
      while (*pSeek != ']' && pSeek < pStop) ++pSeek;
      if (pSeek + 5 >= pStop) {ASSERTP(false, "Find next language tag error."); return false;}
      UINT wTagLen = pSeek - pTagStart + 1;
      if (WCSncmp(pTagStart + 1, wszNameTagStart, (sizeof(wszNameTagStart)-1)/2)==0)
        {pSeek = pTagStart; break;} //New name tag found.

      //Which language?
      LANGUAGE_CODE eLanguage;
      if (WCSncmp(pTagStart + 1, wszEnglish, wTagLen - 2)==0)
        eLanguage = English;
      else if (WCSncmp(pTagStart + 1, wszFrench, wTagLen - 2)==0)
        eLanguage = French;
      else if (WCSncmp(pTagStart + 1, wszRussian, wTagLen - 2)==0)
        eLanguage = Russian;
      else
      {
        ASSERTP(false, "Unexpected language."); return false;
      }
      
      //Find start of text.
      pSeek = pTagStart + wTagLen;
      while (pSeek < pStop && IS_WHITESPACE(*pSeek)) ++pSeek;
      if (pSeek + 1 >= pStop) return true;
      if (*pSeek == '[' && pSeek[1] != '[') //An unescaped left bracket was found before any text.
      {
        //This is an empty string.
        AddMessageText(TextStorage, dwUseMessageID, eLanguage, wszEmpty);
        continue;
      }
      pTagStart = pSeek;

      //Find end of text by finding first unescaped left bracket and then backing up
      //past any whitespace.
      while (true)
      {
        while (pSeek < pStop && *pSeek != '[') ++pSeek;
        if (pSeek == pStop) break;
        if (pSeek + 1 < pStop && pSeek[1] == '[') //Escaped left bracket.
        {
          pSeek += 2;
          continue;
        }
        else //Unescaped left bracket.
          break;
      }
      --pSeek;
      while (pSeek > pTagStart && IS_WHITESPACE(*pSeek)) --pSeek;
      if (pSeek < pTagStart) {ASSERTP(false, "Find end of text error."); return false;}
      ++pSeek;
      wTagLen = pSeek - pTagStart;
      if (wTagLen > 10000) {ASSERTP(false, "Find end of text error.(2)"); return false;} //10000 = unreasonably lengthy text.

      //Copy text into a buffer.
      WCHAR *pwzText = new WCHAR[wTagLen + 1];
      WCHAR *pwzWrite = pwzText;
      const WCHAR *pRead = pTagStart, *pStopRead = pRead + wTagLen;
      if (!pwzText) {ASSERTP(false, "Alloc failed."); return false;}
      while (pRead < pStopRead)
      {
        if (*pRead == '[') //Escaped left bracket.
        {
          ASSERT(pRead[1] == '[' && pRead + 1 < pStopRead);
          pWCv(pwzWrite++) = '[';
          pRead += 2;
        }
        else if (*pRead == ']') //Escaped right bracker.
        {
          ASSERT(pRead[1] == ']' && pRead + 1 < pStopRead);
          pWCv(pwzWrite++) = ']';
          pRead += 2;
        }
        else
          *(pwzWrite++) = *(pRead++);
      }
      pWCv(pwzWrite) = 0;

      //Add MessageTexts record to database.
      AddMessageText(TextStorage, dwUseMessageID, eLanguage, pwzText);
      delete [] pwzText;
    } //...for each language tag.
  } //...for each name tag.

# undef IS_WHITESPACE
  
  return true;
}

//****************************************************************************************************
void CUtil1_6::AddMessageText(
//Add MessageTexts record to database.
//
//Params:
    c4_Storage &TextStorage,  //(in)  text.dat
    DWORD dwMessageID,        //(in)  These params supply field values for record.
    LANGUAGE_CODE eLanguage,  //(in)
    const WCHAR *pwszText)    //(in)
{
  //Get next MessageTextID.
  c4_View IncrementedIDsView = TextStorage.View("IncrementedIDs");
	DWORD dwMessageTextID = p_MessageTextID(IncrementedIDsView[0]);
	p_MessageTextID(IncrementedIDsView[0]) = ++dwMessageTextID;

  //Add record.
  c4_Bytes TextBytes(pwszText, (WCSlen(pwszText) + 1)*sizeof(WCHAR));
  c4_View MessageTextsView = TextStorage.View("MessageTexts");
  MessageTextsView.Add( 
      p_MessageTextID[ dwMessageTextID ] +
      p_MessageID[ dwMessageID ] +
      p_LanguageCode[ (int)eLanguage ] +
      p_MessageText[ TextBytes ] );
}

//****************************************************************************************************
void CUtil1_6::GetAssignedMIDs(
//Populate map of MID names-to-values from existing MIDs.h file.
//
//Params:
    const WCHAR *pwzMIDFilepath,        //(in)  Path to mids.h.
    ASSIGNEDMIDS &AssignedMIDs,         //(out) Receives name/value elements.
    DWORD &dwLastMessageID)             //(out) Receives largest MID value found.
const
{
    dwLastMessageID = 0;

    //Read file into buffer.
    CStretchyBuffer buf;
    CFiles files;
    if (!files.ReadFileIntoBuffer(pwzMIDFilepath, buf)) {ASSERTP(false, "Read file failed."); return;}
    
    //Skip to first "{" which indicates start of enum values.
    const BYTE *pszSeek = (const BYTE *)buf;
    while (*pszSeek && *pszSeek != '{') ++pszSeek;
    if (!*pszSeek) return;

    //Each iteration reads one name/value pair and adds to assigned mids param.
    const UINT MAXLEN_VALUE = 15;
    char szName[MAXLEN_NAMETAG + 1], szValue[MAXLEN_VALUE + 1], *pszWrite;
    while (true)
    {
        //Find next "MID_".  If I don't find it then return.
        while (*pszSeek)
        {
            if (pszSeek[0] == 'M' && pszSeek[1] == 'I' && pszSeek[2] == 'D' && pszSeek[3] == '_')
                break;
            ++pszSeek;
        }
        if (!*pszSeek) return;
        
        //Found MID name.  Copy until space found.
        ASSERT(*pszSeek == 'M');
        pszWrite = szName;
        while (*pszSeek && *pszSeek != ' ')
        {
            if (pszWrite - szName > static_cast<int>(MAXLEN_NAMETAG)) {ASSERTP(false, "Copy MID name error."); return;}
            *(pszWrite++) = *(pszSeek++);
        }
        if (!*pszSeek) return;
        *pszWrite = 0;

        //Check for end of enum values.  Last value, "MID_Count", will not have a 
        //space/equal after it.
        if (pszSeek[0] != ' ' || pszSeek[1] != '=' || pszSeek[2] != ' ') return;
        pszSeek += 3;

        //Copy value until space found.
        pszWrite = szValue;
        while (*pszSeek >= '0' && *pszSeek <= '9')
        {
            if (pszWrite - szValue > static_cast<int>(MAXLEN_VALUE)) {ASSERTP(false, "Copy value error."); return;}
            *(pszWrite++) = *(pszSeek++);
        }
        if (!*pszSeek) return;
        *pszWrite = 0;
        
        //Add element.
        string strName = szName;
        DWORD dwValue = atol(szValue);
        AssignedMIDs[strName] = dwValue;

        //If this is the largest MID value, remember it.
        if (dwValue > dwLastMessageID) dwLastMessageID = dwValue;
    
    } //...keep looping forever.  Exit condition inside of loop.
}

// $Log: Util1_6.cpp,v $
// Revision 1.26  2003/10/06 02:51:19  erikh2000
// Updated MIDs.h generation routine with new unstored MID constants.
//
// Revision 1.25  2003/10/06 02:48:36  erikh2000
// Added descriptions to assertians.
//
// Revision 1.24  2003/09/05 18:44:22  erikh2000
// Added code to specify some new non-DB messages in MIDs.h.
//
// Revision 1.23  2003/08/14 03:56:12  mrimer
// Fixed demos broken by new tar mother and roach egg spawning timing.
//
// Revision 1.22  2003/07/24 19:43:12  mrimer
// Port fixes.
//
// Revision 1.21  2003/07/24 18:16:39  mrimer
// More Linux port fixes from Gerry JJ.
//
// Revision 1.20  2003/07/24 18:01:33  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.19  2003/07/19 02:08:20  mrimer
// Added (empty) EndHoldMessageID field to Dugan's Dungeon.
//
// Revision 1.18  2003/07/15 00:25:31  mrimer
// Froze Dugan's Dungeon hold's LastModified timestamp to be the same as the Created time.
//
// Revision 1.17  2003/07/14 16:37:52  mrimer
// Changed MID_Count to MID_END_UNUSED.
//
// Revision 1.16  2003/06/30 17:46:58  schik
// Fixed the orientation of Tar Mother Eyes in Dugan's Dungeon
//
// Revision 1.15  2003/06/27 17:26:21  mrimer
// Added hold editing rights for Dugan's Dungeon (Conquerors).
//
// Revision 1.14  2003/06/22 17:49:09  erikh2000
// MIDs are given previously assigned values from existing MIDs.h.
//
// Revision 1.13  2003/06/17 14:34:30  mrimer
// Fixed some bugs uncovered by other bug fixes.  Code maintenance.
//
// Revision 1.12  2003/06/13 00:15:00  mrimer
// Froze creation timestamp of non-local player records.
//
// Revision 1.11  2003/06/09 21:36:21  mrimer
// Modified to not add the continue saved game for the stub player record in 1.5.
//
// Revision 1.10  2003/06/09 21:24:19  erikh2000
// Fixed bad constant for player IDs.
//
// Revision 1.9  2003/05/29 00:52:48  erikh2000
// Stub player record from drod1_5.dat is now not copied to player.dat.
//
// Revision 1.8  2003/05/28 23:21:44  erikh2000
// DbPRops1_5.h moved to drodlib dir--revised include.
//
// Revision 1.7  2003/05/25 22:54:59  erikh2000
// Fixed some includes.
//
// Revision 1.6  2003/05/23 21:44:59  mrimer
// Tweaking.
//
// Revision 1.5  2003/05/22 23:42:15  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.4  2003/05/19 21:14:33  mrimer
// Some code maintenance.
//
// Revision 1.3  2003/05/08 23:34:51  erikh2000
// Deleting, creating, and importing works.
//
// Revision 1.2  2003/04/28 18:04:55  erikh2000
// It compiles now.
//
// Revision 1.1  2003/04/28 14:25:40  erikh2000
// Initial check-in.
//
