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
 * Michael Welsh Duggan (md5i), Richard Cookney (timeracer), JP Burford (jpburford),
 * John Wm. Wicks (j_wicks), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//CurrentGame.h
//Declarations for CCurrentGame.h.
//
//GENERAL
//
//Class for accessing and manipulating data used for current game.  This class should
//be completely insulated from graphics, sound, and input devices.  CCurrentGame can be 
//used without any UI, since there are no UI-related calls inside of it or its called
//code.
//
//USAGE
//
//Get an instance of CCurrentGame using either CDb::GetSavedCurrentGame() or 
//CDb::GetNewCurrentGame().  Your commands will probably either come from a UI (user types
//commands with keyboard) or from a prerecorded game.  These commands are passed via 
//CCurrentGame::ProcessCommand().  ProcessCommand() will return cue events, a full list of
//which are available from CueEvents.h.  Some cue events have to be responded to or the
//game will not appear to work, i.e. CID_MonsterKilledPlayer.  Others cue events, like
//CID_TarDestroyed, do not require handling, but can be used to cue sound and graphical 
//effects.
//
//Multiple instances of CCurrentGame may be open simultaneously, but don't access 
//CCurrentGame instances on different threads.
//
//DOES MY NEW METHOD GO IN CDbRoom OR CCurrentGame?
//
//This question comes up because CDbRoom and CCurrentGame both seem to lay claim to
//the same problem space.  CDbRoom is for accessing room data both before and after the
//swordsman has arrived in that room.  CCurrentGame contains the position of the swordsman,
//game state information, and an instance of CDbRoom that is the current room the swordsman
//is in.
//
//Here is the process to figure out which place the method goes:
//
//  Does the method access members of just one of the classes?  If so, then method goes 
//  in that class.
//
//  Does the method need to write to a member of CCurrentGame?  If so, then method goes 
//  in CCurrentGame.
//
//Assuming that the above two questions were answered "no", the method should go in CDbRoom.

#ifndef CURRENTGAME_H
#define CURRENTGAME_H

#include "Assert.h"
#include "AttachableObject.h"
#include "CoordStack.h"
#include "DbSavedGames.h"
#include "Monster.h"
#include "MonsterMessage.h"
#include "Swordsman.h"
#include "Types.h"

#include <list>
#include <string>

using namespace std;

//Class for storing one set of coords.
class CCoord : public CAttachableObject
{
public:
	CCoord(const UINT wSetCol, const UINT wSetRow) 
		: wCol(wSetCol), wRow(wSetRow)
	{ }
	UINT wCol, wRow;
};

//Class for storing coords plus direction of movement onto.
class CMoveCoord : public CAttachableObject
{
public:
	CMoveCoord(const UINT wSetCol, const UINT wSetRow, const UINT wSetO) 
		: wCol(wSetCol), wRow(wSetRow), wO(wSetO)
	{ }
	UINT wCol, wRow, wO;
};

class CHighlightDemoInfo : public CAttachableObject
{
public:
	CHighlightDemoInfo(const DWORD dwSetRoomID, const DWORD dwSetDemoID)
		: dwRoomID(dwSetRoomID), dwDemoID(dwSetDemoID)
	{ }
	DWORD	dwRoomID;
	DWORD	dwDemoID;
};

class CDbLevel;
class CDbHold;

//Type used for grouping all the demo recording vars together.
typedef struct tagDemoRecInfo
{
	DWORD	dwDescriptionMessageID;	//Description of demo.
	UINT	wBeginTurnNo;			//Turn at which recording began.
	DWORD	dwPrevDemoID;			//DemoID for a previous demo record with recording
									//for room visited before the current.  That record
									//may be updated to link to demo record storing recording
									//for current room.
	DWORD	dwFirstDemoID;			//DemoID of first demo in a series or demoID of the only
									//demo if there was just one.
} DEMO_REC_INFO;

//Auto-save option flags.
const DWORD ASO_NONE = 0L;
const DWORD ASO_CHECKPOINT = 1L;
const DWORD ASO_ROOMBEGIN = 2L;
const DWORD ASO_LEVELBEGIN = 4L;
const DWORD ASO_CONQUERDEMO = 8L;
const DWORD ASO_DIEDEMO = 16L;
const DWORD ASO_HIGHLIGHTDEMO = 32L;
const DWORD ASO_DEFAULT = ASO_CHECKPOINT | ASO_ROOMBEGIN | ASO_LEVELBEGIN | ASO_HIGHLIGHTDEMO;

//*******************************************************************************
class CDb;
class CMimic;
class CCurrentGame : public CDbSavedGame
{
protected:
	friend CDb;
	CCurrentGame(void);

public:
	~CCurrentGame(void);

	void		BeginDemoRecording(const WCHAR *pwczSetDescription);
	void		Clear(void);
	void		ProcessSwordHit(const UINT wX, const UINT wY, CCueEvents &CueEvents, 
			CMimic *pMimic = NULL);
	DWORD		EndDemoRecording(void);
	void		FreezeCommands(void);
	DWORD		GetChecksum(void) const;
	UINT		GetRoomExitDirection(const UINT wMoveO) const;
	UINT		GetSwordMovement() const
			{return swordsman.wSwordMovement;}	//note: set in ProcessSwordsman()
	bool		IsCurrentLevelComplete(void) const;
	bool		IsCurrentRoomConquered(void) const;
	bool		IsCurrentRoomExplored(void) const;
	bool		IsDemoRecording(void) const {return this->bIsDemoRecording;}
	bool		IsLevelStart(void);
	bool		IsRoomAtCoordsConquered(const DWORD dwRoomX, const DWORD dwRoomY) const;
	bool		IsRoomAtCoordsExplored(const DWORD dwRoomX, const DWORD dwRoomY) const;
	bool		LoadFromHold(const DWORD dwHoldID, CCueEvents &CueEvents);
	bool		LoadFromLevel(const DWORD dwLevelID, CCueEvents &CueEvents);
	bool		LoadFromRoom(const DWORD dwRoomID, CCueEvents &CueEvents,
			const UINT wX, const UINT wY, const UINT wO);
	bool		LoadFromSavedGame(const DWORD dwSavedGameID, CCueEvents &CueEvents,
			bool bRestoreAtRoomStart = false);
	bool		LoadNewRoomForExit(const UINT wExitO, CCueEvents &CueEvents);
	void		ProcessCommand(const int nCommand, CCueEvents &CueEvents);
	void		RestartRoom(CCueEvents &CueEvents);
	void		RestartRoomFromLastCheckpoint(CCueEvents &CueEvents);
	void		SaveToContinue(void);
	void		SaveToRoomBegin(void);
	void		SaveToLevelBegin(void);
	void		SaveToCheckpoint(void);
	void		SetAutoSaveOptions(const DWORD dwSetAutoSaveOptions)
			{this->dwAutoSaveOptions = dwSetAutoSaveOptions;}
	void		SetCurrentRoomConquered(void);
	void		SetCurrentRoomExplored(void);
	void		SetHighlightRoomIDs(const CIDList &SetRoomIDs);
	void		SetRoomStatusFromAllSavedGames(const DWORD dwCurrentRoomID);
	void		SetSwordsman(const UINT wSetX, const UINT wSetY);
	bool		SetSwordsmanToEastExit(void);
	bool		SetSwordsmanToNorthExit(void);
	bool		SetSwordsmanToSouthExit(void);
	bool		SetSwordsmanToWestExit(void);
	void		SetTurn(const UINT wSetTurnNo, CCueEvents &CueEvents);
	bool		SwordsmanIsDying(void) const;
	void		UndoCommand(CCueEvents &CueEvents);
	void		UndoCommands(const UINT wUndoCount, CCueEvents &CueEvents);
	void		UnfreezeCommands(void);
	bool		WalkDownStairs(void);

	CDbRoom *	pRoom;
	CDbLevel *	pLevel;
	CDbHold *	pHold;

	//Player state
	CSwordsman	swordsman;

	//Game state vars
	bool		bIsDemoRecording;
	bool		bIsGameActive;
	UINT		wTurnNo;
	UINT		wSpawnCycleCount;
	UINT		wMonsterKills;	//total monsters killed in current room
	bool		bOnCheckpoint;

private:
	void		AddQuestionsToList(CCueEvents &CueEvents, 
			list<CMonsterMessage> &QuestionList) const;
	void		ClearRoomLists(void);
	bool		LoadEastRoom(CCueEvents &CueEvents);
	bool		LoadNorthRoom(CCueEvents &CueEvents);
	bool		LoadSouthRoom(CCueEvents &CueEvents);
	bool		LoadWestRoom(CCueEvents &CueEvents);
	bool		MonsterWithMovementTypeExists(const MovementType eMovement) const;
	bool		PlayCommands(UINT wCommandCount, CCueEvents &CueEvents);
	void		ProcessMimicPlacement(int nCommand, CCueEvents &CueEvents);
	void		ProcessMonsters(int nLastCommand, CCueEvents &CueEvents);
	void		ProcessSimultaneousSwordHits(CCueEvents &CueEvents);
	void		ProcessSwordsman(const int nCommand, CCueEvents &CueEvents);
	void		ProcessSwordsman_HandleLeaveLevel(CCueEvents &CueEvents);
	bool		ProcessSwordsman_HandleLeaveRoom(const UINT wMoveO,
			CCueEvents &CueEvents);
	void		ProcessUnansweredQuestions(int nCommand, 
			list<CMonsterMessage> &UnansweredQuestions, CCueEvents &CueEvents);
	void		SetMembersAfterRoomLoad(CCueEvents &CueEvents);
	void		SetSwordsmanMood(CCueEvents &CueEvents);
	void		SetSwordsmanToRoomStart(void);
	bool		SetRoomAtCoords(const DWORD dwRoomX, const DWORD dwRoomY);
	void		SetRoomStartToSwordsman(void);
	bool		WasRoomConqueredOnThisVisit(void) const;
	DWORD		WriteCurrentRoomDemo(DEMO_REC_INFO &dri, bool bHidden=false);
	DWORD		WriteCurrentRoomDieDemo(void);
	DWORD		WriteCurrentRoomConquerDemo(void);
	DWORD		WriteCurrentRoomHighlightDemo(void);

	void		CalcPathMaps(void);
	void		CreatePathMaps(void);
	void		SetPathMapsTarget(const UINT wX, const UINT wY);

	bool		IsSwordsmanTired(void);

	CCoordStack simulSwordHits;	//vulnerable tar hit simultaneously

	//"swordsman exhausted/relieved" event logic
	unsigned char *	pbMonstersKilled;	//rolling sum of monsters killed in recent turns
	UINT		wMonstersKilledRecently;
	bool		bLotsOfMonstersKilled;

	DEMO_REC_INFO			DemoRecInfo;
	list<CMonsterMessage>	UnansweredQuestions;
	CIDList					HighlightRoomIDs;
	bool					bIsNewRoom;

	DWORD		dwLastCheckpointSavedGameID;
	DWORD		dwAutoSaveOptions;

	PREVENT_DEFAULT_COPY(CCurrentGame);
};

#endif //...#ifndef CURRENTGAME_H

// $Log: CurrentGame.h,v $
// Revision 1.1  2003/02/25 00:01:19  erikh2000
// Initial check-in.
//
// Revision 1.71  2003/02/12 19:58:08  mrimer
// Fixed assertion on leaving room diagonally.
//
// Revision 1.70  2003/01/08 00:43:28  mrimer
// Minor changes.
//
// Revision 1.69  2002/12/22 01:43:20  mrimer
// Refactored player vars and methods into CSwordsman.
// Removed SetLevelToDefaultNext().
// Made some parameters const.
//
// Revision 1.68  2002/11/22 02:07:20  mrimer
// Added LoadFromRoom().  Made some vars const.  Cleaned up old code.  Revised calls to CPathMap.
//
// Revision 1.67  2002/11/13 22:09:33  mrimer
// Moved member variable initialization into the constructors' initialization list.
// Made some parameters const.
//
// Revision 1.66  2002/10/21 20:13:43  mrimer
// Added bOnCheckpoint.  Checkpoints are now saved only when stepping onto a checkpoint (not standing on it).
//
// Revision 1.65  2002/10/17 16:46:57  mrimer
// Added wMonsterKills.
//
// Revision 1.64  2002/10/09 00:54:23  erikh2000
// Reaaranged some code for readability.
// Conquer and highlight demos may now be saved when swordsman exits a level.
//
// Revision 1.63  2002/10/03 19:00:26  mrimer
// Added SwordsmanIsDying().
//
// Revision 1.62  2002/09/06 20:06:24  erikh2000
// Rerenamed "CtagHighlightDemoInfo" to "CHighlightDemoInfo".
//
// Revision 1.61  2002/09/01 00:04:08  erikh2000
// Split wTurnsTaken into wSpawnCycleCount and wTurnNo.  wSpawnCycleCount is incremented less often than wTurnNo, and is used for determining spawning related events.  wTurnNo is used for everything else.
//
// Revision 1.60  2002/08/28 20:27:00  mrimer
// Added SwordsmanTired event logic.
//
// Revision 1.59  2002/07/19 20:23:15  mrimer
// Added CAttachableObject references.
//
// Revision 1.58  2002/07/10 03:55:35  erikh2000
// Moved room exiting logic into a separate function called by ProcessSwordsman().
// Room criteria for highlight demos can be set.
// Highlight demos are automatically saved.
//
// Revision 1.57  2002/07/05 17:50:40  mrimer
// Minor fixes.
//
// Revision 1.56  2002/07/03 21:58:49  mrimer
// Revised SetRoomStatusFromAllSavedGames() to show conquered rooms based on current room.
//
// Revision 1.55  2002/06/23 10:46:37  erikh2000
// Added method to determine if game is at start of a level.
//
// Revision 1.54  2002/06/21 22:46:29  mrimer
// Added MonsterWithMovementTypeExists() to determine which pathmaps need to be created.
//
// Revision 1.53  2002/06/21 04:16:21  mrimer
// Added multiple pathmap support.
//
// Revision 1.52  2002/06/20 04:03:25  erikh2000
// Changed methods to return wstrings instead of using WCHAR arrays.
//
// Revision 1.51  2002/06/16 22:02:46  erikh2000
// Added automatic demo-saving for player deaths and conquered rooms.
//
// Revision 1.50  2002/06/09 06:02:10  erikh2000
// Added new auto-save flags.
//
// Revision 1.49  2002/06/05 23:58:34  mrimer
// Added an #include.
//
// Revision 1.48  2002/05/21 19:03:45  erikh2000
// Made changes related to demo recording.
//
// Revision 1.47  2002/05/15 01:16:31  erikh2000
// Made changes related to recording demos.
//
// Revision 1.46  2002/05/14 22:05:04  mrimer
// Wrote part of animating the swordsman walking down the stairs.
//
// Revision 1.45  2002/05/14 17:22:51  mrimer
// Now basing particle explosion direction on sword movement (not orientation).
// Changed tar babies' stab appearance to TarStabEffect.
//
// Revision 1.44  2002/05/12 03:08:09  erikh2000
// Made changes in how automatic games are saved.  It is now possible to disable all or some of the automatic saving behaviour.
//
// Revision 1.43  2002/04/28 23:44:30  erikh2000
// Added SetRoomStatusFromAllSavedGames().
//
// Revision 1.42  2002/04/12 21:48:27  mrimer
// Added CID_SwordsmanAfraid and CID_SwordsmanAggressive events
// and code to calculate when they occur.
// Added simultaneous tar stabbings.
//
// Revision 1.41  2002/04/12 05:09:44  erikh2000
// Added new CMoveCoord class.
//
// Revision 1.40  2002/04/09 01:01:04  erikh2000
// Changed OLECHAR declarations to WCHAR for consistency.
//
// Revision 1.39  2002/03/30 21:51:22  erikh2000
// Changed declaration of dwLastCheckPointSavedGameID to DWORD.
//
// Revision 1.38  2002/03/30 05:10:34  erikh2000
// Wrote RestartRoomFromLastCheckpoint().  (Committed on behalf of mrimer.)
//
// Revision 1.37  2002/03/17 23:04:36  erikh2000
// Added RestartRoomFromLastCheckpoint() stub.
//
// Revision 1.36  2002/03/16 12:20:47  erikh2000
// Changes made for GetLevelPosition() to work.  (Committed on behalf of mrimer.)
//
// Revision 1.35  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.34  2002/02/28 04:51:04  erikh2000
// Added SaveToRoomBegin(), SaveToLevelBegin(), and SaveToCheckpoint().
// Removed Save().
//
// Revision 1.33  2002/02/26 11:47:11  erikh2000
// Added dwFirstDemoID member to DemoRecInfo struct.
//
// Revision 1.32  2002/02/25 03:37:58  erikh2000
// Added UndoCommand() and changed UndoCommands() to return cue events.
//
// Revision 1.31  2002/02/24 03:44:07  erikh2000
// Added ProcessSwordHit() method.
//
// Revision 1.30  2002/02/24 01:24:45  erikh2000
// Added CueEvents parameter to several CCurrentGame methods related to room loading.  Whenever a room is loaded in CCurrentGame, the swordsman is taking a step into the room that can cause things to happen.
//
// Revision 1.29  2002/02/23 04:59:21  erikh2000
// Added FreezeCommands() and UnfreezeCommands() methods.
// Removed bAddCommands parameter from ProcessCommand() because Freeze/UnfreezeCommands() provide better solution.
// Changed PlayCommands() to return success or failure.
//
// Revision 1.28  2002/02/15 02:46:27  erikh2000
// Added option to restore from beginning of room to CDb::LoadFromSavedGame().
//
// Revision 1.27  2002/02/13 01:05:26  erikh2000
// Changed CCurrentGame::EndDemoRecording() to return DemoID of new demo.
//
// Revision 1.26  2002/02/10 02:34:15  erikh2000
// Added CCurrentGame::PlayCommands().
// Added CCurrentGame::SaveToContinue().
// Moved CDbSavedGame::wTurnsTaken, wX, wY, wO, bIsPlacingMimic, wMimicCursorX, wMimicCursorY, and bIsVisible to CCurrentGame.
//
// Revision 1.25  2002/02/09 00:58:16  erikh2000
// Added comments about adding methods to CDbRoom vs CCurrentGame.
// Moved md5i's CCurrentGame::StabTar() to CDbRoom::StabTar().
// Moved md5i's CCurrentGame::GrowTar() to CDbRoom::GrowTar().
//
// Revision 1.24  2002/02/07 22:31:09  erikh2000
// Removed CCurrentGame::bIsFirstTurn member and references to it.
//
// Revision 1.23  2001/12/16 02:12:53  erikh2000
// Added processing for monster questions.
// Added CCurrentGame::GetChecksum().
//
// Revision 1.22  2001/12/08 01:39:42  erikh2000
// Added PREVENT_DEFAULT_COPY() macro to class declaration.
//
// Revision 1.21  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.20  2001/11/23 01:07:53  erikh2000
// Added CCurrentGame::UndoCommands() stub.
//
// Revision 1.19  2001/11/20 00:51:53  erikh2000
// Added a bunch of demo-related stuff that doesn't work yet.  This is an early check-in to avoid later merging conflicts.
//
// Revision 1.18  2001/11/17 23:10:50  erikh2000
// Added members to track location at which swordsman was previously.  Needed for mimic processing.  (Committed on behalf of j_wicks.)
//
// Revision 1.17  2001/11/13 05:35:54  md5i
// Added TarMother and growing tar.
//
// Revision 1.16  2001/11/12 03:22:02  erikh2000
// Added StabTar() stub.
//
// Revision 1.15  2001/11/08 11:31:57  erikh2000
// Removed bIsPlacingMimic member.  (Committed on behalf of jpburford.)
//
// Revision 1.14  2001/11/05 04:57:40  erikh2000
// Changed level exit code to return list of level IDs instead of level pointer.
//
// Revision 1.13  2001/11/03 20:16:19  erikh2000
// Removed OnPlot() and OnLoad() references.  Added code that creates CID_Plots cue event.  Renamed bIsSwordsmanDead member to "bIsGameActive" so that it can be used more generally.
//
// Revision 1.12  2001/10/30 02:04:30  erikh2000
// Added clearing of conquered/explored room lists to Clear() method.  (Committed on behalf of timeracer.)
//
// Revision 1.11  2001/10/27 20:25:33  erikh2000
// Added room jump commands.
// Fixed bug where swordsman is sometimes not killed when more than one monster is in the room.
//
// Revision 1.10  2001/10/27 04:42:35  erikh2000
// Added SetOnLoad() and SetOnPlot() methods.
// Replaced room ID-checking code for explored/conquered rooms and level completion with calls to class methods that do same.
//
// Revision 1.9  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.8  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.7  2001/10/24 01:44:33  md5i
// Add turn counter.
//
// Revision 1.6  2001/10/22 23:55:44  erikh2000
// Revised level-loading methods.
//
// Revision 1.5  2001/10/21 00:27:37  erikh2000
// Fixed bugs preventing monsters from being processed.
// Monsters can now kill the swordsman.
// Wrote Save() method that just calls CDbSavedGame::Update() stub.
// Now keeps track of which rooms have been explored or conquered, and handles effects of.
// ProcessSwordsman() now sends back cue events for stepping on/off scrolls.
//
// Revision 1.4  2001/10/20 20:05:46  erikh2000
// Updated ProcessSwordsman() to stab monsters, destroy trapdoors, and strike orbs.
// Moved most of the room-exiting logic into LoadNewRoomForExit() to improve readability of ProcessSwordsman().
//
// Revision 1.3  2001/10/16 00:02:31  erikh2000
// Wrote code to display current room and handle basic swordsman movement.
//
// Revision 1.2  2001/10/07 04:38:34  erikh2000
// General cleanup--finished removing game logic code from DROD.EXE.
// Added ProcessCommand() method to CCurrentGame to be used as interface between UI and game logic.
//
// Revision 1.1.1.1  2001/10/01 22:20:07  erikh2000
// Initial check-in.
//
