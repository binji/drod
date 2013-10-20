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
 * Rik Cookney (timeracer)
 *
 * ***** END LICENSE BLOCK ***** */

//MonsterFactory.h
//Declarations for CMonsterFactory.
//Class for creating monster objects derived from CMonster.

#ifndef MONSTERFACTORY_H
#define MONSTERFACTORY_H

//for automatic (convenient) inclusion into all monster types
#include "CueEvents.h"
#include "CurrentGame.h"
#include "DbRooms.h"
#include "GameConstants.h"
#include "TileConstants.h"

#include "Types.h"

//Monster types.
enum MONSTERTYPE {
	M_ROACH=0,
	M_QROACH,
	M_REGG,
	M_GOBLIN,
	M_NEATHER,
	M_WWING,
	M_EYE,
	M_SERPENT,
	M_TARMOTHER,
	M_TARBABY,
	M_BRAIN,
	M_MIMIC,
	M_SPIDER,
	MONSTER_TYPES
};

#define IsValidMonsterType(mt)	((mt)>=0 && (mt)<MONSTER_TYPES)

class CMonster;
class CMonsterFactory
{
public:
	CMonsterFactory(CCurrentGame *pSetCurrentGame = NULL) 
	{
		this->pCurrentGame = pSetCurrentGame;
	}
	~CMonsterFactory(void) {}
	CMonster *GetNewMonster(const MONSTERTYPE eType);

	CCurrentGame *pCurrentGame;
};

#endif //...#ifndef MONSTERFACTORY_H

// $Log: MonsterFactory.h,v $
// Revision 1.1  2003/02/25 00:01:36  erikh2000
// Initial check-in.
//
// Revision 1.7  2003/01/08 00:39:52  mrimer
// Changed monster consts into an enumeration.
//
// Revision 1.6  2002/06/21 04:21:37  mrimer
// Revised includes.
//
// Revision 1.5  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/11/12 01:25:54  erikh2000
// Added code for CSpider monster class.  (Committed on behalf of timeracer.)
//
// Revision 1.2  2001/10/20 05:48:25  erikh2000
// Added IsValidMonsterType() macro.
//
// Revision 1.1.1.1  2001/10/01 22:20:18  erikh2000
// Initial check-in.
//
