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
 * JP Burford (jpburford), John Wm. Wicks (j_wicks), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DbRooms.h
//Declarations for CDbRooms, CDbRoom and several small data-containment classes.
//Class for accessing room data from database.

#ifndef DBROOMS_H
#define DBROOMS_H

#include "DbVDInterface.h"
#include "DbDemos.h"
#include "DbSavedGames.h"
#include "Monster.h"
#include "Pathmap.h"
#include <BackEndLib/CoordIndex.h>
#include <BackEndLib/CoordStack.h>

#include <list>

//Orb agent types.
const int OA_NULL = 0;
const int OA_TOGGLE = 1;
const int OA_OPEN = 2;
const int OA_CLOSE = 3;

//******************************************************************************************
class COrbAgentData
{
public:
	COrbAgentData() : wX(0), wY(0), wAction(0)
	{ }
	COrbAgentData(const UINT wX, const UINT wY, const UINT wAction)
		: wX(wX), wY(wY), wAction(wAction) { }

	UINT			wX;
	UINT			wY;
	UINT			wAction;
};

//******************************************************************************************
class COrbData : public CAttachableObject
{
public:
	COrbData()
		: CAttachableObject()
      , wX(0), wY(0), wAgentCount(0)
		, parrAgents(NULL)
	{ }
	~COrbData()
	{
		delete [] parrAgents;
	}

	COrbData(const COrbData &Src) : CAttachableObject() {SetMembers(Src);}
	COrbData &operator= (const COrbData &Src) 
	{
		delete [] parrAgents;
		SetMembers(Src);
		return *this;
	}

	COrbAgentData* AddAgent(const UINT wX, const UINT wY, const UINT wAction)
	{
		//Add new agent.  Reallocate agent array.
		COrbAgentData *pNewAgents = new COrbAgentData[this->wAgentCount+1];
		COrbAgentData *pNewAgent = pNewAgents + this->wAgentCount;
		for (UINT wAgentI = 0; wAgentI < this->wAgentCount; wAgentI++)
			pNewAgents[wAgentI] = this->parrAgents[wAgentI];
		pNewAgent->wAction = wAction;
		pNewAgent->wX = wX;
		pNewAgent->wY = wY;
		++this->wAgentCount;

		delete[] this->parrAgents;
		this->parrAgents = pNewAgents;

		return pNewAgent;
	}

	void AddAgent(COrbAgentData *pAgent)
	{
		//Add new agent.  Reallocate agent array.
		COrbAgentData *pNewAgents = new COrbAgentData[this->wAgentCount+1];
		for (UINT wAgentI = 0; wAgentI < this->wAgentCount; wAgentI++)
			pNewAgents[wAgentI] = this->parrAgents[wAgentI];
		pNewAgents[this->wAgentCount] = *pAgent;
		++this->wAgentCount;

		delete[] this->parrAgents;
		this->parrAgents = pNewAgents;
	}

	bool DeleteAgent(COrbAgentData* const pOrbAgent)
	{
		ASSERT(pOrbAgent);
		UINT wAgentI;
		for (wAgentI=0; wAgentI < this->wAgentCount; wAgentI++)
			if (this->parrAgents+wAgentI == pOrbAgent)
			{
				break;	//Found it.
			}
		if (wAgentI == this->wAgentCount)
			return false;	//didn't find it

		//Reorganize agents array (move last one forward).
		this->parrAgents[wAgentI] = this->parrAgents[--this->wAgentCount];
		return true;
	}

	COrbAgentData* GetAgentAt(const UINT wX, const UINT wY) const
	{
		for (UINT wAgentI=0; wAgentI < this->wAgentCount; wAgentI++)
			if (this->parrAgents[wAgentI].wX == wX &&
					this->parrAgents[wAgentI].wY == wY)
				return this->parrAgents + wAgentI;

		return NULL;	//didn't find it
	}

	UINT			wX;
	UINT			wY;
	UINT			wAgentCount;
	COrbAgentData *	parrAgents;

private:
	void SetMembers(const COrbData &Src)
	{
		this->wX = Src.wX;
		this->wY = Src.wY;
		this->wAgentCount = Src.wAgentCount;
		this->parrAgents = new COrbAgentData[this->wAgentCount];
		for (UINT wAgentI = 0; wAgentI < this->wAgentCount; ++wAgentI)
		{
			this->parrAgents[wAgentI] = Src.parrAgents[wAgentI];
		}
	}
};

//******************************************************************************************
class CScrollData
{
public:
	CScrollData() 
		: wX(0), wY(0)
	{ }
	~CScrollData()
	{ }

	CScrollData(CScrollData &Src) {SetMembers(Src);}
	CScrollData& operator= (CScrollData &Src)
	{
		SetMembers(Src);
		return *this;
	}

	UINT			wX;
	UINT			wY;
	CDbMessageText	ScrollText;

private:
	void SetMembers(CScrollData &Src)
	{
		wX = Src.wX;
		wY = Src.wY;
		ScrollText = (const WCHAR*)Src.ScrollText;
	}
};

//******************************************************************************************
class CExitData
{
public:
	CExitData()
		: dwLevelID(0), wLeft(0), wRight(0), wTop(0), wBottom(0)
	{ }
	CExitData(const DWORD dwLevelID, const UINT wLeft, const UINT wRight,
         const UINT wTop, const UINT wBottom)
		: dwLevelID(dwLevelID), wLeft(wLeft), wRight(wRight), wTop(wTop), wBottom(wBottom)
	{ }
	DWORD			dwLevelID;
	UINT			wLeft, wRight, wTop, wBottom;
};

//******************************************************************************************
class CDbRooms;
class CDbSavedGame;
class CDbLevel;
class CCueEvents;
class CMimic;
class CDbRoom : public CDbBase
{

protected:
	friend class CDbLevel;
	friend class CDbRooms;
	friend class CDbSavedGame;
	friend class CCurrentGame;
	friend class CDbVDInterface<CDbRoom>;

	CDbRoom();
	void				ClearPlotHistory();

	bool				bPlotsMade;

public:
	CDbRoom(CDbRoom &Src);
	CDbRoom &operator= (const CDbRoom &Src) {
		SetMembers(Src);
		return *this;
	}
	void MakeCopy(const CDbRoom &Src) {SetMembers(Src, false);}
	CDbRoom* MakeCopy() const;

	virtual ~CDbRoom();
	
	DWORD				dwRoomID;
	DWORD				dwLevelID;
	DWORD				dwRoomX;
	DWORD				dwRoomY;
	UINT				wRoomCols;
	UINT				wRoomRows;
	UINT				wStyle;
	bool				bIsRequired;
	char *				pszOSquares;
	char *				pszTSquares;
	UINT				wOrbCount;
	COrbData *			parrOrbs;
	UINT				wMonsterCount;
	UINT				wBrainCount;
	CMonster *			pFirstMonster;
	CMonster *			pLastMonster;
	CMonster **			pMonsterSquares;	//points to monster occupying each square
	UINT				wScrollCount;
	CScrollData *		parrScrolls;
	vector<CExitData*> Exits;
	UINT				wTrapDoorsLeft;
	CPathMap *			pPathMap[NumMovementTypes];
	CDbDemos			Demos;
	CDbSavedGames		SavedGames;

	void				ActivateOrb(const UINT wX, const UINT wY,
			CCueEvents &CueEvents, CMimic *pMimic=NULL);
	CMonster *		AddNewMonster(const UINT wMonsterType, const UINT wX,
			const UINT wY);
	void				AddOrb(COrbData *pOrb);
	COrbData *		AddOrbToSquare(const UINT wX, const UINT wY);
	void				AddScroll(CScrollData *pScroll);
	bool				AddExit(CExitData *pExit);
	bool				AreMonstersInRect(const UINT wLeft, const UINT wTop,
			const UINT wRight, const UINT wBottom) const;
	bool				BrainSensesSwordsman() const;
   bool           CanSetSwordsman(const DWORD dwX, const DWORD dwY,
         const bool bRoomConquered=true) const;
	DWORD				CalcRoomArea() const {return this->wRoomCols * this->wRoomRows;}
	bool				ChangeTiles(const UINT unOldTile, const UINT unNewTile);
	void				ClearDeadMonsters();
	void				ClearMonsters();
	void				CreatePathMap(const UINT wX, const UINT wY,
			const MovementType eMovement);
	void				DecMonsterCount(CCueEvents &CueEvents);
	void				DeleteExitAtSquare(const UINT wX, const UINT wY);
	void				DeleteOrbAtSquare(const UINT wX, const UINT wY);
	void				DeleteScrollTextAtSquare(const UINT wX, const UINT wY);
	void				DestroyCrumblyWall(const UINT wX, const UINT wY,
			CCueEvents &CueEvents);
	void				DestroyTrapdoor(const UINT wX, const UINT wY, CCueEvents &CueEvents);
	bool				DoesSquareContainMimicPlacementObstacle(const UINT wX, const UINT wY) const;
	bool				DoesSquareContainMimicSword(const UINT wX, const UINT wY) const;
	bool				DoesSquareContainSwordsmanObstacle(const UINT wX, const UINT wY, 
			const UINT wO) const;
	void				FloodPlot(const UINT wX, const UINT wY, const UINT wNewTile);
   void           GetAllDoorSquares(const UINT wX, const UINT wY, CCoordStack& squares) const;
	UINT           GetExitIndexAt(const UINT wX, const UINT wY) const;
	bool				GetExitLevelIDAt(const UINT wX, const UINT wY, DWORD &dwLevelID) const;
	CDbLevel *			GetLevel() const;
	void				GetLevelPositionDescription(WSTRING &wstrDescription,
         const bool bAbbreviate=false);
	void				GetMimicSwordCoords(CCoordIndex &MimicSwordCoords) const;
	void				GetSwordCoords(CCoordIndex &SwordCoords) const;
	CMonster *			GetMonsterAtSquare(const UINT wX, const UINT wY) const;
	COrbData *			GetOrbAtCoords(const UINT wX, const UINT wY) const;
	const WCHAR *			GetScrollTextAtSquare(const UINT wX, const UINT wY) const;
	UINT				GetOSquare(const UINT wX, const UINT wY) const;
	UINT				GetTSquare(const UINT wX, const UINT wY) const;
	void				GrowTar(CCueEvents &CueEvents);
	void				KillMonster(CMonster *pMonster, CCueEvents &CueEvents);
	bool				KillMonsterAtSquare(const UINT wX, const UINT wY, CCueEvents &CueEvents);
	bool				MonsterHeadIsAt(const UINT wX, const UINT wY) const;
	void				MoveMonster(CMonster* const pMonster, const UINT wDestX, const UINT wDestY);
	bool				IsBrainPresent() const {return (this->wBrainCount != 0);}
	bool				IsDoorOpen(const int nCol, const int nRow);
	bool				IsMonsterWithin(const UINT wX, const UINT wY, const UINT wSquares) const;
	static bool		IsRequired(const DWORD dwRoomID);
	bool				IsSwordWithinRect(const UINT wMinX, const UINT wMinY,
			const UINT wMaxX, const UINT wMaxY) const;
	bool				IsTarStableAt(const UINT wX, const UINT wY) const;
	bool				IsTarVulnerableToStab(const UINT wX, const UINT wY) const;
	bool				IsValidColRow(const UINT wX, const UINT wY) const;
	bool				Load(const DWORD dwLoadRoomID);
	void				Plot(const UINT wX, const UINT wY, const UINT wTileNo,
			CMonster *pMonster=NULL);
	void				Reload();
	bool				RemoveBlueDoors();
	bool				RemoveGreenDoors();
	bool				RemoveRedDoors();
	bool				RemoveSerpents();
	void				RemoveStabbedTar(const UINT wX, const UINT wY, CCueEvents &CueEvents);
	void				ResetMonsterFirstTurnFlags();
	void				SetCurrentGame(const CCurrentGame *pSetCurrentGame);

   bool				SetMembers(const CDbRoom &Src, const bool bCopyLocalInfo=true);

	//Import handling
	virtual MESSAGE_ID	SetProperty(const PROPTYPE pType, char* const str,
			CImportInfo &info, bool &bSaveRecord);
	virtual MESSAGE_ID	SetProperty(const VIEWPROPTYPE vpType, const PROPTYPE pType,
			char* const str, CImportInfo &info);

	void				SetScrollTextAtSquare(const UINT wX, const UINT wY, WCHAR *pwczScrollText);
	void				SetExit(const DWORD dwLevelID, const UINT wX, const UINT wY,
         const UINT wX2=(UINT)-1, const UINT wY2=(UINT)-1);
	bool				SomeMonsterCanSmellSwordsman() const;
	bool				StabTar(const UINT wX, const UINT wY, CCueEvents &CueEvents,
			const bool removeTarNow);
	virtual bool	Update();
   void UpdateExitIDs(const DWORD dwNewHoldID=0, const bool bResetIDs=true);

private:
	enum tartype {oldtar, newtar, notar};

	void				Clear();
	void				CloseYellowDoor(const UINT wX, const UINT wY);
	void				DeletePathMaps();
	bool				DoesSquareContainPathMapObstacle(const UINT wX, const UINT wY,
			const MovementType eMovement) const;
	CMonster*		FindLongMonster(const UINT wX, const UINT wY,
			const UINT wFromDirection=10) const;
	void				GetLevelPositionDescription_English(WSTRING &wstrDescription,
         const int dx, const int dy, const bool bAbbrev=false);
	DWORD				GetLocalID() const;
	void				GetNumber_English(const DWORD num, WCHAR *str);
   void           InitRoomStats();
	void				LinkMonster(CMonster *pMonster);
   void           LinkMonsterSegments();
	bool				LoadOrbs(c4_View &OrbsView);
	bool				LoadMonsters(c4_View &MonstersView);
	bool				LoadScrolls(c4_View &ScrollsView);
	bool				LoadExits(c4_View &ExitsView);
	bool				NewTarWouldBeStable(tartype *added_tar, const UINT tx, const UINT ty);
	void				OpenYellowDoor(const UINT wX, const UINT wY);
	c4_Bytes *				PackSquares() const;
	bool				RemoveLongMonsterPieces(CMonster *pMonster);
	void				SaveOrbs(c4_View &OrbsView) const;
	void				SaveMonsters(c4_View &MonstersView) const;
	void				SaveScrolls(c4_View &ScrollsView);
	void				SaveExits(c4_View &ExitsView) const;
	void				SetCurrentGameForMonsters(const CCurrentGame *pSetCurrentGame);
	void				ToggleYellowDoor(const UINT wX, const UINT wY);
	static bool	UnpackSquares(const BYTE *pSrc, const DWORD dwSrcSize,
			const DWORD dwSquareCount, char *pszOSquares, char *pszTSquares);

	bool				UpdateExisting();
	bool				UpdateNew();

	list<CMonster *>	DeadMonsters;
	vector<DWORD> deletedScrollIDs;  //message text IDs to be deleted on Update
	const CCurrentGame *pCurrentGame;
};

//******************************************************************************************
class CDbRooms : public CDbVDInterface<CDbRoom>
{
protected:
	friend class CDbLevel;
	friend class CDb;
	CDbRooms()
		: CDbVDInterface<CDbRoom>("Rooms", p_RoomID),
		  dwFilterByLevelID(0)
	{ }

public:
	virtual ~CDbRooms() { }

	virtual void		Delete(const DWORD dwRoomID);
	virtual string	ExportXML(const DWORD dwRoomID, CDbRefs &dbRefs, const bool bRef=false);
	void		FilterBy(const DWORD dwSetFilterByLevelID);
	static DWORD		FindIDAtCoords(const DWORD dwLevelID, const DWORD dwRoomX,
			const DWORD dwRoomY);
	static CDbRoom *	GetByCoords(const DWORD dwLevelID, const DWORD dwRoomX,
			const DWORD dwRoomY);
	virtual CDbRoom *	GetNew();

private:
	virtual void		LoadMembership();

	DWORD		dwFilterByLevelID;
};

bool bIsArrowObstacle(const UINT nArrowTile, const UINT nO);

#endif //...#ifndef DBROOMS_H

// $Log: DbRooms.h,v $
// Revision 1.78  2004/08/10 01:56:57  mrimer
// Fixed scroll message bugs.
//
// Revision 1.77  2003/12/01 19:13:09  mrimer
// Fixed DeleteScrollTextAtSquare() to not delete message texts until Update() is called.
//
// Revision 1.76  2003/11/09 05:23:22  mrimer
// Fixed bug: stair destinations not changed when room is moved to new hold.
//
// Revision 1.75  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.74  2003/09/16 20:04:22  mrimer
// Changed temporary import vars from object members to static method vars in SetProp().
// Fixed another possible memory leak in SetProp(), and added an assertion.
//
// Revision 1.73  2003/09/11 17:37:45  mrimer
// Fixed bug: can't enter conquered room where serpent was on screen edge
//
// Revision 1.72  2003/08/02 01:07:43  mrimer
// Fixed memory leaks in SetMembers().
//
// Revision 1.71  2003/07/30 04:27:41  mrimer
// Added LinkMonsterSegments().  Fixed bug in SetMembers(): pointers to long monster segments not being copied.
//
// Revision 1.70  2003/07/24 01:31:41  mrimer
// Changed room exits data to a vector, and removed wExitCount.  Added DeleteExitAtSquare() and GetExitIndexAt().
//
// Revision 1.69  2003/07/19 02:30:32  mrimer
// Added CanSetSwordsman().
//
// Revision 1.68  2003/07/11 17:27:41  mrimer
// Now correctly filtering loaded room's saved game list by player also.
// Changed SetExit() to explicitly receive the stair region as a parameter.
//
// Revision 1.67  2003/07/09 21:24:41  mrimer
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.66  2003/07/09 11:25:30  schik
// Fixed unitialized var
//
// Revision 1.65  2003/07/07 23:32:17  mrimer
// Modified SetMembers().
//
// Revision 1.64  2003/07/03 21:39:45  mrimer
// Added option to get abbreviated level position description.
//
// Revision 1.63  2003/06/21 06:49:43  mrimer
// Changed return value on GetAllDoorSquareS() to void.
//
// Revision 1.62  2003/06/20 23:48:50  mrimer
// Added BrainSensesSwordsman().
//
// Revision 1.61  2003/06/19 20:10:37  schik
// Added a function to get all squares in a door, given one of the squares.
//
// Revision 1.60  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.59  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.58  2003/05/22 23:39:00  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.57  2003/05/20 18:11:39  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.56  2003/05/08 23:21:01  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.55  2003/05/05 15:59:40  erikh2000
// Fixed inconsistencies between declarations and definitions.  Maybe caused by an incomplete check-in.
//
// Revision 1.54  2003/04/21 21:00:46  mrimer
// Made level position query methods non const for quick DB access.
//
// Revision 1.53  2003/04/17 20:51:51  mrimer
// Added room required for level completion code.
//
// Revision 1.52  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.51  2003/02/24 17:06:29  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.50  2003/02/16 20:29:32  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.49  2002/12/22 01:30:08  mrimer
// Added XML import/export support.  Added CDbRoom::AddOrb(), AddScroll(), AddExit().  Added COrbData::AddAgent().
// Renamed SpecialExit* to Exit*.  (Default exits no longer supported in level records.)
//
// Revision 1.48  2002/11/23 00:12:15  mrimer
// Simplified some code.
//
// Revision 1.47  2002/11/22 21:58:17  mrimer
// Made Delete() not const.
//
// Revision 1.46  2002/11/15 01:23:52  mrimer
// Added IsSwordWithinRect().
//
// Revision 1.45  2002/11/14 19:17:27  mrimer
// Added copy constructors and assignment operators.
// Added routines to add and delete special room member data.
// Added Delete(), Update(), MonsterHeadIsAt(), LinkMonster().
// Added agent support routines to COrbData.
// Moved member initialization into constructor initialization list.
// Made some methods static.  Made some parameters const.
//
// Revision 1.44  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.43  2002/09/24 21:12:00  mrimer
// Added SomeMonsterCanSmellSwordsman().
//
// Revision 1.42  2002/09/13 21:25:58  mrimer
// Moved some code into DecMonsterCount().
//
// Revision 1.41  2002/09/10 18:00:45  mrimer
// Added GetSwordCoords().
//
// Revision 1.40  2002/09/03 22:37:21  mrimer
// Tweaking.
//
// Revision 1.39  2002/08/28 20:25:35  mrimer
// Added IsMonsterWithin().
//
// Revision 1.38  2002/07/25 18:50:21  mrimer
// Tweaking.
//
// Revision 1.37  2002/07/19 20:23:15  mrimer
// Added CAttachableObject references.
//
// Revision 1.36  2002/07/17 20:08:34  erikh2000
// Changed tar removal methods to return cue events.
//
// Revision 1.35  2002/07/05 17:59:34  mrimer
// Minor fixes (includes, etc.)
//
// Revision 1.34  2002/06/21 04:18:49  mrimer
// Added multiple pathmap support.
// Revised function prototypes.
//
// Revision 1.33  2002/06/05 03:03:09  mrimer
// Added includes.
//
// Revision 1.32  2002/04/28 23:48:54  erikh2000
// Added member to CDbRoom that can be used to access all saved games for a room.
// Added dwLevelID parameter to CDbRooms::FindAtCoords()--necessary because in the future rooms on different levels will have same room coords.
//
// Revision 1.31  2002/04/22 21:52:29  mrimer
// Augmented CID_CrumblyWallDestroyed event to include player orientation info.
//
// Revision 1.30  2002/04/12 21:43:54  mrimer
// Added code to perform simultaneous tar stabbings.
// Fixed two bugs in tar growth code.
//
// Revision 1.29  2002/04/12 05:08:27  erikh2000
// Added copy constructor to COrbData.
//
// Revision 1.28  2002/03/16 11:45:35  erikh2000
// Changed CDbRoom::KillMonster() and KillMonsterAtSquare() to return new cueevents param.  (Committed on behalf of mrimer.)
//
// Revision 1.27  2002/03/14 03:53:09  erikh2000
// Added CDbRoom::IsTarVulnerableToStab() and CDbRoom::RemoveStabbedTar().
// Changed wCol,wRow parameters to wX,wY for consistency.
//
// Revision 1.26  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.25  2002/02/27 23:52:45  erikh2000
// Added CDbRoom::ResetMonsterFirstTurnFlags().
//
// Revision 1.24  2002/02/25 03:36:38  erikh2000
// Added CDbRoom::DoesSquareContainPathmapObstacle().
//
// Revision 1.23  2002/02/24 06:19:13  erikh2000
// Added CDbRoom::GetMimicSwordCoords() and CDbRoom::DoesSquareContainMimicSword().
//
// Revision 1.22  2002/02/24 03:45:20  erikh2000
// Added CDbRoom::DestroyCrumblyWall().
// Added CDbRoom::DoesSquareContainMimicPlacementObstacle().
//
// Revision 1.21  2002/02/09 00:56:17  erikh2000
// Changed current game pointers to const.
// Moved md5i's CCurrentGame::StabTar() to CDbRoom::StabTar().
// Moved md5i's CCurrentGame::GrowTar() to CDbRoom::GrowTar().
//
// Revision 1.20  2002/02/08 23:18:14  erikh2000
// Changed CDbRoom to delete monsters that have been killed during object destruction.
// CDbRoom::DeleteMonster() and ::DeleteMonsterAtSquare() renamed to ::KillMonster() and ::KillMonsterAtSquare().
//
// Revision 1.19  2002/02/07 23:28:18  erikh2000
// Added CDbRoom::IsDoorOpen().  (Committed on behalf of j_wicks.)
//
// Revision 1.18  2001/12/16 02:11:27  erikh2000
// Added CDbRoom::Demos member.
// Allow CDb to construct CDbRooms.
//
// Revision 1.17  2001/12/08 05:17:09  erikh2000
// Added CDbRoom::GetFirst() and CDbRoom::GetNext() methods.
//
// Revision 1.16  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.15  2001/11/19 09:22:40  erikh2000
// Changed CDbRoom::AddNewMonster() to not take a process sequence parameter.
//
// Revision 1.14  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.13  2001/11/11 05:01:16  md5i
// Added serpents.
//
// Revision 1.12  2001/11/06 08:49:58  erikh2000
// Wrote DestroyTrapdoor() and added code to track number of trap doors left.  (Committed on behalf of jpburford.)
//
// Revision 1.11  2001/11/03 20:17:28  erikh2000
// Removed OnPlot() and OnLoad() references.  Added plot history tracking.
//
// Revision 1.10  2001/10/29 19:31:41  erikh2000
// Added JP Burford to contributors list.
//
// Revision 1.9  2001/10/29 19:29:33  erikh2000
// Added CDbRoom::ChangeTiles() method and calls from door-removal methods.  (Committed on behalf of jpburford.)
//
// Revision 1.8  2001/10/27 04:41:07  erikh2000
// Added OnLoad callbacks.
// Changed SetOnLoad() and SetOnPlot() to protected to encourage setting callbacks with CCurrentGame methods.
//
// Revision 1.7  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.6  2001/10/21 00:28:24  erikh2000
// Fixed bugs preventing monsters from being processed.
// Added CDbRoom::Update() stub.
// Fixed access violation caused by bad CMonster.pNext pointer.
//
// Revision 1.5  2001/10/20 20:05:12  erikh2000
// Wrote ActivateOrb().
//
// Revision 1.4  2001/10/20 05:45:36  erikh2000
// Added Plot() and SetOnPlot() methods.
// Added stubs for some routines to be written.
// Removed dead code.
//
// Revision 1.3  2001/10/16 00:02:31  erikh2000
// Wrote code to display current room and handle basic swordsman movement.
//
// Revision 1.2  2001/10/07 04:38:34  erikh2000
// General cleanup--finished removing game logic code from DROD.EXE.
// Added ProcessCommand() method to CCurrentGame to be used as interface between UI and game logic.
//
// Revision 1.1.1.1  2001/10/01 22:20:13  erikh2000
// Initial check-in.
//
