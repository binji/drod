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

#include <BackEndLib/Types.h>

enum LANGUAGE_CODE
{
	English = 1,
	French = 2,
	Russian = 3
};

typedef DWORD MESSAGE_ID;

#endif //...#ifndef MESSAGEIDS_H

// $Log: MessageIDs.h,v $
// Revision 1.2  2003/05/25 22:45:08  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.39  2003/05/08 23:22:10  erikh2000
// All MIDs are included from a DRODUTIL-generated MIDs.h.
//
// Revision 1.38  2003/05/03 23:27:31  mrimer
// Added an ID.
//
// Revision 1.37  2003/04/28 22:14:07  mrimer
// Added an ID.
//
// Revision 1.36  2003/04/24 22:45:39  mrimer
// Added uncertain hold solvability MID.
//
// Revision 1.35  2003/04/21 21:00:06  mrimer
// Added MIDs for room position in level.
//
// Revision 1.34  2003/04/17 20:50:39  mrimer
// Changed a message ID.
//
// Revision 1.33  2003/04/13 02:06:04  mrimer
// Changed MID_GameStart to MID_HoldStart.
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
