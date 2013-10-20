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

//DbRefs.cpp
//Implementation of CDbRefs.

#include "DbRefs.h"
#include <memory.h>

//
//CDbRefs public methods.
//

//*****************************************************************************
CDbRefs::CDbRefs(void)
//Constructor.
{
	//Allocate each member to size of elements in that DB view.

	c4_View DemosView = GetView(ViewTypeStr(V_Demos));
	const DWORD dwDemoCount = DemosView.GetSize();
	this->pbDemos = new bool[dwDemoCount];
	memset(this->pbDemos, false, dwDemoCount * sizeof(bool));

	c4_View HoldsView = GetView(ViewTypeStr(V_Holds));
	const DWORD dwHoldCount = HoldsView.GetSize();
	this->pbHolds = new bool[dwHoldCount];
	memset(this->pbHolds, false, dwHoldCount * sizeof(bool));

	c4_View LevelsView = GetView(ViewTypeStr(V_Levels));
	const DWORD dwLevelCount = LevelsView.GetSize();
	this->pbLevels = new bool[dwLevelCount];
	memset(this->pbLevels, false, dwLevelCount * sizeof(bool));

	c4_View PlayersView = GetView(ViewTypeStr(V_Players));
	const DWORD dwPlayerCount = PlayersView.GetSize();
	this->pbPlayers = new bool[dwPlayerCount];
	memset(this->pbPlayers, false, dwPlayerCount * sizeof(bool));

	c4_View RoomsView = GetView(ViewTypeStr(V_Rooms));
	const DWORD dwRoomCount = RoomsView.GetSize();
	this->pbRooms = new bool[dwRoomCount];
	memset(this->pbRooms, false, dwRoomCount * sizeof(bool));

	c4_View SavedGameView = GetView(ViewTypeStr(V_SavedGames));
	const DWORD dwSavedGameCount = SavedGameView.GetSize();
	this->pbSavedGames = new bool[dwSavedGameCount];
	memset(this->pbSavedGames, false, dwSavedGameCount * sizeof(bool));
}

//*****************************************************************************
CDbRefs::~CDbRefs(void)
//Destructor.
{
	delete[] this->pbDemos;
	delete[] this->pbHolds;
	delete[] this->pbLevels;
	delete[] this->pbPlayers;
	delete[] this->pbRooms;
	delete[] this->pbSavedGames;
}

//*****************************************************************************
bool CDbRefs::IsSet(
//Returns: whether view row key has been marked
	const VIEWTYPE vType,	//(in) DB view
	const DWORD dwID)		//(in) view row key
const
{	
	c4_View View = GetView(ViewTypeStr(vType));
	switch (vType)
	{
		case V_Demos:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_DemoID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Demos--bad ID."); return false;}
			return this->pbDemos[dwRowI];
		}
		case V_Holds:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_HoldID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Holds--bad ID."); return false;}
			return this->pbHolds[dwRowI];
		}
		case V_Levels:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_LevelID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Levels--bad ID."); return false;}
			return this->pbLevels[dwRowI];
		}
		case V_Players:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_PlayerID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Players--bad ID."); return false;}
			return this->pbPlayers[dwRowI];
		}
		case V_Rooms:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_RoomID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Rooms--bad ID."); return false;}
			return this->pbRooms[dwRowI];
		}
		case V_SavedGames:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_SavedGameID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_SavedGames--bad ID."); return false;}
			return this->pbSavedGames[dwRowI];
		}
		default:
			ASSERTP(false, "Unexpected view type.");
			return false;
	}
}

//*****************************************************************************
void CDbRefs::Set(
//Pre-cond: index is valid for view
//
//Set index when record at that row is exported/imported
//(or has a reference imported/exported).
	const VIEWTYPE vType,	//(in) DB view
	const DWORD dwID)		//(in) view row index
{	
	c4_View View = GetView(ViewTypeStr(vType));
	switch (vType)
	{
		case V_Demos:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_DemoID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Demos set--bad ID."); return;}
			this->pbDemos[dwRowI] = true;
			break;
		}
		case V_Holds:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_HoldID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Holds set--bad ID."); return;}
			this->pbHolds[dwRowI] = true;
			break;
		}
		case V_Levels:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_LevelID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Levels set--bad ID."); return;}
			this->pbLevels[dwRowI] = true;
			break;
		}
		case V_Players:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_PlayerID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Players set--bad ID."); return;}
			this->pbPlayers[dwRowI] = true;
			break;
		}
		case V_Rooms:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_RoomID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_Rooms set--bad ID."); return;}
			this->pbRooms[dwRowI] = true;
			break;
		}
		case V_SavedGames:
		{
			const DWORD dwRowI = LookupRowByPrimaryKey(dwID, p_SavedGameID, View);
			if (dwRowI == ROW_NO_MATCH) {ASSERTP(false, "V_SavedGames set--bad ID."); return;}
			this->pbSavedGames[dwRowI] = true;
			break;
		}
		default:
			ASSERTP(false, "Unexpected view type.");
	}
}

// $Log: DbRefs.cpp,v $
// Revision 1.5  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.4  2003/07/19 02:39:30  mrimer
// Made DB view access more robust and maintainable.
//
// Revision 1.3  2003/06/09 19:26:51  mrimer
// Changed some comments.
//
// Revision 1.2  2003/02/24 17:06:26  erikh2000
// Replaced calls to obsolete CDbBase methods with calls to new methods.
//
// Revision 1.1  2002/12/22 01:47:46  mrimer
// Initial check-in.
//
