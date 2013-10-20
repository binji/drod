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

#include "ImportInfo.h"
#include "../Texts/MIDs.h"

//*****************************************************************************
CImportInfo::CImportInfo()
   : bReplaceOldPlayers(false), bReplaceOldHolds(false)
   , typeBeingImported(None)
   , dwRepairHoldID(0)
   , dwPlayerImportedID(0), dwHoldImportedID(0)
   , ImportStatus(MID_ImportSuccessful)
{}

//*****************************************************************************
void CImportInfo::Clear(
//
//Params:
   const bool bPartialClear)  //set this to true when import was
                              //interrupted and will be performed again
{
   this->DemoIDMap.clear();
   this->HoldIDMap.clear();
   this->LevelIDMap.clear();
   this->PlayerIDMap.clear();
   this->RoomIDMap.clear();
   this->SavedGameIDMap.clear();

   this->dwRepairHoldID = 0;
   //Leave the following intact for query following import
   //this->dwPlayerImportedID = this->dwHoldImportedID = 0;

   if (!bPartialClear)
   {
      this->bReplaceOldHolds = false;
      this->bReplaceOldPlayers = false;
      this->typeBeingImported = None;

      this->localHoldIDs.clear();

      //Iterate through the vector and free each record.
      for (PlayerLevelIDList::iterator iter=this->playerLevelIDs.begin();
            iter!=this->playerLevelIDs.end(); ++iter)
      {
         (*iter)->levelIndices.clear();
         delete *iter;
      }
      this->playerLevelIDs.clear();
   }
}

// $Log: ImportInfo.cpp,v $
// Revision 1.3  2003/07/25 00:00:17  mrimer
// Changed recorded level representation from IDs to GIDs.
//
// Revision 1.2  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.1  2003/07/09 21:15:41  mrimer
// Initial check-in (class definition moved from CDbBase.h).
//
