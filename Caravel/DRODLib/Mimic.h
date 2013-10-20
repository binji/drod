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

//Mimic.h
//Declarations for CMimic.
//Class for handling mimic monster game logic.  C-style routines need to
//be converted into CMimic class.

#ifndef MIMIC_H
#define MIMIC_H

#include "Monster.h"
#include "MonsterFactory.h"
#include "GameConstants.h"

class CMimic : public CMonster
{
public:
	CMimic(CCurrentGame *pSetCurrentGame = NULL);
	IMPLEMENT_CLONE(CMonster, CMimic)

	virtual bool	DoesSquareContainObstacle(const UINT wCol, const UINT wRow) const;
	virtual bool	IsAggressive(void) {return false;}
	virtual bool	IsTileObstacle(const UINT wTileNo) const;
	virtual void	Process(const int nLastCommand, CCueEvents &CueEvents);

	UINT			GetSwordX(void) const {return this->wX + nGetOX(this->wO);}
	UINT			GetSwordY(void) const {return this->wY + nGetOY(this->wO);}
};
	
#endif //...#ifndef MIMIC_H

// $Log: Mimic.h,v $
// Revision 1.14  2002/11/14 19:25:05  mrimer
// Added object copy support.
// Made virtual methods explicit.
//
// Revision 1.13  2002/08/29 21:59:25  mrimer
// Changed Process' return type to void.
//
// Revision 1.12  2002/08/25 18:57:33  erikh2000
// Declared some methods as virtual for clarity.
//
// Revision 1.11  2002/07/05 10:43:47  erikh2000
// Fixed a few bugs involving mimics not treating the swordsman as an obstacle.
//
// Revision 1.10  2002/06/22 05:45:06  erikh2000
// Replaced wSwordX and wSwordY members with GetSwordX() and GetSwordY() methods to fix a problem.
//
// Revision 1.9  2002/06/21 04:19:53  mrimer
// Revised includes.
//
// Revision 1.8  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.7  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.6  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.5  2002/02/24 03:45:55  erikh2000
// Removed CheckSwordHit().
// Added IsTileObstacle() override since mimics can step on potions.
//
// Revision 1.4  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.3  2001/11/17 23:08:10  erikh2000
// Wrote code to make mimics work.  (Committed on behalf of j_wicks.)
//
// Revision 1.2  2001/10/20 05:46:23  erikh2000
// Removed dead code.
//
// Revision 1.1.1.1  2001/10/01 22:20:17  erikh2000
// Initial check-in.
//
