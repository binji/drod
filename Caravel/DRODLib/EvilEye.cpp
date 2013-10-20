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

//EvilEye.cpp
//Implementation of CEvilEye.

#include "EvilEye.h"

//
//Public methods.
//

//*****************************************************************************************
CEvilEye::CEvilEye (CCurrentGame *pSetCurrentGame) : CMonster(M_EYE, pSetCurrentGame)
{
	this->isActive = false;
}

//*****************************************************************************************
void CEvilEye::Process(
//Process an Evil Eye for movement.
//
//Params:
	const int /*nLastCommand*/,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	if (!this->isActive && CanFindSwordsman()) {
		//Check whether evil eye sees player and wakes up.
		const int dx = nGetOX(this->wO);
		const int dy = nGetOY(this->wO);
		UINT cx = this->wX, cy = this->wY;
		bool done = false;
		while (!done) 
		{
			cx += dx;
			cy += dy;
			if (cx == this->pCurrentGame->swordsman.wX &&
					cy == this->pCurrentGame->swordsman.wY)
			{
				this->isActive = true;
				CueEvents.Add(CID_EvilEyeWoke, this);
				break;
			}
			if (cx >= this->pCurrentGame->pRoom->wRoomCols ||
			    cy >= this->pCurrentGame->pRoom->wRoomRows)
				break;

			switch (this->pCurrentGame->pRoom->GetOSquare(cx, cy)) {
			case T_FLOOR:
			case T_CHECKPOINT:
			case T_PIT:
			case T_DOOR_YO:
			case T_TRAPDOOR: 
				if (this->pCurrentGame->pRoom->GetTSquare(cx,cy) != T_ORB)
					break;
			default:
					done = true;
			}
		}
	}

	if (!this->isActive)
		return;

	//Get movement offsets.
	int dxFirst, dyFirst, dx, dy;
	if (!GetDirectMovement(dxFirst, dyFirst, dx, dy))
		return;
 
	//Move evil eye to new destination square.
	MakeStandardMove(CueEvents,dx,dy);
	SetOrientation(dxFirst, dyFirst);
}

// $Log: EvilEye.cpp,v $
// Revision 1.19  2003/07/12 00:35:17  mrimer
// This time it's really fixed.
//
// Revision 1.18  2003/07/12 00:16:26  mrimer
// Fixed bug: evil eye becomes active when unsensed invisible player enters line of sight.
//
// Revision 1.17  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.16  2002/12/22 01:53:25  mrimer
// Revised swordsman vars.
//
// Revision 1.15  2002/09/13 22:41:46  mrimer
// Refactored general monster code into base class.
//
// Revision 1.14  2002/08/29 22:00:36  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.13  2002/07/17 20:37:38  erikh2000
// Cue event is now added for eye activation.
// Removed unneeded extra vars storage.
//
// Revision 1.12  2002/06/21 04:34:37  mrimer
// Revised includes.
// Added movement only on non-zero delta.
//
// Revision 1.11  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.10  2002/03/13 06:32:18  erikh2000
// Made orbs block sight of swordsman in Process().
//
// Revision 1.9  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.8  2002/03/04 22:25:33  erikh2000
// Changed obstacle logic to evaluate T_CHECKPOINT as clear.
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
// Revision 1.4  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.3  2001/11/11 17:20:38  md5i
// Another EOL fix.
//
// Revision 1.2  2001/10/30 16:28:25  md5i
// Fix bad EOLs in log.  (I think I know what is causing this now.)
//
// Revision 1.1  2001/10/30 04:11:53  md5i
// Add EvilEyes.
//
