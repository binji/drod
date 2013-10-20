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
 * Portions created by the Initial Developer are Copyright (C) 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DrodScreenManager.h
//Declarations for CDrodScreenManager.
//CScreenManager loads and unloads DROD screens, with knowledge of how DROD uses the
//screens.  It directs execution to an input loop in an appropriate CScreen class.

#ifndef DRODSCREENMANAGER_H
#define DRODSCREENMANAGER_H

#include <FrontEndLib/ScreenManager.h>

//All of these screentypes should be handled in GetNewScreen() and GetScreenName().
enum SCREENTYPE {
   SCR_None = SCREENLIB::SCR_None,		//Not an actual screen--indicates no screen or application exit.
	SCR_Return = SCREENLIB::SCR_Return,	//Not an actual screen--indicates the screen previously visited.
	SCR_Title,
	SCR_Game,
	SCR_WinStart,
	SCR_WinAudience,
	SCR_WinRoom,
	SCR_WinPic,
	SCR_Settings,
	SCR_Restore,
	SCR_Demo,
	SCR_Demos,
	SCR_LevelStart,
	SCR_Credits,
	SCR_NewPlayer,
   SCR_SelectPlayer,
	SCR_EditSelect,
	SCR_EditRoom,
	SCR_HoldSelect,
   SCR_Setup
};

//Cursor icons.
enum CURSORTYPE {
   CUR_Select = SCREENLIB::CUR_Select,
   CUR_Wait = SCREENLIB::CUR_Wait,
	CUR_Count
};

//*****************************************************************************
class CScreen;
class CDrodScreenManager : public CScreenManager
{
public:
	CDrodScreenManager(SDL_Surface *pSetScreenSurface);

	virtual UINT		Init();
    virtual void        GetScreenName(const UINT eScreen, string &strName) const;

protected:
	virtual void FreeCursors();
	virtual CScreen *	GetNewScreen(const UINT eScreen);
	virtual bool LoadCursors();
};

//Define global pointer to the one and only CDrodScreenManager object.
#ifndef INCLUDED_FROM_DRODSCREENMANAGER_CPP
	extern CDrodScreenManager *g_pTheDSM;
#endif

#endif //...#ifndef DRODSCREENMANAGER_H

// $Log: DrodScreenManager.h,v $
// Revision 1.6  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.5  2003/07/12 01:12:00  mrimer
// Revised screen/cursor naming in FrontEndLib.
//
// Revision 1.4  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
// Revision 1.3  2003/05/26 01:56:53  erikh2000
// Added SCR_Screen constant.
//
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.1  2003/05/22 23:35:04  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
// Revision 1.23  2003/04/15 14:53:41  mrimer
// Added support for multiple cursor graphics.
//
// Revision 1.22  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.21  2002/12/22 02:18:51  mrimer
// Added HoldSelectScreen.  Added Pan transition effect.
//
// Revision 1.20  2002/11/15 02:24:05  mrimer
// Added EditSelect and EditRoom screens.
//
// Revision 1.19  2002/10/10 01:01:25  erikh2000
// Added code for new CWinStartScreen.
//
// Revision 1.18  2002/10/01 22:54:34  erikh2000
// Moved fade routines into Fade.cpp/h.
//
// Revision 1.17  2002/09/05 18:50:18  mrimer
// Added NewPlayer screen.
//
// Revision 1.16  2002/08/28 21:40:08  erikh2000
// Cursor is created from Cursor.bmp once when CScreenManager is initialized.
//
// Revision 1.15  2002/07/26 18:23:11  mrimer
// Adde ClearReturnScreens().
//
// Revision 1.14  2002/07/19 20:30:58  mrimer
// Added SCR_Credits.
//
// Revision 1.13  2002/07/11 20:56:04  mrimer
// Removed bFirstScreen.
//
// Revision 1.12  2002/07/10 04:14:34  erikh2000
// Minimal fixes to get past compile errors.  Will probably overwritten from mrimer soon.
//
// Revision 1.11  2002/07/09 23:14:09  mrimer
// Added win room types.  Added SetDestTransition() and varied transition handling.
//
// Revision 1.10  2002/07/05 10:35:00  erikh2000
// Made a few changes to code that activates screens.
// Added a method to remove a return screen.
//
// Revision 1.9  2002/06/23 10:58:49  erikh2000
// Added two methods that change the return screen for an active screen.
//
// Revision 1.8  2002/06/22 06:01:36  erikh2000
// FadeBetween24 takes two Uint8 arrays that are screen-ordered surface bytes instead of RGB-ordered bytes.
//
// Revision 1.7  2002/06/21 22:30:22  erikh2000
// Added handling for new level start screen.
//
// Revision 1.6  2002/06/21 04:57:53  mrimer
// Added FadeBetween().
//
// Revision 1.5  2002/06/14 22:20:52  mrimer
// Added temporary surface for instantaneous screen transitions.
//
// Revision 1.4  2002/05/15 01:30:41  erikh2000
// Two new screens were added.
//
// Revision 1.3  2002/04/10 00:28:04  erikh2000
// Added CRestoreScreen and CSettingsScreen to class factory.
//
// Revision 1.2  2002/04/09 10:05:40  erikh2000
// Fixed revision log macro.
//
