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

//RoachEgg.cpp
//Implementation of CRoachEgg.

#include "RoachEgg.h"

//
//Public methods.
//

//*****************************************************************************************
CRoachEgg::CRoachEgg(CCurrentGame *pSetCurrentGame) : CMonster(M_REGG, pSetCurrentGame)
{
	// Egg deposited at 8 o'clock, born at midnight (5 moves in all).
	this->wO = SW;
}

//*****************************************************************************************
void CRoachEgg::Process(
//Process a roach egg for movement.
//
//Params:
	const int /*nLastCommand*/,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	switch (this->wO) {
	case SW:
		this->wO = W;
		break;
	case W:
		this->wO = NW;
		break;
	case NW:
		this->wO = N;
		break;
	case N:
		// Get rid of the egg and spawn a roach.
		CueEvents.Add(CID_EggHatched, this);
		break;
	default:
		ASSERTP(false, "Bad orientation.");
	}
}

// $Log: RoachEgg.cpp,v $
// Revision 1.14  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.13  2003/08/13 21:17:39  mrimer
// Fixed tar mother to spawn one move later the first time, making it every 30th move.
// Extended the roach egg hatching cycle by one turn, from four turns to five.
//
// Revision 1.12  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2002/08/29 22:00:40  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.10  2002/06/21 04:44:55  mrimer
// Revised includes.
// Moved add new roach code to when cue event is processed.
//
// Revision 1.9  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.8  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.7  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.6  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.5  2001/11/19 09:25:52  erikh2000
// Added monster type and process sequence params to CMonster constructor call.
//
// Revision 1.4  2001/10/27 20:26:37  erikh2000
// Removed CueEvents.Clear calls and fixed some kind of EOL translation causing double-spacing.
//
// Revision 1.3  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.2  2001/10/24 02:18:13  md5i
// Fixed egg tiles.  Used bIsFirstTurn status.
//
// Revision 1.1  2001/10/24 01:41:38  md5i
// Added Roach Queens and Eggs.
//
