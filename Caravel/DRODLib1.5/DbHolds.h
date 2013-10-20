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

//DbHolds.h
//Declarations for CDbHolds and CDbHold.
//Classes for accessing hold data from database.

#ifndef DBHOLDS_H
#define DBHOLDS_H

#pragma warning(disable: 4786)

#include "DbBase.h"
#include "DbLevels.h"
#include "DbMessageText.h"
#include "DbRefs.h"
#include "DbSavedGames.h"
#include "IDList.h"

//******************************************************************************************
class CDbHolds;
class CCurrentGame;
class CDbHold : public CDbBase
{
protected:
	friend CDbHolds;
	friend CDbLevel;
	friend CCurrentGame;
	CDbHold(void);

public:
	~CDbHold(void);

	CDbLevel *	GetStartingLevel(void) const;
	bool			Load(const DWORD dwHoldID);
	virtual MESSAGE_ID	SetProp(const PROPTYPE pType, char* const str,
			PrimaryKeyMaps &Maps, bool &bSaveRecord);
	virtual bool	Update(void);
	
	CDbLevels		Levels;
	CDbSavedGames	SavedGames;

	CDbMessageText	DescriptionText;
	CDbMessageText	NameText;
	CDbDate			LastUpdated;
	DWORD				dwLevelID;	//first level
	DWORD				dwHoldID;
	DWORD				dwPlayerID;	//author (for GUID)

private:
	void		Clear(void);
	DWORD		GetLocalID(void) const;
	bool		UpdateExisting(void);
	bool		UpdateNew(void);

	CDbDate			Created;		//GUID field
	float				dwRequiredDRODVersion;
	DWORD				dwNewLevelIndex;	//for relative level GUIDs
};

//******************************************************************************************
class CDb;
class CDbHolds : public CDbBase
{
protected:
	friend CCurrentGame;
	friend CDb;
	CDbHolds(void)
		: bIsMembershipLoaded(false)
		, pCurrentRow(NULL)
	{}

public:
	~CDbHolds(void) {}

	void			Delete(const DWORD dwHoldID);
	string		ExportXML(const DWORD dwHoldID, CDbRefs &dbRefs, const bool bRef=false);
	static CDbHold *	GetByID(const DWORD dwHoldID);
	CDbHold *	GetFirst(void);
	CDbHold *	GetNext(void);
	static DWORD	GetNewLevelIndex(const DWORD dwHoldID);
	CDbHold *	GetNew(void);
	virtual bool	Update(void) {return false;}

private:
	void		LoadMembership(void);

	bool		bIsMembershipLoaded;
	CIDList		MembershipIDs;
	IDNODE *	pCurrentRow;
};

#endif //...#ifndef DBHOLDS_H

// $Log: DbHolds.h,v $
// Revision 1.1  2003/02/25 00:01:29  erikh2000
// Initial check-in.
//
// Revision 1.11  2003/02/24 17:06:24  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.10  2002/12/22 01:37:27  mrimer
// Added XML import/export support.  (Added GUID support.)
// Added Created, LastUpdated, and author ID.
//
// Revision 1.9  2002/11/22 21:58:17  mrimer
// Made Delete() not const.
//
// Revision 1.8  2002/11/14 19:06:45  mrimer
// Added methods GetNew(), Delete(), and Update(), etc.
// Made GetByID() static.
// Moved member initialization into constructor initialization list.
//
// Revision 1.7  2002/06/09 06:14:14  erikh2000
// Changed code to use new message text class.
//
// Revision 1.6  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.5  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.4  2001/12/08 03:18:31  erikh2000
// Added Levels member to CDbHold.
//
// Revision 1.3  2001/12/08 02:37:03  erikh2000
// Wrote CDbHolds::GetFirst() and CDbHolds::GetNext().
//
// Revision 1.2  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.1.1.1  2001/10/01 22:20:09  erikh2000
// Initial check-in.
//
