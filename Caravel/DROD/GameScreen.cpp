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
 * Richard Cookney (timeracer), JP Burford (jpburford), John Wm. Wicks (j_wicks),
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//#define ENABLE_CHEATS

#include "GameScreen.h"
#include "Browser.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodScreenManager.h"
#include "DrodSound.h"
#include "TileImageCalcs.h"

#include "BloodEffect.h"
#include "CheckpointEffect.h"
#include "DebrisEffect.h"
#include "StrikeOrbEffect.h"
#include "TarStabEffect.h"
#include "TrapdoorFallEffect.h"
#include "SwordsmanSwirlEffect.h"

#include "FaceWidget.h"
#include "MapWidget.h"
#include "RoomWidget.h"
#include <FrontEndLib/Fade.h>
#include <FrontEndLib/FlashMessageEffect.h>
#include <FrontEndLib/BumpObstacleEffect.h>
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/DialogWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include "../Texts/MIDs.h"

#include "../DRODLib/GameConstants.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbRooms.h"
#include "../DRODLib/DbPlayers.h"
#include "../DRODLib/DbXML.h"
#include "../DRODLib/Monster.h"
#include "../DRODLib/CueEvents.h"
#include "../DRODLib/MonsterFactory.h"
#include "../DRODLib/CueEvents.h"
#include "../DRODLib/Mimic.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/IDList.h>
#include <BackEndLib/CoordIndex.h>
#include <BackEndLib/CoordStack.h>
#include <BackEndLib/Wchar.h>
#include <BackEndLib/Ports.h>

static const UINT MAX_REPEAT_RATE = 60;

//
//CGameScreen public methods.
//

//*****************************************************************************
bool CGameScreen::LoadContinueGame()
//Loads current game from current player's continue saved game slot.
//
//Returns:
//True if successful, false if not.
{
	//Load the game.
	const DWORD dwContinueID = g_pTheDB->SavedGames.FindByContinue();
	if (!dwContinueID) return false;
	return LoadSavedGame(dwContinueID);
}

//*****************************************************************************
bool CGameScreen::LoadSavedGame(
//Loads current game from a saved game.
//
//Params:
	const DWORD dwSavedGameID,	//(in)	Saved game to load.
	bool bRestoreFromStart)	//(in)	If true, game will be restored from start
							//		without playing back commands.  Default is
							//		false.
//
//Returns:
//True if successful, false if not.
{
	//Get rid of current game if needed.
	delete this->pCurrentGame;

	//Load the game.
	this->bIsSavedGameStale = false;
	CCueEvents Ignored;
	this->pCurrentGame = g_pTheDB->GetSavedCurrentGame(dwSavedGameID, Ignored,
			bRestoreFromStart);
	if (!this->pCurrentGame)
		return false;

	this->bPlayTesting = false;

	this->pCurrentGame->SetHighlightRoomIDs(this->HighlightRoomIDs);
	SetSignTextToCurrentRoom();
	return true;
}

//*****************************************************************************
bool CGameScreen::LoadNewGame(
//Loads current game from beginning of specified hold.
//
//Returns:
//True if successful, false if not.
//
//Params:
	const DWORD dwHoldID)	//(in)
{
	//Get rid of current game if needed.
	delete this->pCurrentGame;

	//Load the game.
	this->bIsSavedGameStale = false;
	CCueEvents Ignored;
	this->pCurrentGame = g_pTheDB->GetNewCurrentGame(dwHoldID, Ignored);
	if (!this->pCurrentGame) return false;

	//Set current game for widgets.
	if (!this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame) ||
			!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		return false;

	this->bPlayTesting = false;

	this->pCurrentGame->SetHighlightRoomIDs(this->HighlightRoomIDs);
	SetSignTextToCurrentRoom();
	return true;
}

//*****************************************************************************
bool CGameScreen::ShouldShowLevelStart()
//Used by screen-changing code outside of CGameScreen to determine if the
//level start screen should be shown before the game screen.
//
//Returns:
//True if it should, false if not.
{
	if (this->bIsSavedGameStale) return false;

	//I only want the caller to go to the level start screen once for each
	//time a saved game is loaded.  The save game is now "stale" until
	//one of the saved game loading methods of this class is called.  Then
	//it will set the flag to false again.
	this->bIsSavedGameStale = true;

	return this->pCurrentGame->IsLevelStart();

}

//*****************************************************************************
void CGameScreen::SetMusicStyle(
//Changes the music to match style.  If music is already matching style, nothing
//will happen.
//
//Params:
	const UINT wStyle)
{
	if (GetScreenType() == SCR_Demo) return;

	if (this->pCurrentGame->IsCurrentLevelComplete())
	{
		g_pTheSound->PlaySong(SONGID_LEVELCOMPLETE);
		return;
	}

	CRoomScreen::SetMusicStyle(wStyle);
}

//*****************************************************************************
bool CGameScreen::TestRoom(
//Returns: whether room is successfully loaded for testing
//
//Params:
	const DWORD dwRoomID,	//(in) room to start in
	const UINT wX, const UINT wY, const UINT wO)	//(in) Starting position
{
	//Get rid of current game if needed.
	delete this->pCurrentGame;

	//Load the game.
	this->bIsSavedGameStale = false;
	CCueEvents Ignored;
	this->pCurrentGame = g_pTheDB->GetNewTestGame(dwRoomID, Ignored, wX, wY, wO);
	if (!this->pCurrentGame) return false;

	//Set current game for widgets.
	if (!this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame) ||
			!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		return false;

	this->bPlayTesting = true;

	SetSignTextToCurrentRoom();
	return true;
}

//*****************************************************************************
bool CGameScreen::UnloadGame()
//Deletes the current game.
//
//Returns true if successful.
{
	if (this->pCurrentGame)
	{
		//Save current game to continue slot, unless this is CGameScreen-derived
		//CDemoScreen, or after play-testing.
		if (CScreen::GetScreenType() == SCR_Game && !this->bPlayTesting)
      {
			this->pCurrentGame->SaveToContinue();
         g_pTheDB->Commit();
      }

		//Free current game.
		delete this->pCurrentGame;
		this->pCurrentGame = NULL;
	}
   this->pRoomWidget->ResetRoom();

	return true;
}

//
//CGameScreen protected methods.
//

//*****************************************************************************
CGameScreen::CGameScreen(const SCREENTYPE eScreen) : CRoomScreen(eScreen)
	, pCurrentGame(NULL)
	, pRoomWidget(NULL)

	, bShowLevelStartBeforeActivate(false)

   , pFaceWidget(NULL)

   , bIsSavedGameStale(false)
//Constructor.
{
	InitHighlightRoomIDs();
}

//******************************************************************************
bool CGameScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Caller didn't load a current game.
	if (!this->pCurrentGame) {ASSERTP(false, "Current game not set."); return false;}

	//Set current game for widgets.
	if (!this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame) ||
			!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		return false;

	SetSignTextToCurrentRoom();
   this->pFaceWidget->SetMood(Normal);
   const bool bOnScroll = this->pCurrentGame->pRoom->GetTSquare(
         this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY) == T_SCROLL;
   this->bIsScrollVisible = bOnScroll;
   this->pFaceWidget->SetReading(bOnScroll);
   if (bOnScroll)
   {
      this->pScrollLabel->SetText(this->pCurrentGame->pRoom->GetScrollTextAtSquare(
         this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY));
   }

	//Init the keysym-to-command map and load other player settings.
	ApplyPlayerSettings();

	//Never return to the restore screen.  Yank it out of the return list so
	//that we go back to the title screen instead.
	if (g_pTheSM->GetReturnScreenType() == SCR_Restore)
		g_pTheSM->RemoveReturnScreen();

	//Set frame rate as high as needed for smooth animations.
	SetBetweenEventsInterval(12);

	SetMusicStyle(this->pCurrentGame->pRoom->wStyle);
	SwirlEffect();

	SelectFirstWidget(false);

	return true;
}

//*****************************************************************************
bool CGameScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	static const int X_ROOM = 102;
	static const int Y_ROOM = 25;
	static const int X_FACE = 17;
	static const int Y_FACE = 10;

	ASSERT(!this->bIsLoaded);

	if (!CRoomScreen::Load(this->pCurrentGame))
		return false;

	//Add widgets.
	this->pRoomWidget = new CRoomWidget(TAG_ROOM, X_ROOM, Y_ROOM,
			CDrodBitmapManager::CX_ROOM, CDrodBitmapManager::CY_ROOM,
			pCurrentGame);
	AddWidget(this->pRoomWidget);

	this->pFaceWidget = new CFaceWidget(0L, X_FACE, Y_FACE, CX_FACE, CY_FACE);
	AddWidget(this->pFaceWidget);

	//Load children.
	this->bIsLoaded = LoadChildren();

	return this->bIsLoaded;
}

//*****************************************************************************
void CGameScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	//Unload children.
	UnloadChildren();

	UnloadGame();

	CRoomScreen::Unload();

	this->bIsLoaded = false;
}

//*****************************************************************************
void CGameScreen::OnClick(
//Called when widget receives a mouse click event.
//
//Params:
	const DWORD dwTagNo)	//(in) Widget that event applied to.
{
	switch (dwTagNo)
	{
		case TAG_ROOM:
			if (!RightMouseButton()) break;

         DisplayRoomStats();
		break;
	}
}

//*****************************************************************************
void CGameScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD dwTagNo,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Key event.
{
#ifdef ENABLE_CHEATS
	CCueEvents Ignored;
	bool bExitRoomForCheat = false;
#endif

	CScreen::OnKeyDown(dwTagNo, Key);

	//Check for a game command.
	if (Key.keysym.sym > -1 && Key.keysym.sym < SDLK_LAST)
	{
		const int nCommand = this->KeysymToCommandMap[Key.keysym.sym];
		if (nCommand != CMD_UNSPECIFIED)
		{
			SCREENTYPE eNextScreen = ProcessCommand(nCommand);
			if (eNextScreen != SCR_Game)
			{
				GoToScreen(eNextScreen);
				return;
			}
		}
	}

	//Check for other keys.
	switch (Key.keysym.sym)
	{
		case SDLK_F1:
			SetFullScreen(false);
			ShowHelp("quickstart.html");
		break;

		case SDLK_F5:
         if (this->bPlayTesting) break;
			if (this->pCurrentGame->IsDemoRecording())
			{
				//End recording and save demo.
				const DWORD dwTagNo = this->pCurrentGame->EndDemoRecording();
				SetSignTextToCurrentRoom();
				PaintSign();

				if (!dwTagNo)
					ShowOkMessage(MID_DemoNotSaved);
				else if (dwTagNo != TAG_ESCAPE)
					ShowOkMessage(MID_DemoSaved);
			}
			else
			{
				WSTRING wstrDescription = this->pCurrentGame->AbbrevRoomLocation();
				const DWORD dwTagNo = ShowTextInputMessage(MID_DescribeDemo,
                  wstrDescription);
				if (dwTagNo == TAG_OK)
				{
					this->pCurrentGame->BeginDemoRecording( (wstrDescription.size()==0) ?
							wszEmpty : wstrDescription.c_str() );

					//Repaint sign to show new recording status.
					SetSignTextToCurrentRoom();
					PaintSign();
				}
			}
		break;

		case SDLK_F6:
         if (!this->bPlayTesting)
			   GoToScreen(SCR_Demos);
		break;

      case SDLK_RETURN:
         if (!(Key.keysym.mod & (KMOD_ALT|KMOD_CTRL)))
            DisplayRoomStats();
      break;

#ifdef ENABLE_CHEATS
		//cheat keys
		case SDLK_F8:
		{
			const DWORD dwNextLevelID = this->pCurrentGame->pLevel->GetDefaultNextLevelID();
			if (dwNextLevelID == 0)
			{
				delete this->pCurrentGame;
				this->pCurrentGame = g_pTheDB->GetNewCurrentGame(
						g_pTheDB->GetHoldID(), Ignored);
				this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame);
				this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame);
			}
			else
			{
				this->pCurrentGame->LoadFromLevel(dwNextLevelID, Ignored);
				this->pRoomWidget->UpdateFromCurrentGame();
				this->pMapWidget->UpdateFromCurrentGame();
			}
			SetSignTextToCurrentRoom();
			SetMusicStyle(this->pCurrentGame->pRoom->wStyle);
			PaintSign();
			this->pMapWidget->Paint();
			this->pRoomWidget->Paint();
			this->pFaceWidget->Paint();
		}
		break;

		case SDLK_F7:
			this->pRoomWidget->ToggleFrameRate();
		break;

      case SDLK_BACKSPACE:	//undo a move
         this->pCurrentGame->UndoCommand(Ignored);
			this->pRoomWidget->ResetForPaint();
			this->pRoomWidget->Paint();
			this->pFaceWidget->Paint();
      break;

      case SDLK_BACKSLASH:	//conquer room
			if (!pCurrentGame->IsCurrentRoomConquered())
			{
				this->pCurrentGame->SetCurrentRoomConquered();
				this->pCurrentGame->RestartRoom(Ignored);
				this->pRoomWidget->ResetForPaint();
				bExitRoomForCheat = true;
			}
		break;

		case SDLK_UP: //Up arrow.
			if (pCurrentGame->SetSwordsmanToNorthExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(N, Ignored);
			}
		break;

		case SDLK_DOWN:	//Down arrow.
			if (pCurrentGame->SetSwordsmanToSouthExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(S, Ignored);
			}
		break;

		case SDLK_LEFT: //Left arrow.
			if (pCurrentGame->SetSwordsmanToWestExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(W, Ignored);
			}
		break;

		case SDLK_RIGHT: //Right arrow.
			if (pCurrentGame->SetSwordsmanToEastExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(E, Ignored);
			}
		break;
#endif

      default: break;
	}

#ifdef ENABLE_CHEATS
	if (bExitRoomForCheat)
	{
		if (!g_pTheDB->SavedGames.FindByRoomBegin(this->pCurrentGame->pRoom->dwRoomID))
			this->pCurrentGame->SaveToRoomBegin();
		this->pRoomWidget->UpdateFromCurrentGame();
		this->pMapWidget->UpdateFromCurrentGame();
		SetSignTextToCurrentRoom();
		PaintSign();
		HideScroll();
      this->pFaceWidget->SetReading(false);
		this->pMapWidget->Paint();
		this->pRoomWidget->Paint();
		this->pFaceWidget->Paint();
	}
#endif

   //Save to the continue slot whenever leaving the game screen.
   if (IsDeactivating())
   {
		this->pCurrentGame->SaveToContinue();
      g_pTheDB->Commit();
   }
}

//*****************************************************************************
void CGameScreen::SwirlEffect()
//Swirl effect to highlight swordsman.
{
	this->pRoomWidget->AddTLayerEffect(
			new CSwordsmanSwirlEffect(this->pRoomWidget, this->pCurrentGame));
}

//
//CGameScreen private methods.
//

//*****************************************************************************
void CGameScreen::AddHighlightDemoInfoToPlayerSettings(
//Adds a var to player settings that contains info needed to retrieve a
//highlight demo for a certain room.
//
//Params:
	const CHighlightDemoInfo *pHDI)	//(in)
{
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
   ASSERT(pCurrentPlayer);

	//Variable name is "hd" + room ID demo was recorded for, i.e. "hd23" is
	//a demo for room ID #23.  The value is the demo ID of the recorded demo.
	string strVarName = "hd";
	char szRoomID[11];
	strVarName += _ltoa(pHDI->dwRoomID, szRoomID, 10);

	pCurrentPlayer->Settings.SetVar(strVarName.c_str(), pHDI->dwDemoID);
	pCurrentPlayer->Update();

	delete pCurrentPlayer;
}

//*****************************************************************************
void CGameScreen::InitHighlightRoomIDs()
//Init the list of room IDs for which to save highlight demos.
{
	//This routine breaks a few rules by assuming that certain RoomIDs will be
	//assigned to rooms I am interested in.  But in the first release, it is
	//safe to assume this.

	this->HighlightRoomIDs.Add(3L);		//1:5N2W
	this->HighlightRoomIDs.Add(34L);	//2:2S3E
	this->HighlightRoomIDs.Add(36L);	//3:3N1E
	this->HighlightRoomIDs.Add(62L);	//4:2E
	this->HighlightRoomIDs.Add(57L);	//4:1N1E
	this->HighlightRoomIDs.Add(69L);	//5:2N2W
	this->HighlightRoomIDs.Add(96L);	//6:Entrance.
	this->HighlightRoomIDs.Add(109L);	//7:1N1E
	this->HighlightRoomIDs.Add(103L);	//7:2N1W
	this->HighlightRoomIDs.Add(134L);	//8:7S3E
	this->HighlightRoomIDs.Add(150L);	//10:1N1E
	this->HighlightRoomIDs.Add(165L);	//11:1S
	this->HighlightRoomIDs.Add(177L);	//12:1S2E
	this->HighlightRoomIDs.Add(200L);	//13:2S2W
	this->HighlightRoomIDs.Add(216L);	//14:1S
	this->HighlightRoomIDs.Add(228L);	//15:1S2W
	this->HighlightRoomIDs.Add(241L);	//16:2S1E
	this->HighlightRoomIDs.Add(247L);	//17:1N
	this->HighlightRoomIDs.Add(265L);	//18:1E
	this->HighlightRoomIDs.Add(278L);	//19:2S2E
	this->HighlightRoomIDs.Add(290L);	//20:Entrance.
	this->HighlightRoomIDs.Add(281L);	//20:3N1E
	this->HighlightRoomIDs.Add(292L);	//21:1N1W
	this->HighlightRoomIDs.Add(308L);	//22:1W
	this->HighlightRoomIDs.Add(322L);	//23:1S1W
	this->HighlightRoomIDs.Add(332L);	//24:1W
	this->HighlightRoomIDs.Add(344L);	//25:Entrance
	this->HighlightRoomIDs.Add(341L);	//25:1N
	this->HighlightRoomIDs.Add(340L);	//25:1N1W
	this->HighlightRoomIDs.Add(339L);	//25:1N2W
	this->HighlightRoomIDs.Add(342L);	//25:2W
	this->HighlightRoomIDs.Add(345L);	//25:1S2W
	this->HighlightRoomIDs.Add(348L);	//25:2S2W
	this->HighlightRoomIDs.Add(349L);	//25:2S1W
	this->HighlightRoomIDs.Add(350L);	//25:2S
	this->HighlightRoomIDs.Add(347L);	//25:1S
	this->HighlightRoomIDs.Add(346L);	//25:1S1W
	this->HighlightRoomIDs.Add(343L);	//25:1W

   CDbXML::info.highlightRoomIDs = this->HighlightRoomIDs;
}

//*****************************************************************************
void CGameScreen::ApplyPlayerSettings()
//Apply player settings to the game screen.
{
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	if (!pCurrentPlayer) {ASSERTP(false, "Couldn't retrieve current player."); return;} //Corrupt db.

	//Set the keysym-to-command map from player settings.
	InitKeysymToCommandMap(pCurrentPlayer->Settings);

	//Set times when saved games and demos are saved automatically.
   if (!this->bPlayTesting)
   {
	   const DWORD dwAutoSaveOptions = pCurrentPlayer->Settings.GetVar(
			   "AutoSaveOptions", ASO_DEFAULT);
	   this->pCurrentGame->SetAutoSaveOptions(dwAutoSaveOptions);
   }

	//Set room widget to either show checkpoints or not.
	if (pCurrentPlayer->Settings.GetVar("ShowCheckpoints", true)!=false)
		this->pRoomWidget->ShowCheckpoints();
	else
		this->pRoomWidget->HideCheckpoints();

	//Move repeat rate.
	const BYTE nRepeatRate = (((long)(pCurrentPlayer->Settings.GetVar("RepeatRate",
			(BYTE)127))) * (MAX_REPEAT_RATE-1) / 255) + 1;	//value from 1 to MAX
	const DWORD dwTimePerRepeat = 1000L / (DWORD)nRepeatRate;
	SetKeyRepeat(dwTimePerRepeat);

	delete pCurrentPlayer;
}

//*****************************************************************************
void CGameScreen::SetSignTextToCurrentRoom()
//Set sign text to description of current room and repaint it.
{
   static const WCHAR wszSignSep[] = {W_t(':'),W_t(' '),W_t(0)};
	WSTRING wstrSignText = (const WCHAR *)this->pCurrentGame->pLevel->NameText;
	wstrSignText += wszSignSep;
	this->pCurrentGame->pRoom->GetLevelPositionDescription(wstrSignText);
	if (this->pCurrentGame->IsDemoRecording())
	{
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_RecordingStatus);
	}
	SetSignText(wstrSignText.c_str());
}

//*****************************************************************************
SCREENTYPE CGameScreen::LevelExit_OnKeydown(
//Handles SDL_KEYDOWN events for the game screen when exiting level.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
//
//Returns:
//Screen to go to next or SCR_Game to remain at game screen.
{
	switch (KeyboardEvent.keysym.sym)
	{
      case SDLK_RALT: case SDLK_LALT:
         return SCR_Game;  //don't advance to next screen
      case SDLK_RETURN:
         if (KeyboardEvent.keysym.mod & KMOD_ALT)
			{
				ToggleScreenSize();
				return SCR_Game;  //changing screen size shouldn't advance to next screen
			}
			break;
		case SDLK_F10:
			ToggleScreenSize();
         return SCR_Game;  //changing screen size shouldn't advance to next screen

		case SDLK_F4:
			if (KeyboardEvent.keysym.mod & KMOD_ALT)
				return SCR_None;	//boss key -- exit immediately
		break;
      default: break;
	}

	return SCR_LevelStart;
}

//*****************************************************************************
void CGameScreen::DisplayRoomStats()
//Show dialog box displaying current game stats.
{
	CDialogWidget *pStatsBox = new CDialogWidget(0L, 0, 0, 200, 150);

	pStatsBox->AddWidget(
			new CLabelWidget(0L, 30, 10, 150, 22,
					F_Header, g_pTheDB->GetMessageText(MID_CurrentGameStats)));
	CButtonWidget *pButton = new CButtonWidget(TAG_OK, 65, 120,
			60, CY_STANDARD_BUTTON, g_pTheDB->GetMessageText(MID_Okay));
	pStatsBox->AddWidget(pButton);

	//Room info.
	pStatsBox->AddWidget(
			new CLabelWidget(0L, 20, 35, 130, 25,
					F_Message, g_pTheDB->GetMessageText(MID_MovesMade)));
	pStatsBox->AddWidget(
			new CLabelWidget(0L, 20, 60, 130, 25,
					F_Message, g_pTheDB->GetMessageText(MID_MonstersKilled)));
	pStatsBox->AddWidget(
			new CLabelWidget(0L, 20, 85, 130, 25,
					F_Message, g_pTheDB->GetMessageText(MID_SpawnCounter)));

	WSTRING text;
	WCHAR dummy[32];
	//Moves made (excludes mimic placement).
	text = _ltoW(this->pCurrentGame->wSpawnCycleCount, dummy, 10);
	pStatsBox->AddWidget(
			new CLabelWidget(0L, 140, 35, 50, 25,
					F_Message, text.c_str()));

	//Monsters killed.
	text = _ltoW(this->pCurrentGame->wMonsterKills, dummy, 10);
	pStatsBox->AddWidget(
			new CLabelWidget(0L, 140, 60, 50, 25,
					F_Message, text.c_str()));

	//Spawn cycle counter.
	text = _ltoW(TURNS_PER_CYCLE -
			(this->pCurrentGame->wSpawnCycleCount % TURNS_PER_CYCLE),
			dummy, 10);
	pStatsBox->AddWidget(
			new CLabelWidget(0L, 140, 85, 50, 25,
					F_Message, text.c_str()));

	//Display.
	this->pRoomWidget->AddWidget(pStatsBox,true);
	pStatsBox->Center();
	pStatsBox->Display();
	this->pRoomWidget->RemoveWidget(pStatsBox);
	this->pRoomWidget->DirtyRoom();
	this->pRoomWidget->Paint();
}

//*****************************************************************************
SCREENTYPE CGameScreen::HandleEventsForLevelExit()
//Plays level exit music, shows Beethro walking down the stairs, etc.
//Allow only certain key commands during this time.
//
//Note that the On*() handlers are not going to be called by CEventHandlerWidget's
//Activate() loop until after this method exits.  Events must be handled here.
//
//Returns:
//Screen to activate after this one.  SCR_None indicates an application exit,
//and SCR_Return indicates the screen previously activated.
{
	bool bDoneDescendingStairs = false;

	//Necessary to call explicitly because I am taking over event-handling.
	StopKeyRepeating();

	//Amount of time to wait before automatically continuing to level start screen.
	static const DWORD EXIT_DELAY = 20000L;

	//Amount of time to wait before fading the music volume.
	static const DWORD FADE_SONG_DURATION = 3000L;
	static const DWORD FADE_SONG_DELAY = EXIT_DELAY - FADE_SONG_DURATION;

	//Original volume is used to calculate volume fading increments and to restore
	//the volume when function exits.
	int nOriginalVolume = g_pTheSound->GetMusicVolume();

	//Play level exit music.
	if (this->pCurrentGame->pLevel->dwLevelID % 2 == 0)
		g_pTheSound->PlaySong(SONGID_WINLEVEL1);
	else g_pTheSound->PlaySong(SONGID_WINLEVEL2);

	HideScroll();
   this->pFaceWidget->SetReading(false);
	this->pFaceWidget->SetReading(false);
	this->pFaceWidget->SetMood(Happy);

	//Show the screen after first arriving here.
	Paint();
	DWORD dwLastStep = SDL_GetTicks(), dwLastAnimate = dwLastStep, dwStarted = dwLastStep;

	//Process events.
	SDL_Event event;
	SCREENTYPE eNextScreen;
	while (true)
	{
		//Get any events waiting in the queue.
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
            case SDL_ACTIVEEVENT:
               OnActiveEvent(event.active);
            break;

				case SDL_KEYDOWN:
					eNextScreen = LevelExit_OnKeydown(event.key);
					if (eNextScreen != SCR_Game)
					{
	               this->pFaceWidget->SetMood(Normal);
						g_pTheSound->StopSong();
						g_pTheSound->SetMusicVolume(nOriginalVolume);
						this->pRoomWidget->ShowSwordsman();
						return eNextScreen;
					}
				break;

				case SDL_MOUSEMOTION:
					OnMouseMotion(this->dwTagNo, event.motion);
				break;

				case SDL_QUIT:
					if (ShowYesNoMessage(MID_ReallyQuit) != TAG_NO)
						return SCR_None;
				break;
			}
		}

		//Show swordsman walking down stairs every 400ms.
		const Uint32 dwNow = SDL_GetTicks();
		if (!bDoneDescendingStairs && dwNow - dwLastStep > 400)
		{
			this->pRoomWidget->DirtyRoom();
			bDoneDescendingStairs = !this->pCurrentGame->WalkDownStairs();
			if (bDoneDescendingStairs)
			{
				//done walking down stairs (swordsman disappears)
				this->pRoomWidget->HideSwordsman();
				this->pRoomWidget->Paint();

				//Beethro's thinking about the next level now.
				this->pFaceWidget->SetMood(Normal);
			}
			dwLastStep = dwNow;
		}

		//Animate every so often.
		if (dwNow - dwLastAnimate > 100)
		{
			this->pRoomWidget->Paint();
			this->pFaceWidget->Paint();
			dwLastAnimate = dwNow;
		}

		//Automatically go to level start screen after max wait time.
		if (dwNow - dwStarted > EXIT_DELAY)
		{
			g_pTheSound->StopSong();
			g_pTheSound->SetMusicVolume(nOriginalVolume);
			this->pRoomWidget->ShowSwordsman();
			return SCR_LevelStart;
		}

		//Music volume fades down after delay.
		if (dwNow - dwStarted > FADE_SONG_DELAY)
		{
			const int nNewVolume = (EXIT_DELAY - dwNow + dwStarted) *
					nOriginalVolume / FADE_SONG_DURATION;
			g_pTheSound->SetMusicVolume(nNewVolume);
		}
	}
}

//*****************************************************************************
void CGameScreen::HandleEventsForPlayerDeath()
//Displays player death animation:
// wait a short period of time while the scream wave plays,
// a CBloodEffect spurts out of his body,
// and the face widget wags its tongue.
//End after the scream wave finishes
//Accept no commands during this period.
//
//Note that the On*() handlers are not going to be called by CEventHandlerWidget's
//Activate() loop until after this method exits.  Events must be handled here.
{
	static const Uint32 dwDeathDuration = 1275;
	//Necessary to call explicitly because I am taking over event-handling.
	StopKeyRepeating();

	CMoveCoord coord(this->pCurrentGame->swordsman.wX,
			this->pCurrentGame->swordsman.wY, NO_ORIENTATION);
	bool bSwordSwingsClockwise = true;

	//Show the screen after first arriving here.
	this->pRoomWidget->Repaint();
	this->pRoomWidget->Paint();
   {
   CFade fade(this->pRoomWidget->pRoomSnapshotSurface,NULL);

   this->pFaceWidget->SetReading(false);
	this->pFaceWidget->SetMood(Dieing);
	g_pTheSound->PlaySoundEffect(SEID_DIE);

	const DWORD dwStart = SDL_GetTicks();
   Uint32 dwLastFade = 0, dwLastSwordWobble = 0, dwLastMonsterChomp = 0;
	SDL_Event event;
	while (true)
	{
		//Get any events waiting in the queue.
		while (SDL_PollEvent(&event))
      {
 			switch (event.type)
			{
            case SDL_ACTIVEEVENT:
               OnActiveEvent(event.active);
            break;
            default: break;
         }
      }

		//Animate as fast as possible.
		const Uint32 dwNow = SDL_GetTicks();

		//Sword wobbles around.
		if (dwNow - dwLastSwordWobble > 50)
		{
			if (bSwordSwingsClockwise)
				this->pCurrentGame->swordsman.SetOrientation(
						nNextCO(this->pCurrentGame->swordsman.wO));
			else
				this->pCurrentGame->swordsman.SetOrientation(
						nNextCCO(this->pCurrentGame->swordsman.wO));
			//sometimes sword rotation changes direction
			if (RAND(3) == 0)
				bSwordSwingsClockwise = !bSwordSwingsClockwise;
         dwLastSwordWobble = dwNow;
		}

		//Fade to black.
		if (dwNow - dwLastFade > 100)
		{
			fade.IncrementFade((dwNow - dwStart) / (float)dwDeathDuration);
			this->pRoomWidget->DirtyRoom();	//repaint whole room each fade
         dwLastFade = dwNow;
		}

		//Monster chomps on Beethro.
		if (dwNow - dwLastMonsterChomp > 200)
		{
			//Animate monster.
			if (this->pRoomWidget->SwitchAnimationFrame(
					this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY))
			{
				//Blood effect.
				this->pRoomWidget->AddLastLayerEffect(	//must go on top to show up
					new CBloodEffect(this->pRoomWidget, coord, 16));
			}
         dwLastMonsterChomp = dwNow;
		}

		this->pFaceWidget->Paint();
		this->pRoomWidget->Paint();

		//Scream has finished.  Return from animation.
		if (dwNow - dwStart > dwDeathDuration)
			break;
	}
   }

	this->pFaceWidget->SetMood(Normal);
}

//*****************************************************************************
SCREENTYPE CGameScreen::ProcessCommand(int nCommand)
//Processes game command, making calls to update game data and respond to cue
//events.
//
//Returns:
//Screen to go to next or SCR_Game to remain at game screen.
{
	CCueEvents CueEvents;
	if (nCommand == CMD_RESTART)
	{
		//Restart the room.
      const bool bRoomConquered = this->pCurrentGame->IsCurrentRoomPendingExit();
		if (this->bRestartRoomAtBeginning)
			this->pCurrentGame->RestartRoom(CueEvents);
		else {
			this->pCurrentGame->RestartRoomFromLastCheckpoint(CueEvents);
			this->bRestartRoomAtBeginning = true;
		}
		this->pRoomWidget->ResetForPaint();
      //If current room was conquered when reset, the room must be redrawn on the map.
      if (bRoomConquered)
      {
         this->pMapWidget->UpdateFromCurrentLevel();
		   this->pMapWidget->Paint();
      }
      SetSignTextToCurrentRoom();
		PaintSign();
		this->pRoomWidget->ClearEffects();
		SwirlEffect();
	}
	else
	{
		//Send command to current game and cue events list back.
		ASSERT(this->pCurrentGame->bIsGameActive); //We should have reloaded a game before getting here.
		this->pCurrentGame->ProcessCommand(nCommand, CueEvents);
		this->bRestartRoomAtBeginning = false;
	}

	//Process cue events list to create effects that should occur before
	//room is drawn.
	const bool bPlayerDied = (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied),
			CIDA_PlayerDied));

	SCREENTYPE eNextScreen = ProcessCueEventsBeforeRoomDraw(CueEvents);
	if (eNextScreen == SCR_Game)
	{
		//Redraw the room.
		this->pRoomWidget->Paint();

		//Process cue events list to create effects that should occur after
		//room is drawn.
		eNextScreen = ProcessCueEventsAfterRoomDraw(CueEvents);

		if (bPlayerDied)
			SwirlEffect();	//must call after ProcessCueEventsBeforeRoomDraw()
   }

	return eNextScreen;
}

//*****************************************************************************
SCREENTYPE CGameScreen::ProcessCueEventsBeforeRoomDraw(
//Process cue events list to create effects that should occur before
//room is drawn.
//
//Params:
	CCueEvents &CueEvents) //(in)
//
//Returns:
//Screen to go to next or SCR_Game to remain at game screen.
{
	SCREENTYPE eNextScreen = SCR_Game;

	//Remember for later if player left room (dieing, level exit, room exit,
	//win game) because room reloading actions will erase cue events.
	const bool bPlayerLeftRoom = CueEvents.HasAnyOccurred(
			IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom);

	//Channel n+1 -- Swordsman's voice.
	if (CueEvents.HasOccurred(CID_HitObstacle))
	{
		g_pTheSound->PlaySoundEffect(SEID_OOF);
		CMoveCoord *pMoveCoord = DYN_CAST(CMoveCoord *, CAttachableObject *,
				CueEvents.GetFirstPrivateData(CID_HitObstacle) );
		this->pRoomWidget->RemoveTLayerEffectsOfType(EBUMPOBSTACLE);
		this->pRoomWidget->AddTLayerEffect(new CBumpObstacleEffect(this->pRoomWidget,
				pMoveCoord->wCol, pMoveCoord->wRow, pMoveCoord->wO));
	}
	else if (CueEvents.HasOccurred(CID_Scared))
		g_pTheSound->PlaySoundEffect(SEID_SCARED);
	else if (CueEvents.HasOccurred(CID_SwordsmanTired) && !CueEvents.HasOccurred(CID_AllMonstersKilled))
		g_pTheSound->PlaySoundEffect(SEID_TIRED);
	else if (CueEvents.HasOccurred(CID_AllMonstersKilled)) {
        this->pMapWidget->UpdateFromCurrentLevel();
		this->pMapWidget->Paint();
        if (!CueEvents.HasOccurred(CID_NeatherExitsRoom)) //Beethro isn't happy about the 'Neather getting away.
	    {
    		g_pTheSound->PlaySoundEffect(SEID_CLEAR);
    	}
    }

	//Channel n+2 -- 'Neather's voice.
	if (CueEvents.HasOccurred(CID_NeatherScared))
		g_pTheSound->PlaySoundEffect(SEID_NSCARED);
	else if (CueEvents.HasOccurred(CID_NeatherFrustrated))
		g_pTheSound->PlaySoundEffect(SEID_NFRUSTRATED);
   else if (CueEvents.HasOccurred(CID_NeatherLaughing))
		g_pTheSound->PlaySoundEffect(SEID_NLAUGHING);
	else if (CueEvents.HasOccurred(CID_EvilEyeWoke))
		g_pTheSound->PlaySoundEffect(SEID_EVILEYEWOKE);

	//Channel n+3.
	if (CueEvents.HasOccurred(CID_MonsterDiedFromStab))
	{
		bool bLastBrain=false;
		g_pTheSound->PlaySoundEffect(SEID_SPLAT);
		CMonster *pMonster = DYN_CAST(CMonster *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab) );
		while (pMonster)
		{
			CMoveCoord coord(pMonster->wX,pMonster->wY,pMonster->wO);
			if (pMonster->wType == M_TARBABY)
				this->pRoomWidget->AddTLayerEffect(
					new CTarStabEffect(this->pRoomWidget, coord));
			else
			{
				if (pMonster->wType == M_BRAIN &&
						this->pCurrentGame->pRoom->wBrainCount==0)
					bLastBrain=true;
				this->pRoomWidget->AddTLayerEffect(
					new CBloodEffect(this->pRoomWidget, coord));
			}
         pMonster = DYN_CAST(CMonster *, CAttachableObject *,
				   CueEvents.GetNextPrivateData());
		}
		if (bLastBrain) g_pTheSound->PlaySoundEffect(SEID_LASTBRAIN);
	}
	if (CueEvents.HasOccurred(CID_TarDestroyed))
	{
      //Green/clean map room might need to be reverted back to red
      //if tar was hit and tar babies were created in a clean room.
		if (!this->pCurrentGame->IsCurrentRoomConquered() &&	//and was it not already cleared?
            CueEvents.HasOccurred(CID_TarBabyFormed))
      {
         const UINT wNumTarBabiesFormed = CueEvents.GetOccurrenceCount(CID_TarBabyFormed);
         if (wNumTarBabiesFormed == this->pCurrentGame->pRoom->wMonsterCount)
         {
            //Room was clean, and now is not.
            this->pMapWidget->UpdateFromCurrentLevel();
		      this->pMapWidget->Paint();
         }
      }
		g_pTheSound->PlaySoundEffect(SEID_STABTAR);
		CMoveCoord *pCoord = DYN_CAST(CMoveCoord *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_TarDestroyed) );
		while (pCoord)
		{
			this->pRoomWidget->AddTLayerEffect(
					new CTarStabEffect(this->pRoomWidget, *pCoord));
			pCoord = DYN_CAST(CMoveCoord *, CAttachableObject *,
				   CueEvents.GetNextPrivateData());
		}
	}
	if (CueEvents.HasOccurred(CID_SnakeDiedFromTruncation))
	{
		g_pTheSound->PlaySoundEffect(SEID_SPLAT);
		CMonster *pMonster = DYN_CAST(CMonster *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_SnakeDiedFromTruncation) );
		while (pMonster)
		{
			CMoveCoord coord(pMonster->wX,pMonster->wY,pMonster->wO);
			this->pRoomWidget->AddTLayerEffect(
					new CBloodEffect(this->pRoomWidget, coord));
			pMonster = DYN_CAST(CMonster *, CAttachableObject *,
				   CueEvents.GetNextPrivateData());
		}
	}

	//Channel n+4.
	if (CueEvents.HasOccurred(CID_OrbActivated))
	{
		g_pTheSound->PlaySoundEffect(SEID_ORBHIT);
		COrbData *pOrbData = DYN_CAST(COrbData *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_OrbActivated) );
		while (pOrbData)
		{
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData);
			pOrbData = DYN_CAST(COrbData *, CAttachableObject *,
				   CueEvents.GetNextPrivateData());
		}
	}
	else if (CueEvents.HasOccurred(CID_OrbActivatedByMimic))
	{
		g_pTheSound->PlaySoundEffect(SEID_ORBHITMIMIC);
		COrbData *pOrbData = DYN_CAST(COrbData *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_OrbActivatedByMimic) );
		while (pOrbData)
		{
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData);
			pOrbData = DYN_CAST(COrbData *, CAttachableObject *,
				   CueEvents.GetNextPrivateData());
		}
	}

	//Channel n+5.
	if (CueEvents.HasOccurred(CID_Step))
		g_pTheSound->PlaySoundEffect(SEID_WALK);
	else if (CueEvents.HasOccurred(CID_MimicPlaced))
	{
		g_pTheSound->PlaySoundEffect(SEID_MIMIC);
		this->pRoomWidget->Repaint();	//remove mimic placement effect
	}

	//Channel n+6.
	if (CueEvents.HasOccurred(CID_SwingSword))
		g_pTheSound->PlaySoundEffect(SEID_SWING);
	else if (CueEvents.HasOccurred(CID_DrankPotion))
		g_pTheSound->PlaySoundEffect(SEID_POTION);

	//Separate channels.
	if (CueEvents.HasOccurred(CID_StepOnScroll))
		g_pTheSound->PlaySoundEffect(SEID_READ);

	if (CueEvents.HasOccurred(CID_CompleteLevel))
	{
		this->pMapWidget->UpdateFromCurrentGame();
		this->pMapWidget->Paint();
		g_pTheSound->PlaySoundEffect(SEID_COMPLETE);
	} else if (CueEvents.HasOccurred(CID_ConquerRoom))
	{
		this->pMapWidget->UpdateFromCurrentGame();
		this->pMapWidget->Paint();
   }
	if (CueEvents.HasOccurred(CID_TrapDoorRemoved))
	{
		g_pTheSound->PlaySoundEffect(SEID_TRAPDOOR);
		CCoord *pCoord = DYN_CAST(CCoord *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_TrapDoorRemoved) );
		while (pCoord)
		{
			this->pRoomWidget->AddTLayerEffect(
					new CTrapdoorFallEffect(this->pRoomWidget, *pCoord));
			pCoord = DYN_CAST(CCoord *, CAttachableObject *,
				   CueEvents.GetNextPrivateData());
		}
	}
	if (CueEvents.HasOccurred(CID_CheckpointActivated) &&
			this->pRoomWidget->AreCheckpointsVisible())
	{
		CCoord *pCoord = DYN_CAST(CCoord *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_CheckpointActivated) );
		this->pRoomWidget->AddTLayerEffect(
				new CCheckpointEffect(this->pRoomWidget, *pCoord));
		g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);
	}
	if (CueEvents.HasOccurred(CID_RedDoorsOpened) || CueEvents.HasOccurred(CID_GreenDoorsOpened))
		g_pTheSound->PlaySoundEffect(SEID_DOOROPEN);
	if (CueEvents.HasOccurred(CID_CrumblyWallDestroyed))
	{
		g_pTheSound->PlaySoundEffect(SEID_BREAKWALL);
		CMoveCoord *pCoord = DYN_CAST(CMoveCoord *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_CrumblyWallDestroyed) );
		while (pCoord)
		{
			this->pRoomWidget->AddTLayerEffect(
					new CDebrisEffect(this->pRoomWidget, *pCoord));
			pCoord = DYN_CAST(CMoveCoord *, CAttachableObject *,
				   CueEvents.GetNextPrivateData());
		}
	}

	//
	//Begin section where room load can occur.  If room load occurs then
	//original cue events from command before room load will be discarded, and cue
	//events from first step into room will be in CueEvents.  Original cue events
	//should be handled before this section or stored in variables for handling
	//after this section.
	//

	bool died = false;
	//Check for player dieing.
	if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied))
	{
		died = true;
		//Update tile image arrays before death sequence.
		if (CueEvents.HasOccurred(CID_Plots))	//Plots occurred.
			this->pRoomWidget->UpdateFromPlots();

		HandleEventsForPlayerDeath();

		//Repaint the sign in case demo recording ended.
		ASSERT(!this->pCurrentGame->IsDemoRecording());
		SetSignTextToCurrentRoom();
		PaintSign();
		HideScroll();
      this->pFaceWidget->SetReading(false);

		CueEvents.Clear();
		this->pCurrentGame->RestartRoomFromLastCheckpoint(CueEvents);
		this->pRoomWidget->ClearEffects();
		this->pRoomWidget->ResetForPaint();
	}
	else
	{
		//Check for level exiting.
		const CAttachableWrapper<DWORD> *pNextLevelID =
				DYN_CAST(CAttachableWrapper<DWORD> *, CAttachableObject *,
				CueEvents.GetFirstPrivateData(CID_ExitLevelPending));
		const DWORD dwNextLevelID = pNextLevelID ? static_cast<DWORD>(*pNextLevelID) : 0;
		if (dwNextLevelID)
		{
			CDbBase::Commit();

			//Show swordsman walking down stairs, etc.
			eNextScreen = HandleEventsForLevelExit();

			CueEvents.Clear();

			if (GetScreenType() == SCR_Game && !this->bPlayTesting)
			{
				//When warping to another hold, save continue slot in this room
				//right before stepping on stairs.
				CDbLevel *pLevel = g_pTheDB->Levels.GetByID(dwNextLevelID);
				ASSERT(pLevel);
				if (pLevel->dwHoldID != this->pCurrentGame->pLevel->dwHoldID)
				{
					CCueEvents Ignored;
					this->pCurrentGame->UndoCommand(Ignored);
					this->pCurrentGame->SaveToContinue();
					g_pTheDB->SetHoldID(pLevel->dwHoldID);
				}
				delete pLevel;
			}

         if (GetScreenType() != SCR_Demo)
         {
			   this->pCurrentGame->LoadFromLevel(dwNextLevelID, CueEvents);
            //stop drawing new room until after Level Start screen
            this->pRoomWidget->ResetRoom();
         }

			return eNextScreen;
		}
	}

	//
	//End section where room load can occur.
	//

   if (died) return eNextScreen;

	if (bPlayerLeftRoom) //Went to a new room or this room was reloaded.
	{
		//Play music for style.
		SetMusicStyle(this->pCurrentGame->pRoom->wStyle);

		//Determine direction of exit (if any).
		UINT wExitOrientation = NO_ORIENTATION;
		const CAttachableWrapper<UINT> *pExitOrientation =
				DYN_CAST(CAttachableWrapper<UINT> *, CAttachableObject *,
				CueEvents.GetFirstPrivateData(CID_ExitRoom));
		if (pExitOrientation)
			wExitOrientation = static_cast<UINT>(*pExitOrientation);
		else
		{
			pExitOrientation =
					DYN_CAST(CAttachableWrapper<UINT> *, CAttachableObject *,
					CueEvents.GetFirstPrivateData(CID_ExitRoomPending));
			if (pExitOrientation)
				wExitOrientation = static_cast<UINT>(*pExitOrientation);
		}

		//Show the new room.
		this->pRoomWidget->ShowRoomTransition(wExitOrientation);

		//Update other UI elements that may have changed.
		this->pMapWidget->UpdateFromCurrentGame();
		this->pMapWidget->Paint();
		SetSignTextToCurrentRoom();
		PaintSign();
	}
	else //Still in the same room.
	{
		//Do an update of tile image arrays.
		if (CueEvents.HasOccurred(CID_Plots))	//Plots occurred.
			this->pRoomWidget->UpdateFromPlots();
	}

	return eNextScreen;
}

//***************************************************************************************
SCREENTYPE CGameScreen::ProcessCueEventsAfterRoomDraw(
//Process cue events list to create effects that should occur after
//room is drawn.
//
//Params:
	CCueEvents &CueEvents) //(in)
{
	SCREENTYPE eNextScreen = SCR_Game;

	if (CueEvents.HasOccurred(CID_CompleteLevel))
	{
		this->pRoomWidget->AddLastLayerEffect(
			new CFlashMessageEffect(this->pRoomWidget, g_pTheDB->GetMessageText(MID_LevelComplete)));
	} else if (CueEvents.HasOccurred(CID_StepOnScroll))
	{
		this->pFaceWidget->SetReading(true);
		CDbMessageText *pScrollText = DYN_CAST(CDbMessageText *, CAttachableObject *,
				CueEvents.GetFirstPrivateData(CID_StepOnScroll));
		ASSERT((const WCHAR *)(*pScrollText));
		this->pScrollLabel->SetText((const WCHAR *)(*pScrollText));
		ShowScroll();
	} else if (this->bIsScrollVisible &&
			this->pCurrentGame->pRoom->GetTSquare(this->pCurrentGame->swordsman.wX,
			this->pCurrentGame->swordsman.wY) != T_SCROLL)
	{
		this->pFaceWidget->SetReading(false);
		HideScroll();
	}
	//priority of player moods
	else if (CueEvents.HasOccurred(CID_SwordsmanAfraid))
		this->pFaceWidget->SetMood(Nervous);
	else if (CueEvents.HasOccurred(CID_SwordsmanAggressive))
		this->pFaceWidget->SetMood(Aggressive);
	else if (CueEvents.HasOccurred(CID_SwordsmanNormal))
		this->pFaceWidget->SetMood(Normal);

	if (CueEvents.HasOccurred(CID_AllMonstersKilled)) 	//priority of temporary moods
		this->pFaceWidget->SetMoodToSoundEffect(Happy, SEID_CLEAR);
	else if (CueEvents.HasOccurred(CID_MonsterDiedFromStab))
		this->pFaceWidget->SetMood(Strike,250);
	else if (CueEvents.HasOccurred(CID_Scared))
		this->pFaceWidget->SetMood(Nervous,250);
	else if (CueEvents.HasOccurred(CID_HitObstacle))
		this->pFaceWidget->SetMood(Aggressive,250);

	CMonsterMessage *pMsg = DYN_CAST(CMonsterMessage *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_MonsterSpoke));
	while (pMsg && GetScreenType() != SCR_Demo)
	{
		switch (pMsg->eType)
		{
			case MMT_OK:
				if (ShowOkMessage(pMsg->eMessageID) == TAG_QUIT)
					eNextScreen = SCR_None;
			break;

			case MMT_YESNO:
				const DWORD dwRet = ShowYesNoMessage(pMsg->eMessageID);
				if (dwRet == TAG_QUIT)
					eNextScreen = SCR_None;
				if (dwRet == TAG_YES)
					//Recursive call.
					eNextScreen = ProcessCommand(CMD_YES);
				else
					//Recursive call.
					eNextScreen = ProcessCommand(CMD_NO);

				//Refresh room in case command changed its state.
				this->pRoomWidget->DirtyRoom();
			break;
		}
		pMsg = DYN_CAST(CMonsterMessage *, CAttachableObject *,
			   CueEvents.GetNextPrivateData());
	}

	//If a highlight demo was saved, then update player settings with it.
	CHighlightDemoInfo *pHDI = DYN_CAST(CHighlightDemoInfo *, CAttachableObject *,
			CueEvents.GetFirstPrivateData(CID_HighlightDemoSaved));
	if (pHDI) AddHighlightDemoInfoToPlayerSettings(pHDI);

	//Check for winning game.
	if (CueEvents.HasOccurred(CID_WinGame) && this->GetScreenType() != SCR_Demo)
	{
		CDbBase::Commit();
		HandleEventsForLevelExit();	//walk down stairs first
		CueEvents.Clear();
      this->pRoomWidget->ResetRoom();
		if (this->bPlayTesting)
		{
			//Return to level editor.
			eNextScreen = SCR_Return;
		} else {
			//Won the game -- set continue slot to beginning of hold.
			LoadNewGame(g_pTheDB->GetHoldID());
			eNextScreen = SCR_WinStart;
		}
      UnloadGame();  //current game has ended (save continue slot, if applicable)
	}

	return eNextScreen;
}

//*****************************************************************************
void CGameScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool bUpdateRect)				//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	PaintBackground();
	PaintScroll();
	PaintSign();
	this->pRoomWidget->ResetForPaint();
	this->pFaceWidget->ResetForPaint();
	PaintChildren();

	if (bUpdateRect) UpdateRect();
}

// $Log: GameScreen.cpp,v $
// Revision 1.204  2005/03/15 22:15:14  mrimer
// Alt-F4 now exits instantly.
//
// Revision 1.203  2004/08/08 21:29:52  mrimer
// Fixed bug: music style changing during demo.
//
// Revision 1.202  2004/07/18 14:06:39  gjj
// Fixed some annoying warnings, and updated the config for fmod 3.7.3
//
// Revision 1.201  2004/06/08 14:19:20  mrimer
// Fixed a comment.
//
// Revision 1.200  2003/11/09 05:21:23  mrimer
// Now commit to DB on transition to demos screen.
//
// Revision 1.199  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.198  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.197  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.196  2003/09/03 21:39:50  erikh2000
// Changed FlushWrites() calls to Commit(), since redundant FlushWrites() was removed.
//
// Revision 1.195  2003/08/19 18:36:44  mrimer
// Added fix for Ctrl-Enter too.
//
// Revision 1.194  2003/08/19 13:30:43  mrimer
// Fixed bug: Alt-Enter pops up stat dialog box.
//
// Revision 1.193  2003/08/18 21:03:09  erikh2000
// Removed cheats.
//
// Revision 1.192  2003/07/29 13:32:44  mrimer
// Fixed bug: scroll not showing when continuing, standing on one.  Fix for importing highlight demo IDs.
//
// Revision 1.191  2003/07/27 05:15:48  mrimer
// Fixed bug: Beethro's eyes keep reading when no longer on scroll (e.g. dying, restoring).
//
// Revision 1.190  2003/07/24 19:48:12  mrimer
// Added optional screen type parameter to constructor.
//
// Revision 1.189  2003/07/22 18:59:35  mrimer
// Changed var to const.
//
// Revision 1.188  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.187  2003/07/15 01:21:24  mrimer
// Turned off cheats.
//
// Revision 1.186  2003/07/15 00:33:37  mrimer
// Refactored code to new DisplayRoomStats().  Now <Enter> also pops up room stats.
// Fixed improper fix of room widget display bug on hold end.
//
// Revision 1.185  2003/07/11 05:46:01  mrimer
// Fixed bug: crashes when winning hold (same dangling room pointer bug as for level end).
//
// Revision 1.184  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.183  2003/07/03 21:55:14  mrimer
// Made demo default description more descriptive.
//
// Revision 1.182  2003/07/02 01:08:19  mrimer
// Fixed bug: scroll left showing is still visible in next game session.
//
// Revision 1.181  2003/06/30 19:37:23  mrimer
// Fixed bug: next level displaying before intro screen on level exit.
// Added handling window focus changes in special event loops.
//
// Revision 1.180  2003/06/28 19:26:45  mrimer
// Now commits DB immediately on unloading a game (e.g. when winning).
//
// Revision 1.179  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.178  2003/06/27 19:16:11  mrimer
// Enabled checkpoints during play-testing (using temporary player that gets deleted after playtesting).
//
// Revision 1.177  2003/06/27 15:33:34  mrimer
// Fixed bug: crash on level exit (when room widget is repainted).
//
// Revision 1.176  2003/06/26 17:53:29  mrimer
// Continue game now gets saved on exiting out to title screen.
//
// Revision 1.175  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.174  2003/06/23 21:32:20  schik
// Fixed AllMonstersKilled effects getting skipped when Beethro was also tired.
//
// Revision 1.173  2003/06/22 05:58:00  mrimer
// Fixed a bug.
//
// Revision 1.172  2003/06/21 06:51:23  mrimer
// Removed unneeded map updates.
//
// Revision 1.171  2003/06/20 23:57:22  mrimer
// Fixed bug: conquered room not returning to red on map on room restart.
//
// Revision 1.170  2003/06/19 02:39:50  schik
// Turned cheat mode back off.
//
// Revision 1.169  2003/06/18 21:21:36  schik
// Added visual cue when room is in conquered state.
//
// Revision 1.168  2003/06/18 04:00:32  mrimer
// Fixed bug: checkpoints (and other saves/demos) active during playtesting from editor.
//
// Revision 1.167  2003/06/18 03:37:32  mrimer
// Improved death animation look.  Changed from frame-based to time-based.  Fixed some bugs.
//
// Revision 1.166  2003/06/17 23:13:58  mrimer
// Code maintenance -- ShowTextInputMessage() now requires text entry by default.
//
// Revision 1.165  2003/06/12 21:47:27  mrimer
// Set default export file name to something more intuitive.
//
// Revision 1.164  2003/06/06 18:11:30  mrimer
// Revised frame rate delay.
//
// Revision 1.163  2003/06/03 06:25:29  mrimer
// Fixed bugs in demo display.
//
// Revision 1.162  2003/06/01 23:14:05  erikh2000
// Disabled cheats.
//
// Revision 1.161  2003/05/25 22:48:11  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.160  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.159  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.158  2003/05/20 20:14:37  schik
// Fixed a hard crash when the player died.
//
// Revision 1.157  2003/05/19 20:29:27  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.156  2003/05/08 23:26:14  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.155  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.154  2003/04/24 22:50:57  mrimer
// Removed a printf.  Revised a comment.
//
// Revision 1.153  2003/04/08 13:08:27  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.152  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.151  2003/02/24 20:41:00  erikh2000
// Revised logic so that GetSavedGame() does not automatically attempt to load a new game when a saved game is missing.  In some cases, this isn't appropriate, so I just fail and let the caller load a new game if he wants to.
//
// Revision 1.150  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.149  2003/02/16 20:32:18  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.148  2003/01/08 00:57:19  mrimer
// Added changing hold ID when stairs warp to a different hold.
//
// Revision 1.147  2003/01/04 23:09:57  mrimer
// Updated swordsman interface.
//
// Revision 1.146  2002/12/22 02:39:55  mrimer
// Added in-game status pop-up display.
// Changed faulty reinterpret_cast's to dynamic_cast's.
// Added support for levels exiting to levels in other holds.
// Added transition effect when changing rooms.
// Revised swordsman vars.
//
// Revision 1.145  2002/11/22 02:34:47  mrimer
// Added TestRoom(), and other playtesting support for the level editor.
//
// Revision 1.144  2002/11/18 17:46:46  mrimer
// Removed frame rate delay to maximize animation rate.
//
// Revision 1.143  2002/11/15 03:12:52  mrimer
// Refactored several methods into new base class CRoomScreen.
// Revised LoadContinueGame() to support multiple players.
// Added new cue event handling.  Removed plot history.
// Revised fade on death to go completely black at end.
// Made some vars and parameters const.
//
// Revision 1.142  2002/10/23 21:48:45  erikh2000
// Fixed problem with yes/no dialog coming up during demo playback.
//
// Revision 1.141  2002/10/23 20:58:56  erikh2000
// Checkpoint activation effect is suppressed when checkpoints are hidden.
//
// Revision 1.140  2002/10/22 22:36:38  erikh2000
// In InitHighlightDemos() fixed an innacurate comment and removed the level 10 exit room highlight demo.
//
// Revision 1.139  2002/10/21 20:24:33  mrimer
// Repaint room when a message command is processed.
//
// Revision 1.138  2002/10/17 17:21:52  mrimer
// Fixed bug: new level doesn't show when Enter (et al) is hit on room exit.
// Added saving to DB on level exit.
//
// Revision 1.137  2002/10/16 01:29:11  erikh2000
// Fixed bugs where scroll doesn't disappear after swordsman leaves it.
//
// Revision 1.136  2002/10/11 17:36:59  mrimer
// Fixed bug: scroll doesn't disappear when exiting level.
//
// Revision 1.135  2002/10/11 15:30:43  mrimer
// Fixed display bugs.
//
// Revision 1.134  2002/10/11 01:56:42  erikh2000
// Disabled cheats.
//
// Revision 1.133  2002/10/10 21:15:03  mrimer
// Revised monster animation during player death.
// Modified to support optimized room drawing.
// Fixed bug where scroll doesn't disappear in certain cases.
//
// Revision 1.132  2002/10/10 00:59:47  erikh2000
// Removed code that shows and hides cursor--just let CScreen take care of that.
//
// Revision 1.131  2002/10/03 21:10:31  mrimer
// Now update changed room tiles in room widget before death.
//
// Revision 1.130  2002/10/03 19:16:25  mrimer
// Revised death effect to look better.
//
// Revision 1.129  2002/10/02 21:42:11  mrimer
// Enhanced death sequence so room fades to black behind player.
// Fixed bug: swordsman is invisible after winning and restoring to last room.
//
// Revision 1.128  2002/10/01 22:52:49  erikh2000
// Finished list of room IDs to add to highlight list.
//
// Revision 1.127  2002/09/30 18:42:36  mrimer
// Enhanced mimic placement effect.
// Made some const members static.
//
// Revision 1.126  2002/09/14 21:41:31  mrimer
// Added code to remove duplicate CBumpObstacleEffect's.
//
// Revision 1.125  2002/09/06 20:07:29  erikh2000
// Added more rooms to highlight demo list.
//
// Revision 1.124  2002/09/05 20:25:45  erikh2000
// Rerenamed "CtagHighlightDemoInfo" to "CHighlightDemoInfo".
//
// Revision 1.123  2002/09/05 18:51:40  mrimer
// Changed references to CHighlightDemoInfo to CtagHighlightDemoInfo.
//
// Revision 1.122  2002/09/05 18:22:10  erikh2000
// Added another highlight demo.
//
// Revision 1.121  2002/09/04 22:30:30  erikh2000
// Renamed "CtagHighlightDemoInfo" to "CHighlightDemoInfo".
//
// Revision 1.120  2002/08/30 23:59:42  erikh2000
// Added frame rate display.
//
// Revision 1.119  2002/08/29 22:03:11  mrimer
// Fixed bug: swordsman doesn't descend stairs on last level.
//
// Revision 1.118  2002/08/28 21:38:27  erikh2000
// New sounds play for crumbly walls breaking, tar stabs, doors opening, and last brain destroyed events.
//
// Revision 1.117  2002/08/28 20:32:52  mrimer
// Revised cue event handling.  Added handling for 'Neather and "tired" sounds.
//
// Revision 1.116  2002/08/25 19:00:01  erikh2000
// Removed code that made Beethro's face talk when a monster speaks.
//
// Revision 1.115  2002/08/24 21:46:04  erikh2000
// Beethro's happy mood after killing all the monsters will last for duration of sound effect.
//
// Revision 1.114  2002/08/23 23:38:57  erikh2000
// Added code to play level clear song instead of style song after player clears level.
// Several constants renamed.
// Text scroll is a little bigger now--coords updated for that.
// Made Beethro's face go from happy to normal after he walks down stairs.
//
// Revision 1.113  2002/07/29 16:36:20  mrimer
// Added resetting game state upon winning.
//
// Revision 1.112  2002/07/26 18:29:45  mrimer
// Fixed room redraw for "undo" move.
//
// Revision 1.111  2002/07/25 18:57:57  mrimer
// Added CID_CheckpointActivated handling.
//
// Revision 1.110  2002/07/25 17:29:34  mrimer
// Fixed setting scroll text.
//
// Revision 1.109  2002/07/23 20:16:23  mrimer
// Fix for showing scroll text.
//
// Revision 1.108  2002/07/22 02:48:52  erikh2000
// Made face and room widgets animated.
//
// Revision 1.107  2002/07/19 20:39:41  mrimer
// Changed HIGHLIGHT_DEMO_INFO to CtagHighlightDemoInfo.
//
// Revision 1.106  2002/07/19 01:31:03  erikh2000
// Beethro no longer says "ha!" when 'Neather exits room.
//
// Revision 1.105  2002/07/17 22:36:46  mrimer
// Added SetMusicStyle() call into SetForActivate().
//
// Revision 1.104  2002/07/17 22:11:07  erikh2000
// Put calls to ClearEffects() in room restart code.
//
// Revision 1.103  2002/07/17 21:22:48  mrimer
// Revised restarting room to return to checkpoint first.
// Remove T-layer effects when SwirlEffect() is called.
//
// Revision 1.102  2002/07/17 18:07:15  mrimer
// Fixed some key handling.
// Re-inserted undo move cheat key.
//
// Revision 1.101  2002/07/11 21:01:36  mrimer
// Added transition to win game screen.
//
// Revision 1.100  2002/07/10 04:12:48  erikh2000
// Rooms that highlight demos will be saved in are specified.
// When a highlight demo is saved, an indice to it will be saved in player settings.
//
// Revision 1.99  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.98  2002/07/03 22:13:15  mrimer
// Added SwirlEffect().
//
// Revision 1.97  2002/06/25 05:45:47  mrimer
// Removed GS_OnKeyup.  Refined key repeat code.  Added call to StopRepeat().
//
// Revision 1.96  2002/06/23 22:22:42  erikh2000
// Removed testing code.
//
// Revision 1.95  2002/06/23 10:51:48  erikh2000
// Wrote method that advises on whether or not to load the level start screen before loading the game screen.
// Change level exit handler to transition to level start screen.
//
// Revision 1.94  2002/06/22 06:19:34  erikh2000
// Fixed bug with BeginDemoRecording() receiving a NULL pointer.
//
// Revision 1.93  2002/06/22 05:58:15  erikh2000
// Fixed problems with sign painting before the fade-in transition.
//
// Revision 1.92  2002/06/21 22:28:27  erikh2000
// Twiddling.
//
// Revision 1.91  2002/06/21 05:17:30  mrimer
// Revised includes.
//
// Revision 1.90  2002/06/20 04:12:51  erikh2000
// Made changes to calls to methods that now take wstring param.
//
// Revision 1.89  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.88  2002/06/16 22:15:34  erikh2000
// Wrote code to load in auto-save and show checkpoint settings.
// Moved settings loading code into one ApplyPlayerSettings() method.
//
// Revision 1.87  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.86  2002/06/15 18:37:05  erikh2000
// Fixed places where a CDbBase-derived class was not being deleted.
//
// Revision 1.85  2002/06/14 00:51:32  erikh2000
// Changed a WCHAR buffer param to safer wstring.
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
//
// Revision 1.84  2002/06/13 21:50:44  mrimer
// Refined demo recording.
//
// Revision 1.83  2002/06/11 22:55:52  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.82  2002/06/09 06:41:25  erikh2000
// Removed two methods used for setting and getting keysym-to-command map from outside the screen.  These are loaded each time from DB-stored player settings when screen is activated.
//
// Revision 1.81  2002/06/05 03:15:06  mrimer
// Removed bad reinterpret_casts.
//
// Revision 1.80  2002/05/23 22:28:22  mrimer
// Load and save keyboard commands from DROD.ini.
//
// Revision 1.79  2002/05/21 18:59:28  erikh2000
// Added "press any key" on-screen text to exit level handler.
//
// Revision 1.78  2002/05/21 18:11:23  mrimer
// Added changing command key mappings in settings screen.
//
// Revision 1.77  2002/05/17 01:12:59  erikh2000
// Fixed problem with ASSERT firing in code that handles player's death.
//
// Revision 1.76  2002/05/17 00:34:26  erikh2000
// F1 now brings up quick start help page.
//
// Revision 1.75  2002/05/16 23:09:50  mrimer
// Added FlashMessageEffect and level cleared message.
//
// Revision 1.74  2002/05/16 19:16:33  mrimer
// Updated death sequence: improved monster animation.
//
// Revision 1.73  2002/05/16 18:22:04  mrimer
// Added player death animation.
//
// Revision 1.72  2002/05/15 23:43:27  mrimer
// Completed exit level sequence.
//
// Revision 1.71  2002/05/15 17:23:21  mrimer
// Added playing win music when level is exited.
//
// Revision 1.70  2002/05/15 01:26:00  erikh2000
// Added handling for F5 (begin/end demo recording) and F6 (go to demo screen).
//
// Revision 1.69  2002/05/14 22:05:03  mrimer
// Wrote part of animating the swordsman walking down the stairs.
//
// Revision 1.68  2002/05/14 20:33:52  mrimer
// Reenabled and tested browser help.
//
// Revision 1.66  2002/05/14 17:22:51  mrimer
// Now basing particle explosion direction on sword movement (not orientation).
// Changed tar babies' stab appearance to TarStabEffect.
//
// Revision 1.65  2002/05/12 03:15:55  erikh2000
// Changed OnKeyDown to call CScreen::OnKeydown().
// Rearranged cheat key code so that one block of code would handle most of the room exit work.
//
// Revision 1.64  2002/05/10 22:35:32  erikh2000
// Revised cursor show/hide code to use methods in CScreen.
//
// Revision 1.63  2002/04/29 00:12:47  erikh2000
// Added code to load current game from a saved game.
//
// Revision 1.62  2002/04/25 22:37:08  erikh2000
// Added code to fully paint face widget when entire screen is refreshed.
//
// Revision 1.61  2002/04/25 19:03:24  mrimer
// Added guarantee that mouse cursor is visible when quit dialog box activated.
//
// Revision 1.60  2002/04/25 18:09:31  mrimer
// Added quit confirmation, screen resizing, and proper mouse cursor display.
//
// Revision 1.59  2002/04/25 09:31:41  erikh2000
// Fixed some room widget refresh problems.
// Added music.
//
// Revision 1.58  2002/04/24 08:13:37  erikh2000
// Removed some testing code.
//
// Revision 1.57  2002/04/22 21:48:24  mrimer
// Added handling of particle effects flagged by cue events.
//
// Revision 1.56  2002/04/22 02:53:28  erikh2000
// Minor change to how strike-orb effect is created.
//
// Revision 1.55  2002/04/20 08:23:29  erikh2000
// Moved positions of elements around.
// Added sign at top of screen.
// Added scroll.
// Fixed some face widget updating problems.
//
// Revision 1.54  2002/04/19 21:56:11  erikh2000
// GameScreen now loads and displays a monolithic 640x480 background including a hidden parts area.
// Removed references to DRODGfx.dib.
// Moved some constants from ScreenConstants.h into constructor, and removed references to ScreenConstants.h.
// Removed FillScreenWithTexture() method.
//
// Revision 1.53  2002/04/18 17:43:26  mrimer
// Added cue events to change facial expression.
// Refined ProcessCueEvents...() path logic.
//
// Revision 1.52  2002/04/16 10:43:42  erikh2000
// If continued saved games can't be loaded, a new game will start.
// Monster messages are hooked up to new message dialogs.
// Cue-event processing routines can now cause screen change by returning a screen type.
//
// Revision 1.51  2002/04/14 00:31:22  erikh2000
// Moved some testing code around.
//
// Revision 1.50  2002/04/13 19:40:56  erikh2000
// Changed logic of some routines to allow for a routine called by Activate() to handle the event loop instead of Activate().
//
// Revision 1.49  2002/04/12 22:53:13  erikh2000
// Changed bitmap loading to use new reference-counting methods of CBitmapManager.
//
// Revision 1.48  2002/04/12 05:21:37  erikh2000
// Added InitKeysymToCommandMap().
// Wrote code to add room widget effects from cue events.
//
// Revision 1.47  2002/04/11 10:19:09  erikh2000
// Changed bitmap loading to use CBitmapManager.
//
// Revision 1.46  2002/04/09 21:53:55  erikh2000
// Added key repeat functionality.  (Committed on behalf of mrimer.)
//
// Revision 1.45  2002/04/09 10:25:33  erikh2000
// Added CGameScreen::Animate().
//
// Revision 1.44  2002/04/09 10:02:18  erikh2000
// Added face widget to screen.
//
// Revision 1.43  2002/04/09 01:16:34  erikh2000
// Functions were put into a new CGameScreen class which uses SDL.
//
// Revision 1.42  2002/03/17 23:07:01  erikh2000
// Changed call from RestartRoom() to RestartRoomFromLastCheckpoint() when player dies.
//
// Revision 1.41  2002/03/16 11:42:32  erikh2000
// Added code to play waves in response to cue events.  (Committed on behalf of mrimer.)
// Reenabled the level number display with a temporary hack.  This is useful until proper level descriptions can be displayed.  (Committed on behalf of mrimer.)
//
// Revision 1.40  2002/03/13 06:48:15  erikh2000
// Added transition to win game screen in response to CID_WINGAME.  (Committed on behalf of mrimer.)
//
// Revision 1.39  2002/03/13 06:30:16  erikh2000
// Added mrimer to contributors list comment.
//
// Revision 1.38  2002/03/13 00:37:53  erikh2000
// Fixed access violation on level exit.  (Help from mrimer.)
// Fixed exit sign not refreshing when new level starts or room restarts.  (Help from mrimer.)
// Fixed scroll display problems.  (Help from mrimer.)
//
// Revision 1.37  2002/03/05 01:52:59  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.36  2002/02/28 01:00:44  erikh2000
// Twiddling.
//
// Revision 1.35  2002/02/27 02:26:25  erikh2000
// Changed logic inside ProcessCueEventsBeforeRoomDraw() to handle cue events correctly.
//
// Revision 1.34  2002/02/25 03:41:40  erikh2000
// Removed debug output for showing room coords.
//
// Revision 1.33  2002/02/24 22:09:41  erikh2000
// Replaced wsprintf with swprintf.
//
// Revision 1.32  2002/02/24 01:26:16  erikh2000
// Fixed problems with sprintf format strings.
// Fixed calls to CCurrentGame methods to use new cue events param.
//
// Revision 1.31  2002/02/23 05:00:34  erikh2000
// Renamed CID_LevelExit reference to "CID_LevelExitPending".
//
// Revision 1.30  2002/02/13 01:04:05  erikh2000
// Added code to show room coords.
//
// Revision 1.29  2002/02/11 00:32:53  erikh2000
// Wrote code to change graphic style for different rooms/levels.
//
// Revision 1.28  2002/02/10 03:59:12  erikh2000
// Added CID_MonsterSpoke handling.
//
// Revision 1.27  2002/02/10 02:41:41  erikh2000
// Moved current game deletion into DROD.cpp for symmetry.
//
// Revision 1.26  2001/12/16 02:22:48  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.25  2001/11/25 02:32:45  erikh2000
// Changed CIDLists used for cue events to new CCueEvents class.
//
// Revision 1.24  2001/11/18 00:48:12  erikh2000
// Fixed transparent blitting problem in DrawMimicCursor().
//
// Revision 1.23  2001/11/17 23:06:46  erikh2000
// Added mimic cursor and mimic drawing.  (Committed on behalf of j_wicks.)
//
// Revision 1.22  2001/11/12 01:30:11  erikh2000
// Added a debug message showing which room has been entered.  (Committed on behalf of timeracer.)
// Fixed problem with tile image arrays not refreshing when room restart key is pressed.
//
// Revision 1.21  2001/11/06 08:45:24  erikh2000
// Added call to play trapdoor removal sound in response to CID_TrapDoorRemoved. (Committed on behalf of jpburford.)
//
// Revision 1.20  2001/11/05 05:45:52  erikh2000
// Added code to load next level when CID_ExitLevel returned.
//
// Revision 1.19  2001/11/03 20:34:33  erikh2000
// Fixed problem with tile images not refreshing when cheat keys are used.
//
// Revision 1.18  2001/11/03 20:10:32  erikh2000
// Fix problems with map not refreshing.  (Committed on behalf of timeracer.)
// Removed OnPlot() and OnLoad() references.  Added processing of CID_Plots cue event.
//
// Revision 1.17  2001/10/30 10:02:13  erikh2000
// Temporary fix made to prevent display and game data getting out of sync.
//
// Revision 1.16  2001/10/30 02:03:32  erikh2000
// Fix map drawing code. (Committed on behalf of timeracer.)
//
// Revision 1.15  2001/10/28 00:33:06  erikh2000
// Added display-buffering to eliminate flicker.
//
// Revision 1.14  2001/10/27 23:16:44  erikh2000
// Changed so some tile images are recalculated after processing instead of all of them.
//
// Revision 1.13  2001/10/27 20:47:48  erikh2000
// Added new TileImageCalcs.cpp/h and moved tile calculation routines from GameScreen into it.
//
// Revision 1.12  2001/10/27 20:24:08  erikh2000
// Added RefreshAffectedTileImages() stub and OnPlot() to call it.
//
// Revision 1.11  2001/10/27 04:43:58  erikh2000
// Created tile image arrays to store tile images for drawing room.
// Added OnLoad() and OnPlot() handling.
// Wrote tile image calculation functions for floor, pit, obstacle, wall, and crumbly wall.
// Added stubs for CalcTileImageForTar() and CalcTileImageForStairs().
//
// Revision 1.10  2001/10/21 00:36:05  erikh2000
// Screen now refreshes after pressing room restart key.
// Room now restarts after player dies.
// Hooked scroll display to cue events for stepping on and off scrolls.
//
// Revision 1.9  2001/10/20 10:17:09  erikh2000
// Added OnPlot() callback.
//
// Revision 1.8  2001/10/20 05:49:41  erikh2000
// Made several changes to handle new tile/tile image separation.
//
// Revision 1.7  2001/10/16 00:02:31  erikh2000
// Wrote code to display current room and handle basic swordsman movement.
//
// Revision 1.6  2001/10/13 01:24:14  erikh2000
// Removed unused fields and members from all projects, mostly UnderTile.
//
// Revision 1.5  2001/10/07 04:38:34  erikh2000
// General cleanup--finished removing game logic code from DROD.EXE.
// Added ProcessCommand() method to CCurrentGame to be used as interface between UI and game logic.
//
// Revision 1.4  2001/10/06 22:51:08  erikh2000
// Removed old restore game and save game code.
// Cleaned up title.cpp.
// Disabled some menu items.
// Changed names of some routines and removed failure returns.
//
// Revision 1.3  2001/10/06 03:38:37  erikh2000
// Fixed errors in app initialization--should at least get to the title screen now.
// Fixed GetSongFilepath() to use new dir structure.
// Removed WinHelp calls.
//
// Revision 1.2  2001/10/06 02:40:58  erikh2000
// Removed best display mode warning code.
// Removed Inval class and references to it.
//
// Revision 1.1.1.1  2001/10/01 22:18:01  erikh2000
// Initial check-in.
//
