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

#include "DbVDInterface.h"
#include "DbLevels.h"
#include "DbSavedGames.h"

//******************************************************************************************
class CDbHolds;
class CCurrentGame;
class CDbHold : public CDbBase
{
protected:
	friend class CDbHolds;
	friend class CDbLevel;
	friend class CCurrentGame;
	friend class CDbVDInterface<CDbHold>;
	CDbHold();

public:
	CDbHold(CDbHold &Src) : CDbBase() {SetMembers(Src);}
	CDbHold &operator= (const CDbHold &Src) {
		Clear();
		SetMembers(Src);
		return *this;
	}

   virtual ~CDbHold();

   bool        ChangeAuthor(const DWORD dwNewAuthorID);
	CDbLevel *	GetStartingLevel() const;
	void			InsertLevel(CDbLevel *pLevel, const DWORD dwLevelSupplantedID=0L);
	bool			Load(const DWORD dwHoldID);
   CDbHold*    MakeCopy();
	void		   RemoveLevel(const DWORD dwLevelID, const DWORD dwNewLevelID);
   bool        Repair();
   DWORD       SaveCopyOfLevels(const DWORD dwNewHoldID);
	virtual MESSAGE_ID	SetProperty(const PROPTYPE pType, char* const str,
			CImportInfo &info, bool &bSaveRecord);
	virtual bool	Update();
	
	bool			VerifySolvability();

	CDbLevels		Levels;
	CDbSavedGames	SavedGames;

	CDbMessageText	DescriptionText;
	CDbMessageText	NameText;
    CDbMessageText	EndHoldText;   //message shown player on completing hold
	CDate			LastUpdated;
	DWORD				dwLevelID;	//first level
	DWORD				dwHoldID;
	DWORD				dwPlayerID;	//author (for GUID)

   enum EditAccess
   {
      Anyone=0,
      OnlyYou,
      YouAndConquerors
   };
   EditAccess     editingPrivileges;  //who can edit the hold

private:
	void		Clear();
	DWORD		GetLocalID() const;
	bool		SetMembers(const CDbHold &Src);
	bool		UpdateExisting();
	bool		UpdateNew();

	CDate		Created;		//GUID field
	DWORD		dwNewLevelIndex;	//for relative level GUIDs
};

//******************************************************************************************
class CDb;
class CDbHolds : public CDbVDInterface<CDbHold>
{
protected:
	friend class CCurrentGame;
	friend class CDb;
	CDbHolds()
		: CDbVDInterface<CDbHold>("Holds", p_HoldID)
	{}

public:
	virtual ~CDbHolds() {}

	virtual void		Delete(const DWORD dwHoldID);
	virtual string		ExportXML(const DWORD dwHoldID, CDbRefs &dbRefs, const bool bRef=false);
   bool        EditableHoldExists() const;
   static DWORD       GetLevelIDAtIndex(const DWORD dwIndex, const DWORD dwHoldID);
	bool		   PlayerCanEditHold(const DWORD dwHoldID) const;
};

#endif //...#ifndef DBHOLDS_H

// $Log: DbHolds.h,v $
// Revision 1.26  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.25  2003/08/12 18:50:00  mrimer
// Moved CDbLevelS::RemoveFromHold() to CDbHold::RemoveLevel().
//
// Revision 1.24  2003/08/06 01:10:26  mrimer
// Fixed bug: level exit IDs not being updated on room copy to different hold.
//
// Revision 1.23  2003/07/21 22:03:10  mrimer
// Added ChangeAuthor().  Fixed bug in SetMembers().
//
// Revision 1.22  2003/07/19 02:31:39  mrimer
// Added EndHoldText member.
//
// Revision 1.21  2003/07/16 07:37:18  mrimer
// Rewrote CDbLevel::InsertIntoHold() as CDbHold::InsertLevel().
//
// Revision 1.20  2003/07/15 00:27:03  mrimer
// Added MakeCopy().
//
// Revision 1.19  2003/07/09 21:22:24  mrimer
// Added Repair().  Revised export and import routines to be more robust.
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.18  2003/07/07 23:35:30  mrimer
// Added new methods to make a copy of an entire hold in the DB.
//
// Revision 1.17  2003/06/27 17:27:32  mrimer
// Added hold editing privilege data.
//
// Revision 1.16  2003/06/09 19:26:27  mrimer
// Added methods for checking whether a hold is editable by a player.
//
// Revision 1.15  2003/05/20 18:11:39  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.14  2003/05/08 23:19:45  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.13  2003/04/29 11:05:53  mrimer
// Moved CDb::verifyHoldSolvability() to CDbHolds::VerifySolvability().
//
// Revision 1.12  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
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
