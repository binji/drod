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

//DBRefs.h

//Used during DB export/import.
//Marks which DB records (of abbreviated references) have been imported/exported.

#ifndef DBREFS_H
#define DBREFS_H

#include "DbBase.h"
#include "DbProps.h"

class CDbRefs : public CDbBase
{
public:
	CDbRefs();
	~CDbRefs();

	bool IsSet(const VIEWTYPE vType, const DWORD dwID) const;

	void Set(const VIEWTYPE vType, const DWORD dwID);

	virtual bool	Update(void) {return false;}

private:
	bool *pbDemos;			//record
	bool *pbHolds;			//ref
	bool *pbLevels;		//ref
	bool *pbPlayers;		//ref
	bool *pbRooms;			//ref
	bool *pbSavedGames;	//record
};

#endif //...#ifndef DBREFS_H

// $Log: DbRefs.h,v $
// Revision 1.1  2003/02/25 00:01:30  erikh2000
// Initial check-in.
//
// Revision 1.1  2002/12/22 01:47:46  mrimer
// Initial check-in.
//
