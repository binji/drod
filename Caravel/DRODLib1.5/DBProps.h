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
 * Michael Welsh Duggan (md5i), JP Burford (jpburford)
 *
 * ***** END LICENSE BLOCK ***** */

//Declares for DROD database.

#ifndef DBPROPS_H
#define DBPROPS_H

#include <mk4.h>

//*****************************************************************************
#define DEFPROP(c,n) static c p_##n(#n)
#define DEFTDEF(n,v) static const char n[] = v

//Props.
DEFPROP(c4_IntProp,		BeginTurnNo);
DEFPROP(c4_IntProp,		Bottom);
DEFPROP(c4_IntProp,		Checksum);
DEFPROP(c4_IntProp,		CheckpointX);
DEFPROP(c4_IntProp,		CheckpointY);
DEFPROP(c4_BytesProp,	Commands);
DEFPROP(c4_IntProp,		Created);
DEFPROP(c4_IntProp,		DemoID);
DEFPROP(c4_IntProp,		DescriptionMessageID);
DEFPROP(c4_IntProp,		EMailMessageID);
DEFPROP(c4_IntProp,		EndTurnNo);
DEFPROP(c4_IntProp,		GID_Created);
DEFPROP(c4_IntProp,		GID_LevelIndex);
DEFPROP(c4_IntProp,		GID_NewLevelIndex);
DEFPROP(c4_IntProp,		GID_OriginalNameMessageID);
DEFPROP(c4_IntProp,		GID_PlayerID);
DEFPROP(c4_IntProp,		HoldID);
DEFPROP(c4_IntProp,		IsFirstTurn);
DEFPROP(c4_IntProp,		IsHidden);
DEFPROP(c4_IntProp,		IsLocal);
DEFPROP(c4_IntProp,		LanguageCode);
DEFPROP(c4_IntProp,		LastUpdated);
DEFPROP(c4_IntProp,		Left);
DEFPROP(c4_IntProp,		LevelID);
DEFPROP(c4_BytesProp,	ExtraVars);
DEFPROP(c4_IntProp,		MessageID);
DEFPROP(c4_BytesProp,	MessageText);
DEFPROP(c4_IntProp,		MessageTextID);
DEFPROP(c4_IntProp,		NameMessageID);
DEFPROP(c4_IntProp,		NextDemoID);
DEFPROP(c4_IntProp,		O);
DEFPROP(c4_IntProp,		PlayerID);
DEFPROP(c4_IntProp,		ProcessSequence);
DEFPROP(c4_IntProp,		RequiredDRODVersion);
DEFPROP(c4_IntProp,		Right);
DEFPROP(c4_IntProp,		RoomID);
DEFPROP(c4_IntProp,		RoomCols);
DEFPROP(c4_IntProp,		RoomRows);
DEFPROP(c4_IntProp,		RoomsNeededToComplete);
DEFPROP(c4_IntProp,		RoomX);
DEFPROP(c4_IntProp,		RoomY);
DEFPROP(c4_IntProp,		SavedGameID);
DEFPROP(c4_BytesProp,	Settings);
DEFPROP(c4_IntProp,		ShowSequenceNo);
DEFPROP(c4_BytesProp,	Squares);
DEFPROP(c4_IntProp,		StartRoomO);
DEFPROP(c4_IntProp,		StartRoomX);
DEFPROP(c4_IntProp,		StartRoomY);
DEFPROP(c4_IntProp,		Style);
DEFPROP(c4_IntProp,		Top);
DEFPROP(c4_IntProp,		Type);
DEFPROP(c4_IntProp,		X);
DEFPROP(c4_IntProp,		Y);

//View props.
DEFPROP(c4_ViewProp,	ConqueredRooms);
DEFPROP(c4_ViewProp,	ExploredRooms);
DEFPROP(c4_ViewProp,	Monsters);
DEFPROP(c4_ViewProp,	OrbAgents);
DEFPROP(c4_ViewProp,	Orbs);
DEFPROP(c4_ViewProp,	Scrolls);
DEFPROP(c4_ViewProp,	Exits);

//View definitions.
DEFTDEF(INCREMENTEDIDS_VIEWDEF,
		"IncrementedIDs["
			"DemoID:I,"
			"LevelID:I,"
			"HoldID:I,"
			"MessageID:I,"
			"MessageTextID:I,"
			"RoomID:I,"
			"SavedGameID:I,"
			"PlayerID:I"
		"]");

DEFTDEF(ROOMS_VIEWDEF,
		"Rooms["
			"RoomID:I,"
			"LevelID:I,"
			"RoomX:I,"
			"RoomY:I,"
			"RoomCols:I,"
			"RoomRows:I,"
			"Style:I,"
			"Squares:B,"
			"Orbs"
			"["
				"X:I,"
				"Y:I,"
				"OrbAgents"
				"["
					"Type:I,"
					"X:I,"
					"Y:I"
				"]"
			"],"
			"Monsters"
			"["
				"Type:I,"
				"X:I,"
				"Y:I,"
				"O:I,"
				"IsFirstTurn:I,"
				"ProcessSequence:I,"
				"ExtraVars:B"
			"],"
			"Scrolls"
			"["
				"X:I,"
				"Y:I,"
				"MessageID:I"
			"],"
			"Exits"
			"["
				"LevelID:I,"
				"Left:I,"
				"Right:I,"
				"Top:I,"
				"Bottom:I"
			"]"
		"]");

DEFTDEF(SAVEDGAMES_VIEWDEF,
		"SavedGames"
		"["
			"SavedGameID:I,"
			"PlayerID:I,"
			"RoomID:I,"
			"Type:I,"
			"CheckpointX:I,"
			"CheckpointY:I,"
			"IsHidden:I,"
			"LastUpdated:I,"
			"StartRoomX:I,"
			"StartRoomY:I,"
			"StartRoomO:I,"
			"ExploredRooms"
			"["
				"RoomID:I"
			"],"
			"ConqueredRooms"
			"["
				"RoomID:I"
			"],"
			"Created:I,"
			"Commands:B"
		"]");

DEFTDEF(DEMOS_VIEWDEF,
		"Demos["
			"DemoID:I,"
			"SavedGameID:I,"
			"IsHidden:I,"
			"DescriptionMessageID:I,"
			"ShowSequenceNo:I,"
			"BeginTurnNo:I,"
			"EndTurnNo:I,"
			"NextDemoID:I,"
			"Checksum:I"
		"]");

DEFTDEF(MESSAGETEXTS_VIEWDEF,
		"MessageTexts"
		"["
			"MessageTextID:I,"
			"MessageID:I,"
			"LanguageCode:I,"
			"MessageText:B"
		"]");

DEFTDEF(LEVELS_VIEWDEF,
		"Levels"
		"["
			"LevelID:I,"
			"HoldID:I,"
			"PlayerID:I,"
			"NameMessageID:I,"
			"DescriptionMessageID:I,"
			"RoomID:I,"
			"X:I,"
			"Y:I,"
			"O:I,"
			"Created:I,"
			"LastUpdated:I,"
			"RoomsNeededToComplete:I,"
			"GID_LevelIndex:I,"
		"]");

DEFTDEF(HOLDS_VIEWDEF,
		"Holds"
		"["
			"HoldID:I,"
			"NameMessageID:I,"
			"DescriptionMessageID:I,"
			"LevelID:I,"
			"GID_Created:I,"
			"LastUpdated:I,"
			"GID_PlayerID:I,"
			"RequiredDRODVersion:I,"
			"GID_NewLevelIndex:I,"
		"]");

DEFTDEF(PLAYERS_VIEWDEF,
		"Players"
		"["
			"PlayerID:I,"
			"IsLocal:I,"
			"NameMessageID:I,"
			"EMailMessageID:I,"
			"GID_OriginalNameMessageID:I,"
			"GID_Created:I,"
			"LastUpdated:I,"
			"Settings:B"
		"]");

#undef DEFPROP
#undef DEFTDEF

//*****************************************************************************
//Used for importing data into the DB.

//ATTENTION:
//Must keep the values below synched with those above.

//All view definitions in the DB.
enum VIEWTYPE
{
	V_First=0,
	V_Demos=V_First,
	V_Holds,
	V_Levels,
	V_MessageTexts,
	V_Players,
	V_Rooms,
	V_SavedGames,
	V_Count,
	V_Invalid
};

//All viewprops in the DB, that aren't just a simple list
enum VIEWPROPTYPE
{
	VP_First=0,
	VP_Monsters=VP_First,
	VP_OrbAgents,
	VP_Orbs,
	VP_Scrolls,
	VP_Exits,
	VP_Count,
	VP_Invalid
};

//All fields in DB views,
//	and viewprops that are simple lists
enum PROPTYPE
{
	P_First=0,
	P_BeginTurnNo=P_First,
	P_Bottom,
	P_Checksum,
	P_CheckpointX,
	P_CheckpointY,
	P_Commands,
	P_ConqueredRooms,	//list
	P_Created,
	P_DemoID,
	P_DescriptionMessage,	//not ID
	P_EMailMessage,			//not ID
	P_EndTurnNo,
	P_ExitLevelID,
	P_ExploredRooms,	//list
	P_GID_Created,
	P_GID_LevelIndex,
	P_GID_NewLevelIndex,
	P_GID_OriginalNameMessage,	//not ID
	P_GID_PlayerID,
	P_HoldID,
	P_IsFirstTurn,
	P_IsHidden,
	P_IsLocal,
	P_LanguageCode,
	P_LastUpdated,
	P_Left,
	P_LevelID,
	P_ExtraVars,
	P_Message,		//not ID
	P_MessageText,
	P_MessageTextID,
	P_NameMessage,	//not ID
	P_NextDemoID,
	P_O,
	P_PlayerID,
	P_ProcessSequence,
	P_RequiredDRODVersion,
	P_Right,
	P_RoomID,
	P_RoomCols,
	P_RoomRows,
	P_RoomsNeededToComplete,
	P_RoomX,
	P_RoomY,
	P_SavedGameID,
	P_Settings,
	P_ShowSequenceNo,
	P_Squares,
	P_StartRoomO,
	P_StartRoomX,
	P_StartRoomY,
	P_Style,
	P_Top,
	P_Type,
	P_X,
	P_Y,
	P_Count,
	P_Invalid,
	P_Start,	//start of viewprop
	P_End		//end of viewprop
};

//*****************************************************************************
const char viewTypeStr[V_Count][13] = {
	"Demos", "Holds", "Levels", "MessageTexts", "Players", "Rooms", "SavedGames"
};

const char viewpropTypeStr[VP_Count][15] = {
	"Monsters",	"OrbAgents", "Orbs", "Scrolls", "Exits"
};

const char propTypeStr[P_Count][26] = {
	"BeginTurnNo","Bottom", "Checksum", "CheckpointX", "CheckpointY",
	"Commands", "ConqueredRooms", "Created", "DemoID", "DescriptionMessage",
	"EMailMessage", "EndTurnNo", "ExitLevelID", "ExploredRooms",
	"GID_Created", "GID_LevelIndex",
	"GID_NewLevelIndex", "GID_OriginalNameMessage", "GID_PlayerID",
	"HoldID", "IsFirstTurn", "IsHidden",
	"IsLocal", "LanguageCode", "LastUpdated", "Left", "LevelID",
	"ExtraVars", "Message", "MessageText", "MessageTextID",
	"NameMessage", "NextDemoID", "O", "PlayerID",
	"ProcessSequence", "RequiredDRODVersion", "Right", "RoomID", "RoomCols",
	"RoomRows", "RoomsNeededToComplete", "RoomX", "RoomY", "SavedGameID",
	"Settings", "ShowSequenceNo", "Squares", "StartRoomO", "StartRoomX",
	"StartRoomY", "Style", "Top", "Type", "X", "Y"
};

inline const char* ViewTypeStr(const VIEWTYPE vType)
//Returns: string corresponding to enumeration
{
	return viewTypeStr[vType];
}

inline const char* ViewpropTypeStr(const VIEWPROPTYPE vpType)
//Returns: string corresponding to enumeration
{
	return viewpropTypeStr[vpType];
}

inline const char* PropTypeStr(const PROPTYPE pType)
//Returns: string corresponding to enumeration
{
	return propTypeStr[pType];
}

#endif //...#ifndef DBPROPS_H

// $Log: DBProps.h,v $
// Revision 1.1  2003/02/25 00:01:21  erikh2000
// Initial check-in.
//
// Revision 1.17  2002/12/22 01:32:54  mrimer
// Added XML import parsing vars.
// Added GID props (for GUID construction).
// Removed level exit support from Levels view (i.e. removed NextLevels subview).
// Removed unneeded Mimics subview.
//
// Revision 1.16  2002/07/05 17:59:34  mrimer
// Minor fixes (includes, etc.)
//
// Revision 1.15  2002/06/20 04:04:19  erikh2000
// Removed "ShowFromIntro" field.
//
// Revision 1.14  2002/06/09 06:05:21  erikh2000
// Added Players table and restructured some other tables to point to it with a foreign key.
//
// Revision 1.13  2002/05/21 21:27:43  erikh2000
// Added new tables for Players and Settings.
// Added PlayerID field to some tables.
//
// Revision 1.12  2002/03/04 22:22:52  erikh2000
// Added "CheckpointX" and "CheckpointY" properties and changed "SavedGames" viewdef to include them.
//
// Revision 1.11  2002/02/28 04:52:48  erikh2000
// Added "Type" field to "SavedGames" viewdef.
//
// Revision 1.10  2002/02/10 02:36:49  erikh2000
// Removed Description property from SavedGames viewdef since it is not used.
// Removed "X", "Y", "O", "IsPlacingMimic", "MimicCursorX", "MimicCursorY", "IsVisible", and "TurnsTaken" properties from SavedGames viewdef because they can all be determined by playing back Commands.
//
// Revision 1.9  2002/02/07 22:33:49  erikh2000
// Removed "IsFirstTurn" field from "SavedGames" viewdef because it can be determined from "TurnsTaken" field.
//
// Revision 1.8  2001/12/16 02:10:10  erikh2000
// Added Checksum property to Demos viewdef.
//
// Revision 1.7  2001/11/20 00:52:18  erikh2000
// Changed fields in Demos and SavedGames viewdefs.
//
// Revision 1.6  2001/11/08 11:35:09  erikh2000
// Added new properties for mimic cursor to SavedGames def.  (Committed on behalf of jpburford.)
//
// Revision 1.5  2001/10/25 05:04:16  md5i
// Used proper turn counter.  Changed egg hatching to CueEvent.  Fixed attributions.
//
// Revision 1.4  2001/10/24 01:42:41  md5i
// Fix typo.
//
// Revision 1.3  2001/10/20 05:45:58  erikh2000
// Removed p_TileNo and p_StartRoomTileNo props and fields.
//
// Revision 1.2  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.1.1.1  2001/10/01 22:20:11  erikh2000
// Initial check-in.
//
