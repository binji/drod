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

//MessageIDs.h
//Message ID constants.

#ifndef MESSAGEIDS_H
#define MESSAGEIDS_H

typedef enum tagLanguageCodeEnum
{
	English = 1
} LANGUAGE_CODE;

typedef unsigned long MESSAGE_ID;

//Message IDs without messages. (0 - 999)
const MESSAGE_ID MID_Success = 0; //Success return code--no message is associated with this.

const MESSAGE_ID MID_DatMissing = 1;		//These messages are not stored in database,
const MESSAGE_ID MID_DatNoAccess = 2;		//because it would be impossible to access them
const MESSAGE_ID MID_CouldNotOpenDB = 3;	//in the conditions they indicate.

//Initialization error messages (1000 - 1999)
const MESSAGE_ID MID_SDLInitFailed = 1000;
const MESSAGE_ID MID_OutOfMemory = 1001;
const MESSAGE_ID MID_CouldNotLoadResources = 1002;
const MESSAGE_ID MID_LoadGameFailed = 1003;
const MESSAGE_ID MID_TTFInitFailed = 1004;

//Hold/player selection screens.
const MESSAGE_ID MID_HighResWarning = 1005;
const MESSAGE_ID MID_SelectHoldPrompt = 1006;
const MESSAGE_ID MID_SelectPlayerPrompt = 1007;
const MESSAGE_ID MID_DeletePlayerPrompt = 1008;
const MESSAGE_ID MID_NewPlayerPrompt = 1009;
const MESSAGE_ID MID_NewPlayer = 1010;
const MESSAGE_ID MID_DeletePlayer = 1011;
const MESSAGE_ID MID_Import = 1012;
const MESSAGE_ID MID_ImportPath = 1013;
const MESSAGE_ID MID_Importing = 1014;
const MESSAGE_ID MID_ImportSuccessful = 1015;
const MESSAGE_ID MID_FileNotFound = 1016;
const MESSAGE_ID MID_FileCorrupted = 1017;
const MESSAGE_ID MID_HoldNotFound = 1018;
const MESSAGE_ID MID_LevelNotFound = 1019;
const MESSAGE_ID MID_SavePlayerPath = 1020;
const MESSAGE_ID MID_PlayerFileSaved = 1021;
const MESSAGE_ID MID_PlayerFileNotSaved = 1022;
const MESSAGE_ID MID_SaveHoldPath = 1023;
const MESSAGE_ID MID_HoldFileSaved = 1024;
const MESSAGE_ID MID_HoldFileNotSaved = 1025;
const MESSAGE_ID MID_Exporting = 1026;

//Game screen text (2000 - 2999).
const MESSAGE_ID MID_UnexpectedNeatherDeath = 2000;
const MESSAGE_ID MID_PleaseSpareMe = 2001;
const MESSAGE_ID MID_DemoSaved = 2002;
const MESSAGE_ID MID_DemoNotSaved = 2003;
const MESSAGE_ID MID_NoDemosForRoom = 2004;
const MESSAGE_ID MID_LevelComplete = 2005;
const MESSAGE_ID MID_RecordingStatus = 2006;
const MESSAGE_ID MID_DescribeDemo = 2007;
const MESSAGE_ID MID_CurrentGameStats = 2010;
const MESSAGE_ID MID_MovesMade = 2011;
const MESSAGE_ID MID_SpawnCounter = 2012;
const MESSAGE_ID MID_MonstersKilled = 2013;

//Text used on more than one screen. (3000 - 3499)
const MESSAGE_ID MID_Yes = 3000;					
const MESSAGE_ID MID_No = 3001;					
const MESSAGE_ID MID_Okay = 3002;
const MESSAGE_ID MID_OkayNoHotkey = 3003;
const MESSAGE_ID MID_Cancel = 3004;
const MESSAGE_ID MID_CancelNoHotkey = 3005;
const MESSAGE_ID MID_ReallyQuit = 3006;
const MESSAGE_ID MID_Help = 3007;
const MESSAGE_ID MID_PressAnyKeyToContinue = 3008;
const MESSAGE_ID MID_ConquerDemoDescription = 3009;
const MESSAGE_ID MID_DieDemoDescription = 3010;
const MESSAGE_ID MID_NoText = 3011;
const MESSAGE_ID MID_MessageLoadError = 3012;

//File access
const MESSAGE_ID MID_CurrentDirectory = 3050;
const MESSAGE_ID MID_FileName = 3051;
const MESSAGE_ID MID_FileType = 3052;

//Dates
const MESSAGE_ID MID_January = 3100;
const MESSAGE_ID MID_February = 3101;
const MESSAGE_ID MID_March = 3102;
const MESSAGE_ID MID_April = 3103;
const MESSAGE_ID MID_May = 3104;
const MESSAGE_ID MID_June = 3105;
const MESSAGE_ID MID_July = 3106;
const MESSAGE_ID MID_August = 3107;
const MESSAGE_ID MID_September = 3108;
const MESSAGE_ID MID_October = 3109;
const MESSAGE_ID MID_November = 3119;
const MESSAGE_ID MID_December = 3120;

//Level start screen text. (3500 - 3999)
const MESSAGE_ID MID_LevelCreated = 3500;
const MESSAGE_ID MID_LevelBy = 3501;

//Restore game screen text. (4000 - 4499)
const MESSAGE_ID MID_RestoreGame = 4000;
const MESSAGE_ID MID_UnconqueredRooms = 4001;
const MESSAGE_ID MID_Restore = 4002;
const MESSAGE_ID MID_RoomStart = 4003;
const MESSAGE_ID MID_LevelStart = 4004;
const MESSAGE_ID MID_GameStart = 4005;
const MESSAGE_ID MID_ChooseLevel = 4006;
const MESSAGE_ID MID_ChooseRoom = 4007;
const MESSAGE_ID MID_ChoosePosition = 4008;

//Demos screen text. (4500 - 4999)
const MESSAGE_ID MID_Demos = 4500;
const MESSAGE_ID MID_Watch = 4501;
const MESSAGE_ID MID_Delete = 4502;
const MESSAGE_ID MID_Export = 4503;
const MESSAGE_ID MID_ReturnToGame = 4504;
const MESSAGE_ID MID_Author = 4505;
const MESSAGE_ID MID_Created = 4506;
const MESSAGE_ID MID_Description = 4507;
const MESSAGE_ID MID_ReallyDeleteDemo = 4508;
const MESSAGE_ID MID_DemosFor = 4509;
const MESSAGE_ID MID_Details = 4510;
const MESSAGE_ID MID_NoDemoSpecified = 4511;
const MESSAGE_ID MID_DemoFileSaved = 4512;
const MESSAGE_ID MID_DemoFileNotSaved = 4513;
const MESSAGE_ID MID_DisplayDemo = 4514;
const MESSAGE_ID MID_SaveDemoPath = 4515;

//Settings screen text. (5000 - 5499)
const MESSAGE_ID MID_Settings = 5000;
const MESSAGE_ID MID_Commands = 5001;
const MESSAGE_ID MID_MoveNorthwest = 5002;
const MESSAGE_ID MID_MoveNorth = 5003;
const MESSAGE_ID MID_MoveNortheast = 5004;
const MESSAGE_ID MID_MoveWest = 5005;
const MESSAGE_ID MID_Wait = 5006;
const MESSAGE_ID MID_MoveEast = 5007;
const MESSAGE_ID MID_MoveSouthwest = 5008;
const MESSAGE_ID MID_MoveSouth = 5009;
const MESSAGE_ID MID_MoveSoutheast = 5010;
const MESSAGE_ID MID_SwingClockwise = 5011;
const MESSAGE_ID MID_SwingCounterclockwise = 5012;
const MESSAGE_ID MID_RestartRoom = 5013;
const MESSAGE_ID MID_RepeatRate = 5014;
const MESSAGE_ID MID_Slow = 5015;
const MESSAGE_ID MID_Fast = 5016;

const MESSAGE_ID MID_GraphicsAndSound = 5020;
const MESSAGE_ID MID_UseFullScreen = 5021;
const MESSAGE_ID MID_PlaySoundEffects = 5022;
const MESSAGE_ID MID_PlayMusic = 5023;
const MESSAGE_ID MID_Quiet = 5024;
const MESSAGE_ID MID_Loud = 5025;

const MESSAGE_ID MID_Special = 5030;
const MESSAGE_ID MID_ShowCheckpoints = 5031;
const MESSAGE_ID MID_AutoSaveDemoOnConquer = 5032;
const MESSAGE_ID MID_AutoSaveDemoOnDie = 5033;

const MESSAGE_ID MID_AutoSave = 5040;
const MESSAGE_ID MID_ItemTips = 5041;

const MESSAGE_ID MID_Personal = 5050;
const MESSAGE_ID MID_Name = 5051;
const MESSAGE_ID MID_Email = 5052;
const MESSAGE_ID MID_EmailDescription = 5053;

const MESSAGE_ID MID_GetKeyCommand = 5090;
const MESSAGE_ID MID_DefineAllKeys = 5091;
const MESSAGE_ID MID_InvalidCommandKey = 5092;

//Editor screens text. (5500-5999)
const MESSAGE_ID MID_Editor = 5500;
const MESSAGE_ID MID_ChooseHold = 5501;
const MESSAGE_ID MID_NewHold = 5502;
const MESSAGE_ID MID_NewLevel = 5503;
const MESSAGE_ID MID_AddRoomPrompt = 5504;
const MESSAGE_ID MID_RoomStyle = 5505;
const MESSAGE_ID MID_EditRoom = 5506;
const MESSAGE_ID MID_DeleteHoldPrompt = 5507;
const MESSAGE_ID MID_DeleteLevelPrompt = 5508;
const MESSAGE_ID MID_DeleteRoomPrompt = 5509;
const MESSAGE_ID MID_SaveRoomPrompt = 5510;

const MESSAGE_ID MID_Rename = 5511;
const MESSAGE_ID MID_Describe = 5512;
const MESSAGE_ID MID_RoomsToConquer = 5513;
const MESSAGE_ID MID_LevelSettings = 5514;
const MESSAGE_ID MID_SafeEditing = 5515;
const MESSAGE_ID MID_ShowErrors = 5516;
const MESSAGE_ID MID_EnterScrollText = 5517;
const MESSAGE_ID MID_EnteringScrollStatus = 5518;
const MESSAGE_ID MID_PlacingLongMonsterStatus = 5519;
const MESSAGE_ID MID_DefiningOrbStatus = 5520;
const MESSAGE_ID MID_TestRoomLocation = 5521;

const MESSAGE_ID MID_HoldNotSaved = 5530;
const MESSAGE_ID MID_LevelNotSaved = 5531;
const MESSAGE_ID MID_RoomNotSaved = 5532;
const MESSAGE_ID MID_NameHold = 5533;
const MESSAGE_ID MID_DescribeHold = 5534;
const MESSAGE_ID MID_NameLevel = 5535;
const MESSAGE_ID MID_DescribeLevel = 5536;
const MESSAGE_ID MID_EraseLongMonster = 5537;
const MESSAGE_ID MID_EraseScroll = 5538;
const MESSAGE_ID MID_EraseOrb = 5539;
const MESSAGE_ID MID_EraseOrbAgent = 5540;
const MESSAGE_ID MID_ExitLevelPrompt = 5541;
const MESSAGE_ID MID_CantDeleteEntranceRoom = 5542;
const MESSAGE_ID MID_MoveLevelEntrance = 5543;
const MESSAGE_ID MID_DefaultExit = 5544;
const MESSAGE_ID MID_ShowAllLevels = 5545;
const MESSAGE_ID MID_DestLevelPrompt = 5546;
const MESSAGE_ID MID_MakeHoldEntranceLevel = 5547;

const MESSAGE_ID MID_STYLE_1 = 5551;
const MESSAGE_ID MID_STYLE_2 = 5552;
const MESSAGE_ID MID_STYLE_3 = 5553;
const MESSAGE_ID MID_STYLE_4 = 5554;
const MESSAGE_ID MID_STYLE_5 = 5555;
const MESSAGE_ID MID_STYLE_6 = 5556;
const MESSAGE_ID MID_STYLE_7 = 5557;
const MESSAGE_ID MID_STYLE_8 = 5558;
const MESSAGE_ID MID_STYLE_9 = 5559;

const MESSAGE_ID MID_OrbAgentTip = 5570;
const MESSAGE_ID MID_LongMonsterTip = 5571;
const MESSAGE_ID MID_TestRoomTip = 5572;
const MESSAGE_ID MID_OrbAgentToggle = 5573;
const MESSAGE_ID MID_OrbAgentOpen = 5574;
const MESSAGE_ID MID_OrbAgentClose = 5575;

//Key descriptions. (6000-6322)
const MESSAGE_ID MID_UNKNOWN	= 6000;
const MESSAGE_ID MID_KEY_BACKSPACE	= 6008;
const MESSAGE_ID MID_KEY_TAB			= 6009;
const MESSAGE_ID MID_KEY_RETURN		= 6013;
const MESSAGE_ID MID_KEY_SPACE		= 6032;

const MESSAGE_ID MID_KEY_EXCLAIM		= 6033;
const MESSAGE_ID MID_KEY_HASH		= 6035;
const MESSAGE_ID MID_KEY_DOLLAR		= 6036;
const MESSAGE_ID MID_KEY_AMPERSAND		= 6038;
const MESSAGE_ID MID_KEY_QUOTE		= 6039;
const MESSAGE_ID MID_KEY_LEFTPAREN		= 6040;
const MESSAGE_ID MID_KEY_RIGHTPAREN		= 6041;
const MESSAGE_ID MID_KEY_ASTERISK		= 6042;
const MESSAGE_ID MID_KEY_PLUS		= 6043;
const MESSAGE_ID MID_KEY_COMMA		= 6044;
const MESSAGE_ID MID_KEY_MINUS		= 6045;
const MESSAGE_ID MID_KEY_PERIOD		= 6046;
const MESSAGE_ID MID_KEY_SLASH		= 6047;
const MESSAGE_ID MID_KEY_0			= 6048;
const MESSAGE_ID MID_KEY_1			= 6049;
const MESSAGE_ID MID_KEY_2			= 6050;
const MESSAGE_ID MID_KEY_3			= 6051;
const MESSAGE_ID MID_KEY_4			= 6052;
const MESSAGE_ID MID_KEY_5			= 6053;
const MESSAGE_ID MID_KEY_6			= 6054;
const MESSAGE_ID MID_KEY_7			= 6055;
const MESSAGE_ID MID_KEY_8			= 6056;
const MESSAGE_ID MID_KEY_9			= 6057;
const MESSAGE_ID MID_KEY_COLON		= 6058;
const MESSAGE_ID MID_KEY_SEMICOLON		= 6059;
const MESSAGE_ID MID_KEY_LESS		= 6060;
const MESSAGE_ID MID_KEY_EQUALS		= 6061;
const MESSAGE_ID MID_KEY_GREATER		= 6062;
const MESSAGE_ID MID_KEY_QUESTION		= 6063;
const MESSAGE_ID MID_KEY_AT			= 6064;
const MESSAGE_ID MID_KEY_LEFTBRACKET	= 6091;
const MESSAGE_ID MID_KEY_RIGHTBRACKET	= 6093;
const MESSAGE_ID MID_KEY_CARET		= 6094;
const MESSAGE_ID MID_KEY_UNDERSCORE		= 6095;
const MESSAGE_ID MID_KEY_BACKQUOTE		= 6096;

//Note: Missing maps to the following keys:  ~%|{}"

const MESSAGE_ID MID_KEY_a			= 6097;
const MESSAGE_ID MID_KEY_b			= 6098;
const MESSAGE_ID MID_KEY_c			= 6099;
const MESSAGE_ID MID_KEY_d			= 6100;
const MESSAGE_ID MID_KEY_e			= 6101;
const MESSAGE_ID MID_KEY_f			= 6102;
const MESSAGE_ID MID_KEY_g			= 6103;
const MESSAGE_ID MID_KEY_h			= 6104;
const MESSAGE_ID MID_KEY_i			= 6105;
const MESSAGE_ID MID_KEY_j			= 6106;
const MESSAGE_ID MID_KEY_k			= 6107;
const MESSAGE_ID MID_KEY_l			= 6108;
const MESSAGE_ID MID_KEY_m			= 6109;
const MESSAGE_ID MID_KEY_n			= 6110;
const MESSAGE_ID MID_KEY_o			= 6111;
const MESSAGE_ID MID_KEY_p			= 6112;
const MESSAGE_ID MID_KEY_q			= 6113;
const MESSAGE_ID MID_KEY_r			= 6114;
const MESSAGE_ID MID_KEY_s			= 6115;
const MESSAGE_ID MID_KEY_t			= 6116;
const MESSAGE_ID MID_KEY_u			= 6117;
const MESSAGE_ID MID_KEY_v			= 6118;
const MESSAGE_ID MID_KEY_w			= 6119;
const MESSAGE_ID MID_KEY_x			= 6120;
const MESSAGE_ID MID_KEY_y			= 6121;
const MESSAGE_ID MID_KEY_z			= 6122;

// Numeric keypad
const MESSAGE_ID MID_KEY_KP0		= 6256;
const MESSAGE_ID MID_KEY_KP1		= 6257;
const MESSAGE_ID MID_KEY_KP2		= 6258;
const MESSAGE_ID MID_KEY_KP3		= 6259;
const MESSAGE_ID MID_KEY_KP4		= 6260;
const MESSAGE_ID MID_KEY_KP5		= 6261;
const MESSAGE_ID MID_KEY_KP6		= 6262;
const MESSAGE_ID MID_KEY_KP7		= 6263;
const MESSAGE_ID MID_KEY_KP8		= 6264;
const MESSAGE_ID MID_KEY_KP9		= 6265;
const MESSAGE_ID MID_KEY_KP_PERIOD		= 6266;
const MESSAGE_ID MID_KEY_KP_DIVIDE		= 6267;
const MESSAGE_ID MID_KEY_KP_MULTIPLY	= 6268;
const MESSAGE_ID MID_KEY_KP_MINUS		= 6269;
const MESSAGE_ID MID_KEY_KP_PLUS		= 6270;
const MESSAGE_ID MID_KEY_KP_ENTER		= 6271;
const MESSAGE_ID MID_KEY_KP_EQUALS	= 6272;

const MESSAGE_ID MID_KEY_DELETE		= 6127;
const MESSAGE_ID MID_KEY_UP				= 6273;
const MESSAGE_ID MID_KEY_DOWN			= 6274;
const MESSAGE_ID MID_KEY_RIGHT		= 6275;
const MESSAGE_ID MID_KEY_LEFT			= 6276;
const MESSAGE_ID MID_KEY_INSERT		= 6277;
const MESSAGE_ID MID_KEY_HOME			= 6278;
const MESSAGE_ID MID_KEY_END			= 6279;
const MESSAGE_ID MID_KEY_PAGEUP		= 6280;
const MESSAGE_ID MID_KEY_PAGEDOWN	= 6281;

#endif //...#ifndef MESSAGEIDS_H

// $Log: MessageIDs.h,v $
// Revision 1.1  2003/02/25 00:01:35  erikh2000
// Initial check-in.
//
// Revision 1.32  2003/01/08 00:46:35  mrimer
// Changed a message ID name.
//
// Revision 1.31  2002/12/22 01:24:52  mrimer
// Added more message IDs.
//
// Revision 1.30  2002/11/22 01:55:18  mrimer
// Added more message IDs for the editor.
//
// Revision 1.29  2002/11/18 18:56:09  mrimer
// Revised message texts for settings screen and added some new ones.
//
// Revision 1.28  2002/11/15 01:26:50  mrimer
// Added IDs for room style names, plus a couple others.
//
// Revision 1.27  2002/11/13 23:16:10  mrimer
// Added messages for level editor, player and hold selection.
//
// Revision 1.26  2002/10/17 16:48:27  mrimer
// Added MID_HighResWarning.
//
// Revision 1.25  2002/08/30 00:25:20  erikh2000
// Fixed a duplicate MID.
//
// Revision 1.24  2002/08/29 09:18:41  erikh2000
// Added some MIDs.
//
// Revision 1.23  2002/07/10 03:57:53  erikh2000
// Added MID_NoText.
//
// Revision 1.22  2002/06/23 10:45:24  erikh2000
// New MIDs.
//
// Revision 1.21  2002/06/21 03:55:04  erikh2000
// Oops, I reused some MID values.  Fixed that.
//
// Revision 1.20  2002/06/21 03:28:26  erikh2000
// Added a few MIDs.
//
// Revision 1.19  2002/06/16 22:05:32  erikh2000
// Added new messages.
//
// Revision 1.18  2002/06/16 06:22:29  erikh2000
// Added new MID for demos screen message.
//
// Revision 1.17  2002/06/13 22:49:33  erikh2000
// Added new MIDs for month names.
//
// Revision 1.16  2002/06/11 22:39:40  mrimer
// Added demos screen MID consts.
//
// Revision 1.15  2002/06/07 22:55:43  mrimer
// Added message IDs to settings screen.
//
// Revision 1.14  2002/05/21 18:12:19  mrimer
// Added MID_UNKNOWN.
//
// Revision 1.13  2002/05/20 17:46:03  mrimer
// Added Personal box with name and email to settings screen.
//
// Revision 1.12  2002/05/16 23:09:50  mrimer
// Added FlashMessageEffect and level cleared message.
//
// Revision 1.11  2002/05/16 20:24:39  mrimer
// Added text descriptions of mappable keys.
//
// Revision 1.10  2002/05/15 01:21:19  erikh2000
// Added some new message IDs.
//
// Revision 1.9  2002/05/12 03:09:38  erikh2000
// Rearranged message IDs into categories and added new constants.
//
// Revision 1.8  2002/04/24 17:15:15  mrimer
// Added quit confirmation message.
//
// Revision 1.7  2002/04/20 08:19:10  erikh2000
// Added new message IDs.
//
// Revision 1.6  2002/04/16 10:34:32  erikh2000
// Added new messages.
//
// Revision 1.5  2002/04/09 01:05:55  erikh2000
// Added new message IDs for initialization errors.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2002/02/11 00:32:20  erikh2000
// Added some comments.
//
// Revision 1.2  2001/12/16 02:15:07  erikh2000
// Added MID_UnexpectedNeatherDeath and MID_PleaseSpareMe constants.
//
// Revision 1.1.1.1  2001/10/01 22:20:16  erikh2000
// Initial check-in.
//
