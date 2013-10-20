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
 *
 * ***** END LICENSE BLOCK ***** */

//Brain.h
//Declarations for CBrain.
//Class for handling brain monster game logic.

#ifndef BRAIN_H
#define BRAIN_H

class CCurrentGame;

#include "Monster.h"
#include "MonsterFactory.h"

class CBrain : public CMonster
{
public:
	CBrain(CCurrentGame *pSetCurrentGame = NULL) : 
			CMonster(M_BRAIN, pSetCurrentGame) {}
	IMPLEMENT_CLONE(CMonster, CBrain)

	virtual bool IsAggressive(void) {return false;}
};

#endif //...#ifndef BRAIN_H

// $Log: Brain.h,v $
// Revision 1.4  2002/11/14 19:26:20  mrimer
// Added object copy support.  Added IsAggressive().
//
// Revision 1.3  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.2  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.1.1.1  2001/10/01 22:20:06  erikh2000
// Initial check-in.
//
