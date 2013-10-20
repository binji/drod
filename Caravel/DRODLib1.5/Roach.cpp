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

//Roach.cpp
//Implementation of CRoach.

#include "Roach.h"

//
//Public methods.
//

//*****************************************************************************************
void CRoach::Process(
//Process a roach for movement.
//
//Params:
	const int nLastCommand,	//(in) Last swordsman command.
	CCueEvents &CueEvents)	//(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	//Get movement offsets.
	int dxFirst, dyFirst, dx, dy;
	if (!GetDirectMovement(dxFirst, dyFirst, dx, dy))
		return;

	//Move roach to new destination square.
	MakeStandardMove(CueEvents,dx,dy);
	SetOrientation(dxFirst, dyFirst);
}

// $Log: Roach.cpp,v $
// Revision 1.1  2003/02/25 00:01:39  erikh2000
// Initial check-in.
//
// Revision 1.14  2002/09/13 22:41:46  mrimer
// Refactored general monster code into base class.
//
// Revision 1.13  2002/08/29 22:00:40  mrimer
// Modified Process(), removing return type.  Some tweaking.
//
// Revision 1.12  2002/06/21 04:43:15  mrimer
// Revised includes.
// Added movement only on non-zero delta.
//
// Revision 1.11  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.10  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.9  2002/02/10 03:59:39  erikh2000
// Twiddling.
//
// Revision 1.8  2001/12/16 02:21:29  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.7  2001/11/25 02:31:31  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.6  2001/11/14 01:43:27  md5i
// Fix for invisibility.
//
// Revision 1.5  2001/10/27 20:26:37  erikh2000
// Removed CueEvents.Clear calls and fixed some kind of EOL translation causing double-spacing.
//
// Revision 1.4  2001/10/21 00:29:57  erikh2000
// Added code to set orientation of moving roach.
//
// Revision 1.3  2001/10/20 05:42:43  erikh2000
// Removed tile image references.
//
// Revision 1.2  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.1.1.1  2001/10/01 22:20:31  erikh2000
// Initial check-in.
//
