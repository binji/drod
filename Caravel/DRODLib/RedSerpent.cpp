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

#include "RedSerpent.h"

//
//Public methods.
//

//*****************************************************************************************
CRedSerpent::CRedSerpent(CCurrentGame *pSetCurrentGame)
	: CSerpent(M_SERPENT, pSetCurrentGame)
{
}

//*****************************************************************************************
void CRedSerpent::Process(
//Process a serpent for movement.
//
//Params:
	const int /*nLastCommand*/,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	if (!this->foundTail)
		FindTail();

	int dxFirst, dyFirst, dx, dy;
	if (!GetSerpentMovement(dxFirst, dyFirst, dx, dy))
   {
      this->wPrevX = this->wX;
      this->wPrevY = this->wY;
		return;
   }

	// Move according to direction chosen.
	LengthenHead(dx,dy,nGetOX(wO),nGetOY(wO));
	if (ShortenTail(CueEvents))
		return;	//snake died

	if (IsOnSwordsman())
		CueEvents.Add(CID_MonsterKilledPlayer, this);
}

// $Log: RedSerpent.cpp,v $
// Revision 1.1  2003/08/02 20:35:09  mrimer
// Initial check-in.
//
