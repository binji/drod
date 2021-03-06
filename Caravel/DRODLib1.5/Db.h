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

//Db.h
//Declarations for CDb.
//Top-level class for accessing DROD data from database.

#ifndef DB_H
#define DB_H

#include "DbSavedGames.h"
#include "DbDemos.h"
#include "DbHolds.h"
#include "DbLevels.h"
#include "DbRooms.h"
#include "DbPlayers.h"

//Hard-coded hold IDs that are always the same each time a Holds table is created.
const DWORD HOLD_DUGANS_DUNGEON = 1L;

class CCurrentGame;

//******************************************************************************************
class CDb : public CDbBase
{
public:

	CDb(void);
	~CDb(void);
	
	//Use these methods to access data in the context of a game.  The CCurrentGame
	//object will simplify many operations and prevent errors.
	CCurrentGame *	GetSavedCurrentGame(const DWORD dwSavedGameID, CCueEvents &CueEvents,
			bool bRestoreAtRoomStart = false);
	CCurrentGame *	GetNewCurrentGame(const DWORD dwHoldID, CCueEvents &CueEvents);
	CCurrentGame *	GetNewTestGame(const DWORD dwRoomID, CCueEvents &CueEvents,
			const UINT wX, const UINT wY, const UINT wO);
	
	//Get the current player with all of his settings.
	CDbPlayer *		GetCurrentPlayer(void)
		{return this->Players.GetByID(GetPlayerID());}

	DWORD			GetHoldID(void);
	DWORD			GetPlayerID(void);

	static MESSAGE_ID	ImportFromFile(const char *pszFilepath);

	void			SetHoldID(const DWORD dwNewHoldID);
	void			SetPlayerID(const DWORD dwNewPlayerID);

	//Call Update() in the derived data members below instead.
	virtual bool	Update(void) {return false;}

	//Use these members to access data directly.  Requires more knowledge of 
	//the database.
	CDbDemos		Demos;
	CDbHolds		Holds;
	CDbLevels		Levels;
	CDbPlayers		Players;
	CDbRooms		Rooms;
	CDbSavedGames	SavedGames;

private:
	static DWORD		dwCurrentHoldID, dwCurrentPlayerID;
};

#endif //...#ifndef DB_H

// $Log: Db.h,v $
// Revision 1.1  2003/02/25 00:01:22  erikh2000
// Initial check-in.
//
// Revision 1.13  2002/12/22 01:41:51  mrimer
// Added XML import/export methods.  Added ImportFromFile().
//
// Revision 1.12  2002/11/22 02:09:23  mrimer
// Added GetNewTestGame().
//
// Revision 1.11  2002/11/14 19:02:36  mrimer
// Added methods to support multiple player and hold selection.
// Made some parameters const.
//
// Revision 1.10  2002/09/03 21:25:31  erikh2000
// Removed a commented-out #include.
//
// Revision 1.9  2002/07/05 17:50:40  mrimer
// Minor fixes.
//
// Revision 1.8  2002/06/09 06:08:42  erikh2000
// Added members used for retrieving player information.
//
// Revision 1.7  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.6  2002/02/24 01:25:03  erikh2000
// Added CueEvents parameter to GetSavedCurrentGame() and GetNewCurrentGame().
//
// Revision 1.5  2002/02/15 02:46:56  erikh2000
// Added option to restore from beginning of room to CDb::GetSavedCurrentGame().
//
// Revision 1.4  2001/12/16 02:07:05  erikh2000
// Added Rooms, Levels, Demos, and SavedGames members to CDb.
//
// Revision 1.3  2001/12/08 02:38:12  erikh2000
// Added Holds member to CDb.
//
// Revision 1.2  2001/10/26 23:01:00  erikh2000
// Removed unused members of CDb.
//
// Revision 1.1.1.1  2001/10/01 22:20:07  erikh2000
// Initial check-in.
//
