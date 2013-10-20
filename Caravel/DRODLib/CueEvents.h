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
 * Michael Welsh Duggan (md5i), JP Burford (jpburford), John Wm. Wicks (j_wicks),
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//CCueEvents class declaration, cue event IDs, and classification macros.

//GENERAL
//
//Cue events are messages returned to the caller of CCurrentGame::ProcessCommand().  
//Typically, these messages would correspond to game events that need a sound
//or graphical effect that the caller should handle.  The ProcessCommand caller is 
//not required to handle any of the cue events.  Since CCurrentGame does nothing 
//UI-related it is possible to run games "silently" without any UI.
//
//Each cue event may have one or more private data pointers (void *pvPrivateData) 
//associated with it.  Some cue events will use this to point to data associated with 
//the cue event.  You will know by the cue event ID which class or data type is 
//pointed to, and can cast to that associated type for access to the private data.  
//The private data pointer should not be deleted.
//
//USAGE
//
//Call Add() to say that an event occurred and optionally associate private data that
//may be used later to react to that event.  For example when a monster is killed,
//a CMonster pointer is associated with the CID_MonsterDiedFromStab event.  If three 
//monsters were killed on a turn, then three CMonster pointers will be available for 
//retrieval.
//
//To simply check whether events occurred, use HasOccurred() or HasAnyOccurred().  The
//IDCOUNT() macro defined in IDList.h should be used for calls to HasAnyOccurred().
//
//To retrieve private data, call GetFirstPrivateData(), and follow up with calls to 
//GetNextPrivateData() if appropriate.  GetFirstPrivateData() will cause 
//GetNextPrivateData() to return the second private data associated with the specified 
//cue event if available.  GetFirstPrivateData() should also be used in lieu of 
//HasOccurred() to check for a cue event when private data may be retrieved.  If you 
//just want to check an ID without retrieving private data, then call HasOccurred() or 
//HasAnyOccurred().
//
//Never delete private data pointers retrieved from cue events.  Deletion will be
//handled in the CueEvents destructor or in CCurrentGame depending on the design
//of the cue event.
//
//DESIGN CONSIDERATIONS FOR CREATING NEW CUE EVENTS
//
//Cue events are mainly for description of UI-related events, but there are other
//tasks where they can be put to good use, for example:
//
//When a monster gets killed by sword stab, CMonster::OnStabbed() passes back a 
//CID_MonsterDiedFromStab cue event, and CCurrentGame::ProcessSwordsman() handles 
//the cue event with a call that deletes the CMonster instance which had its 
//OnStabbed() member called.  It would be possible to execute "delete this" within
//OnStabbed(), but to avoid problems with bad pointers, the deletion is handled 
//outside of CMonster code using the cue event.
//
//The tar mother is another example.  She causes tar in a room to grow every 30 turns.
//If there are multiple tar mothers in the room, the tar should grow just once--not for 
//each tar mother processed.  By checking for a CID_TarGrew cue event after all of the
//tar mothers have been processed, we can make the tar grow just once if there are any
//tar mothers in the room and 30 turns have passed.  If the tar growing was performed
//up in CTarMother::Process() there would need to be some kind of state-checking 
//between the tar mothers to ensure the tar grew one time only.
//
//There may be other valid uses of cue events.  Here are some other guidelines for
//making cue events.
//
//1. Avoid creating cue events that don't correspond to actions which the 
//ProcessCommand() caller will handle.  The two cases described above are 
//valid exceptions.  Typically, you can handle a purely game-logic event in the code 
//that determines the event should occur.  For example, in ProcessSwordsman() you 
//can detect that the swordsman has destroyed the last trapdoor and make a call to 
//RemoveRedDoors() from that routine.  This is preferable to passing a 
//CID_RemoveRedDoors (fictitious) cue event back to ProcessCommand() or to its 
//caller for handling the removal of red doors.  A local handling of an event is 
//usually clearer to other developers and requires less time to process during 
//execution.
//
//2. Wait for a UI feature to be planned before adding a cue event that may or may 
//not be handled by the ProcessCommand() caller in the future.  It's easy to add 
//cue events later, and we don't want to have the clutter and inefficiency of unused 
//cue events.
//
//PRIVATE DATA VALIDITY GUARANTEE
//
//Private data pointers returned by ProcessCommand() are guaranteed to be valid 
//until the CCurrentGame instance that originally returned them goes out
//of scope or the ProcessCommand() method is called again on the same instance.
//
//Conversely, when creating new cue events that have private data, you should design 
//so that the above guarantee is honored.  If your private data object is only used
//to send data to the ProcessCommand caller, then probably you should set the 
//bIsAttached parameter to true in your call to CCueEvents::Add().  Otherwise, the
//bIsAttached parameter should be false, and you must make sure that the pointer
//will be good at least until the next call to ProcessCommand().  This should usually 
//be accomplished by using a member variable of CCurrentGame to hold the data to 
//which you will return a pointer.

#ifndef CUEEVENTS_H
#define CUEEVENTS_H

#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>

#include <memory.h>

//
//Cue event IDs.
//

enum CUEEVENT_ID
{
	//A monster killed the swordsman.  
	//
	//Private data: CMonster *pMonsterThatKilledPlayer (one)
	CID_MonsterKilledPlayer = 0,

	//A serpent died when it got too short to live.  
	//
	//Private data: CSerpent *pSerpentThatDied (one or more)
	CID_SnakeDiedFromTruncation,

	//A monster died from a stab--could be from the swordsman or a mimic.  
	//
	//Private data: CMonster *pMonsterThatDied (one or more)
	CID_MonsterDiedFromStab,

	//The swordsman exited the room by walking across one of its four borders.  The
	//new room is loaded now.  Contrast to CID_ExitRoomPending which indicates that
	//the room has not yet been loaded.
	//
	//Private data: UINT *pwExitOrientation (one)
	CID_ExitRoom,

	//The swordsman has exited the level, but the new level has not been loaded yet.
	//The next level ID is attached, for the ProcessCommand() caller to
	//initiate the level load in response.
	//
	//Private data: DWORD *pdwNextLevelID (one)
	CID_ExitLevelPending,

	//Player has won the game.  The current room/level remains loaded.  The 
	//ProcessCommand() caller may choose to transition to a win screen.
	//
	//Private data: NONE
	CID_WinGame,

	//The room has been conquered.  This will occur when the player has exited a
	//room cleared of monsters, or when the player enters an unvisited room that contains
	//no monsters.
	//
	//Private data: NONE
	CID_ConquerRoom,

	//Enough rooms in level have been conquered for the level to be considered "complete".
	//In the level-complete state, blue doors will be open.
	//
	//Private data: NONE
	CID_CompleteLevel,

	//The swordsman stepped onto and is currently in a square containing a scroll.
	//
	//Private data: WCHAR *pwczScrollText (one)
	CID_StepOnScroll,

	//A roach egg has hatched.  This event is used by ProcessMonsters() to delete
	//the CRoachEgg monster.  A new CRoach monster will be present in the same
	//square.
	//
	//Private data: CRoachEgg *pEggThatHatched (one or more)
	CID_EggHatched,

	//If any calls to CDbRoom::Plot() were made in the current room, this event will
	//fire.  Useful for making efficient updates to display structures.
	//
	//Private data: NONE
	CID_Plots,

	//The swordsman or a mimic stepped off a trapdoor square and that square now
	//contains a pit.
	//
	//Private data: CCoord *pSquareRemovedAt (one or more)
	CID_TrapDoorRemoved,

	//The swordsman or a mimic stabbed a square of tar and destroyed it.
	//
	//Private data: CCoord *pSquareDestroyedAt (one or more)
	CID_TarDestroyed,

	//One or more tar mothers are present in the room and caused the tar to grow at
	//the tar growth interval.
	//
	//Private data: NONE
	CID_TarGrew,

	//A monster sent a message to the player.  It could be a simple statement for
	//the ProcessCommand() caller to display.  Or it could be a question that the
	//player must answer.  In the second case, ProcessCommand(), on the next call, will 
	//be expecting a an appropriate command constant that indicates the player's answer.
	//It is possible for multiple messages to be attached to this event.
	//
	//Private data: CMonsterMessage *pMonsterMessage (one or more)
	CID_MonsterSpoke,

	//The 'Neather has exited the room.  Used by ProcessMonsters() to delete the
	//CNeather monster.
	//
	//Private data: NONE
	CID_NeatherExitsRoom,

	//The swordsman exited the room by walking across one of its four borders.  The
	//new room is not loaded now.  The ProcessCommand() caller may load the new room
	//with a call to LoadNewRoomForExit().  Contrast to CID_ExitRoom which indicates that
	//the room has been loaded.
	//
	//Private data: UINT *pwExitOrientation (one)
	CID_ExitRoomPending,

	//The swordsman or a mimic stabbed a crumbly wall square and destroyed it.
	//
	//Private data: CCoord *pSquareDestroyedAt (one or more)
	CID_CrumblyWallDestroyed,

	//The swordsman hit an orb.
	//
	//Private data: COrbData *pOrbData (none, one or more)
	CID_OrbActivated,

	//A mimic hit an orb.
	//
	//Private data: COrbData *pOrbData (none, one or more)
	CID_OrbActivatedByMimic,

	//All the monsters in the room were killed.
	//
	//Private data: NONE
	CID_AllMonstersKilled,

	//Swordsman drank (mimic/invisibility) potion.
	//
	//Private data: NONE
	CID_DrankPotion,

	//Swordsman placed mimic.
	//
	//Private data: NONE
	CID_MimicPlaced,

	//Swordsman ran into obstacle.
	//
	//Private data: CMoveCoord *pObstacle (one)
	CID_HitObstacle,

	//Swordsman swung sword in new direction.
	//
	//Private data: NONE
	CID_SwingSword,

	//Swordsman got scared (almost stepped into pit).
	//
	//Private data: NONE
	CID_Scared,

	//Swordsman took a step.
	//
	//Private data: NONE
	CID_Step,

	//Swordsman afraid (monster has upper hand).
	//
	//Private data: NONE
	CID_SwordsmanAfraid,

	//Swordsman agressive (has upper hand on monster).
	//
	//Private data: NONE
	CID_SwordsmanAggressive,

	//Swordsman normal (not close to monsters).
	//
	//Private data: NONE
	CID_SwordsmanNormal,

	//A highlight demo was saved for a room.  The player entered the room in an unconquered
	//state and left the room conquered.
	//
	//Private data: HIGHLIGHT_DEMO_INFO *pHighlightDemoInfo (one)
	CID_HighlightDemoSaved,

	//An evil eye woke up.
	//
	//Private data: CEvilEye *pEvilEye (one or more)
	CID_EvilEyeWoke,

	//A tar baby was formed.
	//
	//Private data: CTarBaby *pTarBaby (one or more)
	CID_TarBabyFormed,

	//The swordsman stepped on a checkpoint.
	//
	//Private data: CCoord *pSquare (one)
	CID_CheckpointActivated,

	//The 'Neather has just pulled off a particularly nasty move,
	//like trapping the player or releasing a horde of goblins on him.
	//
	//Private data: NONE
	CID_NeatherLaughing,

	//The 'Neather's best laid plans have failed.
	//
	//Private data: NONE
	CID_NeatherFrustrated,

	//Beethro has gotten a little too close to the 'Neather.
	//
	//Private data: NONE
	CID_NeatherScared,

	//The player has just finished killing a horde of monsters.
	//
	//Private data: NONE
	CID_SwordsmanTired,

	//Green doors opened.
	//
	//Private data: NONE
	CID_GreenDoorsOpened,

	//Red doors opened.
	//
	//Private data: NONE
	CID_RedDoorsOpened,

	//End of enumeration typedef.
	CUEEVENT_COUNT
};

#define IS_VALID_CID(cid)	((UINT)(cid) < CUEEVENT_COUNT)

//
//Cue event comparison ID arrays.
//

//Has player left room or will he shortly be leaving?  Another way of looking at: are
//any more commands going to be processed in the current room.  
const CUEEVENT_ID CIDA_PlayerLeftRoom[] = {CID_ExitRoomPending, CID_ExitRoom, 
		CID_ExitLevelPending, CID_WinGame, CID_MonsterKilledPlayer};

//Did something kill the player?  Currently only one event in the array, but the
//array should be checked instead of the one event, so that future causes of death
//will be checked without project-wide changes to code.
const CUEEVENT_ID CIDA_PlayerDied[] = {CID_MonsterKilledPlayer};

//Did a monster die?
const CUEEVENT_ID CIDA_MonsterDied[] = {CID_SnakeDiedFromTruncation,
      CID_MonsterDiedFromStab, CID_NeatherExitsRoom};

class CAttachableObject;

//
//Linked list node for storing private data.
typedef struct tagCIDPrivDataNode CID_PRIVDATA_NODE;
typedef struct tagCIDPrivDataNode
{
	CAttachableObject *pvPrivateData;
	bool bIsAttached;
	CID_PRIVDATA_NODE *pNext;
} CID_PRIVDATA_NODE;

//******************************************************************************************
class CCueEvents
{
public:
	CCueEvents(void) {Zero();}
	~CCueEvents(void) {Clear();}
	
	void		Add(CUEEVENT_ID eCID, CAttachableObject *pvPrivateData = NULL, bool bIsAttached=false);
	void		Clear(void);
	CAttachableObject *		GetFirstPrivateData(CUEEVENT_ID eCID);
	CAttachableObject *		GetNextPrivateData(void);
	UINT		GetEventCount(void) const {return this->wEventCount;}
	UINT		GetOccurrenceCount(CUEEVENT_ID eCID) const;
	bool		HasOccurred(CUEEVENT_ID eCID) const {ASSERT(IS_VALID_CID(eCID)); return this->barrIsCIDSet[eCID];}
	bool		HasOccurredWith(CUEEVENT_ID eCID, const CAttachableObject *pvPrivateData) const;
	bool		HasAnyOccurred(UINT wCIDArrayCount, const CUEEVENT_ID *peCIDArray) const;

protected:
	CID_PRIVDATA_NODE *	pNextPrivateData;

	bool				barrIsCIDSet[CUEEVENT_COUNT];
	CID_PRIVDATA_NODE *	parrCIDPrivateData[CUEEVENT_COUNT];

	UINT		wEventCount;

private:
	void		Zero(void);

	PREVENT_DEFAULT_COPY(CCueEvents);
};

#endif //...#ifndef CUEEVENTS_H

// $Log: CueEvents.h,v $
// Revision 1.40  2003/09/16 19:34:17  mrimer
// Fixed bug: not checking for monster removal when checking for room cleaned event.
//
// Revision 1.39  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.38  2003/05/22 23:39:00  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.37  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.36  2002/12/22 01:45:00  mrimer
// Added exit orientation to CID_ExitRoom.
// Changed CID_ExitLevelPending to take only one level ID.
// CID_OrbActivated (and *ByMimic) can now accept zero orb objects.
//
// Revision 1.35  2002/11/14 19:01:22  mrimer
// Removed private data from CID_Plots.
// Added CID_OrbActivatedByMimic.
//
// Revision 1.34  2002/10/16 01:27:43  erikh2000
// Removed CID_StepOffScroll.
// Put CIDs in an enum and changed params for CCueEvents to match.
//
// Revision 1.33  2002/08/28 21:35:43  erikh2000
// Added CIDs for green and red doors opening.
//
// Revision 1.32  2002/08/28 20:26:21  mrimer
// Added events (Neather* and SwordsmanTired).
//
// Revision 1.31  2002/07/25 18:50:50  mrimer
// Added CID_CheckpointActivated event.
//
// Revision 1.30  2002/07/22 00:51:57  erikh2000
// Removed dangerous object-zeroing memset call.
//
// Revision 1.29  2002/07/21 00:18:06  erikh2000
// Added contributor credits for mrimer.
//
// Revision 1.28  2002/07/19 20:23:15  mrimer
// Added CAttachableObject references.
//
// Revision 1.27  2002/07/17 02:12:06  erikh2000
// Added new method to get number of occurrences of an event.
// Added new CIDs.
//
// Revision 1.26  2002/07/10 03:53:57  erikh2000
// Added new cue event for highlight demo.
//
// Revision 1.25  2002/07/05 17:50:40  mrimer
// Minor fixes.
//
// Revision 1.24  2002/06/05 23:58:34  mrimer
// Added an #include.
//
// Revision 1.23  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.22  2002/04/20 08:18:51  erikh2000
// Changed comments for CID_StepOnScroll to indicate new private data.
//
// Revision 1.21  2002/04/18 17:45:24  mrimer
// Added new cue event constant: CID_SwordsmanNormal.
//
// Revision 1.20  2002/04/12 21:48:26  mrimer
// Added CID_SwordsmanAfraid and CID_SwordsmanAggressive events
// and code to calculate when they occur.
// Added simultaneous tar stabbings.
//
// Revision 1.19  2002/04/12 05:12:33  erikh2000
// Changed comments for cue event for swordsman hitting obstacle.
//
// Revision 1.18  2002/03/16 11:44:26  erikh2000
// Added new cue event constants: CID_AllMonstersKilled, CID_DrankPotion, CID_MimicPlaced, CID_HitObstacle, CID_SwingSword, CID_Scared, and CID_Step.  (Committed on behalf of mrimer.)
//
// Revision 1.17  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.16  2002/02/24 02:03:00  erikh2000
// Added CID_OrbActivated and CID_CrumblyWallDestroyed cue events.
//
// Revision 1.15  2002/02/23 05:00:06  erikh2000
// Added comments for all the cue events.
// Added CID_ExitRoomPending.
// Renamed CID_ExitLevel to "CID_ExitLevelPending" for consistency.
// Updated CIDA consts related to room and level exiting.
//
// Revision 1.14  2002/02/07 23:29:14  erikh2000
// Added CID_NeatherExitsRoom cue event.  (Committed on behalf of j_wicks.)
//
// Revision 1.13  2001/12/16 02:12:21  erikh2000
// Added CID_MonsterSpoke cue event.
//
// Revision 1.12  2001/12/08 01:39:42  erikh2000
// Added PREVENT_DEFAULT_COPY() macro to class declaration.
//
// Revision 1.11  2001/11/25 02:28:36  erikh2000
// Added CCueEvents class.
//
// Revision 1.10  2001/11/19 09:22:10  erikh2000
// Removed CID_MimicKilledPlayer since CID_MonsterKilledPlayer covers it.
//
// Revision 1.9  2001/11/13 05:35:54  md5i
// Added TarMother and growing tar.
//
// Revision 1.8  2001/11/12 03:21:37  erikh2000
// Added CID_TarDestroyed.
//
// Revision 1.7  2001/11/06 08:47:06  erikh2000
// Added CID_TrapDoorRemoved cue event.  (Committed on behalf of jpburford.)
//
// Revision 1.6  2001/11/05 23:14:23  erikh2000
// Added additional comments on usage.
//
// Revision 1.5  2001/11/03 20:15:47  erikh2000
// Added CID_Plots cue event.
//
// Revision 1.4  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.3  2001/10/21 00:33:04  erikh2000
// Added new cue events for conquering room, completing level, stepping on or off a scroll.
//
// Revision 1.2  2001/10/07 04:38:34  erikh2000
// General cleanup--finished removing game logic code from DROD.EXE.
// Added ProcessCommand() method to CCurrentGame to be used as interface between UI and game logic.
//
// Revision 1.1.1.1  2001/10/01 22:20:06  erikh2000
// Initial check-in.
//
