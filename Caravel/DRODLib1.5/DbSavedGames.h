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

#pragma warning(disable: 4786)

#include "DbBase.h"
#include "DbCommands.h"
#include "DbDate.h"
#include "DbRefs.h"
#include "IDList.h"

//Save types.
typedef enum tagSaveType
{
	ST_Unknown		= 0,
	ST_Continue,	//1
	ST_Demo,		//2
	ST_LevelBegin,	//3
	ST_RoomBegin,	//4
	ST_Checkpoint	//5
} SAVETYPE;

//******************************************************************************************
class CDbSavedGames;
class CCurrentGame;
class CDbRoom;
class CDbSavedGame : public CDbBase
{
protected:
	friend CDbSavedGames;
	friend CCurrentGame;

	CDbSavedGame(void);

public:
	virtual ~CDbSavedGame(void);

	CDbRoom *	GetRoom(void) const;
	bool		Load(const DWORD dwSavedGameID);
	virtual MESSAGE_ID	SetProp(const PROPTYPE pType, char* const str,
			PrimaryKeyMaps &Maps, bool &bSaveRecord);
	virtual bool	Update(void);

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
	CDbDate		Created;
	CDbDate		LastUpdated;
	CDbCommands	Commands;
	
private:
	void		Clear(void);
	bool		UpdateExisting(void);
	bool		UpdateNew(void);
};

//******************************************************************************************
class CDb;
class CDbRoom;
class CDbLevel;
class CDbHold;
class CDbSavedGames : public CDbBase
{
protected:
	friend CDb;
	friend CDbRoom;
	friend CDbLevel;
	friend CDbHold;
	CDbSavedGames(void)
		: dwFilterByHoldID(0L), dwFilterByLevelID(0L), dwFilterByPlayerID(0L)
		, dwFilterByRoomID(0L)
		, pCurrentRow(NULL)
		, bIsMembershipLoaded(false)
	{ }

public:
	~CDbSavedGames(void) {}
	
	void			Delete(const DWORD dwSavedGameID);
	string		ExportXML(const DWORD dwSavedGameID, CDbRefs &dbRefs);
	CDbSavedGame *	GetByID(const DWORD dwSavedGameID) const;
	CDbSavedGame *	GetFirst(void);
	void			GetIDs(CIDList &IDs);
	CDbSavedGame * GetNew(void);
	CDbSavedGame *	GetNext(void);
	void			FilterByHold(const DWORD dwSetFilterByHoldID);
	void			FilterByLevel(const DWORD dwSetFilterByLevelID);
	void			FilterByPlayer(const DWORD dwSetFilterByPlayerID);
	void			FilterByRoom(const DWORD dwSetFilterByRoomID);
	DWORD			FindByCheckpoint(const DWORD dwRoomID, const UINT wCol,
			const UINT wRow);
	DWORD			FindByContinue();
	DWORD			FindByContinueLatest();
	DWORD			FindByLevelBegin(const DWORD dwLevelID);
	DWORD			FindByLevelLatest(const DWORD dwLevelID);
	DWORD			FindByRoomBegin(const DWORD dwRoomID);
	DWORD			FindByRoomLatest(const DWORD dwRoomID);
	virtual bool	Update(void) {return false;}

private:
	void		LoadMembership(void);
	void		LoadMembership_All(void);
	void		LoadMembership_ByHold(const DWORD dwByHoldID);
	void		LoadMembership_ByLevel(const DWORD dwByLevelID);
	void		LoadMembership_ByPlayer(const DWORD dwByPlayerID);
	void		LoadMembership_ByRoom(const DWORD dwByRoomID);

	bool		bIsMembershipLoaded;
	DWORD		dwFilterByHoldID;
	DWORD		dwFilterByLevelID;
	DWORD		dwFilterByPlayerID;
	DWORD		dwFilterByRoomID;
	CIDList		MembershipIDs;
	IDNODE *	pCurrentRow;
};

#endif //...#ifndef DBSAVEDGAMES_H

// $Log: DbSavedGames.h,v $
// Revision 1.1  2003/02/25 00:01:31  erikh2000
// Initial check-in.
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
