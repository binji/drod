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

//RoachQueen.h
//Declarations for CRoachQueen.
//Class for handling roach queen monster game logic.

#ifndef ROACHQUEEN_H
#define ROACHQUEEN_H

#include "Monster.h"
#include "MonsterFactory.h"

class CRoachQueen : public CMonster
{
public:
	CRoachQueen(CCurrentGame *pSetCurrentGame = NULL) : CMonster(M_QROACH, pSetCurrentGame) {}
	IMPLEMENT_CLONE(CMonster, CRoachQueen)

	virtual bool IsAggressive(void) {return false;}
	virtual void GetBrainDirectedMovement(int &dxFirst, int &dyFirst, 
								  int &dx, int &dy) const;
	virtual void Process(const int nLastCommand, CCueEvents &CueEvents);
};

#endif //...#ifndef ROACHQUEEN_H

// $Log: RoachQueen.h,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.13  2002/11/14 19:25:04  mrimer
// Added object copy support.
// Made virtual methods explicit.
//
// Revision 1.12  2002/09/19 17:46:43  mrimer
// Explicitly declared virtual methods.
//
// Revision 1.11  2002/08/29 21:59:25  mrimer
// Changed Process' return type to void.
//
// Revision 1.10  2002/06/21 04:25:57  mrimer
// Revised includes.
//
// Revision 1.9  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.8  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.7  2002/02/25 03:39:43  erikh2000
// Made read-only methods const.
//
// Revision 1.6  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.5  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.4  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.3  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.2  2001/10/24 01:41:38  md5i
// Added Roach Queens and Eggs.
//
// Revision 1.1.1.1  2001/10/01 22:20:31  erikh2000
// Initial check-in.
//
