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
 * Michael Welsh Duggan (md5i)
 *
 * ***** END LICENSE BLOCK ***** */

//Serpent.h
//Declarations for CSerpent.
//Class for handling serpent monster game logic.

//FUTURE CONSIDERATIONS
//
//We may want an improved serpent class, i.e.
//CSerpent2, that has the following changes:
//- Can move over scrolls.
//- Can move over arrows.
//- Should use a pathfinding routine that takes into
//  account no diagonal movement.

#ifndef SERPENT_H
#define SERPENT_H

#include "Monster.h"
#include "MonsterFactory.h"

class CSerpent : public CMonster
{
public:
	CSerpent(CCurrentGame *pSetCurrentGame = NULL);
	IMPLEMENT_CLONE(CMonster, CSerpent)

	virtual void Process(const int nLastCommand, CCueEvents &CueEvents);
	virtual bool IsTileObstacle(const UINT wTileNo) const;
	virtual bool DoesSquareContainObstacle(UINT wCol, UINT wRow) const;
	virtual void GetBrainDirectedMovement(int &dxFirst, int &dyFirst, 
								  int &dx, int &dy) const;
	virtual bool OnStabbed(CCueEvents &CueEvents) {return false;} //Stabs don't kill serpent.

	void GetTail(UINT &wTailX, UINT &wTailY);

protected:
	bool GetSerpentMovement(int &dxFirst, int &dyFirst, int &dx, int &dy) const;
	bool LengthenHead(const int dx, const int dy, const int oX, const int oY);
	bool ShortenTail(CCueEvents &CueEvents);

private:
	void FindTail(void);
	void GetNormalMovement(int&, int&) const;
	bool CanMoveTo(const int x, const int y) const;

	UINT tailX, tailY, tailO;
	bool foundTail;
};

#endif //...#ifndef SERPENT_H

// $Log: Serpent.h,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.15  2002/11/18 18:35:53  mrimer
// Added GetTail().
//
// Revision 1.14  2002/11/14 19:25:04  mrimer
// Added object copy support.
// Made virtual methods explicit.
//
// Revision 1.13  2002/10/03 21:16:40  mrimer
// Cleaned up code.
//
// Revision 1.12  2002/09/19 17:49:28  mrimer
// Cleaned up serpent code.  Explicitly declared vitual methods.
// Added player invisibility rule for serpents.
//
// Revision 1.11  2002/09/10 19:24:44  mrimer
// Tweaking.
//
// Revision 1.10  2002/08/29 21:59:25  mrimer
// Changed Process' return type to void.
//
// Revision 1.9  2002/06/24 23:27:15  mrimer
// Added FUTURE CONSIDERATIONS.
//
// Revision 1.8  2002/06/21 04:25:57  mrimer
// Revised includes.
//
// Revision 1.7  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.6  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.5  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.4  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.3  2001/11/12 01:27:25  erikh2000
// Made serpents not die when stabbed.
// Made serpents traverse squares containing swords.
//
// Revision 1.2  2001/11/11 05:01:16  md5i
// Added serpents.
//
// Revision 1.1.1.1  2001/10/01 22:20:32  erikh2000
// Initial check-in.
//
