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

//TarMother.h
//Declarations for CTarMother.
//Class for handling tar mother monster game logic.

#ifndef TARMOTHER_H
#define TARMOTHER_H

#include "Monster.h"
#include "MonsterFactory.h"

class CTarMother : public CMonster
{
public:
	CTarMother(CCurrentGame *pSetCurrentGame = NULL)
		: CMonster(M_TARMOTHER, pSetCurrentGame)
		, bEyeSet(false)
	{ }
	IMPLEMENT_CLONE(CMonster, CTarMother)

	virtual bool IsAggressive(void) {return false;}
	virtual void Process(const int nLastCommand, CCueEvents &CueEvents);

private:
	bool bEyeSet, bLeftEye;
};

#endif //...#ifndef TARMOTHER_H

// $Log: TarMother.h,v $
// Revision 1.15  2003/08/05 15:43:51  erikh2000
// Tar mothers won't spawn when tar babies see Beethro anymore.
//
// Revision 1.14  2002/11/18 18:35:10  mrimer
// Added IsAggressive().
//
// Revision 1.13  2002/11/15 01:25:33  mrimer
// Added vars to tell whether this is a right or left eye.
//
// Revision 1.12  2002/11/14 19:25:04  mrimer
// Added object copy support.
// Made virtual methods explicit.
//
// Revision 1.11  2002/09/19 17:47:30  mrimer
// Added player invisibility rule for tar growth.
//
// Revision 1.10  2002/08/29 21:59:25  mrimer
// Changed Process' return type to void.
//
// Revision 1.9  2002/07/17 20:38:10  erikh2000
// Removed commented out code.
//
// Revision 1.8  2002/06/21 04:25:57  mrimer
// Revised includes.
//
// Revision 1.7  2002/05/14 17:22:51  mrimer
// Now basing particle explosion direction on sword movement (not orientation).
// Changed tar babies' stab appearance to TarStabEffect.
//
// Revision 1.6  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.5  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.4  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.3  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.2  2001/11/13 05:35:54  md5i
// Added TarMother and growing tar.
//
// Revision 1.1.1.1  2001/10/01 22:20:32  erikh2000
// Initial check-in.
//
