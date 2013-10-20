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
 * Portions created by the Initial Developer are Copyright (C) 2003 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//Util1_6.h
//Declarations for CUtil1_6.

#ifndef UTIL1_6_H
#define UTIL1_6_H

#include "Util.h"

#include <BackEndLib/MessageIDs.h>

#include <mk4.h>

#include <list>
#include <map>
using namespace std;

class CUtil1_6 : public CUtil
{
private:
    typedef map<string,DWORD> ASSIGNEDMIDS;
public:
	CUtil1_6(const WCHAR* pszSetPath) : CUtil(v1_6, pszSetPath) { };
	
	bool	PrintCreate(const COptionList &Options) const;
	bool	PrintDelete(const COptionList &Options) const;
	bool	PrintImport(const COptionList &Options, const WCHAR* pszSrcPath, VERSION eSrcVersion) const;

private:
    static void AddMessageText(c4_Storage &TextStorage, DWORD dwMessageID, LANGUAGE_CODE eLanguage, 
            const WCHAR *pwszText);
    bool        AssignOriginalNames(void) const;
	static bool DeleteDat(const WCHAR *pwszFilepath);
    void        GetAssignedMIDs(const WCHAR *pwzMIDFilepath, ASSIGNEDMIDS &AssignedMIDs, 
            DWORD &dwLastMessageID) const;
	void        GetHoldFilepath(WSTRING &wstrFilepath) const;
	void        GetPlayerFilepath(WSTRING &wstrFilepath) const;
	void        GetTextFilepath(WSTRING &wstrFilepath) const;
    bool        ImportBasicMessages(const WCHAR *pwzSrcPath, c4_Storage &TextStorage) const;
    bool        ImportHolds(c4_Storage &SourceStorage, c4_Storage &DestStorage, list<DWORD> &NeededMessages) const;
    bool        ImportLevels(c4_Storage &SourceStorage, c4_Storage &DestStorage, list<DWORD> &NeededMessages) const;
    bool        ImportRooms(c4_Storage &SourceStorage, c4_Storage &DestStorage, list<DWORD> &NeededMessages) const;
    bool        ImportSavedGames(c4_Storage &SourceStorage, c4_Storage &DestStorage, list<DWORD> &NeededMessages) const;
    bool        ImportDemos(c4_Storage &SourceStorage, c4_Storage &DestStorage, list<DWORD> &NeededMessages) const;
    bool        ImportPlayers(c4_Storage &SourceStorage, c4_Storage &DestStorage, list<DWORD> &NeededMessages) const;
    bool        ImportMessageTexts(c4_Storage &SourceStorage, c4_Storage &DestStorage, 
            const list<DWORD> &NeededMessages) const;
    bool        ImportUNI(const WCHAR *pwzFilepath, c4_Storage &TextStorage, const ASSIGNEDMIDS &AssignedMIDs,
            DWORD &dwLastMessageID, string &strMIDs) const;
    bool        Is1_5RoomRequired(DWORD dwRoomID) const;
};

#endif //...#ifndef UTIL1_6_H

// $Log: Util1_6.h,v $
// Revision 1.6  2003/06/22 17:49:09  erikh2000
// MIDs are given previously assigned values from existing MIDs.h.
//
// Revision 1.5  2003/05/25 22:54:59  erikh2000
// Fixed some includes.
//
// Revision 1.4  2003/05/22 23:42:15  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.3  2003/05/08 23:34:52  erikh2000
// Deleting, creating, and importing works.
//
// Revision 1.2  2003/04/28 18:04:56  erikh2000
// It compiles now.
//
// Revision 1.1  2003/04/28 14:25:41  erikh2000
// Initial check-in.
//
