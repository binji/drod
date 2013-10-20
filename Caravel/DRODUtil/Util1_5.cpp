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
 * Michael Welsh Duggan (md5i), JP Burford (jpburford), Rik Cookney (timeracer),
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//1.5 is unimplemented.  See notes in header.

// $Log: Util1_5.cpp,v $
// Revision 1.73  2003/05/08 23:34:10  erikh2000
// Removed all implementation code, because I didn't want to have code that supports an
// in-between version data format.  See comments in util1_5.h.
//
// Revision 1.72  2003/05/03 23:30:29  mrimer
// Added a message text.
//
// Revision 1.71  2003/05/01 18:47:17  schik
// Added a new mysql command, which outputs hold information for use in the forum.
//
// Revision 1.70  2003/04/28 18:07:45  erikh2000
// Minor corrections.
//
// Revision 1.69  2003/04/24 22:47:59  mrimer
// Added text for MID_SolvabilityUnknown.
//
// Revision 1.68  2003/04/23 00:01:13  mrimer
// Fixed off by one error in the IsRequired list.
//
// Revision 1.67  2003/04/21 21:59:45  mrimer
// Added IsRequired room setting.  Characterized all rooms.
// Added new message texts.
// Added Unicode support.
//
// Revision 1.66  2003/02/24 20:38:56  erikh2000
// Fixed a bug causing demos to be written without saved games.
//
// Revision 1.65  2003/02/24 19:03:03  erikh2000
// Revised functions to rely on CDbBase functions instead of explicit calls to Metakit functions.  This should ease transition to 1.6 divided database format.
//
// Revision 1.64  2003/01/08 00:51:48  mrimer
// Changed a message.
//
// Revision 1.63  2002/12/22 00:26:57  mrimer
// Changed level exit model to have each staircase state its destination level explicitly.  Added GetRoomExitsView().
// Added GUID info for Dugan's Dungeon hold, levels, and authors.
// Moved author player record addition from "create" to "import" command.
// Removed BETA conditional compiles.
// Added more message texts.
//
// Revision 1.62  2002/11/22 01:46:44  mrimer
// Added more message texts, modified a couple for the settings screen.
//
// Revision 1.61  2002/11/15 03:15:52  mrimer
// Added more message texts.  Added HOLD_DUGANS_DUNGEON const.  Made some vars const.
//
// Revision 1.60  2002/11/13 21:58:00  mrimer
// Added messages for level editor, player and hold selection.
// Removed empty local player record and continue saved game slot being added on "create".
// Moved adding hold record for Dugan's Dungeon from "create" to "import".
// Fixed east tar mother eyes to look different from the west eye.
// Made some vars const.
//
// Revision 1.59  2002/10/25 01:42:17  erikh2000
// Removed BETA.
//
// Revision 1.58  2002/10/21 01:19:18  erikh2000
// Fixed bug causing Metakit assertian on exit.
//
// Revision 1.57  2002/10/17 21:02:05  mrimer
// Added message for MID_HighResWarning.
// Modified a scroll's text.
//
// Revision 1.56  2002/10/15 23:05:57  erikh2000
// Added support for 1.5 .dm2 file importing.
//
// Revision 1.55  2002/10/15 20:38:59  erikh2000
// Fixed a grammar error in one of the messages.
//
// Revision 1.54  2002/09/05 20:14:38  mrimer
// Added check for adding duplicate MIDs into the DB.
//
// Revision 1.53  2002/09/05 18:49:12  mrimer
// Gave default player record an empty name string.
//
// Revision 1.52  2002/08/29 09:15:14  erikh2000
// Added new message texts.
//
// Revision 1.51  2002/07/20 23:22:53  erikh2000
// Revised #includes.
//
// Revision 1.50  2002/07/10 04:17:44  erikh2000
// Added a message text.
//
// Revision 1.49  2002/07/09 19:57:19  mrimer
// Moved Commit call to speed up room import.  Fixed demo import path.
//
// Revision 1.48  2002/07/05 17:47:53  mrimer
// Revised #includes.  Updated checkpoint.
//
// Revision 1.47  2002/07/05 10:45:27  erikh2000
// Made a correction to a level description message.
//
// Revision 1.46  2002/06/23 10:44:55  erikh2000
// New message texts.
//
// Revision 1.45  2002/06/21 03:30:27  erikh2000
// Added some new message texts for demos screen.
//
// Revision 1.44  2002/06/21 01:26:28  erikh2000
// Added code to import demos for intro show sequence.
//
// Revision 1.43  2002/06/20 04:20:11  erikh2000
// Removed references to "ShowFromIntro" field of "Demos" table.
//
// Revision 1.42  2002/06/16 22:08:00  erikh2000
// Fixed a few errors.
// Added message texts.
//
// Revision 1.41  2002/06/16 06:23:09  erikh2000
// Added a message text for demos screen.
//
// Revision 1.40  2002/06/15 18:38:29  erikh2000
// Added level description text.
//
// Revision 1.39  2002/06/13 22:50:10  erikh2000
// Now adding message texts for month names.
//
// Revision 1.38  2002/06/11 22:38:59  mrimer
// Added demos screen buttons/labels text.  Added more button hotkeys.
//
// Revision 1.37  2002/06/09 06:51:55  erikh2000
// Wrote code to create and populate Players table.
// Changed record writing code to use new table structures.
// Changed code to use new message text class.
//
// Revision 1.36  2002/06/07 22:54:28  mrimer
// Added more dialog/button messages.
//
// Revision 1.35  2002/06/05 03:23:23  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.34  2002/05/25 04:28:59  mrimer
// Changed some button text to mark hotkeys.
//
// Revision 1.33  2002/05/21 18:13:44  mrimer
// Added messages for unspecified key mapping and automatic demo save options.
//
// Revision 1.32  2002/05/20 17:46:03  mrimer
// Added Personal box with name and email to settings screen.
//
// Revision 1.31  2002/05/20 15:39:38  mrimer
// Removed obsolete "save game" scrolls.
//
// Revision 1.30  2002/05/16 23:09:50  mrimer
// Added FlashMessageEffect and level cleared message.
//
// Revision 1.29  2002/05/16 20:24:39  mrimer
// Added text descriptions of mappable keys.
//
// Revision 1.28  2002/05/15 13:09:25  mrimer
// Added log macro to end of file.
//
