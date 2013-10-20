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

//DbSavedGames.h
//Declarations for CDbSavedGames and CDbSavedGame.
//Classes for accessing saved game data from database.

#ifndef DBSAVEGAMES_H
#define DBSAVEGAMES_H

#include "DbVDInterface.h"
#include "DbCommands.h"
#include <BackEndLib/Date.h>

//Save types.
typedef enum tagSaveType
{
	ST_Unknown		= 0,
	ST_Continue,	//1
	ST_Demo,		//2
	ST_LevelBegin,	//3
	ST_RoomBegin,	//4
	ST_Checkpoint,	//5
	ST_EndHold		//6
} SAVETYPE;

//******************************************************************************************
class CDbSavedGames;
class CCurrentGame;
class CDbRoom;
class CDbSavedGame : public CDbBase
{
protected:
	friend class CDbSavedGames;
	friend class CCurrentGame;
	friend class CDbVDInterface<CDbSavedGame>;

	CDbSavedGame();

public:
	virtual ~CDbSavedGame();

	CDbRoom *	GetRoom() const;
	bool		Load(const DWORD dwSavedGameID);
	virtual MESSAGE_ID	SetProperty(const PROPTYPE pType, char* const str,
			CImportInfo &info, bool &bSaveRecord);
	virtual bool	Update();

	DWORD		dwSavedGameID;
	DWORD		dwRoomID;
	DWORD		dwPlayerID;
	bool		bIsHidden;
	
	SAVETYPE	eType;
	UINT		wCheckpointX;
	UINT		wCheckpointY;
	UINT		wStartRoomX;
	UINT		wStartRoomY;
	UINT		wStartRoomO;
	CIDList		ExploredRooms;
	CIDList		ConqueredRooms;
	CDate		Created;
	CDate		LastUpdated;
	CDbCommands	Commands;
	
private:
	void		Clear();
	bool		UpdateExisting();
	bool		UpdateNew();
};

//******************************************************************************************
class CDb;
class CDbRoom;
class CDbLevel;
class CDbHold;
class CDbSavedGames : public CDbVDInterface<CDbSavedGame>
{
protected:
	friend class CDb;
	friend class CDbRoom;
	friend class CDbLevel;
	friend class CDbHold;
	CDbSavedGames()
		: CDbVDInterface<CDbSavedGame>("SavedGames", p_SavedGameID)
		, dwFilterByHoldID(0L), dwFilterByLevelID(0L), dwFilterByPlayerID(0L)
		, dwFilterByRoomID(0L)
      , bLoadHidden(false)
	{ }

public:
	virtual ~CDbSavedGames() {}

	virtual void		Delete(const DWORD dwSavedGameID);
   void        DeleteForRoom(const DWORD dwRoomID);
	virtual string		ExportXML(const DWORD dwVDID, CDbRefs &dbRefs, const bool bRef=false);
	void			FilterByHold(const DWORD dwSetFilterByHoldID);
	void			FilterByLevel(const DWORD dwSetFilterByLevelID);
	void			FilterByPlayer(const DWORD dwSetFilterByPlayerID);
	void			FilterByRoom(const DWORD dwSetFilterByRoomID);
   void        FindHiddens(const bool bFlag);

	DWORD			FindByCheckpoint(const DWORD dwRoomID, const UINT wCol,
			const UINT wRow);
	DWORD			FindByContinue();
	DWORD			FindByContinueLatest(const DWORD dwLookupPlayerID);
	DWORD			FindByEndHold(const DWORD dwHoldID);
	DWORD			FindByLevelBegin(const DWORD dwLevelID);
	DWORD			FindByLevelLatest(const DWORD dwLevelID);
	DWORD			FindByRoomBegin(const DWORD dwRoomID);
	DWORD			FindByRoomLatest(const DWORD dwRoomID);

   DWORD       GetHoldIDofSavedGame(const DWORD dwSavedGameID) const;

  	DWORD			SaveNewContinue(const DWORD dwPlayerID);

private:
	virtual void		LoadMembership();
	void		LoadMembership_All();
	void		LoadMembership_ByHold(const DWORD dwByHoldID);
	void		LoadMembership_ByLevel(const DWORD dwByLevelID);
	void		LoadMembership_ByPlayer(const DWORD dwByPlayerID);
	void		LoadMembership_ByRoom(const DWORD dwByRoomID);

	DWORD		dwFilterByHoldID;
	DWORD		dwFilterByLevelID;
	DWORD		dwFilterByPlayerID;
	DWORD		dwFilterByRoomID;
   bool     bLoadHidden;  //whether hidden saved games should be loaded also
};

#endif //...#ifndef DBSAVEDGAMES_H

// $Log: DbSavedGames.h,v $
// Revision 1.34  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.33  2003/08/20 22:13:37  mrimer
// Revised saved game handling.
//
// Revision 1.32  2003/07/09 21:29:41  mrimer
// Made deletion routine more robust.
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.31  2003/06/26 17:25:19  mrimer
// Added GetHoldIDofSavedGame().  FindByContinueLatest() now takes parameter.
//
// Revision 1.30  2003/06/24 20:12:10  mrimer
// Augmented filtering to optionally include hidden saved games.
// Fixed some bugs relating to this (in export and player deletion).
//
// Revision 1.29  2003/05/21 03:05:26  mrimer
// (Re)Initialized two uninitialized member vars.
//
// Revision 1.28  2003/05/20 18:11:39  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.27  2003/05/03 23:29:23  mrimer
// Added support for a new save game type: ST_EndHold, that gets saved when a
// player exits a hold, to mark that they have completed it.
//
// Revision 1.26  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.25  2003/02/24 17:06:33  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.24  2002/12/22 01:27:14  mrimer
// Added XML import/export support.
// Removed BETA conditional compiles.
//
// Revision 1.23  2002/11/22 02:01:06  mrimer
// Added DeleteContinuesForPlayer().  Revised FindByContinue() to add continue saved game if none exists.
//
// Revision 1.22  2002/11/14 19:18:53  mrimer
// Added FilterByPlayer support.
// Moved member initialization into constructor initialization list.
// Made some parameters const.
//
// Revision 1.21  2002/08/30 00:24:33  erikh2000
// Put in temporary code to allow CDbDemos to construct a CDbSavedGame.
//
// Revision 1.20  2002/07/21 00:12:22  erikh2000
// Fixed error that stopped membership from loading.
//
// Revision 1.19  2002/06/16 06:21:52  erikh2000
// Added method to delete a saved game.
//
// Revision 1.18  2002/06/15 18:29:54  erikh2000
// Changed code so that storage is not committed after each database write.
//
// Revision 1.17  2002/06/09 06:18:03  erikh2000
// Added new PlayerID field.
//
// Revision 1.16  2002/05/10 22:32:29  erikh2000
// Added more ways to find saved games using DbSavedGames.
//
// Revision 1.15  2002/04/28 23:50:03  erikh2000
// Class now supports enumeration saved games using GetFirst() and GetNexT() methods.  Saved games can be filtered by hold, level, and room.
//
// Revision 1.14  2002/03/04 22:22:34  erikh2000
// Added CDbSavedGame::wCheckpointX and CDbSavedGame::wCheckpointY members.
//
// Revision 1.13  2002/02/28 04:52:06  erikh2000
// Added save CDbSavedGame->eType member to hold data from new "Type" field of "SavedGames" table.
// Added CDbSavedGames::FindByRoomBegin(), CDbSavedGames::FindByLevelBegin(), and CDbSavedGames::FindByCheckpoint().
//
// Revision 1.12  2002/02/10 02:36:25  erikh2000
// Moved CDbSavedGame::wTurnsTaken, wX, wY, wO, bIsPlacingMimic, wMimicCursorX, wMimicCursorY, and bIsVisible to CCurrentGame.
//
// Revision 1.11  2002/02/07 22:34:30  erikh2000
// Wrote CDbSavedGame::Update().
//
// Revision 1.10  2001/12/16 02:19:53  erikh2000
// Allowed CDb to construct CDbSavedGames.
//
// Revision 1.9  2001/12/08 01:42:28  erikh2000
// Added CDbSavedGame::Commands member var.
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
