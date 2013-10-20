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

#ifndef IMPORTINFO_H
#define IMPORTINFO_H
#ifdef WIN32
#	pragma warning(disable:4786)
#endif

#include <BackEndLib/IDList.h>
#include <BackEndLib/MessageIDs.h>
#include <BackEndLib/Types.h>

#include <map>
#include <vector>
using std::vector;

typedef struct {
   DWORD dwPlayerID;
   DWORD dwHoldID;
   vector<DWORD> levelIndices;   //LevelIndex values (GIDs)
   bool bEndHoldSave;
} PlayerLevelIDs;

//Used during DB import.
//Maps old local IDs to new local IDs and stores other needed information.
typedef std::map<DWORD, DWORD> PrimaryKeyMap;
typedef vector<PlayerLevelIDs*> PlayerLevelIDList;
class CImportInfo {
public:
   CImportInfo();

   void Clear(const bool bPartialClear=false);

	PrimaryKeyMap DemoIDMap;
	PrimaryKeyMap HoldIDMap;
	PrimaryKeyMap LevelIDMap;
	PrimaryKeyMap PlayerIDMap;
	PrimaryKeyMap RoomIDMap;
	PrimaryKeyMap SavedGameIDMap;

   bool  bReplaceOldPlayers;
   bool  bReplaceOldHolds;

   enum ImportType
   {
      None=0,
      Demo,
      Hold,
      Player
   };
   ImportType  typeBeingImported;

   DWORD dwRepairHoldID;

   DWORD dwPlayerImportedID, dwHoldImportedID;
   vector<DWORD> localHoldIDs;

   PlayerLevelIDList playerLevelIDs;

   CIDList  highlightRoomIDs;

   MESSAGE_ID ImportStatus;   //result of import process
};

#endif //...#ifndef IMPORTINFO_H

// $Log: ImportInfo.h,v $
// Revision 1.4  2003/07/29 13:34:00  mrimer
// Added player import fix: UpdateHighlightDemoIDs().
//
// Revision 1.3  2003/07/25 00:00:17  mrimer
// Changed recorded level representation from IDs to GIDs.
//
// Revision 1.2  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.1  2003/07/09 21:15:41  mrimer
// Initial check-in (class definition moved from CDbBase.h).
//
