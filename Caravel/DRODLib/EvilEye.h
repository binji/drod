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

//EvilEye.h
//Declarations for CEvilEye.
//Class for handling evil eye monster game logic.

#ifndef EVILEYE_H
#define EVILEYE_H

class CCurrentGame;

#include "Monster.h"
#include "MonsterFactory.h"

class CEvilEye : public CMonster
{

public:
	CEvilEye(CCurrentGame *pSetCurrentGame = NULL);
	IMPLEMENT_CLONE(CMonster, CEvilEye)

	virtual bool IsAggressive(void) {return this->isActive;}
	virtual void Process(const int nLastCommand, CCueEvents &CueEvents);

private:
	bool isActive;
};

#endif //...#ifndef EVILEYE_H

// $Log: EvilEye.h,v $
// Revision 1.8  2002/12/22 01:26:01  mrimer
// Added IsAggressive().
//
// Revision 1.7  2002/11/14 19:25:05  mrimer
// Added object copy support.
// Made virtual methods explicit.
//
// Revision 1.6  2002/08/29 21:59:24  mrimer
// Changed Process' return type to void.
//
// Revision 1.5  2002/06/21 04:19:53  mrimer
// Revised includes.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.2  2001/10/30 04:11:54  md5i
// Add EvilEyes.
//
// Revision 1.1.1.1  2001/10/01 22:20:14  erikh2000
// Initial check-in.
//
