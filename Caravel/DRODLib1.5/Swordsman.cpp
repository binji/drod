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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//Swordsman.cpp.
//Implementation of CSwordsman,

#include "Swordsman.h"
#include "Assert.h"

//
// Public methods
//

//*****************************************************************************
void CSwordsman::Clear()
{
	this->wX = this->wY = this->wO = this->wSwordX = this->wSwordY = 0;
	this->bIsPlacingMimic = false;
	this->bIsVisible = true;
}

//*****************************************************************************
bool CSwordsman::Move(
//Move player to new location
//
//Returns: whether player was moved
//
//Params:
	const UINT wSetX, const UINT wSetY)	//(in) position to move to
{
	const bool bMoved = !(this->wX == wSetX && this->wY == wSetY);

	//Set new swordsman coords.
	this->wPrevX = this->wX;
	this->wPrevY = this->wY;
	this->wX = wSetX;
	this->wY = wSetY;

	SetSwordCoords();

	return bMoved;
}

//*****************************************************************************
void CSwordsman::SetOrientation(
//Move player to new location
//
//Returns: whether player was moved
//
//Params:
	const UINT wO)	//(in) new orientation
{
	ASSERT(IsValidOrientation(wO));
	this->wO = wO;

	SetSwordCoords();
}

//*****************************************************************************
void CSwordsman::SetSwordMovement(
//Returns: movement sword made to 
//
//Params:
	const int nCommand)		//(in)	Game command.
{
	switch (nCommand)
	{
		case CMD_C: //sword moved orthogonal to direction it's now facing
			switch (this->wO)
			{
				case NW: this->wSwordMovement = NE;	break;
				case N: this->wSwordMovement = E;	break;
				case NE: this->wSwordMovement = SE;	break;
				case W: this->wSwordMovement = N;	break;
				case E: this->wSwordMovement = S;	break;
				case SW: this->wSwordMovement = NW;	break;
				case S: this->wSwordMovement = W;	break;
				case SE: this->wSwordMovement = SW;	break;
			}
			break;
		case CMD_CC: 
			switch (this->wO)
			{
				case NW: this->wSwordMovement = SW;	break;
				case N: this->wSwordMovement = W;	break;
				case NE: this->wSwordMovement = NW;	break;
				case W: this->wSwordMovement = S;	break;
				case E: this->wSwordMovement = N;	break;
				case SW: this->wSwordMovement = SE;	break;
				case S: this->wSwordMovement = E;	break;
				case SE: this->wSwordMovement = NE;	break;
			}
			break;
		case CMD_NW: this->wSwordMovement = NW;	break;
		case CMD_N: this->wSwordMovement = N;		break;
		case CMD_NE: this->wSwordMovement = NE;	break;
		case CMD_W: this->wSwordMovement = W;		break;
		case CMD_E: this->wSwordMovement = E;		break;
		case CMD_SW: this->wSwordMovement = SW;	break;
		case CMD_S: this->wSwordMovement = S;		break;
		case CMD_SE: this->wSwordMovement = SE;	break;	//any movement direction
	}
}

//
// Private methods
//

//*****************************************************************************
void CSwordsman::SetSwordCoords()
//Set sword coordinates.
{
	this->wSwordX = this->wX + nGetOX(this->wO);
	this->wSwordY = this->wY + nGetOY(this->wO);
}

// $Log: Swordsman.cpp,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.2  2003/01/04 23:06:14  mrimer
// Added SetOrientation().
//
// Revision 1.1  2002/12/22 00:36:56  mrimer
// Initial check-in.  (Code refactored from CurrentGame.*)
//
