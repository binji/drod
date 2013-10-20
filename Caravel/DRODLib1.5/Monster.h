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
 * Michael Welsh Duggan (md5i), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//Monster.h
//Declarations for CMonster. 
//Abstract base class for holding monster data and performing game operations 
//on it.

//SUMMARY
//
//CMonster is an abstract base class from which all monster types derive.
//A monster is just a collection of data and behavior common to most monster
//types.
//
//USAGE
//
//You can derive new monsters from CMonster.  It's necessary to add a new
//MONSTERTYPE enumeration at the top of MonsterFactory.h and add a switch
//handler in CMonsterFactory::GetNewMonster() that will construct your new class.
//Also, add an entry for the monster in TileConstants.h.
//
//In the derived monster class definition, add (1) a constructor calling CMonster,
//passing in the new monster enumeration, and
//(2) IMPLEMENT_CLONE(CMonster, <new monster type>) to make instances of your
//monster type copyable.
//
//If you followed all the above instructions without doing anything else, you
//would have a monster that just sits there (for example, see CBrain).
//
//You can make the monster do things by performing the following.
//1. Override Process().  This determines what the monster will do each turn.
//If a behavior specified in Process() is common among several monster types,
//consider refactoring that behavior into CMonster as an inheritable method.
//
//FRONT END
//
//For the front end, graphics for the new monster type will need to be
//created and specified (such as in GeneralTiles.bmp and TileImageCalcs.h).
//Other components, such as the level editor, probably also need to
//be revised accordingly to include the new monster type.
//
//

#ifndef MONSTER_H
#define MONSTER_H

#include "DbPackedVars.h"
#include "AttachableObject.h"

const int DEFAULT_PROCESS_SEQUENCE = 1000;

//Within this radius, a monster can sense the player when invisible.
#define DEFAULT_SMELL_RANGE (5)

//# turns between which monsters do special things
#define TURNS_PER_CYCLE	(30)

//Monster movement ability types. (update as needed)
typedef enum {
	GROUND,
	AIR,
	NumMovementTypes
} MovementType;

//******************************************************************************************
#define IMPLEMENT_CLONE(CBase, CDerived) virtual CBase *Clone() const \
	{ return new CDerived(*this); }	//should be public

class CMonsterFactory;
class CCurrentGame;
class CCueEvents;
class CMonster : public CAttachableObject
{
protected:
	friend CMonsterFactory;
	CMonster(const UINT wSetType, const CCurrentGame *pSetCurrentGame = NULL,
			const MovementType eMovement = GROUND,
			const UINT wSetProcessSequence = DEFAULT_PROCESS_SEQUENCE)
		: wType(wSetType)
		, wProcessSequence(wSetProcessSequence)
		, pCurrentGame(pSetCurrentGame)
		, eMovement(eMovement)
		, pPrevious(NULL), pNext(NULL)
		, wX(0), wY(0), wO(0)
		, bIsFirstTurn(false)
	{	}

public:
	virtual ~CMonster(void) 
	{
		Clear();
	}

	virtual CMonster *Clone() const=0;

	void			AskYesNo(DWORD dwMessageID, CCueEvents &CueEvents) const;
	virtual bool	CanFindSwordsman(void) const;
	virtual bool	CanSmellObjectAt(const UINT wX, const UINT wY) const;
	void			Clear(void);
	bool			DoesArrowPreventMovement(const int dx, const int dy) const;
	virtual bool	DoesSquareContainObstacle(UINT wX, UINT wY) const;
	void			GetBestMove(int &dx, int &dy) const;
	void			GetBeelineMovement(int &dxFirst, int &dyFirst, 
			int &dx, int &dy) const;
	virtual void	GetBrainDirectedMovement(int &dxFirst, int &dyFirst, 
			int &dx, int &dy) const;
	bool			GetDirectMovement(int &dxFirst, int &dyFirst, 
			int &dx, int &dy) const;
	virtual bool	IsAggressive(void) {return true;}
	bool			IsOnSwordsman(void) const;
	bool			IsSwordsmanWithin(const UINT wSquares) const;
	bool			IsSwordWithin(const UINT wSquares) const;
	virtual bool	IsTileObstacle(const UINT wTileNo) const;
	void			MakeStandardMove(CCueEvents &CueEvents,
			const int dx, const int dy);
	virtual void	Move(const UINT wX, const UINT wY);
	virtual bool	OnAnswer(int nCommand, CCueEvents &CueEvents);
	virtual bool	OnStabbed(CCueEvents &CueEvents);
	virtual void	Process(const int nLastCommand, CCueEvents &CueEvents);
	void			ResetCurrentGame();
	void			Say(DWORD dwMessageID, CCueEvents &CueEvents) const;
	void			SetCurrentGame(const CCurrentGame *pSetCurrentGame);
	virtual void	SetExtraVarsFromMembers(void) { }
	virtual void	SetMembersFromExtraVars(void) { }
	void				SetOrientation(const int& dxFirst, const int& dyFirst);

	UINT			wType;
	UINT			wX;
	UINT			wY;
	UINT			wO;
	UINT			wProcessSequence;
	bool			bIsFirstTurn;
	MovementType eMovement;

	CDbPackedVars	ExtraVars;

	CMonster *		pNext;	//should be updated by caller when copying a monster
	CMonster *		pPrevious;

protected:
	const CCurrentGame *	pCurrentGame;
};

#endif //...#ifndef MONSTER_H

// $Log: Monster.h,v $
// Revision 1.1  2003/02/25 00:01:35  erikh2000
// Initial check-in.
//
// Revision 1.31  2003/01/08 00:40:28  mrimer
// Added documentation for CMonster.
//
// Revision 1.30  2002/12/22 01:24:17  mrimer
// Removed parameters dxFirst/dyFirst from GetBestMove().
//
// Revision 1.29  2002/11/22 01:54:41  mrimer
// Added ResetCurrentGame().
//
// Revision 1.28  2002/11/15 01:24:34  mrimer
// Added IsSwordsmanWithin() and IsSwordWithin().
//
// Revision 1.27  2002/11/14 19:20:13  mrimer
// Added cloning support.  Added IsAggressive().
// Moved member initialization into constructor initialization list.
// Made some parameters const.
//
// Revision 1.26  2002/09/27 17:49:48  mrimer
// Fixed usage of DEFAULT_SMELL_RANGE.
//
// Revision 1.25  2002/09/24 21:13:19  mrimer
// Added CanSmellObjectAt().  Moved DEFAULT_SMELL_RANGE to .h file.
//
// Revision 1.24  2002/09/19 17:46:43  mrimer
// Explicitly declared virtual methods.
//
// Revision 1.23  2002/09/19 16:22:50  mrimer
// Made some parameters const.
//
// Revision 1.22  2002/09/13 22:40:04  mrimer
// Refactored general monster code from derived classes into GetDirectMovement(), MakeStandardMove(), and SetOrientation().
//
// Revision 1.21  2002/08/29 21:59:25  mrimer
// Changed Process' return type to void.
//
// Revision 1.20  2002/07/19 20:23:16  mrimer
// Added CAttachableObject references.
//
// Revision 1.19  2002/07/05 10:43:47  erikh2000
// Fixed a few bugs involving mimics not treating the swordsman as an obstacle.
//
// Revision 1.18  2002/06/24 20:32:43  mrimer
// Fixed NumMovementTypes initialization.
//
// Revision 1.17  2002/06/21 04:20:45  mrimer
// Added multiple pathmap support.
//
// Revision 1.16  2002/05/15 14:29:54  mrimer
// Moved animation data out of DRODLIB (CMonster) and into DROD (CRoomWidget).
//
// Revision 1.15  2002/05/14 19:06:16  mrimer
// Added monster animation.
//
// Revision 1.14  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.13  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.12  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.11  2002/02/09 00:57:49  erikh2000
// Changed current game pointers to const.
//
// Revision 1.10  2001/12/16 02:15:37  erikh2000
// Added CMonster::AskYesNo(), CMonster::Say(), and CMonster::OnAnswer() methods.
//
// Revision 1.9  2001/11/25 21:25:45  erikh2000
// Added CMonster::SetExtraVarsFromMembers() and CMonster::SetMembersFromExtraVars().
//
// Revision 1.8  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.7  2001/11/19 09:21:21  erikh2000
// Changed monster class to construct with a monster type and process sequence variable.
//
// Revision 1.6  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.5  2001/10/23 03:28:28  md5i
// Add GetBestMove routine.
//
// Revision 1.4  2001/10/21 00:29:32  erikh2000
// Fixed bugs in GetBeelineMovement().
// Added checks for monster and sword in DoesSquareContainObstacle().
//
// Revision 1.3  2001/10/20 05:42:44  erikh2000
// Removed tile image references.
//
// Revision 1.2  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.1.1.1  2001/10/01 22:20:18  erikh2000
// Initial check-in.
//
