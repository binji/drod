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

//MonsterFactory.cpp
//Implementation of CMonsterFactory.

#include "MonsterFactory.h"
#include <BackEndLib/Assert.h>

//Monster child class includes.
#include "Brain.h"
#include "Roach.h"
#include "RoachQueen.h"
#include "RoachEgg.h"
#include "Goblin.h"
#include "Neather.h"
#include "Wraithwing.h"
#include "EvilEye.h"
#include "RedSerpent.h"
#include "TarMother.h"
#include "TarBaby.h"
#include "Mimic.h"
#include "Spider.h"

//******************************************************************************************
CMonster * CMonsterFactory::GetNewMonster(
//Class factory for classes derived from CMonster.  New monster will not be
//associated with room yet and will need its members set before it is used.
//
//Params:
	const MONSTERTYPE eType)		//(in)	One of the M_* monster enumerations.
//
//Returns:
//Pointer to a new instance of class derived from CMonster.
{
	ASSERT(eType < MONSTER_TYPES);

	//Can use a function pointer array later for better speed.

	switch (eType)
	{
		case M_ROACH:		
		return new CRoach(this->pCurrentGame);
		
		case M_QROACH:		
		return new CRoachQueen(this->pCurrentGame);
		
		case M_REGG:		
		return new CRoachEgg(this->pCurrentGame);

		case M_GOBLIN:		
		return new CGoblin(this->pCurrentGame);

		case M_NEATHER:
		return new CNeather(this->pCurrentGame);

		case M_WWING:
		return new CWraithWing(this->pCurrentGame);

		case M_EYE:
		return new CEvilEye(this->pCurrentGame);

		case M_SERPENT:
		return new CRedSerpent(this->pCurrentGame);

		case M_TARMOTHER:
		return new CTarMother(this->pCurrentGame);

		case M_TARBABY:
		return new CTarBaby(this->pCurrentGame);

		case M_BRAIN:
		return new CBrain(this->pCurrentGame);
		
		case M_MIMIC:
		return new CMimic(this->pCurrentGame);

		case M_SPIDER:
		return new CSpider(this->pCurrentGame);

		default:
			ASSERTP(false, "Unexpected monster type.");
			return NULL; 
	}
}

// $Log: MonsterFactory.cpp,v $
// Revision 1.13  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.12  2003/08/02 20:36:19  mrimer
// Changed CSerpent to CRedSerpent.
//
// Revision 1.11  2003/07/22 18:36:21  mrimer
// Removed reinterpret_casts.
//
// Revision 1.10  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.9  2003/05/22 23:39:02  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.8  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.7  2003/01/08 00:47:16  mrimer
// Changed monster type to an enumeration.
//
// Revision 1.6  2002/06/21 04:42:38  mrimer
// Revised includes.
//
// Revision 1.5  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.2  2001/11/12 01:25:54  erikh2000
// Added code for CSpider monster class.  (Committed on behalf of timeracer.)
//
// Revision 1.1.1.1  2001/10/01 22:20:18  erikh2000
// Initial check-in.
//
