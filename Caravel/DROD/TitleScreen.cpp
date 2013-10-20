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
 * John Wm. Wicks (j_wicks), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "TitleScreen.h"
#include "Browser.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include "DemoScreen.h"
#include "../Texts/MIDs.h"
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>

#include "../DRODLib/Db.h"

//
//Protected methods.
//

//******************************************************************************
CTitleScreen::CTitleScreen() : CDrodScreen(SCR_Title)
	, pTitleSurface(NULL)
	, pHeadsSurface(NULL)
	, pCurrentDemoID(NULL)
	, dwCurrentDemoHoldID(0L)
	, nHeadOffsetX(0)
	, wHeadsO(S)
	, wSelectedPos(MNU_UNSPECIFIED)
	, dwFirstPaint(0L)
	, bSavedGameExists(false)
//Constructor.
{
	//Animate the screen in time (roughly) with the music.
	SetBetweenEventsInterval(96);
}

//******************************************************************************
bool CTitleScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);

	//Load Title bitmap.
	this->pTitleSurface = g_pTheBM->GetBitmapSurface("Title");
	if (!this->pTitleSurface) return false;

	//Load Heads bitmap.
	this->pHeadsSurface = g_pTheBM->GetBitmapSurface("Heads");
	if (!this->pHeadsSurface) return false;
	Uint32 TransparentColor = SDL_MapRGB(this->pHeadsSurface->format, 0, 255, 0);
	SDL_SetColorKey(this->pHeadsSurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, TransparentColor);

	//Load children.
	this->bIsLoaded = LoadChildren();
	
	//Set mouse cursor graphic.
	SetCursor();

	return this->bIsLoaded;
}

//******************************************************************************
void CTitleScreen::LoadDemos()
//Load demo show sequence for currently selected hold.
{
	const DWORD dwCurrentHoldID = g_pTheDB->GetHoldID();
	if (this->dwCurrentDemoHoldID != dwCurrentHoldID)
	{
		//(Re)Load demo list if selected hold has changed.
      CDb db;
		this->dwCurrentDemoHoldID = dwCurrentHoldID;
		db.Demos.FilterByShow();
		db.Demos.GetIDs(this->ShowSequenceDemoIDs);
		this->pCurrentDemoID = NULL;
	}
}

//******************************************************************************
void CTitleScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	//Unload children.
	UnloadChildren();

	if (this->pTitleSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Title");
		this->pTitleSurface = false;
	}

	if (this->pHeadsSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Heads");
		this->pHeadsSurface = false;
	}
	
	this->bIsLoaded = false;
}

//******************************************************************************
bool CTitleScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
   //There ought to always be an active player at the title screen.
	const DWORD dwPlayerID = g_pTheDB->GetPlayerID();
   ASSERT(dwPlayerID);

   //Music might have been enabled upon going to Win Screen.
	//Now restore music setting to original state upon coming to this screen.
	CDbPlayer *pPlayer = g_pTheDB->GetCurrentPlayer();
	if (pPlayer)
   {
	   g_pTheSound->EnableMusic(pPlayer->Settings.GetVar("Music", true) != 0);
	   delete pPlayer;
	   SetSavedGameExists();
   }

	g_pTheSound->PlaySong(SONGID_INTRO);

	//Select a menu item that mouse may be over right now.
	int nX, nY;
	ShowCursor();
	SDL_GetMouseState(&nX, &nY);
	this->wSelectedPos = GetMenuPosFromCoords(nX, nY);

	this->dwFirstPaint = SDL_GetTicks();

	LoadDemos();

 	return true;
}

//
//Private methods.
//

//*****************************************************************************
void CTitleScreen::BlitEmptyRestoreBrick() const
//Blits an empty brick (no "restore" text) over the restore brick area.
{
	static SDL_Rect EmptyRestoreSrc = {641,285,92,53};
	static SDL_Rect EmptyRestoreDest = {548,68,92,53};
	SDL_BlitSurface(this->pTitleSurface, &EmptyRestoreSrc, GetDestSurface(), 
		&EmptyRestoreDest);
}

//*****************************************************************************
void CTitleScreen::OnBetweenEvents()
//Handle events 
{
	CScreen::OnBetweenEvents();

	Animate();
	
   //Show tool tip for highlighted menu option.
   switch (this->wSelectedPos)
   {
      case MNU_CONTINUE: RequestToolTip(MID_ContinueTip); break;
      case MNU_RESTORE: RequestToolTip(MID_RestoreTip); break;
      case MNU_SETTINGS: RequestToolTip(MID_SettingsTip); break;
      case MNU_HELP: RequestToolTip(MID_HelpTip); break;
      case MNU_DEMO: RequestToolTip(MID_DemoTip); break;
      case MNU_QUIT: RequestToolTip(MID_QuitTip); break;
      case MNU_BUILD: RequestToolTip(MID_BuildTip); break;
      case MNU_WHO: RequestToolTip(MID_WhoTip); break;
      case MNU_WHERE: RequestToolTip(MID_WhereTip); break;
      default: break;
   }

   //Go to demo screen after 10 seconds.
	if (SDL_GetTicks() - this->dwFirstPaint > 10000)
	{
		const DWORD dwDemoID = GetNextDemoID();
		if (dwDemoID)
		{
			CDemoScreen *pDemoScreen = DYN_CAST(CDemoScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Demo));
			if (!pDemoScreen || !pDemoScreen->LoadDemoGame(dwDemoID))
				ShowOkMessage(MID_LoadGameFailed);
         else {
            pDemoScreen->SetReplayOptions(false);
				GoToScreen(SCR_Demo);
         }
		}
	}
}

//*****************************************************************************
void CTitleScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD /*dwTagNo*/,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Key event.
{
	//Reset the wait countdown for going to the demo screen.
	this->dwFirstPaint = SDL_GetTicks();

	TitleSelection wSetPos;
	switch (Key.keysym.sym)
	{
      case SDLK_RETURN:
         if (!(Key.keysym.mod & KMOD_ALT))
         {
            wSetPos = MNU_CONTINUE; break;
         }
         else
            //going to next case
		case SDLK_F10:
			ToggleScreenSize();
		return;

		case SDLK_F4:
			if (Key.keysym.mod & KMOD_ALT)
				GoToScreen(SCR_None);	//boss key -- exit immediately
		return;

		case SDLK_ESCAPE:
         //Pressing ESC will quit without confirmation dialog.
			HighlightMenuItem(MNU_QUIT);
		   GoToScreen(SCR_None);
		return;

		//Menu selections.
		case SDLK_c: case SDLK_SPACE: //SDLK_RETURN handled above
			wSetPos = MNU_CONTINUE; break;
		case SDLK_r:
			if (this->bSavedGameExists)
			{
				wSetPos = MNU_RESTORE; break;
			} else
				return;
		case SDLK_s: wSetPos = MNU_SETTINGS; break;
		case SDLK_F1:
		case SDLK_h: wSetPos = MNU_HELP; break;
		case SDLK_d: wSetPos = MNU_DEMO; break;
		case SDLK_q: wSetPos = MNU_QUIT; break;
		case SDLK_b: wSetPos = MNU_BUILD; break;
		case SDLK_w: wSetPos = MNU_WHO; break;
		case SDLK_e: wSetPos = MNU_WHERE; break;
		
		default: 
		return;
	}
	
	//One of the menu items was chosen.
	
	//Show brick being pushed.
	HighlightMenuItem(wSetPos);
	g_pTheSound->PlaySoundEffect(SEID_TITLEBUTTON);
	DrawPushedBrick(wSetPos);
	SDL_Delay(200);
	DrawBrick(wSetPos);
	SDL_Delay(50);

	SCREENTYPE eNextScreen = ProcessMenuSelection(wSetPos);
	if (eNextScreen != SCR_Title)
		GoToScreen(eNextScreen);
	this->dwFirstPaint = SDL_GetTicks();
}

//*****************************************************************************
void CTitleScreen::OnMouseMotion(
//Handles SDL_MOUSEMOTION events for the title screen.
//
//Params:
	const DWORD dwTagNo,						//(in) Widget receiving event.
	const SDL_MouseMotionEvent &MotionEvent)	//(in) Event to handle.
{
	CScreen::OnMouseMotion(dwTagNo,MotionEvent);

	//Reset the wait countdown for going to the demo screen.
	this->dwFirstPaint = SDL_GetTicks();

	//Calc new head offset.
	const int CX_OFFSET_RANGE = 70; //Distance main head can move left or right.
	const int X_MIDPOINT = static_cast<int>(CX_SCREEN * .5);
	this->nHeadOffsetX = (static_cast<int>(MotionEvent.x) -
 			X_MIDPOINT) * CX_OFFSET_RANGE / X_MIDPOINT;

	//Show possible new menu highlight.
	HighlightMenuItem(GetMenuPosFromCoords(MotionEvent.x, MotionEvent.y));

	return;
}

//*****************************************************************************
void CTitleScreen::OnMouseUp(
//Handles SDL_MOUSEBUTTONUP events for the title screen.
//
//Params:
	const DWORD /*dwTagNo*/,								//(in) Widget receiving event.
	const SDL_MouseButtonEvent &/*ButtonEvent*/)	//(in) Event to handle.
{
	if (this->wSelectedPos != MNU_UNSPECIFIED &&
			!(this->wSelectedPos == MNU_RESTORE && !this->bSavedGameExists))
	{
		//Show brick being pushed.
		g_pTheSound->PlaySoundEffect(SEID_TITLEBUTTON);
		DrawPushedBrick(wSelectedPos);
		SDL_Delay(200);
		DrawBrick(wSelectedPos);
		SDL_Delay(50);

		SCREENTYPE eNextScreen = ProcessMenuSelection(this->wSelectedPos);
			if (eNextScreen != SCR_Title)
				GoToScreen(eNextScreen);
	}

	//Reset the wait countdown for going to the demo screen.
	this->dwFirstPaint = SDL_GetTicks();
}

//*****************************************************************************
void CTitleScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	//Blit the main tile graphic.
	static SDL_Rect Dest = {0, 0, 640, 480};
	static SDL_Rect Src = {0, 0, 640, 480};
	SDL_BlitSurface(this->pTitleSurface, &Src, GetDestSurface(), &Dest);

	if (!this->bSavedGameExists) BlitEmptyRestoreBrick();

	if (this->wSelectedPos != MNU_UNSPECIFIED)
		DrawHighlightedBrick(this->wSelectedPos);

	PaintHeads();

	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CTitleScreen::HighlightMenuItem(
//Highlights a menu item, updating screen to show it.
//
//Params:
	TitleSelection wMenuPos)	//(in)	New item.
{
	if (wMenuPos == this->wSelectedPos) return; //Nothing to do.

   RemoveToolTip();

   //Unselect current brick.
	if (this->wSelectedPos != MNU_UNSPECIFIED)
		DrawBrick(this->wSelectedPos);
	
	//Select new brick.
	this->wSelectedPos = wMenuPos;
	if (this->wSelectedPos != MNU_UNSPECIFIED)
		DrawHighlightedBrick(this->wSelectedPos);
}

//*****************************************************************************
SCREENTYPE CTitleScreen::ProcessMenuSelection(
//Processes menu selection.  No UI-related performed here.
//
//Params:
	TitleSelection wMenuPos)	//(in) One of the MNU_* constants.
//
//Returns:
//Screen to go to next or SCR_Title to remain at title screen.
{
	this->wSelectedPos = MNU_UNSPECIFIED;	//highlighting fix

	switch (wMenuPos)
	{
		case MNU_CONTINUE:
		{
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetLoadedScreen(SCR_Game));
			const DWORD dwCurrentHoldID = g_pTheDB->GetHoldID();
			if (!pGameScreen->IsGameLoaded())
			{
				if (!pGameScreen->LoadContinueGame())
				{
					//Probable continue slot is missing.
					if (!pGameScreen->LoadNewGame(dwCurrentHoldID))
					{
						ShowOkMessage(MID_LoadGameFailed);
						return SCR_Title;
					}
				}
			}
			ASSERT(pGameScreen->IsGameLoaded());

			{
				//While playing the game, the author of a hold might alter the demo
				//show sequence.  So reset the title screen's demo hold ID to reload
				//the demo show sequence on return to the title screen.
				CDbHold *pHold = g_pTheDB->Holds.GetByID(dwCurrentHoldID);
				if (pHold->dwPlayerID == g_pTheDB->GetPlayerID())
					this->dwCurrentDemoHoldID = 0L;
				delete pHold;
			}

			if (pGameScreen->ShouldShowLevelStart())
				return SCR_LevelStart;

			return SCR_Game;
		}

		case MNU_HELP:
			SetFullScreen(false);
			ShowHelp("contents.html");
		return SCR_Title;

		case MNU_SETTINGS:
		return SCR_Settings;

		case MNU_RESTORE:
			ASSERT(this->bSavedGameExists);
		return SCR_Restore;

		case MNU_DEMO:
		{
			const DWORD dwDemoID = GetNextDemoID();
			if (dwDemoID)
			{
				CDemoScreen *pDemoScreen = DYN_CAST(CDemoScreen*, CScreen*,
						g_pTheSM->GetScreen(SCR_Demo));
				if (!pDemoScreen || !pDemoScreen->LoadDemoGame(dwDemoID))
					ShowOkMessage(MID_LoadGameFailed);
            else {
               pDemoScreen->SetReplayOptions(false);
					return SCR_Demo;
            }
			}
		}
		return SCR_Title;

		case MNU_BUILD:
      {
         if (!g_pTheDB->Holds.EditableHoldExists())
            if (ShowYesNoMessage(MID_CreateHoldPrompt) != TAG_YES)
               break;

         //Editing room of current game could break it -- so unload it now.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetLoadedScreen(SCR_Game));
			pGameScreen->UnloadGame();
      }
		return SCR_EditSelect;

		case MNU_QUIT:
			if (ShowYesNoMessage(MID_ReallyQuit) == TAG_YES)
				return SCR_None;
   	break;

      case MNU_WHO:
      {
         //Changing player will make the current game invalid -- so unload it now.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetLoadedScreen(SCR_Game));
			pGameScreen->UnloadGame();
      }
      return SCR_SelectPlayer;

		case MNU_WHERE:
      {
         //Deleting hold of current game would cause it to save a continue
         //slot for a room that no longer exists -- so unload the game now.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetLoadedScreen(SCR_Game));
			pGameScreen->UnloadGame();
      }
		return SCR_HoldSelect;

      default: break;
	}

	return SCR_Title;
}

//*****************************************************************************
void CTitleScreen::Animate()
//Animates the screen.
{
	this->wHeadsO = nNextCO(this->wHeadsO);
	PaintHeads();

	AnimateWaves();
	AnimateFlag();

   this->pEffects->DrawEffects();
}

//*****************************************************************************
void CTitleScreen::PaintHeads()
//Paints the spinning heads.
{
	static const UINT CX_HEAD_SPACE = 64;
	static const double SCALE_FACTOR = 0.6;
	static const double SCALE_FACTOR_2 = SCALE_FACTOR * SCALE_FACTOR;
	static const double SCALE_FACTOR_3 = SCALE_FACTOR * SCALE_FACTOR * SCALE_FACTOR;
	static const double SCALE_FACTOR_4 = SCALE_FACTOR * SCALE_FACTOR * SCALE_FACTOR * SCALE_FACTOR;
	static const int X_MAIN = 240;
	static const int Y_MAIN = 196;
	static const int X_SECOND_LEFT = X_MAIN - CX_HEAD_SPACE;
	static const int X_SECOND_RIGHT = X_MAIN + CX_HEAD_SPACE;
	static const int Y_SECOND = Y_MAIN;
	static const int X_THIRD_LEFT = X_SECOND_LEFT - static_cast<int>(CX_HEAD_SPACE * SCALE_FACTOR);
	static const int X_THIRD_RIGHT = X_SECOND_RIGHT + static_cast<int>(CX_HEAD_SPACE * SCALE_FACTOR);
	static const int Y_THIRD = Y_MAIN;
	static const int X_FOURTH_LEFT = X_THIRD_LEFT - static_cast<int>(CX_HEAD_SPACE * SCALE_FACTOR_2);
	static const int X_FOURTH_RIGHT = X_THIRD_RIGHT + static_cast<int>(CX_HEAD_SPACE * SCALE_FACTOR_2);
	static const int Y_FOURTH = Y_MAIN;
   static const int X_FIFTH_LEFT = X_FOURTH_LEFT - static_cast<int>(CX_HEAD_SPACE * SCALE_FACTOR_3);
	static const int X_FIFTH_RIGHT = X_FOURTH_RIGHT + static_cast<int>(CX_HEAD_SPACE * SCALE_FACTOR_3);
	static const int Y_FIFTH = Y_MAIN;
	
	static const int X_HEADS = 61;
	static const int Y_HEADS = 0;
	static const UINT CX_HEADS = 368;
	static const UINT CY_HEADS = 220;
	static SDL_Rect HeadsRect = {X_HEADS, Y_HEADS, CX_HEADS, CY_HEADS};

	//Blit background over head area.
	SDL_Surface *pScreenSurface = GetDestSurface();
	SDL_BlitSurface(this->pTitleSurface, &HeadsRect, pScreenSurface, &HeadsRect);

	//Calc offsets for 2nd, 3rd, 4th, and 5th heads.
	const int nHead2OffsetX = static_cast<int>(this->nHeadOffsetX * SCALE_FACTOR);
	const int nHead3OffsetX = static_cast<int>(this->nHeadOffsetX * SCALE_FACTOR_2);
	const int nHead4OffsetX = static_cast<int>(this->nHeadOffsetX * SCALE_FACTOR_3);
	const int nHead5OffsetX = static_cast<int>(this->nHeadOffsetX * SCALE_FACTOR_4);

	//Blit heads.
	DrawFifthHead(X_FIFTH_LEFT + nHead5OffsetX, Y_FIFTH, this->wHeadsO);
	DrawFifthHead(X_FIFTH_RIGHT + nHead5OffsetX, Y_FIFTH, this->wHeadsO);
	DrawFourthHead(X_FOURTH_LEFT + nHead4OffsetX, Y_FOURTH, this->wHeadsO);
	DrawFourthHead(X_FOURTH_RIGHT + nHead4OffsetX, Y_FOURTH, this->wHeadsO);
	DrawThirdHead(X_THIRD_LEFT + nHead3OffsetX, Y_THIRD, this->wHeadsO);
	DrawThirdHead(X_THIRD_RIGHT + nHead3OffsetX, Y_THIRD, this->wHeadsO);
	DrawSecondHead(X_SECOND_LEFT + nHead2OffsetX, Y_SECOND, this->wHeadsO);
	DrawSecondHead(X_SECOND_RIGHT + nHead2OffsetX, Y_SECOND, this->wHeadsO);
	DrawMainHead(X_MAIN + this->nHeadOffsetX, Y_MAIN, this->wHeadsO);

	//Update screen.
	UpdateRect(HeadsRect);
}

//*****************************************************************************
void CTitleScreen::AnimateWaves()
//Animates the waves in the Caravel logo.
{
	//Waves area.
	const int X_WAVES = 556;
	const int Y_WAVES = 435;
	const UINT CX_WAVES = 44;
	const UINT CY_WAVES = 3;
	static SDL_Rect WavesRect = {X_WAVES, Y_WAVES, CX_WAVES, CY_WAVES};

	static UINT wIndex=0;

	++wIndex;
	if (wIndex==CX_WAVES) wIndex=0;

	//Draw left side of waves.
	SDL_Surface *pScreenSurface = GetDestSurface();
	SDL_Rect Src = {X_WAVES+wIndex, Y_WAVES, CX_WAVES-wIndex, CY_WAVES};
	SDL_Rect Dest = {X_WAVES, Y_WAVES, CX_WAVES-wIndex, CY_WAVES};
	SDL_BlitSurface(this->pTitleSurface, &Src, pScreenSurface, &Dest);

	//Draw right side of waves.
	if (wIndex)  
	{
		Src.x = X_WAVES;
		Src.w = wIndex;
		Dest.x = X_WAVES+CX_WAVES-wIndex;
		Dest.w = wIndex;
		SDL_BlitSurface(this->pTitleSurface, &Src, pScreenSurface, &Dest);
	}

	UpdateRect(WavesRect);
}

//*****************************************************************************
void CTitleScreen::AnimateFlag()
//Animates the flag in the Caravel logo.
{
	//Flag area.
	const int X_FLAG = 573;
	const int Y_FLAG = 357;
	const UINT CX_FLAG = 11;
	const UINT CY_FLAG = 4;
	static SDL_Rect FlagRect = {X_FLAG, Y_FLAG, CX_FLAG, CY_FLAG+1};

	static UINT wIndex=0;

	++wIndex;
	if (wIndex==CX_FLAG) wIndex=0;

	//Draw left side of flag.
	SDL_Surface *pScreenSurface = GetDestSurface();
	SDL_Rect Src = {X_FLAG+wIndex, Y_FLAG, CX_FLAG-wIndex, CY_FLAG};
	SDL_Rect Dest = {X_FLAG, Y_FLAG, CX_FLAG-wIndex, CY_FLAG};
	SDL_BlitSurface(this->pTitleSurface, &Src, pScreenSurface, &Dest);

	//Draw right side of flag.
	if (wIndex)  
	{
		Src.x = X_FLAG;
		Src.w = wIndex;
		Dest.x = X_FLAG+CX_FLAG-wIndex;
		Dest.y = Y_FLAG + 1;
		Dest.w = wIndex;
		SDL_BlitSurface(this->pTitleSurface, &Src, pScreenSurface, &Dest);
	}

	UpdateRect(FlagRect);
}

//*****************************************************************************
void CTitleScreen::DrawMainHead(
//Blits main head to screen.
//
//Params:
	int nX, int nY,		//(in)	Screen coords to center head on.
	UINT wO)			//(in)	Direction head will face--S is straight ahead,
						//		E is facing right, etc.
{
	static SDL_Rect SrcArray[ORIENTATION_COUNT] =
	{
		{93, 2, 141, 219},	//Northwest.
		{15, 1, 78, 221},	//North.
		{962, 2, 165, 218},	//Northeast.
		{239, 0, 196, 220},	//West.
		{0, 0, 0, 0},		//Illegal orientation.
		{776, 8, 185, 212},	//East.
		{436, 14, 126, 204},//Southwest.
		{562, 13, 80, 208},	//South.
		{643, 17, 132, 203} //Southeast.
	};
	static const int xOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-55,  -40,  -108, -78,  0, -112, -68,  -38,  -66
	};
	static const int yOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-200, -200,	-198, -205, 0, -195, -187, -186, -182
	};

	SDL_Rect Dest = {nX + xOffset[wO], nY + yOffset[wO], SrcArray[wO].w, SrcArray[wO].h};
	SDL_BlitSurface(this->pHeadsSurface, &SrcArray[wO], GetDestSurface(), &Dest);
}

//*****************************************************************************
void CTitleScreen::DrawSecondHead(
//Blits second head to screen.
//
//Params:
	int nX, int nY,		//(in)	Screen coords to center head on.
	UINT wO)			//(in)	Direction head will face--S is straight ahead,
						//		E is facing right, etc.
{
	static SDL_Rect SrcArray[ORIENTATION_COUNT] =
	{
		{52, 226, 85, 132},		//Northwest.
		{3, 225, 46, 133},		//North.
		{571, 226, 99, 131},	//Northeast.
		{137, 225, 117, 132},	//West.
		{0, 0, 0, 0},			//Illegal orientation.
		{459, 229, 111, 128},	//East.
		{256, 233, 75, 123},	//Southwest.
		{331, 232, 48, 126},	//South.
		{378, 235, 79, 122}		//Southeast.
	};
	const int xOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-33,  -24,  -65,  -47,  0, -67,  -41,  -23,  -40
	};
	const int yOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-120, -120,	-119, -123, 0, -117, -112, -111, -109
	};

	SDL_Rect Dest = {nX + xOffset[wO], nY + yOffset[wO], SrcArray[wO].w, SrcArray[wO].h};
	SDL_BlitSurface(this->pHeadsSurface, &SrcArray[wO], GetDestSurface(), &Dest);
}

//*****************************************************************************
void CTitleScreen::DrawThirdHead(
//Blits third head to screen.
//
//Params:
	int nX, int nY,		//(in)	Screen coords to center head on.
	UINT wO)			//(in)	Direction head will face--S is straight ahead,
						//		E is facing right, etc.
{
	static SDL_Rect SrcArray[ORIENTATION_COUNT] =
	{
		{30, 361, 51, 79},	//Northwest.
		{1, 361, 28, 79},	//North.
		{342, 361, 59, 79},	//Northeast.
		{82, 361, 70, 78},	//West.
		{0, 0, 0, 0},		//Illegal orientation.
		{275, 363, 66, 77},	//East.
		{153, 365, 45, 74},	//Southwest.
		{198, 365, 29, 75},	//South.
		{228, 366, 47, 73}	//Southeast.
	};
	const int xOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-20,  -14,  -39,  -28,  0, -40,  -24,  -14,  -24
	};
	const int yOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-72,  -72,	-71,  -74,  0, -70,  -67,  -67,  -66
	};

	SDL_Rect Dest = {nX + xOffset[wO], nY + yOffset[wO], SrcArray[wO].w, SrcArray[wO].h};
	SDL_BlitSurface(this->pHeadsSurface, &SrcArray[wO], GetDestSurface(), &Dest);
}

//*****************************************************************************
void CTitleScreen::DrawFourthHead(
//Blits fourth head to screen.
//
//Params:
	int nX, int nY,		//(in)	Screen coords to center head on.
	UINT wO)			//(in)	Direction head will face--S is straight ahead,
						//		E is facing right, etc.
{
	static SDL_Rect SrcArray[ORIENTATION_COUNT] =
	{
		{457, 394, 29, 46},	//Northwest.
		{440, 393, 17, 47},	//North.
		{639, 394, 34, 46},	//Northeast.
		{487, 393, 41, 47},	//West.
		{0, 0, 0, 0},		//Illegal orientation.
		{600, 395, 38, 45},	//East.
		{529, 397, 26, 43},	//Southwest.
		{555, 396, 17, 44},	//South.
		{572, 396, 28, 44}	//Southeast.
	};
	const int xOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-12,  -9,   -24,  -17,  0, -25,  -15,  -8,   -15
	};
	const int yOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-44,  -44,	-43,  -45,  0, -42,  -41,  -41,  -40
	};

	SDL_Rect Dest = {nX + xOffset[wO], nY + yOffset[wO], SrcArray[wO].w, SrcArray[wO].h};
	SDL_BlitSurface(this->pHeadsSurface, &SrcArray[wO], GetDestSurface(), &Dest);
}

//*****************************************************************************
void CTitleScreen::DrawFifthHead(
//Blits fifth head to screen.
//
//Params:
	int nX, int nY,		//(in)	Screen coords to center head on.
	UINT wO)			//(in)	Direction head will face--S is straight ahead,
						//		E is facing right, etc.
{
	static SDL_Rect SrcArray[ORIENTATION_COUNT] =
	{
		{710, 412, 17, 26},	//Northwest.
		{701, 412, 10, 26},	//North.
		{815, 412, 19, 26},	//Northeast.
		{728, 412, 23, 26},	//West.
		{0, 0, 0, 0},		//Illegal orientation.
		{793, 413, 21, 25},	//East.
		{752, 414, 14, 24},	//Southwest.
		{767, 413, 9, 25},	//South.
		{776, 414, 16, 24}	//Southeast.
	};
	const int xOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-7,   -5,   -14,  -10,  0, -15,  -9,   -5,   -9
	};
	const int yOffset[ORIENTATION_COUNT] =
	{
		//NW   N     NE    W    -   E     SW    S     SE
		-26,  -26,	-25,  -27,  0, -25,  -24,  -24,  -24
	};

	SDL_Rect Dest = {nX + xOffset[wO], nY + yOffset[wO], SrcArray[wO].w, SrcArray[wO].h};
	SDL_BlitSurface(this->pHeadsSurface, &SrcArray[wO], GetDestSurface(), &Dest);
}

//*****************************************************************************
void CTitleScreen::DrawBrick(
//Blits one of the menu item bricks to its associated destination on the screen.
//Brick will be in normal state.
//
//Params:
	TitleSelection wMenuPos)	//(in)	One of MNU_* constants indicating which brick.
{
	static SDL_Rect RectArray[MNU_COUNT] =
	{
		{424,86,126,50},	//MNU_CONTINUE
		{544,64,96,59},		//MNU_RESTORE
		{454,65,98,26},		//MNU_SETTINGS
		{490,200,79,69},	//MNU_HELP
		{589,154,51,53},	//MNU_DEMO
		{516,287,73,44},	//MNU_QUIT
		{481,114,119,101},	//MNU_BUILD
		{504,253,67,45},	//MNU_WHO
		{570,254,53,34}		//MNU_WHERE
	};

	if (wMenuPos==MNU_RESTORE && !this->bSavedGameExists) 
		BlitEmptyRestoreBrick();
	else		
		SDL_BlitSurface(this->pTitleSurface, &RectArray[wMenuPos], 
				GetDestSurface(), &RectArray[wMenuPos]);

	UpdateRect(RectArray[wMenuPos]);
}

//*****************************************************************************
void CTitleScreen::DrawPushedBrick(
//Blits one of the menu item bricks to its associated destination on the screen.
//Brick will be in pushed state.
//
//Params:
	TitleSelection wMenuPos)	//(in)	One of MNU_* constants indicating which brick.
{
	ASSERT(!(wMenuPos == MNU_RESTORE && !this->bSavedGameExists));

	static SDL_Rect SrcArray[MNU_COUNT] =
	{
		{641,69,126,50},	//MNU_CONTINUE
		{733,120,96,59},	//MNU_RESTORE
		{733,0,98,26},		//MNU_SETTINGS
		{641,173,79,69},	//MNU_HELP
		{794,229,51,53},	//MNU_DEMO
		{721,240,73,44},	//MNU_QUIT
		{732,370,119,101},	//MNU_BUILD
		{641,381,67,45},	//MNU_WHO
		{768,56,53,34}		//MNU_WHERE
	};
	static SDL_Rect DestArray[MNU_COUNT] =
	{
		{424,86,126,50},	//MNU_CONTINUE
		{544,65,96,59},		//MNU_RESTORE
		{454,65,98,26},		//MNU_SETTINGS
		{490,200,79,69},	//MNU_HELP
		{589,154,51,53},	//MNU_DEMO
		{516,287,73,44},	//MNU_QUIT
		{481,114,119,101},	//MNU_BUILD
		{504,253,67,45},	//MNU_WHO
		{570,254,53,34}		//MNU_WHERE
	};

	ASSERT(SrcArray[wMenuPos].w == DestArray[wMenuPos].w &&
		SrcArray[wMenuPos].h == DestArray[wMenuPos].h);
	SDL_BlitSurface(this->pTitleSurface, &SrcArray[wMenuPos], 
			GetDestSurface(), &DestArray[wMenuPos]);
	UpdateRect(DestArray[wMenuPos]);
}

//*****************************************************************************
void CTitleScreen::DrawHighlightedBrick(
//Blits one of the menu item bricks to its associated destination on the screen.
//Brick will be in highlighted state.
//
//Params:
	TitleSelection wMenuPos)	//(in)	One of MNU_* constants indicating which brick.
{
	if (wMenuPos == MNU_RESTORE && !this->bSavedGameExists) return;

	static SDL_Rect SrcArray[MNU_COUNT] =
	{
		{641,27,117,41},	//MNU_CONTINUE
		{641,119,92,53},	//MNU_RESTORE
		{641,0,92,22},		//MNU_SETTINGS
		{721,179,71,61},	//MNU_HELP
		{793,179,47,50},	//MNU_DEMO
		{641,243,66,39},	//MNU_QUIT
		{734,285,100,84},	//MNU_BUILD
		{641,339,60,41},	//MNU_WHO
		{768,27,52,28}		//MNU_WHERE
	};
	static SDL_Rect DestArray[MNU_COUNT] =
	{
		{430,90,117,41},	//MNU_CONTINUE
		{548,68,92,53},		//MNU_RESTORE
		{456,67,92,22},		//MNU_SETTINGS
		{494,204,71,61},	//MNU_HELP
		{593,156,47,50},	//MNU_DEMO
		{519,290,66,39},	//MNU_QUIT
		{494,119,100,84},	//MNU_BUILD
		{510,254,60,41},	//MNU_WHO
		{570,255,52,28}		//MNU_WHERE
	};

	if (wMenuPos == MNU_RESTORE && !this->bSavedGameExists)
		return;

	ASSERT(SrcArray[wMenuPos].w == DestArray[wMenuPos].w &&
		SrcArray[wMenuPos].h == DestArray[wMenuPos].h);
	SDL_BlitSurface(this->pTitleSurface, &SrcArray[wMenuPos], 
			GetDestSurface(), &DestArray[wMenuPos]);
	UpdateRect(DestArray[wMenuPos]);
}

//*****************************************************************************
TitleSelection CTitleScreen::GetMenuPosFromCoords(
//Figure out what menu brick coords are within, if any.
//
//Params:
	int nX, int nY) //(in) Screen coords to evaluate.
//
//Returns:
//MNU_* constant, MNU_UNSPECIFIED if coords aren't over any menu brick.
const
{
	static SDL_Rect BrickArray[MNU_COUNT] =
	{
		{442,89,104,28},	//MNU_CONTINUE
		{549,70,91,50},		//MNU_RESTORE
		{465,70,82,18},		//MNU_SETTINGS
		{503,206,62,47},	//MNU_HELP
		{597,157,43,26},	//MNU_DEMO
		{528,291,56,35},	//MNU_QUIT
		{518,119,76,64},	//MNU_BUILD
		{517,254,53,36},	//MNU_WHO
		{574,256,47,25}		//MNU_WHERE
	};

	for (int wMenuPos = 0; wMenuPos < MNU_COUNT; wMenuPos++)
	{
		if (IS_IN_RECT(nX, nY, BrickArray[wMenuPos]))
			return (TitleSelection)wMenuPos; //Match.
	}

	//No match.
	return MNU_UNSPECIFIED;
}


//*****************************************************************************
DWORD CTitleScreen::GetNextDemoID()
//Returns:
//DemoID of next demo in sequence to show or 0L if there are no demos to show.
{
	if (!this->pCurrentDemoID)
	{
		//Start from the first demo.
		this->pCurrentDemoID = this->ShowSequenceDemoIDs.Get(0);
		if (!this->pCurrentDemoID)
			return 0L; //No demos to show.
	}

	const DWORD dwFirstDemoID = this->pCurrentDemoID->dwID;
	DWORD dwRetDemoID;	//This demo ID is returned.

	//Advance to next demo ID (for play next time), looping back to start if needed.
	//NOTE: Only select demos from currently selected hold.
	const DWORD dwCurrentHoldID = g_pTheDB->GetHoldID();
	DWORD dwDemoHoldID;
	do	{
		//Skip demos not belonging to the current hold.
		dwRetDemoID = this->pCurrentDemoID->dwID;
		dwDemoHoldID = g_pTheDB->Demos.GetHoldIDofDemo(dwRetDemoID);

		//Get next demo marked for show (for next time).
		this->pCurrentDemoID = this->pCurrentDemoID->pNext;
		if (!this->pCurrentDemoID)
			this->pCurrentDemoID = this->ShowSequenceDemoIDs.Get(0);
	} while (dwDemoHoldID != dwCurrentHoldID &&
			this->pCurrentDemoID->dwID != dwFirstDemoID);	//haven't wrapped around

	if (this->pCurrentDemoID->dwID == dwFirstDemoID &&
			dwDemoHoldID != dwCurrentHoldID)
		dwRetDemoID = 0L;	//No demos for this hold were found.

	return dwRetDemoID;
}

//*****************************************************************************
void CTitleScreen::SetSavedGameExists()
//Sets this->bSavedGameExists (whether Restore option is available).
{
   CDb db;
	CIDList SavedGameIDs;
	db.SavedGames.FilterByHold(g_pTheDB->GetHoldID());
   db.SavedGames.FilterByPlayer(g_pTheDB->GetPlayerID());
	db.SavedGames.GetIDs(SavedGameIDs);
	//Saved games for continue slot and demos won't be counted since they are hidden.
	this->bSavedGameExists = (SavedGameIDs.GetSize() != 0);
}

//See deleted Title.cpp for source code that has been reused in this file.

// $Log: TitleScreen.cpp,v $
// Revision 1.79  2005/03/15 22:15:15  mrimer
// Alt-F4 now exits instantly.
//
// Revision 1.78  2003/08/19 20:11:29  mrimer
// Linux maintenance (committed on behalf of Gerry JJ).
//
// Revision 1.77  2003/08/16 03:57:55  mrimer
// Now tool tip is removed when mouse leaves brick region.
//
// Revision 1.76  2003/08/16 01:54:19  mrimer
// Moved EffectList to FrontEndLib.  Added general tool tip support to screens.  Added RoomEffectList to room editor screen/widget.
//
// Revision 1.75  2003/07/31 21:49:02  schik
// Added changing of playback speed of demos (except those run from the title screen)
//
// Revision 1.74  2003/07/22 19:00:27  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.73  2003/07/11 17:30:53  mrimer
// Removed confirmation dialog when hitting ESC to exit.
//
// Revision 1.72  2003/07/09 21:08:34  mrimer
// Unload current game when going to screens that could invalidate it.
//
// Revision 1.71  2003/07/07 23:25:45  mrimer
// Now unload current game when entering editor.
//
// Revision 1.70  2003/07/03 21:45:24  mrimer
// Hitting F1 now brings up help from title screen.
//
// Revision 1.69  2003/07/03 08:11:19  mrimer
// Simplify player selection logic (assume there's always an active player at title screen).
//
// Revision 1.68  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
// Revision 1.67  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.66  2003/06/26 17:47:24  mrimer
// Revised SetForActivate().
//
// Revision 1.65  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.64  2003/06/17 18:17:40  mrimer
// Fixed exit bug.
//
// Revision 1.63  2003/06/09 19:27:47  mrimer
// Added prompt for adding a hold when going to the editor and no editable holds are present.
//
// Revision 1.62  2003/06/06 18:16:56  mrimer
// Revised some SDL calls to call our wrapper functions.
//
// Revision 1.61  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.60  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.59  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.58  2003/05/21 03:07:39  mrimer
// Fixed some logic bugs.
//
// Revision 1.57  2003/05/19 20:29:28  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.56  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.55  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.54  2003/04/29 11:14:08  mrimer
// Fixed a screen transition bug.
//
// Revision 1.53  2003/04/28 22:21:52  mrimer
// Fixed a bug.
//
// Revision 1.52  2003/04/13 02:03:17  mrimer
// Changed show demo selection to only show demos from the current hold.
// Will reload the demo show list when a hold author plays their hold (possibly altering the show list).
//
// Revision 1.51  2003/02/24 20:41:01  erikh2000
// Revised logic so that GetSavedGame() does not automatically attempt to load a new game when a saved game is missing.  In some cases, this isn't appropriate, so I just fail and let the caller load a new game if he wants to.
//
// Revision 1.50  2003/02/01 23:53:53  erikh2000
// Fixed errors in rect coordinates.
// Changed hot keys to match button graphics.
//
// Revision 1.49  2003/01/26 01:32:28  erikh2000
// Updated coords for new build, who, and where bricks.
//
// Revision 1.48  2002/12/22 02:17:00  mrimer
// Removed hold selection code to CHoldSelectScreen.
//
// Revision 1.47  2002/11/22 02:13:17  mrimer
// Added SetSavedGameExists() to handle saved games for multiple holds.  Updated hold selection code.
//
// Revision 1.46  2002/11/15 02:59:40  mrimer
// Added non-visual handling of Edit, Who and Where selections.  Added SetHoldID().  Made selections into an enumeration.
//
// Revision 1.45  2002/10/22 16:31:41  mrimer
// Revised includes.
//
// Revision 1.44  2002/10/16 23:04:45  mrimer
// Added calls to base event-handling methods to make mouse showing/hiding the same as on other screens.
//
// Revision 1.43  2002/10/10 01:31:24  erikh2000
// Removed F8 cheat key.
//
// Revision 1.42  2002/10/04 17:59:20  mrimer
// Refactored quit logic into OnQuit().
//
// Revision 1.41  2002/10/03 22:44:31  mrimer
// Added show mouse cursor on activation.
//
// Revision 1.40  2002/09/30 16:55:28  mrimer
// Made some vars const and/or static.
//
// Revision 1.39  2002/09/03 21:40:33  erikh2000
// Added a cheat key to go right to the win screen for easy testing.
//
// Revision 1.38  2002/08/25 20:47:55  erikh2000
// Fixed rect coordinate errors for the spinning heads.
//
// Revision 1.37  2002/08/25 20:15:50  erikh2000
// Added code to draw empty restore brick.
//
// Revision 1.36  2002/08/23 23:55:16  erikh2000
// Sound effect plays when brick is pushed.
// Some constants renamed.
//
// Revision 1.35  2002/08/01 17:27:13  mrimer
// Revised some screen transition and music setting logic.
//
// Revision 1.34  2002/07/19 20:44:09  mrimer
// Added disabling/hiding Restore button when there are no saved games.
//
// Revision 1.33  2002/07/17 18:08:05  mrimer
// Added ALT-ENTER to change screen size.
//
// Revision 1.32  2002/07/16 18:28:34  mrimer
// <Enter> and <space> will now activate Continue as well as 'c'.
//
// Revision 1.31  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.30  2002/06/24 21:21:44  mrimer
// Fixed ESCAPE key behavior.
//
// Revision 1.29  2002/06/23 10:57:46  erikh2000
// When continuing a game, the next screen will either be the game screen or the level start screen, whichever is appropriate.
//
// Revision 1.28  2002/06/21 01:28:00  erikh2000
// Title screen now plays demos from show sequence.
//
// Revision 1.27  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.26  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.25  2002/06/14 17:50:35  mrimer
// Fixed clipping problem on left edge of heads.
//
// Revision 1.24  2002/06/13 21:46:41  mrimer
// Removed "new game" button.
//
// Revision 1.23  2002/06/11 22:49:59  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.22  2002/06/07 23:00:09  mrimer
// Added any event resetting the countdown to demo display timer.
//
// Revision 1.21  2002/06/07 17:53:59  mrimer
// Changed buttons to active on mouse up.
// Added showing brick being pushed when hotkey is pressed.
// Fixed Settings and Help hotkeys to match new titlescreen graphic.
//
// Revision 1.20  2002/06/02 10:48:23  erikh2000
// Brand new title screen--extensive changes.
//
// Revision 1.19  2002/05/21 22:54:29  erikh2000
// If there are no demos, demo screen will not be loaded.
//
// Revision 1.18  2002/05/21 21:48:59  mrimer
// Added switch to demo after short wait.
//
// Revision 1.17  2002/05/21 21:38:28  erikh2000
// Added stub method to return next demo ID in sequence.
//
// Revision 1.16  2002/05/15 01:32:37  erikh2000
// Selecting "Demo" will now go the demo screen and play back the last demo.
//
// Revision 1.15  2002/05/14 20:33:52  mrimer
// Reenabled and tested browser help.
//
// Revision 1.14  2002/05/12 03:23:12  erikh2000
// Fixed bug with screen redrawn incorrect when switching to fullscreen.
//
// Revision 1.13  2002/05/10 22:43:09  erikh2000
// Reenabled "really quit?" messages.
//
// Revision 1.12  2002/04/29 00:37:14  erikh2000
// Disabled "really quit?" dialog because cursor was missing and I couldn't click on it.
//
// Revision 1.11  2002/04/25 18:25:07  mrimer
// Added quit confirmation message and window resizing event.
// Set mouse cursor to custom image.
// Added hotkey to switch screen modes.
//
// Revision 1.10  2002/04/25 09:32:08  erikh2000
// Music plays when screen is activated.
//
// Revision 1.9  2002/04/20 08:25:00  erikh2000
// Added an include to get file to compile.
//
// Revision 1.8  2002/04/19 21:44:52  erikh2000
// Renamed SDL event handling methods to prevent override compile error.
//
// Revision 1.7  2002/04/14 00:38:38  erikh2000
// Changed calls to GetSurfaceColor().
//
// Revision 1.6  2002/04/13 19:38:41  erikh2000
// Renamed DisplayErrorMessage calls to "ShowOkMessage".
//
// Revision 1.5  2002/04/12 22:52:23  erikh2000
// Changed bitmap loading to use new reference-counting methods of CBitmapManager.
//
// Revision 1.4  2002/04/11 10:15:09  erikh2000
// Changed bitmap loading to use CBitmapManager.
//
// Revision 1.3  2002/04/10 00:29:04  erikh2000
// Menu will now navigate to restore and settings screens.
//
// Revision 1.2  2002/04/09 10:05:40  erikh2000
// Fixed revision log macro.
//
