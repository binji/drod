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
 * John Wm. Wicks (j_wicks)
 *
 * ***** END LICENSE BLOCK ***** */

//Neather.h
//Declarations for Neather.
//Class for handling neather monster game logic.

#ifndef NEATHER_H
#define NEATHER_H

#include "Monster.h"
#include "MonsterFactory.h"

typedef enum tagNeatherState
// The following convention was used in the naming of the enumerations
// rx = State consistent in all rooms
// r# = Room in which the state is being used
// Duplicate enumeration values were used because of the previous reuse of
// various states in different contexts within different rooms
// Where possible the rx enum designation was used for consistent states that
// appear in several rooms.
{
	rx_stAsterix=42,
	r7_stMovingLock3=42,
	rx_stZero=48,
	rx_stOne=49,
	r3_stClosingNWPassage=49,
	r4_stGuardingRoomC=49,
	r5_SettingCentralTrapStep1=49,
	r6_stMovingControlArea=49,
	r7_stGuardingLock3=49,
	rx_stTwo=50,
	r3_stMovingNWControlArea=50,
	r4_stGuardingRoomD=50,
	r5_SettingCentralTrapStep2=50,
	r6_stEnteringControlCenter=50,
	r7_stGuardingLock4=50,
	rx_stThree=51,
	r3_stReleasingRoaches=51,
	r4_stGuardingRoomE=51,
	r5_SettingCentralTrapStep3=51,
	r6_GuardingControlCenter=51,
	r7_stMovingLock1=51,
	rx_stFour=52,
	r3_stOpening3DoorAreaToRoach=52,
	r5_SettingCentralTrapStep4=52,
	r6_PreparingEscapeRoute=52,
	r7_stCheckingPathStatus=52,
	rx_stFive=53,
	r5_SpringingTrap=53,
	r6_stExitingControlCenter=53,
	r7_stGuardingLock1=53,
	rx_stSix=54,
	r4_GuardingNWFork=54,
	r5_stGuardingRoomA=54,
	r6_stMovingNorthPassage=54,
	r7_stMovingLock2=54,
	rx_stSeven=55,
	r5_stGuardingSouthPassage=55,
	r7_stGuardingLock2=55,
	rx_stEight=56,
	r4_stGuardingOrbPit=56,
	r5_stSpringingSouthPassageTrap=56,
	r7_stSearchingPaths=56,
	rx_stNine=57,
	r4_stLeavingOrbPit=57,
	r5_stGuardingEastDoorRoomA=57,
	rx_stA=65,					rx_stB=66,						rx_stC=67,
	rx_stD=68,					rx_stE=69,						rx_stF=70,
	rx_stG=71,					rx_stH=72,						rx_stI=73,
	rx_stJ=74,					rx_stK=75,						rx_stL=76,
	rx_stM=77,					rx_stN=78,						rx_stO=79,
	rx_stP=80,					rx_stQ=81,						rx_stR=82,
	rx_stS=83,					rx_stT=84,						rx_stU=85,
	rx_stV=86,					rx_stW=87,						rx_stX=88,
	rx_stY=89,					rx_stZ=90,
	rx_sta=97,
	r4_stReturningToNWFork=97,
	r5_stEscapingRoomA=97,
	r7_stMovingPath1Step1=97,
	rx_stb=98,
	r4_stFindingOpenPassage=98,
	r5_RetreatingEastSideCentralRoom=98,
	rx_stc=99,
	rx_stMovingSWCorner=99,
	r5_stExitingCentralRoom=99,
	rx_std=100,
	rx_stMovingSECorner=100,
	r5_stEscapingViaTrapDoor=100,
	rx_ste=101,
	rx_stMovingNNECorner=101,
	r5_stFindingHideOut=101,
	rx_stf=102,
	rx_stFleeing=102,
	rx_stg=103,
	r4_stOpeningDoorsForSwordsman=103,
	r5_EscapingHideOut=103,
	rx_sth=104,
	rx_stGuardingLastEntrance=104,
	r5_stEscapingFinalDoor=104,
	rx_sti=105,
	rx_stClosingLastEntrance=105,
	r5_stHidingInCloset=105,
	rx_stj=106,
	rx_stLeavingRoom=106,
	rx_stk=107,
	rx_stl=108,
	rx_stm=109,
	rx_stn=110,
	rx_sto=111,
	rx_stp=112,
	rx_stq=113,
	rx_str=114,
	rx_sts=115,
	rx_stt=116,
	r2_stSpringingTrap=116,
	rx_stu=117,
	rx_stv=118,
	rx_stw=119,
	rx_stWaiting=119,
	rx_stx=120,
	r2_stMovingSWPassage=120,
	rx_stGuardingFirstDoors=120,
	rx_sty=121,
	rx_stz=122
} NEATHERSTATE;

typedef enum tagGoalType {wait=0, strikeorb=1, move=2, exitroom=3} GOALTYPE;
typedef struct tagGoal
{
	GOALTYPE eType;
	int nX, nY;
} GOAL;

class CNeather : public CMonster
{
public:
	CNeather(CCurrentGame *pSetCurrentGame = NULL) : CMonster(M_NEATHER,
		pSetCurrentGame, GROUND, 200)
		, bStrikingOrb(false), bLaughWhenOrbHit(false) { }
	IMPLEMENT_CLONE(CMonster, CNeather)

	virtual bool IsAggressive(void) {return false;}
	virtual bool OnAnswer(int nCommand, CCueEvents &CueEvents);
	virtual bool OnStabbed(CCueEvents &CueEvents);
	virtual void Process(const int nLastCommand, CCueEvents &CueEvents);
	virtual void SetMembersFromExtraVars(void) { this->m_CurrentState =
		(NEATHERSTATE)this->ExtraVars.GetVar("State", (int)0); }
	virtual void SetExtraVarsFromMembers(void) {
		this->ExtraVars.SetVar("State", (int)(this->m_CurrentState)); }
	
	bool GetGoal(GOAL &pGoal, CCueEvents &CueEvents);
	bool SetGoal(const int nX, const int nY, const GOALTYPE eType, GOAL &pGoal,
			const bool bLaugh=false);
	bool GetGoalDirectedMovement(int &dx, int &dy, const GOAL &pGoal) const;

	bool bStrikingOrb;

private:
	bool GetRoom1Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom2Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom3Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom4Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom5Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom6Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom7Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom8Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom9Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom10Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom11Goal(GOAL &pGoal, CCueEvents &CueEvents);
	bool GetRoom12Goal(GOAL &pGoal, CCueEvents &CueEvents);

	bool SquareFree(const UINT sx, const UINT sy);

	GOAL m_CurrentGoal;
	NEATHERSTATE m_CurrentState;

	bool bLaughWhenOrbHit;	//set during call to SetGoal()
};

#endif //...#ifndef NEATHER_H

// $Log: Neather.h,v $
// Revision 1.1  2003/02/25 00:01:37  erikh2000
// Initial check-in.
//
// Revision 1.14  2002/12/22 01:23:13  mrimer
// Removed some parameters to GetGoalDirectedMovement().
// Revised swordsman vars.
//
// Revision 1.13  2002/11/14 19:25:04  mrimer
// Added object copy support.
// Made virtual methods explicit.
//
// Revision 1.12  2002/08/29 21:59:25  mrimer
// Changed Process' return type to void.
//
// Revision 1.11  2002/08/28 20:24:47  mrimer
// Added sound effect logic.
// Fixed a room 3 bug.
//
// Revision 1.10  2002/07/23 20:11:16  mrimer
// Tweaking.
//
// Revision 1.9  2002/07/22 17:32:29  mrimer
// Added bStrikingOrb.
//
// Revision 1.8  2002/07/16 18:16:58  mrimer
// Added SquareFree().
//
// Revision 1.7  2002/06/21 04:22:30  mrimer
// Added GROUND movement.
//
// Revision 1.6  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.5  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.4  2002/02/07 23:27:39  erikh2000
// Implemented CNeather.  (Committed on behalf of j_wicks.)
//
// Revision 1.3  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.2  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.1.1.1  2001/10/01 22:20:24  erikh2000
// Initial check-in.
//
