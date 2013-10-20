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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#define INCLUDED_FROM_DRODSCREENMANAGER_CPP
#include "DrodScreenManager.h"
#undef INCLUDED_FROM_DRODSCREENMANAGER_CPP

//All the screens being instantiated.
#include "GameScreen.h"
#include "TitleScreen.h"
#include "WinStartScreen.h"
#include "WinAudienceScreen.h"
#include "WinRoomScreen.h"
#include "WinPicScreen.h"
#include "SettingsScreen.h"
#include "RestoreScreen.h"
#include "DemoScreen.h"
#include "DemosScreen.h"
#include "LevelStartScreen.h"
#include "CreditsScreen.h"
#include "NewPlayerScreen.h"
#include "SelectPlayerScreen.h"
#include "EditSelectScreen.h"
#include "EditRoomScreen.h"
#include "HoldSelectScreen.h"
#include "SetupScreen.h"
#include "../Texts/MIDs.h"

//Holds the only instance of CDrodScreenManager for the app.
CDrodScreenManager *g_pTheDSM = NULL;

//
//CScreenManager public methods.
//

//*****************************************************************************
UINT CDrodScreenManager::Init(void)
//Inits the screen manager.
//
//Returns:
//MID_Success or an message ID for failure.
{
	//Load the cursor.
	if (!LoadCursors()) {
		return MID_CouldNotLoadResources;
	}

	//Load these screens now because they are necessary to play the game, and
	//I want to know about their load failures early.
	if (!LoadScreen(SCR_Title) ||
			!LoadScreen(SCR_Game)) 
		return MID_CouldNotLoadResources;

	//Success.
	return MID_Success;
}

//*****************************************************************************
CDrodScreenManager::CDrodScreenManager(
//Constructor.
//
//Params:
	SDL_Surface *pSetScreenSurface) //(in)	The screen surface.
	: CScreenManager(pSetScreenSurface)
{
}

//
//CDrodScreenManager private methods.
//

//*****************************************************************************
void CDrodScreenManager::FreeCursors()
{
	if (this->pCursor)
	{
		for (UINT i=CUR_Count; i--; )
			SDL_FreeCursor(this->pCursor[i]);
		delete[] this->pCursor;
	}
}

//*****************************************************************************
bool CDrodScreenManager::LoadCursors()
//Loads special cursors used by app.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->pCursor);
	this->pCursor = new SDL_Cursor*[CUR_Count];
	if (!this->pCursor) return false;

	UINT nIndex = 0;
	this->pCursor[nIndex++] = LoadCursor("Cursor");
	this->pCursor[nIndex++] = LoadCursor("hourglass");
	ASSERT(nIndex == CUR_Count);
	for (nIndex=CUR_Count; nIndex--; )
		if (!this->pCursor[nIndex]) return false;

	return true;
}

//*****************************************************************************
CScreen * CDrodScreenManager::GetNewScreen(
//Class factory for CScreen-derived classes.
//
//Params:
	const UINT eScreen)	//(in)	Type of screen to construct.
//
//Returns:
//New screen object.
{
   switch (eScreen)
   {
      case SCR_Title:
         return (new CTitleScreen);
      
      case SCR_Game:
         return (new CGameScreen);
      
      case SCR_WinStart:
         return (new CWinStartScreen);
      
      case SCR_WinAudience:
         return (new CWinAudienceScreen);
      
      case SCR_WinRoom:
         return (new CWinRoomScreen);
      
      case SCR_WinPic:
         return (new CWinPicScreen);
      
      case SCR_Settings:
         return (new CSettingsScreen);
      
      case SCR_Restore:
         return (new CRestoreScreen);
      
      case SCR_Demo:
         return (new CDemoScreen);
      
      case SCR_Demos:
         return (new CDemosScreen);
      
      case SCR_LevelStart:
         return (new CLevelStartScreen);
      
      case SCR_Credits:
         return (new CCreditsScreen);
      
      case SCR_NewPlayer:
         return (new CNewPlayerScreen);
      
      case SCR_SelectPlayer:
         return (new CSelectPlayerScreen);
      
      case SCR_EditSelect:
         return (new CEditSelectScreen);
      
      case SCR_EditRoom:
         return (new CEditRoomScreen);
      
      case SCR_HoldSelect:
         return (new CHoldSelectScreen);
      
      case SCR_Setup:
         return (new CSetupScreen);

      default:
	      //Bad screen type value.
	      ASSERTP(false, "Bad screen type.");
	      return NULL;
	}
}

//*****************************************************************************
void CDrodScreenManager::GetScreenName(const UINT eScreen, string &strName) const
{
    switch (eScreen)
   {
      case SCR_Title:           strName = "Title"; return;      
      case SCR_Game:            strName = "Game"; return;
      case SCR_WinStart:        strName = "WinStart"; return;
      case SCR_WinAudience:     strName = "WinAudience"; return;
      case SCR_WinRoom:         strName = "WinRoom"; return;
      case SCR_WinPic:          strName = "WinPic"; return;
      case SCR_Settings:        strName = "Settings"; return;
      case SCR_Restore:         strName = "Restore"; return;
      case SCR_Demo:            strName = "Demo"; return;
      case SCR_Demos:           strName = "Demos"; return;
      case SCR_LevelStart:      strName = "LevelStart"; return;
      case SCR_Credits:         strName = "Credits"; return;
      case SCR_NewPlayer:       strName = "NewPlayer"; return;
      case SCR_SelectPlayer:    strName = "SelectPlayer"; return;
      case SCR_EditSelect:      strName = "EditSelect"; return;
      case SCR_EditRoom:        strName = "EditRoom"; return;
      case SCR_HoldSelect:      strName = "HoldSelect"; return;
      case SCR_Setup:           strName = "Setup"; return;
      default:                  strName = "Unknown"; ASSERTP(false, "Bad screen type."); return;
	}
}

// $Log: DrodScreenManager.cpp,v $
// Revision 1.9  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.8  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.7  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.6  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/07/03 08:05:42  mrimer
// Portability tweaking.
//
// Revision 1.4  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
// Revision 1.3  2003/05/28 23:09:47  erikh2000
// Added SCR_Setup.
//
// Revision 1.2  2003/05/25 22:47:33  erikh2000
// Added setup screen--doesn't do anything yet.
//
// Revision 1.1  2003/05/22 23:35:04  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
// Revision 1.31  2003/04/15 14:58:23  mrimer
// Added support for multiple cursor graphics.
//
// Revision 1.30  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.29  2002/12/22 02:18:51  mrimer
// Added HoldSelectScreen.  Added Pan transition effect.
//
// Revision 1.28  2002/11/15 02:24:05  mrimer
// Added EditSelect and EditRoom screens.
//
// Revision 1.27  2002/10/10 01:01:25  erikh2000
// Added code for new CWinStartScreen.
//
// Revision 1.26  2002/10/02 21:43:00  mrimer
// Updated screen fade calls.
//
// Revision 1.25  2002/10/01 22:54:34  erikh2000
// Moved fade routines into Fade.cpp/h.
//
// Revision 1.24  2002/09/30 16:56:02  mrimer
// Initialized eTransition.
//
// Revision 1.23  2002/09/05 18:50:18  mrimer
// Added NewPlayer screen.
//
// Revision 1.22  2002/09/03 21:38:17  erikh2000
// Removed a couple of unneeded variable assignments.
//
// Revision 1.21  2002/08/28 21:40:08  erikh2000
// Cursor is created from Cursor.bmp once when CScreenManager is initialized.
//
// Revision 1.20  2002/07/26 18:23:11  mrimer
// Adde ClearReturnScreens().
//
// Revision 1.19  2002/07/19 20:42:53  mrimer
// Added SCR_Credits screen.
//
// Revision 1.18  2002/07/11 21:00:44  mrimer
// Added win screen selections.
//
// Revision 1.17  2002/07/10 04:14:34  erikh2000
// Minimal fixes to get past compile errors.  Will probably overwritten from mrimer soon.
//
// Revision 1.16  2002/07/05 10:35:00  erikh2000
// Made a few changes to code that activates screens.
// Added a method to remove a return screen.
//
// Revision 1.15  2002/06/23 10:58:49  erikh2000
// Added two methods that change the return screen for an active screen.
//
// Revision 1.14  2002/06/22 02:46:12  erikh2000
// Optimized fade routine by removing unneeded pixel format shifts.
//
// Revision 1.13  2002/06/22 02:00:27  erikh2000
// Changed fading routine to use the surface containing the from image as the destination surface.  This allowed me to remove two temporary surfaces, and an extra blit to the screen surface that was causing the transition to briefly show the to image at the beginning of the fade.
//
// Revision 1.12  2002/06/21 22:30:22  erikh2000
// Added handling for new level start screen.
//
// Revision 1.11  2002/06/21 05:22:30  mrimer
// Revised includes.
// Added FadeBetween().
//
// Revision 1.10  2002/06/20 00:57:02  erikh2000
// Screen activation now makes a call to dest screen's OnBeforeActivate().
//
// Revision 1.9  2002/06/15 18:35:05  erikh2000
// Added call to UpdateRect() so that you can see screen transition code.
//
// Revision 1.8  2002/06/14 22:20:52  mrimer
// Added temporary surface for instantaneous screen transitions.
//
// Revision 1.7  2002/05/15 01:30:41  erikh2000
// Two new screens were added.
//
// Revision 1.6  2002/05/10 22:42:25  erikh2000
// Renamed "InitWidgets()" call to "SetWidgetScreenSurface()".
//
// Revision 1.5  2002/04/29 00:20:40  erikh2000
// Added a special case for leaving SCR_Restore and arriving at SCR_Game.
//
// Revision 1.4  2002/04/20 08:25:00  erikh2000
// Added an include to get file to compile.
//
// Revision 1.3  2002/04/10 00:28:04  erikh2000
// Added CRestoreScreen and CSettingsScreen to class factory.
//
// Revision 1.2  2002/04/09 10:05:40  erikh2000
// Fixed revision log macro.
//
