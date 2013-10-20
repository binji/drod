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

#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include "RoomScreen.h"

#include "../DRODLib/CurrentGame.h"
#include <BackEndLib/Types.h>

#include <SDL.h>

//***************************************************************************************
class CFaceWidget;
class CGameScreen : public CRoomScreen
{
public:
	CCurrentGame * const GetCurrentGame() const {return this->pCurrentGame;}
	bool		IsGameLoaded() const {return (this->pCurrentGame!=NULL);}
	bool		LoadContinueGame();
	bool		LoadNewGame(const DWORD dwHoldID);
	bool		LoadSavedGame(const DWORD dwSavedGameID, bool bRestoreFromStart = false);
	void		SetMusicStyle(const UINT wStyle);
	bool		ShouldShowLevelStart();
	bool		TestRoom(const DWORD dwRoomID, const UINT wX, const UINT wY,
			const UINT wO);
	bool		UnloadGame();

protected:
	friend class CDrodScreenManager;

	CGameScreen(const SCREENTYPE eScreen=SCR_Game);
   virtual ~CGameScreen() { }

	virtual bool   Load();
	virtual void   Paint(bool bUpdateRect=true);
	SCREENTYPE     ProcessCommand(int nCommand);
	virtual bool   SetForActivate();
	virtual void   Unload();

	//These are called by CDemoScreen.
	void				SetSignTextToCurrentRoom();

	//These are accessed by CDemoScreen.
	CCurrentGame *		pCurrentGame;
	CRoomWidget *	pRoomWidget;

private:
	void				AddHighlightDemoInfoToPlayerSettings(const CHighlightDemoInfo *pHDI);
	void				ApplyPlayerSettings();
   void           DisplayRoomStats();
	SCREENTYPE			HandleEventsForLevelExit();
	void				HandleEventsForPlayerDeath();
	void				InitHighlightRoomIDs();
	SCREENTYPE			LevelExit_OnKeydown(const SDL_KeyboardEvent &KeyboardEvent);
	virtual void	OnClick(const DWORD dwTagNo);
	virtual void		OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &KeyboardEvent);
	SCREENTYPE			ProcessCueEventsAfterRoomDraw(CCueEvents &CueEvents);
	SCREENTYPE			ProcessCueEventsBeforeRoomDraw(CCueEvents &CueEvents);
	void				SwirlEffect();

	bool			bShowLevelStartBeforeActivate;

	CFaceWidget *	pFaceWidget;

	CIDList			HighlightRoomIDs;
	bool			bIsSavedGameStale;
	bool			bRestartRoomAtBeginning;
	bool			bPlayTesting;	//from editor
};

#endif //...#ifndef GAMESCREEN_H

// $Log: GameScreen.h,v $
// Revision 1.61  2003/07/24 19:46:24  mrimer
// Added empty virtual desctuctors.
//
// Revision 1.60  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.59  2003/07/15 00:33:37  mrimer
// Refactored code to new DisplayRoomStats().  Now <Enter> also pops up room stats.
// Fixed improper fix of room widget display bug on hold end.
//
// Revision 1.58  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.57  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.56  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.55  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.54  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.53  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.52  2002/11/22 02:18:33  mrimer
// Added TestRoom() for playtesting from the editor.
//
// Revision 1.51  2002/11/15 02:41:57  mrimer
// Refactored code into new base CRoomScreen class.
//
// Revision 1.50  2002/10/10 00:59:47  erikh2000
// Removed code that shows and hides cursor--just let CScreen take care of that.
//
// Revision 1.49  2002/09/05 20:25:45  erikh2000
// Rerenamed "CtagHighlightDemoInfo" to "CHighlightDemoInfo".
//
// Revision 1.48  2002/09/05 18:51:40  mrimer
// Changed references to CHighlightDemoInfo to CtagHighlightDemoInfo.
//
// Revision 1.47  2002/09/04 22:30:30  erikh2000
// Renamed "CtagHighlightDemoInfo" to "CHighlightDemoInfo".
//
// Revision 1.46  2002/07/22 02:48:52  erikh2000
// Made face and room widgets animated.
//
// Revision 1.45  2002/07/19 20:26:48  mrimer
// Changed argument type to AddHighlightDemoInfoToPlayerSettings().
//
// Revision 1.44  2002/07/17 21:23:45  mrimer
// Added bRestartRoomAtBeginning to restart at last checkpoint.
//
// Revision 1.43  2002/07/10 04:12:48  erikh2000
// Rooms that highlight demos will be saved in are specified.
// When a highlight demo is saved, an indice to it will be saved in player settings.
//
// Revision 1.42  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.41  2002/07/03 22:00:22  mrimer
// Added SwirlEffect().
//
// Revision 1.40  2002/06/25 02:35:03  mrimer
// Removed keyDelay and activeKey.
//
// Revision 1.39  2002/06/23 10:51:48  erikh2000
// Wrote method that advises on whether or not to load the level start screen before loading the game screen.
// Change level exit handler to transition to level start screen.
//
// Revision 1.38  2002/06/21 04:55:44  mrimer
// Revised includes.
//
// Revision 1.37  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.36  2002/06/16 22:15:34  erikh2000
// Wrote code to load in auto-save and show checkpoint settings.
// Moved settings loading code into one ApplyPlayerSettings() method.
//
// Revision 1.35  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.34  2002/06/09 06:41:25  erikh2000
// Removed two methods used for setting and getting keysym-to-command map from outside the screen.  These are loaded each time from DB-stored player settings when screen is activated.
//
// Revision 1.33  2002/06/05 19:48:25  mrimer
// Moved REPEAT_DELAY to CScreen.
//
// Revision 1.32  2002/05/23 22:28:22  mrimer
// Load and save keyboard commands from DROD.ini.
//
// Revision 1.31  2002/05/21 19:01:24  erikh2000
// When loading a saved game for screen, it's now possible to restore from beginning.
//
// Revision 1.30  2002/05/21 18:11:24  mrimer
// Added changing command key mappings in settings screen.
//
// Revision 1.29  2002/05/16 18:22:04  mrimer
// Added player death animation.
//
// Revision 1.28  2002/05/15 23:43:28  mrimer
// Completed exit level sequence.
//
// Revision 1.27  2002/05/15 01:26:00  erikh2000
// Added handling for F5 (begin/end demo recording) and F6 (go to demo screen).
//
// Revision 1.26  2002/05/14 22:05:04  mrimer
// Wrote part of animating the swordsman walking down the stairs.
//
// Revision 1.25  2002/05/12 03:15:55  erikh2000
// Changed OnKeyDown to call CScreen::OnKeydown().
// Rearranged cheat key code so that one block of code would handle most of the room exit work.
//
// Revision 1.24  2002/05/10 22:35:32  erikh2000
// Revised cursor show/hide code to use methods in CScreen.
//
// Revision 1.23  2002/04/29 00:12:47  erikh2000
// Added code to load current game from a saved game.
//
// Revision 1.22  2002/04/25 18:02:58  mrimer
// Added variable to keep track of whether mouse cursor should be visible.
//
// Revision 1.21  2002/04/25 09:31:41  erikh2000
// Fixed some room widget refresh problems.
// Added music.
//
// Revision 1.20  2002/04/20 08:23:29  erikh2000
// Moved positions of elements around.
// Added sign at top of screen.
// Added scroll.
// Fixed some face widget updating problems.
//
// Revision 1.19  2002/04/19 21:56:11  erikh2000
// GameScreen now loads and displays a monolithic 640x480 background including a hidden parts area.
// Removed references to DRODGfx.dib.
// Moved some constants from ScreenConstants.h into constructor, and removed references to ScreenConstants.h.
// Removed FillScreenWithTexture() method.
//
// Revision 1.18  2002/04/16 10:42:03  erikh2000
// Cue-event processing routines can now cause screen change by returning a screen type.
//
// Revision 1.17  2002/04/13 19:45:59  erikh2000
// Added declaration for OnMouseMotion().
//
// Revision 1.16  2002/04/12 05:19:25  erikh2000
// Added InitKeysymToCommandMap().
//
// Revision 1.15  2002/04/09 21:53:55  erikh2000
// Added key repeat functionality.  (Committed on behalf of mrimer.)
//
// Revision 1.14  2002/04/09 10:25:33  erikh2000
// Added CGameScreen::Animate().
//
// Revision 1.13  2002/04/09 10:02:19  erikh2000
// Added face widget to screen.
//
// Revision 1.12  2002/04/09 01:16:34  erikh2000
// Functions were put into a new CGameScreen class which uses SDL.
//
// Revision 1.11  2002/03/13 06:31:13  erikh2000
// Added ChangeMusicStyle() prototype.  (Committed on behalf of mrimer.)
//
// Revision 1.10  2002/03/05 01:52:59  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.9  2002/02/13 01:04:05  erikh2000
// Added code to show room coords.
//
// Revision 1.8  2001/11/03 20:34:33  erikh2000
// Fixed problem with tile images not refreshing when cheat keys are used.
//
// Revision 1.7  2001/11/03 20:10:32  erikh2000
// Fix problems with map not refreshing.  (Committed on behalf of timeracer.)
// Removed OnPlot() and OnLoad() references.  Added processing of CID_Plots cue event.
//
// Revision 1.6  2001/10/27 04:43:58  erikh2000
// Created tile image arrays to store tile images for drawing room.
// Added OnLoad() and OnPlot() handling.
// Wrote tile image calculation functions for floor, pit, obstacle, wall, and crumbly wall.
// Added stubs for CalcTileImageForTar() and CalcTileImageForStairs().
//
// Revision 1.5  2001/10/20 10:17:09  erikh2000
// Added OnPlot() callback.
//
// Revision 1.4  2001/10/07 04:38:34  erikh2000
// General cleanup--finished removing game logic code from DROD.EXE.
// Added ProcessCommand() method to CCurrentGame to be used as interface between UI and game logic.
//
// Revision 1.3  2001/10/06 22:51:08  erikh2000
// Removed old restore game and save game code.
// Cleaned up title.cpp.
// Disabled some menu items.
// Changed names of some routines and removed failure returns.
//
// Revision 1.2  2001/10/06 02:40:58  erikh2000
// Removed best display mode warning code.
// Removed Inval class and references to it.
//
// Revision 1.1.1.1  2001/10/01 22:18:01  erikh2000
// Initial check-in.
//
