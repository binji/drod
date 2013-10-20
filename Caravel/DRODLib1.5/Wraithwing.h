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

//WraithWing.h
//Declarations for CWraithWing.
//Class for handling wraithwing monster game logic.

#ifndef WRAITHWING_H
#define WRAITHWING_H

#include "Monster.h"
#include "MonsterFactory.h"

class CWraithWing : public CMonster
{
public:
	CWraithWing(CCurrentGame *pSetCurrentGame = NULL) : CMonster(M_WWING, pSetCurrentGame, AIR) {}
	IMPLEMENT_CLONE(CMonster, CWraithWing)

	virtual void Process(const int nLastCommand, CCueEvents &CueEvents);
};

#endif //...#ifndef WRAITHWING_H

// $Log: Wraithwing.h,v $
// Revision 1.1  2003/02/25 00:01:41  erikh2000
// Initial check-in.
//
// Revision 1.10  2002/11/14 19:25:04  mrimer
// Added object copy support.
// Made virtual methods explicit.
//
// Revision 1.9  2002/08/29 21:59:25  mrimer
// Changed Process' return type to void.
//
// Revision 1.8  2002/06/21 04:27:31  mrimer
// Added AIR movement flag.
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
// Revision 1.3  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.2  2001/10/26 00:49:55  md5i
// Wraith-wings added.
//
// Revision 1.1.1.1  2001/10/01 22:20:33  erikh2000
// Initial check-in.
//
