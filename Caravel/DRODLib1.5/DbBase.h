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

//DbBase.h
//Declarations for CDbBase.
//Class for handling common database functionality.

//Note: There is some 1.6 code in this module that has been revised to work with 1.5.
//The gist of it is that whenever a player or text database is referred to, the hold database 
//(containing player and text) is used.  Sorry for the silliness. -Erik

#ifndef DBBASE_H
#define DBBASE_H

#include "Assert.h"
#include "DbProps.h"
#include "Types.h"
#include "Wchar.h"
#include "MessageIDs.h"

#include <mk4.h>

#pragma warning(disable: 4786)
#include <map>

const unsigned long ROW_NO_MATCH = (unsigned long) -1;

//For data import: map old local IDs to new local IDs.
typedef std::map<DWORD, DWORD> PrimaryKeyMap;
typedef struct {
	PrimaryKeyMap DemoIDMap;
	PrimaryKeyMap HoldIDMap;
	PrimaryKeyMap LevelIDMap;
	PrimaryKeyMap PlayerIDMap;
	PrimaryKeyMap RoomIDMap;
	PrimaryKeyMap SavedGameIDMap;
} PrimaryKeyMaps;

class CDbBase
{
public:
	CDbBase(void);
	virtual	~CDbBase(void);

	DWORD					AddMessageText(const WCHAR *pwczText);
	void					ChangeMessageText(const DWORD dwMessageID, const WCHAR *pwczText);
	void					Close(void);
	static void				Commit(void);
	static c4_View			CreateHDbViewDef(const char *pszViewDef);
	static c4_View			CreatePDbViewDef(const char *pszViewDef);
	static c4_View			CreateTDbViewDef(const char *pszViewDef);
	static void				DeleteMessage(const DWORD dwMessageID);
	static void				FlushWrites(void) {Commit();}
	WCHAR *					GetAllocMessageText(MESSAGE_ID dwMessageID, DWORD *pdwLen = NULL) const;
	static DWORD			GetIncrementedID(const c4_IntProp &propID);
	LANGUAGE_CODE			GetLanguage(void) const;
	WCHAR *					GetMessageText(const MESSAGE_ID dwMessageID, DWORD *pdwLen = NULL);
		static bool				IsOpen(void);
	static c4_ViewRef		GetView(const char *pszViewName);
	MESSAGE_ID				Open(const char *pszPath = NULL);
	static void				Rollback(void);
	void					SetLanguage(LANGUAGE_CODE eSetLanguageCode);
	virtual MESSAGE_ID		SetProp(const PROPTYPE pType, char* const str,
			PrimaryKeyMaps &Maps, bool &bSaveRecord) {return false;}
	virtual MESSAGE_ID		SetProp(const VIEWPROPTYPE vpType, const PROPTYPE pType,
			char* const str, PrimaryKeyMaps &Maps) {return false;}
	virtual bool			Update(void)=0;

	static DWORD		LookupRowByPrimaryKey(const DWORD dwID, c4_IntProp &IDProp, 
									c4_View &View);

private:
	DWORD	FindMessageText(const DWORD dwMessageID,c4_View &MessageTextsView) const;
	WCHAR *	SetLastMessageText(const WCHAR *pwczNewMessageText, const DWORD dwNewMessageTextLen);

	WCHAR *	pwczLastMessageText;

	PREVENT_DEFAULT_COPY(CDbBase);
};

DWORD GetDbRefCount(void);

//Debugging macros to check for a reference count change.  Put BEGIN_DBREFCOUNT_CHECK in
//front of code that creates CDbBase-derived objects, and END_DBREFCOUNT_CHECK at the end
//of the code.  They both need to be in the same scope.
#ifdef _DEBUG
	#define BEGIN_DBREFCOUNT_CHECK		DWORD dwStartingDbRefCount = GetDbRefCount()
	#define END_DBREFCOUNT_CHECK		ASSERT(GetDbRefCount() == dwStartingDbRefCount)
#else
	#define BEGIN_DBREFCOUNT_CHECK		NULL
	#define END_DBREFCOUNT_CHECK		NULL
#endif

#endif //...#ifndef DBBASE_H

// $Log: DbBase.h,v $
// Revision 1.1  2003/02/25 00:01:26  erikh2000
// Initial check-in.
//
// Revision 1.21  2003/02/24 19:01:44  erikh2000
// Added methods for creating viewdefs.
//
// Revision 1.20  2003/02/24 17:00:18  erikh2000
// Wrote code to handle data in three databases instead of one.  Code is temporarily commented to only use one database for now.
//
// Revision 1.19  2003/02/17 00:48:11  erikh2000
// Remove L" string literals.
//
// Revision 1.18  2002/12/22 01:40:47  mrimer
// Added XML import (parsing) support.  Added PrimaryKeyMaps struct.
//
// Revision 1.17  2002/11/13 23:12:24  mrimer
// Moved GetPlayerID() and SetPlayerID() to CDb.
// Made DeleteMessage() static.
// Made some parameters const.
//
// Revision 1.16  2002/10/17 16:47:37  mrimer
// Added FlushWrites().  Made class methods static.
//
// Revision 1.15  2002/09/05 20:15:37  mrimer
// Made LookupRowByPrimaryKey() unprotected.
//
// Revision 1.14  2002/07/22 01:56:54  erikh2000
// Removed an incorrect comment.
//
// Revision 1.13  2002/07/05 17:52:54  mrimer
// Minor fixes.
//
// Revision 1.12  2002/06/16 06:20:53  erikh2000
// Added method to delete a message.
//
// Revision 1.11  2002/06/15 18:24:37  erikh2000
// Added new Commit() method.
// Moved some static object members to module-scope variables for clarity.
// Changed code so that storage is not committed after each database write.
//
// Revision 1.10  2002/06/09 06:10:27  erikh2000
// Added ChangeMessageText() method.
//
// Revision 1.9  2002/06/05 03:03:09  mrimer
// Added includes.
//
// Revision 1.8  2002/05/21 21:29:10  erikh2000
// Added stub methods for getting and setting current player.
//
// Revision 1.7  2002/04/09 01:03:14  erikh2000
// Twiddling.
//
// Revision 1.6  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.5  2002/02/28 04:53:06  erikh2000
// Added CDbBase::GetLanguage().
//
// Revision 1.4  2001/12/08 01:39:42  erikh2000
// Added PREVENT_DEFAULT_COPY() macro to class declaration.
//
// Revision 1.3  2001/11/20 00:52:48  erikh2000
// Added CDbBase::GetIncrementedID() and CDbBase::AddMessageText().
//
// Revision 1.2  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.1.1.1  2001/10/01 22:20:07  erikh2000
// Initial check-in.
//
