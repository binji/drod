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

//DbLevels.h
//Declarations for CDbLevels and CDbLevel.
//Classes for accessing level data from database.

#ifndef DBLEVELS_H
#define DBLEVELS_H

#include "DbVDInterface.h"
#include "DbRooms.h"

//******************************************************************************************
class CDbLevels;
class CDbHold;
class CCurrentGame;
class CDbLevel : public CDbBase
{
protected:
	friend class CDbLevels;
	friend class CDbHold;
	friend class CDbRoom;
	friend class CCurrentGame;
	friend class CDbVDInterface<CDbLevel>;
	CDbLevel();

public:
	CDbLevel(CDbLevel &Src) : CDbBase() {SetMembers(Src);}
	CDbLevel &operator= (const CDbLevel &Src) {
		Clear();
		SetMembers(Src);
		return *this;
	}

	virtual ~CDbLevel();

	DWORD			FindRoomIDAtCoords(const DWORD dwRoomX, const DWORD dwRoomY) const;
	const WCHAR *	GetAuthorText() const;
	DWORD			GetDefaultNextLevelID() const;
	CDbHold *		GetHold() const;
	CDbRoom *		GetRoomAtCoords(const DWORD dwRoomX, const DWORD dwRoomY);
	void			GetRequiredRooms(CIDList& requiredRooms);
	CDbRoom *		GetStartingRoom();
	void			GetStartingRoomCoords(DWORD &dwRoomX, DWORD &dwRoomY);
	bool			Load(const DWORD dwLevelID);
   CDbLevel*   MakeCopy(const DWORD dwNewHoldID);
   DWORD       SaveCopyOfRooms(const DWORD dwNewLevelID);
	virtual MESSAGE_ID	SetProperty(const PROPTYPE pType, char* const str,
			CImportInfo &info, bool &bSaveRecord);
	virtual bool	Update();

	CDbRooms		Rooms;
	CDbSavedGames	SavedGames;

	DWORD			dwLevelID;
	DWORD			dwHoldID;
	DWORD			dwPlayerID;
	CDbMessageText	NameText;
	CDbMessageText	DescriptionText;
	DWORD			dwRoomID;
	UINT			wX;
	UINT			wY;
	UINT			wO;
	CDate			Created;
	CDate			LastUpdated;

	bool			bGotStartingRoomCoords;
	DWORD			dwStartingRoomX;
	DWORD			dwStartingRoomY;

	DWORD			dwLevelIndex;	//GUID w/in scope of owner hold

private:
	void		Clear();
	DWORD		GetLocalID() const;
	bool		SetMembers(const CDbLevel &Src);
	bool		UpdateExisting();
	bool		UpdateNew();
};

//*****************************************************************************
class CDbLevels : public CDbVDInterface<CDbLevel>
{
protected:
	friend class CCurrentGame;
	friend class CDbHold;
	friend class CDb;
	CDbLevels()
		: CDbVDInterface<CDbLevel>("Levels", p_LevelID),
		  dwFilterByHoldID(0)
	{}

public:
	virtual ~CDbLevels() {}

	virtual void		Delete(const DWORD dwLevelID);
	virtual string	ExportXML(const DWORD dwLevelID, CDbRefs &dbRefs, const bool bRef=false);
	void		FilterBy(const DWORD dwSetFilterByHoldID);
	virtual CDbLevel *	GetNew();
   static void UpdateExitIDs(const DWORD dwLevelID, const DWORD dwNewHoldID,
         const bool bResetIDs = true);

private:
	virtual void		LoadMembership();

	DWORD		dwFilterByHoldID;
};

#endif //...#ifndef DBLEVELS_H

// $Log: DbLevels.h,v $
// Revision 1.33  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.32  2003/08/12 18:50:00  mrimer
// Moved CDbLevelS::RemoveFromHold() to CDbHold::RemoveLevel().
//
// Revision 1.31  2003/08/06 01:10:26  mrimer
// Fixed bug: level exit IDs not being updated on room copy to different hold.
//
// Revision 1.30  2003/07/16 07:37:18  mrimer
// Rewrote CDbLevel::InsertIntoHold() as CDbHold::InsertLevel().
//
// Revision 1.29  2003/07/15 00:26:31  mrimer
// Added MakeCopy().
//
// Revision 1.28  2003/07/09 21:22:57  mrimer
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.27  2003/07/09 04:55:05  schik
// Fixed unitialized variable
//
// Revision 1.26  2003/07/07 23:33:26  mrimer
// Added SaveCopyOfRooms().
//
// Revision 1.25  2003/07/03 08:02:16  mrimer
// Made dwLevelIndex public.
//
// Revision 1.24  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.23  2003/05/20 18:11:39  mrimer
// Refactored common DB ops into new virtual base class CDbVDInterface.
//
// Revision 1.22  2003/04/17 20:53:38  mrimer
// Added GetRequiredRooms().
//
// Revision 1.21  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.20  2003/02/24 17:06:25  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.19  2002/12/22 01:36:02  mrimer
// Added XML import/export support.  (Added GUID support.)
// Added assignment operator and copy constructor.
//
// Revision 1.18  2002/11/22 21:58:17  mrimer
// Made Delete() not const.
//
// Revision 1.17  2002/11/22 02:04:43  mrimer
// Added AddNextLevelID().
//
// Revision 1.16  2002/11/14 19:09:14  mrimer
// Added GetNew(), GetByID() and Update() methods.
// Moved member initialization into constructor initialization list.
// Made some parameters const.
//
// Revision 1.15  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.14  2002/07/10 03:56:21  erikh2000
// Added CDbLevels method to return IDs contained in membership.
//
// Revision 1.13  2002/06/21 22:27:24  erikh2000
// Added method to lookup message text for author of a level.
//
// Revision 1.12  2002/06/09 06:15:21  erikh2000
// Changed code to use new message text class.
// Added PlayerID field.
//
// Revision 1.11  2002/04/28 23:47:10  erikh2000
// Added member to CDbLevel that can be used to access all saved games for a level.
//
// Revision 1.10  2002/03/14 23:42:07  erikh2000
// Added CDbLevel::GetStartingRoomCoords().
//
// Revision 1.9  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.8  2001/12/16 02:09:26  erikh2000
// Allow CDb to construct CDbLevels.
//
// Revision 1.7  2001/12/08 05:16:49  erikh2000
// Added CDbRooms member to CDbLevel.
//
// Revision 1.6  2001/12/08 03:19:10  erikh2000
// Added CDbLevels::GetFirst() and CDbLevel::GetNext() methods.
//
// Revision 1.5  2001/11/03 20:16:57  erikh2000
// Removed OnPlot() and OnLoad() references.
//
// Revision 1.4  2001/10/27 04:41:59  erikh2000
// Added SetOnLoad calls.
//
// Revision 1.3  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.2  2001/10/22 23:56:04  erikh2000
// Changed GetDefaultNextLevel() to GetDefaultNextLevelID().
//
// Revision 1.1.1.1  2001/10/01 22:20:09  erikh2000
// Initial check-in.
//
