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

//Declares for DROD database.  Snapshot from distributed 1.5 format. (not to be confused with
//format that evolved between 1.5 and 1.6)

#ifndef DBPROPS15_H
#define DBPROPS15_H

#include <mk4.h>

//*****************************************************************************
#define DEFPROP(c,n) static c p_##n(#n)
#define DEFTDEF(n,v) static const char n[] = v

namespace ns1_5
{

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
DEFPROP(c4_IntProp,		ExitLevelID);
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
DEFPROP(c4_IntProp,		ProcessSequence);
DEFPROP(c4_FloatProp,	RequiredDRODVersion);
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
DEFPROP(c4_IntProp,		PlayerID);
DEFPROP(c4_IntProp,		X);
DEFPROP(c4_IntProp,		Y);

//View props.
DEFPROP(c4_ViewProp,	ConqueredRooms);
DEFPROP(c4_ViewProp,	ExploredRooms);
DEFPROP(c4_ViewProp,	Mimics);
DEFPROP(c4_ViewProp,	Monsters);
DEFPROP(c4_ViewProp,	NextLevels);
DEFPROP(c4_ViewProp,	OrbAgents);
DEFPROP(c4_ViewProp,	Orbs);
DEFPROP(c4_ViewProp,	Scrolls);
DEFPROP(c4_ViewProp,	SpecialExits);

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
			"SpecialExits"
			"["
				"ExitLevelID:I,"
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
			"RequiredDRODVersion:F,"
			"NextLevels"
			"["
				"LevelID:I"
			"]"
		"]");

DEFTDEF(HOLDS_VIEWDEF,
		"Holds"
		"["
			"HoldID:I,"
			"NameMessageID:I,"
			"DescriptionMessageID:I,"
			"LevelID:I"
		"]");

DEFTDEF(PLAYERS_VIEWDEF,
		"Players"
		"["
			"PlayerID:I,"
			"IsLocal:I,"
			"NameMessageID:I,"
			"EMailMessageID:I,"
			"Settings:B"
		"]");

}; //..namespace ns1_5

#undef DEFPROP
#undef DEFTDEF

#endif //...#ifndef DBPROPS15_H

// $Log: dbprops1_5.h,v $
// Revision 1.1  2003/05/29 02:59:29  erikh2000
// Moved from drodutil dir.
//
// Revision 1.1  2003/05/09 02:02:26  erikh2000
// Initial commit.
//
