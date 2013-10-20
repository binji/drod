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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "RestoreScreen.h"
#include "Browser.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodScreenManager.h"
#include "GameScreen.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/LabelWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/CueEvents.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

//Widget tags.
const DWORD TAG_ROOM_START = 1010;

const DWORD TAG_LEVEL_START = 1020;

const DWORD TAG_HOLD_START = 1021;
const DWORD TAG_LEVEL_LBOX = 1022;
const DWORD TAG_POSITION_LABEL = 1023;

const DWORD TAG_RESTORE = 1091;
const DWORD TAG_CANCEL = 1092;
const DWORD TAG_HELP = 1093;

//Reserve 2000 to 2999 for checkpoint tags.
const DWORD TAG_CHECKPOINT = 2000;
#define IS_CHECKPOINT_TAG(t) ((t) >= 2000 && (t) < 3000)

//
//Protected methods.
//

//*****************************************************************************************
CRestoreScreen::CRestoreScreen()
	: CScreen(SCR_Restore)
	, dwSelectedSavedGameID(0L), dwLastGameID(0L)
   , wConqueredRooms(0)
	, pCurrentRestoreGame(NULL)
	, pRoomWidget(NULL)
	, pScaledRoomWidget(NULL)
	, pMapWidget(NULL)
	, pLevelListBoxWidget(NULL)
	, pBackgroundSurface(NULL)
//Constructor.
{
	SetKeyRepeat(66);
}

//************************************************************************************
bool CRestoreScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);

	//Load background graphic.
	ASSERT(!this->pBackgroundSurface);
	this->pBackgroundSurface = g_pTheBM->GetBitmapSurface("Background");
	if (!this->pBackgroundSurface) return false;

	const UINT CX_SPACE = 8;
	const UINT CY_SPACE = 8;
	const UINT CX_TITLE = 158;
	const UINT CY_TITLE = 32;
	const UINT CY_TITLE_SPACE = 8;
	const int X_TITLE = (this->w - CX_TITLE) / 2;
	const int Y_TITLE = CY_TITLE_SPACE;

	const UINT CX_RESTORE_BUTTON = 70;
	const UINT CY_RESTORE_BUTTON = CY_STANDARD_BUTTON;
	const UINT CX_CANCEL_BUTTON = CX_RESTORE_BUTTON;
	const UINT CY_CANCEL_BUTTON = CY_RESTORE_BUTTON;
	const UINT CX_HELP_BUTTON = CX_RESTORE_BUTTON;
	const UINT CY_HELP_BUTTON = CY_RESTORE_BUTTON;
	const int X_CANCEL_BUTTON = (this->w - CX_CANCEL_BUTTON) / 2;
	const int X_RESTORE_BUTTON = X_CANCEL_BUTTON - CX_RESTORE_BUTTON - CX_SPACE;
	const int X_HELP_BUTTON = X_CANCEL_BUTTON + CX_CANCEL_BUTTON + CX_SPACE;
	const int Y_RESTORE_BUTTON = this->h - CY_SPACE - CY_RESTORE_BUTTON;
	const int Y_CANCEL_BUTTON = Y_RESTORE_BUTTON;
	const int Y_HELP_BUTTON = Y_RESTORE_BUTTON;

	//Mini-room widget has strict proportions and its dimensions will define 
	//placement of most everything else.
	const int Y_CHOOSE_POS_LABEL = Y_TITLE + CY_TITLE + CY_TITLE_SPACE;
	const UINT CY_CHOOSE_POS_LABEL = CY_STANDARD_BUTTON;
	const int Y_POSITION_LABEL = Y_CHOOSE_POS_LABEL + CY_CHOOSE_POS_LABEL;
	const UINT CY_POSITION_LABEL = 15;
	const int Y_MINIROOM = Y_POSITION_LABEL + CY_POSITION_LABEL;
	const UINT CY_MINIROOM = this->h - Y_MINIROOM - CY_SPACE - CY_STANDARD_BUTTON - CY_SPACE;
	//Width of mini-room must be proportional to regular room display.
	const UINT CX_MINIROOM = CY_MINIROOM * CDrodBitmapManager::CX_ROOM /
			CDrodBitmapManager::CY_ROOM;
	const int X_MINIROOM = this->w - CX_SPACE - CX_MINIROOM;
	const int X_CHOOSE_POS_LABEL = X_MINIROOM;
	const UINT CX_CHOOSE_POS_LABEL = 100;
	const UINT CX_ROOM_START = 70;
	const int X_ROOM_START = this->w - CX_SPACE - CX_ROOM_START;
	const int Y_ROOM_START = Y_CHOOSE_POS_LABEL;
	const UINT CY_ROOM_START = CY_CHOOSE_POS_LABEL;
	const int X_POSITION_LABEL = X_MINIROOM;
	const UINT CX_POSITION_LABEL= CX_MINIROOM;

	const UINT CX_MAP = this->w - CX_SPACE - CX_MINIROOM - CX_SPACE - CX_SPACE;
	const UINT CY_MAP = CY_MINIROOM * CX_MAP / CX_MINIROOM;
	const int X_MAP = CX_SPACE;
	const int Y_MAP = Y_RESTORE_BUTTON - CY_SPACE - CY_MAP;
	const int X_CHOOSE_ROOM_LABEL = X_MAP;
	const UINT CX_CHOOSE_ROOM_LABEL = 100;
	const int CY_CHOOSE_ROOM_LABEL = CY_STANDARD_BUTTON;
	const int Y_CHOOSE_ROOM_LABEL = Y_MAP - CY_CHOOSE_ROOM_LABEL - 1;
	const UINT CX_LEVEL_START = CX_ROOM_START;
	const int X_LEVEL_START = X_MAP + CX_MAP - CX_LEVEL_START;
	const int Y_LEVEL_START = Y_CHOOSE_ROOM_LABEL;
	const int CY_LEVEL_START = CY_CHOOSE_ROOM_LABEL;

	const int X_CHOOSE_LEVEL_LABEL = CX_SPACE;
	const int Y_CHOOSE_LEVEL_LABEL = Y_CHOOSE_POS_LABEL;
	const UINT CX_CHOOSE_LEVEL_LABEL = 100;
	const UINT CY_CHOOSE_LEVEL_LABEL = CY_STANDARD_BUTTON;
	const UINT CX_GAME_START = CX_ROOM_START;
	const UINT CY_GAME_START = CY_CHOOSE_LEVEL_LABEL;
	const int X_GAME_START = X_MAP + CX_MAP - CX_GAME_START;
	const int Y_GAME_START = Y_CHOOSE_LEVEL_LABEL;
	const int X_LEVEL_LBOX = X_CHOOSE_LEVEL_LABEL;
	const int Y_LEVEL_LBOX = Y_CHOOSE_LEVEL_LABEL + CY_CHOOSE_LEVEL_LABEL + 1;
	const UINT CX_LEVEL_LBOX = CX_MAP;
	const UINT CY_LEVEL_LBOX = Y_CHOOSE_ROOM_LABEL - Y_CHOOSE_LEVEL_LABEL -
			CY_CHOOSE_LEVEL_LABEL - CY_SPACE * 2;

	CButtonWidget *pButton;

	//Title.
	AddWidget(new CLabelWidget(0L, X_TITLE, Y_TITLE,
			CX_TITLE, CY_TITLE, F_Title, g_pTheDB->GetMessageText(MID_RestoreGame)));

	//Level selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_LEVEL_LABEL, Y_CHOOSE_LEVEL_LABEL, 
				CX_CHOOSE_LEVEL_LABEL, CY_CHOOSE_LEVEL_LABEL, F_Header, 
				g_pTheDB->GetMessageText(MID_ChooseLevel)));
	pButton = new CButtonWidget(TAG_HOLD_START, X_GAME_START, Y_GAME_START, 
				CX_GAME_START, CY_GAME_START, g_pTheDB->GetMessageText(MID_HoldStart));
	AddWidget(pButton);

	this->pLevelListBoxWidget = new CListBoxWidget(TAG_LEVEL_LBOX,
			X_LEVEL_LBOX, Y_LEVEL_LBOX, CX_LEVEL_LBOX, CY_LEVEL_LBOX);
	AddWidget(this->pLevelListBoxWidget);

	//Room selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_ROOM_LABEL, Y_CHOOSE_ROOM_LABEL, 
				CX_CHOOSE_ROOM_LABEL, CY_CHOOSE_ROOM_LABEL, F_Header, 
				g_pTheDB->GetMessageText(MID_ChooseRoom)));
	pButton = new CButtonWidget(TAG_LEVEL_START, X_LEVEL_START, Y_LEVEL_START, 
				CX_LEVEL_START, CY_LEVEL_START, g_pTheDB->GetMessageText(MID_LevelStart));
	AddWidget(pButton);

	this->pMapWidget = new CMapWidget(TAG_MAP, X_MAP, Y_MAP, CX_MAP, CY_MAP, NULL);
	AddWidget(this->pMapWidget);

	//Position selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_POS_LABEL, Y_CHOOSE_POS_LABEL, 
				CX_CHOOSE_POS_LABEL, CY_CHOOSE_POS_LABEL, F_Header, 
				g_pTheDB->GetMessageText(MID_ChoosePosition)));
	pButton = new CButtonWidget(TAG_ROOM_START, X_ROOM_START, Y_ROOM_START,
				CX_ROOM_START, CY_ROOM_START, g_pTheDB->GetMessageText(MID_RoomStart));
	AddWidget(pButton);

	AddWidget(new CLabelWidget(TAG_POSITION_LABEL, X_POSITION_LABEL, Y_POSITION_LABEL, 
				CX_POSITION_LABEL, CY_POSITION_LABEL, F_Small, wszEmpty));

	this->pScaledRoomWidget = new CScalerWidget(0L, X_MINIROOM, Y_MINIROOM, 
			CX_MINIROOM, CY_MINIROOM);
	AddWidget(this->pScaledRoomWidget);
	this->pRoomWidget = new CRoomWidget(0L, 0, 0, CDrodBitmapManager::CX_ROOM,
			CDrodBitmapManager::CY_ROOM, NULL);
	this->pScaledRoomWidget->AddScaledWidget(this->pRoomWidget);

	//Restore, cancel and help buttons.
	pButton = new CButtonWidget(TAG_RESTORE, X_RESTORE_BUTTON, Y_RESTORE_BUTTON, 
				CX_RESTORE_BUTTON, CY_RESTORE_BUTTON, g_pTheDB->GetMessageText(MID_Restore));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_CANCEL, X_CANCEL_BUTTON, Y_CANCEL_BUTTON,
				CX_CANCEL_BUTTON, CY_CANCEL_BUTTON, g_pTheDB->GetMessageText(MID_Cancel));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_HELP, X_HELP_BUTTON, Y_HELP_BUTTON, 
				CX_HELP_BUTTON, CY_HELP_BUTTON, g_pTheDB->GetMessageText(MID_Help));
	AddWidget(pButton);
	AddHotkey(SDLK_F1,TAG_HELP);

	//Load children widgets.
	this->bIsLoaded = LoadChildren();

	return this->bIsLoaded;
}

//************************************************************************************
void CRestoreScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	for(CHECKPOINT_LIST::const_iterator iSeek = this->Checkpoints.begin();
			iSeek != this->Checkpoints.end(); ++iSeek)
		delete static_cast<char*>(*iSeek);
	this->Checkpoints.clear();

	if (this->pBackgroundSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Background");
		this->pBackgroundSurface = NULL;
	}

	delete this->pCurrentRestoreGame;
	this->pCurrentRestoreGame = NULL;

	this->bIsLoaded = false;
}

//******************************************************************************
bool CRestoreScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Get widgets and current games ready.
	if (!SetWidgets()) return false;

	SelectFirstWidget(false);

	return true;
}

//
//Private methods.
//

//******************************************************************************
void CRestoreScreen::OnClick(
//Called when widget receives a click event.
//
//Params:
	const DWORD dwTagNo) //(in)	Widget that event applied to.
{
	switch (dwTagNo)
	{
		case TAG_ESCAPE:
		case TAG_CANCEL:
			GoToScreen(SCR_Return);
		break;

		case TAG_RESTORE:
			if (this->dwSelectedSavedGameID)
			{
				if (this->dwLastGameID)
				{
					//If the level being restored to is the same as the one
					//for the current game or continue slot, prompt the player
					//if a saved game with fewer conquered rooms is being selected.
					CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetByID(
							this->dwLastGameID);
					if (pSavedGame)
               {
					   CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(pSavedGame->dwRoomID);
					   delete pSavedGame;
					   ASSERT(pRoom);
					   if (pRoom->dwLevelID == pCurrentRestoreGame->pRoom->dwLevelID)
					   {
						   if (this->wConqueredRooms >
								   this->pCurrentRestoreGame->ConqueredRooms.GetSize())
						   {
							   if (ShowYesNoMessage(MID_UnconqueredRooms) == TAG_NO)
                        {
                           delete pRoom;
								   break;
                        }
						   }
					   }
					   delete pRoom;
               } //else: the "last" saved game could be for an empty continue slot,
               //in which case it shouldn't be compared against.
				}
				CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
						g_pTheSM->GetLoadedScreen(SCR_Game));
				pGameScreen->LoadSavedGame(this->dwSelectedSavedGameID);
				if (pGameScreen->ShouldShowLevelStart())
					GoToScreen(SCR_LevelStart);
				else
					GoToScreen(SCR_Game);
			}
		break;

		case TAG_HELP:
			SetFullScreen(false);
			ShowHelp("restore.html");
		break;

		case TAG_HOLD_START:
		{
			//For currently selected hold.
			this->pLevelListBoxWidget->SelectItem(
					this->pCurrentRestoreGame->pHold->dwLevelID);
			ChooseLevelStart(this->pCurrentRestoreGame->pHold->dwLevelID);
			Paint();
		}
		break;

		case TAG_LEVEL_START:
			ChooseLevelStart(this->pCurrentRestoreGame->pLevel->dwLevelID);
			Paint();
		break;

		case TAG_ROOM_START:
			ChooseRoomStart(this->pCurrentRestoreGame->pRoom->dwRoomX, 
					this->pCurrentRestoreGame->pRoom->dwRoomY);
			Paint();
		break;		

		default:
			if (IS_CHECKPOINT_TAG(dwTagNo))
			{
				ChooseCheckpoint(dwTagNo);
				SelectWidget(TAG_RESTORE);
				this->pScaledRoomWidget->Paint();
            this->pMapWidget->Paint();
			}
		break;
	}	//...switch (dwActionNo)
}

//*****************************************************************************
void CRestoreScreen::OnSelectChange(
//Handles a selection change.
//
//Params:
	const DWORD dwTagNo) //(in) Widget event applies to.
{
	switch (dwTagNo)
	{
		case TAG_LEVEL_LBOX:
			ChooseLevelLatest(this->pLevelListBoxWidget->GetSelectedItem());
			Paint();
		break;

		case TAG_MAP:
		{
			DWORD dwRoomX, dwRoomY;
			this->pMapWidget->Paint();
			this->pMapWidget->GetSelectedRoomXY(dwRoomX, dwRoomY);
			ChooseRoomLatest(dwRoomX, dwRoomY);
			Paint();
		}
		break;
	}
}

//*****************************************************************************
bool CRestoreScreen::SetWidgets()
//Set up widgets and data used by them when user first arrives at restore
//screen.  Should only be called by SetForActivate().
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = true;

	//Update level selection list box.
	PopulateLevelListBoxFromSavedGames();

	//Delete any existing current games for this screen.
	delete this->pCurrentRestoreGame;
	this->pCurrentRestoreGame = NULL;

	//Load current room and level from game screen if it has a game loaded.
	CCueEvents Ignored;
	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetLoadedScreen(SCR_Game));
   ASSERT(pGameScreen);
	const CCurrentGame *pCurrentGame = pGameScreen->GetCurrentGame(); //Delete not needed.
	if (pCurrentGame)
	{
		this->dwLastGameID = g_pTheDB->SavedGames.FindByLevelLatest(
				pCurrentGame->pLevel->dwLevelID);
		if (!this->dwLastGameID) {bSuccess=false; goto Cleanup;}
		this->pCurrentRestoreGame = g_pTheDB->GetSavedCurrentGame(
				this->dwLastGameID, Ignored);
		if (!this->pCurrentRestoreGame) {bSuccess=false; goto Cleanup;}
      this->wConqueredRooms = this->pCurrentRestoreGame->ConqueredRooms.GetSize();
		this->pCurrentRestoreGame->SetRoomStatusFromAllSavedGames();

		ChooseRoomLatest(pCurrentGame->pRoom->dwRoomX, pCurrentGame->pRoom->dwRoomY);
	}
	else //I couldn't get current game from game screen.
	{
		//Load game from the continue slot.
		this->dwLastGameID = g_pTheDB->SavedGames.FindByContinue();
		this->pCurrentRestoreGame = g_pTheDB->GetSavedCurrentGame(
				this->dwLastGameID, Ignored);
		if (!this->pCurrentRestoreGame)
		{
         //No continue slot yet, load from beginning of game.
         this->pCurrentRestoreGame = g_pTheDB->GetNewCurrentGame(g_pTheDB->GetHoldID(), Ignored);
			if (!this->pCurrentRestoreGame) {bSuccess=false; goto Cleanup;}
		}
		this->dwSelectedSavedGameID = this->pCurrentRestoreGame->dwSavedGameID;
      this->wConqueredRooms = this->pCurrentRestoreGame->ConqueredRooms.GetSize();

      UpdateWidgets();

		//Update room label.
		WSTRING wstrDesc;
		this->pCurrentRestoreGame->pRoom->GetLevelPositionDescription(wstrDesc);
		CLabelWidget *pRoomLabel = DYN_CAST(CLabelWidget*, CWidget*,
				GetWidget(TAG_POSITION_LABEL));
		pRoomLabel->SetText(&*wstrDesc.begin());
	}
	
	//Select level from the list box.
	this->pLevelListBoxWidget->SelectItem(this->pCurrentRestoreGame->pLevel->dwLevelID);

Cleanup:
	return bSuccess;
}

//*****************************************************************************
void CRestoreScreen::ShowCheckpointButtonsForSavedGame(
//Hides checkpoint button corresponding to a saved game and shows all the rest.
//
//Params:
	DWORD dwSavedGameID)	//(in)
{
	for(CHECKPOINT_LIST::const_iterator iSeek = this->Checkpoints.begin();
			iSeek != this->Checkpoints.end(); ++iSeek)
	{
		CHECKPOINT *pCheckpoint = static_cast<CHECKPOINT *>(*iSeek);
		if (pCheckpoint->dwSavedGameID == dwSavedGameID)
			pCheckpoint->pButton->Hide();
		else
			pCheckpoint->pButton->Show();
	}
}

//*****************************************************************************
void CRestoreScreen::SetCheckpoints()
//Adds and removes widgets so that a checkpoint button exists over each 
//checkpoint in the room widget with a corresponding saved game.
{
	//Remove all current checkpoint buttons.
	for(CHECKPOINT_LIST::const_iterator iSeek = this->Checkpoints.begin();
			iSeek != this->Checkpoints.end(); ++iSeek)
	{
		CHECKPOINT *pCheckpoint = static_cast<CHECKPOINT *>(*iSeek);
		this->pScaledRoomWidget->RemoveWidget(pCheckpoint->pButton);
		delete static_cast<char*>(*iSeek);
	}
	this->Checkpoints.clear();

	//Each iteration looks at one saved game in the room.
	DWORD dwTagNo = TAG_CHECKPOINT;
	CDbSavedGame *pSavedGame =
			this->pCurrentRestoreGame->pRoom->SavedGames.GetFirst();
	while (pSavedGame)
	{
		//Is it a checkpoint saved game?
		if (pSavedGame->eType == ST_Checkpoint) 
		{
			//Yes--add a new button for it.
			CButtonWidget *pCheckpointButton;
			SDL_Rect SquareRect;
			SDL_Rect ScaledRoomRect;
			this->pScaledRoomWidget->GetRect(ScaledRoomRect);

			this->pRoomWidget->GetSquareRect(pSavedGame->wCheckpointX, 
					pSavedGame->wCheckpointY, SquareRect);
			SquareRect.x = this->pScaledRoomWidget->GetScaledX(SquareRect.x) - 
					ScaledRoomRect.x - 5;
			SquareRect.y = this->pScaledRoomWidget->GetScaledY(SquareRect.y) -
					ScaledRoomRect.y - 5;
			SquareRect.w = this->pScaledRoomWidget->GetScaledW(SquareRect.w) + 10;
			SquareRect.h = this->pScaledRoomWidget->GetScaledH(SquareRect.h) + 10;

         static const WCHAR wszX[] = {W_t('x'),W_t(0)};
			pCheckpointButton = new CButtonWidget(dwTagNo, SquareRect.x, SquareRect.y,
					SquareRect.w, SquareRect.h, wszX);
			this->pScaledRoomWidget->AddWidget(pCheckpointButton, true);
			++dwTagNo;
			
			//Add button to checkpoint button list.
			if (pCheckpointButton)
			{
				CHECKPOINT *pNew = new CHECKPOINT;
				pNew->pButton = pCheckpointButton;
				pNew->dwSavedGameID = pSavedGame->dwSavedGameID;
				this->Checkpoints.push_back(pNew);
			}
			else
				ASSERTP(false, "No checkpoint button.");
		}
		delete pSavedGame;
		pSavedGame = this->pCurrentRestoreGame->pRoom->SavedGames.GetNext();
	}
}

//*****************************************************************************
void CRestoreScreen::ChooseCheckpoint(
//Chooses saved game from a checkpoint.
//
//Params:
	const DWORD dwTagNo)	//(in)	Tag# of pressed checkpoint button.
{
	//Find checkpoint info.
	CHECKPOINT *pCheckpoint = NULL;
   CHECKPOINT_LIST::const_iterator iSeek;
	for (iSeek = this->Checkpoints.begin(); iSeek != this->Checkpoints.end(); ++iSeek)
	{
		pCheckpoint = static_cast<CHECKPOINT *>(*iSeek);
		if (pCheckpoint->pButton->GetTagNo() == dwTagNo) break; //Found it.
	}
	if (iSeek == this->Checkpoints.end()) return; //No match.

	ChooseRoomSavedGame(pCheckpoint->dwSavedGameID);
}

//*****************************************************************************
void CRestoreScreen::ChooseRoomLatest(
//Chooses latest saved game for room in the current level and updates display.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of chosen room.
{
	//Find the room.
	const DWORD dwRoomID = g_pTheDB->Rooms.FindIDAtCoords(
			this->pCurrentRestoreGame->pLevel->dwLevelID, dwRoomX, dwRoomY);
	if (dwRoomID)
	{
		//Find the saved game.
		const DWORD dwSavedGameID = g_pTheDB->SavedGames.FindByRoomLatest(dwRoomID);
		if (!dwSavedGameID) return;
		
		ChooseRoomSavedGame(dwSavedGameID);
	}
}

//*****************************************************************************
void CRestoreScreen::ChooseRoomStart(
//Chooses room start saved game for room in the current level and updates display.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of chosen room.
{
	//Find the room.
	const DWORD dwRoomID = g_pTheDB->Rooms.FindIDAtCoords(
			this->pCurrentRestoreGame->pLevel->dwLevelID, dwRoomX, dwRoomY);
	if (dwRoomID)
	{
		//Find the saved game.
		const DWORD dwSavedGameID = g_pTheDB->SavedGames.FindByRoomBegin(dwRoomID);
		if (!dwSavedGameID) return;
		
		ChooseRoomSavedGame(dwSavedGameID);
	}
}

//*****************************************************************************
void CRestoreScreen::ChooseRoomSavedGame(
//Chooses a room in the current level and updates display.
//
//Params:
	const DWORD dwSavedGameID)	//(in) Saved game to use.
{
	ASSERT(dwSavedGameID);

	//Load the saved game.
	CCueEvents Ignored;
	CCurrentGame *pNewGame = g_pTheDB->GetSavedCurrentGame(dwSavedGameID, Ignored);
	if (pNewGame)
	{
		//Switch current game over to new one from saved game.
		if (this->pCurrentRestoreGame) delete this->pCurrentRestoreGame;
		this->pCurrentRestoreGame = pNewGame;
		this->dwSelectedSavedGameID = dwSavedGameID;

      UpdateWidgets();
		this->pMapWidget->SelectRoom(this->pCurrentRestoreGame->pRoom->dwRoomX,
				this->pCurrentRestoreGame->pRoom->dwRoomY);
						
		//Update room label.
		WSTRING wstrDesc;
		this->pCurrentRestoreGame->pRoom->GetLevelPositionDescription(wstrDesc);
		CLabelWidget *pRoomLabel = DYN_CAST(CLabelWidget*, CWidget*,
				GetWidget(TAG_POSITION_LABEL));
		pRoomLabel->SetText(wstrDesc.c_str());
		
		//Put buttons over the room corresponding to saved games.
		SetCheckpoints();
		ShowCheckpointButtonsForSavedGame(dwSavedGameID);
	}
}

//*****************************************************************************
void CRestoreScreen::ChooseLevelStart(
//Choose a level start saved game and updates display.
//
//Params:
	const DWORD dwLevelID)	//(in) Level to load saved game for.
{
	ASSERT(dwLevelID);

	//Find the level start saved game.
	const DWORD dwSavedGameID = g_pTheDB->SavedGames.FindByLevelBegin(dwLevelID);
	if (!dwSavedGameID) return;

	ChooseLevelSavedGame(dwSavedGameID);
}

//*****************************************************************************
void CRestoreScreen::ChooseLevelLatest(
//Choose latest saved game in a level and updates display.
//
//Params:
	const DWORD dwLevelID)	//(in) Level to load saved game for.
{
	ASSERT(dwLevelID);

	//Don't update when selecting the same level.
	if (dwLevelID == this->pCurrentRestoreGame->pLevel->dwLevelID) return;

	//Find the latest saved game on the level.
	const DWORD dwSavedGameID = g_pTheDB->SavedGames.FindByLevelLatest(dwLevelID);
	if (!dwSavedGameID) return;

	ChooseLevelSavedGame(dwSavedGameID);
}

//*****************************************************************************
void CRestoreScreen::ChooseLevelSavedGame(
//Chooses a level and updates display.
//
//Params:
	const DWORD dwSavedGameID)	//(in) Saved game to load.
{
	ASSERT(dwSavedGameID);
	
	//Load the saved game.
	CCueEvents Ignored;
	CCurrentGame *pNewGame = g_pTheDB->GetSavedCurrentGame(dwSavedGameID, Ignored);
	if (pNewGame)
	{
		//Switch current game over to new one from saved game.
		if (this->pCurrentRestoreGame) delete this->pCurrentRestoreGame;
		this->pCurrentRestoreGame = pNewGame;
		const DWORD dwRoomSavedGameID = dwSavedGameID;
		if (!dwRoomSavedGameID) {ASSERTP(false, "Bad saved game ID."); return;}
		this->dwSelectedSavedGameID = dwSavedGameID;

      UpdateWidgets();

		//Update room label.
		WSTRING wstrDesc;
		this->pCurrentRestoreGame->pRoom->GetLevelPositionDescription(wstrDesc);
		CLabelWidget *pRoomLabel = DYN_CAST(CLabelWidget*, CWidget*,
				GetWidget(TAG_POSITION_LABEL));
		pRoomLabel->SetText(wstrDesc.c_str());

		//Put buttons over the room corresponding to saved games.
		SetCheckpoints();
		ShowCheckpointButtonsForSavedGame(dwSavedGameID);
	}
}

//*****************************************************************************
void CRestoreScreen::UpdateWidgets()
//Update the map and room widgets to reflect the current game.
//Update the map widget to show rooms which are not explored in the saved game.
{
	//Update the map and room widgets with new current game.
	this->pRoomWidget->LoadFromCurrentGame(this->pCurrentRestoreGame);
	this->pMapWidget->LoadFromCurrentGame(this->pCurrentRestoreGame);

	//Set conquered/explored status of rooms from all saved games in the level.
   CIDList CurrentExploredRooms = this->pCurrentRestoreGame->ExploredRooms;
	this->pCurrentRestoreGame->SetRoomStatusFromAllSavedGames();

   CIDList DarkenedRooms = this->pCurrentRestoreGame->ExploredRooms;
	DarkenedRooms -= CurrentExploredRooms;
	this->pMapWidget->SetDarkenedRooms(DarkenedRooms);
}

//*****************************************************************************
void CRestoreScreen::Paint(
//Paint the screen.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	SDL_BlitSurface(this->pBackgroundSurface, NULL, GetDestSurface(), NULL);

	PaintChildren();
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CRestoreScreen::PopulateLevelListBoxFromSavedGames()
//Put levels into list box that have at least one saved game associated with
//them.
{
	BEGIN_DBREFCOUNT_CHECK;

	this->pLevelListBoxWidget->Clear();
	
	{
		//Get the hold.
		CDbHold *pHold = g_pTheDB->Holds.GetByID(g_pTheDB->GetHoldID());
		if (!pHold) {ASSERTP(false, "Failed to retrieve hold."); return;} //Probably corrupted DB.
			
		//Add first level even if it doesn't have a saved game.
		CDbLevel *pLevel = pHold->Levels.GetFirst();
		if (!pLevel) {ASSERTP(false, "Failed to retrieve level."); delete pHold; return;} //Probably corrupted DB.
		this->pLevelListBoxWidget->AddItem(pLevel->dwLevelID, pLevel->NameText);
		delete pLevel;
		pLevel = pHold->Levels.GetNext();
		
		//Add level IDs containing level start saved games in the
		//current hold.
		while (pLevel)
		{
			if (g_pTheDB->SavedGames.FindByLevelBegin(pLevel->dwLevelID))
				this->pLevelListBoxWidget->AddItem(pLevel->dwLevelID, pLevel->NameText);

			delete pLevel;
			pLevel = pHold->Levels.GetNext();
		}
		delete pHold;
	}

	END_DBREFCOUNT_CHECK;
}

// $Log: RestoreScreen.cpp,v $
// Revision 1.63  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.62  2003/08/25 22:06:58  mrimer
// Added UpdateWidgets(), and simplified saved game representation.
//
// Revision 1.61  2003/08/24 21:18:07  erikh2000
// Calls to a function were broken.  Looks like a partial check-in put code out of synch.
//
// Revision 1.60  2003/08/11 12:48:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.59  2003/07/22 19:00:25  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.58  2003/07/22 18:30:33  mrimer
// Fixed bug: room not synched w/ map and when showing continue saved game.
//
// Revision 1.57  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.56  2003/07/07 23:26:55  mrimer
// Fixed a couple bugs.
//
// Revision 1.55  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.54  2003/06/24 20:13:42  mrimer
// Fixed a bug (when continue slot is empty).
//
// Revision 1.53  2003/06/20 23:54:59  mrimer
// Fixed 2 bugs: map not updating correctly (on restore screen: checkpoints/green rooms).
//
// Revision 1.52  2003/06/16 18:58:35  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.51  2003/05/30 04:04:49  mrimer
// Fixed some bugs in the restore logic (again, for the last time).
//
// Revision 1.50  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.49  2003/05/23 21:43:04  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.48  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.47  2003/05/21 03:06:44  mrimer
// Suppressed "Really Restore?" prompt when it doesn't relate to the selected level.
//
// Revision 1.46  2003/05/19 20:29:28  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.45  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.44  2003/04/13 02:05:03  mrimer
// Changed MID_GameStart to MID_HoldStart.
//
// Revision 1.43  2003/04/08 13:08:28  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.42  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.41  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.40  2003/02/16 20:32:19  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.39  2002/11/22 02:31:10  mrimer
// Fixed "Game start" button to work for other holds.
//
// Revision 1.38  2002/11/15 02:34:12  mrimer
// Added multiple hold and player support.
// Made several vars and parameters const.
//
// Revision 1.37  2002/10/22 03:06:16  mrimer
// Removed several includes.  Revised constructor.
//
// Revision 1.36  2002/09/24 21:33:10  mrimer
// Removed superfluous, erroneous hotkey mappings.
//
// Revision 1.35  2002/08/30 20:01:19  mrimer
// NULLed all object pointers in the constructor.
//
// Revision 1.34  2002/07/22 18:44:09  mrimer
// Set cursor repeat rate to standard speed on this screen.
//
// Revision 1.33  2002/07/22 01:54:47  erikh2000
// Put in correct filename for help HTML page, even though the page doesn't exist yet.
//
// Revision 1.32  2002/07/20 23:16:54  erikh2000
// Screen now uses new CScalerWidget to draw scaled room.
//
// Revision 1.31  2002/07/11 21:01:16  mrimer
// Minor bug fix.
//
// Revision 1.30  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.29  2002/07/03 22:03:59  mrimer
// Tweaked SetRoomStatusFromAllSavedGames calls.
//
// Revision 1.28  2002/06/25 05:46:53  mrimer
// Added calls to StopRepeat().
//
// Revision 1.27  2002/06/23 22:22:08  erikh2000
// Fixed bug causing inappropriate restoral from continue game.
//
// Revision 1.26  2002/06/23 10:56:49  erikh2000
// When restoring a game, the next screen will either be the game screen or the level start screen, whichever is appropriate.
//
// Revision 1.25  2002/06/22 05:59:01  erikh2000
// Fixed a problem with the full screen sometimes painting before the fade-in transition.
//
// Revision 1.24  2002/06/21 22:32:07  mrimer
// Tweaked height of level list box.
//
// Revision 1.23  2002/06/21 18:32:31  mrimer
// Fixed list box event behavior.
//
// Revision 1.22  2002/06/21 05:19:53  mrimer
// Replaced MoveFocus() calls with InitFocus().
//
// Revision 1.21  2002/06/20 04:12:51  erikh2000
// Made changes to calls to methods that now take wstring param.
//
// Revision 1.20  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.19  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.18  2002/06/15 18:37:05  erikh2000
// Fixed places where a CDbBase-derived class was not being deleted.
//
// Revision 1.17  2002/06/14 00:59:03  erikh2000
// Changed call to obsolete GetScreenSurfaceColor() method so that it gets color from dest surface instead.
//
// Revision 1.16  2002/06/13 21:49:40  mrimer
// Minor fixes in Activate().
//
// Revision 1.15  2002/06/11 22:54:25  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.14  2002/06/11 17:44:59  mrimer
// Refined key repeat behavior.
//
// Revision 1.13  2002/06/09 06:44:32  erikh2000
// Changed code to use new message text class.
//
// Revision 1.12  2002/06/07 22:57:59  mrimer
// Added help button.
//
// Revision 1.11  2002/06/07 17:52:23  mrimer
// Fixed mouse click handling.
//
// Revision 1.10  2002/06/06 00:03:21  mrimer
// Completed mouseless UI.
//
// Revision 1.9  2002/06/05 03:18:48  mrimer
// Implemented mouseless UI.  Moved focus vars to CScreen.
//
// Revision 1.8  2002/06/01 04:57:47  mrimer
// Finished implementing hotkeys.
//
// Revision 1.7  2002/05/17 09:23:38  erikh2000
// Fixed problem with continue game showing up in room view.
//
// Revision 1.6  2002/05/12 03:18:50  erikh2000
// Many changes--basically bug fixes and checkpoint restore interface.
//
// Revision 1.5  2002/05/10 22:39:04  erikh2000
// Many changes related to logic for updating widgets.
//
// Revision 1.4  2002/04/29 00:17:20  erikh2000
// Several additions and changes that make the screen do more than previously.  You can now restore games from beginning of any explored level or room.
//
// Revision 1.3  2002/04/25 09:29:09  erikh2000
// Finished widget placement code in Load().
//
// Revision 1.2  2002/04/19 21:57:11  erikh2000
// RestoreScreen now loads and displays a monolithic 640x480 background.
//
// Revision 1.1  2002/04/10 00:26:30  erikh2000
// Initial check-in.
//
