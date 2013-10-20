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

//CSwordsman.h
//Declarations for CSwordsman.h.
//
//GENERAL
//
//Class for accessing and manipulating player game state.

#ifndef CSWORDSMAN_H
#define CSWORDSMAN_H

#include "GameConstants.h"

class CSwordsman
{
public:
	CSwordsman(void) {Clear();}
	~CSwordsman(void) {}

	void Clear();
	bool Move(const UINT wSetX, const UINT wSetY);
	void SetOrientation(const UINT wO);
	void SetSwordMovement(const int nCommand);

	//Room position and orientation
	UINT		wX;
	UINT		wY;
	UINT		wO;

	//Previous position
	UINT		wPrevX;
	UINT		wPrevY;

	//Sword position
	UINT		wSwordX;
	UINT		wSwordY;
	UINT		wSwordMovement;

	//Mimic placing
	bool		bIsPlacingMimic;
	UINT		wMimicCursorX;
	UINT		wMimicCursorY;

	//Whether player can be seen by monsters
	bool		bIsVisible;

private:
	void SetSwordCoords();
};

#endif	//...#ifndef CSWORDSMAN_H

// $Log: Swordsman.h,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.2  2003/01/04 23:06:15  mrimer
// Added SetOrientation().
//
// Revision 1.1  2002/12/22 00:36:56  mrimer
// Initial check-in.  (Code refactored from CurrentGame.*)
//
