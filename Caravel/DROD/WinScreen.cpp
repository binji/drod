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

#include "WinScreen.h"
#include "WinRoomScreen.h"
#include "WinPicScreen.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/MonsterFactory.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Ports.h>

//Used to fix scene selection problems when evaluating demos that are no longer compatible.
#define FIX_BAD_SCENE(scene) \
        (scene).wBeginTurnNo = ((scene).wBeginTurnNo <= this->wDemoEndTurnNo) ? \
                (scene).wBeginTurnNo : this->wDemoEndTurnNo; \
        (scene).wEndTurnNo = ((scene).wEndTurnNo >= (scene).wBeginTurnNo) ? \
                (scene).wEndTurnNo : (scene).wBeginTurnNo; \
        (scene).wEndTurnNo = ((scene).wEndTurnNo <= this->wDemoEndTurnNo) ? \
                (scene).wEndTurnNo : this->wDemoEndTurnNo;

//Vars that are shared between audience and room screens.
bool            CWinScreen::bBeethroTalking = false;
SCRIPTCOMMAND   CWinScreen::eCommand = SC_Intro;
UINT            CWinScreen::wDemoEndTurnNo = 0L;
WINWAITTYPE     CWinScreen::eNextCommandWaitFor = WW_Nothing;
CCurrentGame *  CWinScreen::pCurrentGame = NULL;
float           CWinScreen::fScrollRateMultiplier = 1.0;
UINT            CWinScreen::wNormalScrollRate = 33L;

//
//Protected methods.
//

//******************************************************************************
CWinScreen::CWinScreen(const SCREENTYPE eScreen) : CDrodScreen(eScreen)
	, pWinSurface(NULL)
//Constructor.
{
}

//******************************************************************************
CWinScreen::~CWinScreen(void)
//Destructor.
{
	//Any win screen may destroy the shared current game member--
	//whichever destructs first.
	if (this->pCurrentGame)
	{
		delete this->pCurrentGame;
		this->pCurrentGame = NULL;
	}
}

#define SetRate(r) this->pScrollingText->SetScrollRate((UINT)\
	(r * this->wNormalScrollRate))

//******************************************************************************
bool CWinScreen::SetForActivate(void)
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Go to credits screen if player escapes.
	g_pTheSM->ClearReturnScreens();
	g_pTheSM->InsertReturnScreen(SCR_Credits);

	//Clear any scrolling text.
	this->pScrollingText->ClearText();
	SetRate(this->bPaused ? 0 : this->fScrollRateMultiplier);

	//Set frame rate as high as needed for smooth animations.
	SetBetweenEventsInterval(12);

	HideCursor();

	return true;
}

//*****************************************************************************************
void CWinScreen::PauseAnimation()
{
	this->bPaused = true;
	SetRate(0);
}

//*****************************************************************************************
void CWinScreen::UnpauseAnimation()
{
	this->bPaused = false;
	SetRate(this->fScrollRateMultiplier);
}

//*****************************************************************************************
void CWinScreen::OnKeyDown(
//
//Params:
	const DWORD dwTagNo, const SDL_KeyboardEvent &Key)
{
	HideCursor();

	CScreen::OnKeyDown(dwTagNo,Key); //For alt-F4, F10, etc.
	if (IsDeactivating())
	{
		ResetScript();
		return;
	}

	//Check for other keys.
	switch (Key.keysym.sym)
	{
		case SDLK_PAUSE:
			PauseAnimation();
			break;
		case SDLK_SPACE:
			//toggle pause animation
			this->bPaused = !this->bPaused;
			SetRate(this->bPaused ? 0 : this->fScrollRateMultiplier);
			break;
		case SDLK_KP2: case SDLK_DOWN:
			//increase scroll rate
			this->bPaused = false;
			if (this->fScrollRateMultiplier < 4.0f) this->fScrollRateMultiplier += 0.10f;
			SetRate(this->fScrollRateMultiplier);
			break;
		case SDLK_KP8: case SDLK_UP:
			//slow scroll rate
			this->bPaused = false;
			if (this->fScrollRateMultiplier > 0.15f) this->fScrollRateMultiplier -= 0.10f;
			SetRate(this->fScrollRateMultiplier);
			break;
		default:
			UnpauseAnimation();
			break;
	}
}

//******************************************************************************
void CWinScreen::OnMouseDown(
//
//Params:
	const DWORD /*dwTagNo*/,	const SDL_MouseButtonEvent &/*Button*/)
{
	UnpauseAnimation();
}

//*****************************************************************************************
void CWinScreen::OnBetweenEvents()
//Top-level function to animate everything on the screen based on state information.
{
	CScreen::OnBetweenEvents();

	//Set Beethro talking or not based on what is shown last on the scrolling text.
	CLabelWidget *pLastLabel = this->pScrollingText->GetLastShownLabel();
	if (pLastLabel && pLastLabel->GetFontType() == F_BeethroSpeech)
		DoBeethroTalking(true);
	else
		DoBeethroTalking(false);

	Animate();

	//Is it time to do the next command?
	if (this->eCommand == 0)
		DoNextCommand();
	else
	{
		switch (this->eNextCommandWaitFor)
		{
			case WW_Nothing:
				DoNextCommand();
			break;

			case WW_ScrollText:
				if (this->pScrollingText->IsAllTextShown())
					DoNextCommand();
			break;

			case WW_DemoEnd:
				if (this->pCurrentGame && this->pCurrentGame->wTurnNo > 
						this->wDemoEndTurnNo)
					DoNextCommand();
			break;

			case WW_ScrollTextAndDemoEnd:
				if (this->pScrollingText->IsAllTextShown() &&
						this->pCurrentGame && 
						this->pCurrentGame->wTurnNo > this->wDemoEndTurnNo)
					DoNextCommand();
			break;
		}
	}
}

//*****************************************************************************
bool CWinScreen::OnQuit(void)
//Called when SDL_QUIT event is received.
//Returns whether Quit action was confirmed.
{
	//Pause action while the "Really quit?" dialog is activated.
	const bool bWasPaused = this->bPaused;
	if (!bWasPaused)
	{
		this->bPaused = true;
		SetRate(0);
	}

	const bool bQuit = CScreen::OnQuit();

	if (!bWasPaused)
	{
		this->bPaused = false;
		SetRate(this->fScrollRateMultiplier);
	} else {
		//redraw screen parts
		PaintChildren();
		UpdateRect();
	}

	return bQuit;
}

#undef SetRate

//*****************************************************************************************
void CWinScreen::DoBeethroTalking(
//
//Params:
	const bool bFlag)	//(in)
{
	this->bBeethroTalking = bFlag;
	if (bFlag)
		DrawBeethroTalking();
	else
		DrawBeethroSilent();
}

//*****************************************************************************
void CWinScreen::PaintBackground()
//Paint background.
{
	static SDL_Rect src = {0, 0, this->w, this->h};
	static SDL_Rect dest = {0, 0, this->w, this->h};
	if (this->pWinSurface)
		SDL_BlitSurface(this->pWinSurface, &src, GetDestSurface(), &dest);
	else
	{
		const SURFACECOLOR Black = {0,0,0};
		DrawFilledRect(dest, Black);
	}	
}

//
//Private methods.
//

//*****************************************************************************
CCurrentGame * CWinScreen::GetHighlightGame(
//Gets a current game loaded for the highlight demo associated with a specified 
//room.
//
//Params:
	const DWORD dwRoomID) //(in)	Room for which to find highlight demo.
//
//Returns:
//Pointer to current game or NULL if no highlight demo for room exists.
{
	PauseAnimation();

	//Get highlight demo.
	CDbDemo *pDemo;
	const DWORD dwDemoID = FindHighlightDemoForRoom(dwRoomID);
	if (!dwDemoID)
	{
		UnpauseAnimation();
		return NULL;
	}

	pDemo = g_pTheDB->Demos.GetByID(dwDemoID);	
	ASSERT(pDemo);
	if (!pDemo)
	{
		UnpauseAnimation();
		return NULL;
	}

	//Load current game for highlight demo.  Its turn will be set to beginning
	//of highlight demo.
	ASSERT(pDemo->wBeginTurnNo==0); //All highlight demos should begin on turn 0.
	CCurrentGame *pGame = pDemo->GetCurrentGame();
	delete pDemo;

	UnpauseAnimation();
	return pGame;
}

//*****************************************************************************
bool CWinScreen::ShowDemoSceneForRoom(
//Show demo scene from highlight demo that has been recorded for a specific
//room.  Only one highlight demo is present for one room ID.
//
//Params:
	const DWORD dwRoomID,		//(in)	Room associated with demo to show.
	const DWORD dwMaxTime)	//(in)	The duration in msecs of the scene to be
						//		selected from the demo.  If 0L (default), entire 
						//		demo will be shown.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwRoomID);

	PauseAnimation();

	const DWORD dwDemoID = FindHighlightDemoForRoom(dwRoomID);
	if (!dwDemoID) 
	{
		UnpauseAnimation();
		return false;
	}

	//Load the demo and select a scene.
	CDemoScene Scene;
	{
		CDbDemo *pDemo = g_pTheDB->Demos.GetByID(dwDemoID);
		if (!pDemo)
		{
			ASSERTP(false, "Failed to retrieve demo.");
			UnpauseAnimation();
			return false;
		}
		
		if (dwMaxTime == 0L)
		{
			Scene.wBeginTurnNo = pDemo->wBeginTurnNo;
			Scene.wEndTurnNo = pDemo->wEndTurnNo;
		}
		else
			pDemo->GetMostInterestingScene(dwMaxTime, Scene);
		delete pDemo;
	}

	//Call routine to finish loading the demo.
	return ShowDemoScene(dwDemoID, Scene); //ShowDemoScene() will unpause anim.
}

//*****************************************************************************
bool CWinScreen::ShowDemoScene(
//Sets the room screen to show a demo scene.  Works whether current screen
//is room or audience view.
//
//Params:
	const DWORD dwDemoID,		//(in)	Demo to show.
	CDemoScene &Scene)	//(in)	Scene within demo to show.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwDemoID);

	PauseAnimation();

	//Get pointer to room view screen.  pRoomScreen might be the same as "this"
	//pointer which seems a little silly, but no harm done.
	CWinRoomScreen *pRoomScreen = DYN_CAST(CWinRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_WinRoom));
	if (!pRoomScreen || !pRoomScreen->LoadDemoScene(dwDemoID, Scene))
	{
		ASSERTP(false, "Failed to retrieve room screen or load demo scene.");
		UnpauseAnimation();
		return false;
	}
	this->wDemoEndTurnNo = Scene.wEndTurnNo;

	UnpauseAnimation();
	return true;
}

//*****************************************************************************
bool CWinScreen::ShowRoomStart(
//Sets the room screen to show start of a visited room.  Works whether current
//screen is room or audience view.
//
//Params:
	const DWORD dwRoomID)		//(in)	Room to show.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwRoomID);

	PauseAnimation();

	//Get pointer to room view screen.  pRoomScreen might be the same as "this"
	//pointer which seems a little silly, but no harm done.
	CWinRoomScreen *pRoomScreen = DYN_CAST(CWinRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_WinRoom));
	if (!pRoomScreen || !pRoomScreen->LoadRoomStart(dwRoomID))
	{
		ASSERTP(false, "Failed to retrieve room screen or load room start.");
		UnpauseAnimation();
		return false;
	}
	this->wDemoEndTurnNo = 0;

	UnpauseAnimation();
	return true;
}

//*****************************************************************************
void CWinScreen::ShowPic(
//Sets the pic screen to show a picture.  Works whether current screen
//is set to pic view or not.
//
//Params:
	const char *pszPicName,			//(in)	Name of pic to show.
	const int nScrollTextX,			//(in)	Rectangle for scrolling text.
	const int nScrollTextY,			//		If first param (nScrollTextX) is not 
	const UINT wScrollTextW,		//		specified, the scrolling text rect 
	const UINT wScrollTextH)		//		will remain where it was placed earlier.
									//		If first param is specified, the other
									//		3 must be as well.
{
	ASSERT(pszPicName && strlen(pszPicName));
	ASSERT(nScrollTextX == -1 || (nScrollTextY > 0  && nScrollTextY > 0 &&
		wScrollTextW > 0 && wScrollTextH > 0));

	//Get pointer to pic view screen.  pPicScreen might be the same as "this"
	//pointer which seems a little silly, but no harm done.
	CWinPicScreen *pPicScreen = DYN_CAST(CWinPicScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_WinPic));
	if (!pPicScreen || !pPicScreen->LoadPic(pszPicName))
		{ASSERTP(false, "Failed to retrieve pic screen or load pic."); return;}
	
	//Set scrolling text rect if it has been specified.
	if (nScrollTextX != -1)
	{
		const SDL_Rect ScrollTextRect = {nScrollTextX, nScrollTextY, 
				wScrollTextW, wScrollTextH};
		pPicScreen->SetScrollingTextRect(ScrollTextRect);
	}

	//Update the screen if it is showing right now.
	if (pPicScreen == this) pPicScreen->FadeToNewPic();
}


//*****************************************************************************
bool CWinScreen::HasPlayerVisitedRoom(
//Has player visited a specific room?
//
//Params:
	const DWORD dwRoomID)	//(in)	Room to check.
//
//Returns:
//True if he has, false if not.
const
{
	ASSERT(dwRoomID);

	//Get the room.
	CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(dwRoomID);
	if (!pRoom) {ASSERTP(false, "Failed to retrieve room."); return false;}

	//Find one saved game in the room that is not for a demo.  (A demo could
	//have been created by another player.)
	CDbSavedGame *pSavedGame = pRoom->SavedGames.GetFirst();
	while (pSavedGame)
	{
		if (pSavedGame->eType != ST_Demo)
		{
			//Player was in the room.
			delete pSavedGame;
			delete pRoom;
			return true;
		}

		delete pSavedGame;
		pSavedGame = pRoom->SavedGames.GetNext();
	}

	//Player hasn't been in the room.
	delete pSavedGame;
	delete pRoom;
	return false;
}

//*****************************************************************************
DWORD CWinScreen::FindHighlightDemoForRoom(
//Finds a highlight demo for a specified room.
//
//Params:
	const DWORD dwRoomID) //(in)	Room for which to lookup demo.
//
//Returns:
//DemoID or 0L for no match.
const
{
	//Retrieve demo ID from player settings.
	DWORD dwDemoID;
	{
		char szRoomID[11];
		string strVarName = "hd";
		strVarName += _ltoa(dwRoomID, szRoomID, 10);
		
		CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
		dwDemoID = pCurrentPlayer->Settings.GetVar(strVarName.c_str(), (DWORD)0L);
		delete pCurrentPlayer;
	}
	
	return dwDemoID;
}

//*****************************************************************************
bool CWinScreen::InitRoom343Info(void)
//Get information used to handle demo scenes for room ID#343.
//
//Returns:
//True if an unexpected error occurred, false if not.
{
	//Get current game corresponding to highlight demo in room ID #343.
    memset(&this->Room343Info, 0, sizeof(this->Room343Info));
	this->Room343Info.dwDemoID = FindHighlightDemoForRoom(343L);
	CCurrentGame *pGame = GetHighlightGame(343L);
	if (!pGame) return false;

	//Because this is a highlight demo, the player conquered the room in it
	//and from knowledge of the room, I can expect that:
	//	 1. the player killed the 'Neather.
	
	//Seek through commands and set scene ranges based on these events.
	{
		CCueEvents CueEvents;
		COMMANDNODE *pCommand = pGame->Commands.GetFirst();
		UINT wTurnNo;
		UINT wEndTurnNo = pGame->Commands.GetSize() - 1;
		for (wTurnNo = 0; wTurnNo <= wEndTurnNo; ++wTurnNo)
		{
			ASSERT(pCommand);
			pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);
			
			//1. Check for 'Neather death.
			CMonster *pMonster = DYN_CAST(CMonster*, CAttachableObject*,
					CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab));
			if (pMonster && pMonster->wType == M_NEATHER)
			{
				//All three scenes are set from this event.
				this->Room343Info.FirstStepToBeforeDeath.wBeginTurnNo = 0;
				this->Room343Info.FirstStepToBeforeDeath.wEndTurnNo = wTurnNo - 2;

				this->Room343Info.NeatherDies.wBeginTurnNo = wTurnNo;
				this->Room343Info.NeatherDies.wEndTurnNo = wTurnNo + 2;
				
				this->Room343Info.AfterDeath.wBeginTurnNo = wTurnNo + 3;
				this->Room343Info.AfterDeath.wEndTurnNo = wEndTurnNo - 1;

				break;
			}

			if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom)
					|| !pGame->bIsGameActive)
			{
				//Probably demo was recorded in different version of DROD.
            //We've got to allow for this eventuality (just show what we can).
				goto success;
			}
				
			pCommand = pGame->Commands.GetNext();
		}
		
		//If this fires, then I didn't find the expected events in the demo.
		//Did demo end without finding expected events?
		if (wTurnNo > wEndTurnNo)
		{
			ASSERTP(false, "Unexpected events in room 343.");
			delete pGame;
			return false;
		}
	}

success:
	delete pGame;
    FIX_BAD_SCENE(this->Room343Info.AfterDeath);
    FIX_BAD_SCENE(this->Room343Info.FirstStepToBeforeDeath);
    FIX_BAD_SCENE(this->Room343Info.NeatherDies);
	return true;
}

//*****************************************************************************
bool CWinScreen::GetLevel25RoomInfo(
//Gets info for a level 25 room.
//
//Params:
	const DWORD dwRoomID,	//(in)	Room for which to get info.
	DWORD &dwDemoID,		//(out)	Highlight demo for room.
	CDemoScene &Scene)		//(out)	A scene within the demo that shows the
							//		the 'Neather fleeing the room or a scene
							//		which shows the 'Neather dieing.
//
//Returns:
//True if a highlight demo was present for the room, false if not.
{
	ASSERT(dwRoomID);

	dwDemoID = FindHighlightDemoForRoom(dwRoomID);
	CCurrentGame *pGame = GetHighlightGame(dwRoomID);
	if (!pGame) return false;

	//Because this is a highlight demo, the player conquered the room in it
	//and from knowledge of the room, I can expect that:
	//	 1. the player killed the 'Neather OR...
	//   2. The 'Neather left the room.
	
	//Seek through commands and set scene ranges based on these events.
	{
		CCueEvents CueEvents;
		COMMANDNODE *pCommand = pGame->Commands.GetFirst();
		UINT wTurnNo;
		const UINT wEndTurnNo = pGame->Commands.GetSize() - 1;
		for (wTurnNo = 0; wTurnNo <= wEndTurnNo; ++wTurnNo)
		{
			const UINT SHOW_TURNS_BEFORE = 20;
			const UINT SHOW_TURNS_AFTER = 10;

			ASSERT(pCommand);
			pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);
			
			//1. Check for 'Neather death.
			CMonster *pMonster = DYN_CAST(CMonster*, CAttachableObject*,
					CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab));
			if (pMonster && pMonster->wType == M_NEATHER)
			{
				Scene.wBeginTurnNo = (wTurnNo > SHOW_TURNS_BEFORE) ? 
						wTurnNo - SHOW_TURNS_BEFORE : 0;
				Scene.wEndTurnNo = (wTurnNo + SHOW_TURNS_AFTER < wEndTurnNo - 1) ? 
						wTurnNo + SHOW_TURNS_AFTER : wEndTurnNo - 1;

				break;
			}

			//2. Check for 'Neather leaving room.
			if (CueEvents.HasOccurred(CID_NeatherExitsRoom))
			{
				Scene.wBeginTurnNo = (wTurnNo > SHOW_TURNS_BEFORE) ? 
						wTurnNo - SHOW_TURNS_BEFORE : 0;
				Scene.wEndTurnNo = (wTurnNo + SHOW_TURNS_AFTER < wEndTurnNo - 1) ? 
						wTurnNo + SHOW_TURNS_AFTER : wEndTurnNo - 1;

				break;
			}

			if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom)
					|| !pGame->bIsGameActive)
			{
				//Probably demo was recorded in different version of DROD.
            //Show whatever can be shown before something bad happens.
            Scene.wBeginTurnNo = 0;
            Scene.wEndTurnNo = wTurnNo > 0 ? wTurnNo - 1 : 0;
				goto success;
			}
				
			pCommand = pGame->Commands.GetNext();
		}
		
		//Did demo end without finding expected events?
		if (wTurnNo > wEndTurnNo)
		{
			ASSERTP(false, "Unexpected events in level 25.");
			delete pGame;
			return false;
		}
	}

success:
	delete pGame;
	return true;
}

//*****************************************************************************
bool CWinScreen::InitRoom344Info(void)
//Gets info for room ID#344.
//
//Returns:
//True if a highlight demo was present for the room, false if not.
{
    memset(&this->Room344Info, 0, sizeof(this->Room344Info));
	this->Room344Info.dwDemoID = FindHighlightDemoForRoom(344L);
	CCurrentGame *pGame = GetHighlightGame(344L);
	if (!pGame) return false;

	//Scene will be the entire set of commands except for last when player 
	//steps out of the room.
    if (pGame->Commands.GetSize() >= 2)
	    this->Room344Info.Scene.wEndTurnNo = pGame->Commands.GetSize() - 2;

	delete pGame;

    FIX_BAD_SCENE(this->Room344Info.Scene);
	return true;
}

//*****************************************************************************
bool CWinScreen::InitRoom103Info(void)
//Get information used to handle demo scenes in RoomID#103.
//
//Returns:
//True if an unexpected error occurred, false if not.  If the player doesn't have
//a highlight demo saved for RoomID#103 that is an expected error.
{
	CCueEvents CueEvents;

	//Get current game corresponding to highlight demo in room ID #103.
    memset(&this->Room103Info, 0, sizeof(this->Room103Info));
	this->Room103Info.dwDemoID = FindHighlightDemoForRoom(103L);
	CCurrentGame *pGame = GetHighlightGame(103L);
	if (!pGame) return false;
	
	//Because this is a highlight demo, the player conquered the room in it
	//and from knowledge of the room, I can expect that:
	//	 1. the southernmost orb was hit.
	//   2. player kills roach queen.
	//   3. the serpent died.

	//Seek through commands and set scene ranges based on these events.
	{
		COMMANDNODE *pCommand = pGame->Commands.GetFirst();
		UINT wTurnNo;
		UINT wEndTurnNo = pGame->Commands.GetSize() - 1;
		for (wTurnNo = 0; wTurnNo <= wEndTurnNo; ++wTurnNo)
		{
			ASSERT(pCommand);
			pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);
			
			//1. Check for first orb hit.
			if (CueEvents.HasOccurred(CID_OrbActivated) &&
					this->Room103Info.FirstStepToOrbHit.wEndTurnNo == 0)
			{
				this->Room103Info.FirstStepToOrbHit.wBeginTurnNo = 0;
				this->Room103Info.FirstStepToOrbHit.wEndTurnNo = 
						this->Room103Info.OrbHitToWait.wBeginTurnNo =
						wTurnNo + 1;
			}

			//2. Check for queen death.
			CMonster *pMonster = DYN_CAST(CMonster*, CAttachableObject*,
					CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab));
			if (this->Room103Info.FirstStepToOrbHit.wEndTurnNo != 0 &&
					this->Room103Info.OrbHitToWait.wEndTurnNo == 0 &&
					pMonster && pMonster->wType == M_QROACH)
			{
				this->Room103Info.OrbHitToWait.wEndTurnNo = 
						this->Room103Info.QueenDeathToEnd.wBeginTurnNo = 
						wTurnNo + 1;
			}

			//3. Check for serpent death.
			if (CueEvents.GetFirstPrivateData(CID_SnakeDiedFromTruncation))
			{
				ASSERT(this->Room103Info.QueenDeathToEnd.wBeginTurnNo != 0);
				this->Room103Info.QueenDeathToEnd.wEndTurnNo = wTurnNo + 5;
				break;
			}

			if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom)
					|| !pGame->bIsGameActive)
			{
				//Probably demo was recorded in different version of DROD.
            //We've got to allow for this eventuality (just show what we can).
				goto success;
			}
				
			pCommand = pGame->Commands.GetNext();
		}
		
		//Did demo end without finding expected events?
		if (wTurnNo > wEndTurnNo ||
			this->Room103Info.FirstStepToOrbHit.wEndTurnNo == 0 ||
			this->Room103Info.OrbHitToWait.wEndTurnNo == 0 ||
			this->Room103Info.QueenDeathToEnd.wEndTurnNo == 0)
		{
			ASSERTP(false, "Unexpected events in room 103.");
			delete pGame;
			return false;
		}
	}

success:
	delete pGame;
    FIX_BAD_SCENE(this->Room103Info.FirstStepToOrbHit);
    FIX_BAD_SCENE(this->Room103Info.OrbHitToWait);
    FIX_BAD_SCENE(this->Room103Info.QueenDeathToEnd);
	return true;
}

//*****************************************************************************
bool CWinScreen::InitRoom165Info(void)
//Get information used to handle demo scenes in RoomID#165.
//
//Returns:
//True if an unexpected error occurred, false if not.
{
	CCueEvents CueEvents;

	//Get highlight demo.
    memset(&this->Room165Info, 0, sizeof(this->Room165Info));
	CDbDemo *pDemo;
	const DWORD dwDemoID = FindHighlightDemoForRoom(165L);
	if (!dwDemoID) return false;
	this->Room165Info.dwDemoID = dwDemoID;

	pDemo = g_pTheDB->Demos.GetByID(dwDemoID);	
	ASSERT(pDemo);
	if (!pDemo) return false;

	//Get current game corresponding to highlight demo in room ID #165.
	CCurrentGame *pGame = pDemo->GetCurrentGame();
	if (!pGame) return false;

	//Because this is a highlight demo, the player conquered the room in it
	//and from knowledge of the room, I can expect that:
	//	 1. the trapdoor square at 15,13 was stepped on.

	//Seek through commands and set scene ranges based on these events.
	{
		COMMANDNODE *pCommand = pGame->Commands.GetFirst();
		UINT wTurnNo;
		UINT wEndTurnNo = pGame->Commands.GetSize() - 1;
		for (wTurnNo = 0; wTurnNo <= wEndTurnNo; ++wTurnNo)
		{
			ASSERT(pCommand);
			pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);
			
			//1. Check for moving onto trapdoor square.
			if (pGame->swordsman.wX == 15 && pGame->swordsman.wY == 13)
			{
				//Get scene of specified length that begins 30 turns before
				//the trapdoor square is discovered.
				wTurnNo = (wTurnNo < 30) ? 0 : wTurnNo - 30;
				pDemo->GetSceneFromBeginTurnNo(30000L, this->Room165Info.Discovery,
						wTurnNo);
				break;
			}

			if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom)
					|| !pGame->bIsGameActive)
			{
				//Probably demo was recorded in different version of DROD.
            //We've got to allow for this eventuality (just show what we can).
				goto success;
			}
				
			pCommand = pGame->Commands.GetNext();
		}
		
		//Did demo end without finding expected events?
		if (wTurnNo > wEndTurnNo ||
				this->Room165Info.Discovery.wEndTurnNo == 0)
		{
			ASSERTP(false, "Unexpected events in room 165.");
			delete pDemo;
			delete pGame;
			return false;
		}
	}

success:
	delete pDemo;
	delete pGame;
    FIX_BAD_SCENE(this->Room165Info.Discovery);
	return true;
}

//*****************************************************************************
bool CWinScreen::InitRoom109Info(void)
//Get information used to handle demo scenes in RoomID#109.
//
//Returns:
//True if demo loaded successfully, false if not.  Errors involving the 
//content of the demo are debug errors.
{
	CCueEvents CueEvents;

	//Get current game corresponding to highlight demo in room ID #109.
    memset(&this->Room109Info, 0, sizeof(this->Room109Info));
	this->Room109Info.dwDemoID = FindHighlightDemoForRoom(109L);
	CCurrentGame *pGame = GetHighlightGame(109L);
	if (!pGame) return false;
	
	//Because this is a highlight demo, the player conquered the room in it
	//and from knowledge of the room, I can expect that:
	//	 1. the orb was hit
	//   2. the serpent was killed.  
	//Seek through commands and set scene ranges based on these two events.
	{
		COMMANDNODE *pCommand = pGame->Commands.GetFirst();
		UINT wTurnNo;
		UINT wEndTurnNo = pGame->Commands.GetSize() - 1;
		for (wTurnNo = 0; wTurnNo <= wEndTurnNo; ++wTurnNo)
		{
			ASSERT(pCommand);
			pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);
			
			//Check for first orb hit.
			if (CueEvents.HasOccurred(CID_OrbActivated) &&
					this->Room109Info.FirstStepToOrbHit.wEndTurnNo == 0)
			{
				this->Room109Info.FirstStepToOrbHit.wBeginTurnNo = 0;
				this->Room109Info.FirstStepToOrbHit.wEndTurnNo = 
						this->Room109Info.OrbHitToSerpentAlmostDead.wBeginTurnNo =
						wTurnNo + 1;
			}

			//Check for serpent death.
			if (CueEvents.HasOccurred(CID_SnakeDiedFromTruncation))
			{
				ASSERT(this->Room109Info.OrbHitToSerpentAlmostDead.wEndTurnNo == 0);
				this->Room109Info.OrbHitToSerpentAlmostDead.wEndTurnNo =
						this->Room109Info.SerpentAlmostDeadToEnd.wBeginTurnNo = 
						wTurnNo - 5;
				this->Room109Info.SerpentAlmostDeadToEnd.wEndTurnNo = wTurnNo + 5 >
                  wEndTurnNo ? wEndTurnNo : wTurnNo + 5;

				//A few safe assumptions being used above:
				//1.	It's going to take more than 5 turns from orb hit to kill 
				//		the snake.
				//2.	It's going to take more than 5 turns to exit the room after
				//		killing the snake.

				break;
			}

			if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom)
					|| !pGame->bIsGameActive)
			{
                //Probably demo was recorded in different version of DROD.
                //We've got to allow for this eventuality (just show what we can).
                goto success;
			}

			pCommand = pGame->Commands.GetNext();
		}

		//Did demo end without finding expected events?
		if (wTurnNo > wEndTurnNo ||
				this->Room109Info.FirstStepToOrbHit.wEndTurnNo == 0 ||
				this->Room109Info.OrbHitToSerpentAlmostDead.wEndTurnNo == 0 ||
				this->Room109Info.SerpentAlmostDeadToEnd.wEndTurnNo == 0)
		{
             //Probably demo was recorded in different version of DROD.
             //We've got to allow for this eventuality (just show what we can).
			goto success;
		}
	}

success:
	delete pGame;
    FIX_BAD_SCENE(this->Room109Info.FirstStep);
    FIX_BAD_SCENE(this->Room109Info.FirstStepToOrbHit);
    FIX_BAD_SCENE(this->Room109Info.OrbHitToSerpentAlmostDead);
    FIX_BAD_SCENE(this->Room109Info.SerpentAlmostDeadToEnd);
	return true;
}

//*****************************************************************************
bool CWinScreen::InitRoom241Info(void)
//Get information used to handle demo scenes in RoomID#241.
//
//Returns:
//True if demo loaded successfully, false if not.  Errors involving the 
//content of the demo are debug errors.
{
	CCueEvents CueEvents;

	//Get current game corresponding to highlight demo in room ID #241.
    memset(&this->Room241Info, 0, sizeof(this->Room241Info));
	this->Room241Info.dwDemoID = FindHighlightDemoForRoom(241L);
	CCurrentGame *pGame = GetHighlightGame(241L);
	if (!pGame) return false;

	//From which direction did the swordsman enter the room?
	if (pGame->swordsman.wY == 0)
		this->Room241Info.wEnterDirection = N;
	else if (pGame->swordsman.wY == 31)
		this->Room241Info.wEnterDirection = S;
	else if (pGame->swordsman.wX == 0)
		this->Room241Info.wEnterDirection = W;
	else
		ASSERTP(false, "Unexpected events in room 241.");

	//Set end turns of scenes to 0.  This will indicate an unspecifed
	//state that can be used in the logic below.
	this->Room241Info.Northwest.wEndTurnNo = 
		this->Room241Info.Northeast.wEndTurnNo = 
		this->Room241Info.Southwest.wEndTurnNo = 
		this->Room241Info.Southeast.wEndTurnNo = 0;
	
	//Because this is a highlight demo, the player conquered the room in it
	//and from knowledge of the room, I can expect that:
	//	 1. the northwest area was entered.
	//   2. the northeast area was entered.
	//   3. the southwest area was entered.
	//   4. the southeast area was entered.
	//
	//   The first four can happen in any sequence.
	//
	//   5. the last monster is killed.
	//
	//Seek through commands and set scene ranges based on these events.
	{
		bool bEnteredNW = false, bEnteredNE = false, 
				bEnteredSW = false, bEnteredSE = false;
		UINT wEnterCount = 0;
		COMMANDNODE *pCommand = pGame->Commands.GetFirst();
		UINT *pwLastSceneEndTurnNo = NULL;
		UINT wTurnNo;
		UINT wEndTurnNo = pGame->Commands.GetSize() - 1;
		for (wTurnNo = 0; wTurnNo <= wEndTurnNo; ++wTurnNo)
		{
			ASSERT(pCommand);
			pGame->ProcessCommand(static_cast<int>(pCommand->bytCommand), CueEvents);
			
			//Check for entrance into northwest area. (#1)
			if ( !bEnteredNW && 
					((pGame->swordsman.wX == 14 && pGame->swordsman.wY == 3) ||
					(pGame->swordsman.wX == 17 && pGame->swordsman.wY == 11)) )
			{
				ASSERT(this->Room241Info.Northwest.wEndTurnNo == 0);
				if (pwLastSceneEndTurnNo)
				{
					this->Room241Info.Northwest.wBeginTurnNo = wTurnNo - 3;
					*pwLastSceneEndTurnNo = wTurnNo - 4;
				}
				else
					this->Room241Info.Northwest.wBeginTurnNo = 0;
				pwLastSceneEndTurnNo = &(this->Room241Info.Northwest.wEndTurnNo);
				this->Room241Info.wNorthwestSequenceNo = wEnterCount++;
				bEnteredNW = true;
			}

			//Check for entrance into northeast area. (#2)
			if ( !bEnteredNE && 
					((pGame->swordsman.wX == 22 && pGame->swordsman.wY == 5) ||
					(pGame->swordsman.wX == 22 && pGame->swordsman.wY == 14)) )
			{
				ASSERT(this->Room241Info.Northeast.wEndTurnNo == 0);
				if (pwLastSceneEndTurnNo)
				{
					this->Room241Info.Northeast.wBeginTurnNo = wTurnNo - 3;
					*pwLastSceneEndTurnNo = wTurnNo - 4;
				}
				else
					this->Room241Info.Northeast.wBeginTurnNo = 0;
				pwLastSceneEndTurnNo = &(this->Room241Info.Northeast.wEndTurnNo);
				this->Room241Info.wNortheastSequenceNo = wEnterCount++;
				bEnteredNE = true;
			}

			//Check for entrance into southwest area. (#3)
			if ( !bEnteredSW && 
					((pGame->swordsman.wX == 10 && pGame->swordsman.wY == 20) ||
					(pGame->swordsman.wX == 15 && pGame->swordsman.wY == 24)) )
			{
				ASSERT(this->Room241Info.Southwest.wEndTurnNo == 0);
				if (pwLastSceneEndTurnNo)
				{
					this->Room241Info.Southwest.wBeginTurnNo = wTurnNo - 3;
					*pwLastSceneEndTurnNo = wTurnNo - 4;
				}
				else
					this->Room241Info.Southwest.wBeginTurnNo = 0;
				pwLastSceneEndTurnNo = &(this->Room241Info.Southwest.wEndTurnNo);
				this->Room241Info.wSouthwestSequenceNo = wEnterCount++;
				bEnteredSW = true;
			}

			//Check for entrance into southeast area. (#4)
			if ( !bEnteredSE && 
					((pGame->swordsman.wX == 21 && pGame->swordsman.wY == 18) ||
					(pGame->swordsman.wX == 19 && pGame->swordsman.wY == 24)) )
			{
				ASSERT(this->Room241Info.Southeast.wEndTurnNo == 0);
				if (pwLastSceneEndTurnNo)
				{
					this->Room241Info.Southeast.wBeginTurnNo = wTurnNo - 3;
					*pwLastSceneEndTurnNo = wTurnNo - 4;
				}
				else
					this->Room241Info.Southeast.wBeginTurnNo = 0;
				pwLastSceneEndTurnNo = &(this->Room241Info.Southeast.wEndTurnNo);
				this->Room241Info.wSoutheastSequenceNo = wEnterCount++;
				bEnteredSE = true;
			}

			//Check for killing the last monster. (#5)
			if (CueEvents.HasOccurred(CID_AllMonstersKilled))
			{
				ASSERT(bEnteredNW && bEnteredNE && bEnteredSW && bEnteredSE);
				ASSERT(pwLastSceneEndTurnNo);

				*pwLastSceneEndTurnNo = wTurnNo + 3;

				//I have seen all the events, so stop looking at commands.
				break;
			}

			if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom)
					|| !pGame->bIsGameActive)
			{
				//Probably demo was recorded in different version of DROD.
            //Show whatever can be shown before something bad happens.
            goto success;
			}
				
			pCommand = pGame->Commands.GetNext();
		}
		
		//Did demo end without finding expected events?
		if (wTurnNo > wEndTurnNo ||
			this->Room241Info.Northwest.wEndTurnNo == 0 ||
			this->Room241Info.Northeast.wEndTurnNo == 0 ||
			this->Room241Info.Southwest.wEndTurnNo == 0 ||
			this->Room241Info.Southeast.wEndTurnNo == 0)
		{
			ASSERTP(false, "Unexpected events in room 241 (2).");
			delete pGame;
			return false;
		}
	}

success:
	delete pGame;
    FIX_BAD_SCENE(this->Room241Info.FirstStep);
    FIX_BAD_SCENE(this->Room241Info.Northeast);
    FIX_BAD_SCENE(this->Room241Info.Northwest);
    FIX_BAD_SCENE(this->Room241Info.Southeast);
    FIX_BAD_SCENE(this->Room241Info.Southwest);
	return true;
}

//*****************************************************************************
void CWinScreen::DoNextCommand(void)
//Calls routines that change widgets and state.
//
//NOTE: Keep this routine at the end of the file since it is so long.  I don't
//want to break it up into smaller routines, since it is easiest to read in
//a linear fashion.
{
	//Macros for readability below.
#	define AUDIENCE(text) \
		this->pScrollingText->AddText((text), F_AudienceSpeech)
#	define BEETHRO(text) \
		this->pScrollingText->AddText((text), F_BeethroSpeech)
#	define AUDIENCE_DB(mid) \
		this->pScrollingText->AddText(g_pTheDB->GetMessageText(mid), F_AudienceSpeech);\
      this->pScrollingText->AddText(wszCRLF, F_AudienceSpeech)
#	define BEETHRO_DB(mid) \
		this->pScrollingText->AddText(g_pTheDB->GetMessageText(mid), F_BeethroSpeech);\
      this->pScrollingText->AddText(wszCRLF, F_BeethroSpeech)
#	define BEFORE_NEW_SCREEN \
      this->pScrollingText->AddText(wszCRLF, F_BeethroSpeech);\
      this->pScrollingText->AddText(wszCRLF, F_BeethroSpeech);\
      this->pScrollingText->AddText(wszCRLF, F_BeethroSpeech)
	
	switch (this->eCommand)
	{
		//Intro to Beethro's storytelling.
		case SC_Intro: 
			AUDIENCE_DB(MID_WinIntro1);
			BEETHRO_DB(MID_WinIntro2); 
			AUDIENCE_DB(MID_WinIntro3);
			BEETHRO_DB(MID_WinIntro4); 
			AUDIENCE_DB(MID_WinIntro5); 
			BEETHRO_DB(MID_WinIntro6); 
			AUDIENCE_DB(MID_WinIntro7);
			BEETHRO_DB(MID_WinIntro8);
			AUDIENCE_DB(MID_WinIntro9);
			BEETHRO_DB(MID_WinIntro10);
			AUDIENCE_DB(MID_WinIntro11);
			BEETHRO_DB(MID_WinIntro12);
         BEFORE_NEW_SCREEN;

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		//Level 1.
		case SC_Level1:
		{
			if (!ShowDemoSceneForRoom(3L, 20000L))	//1:5N2W
			{
				this->eCommand = SC_Error; //No demo found--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			GoToScreen(SCR_WinRoom);
			this->eNextCommandWaitFor = WW_Nothing;
		}
		break;

		case SC_Level1b:
			BEETHRO_DB(MID_WinLevel1);
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 2.
		case SC_Level2:
			if (!ShowDemoSceneForRoom(34L, 15000L)) //2:2S3E
			{
				this->eCommand = SC_Error; //No demo found--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			BEETHRO_DB(MID_WinLevel2_1);
			AUDIENCE_DB(MID_WinLevel2_2);
			BEETHRO_DB(MID_WinLevel2_3);
			AUDIENCE_DB(MID_WinLevel2_4);
			BEFORE_NEW_SCREEN;

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level2b:
			GoToScreen(SCR_WinAudience);
			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_Level2c:
			BEETHRO_DB(MID_WinLevel2_5);
			AUDIENCE_DB(MID_WinLevel2_6);
			BEETHRO_DB(MID_WinLevel2_7);
			AUDIENCE_DB(MID_WinLevel2_8);
			BEETHRO_DB(MID_WinLevel2_9);
			BEFORE_NEW_SCREEN;
			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		//Level 3.
		case SC_Level3:
			if (!ShowDemoSceneForRoom(36L, 30000L)) //3:3N1E
			{
				this->eCommand = SC_Error; //No demo found--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			GoToScreen(SCR_WinRoom);
			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_Level3b:
			BEETHRO_DB(MID_WinLevel3_1);
			AUDIENCE_DB(MID_WinLevel3_2);
			BEETHRO_DB(MID_WinLevel3_3);
			AUDIENCE_DB(MID_WinLevel3_4);
			BEETHRO_DB(MID_WinLevel3_5);
			AUDIENCE_DB(MID_WinLevel3_6);
			BEETHRO_DB(MID_WinLevel3_7);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 4.
		case SC_Level4:
			if (!ShowDemoSceneForRoom(62L, 20000L)) //4:2E.
			{
				this->eCommand = SC_Error; //No demo found--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			BEETHRO_DB(MID_WinLevel4_1);
			AUDIENCE_DB(MID_WinLevel4_2);
			BEETHRO_DB(MID_WinLevel4_3);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level4b:
			if (!ShowDemoSceneForRoom(57L, 10000L)) //4:1N1E.
			{
				this->eCommand = SC_Error; //No demo found--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			AUDIENCE_DB(MID_WinLevel4_4);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
			if (HasPlayerVisitedRoom(55L)) //4:2N1E
			{
				BEETHRO_DB(MID_WinLevel4_5);
            BEETHRO(wszCRLF);

				//Player could have skipped over level 5 and 6.  Check for the 
				//highlight demos that I will need to show in script commands below.  
				//If they aren't there, then I'll assume the player skipped the 
				//one or both of these levels.
				const bool bSkippedLevel5 = !FindHighlightDemoForRoom(69L); //5:2N2W
				const bool bSkippedLevel6 = !FindHighlightDemoForRoom(96L); //6:Entrance
				if (bSkippedLevel5 || bSkippedLevel6)
				{
					WSTRING wstrText = g_pTheDB->GetMessageText(MID_WinLevel4_6);
               wstrText += wszSpace;
					wstrText += bSkippedLevel5 && bSkippedLevel6 ?
                     g_pTheDB->GetMessageText(MID_WinLevel4_6Levels) :
                     g_pTheDB->GetMessageText(MID_WinLevel4_6Level);
               wstrText += wszSpace;
					if (bSkippedLevel5) 
					{
						wstrText += g_pTheDB->GetMessageText(MID_Five);
                  wstrText += wszSpace;
						if (bSkippedLevel6)
                     wstrText += g_pTheDB->GetMessageText(MID_And);
                  wstrText += wszSpace;
					}
					if (bSkippedLevel6)
               {
                  wstrText += wszSpace;
						wstrText += g_pTheDB->GetMessageText(MID_Six);
               }
					wstrText += wszPeriod;
               wstrText += wszCRLF;
					BEETHRO(	wstrText.c_str());
					AUDIENCE_DB(MID_WinLevel4_7);
					BEETHRO_DB(MID_WinLevel4_8);
					AUDIENCE_DB(MID_WinLevel4_9);
				}
				
				if (bSkippedLevel5)
				{
					if (bSkippedLevel6)
						this->eCommand = SC_Level7;
					else
						this->eCommand = SC_Level6;
					return;
				}
			}
			else
			{
				BEETHRO_DB(MID_WinLevel4_10);
				AUDIENCE_DB(MID_WinLevel4_11);
				BEETHRO_DB(MID_WinLevel4_12);
				AUDIENCE_DB(MID_WinLevel4_13);
			}
		break;

		//Level 5.
		case SC_Level5:
			if (!InitRoom109Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room109Info.dwDemoID, 
					this->Room109Info.FirstStep)) ASSERTP(false, "Unexpected events in room 109.");
			
			BEETHRO_DB(MID_WinLevel5_1);
			AUDIENCE_DB(MID_WinLevel5_2);
			BEETHRO_DB(MID_WinLevel5_3);
			AUDIENCE_DB(MID_WinLevel5_4);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level5b:
			if (!ShowDemoSceneForRoom(69L, 10000L)) //5:2N2W
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			BEETHRO_DB(MID_WinLevel5_5);
		
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 6.
		case SC_Level6:
		{
			if (!ShowDemoScene(this->Room109Info.dwDemoID, 
					this->Room109Info.FirstStep)) ASSERTP(false, "Unexpected events in room 109 (2).");
			BEETHRO_DB(MID_WinLevel6_1);
			AUDIENCE_DB(MID_WinLevel6_2);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;

			const bool bSkippedLevel6 = !FindHighlightDemoForRoom(96L); //6:Entrance
			if (bSkippedLevel6)
			{
				BEETHRO_DB(MID_WinLevel6_3);
				AUDIENCE_DB(MID_WinLevel6_4);
				BEETHRO_DB(MID_WinLevel6_5);
				AUDIENCE_DB(MID_WinLevel6_6);
				
				this->eCommand = SC_Level7;
				return;
			}
		}
		break;

		case SC_Level6b:
			if (!ShowDemoSceneForRoom(96L, 5000L)) //6:Entrance
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			BEETHRO_DB(MID_WinLevel6_7);
			AUDIENCE_DB(MID_WinLevel6_8);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 7.
		case SC_Level7:
			if (!ShowDemoScene(this->Room109Info.dwDemoID, 
					this->Room109Info.FirstStepToOrbHit)) ASSERTP(false, "Unexpected events in room 109 (3).");
			
			BEETHRO_DB(MID_WinLevel7_1);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level7b:
			if (!ShowDemoScene(this->Room109Info.dwDemoID,
					this->Room109Info.OrbHitToSerpentAlmostDead)) ASSERTP(false, "Unexpected events in room 109 (4).");

			BEETHRO_DB(MID_WinLevel7_2);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level7c:
			if (!ShowDemoScene(this->Room109Info.dwDemoID,
					this->Room109Info.SerpentAlmostDeadToEnd)) ASSERTP(false, "Unexpected events in room 109 (5).");

			BEETHRO_DB(MID_WinLevel7_3);

			AUDIENCE_DB(MID_WinLevel7_4);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level7d:
			if (!InitRoom103Info() || !ShowRoomStart(103L))
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			
			BEETHRO_DB(MID_WinLevel7_5);

			this->eNextCommandWaitFor = WW_ScrollText;
			
			if (!this->Room103Info.dwDemoID) //Player didn't conquer room.
			{
				BEETHRO_DB(MID_WinLevel7_6);
				AUDIENCE_DB(MID_WinLevel7_7);
				BEETHRO_DB(MID_WinLevel7_8);
				
				//Skip over script commands dealing with this room.
				this->eCommand = SC_Level8;
				return;
			}
		break;

		case SC_Level7e:
			if (!ShowDemoScene(this->Room103Info.dwDemoID, 
					this->Room103Info.FirstStepToOrbHit))
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel7_9);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level7f:
			if (!ShowDemoScene(this->Room103Info.dwDemoID, 
					this->Room103Info.OrbHitToWait))
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel7_10);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level7g:
			if (!ShowDemoScene(this->Room103Info.dwDemoID, 
					this->Room103Info.QueenDeathToEnd))
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel7_11);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 8.
		case SC_Level8:
			if (!ShowDemoSceneForRoom(134L, 60000L)) //8:7S3E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			AUDIENCE_DB(MID_WinLevel8_1);
			BEETHRO_DB(MID_WinLevel8_2);
			AUDIENCE_DB(MID_WinLevel8_3);
			BEETHRO_DB(MID_WinLevel8_4);
			AUDIENCE_DB(MID_WinLevel9_1);
			BEETHRO_DB(MID_WinLevel9_2);
			AUDIENCE_DB(MID_WinLevel10_1);
			BEETHRO_DB(MID_WinLevel10_2);
			AUDIENCE_DB(MID_WinLevel10_3);
			BEETHRO_DB(MID_WinLevel10_4);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 10.
		case SC_Level10:
			if (!ShowDemoSceneForRoom(150L, 30000L)) //10:1N1E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			BEETHRO_DB(MID_WinLevel10_5);
			AUDIENCE_DB(MID_WinLevel10_6);
			BEETHRO_DB(MID_WinLevel10_7);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 11.
		case SC_Level11:
			if (!InitRoom165Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room165Info.dwDemoID, 
					this->Room165Info.Discovery)) ASSERTP(false, "Unexpected events in room 165.");

			BEETHRO_DB(MID_WinLevel11_1);
			AUDIENCE_DB(MID_WinLevel11_2);
			BEETHRO_DB(MID_WinLevel11_3);
			AUDIENCE_DB(MID_WinLevel11_4);
			BEFORE_NEW_SCREEN;
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Halph's question.
		case SC_HalphQuestionA:
			ShowPic("HalphHopeful", 9, 10, 208, 140);
			GoToScreen(SCR_WinPic);
			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_HalphQuestionB:
			AUDIENCE_DB(MID_WinHalph1);
			BEETHRO_DB(MID_WinHalph2);

			this->eNextCommandWaitFor = WW_ScrollText;
		break;
		
		case SC_HalphQuestionC:
			ShowPic("HalphSad");
			
			AUDIENCE_DB(MID_WinHalph3);
			BEETHRO_DB(MID_WinHalph4);
			AUDIENCE_DB(MID_WinHalph5);
			BEETHRO_DB(MID_WinHalph6);
			AUDIENCE_DB(MID_WinHalph7);
			BEFORE_NEW_SCREEN;
			
			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		//Level 12.
		case SC_Level12:
			if (!ShowDemoSceneForRoom(177L, 45000L))	//12:1S2E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			GoToScreen(SCR_WinRoom);
			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_Level12b:
			BEETHRO_DB(MID_WinLevel12_1);
  			BEFORE_NEW_SCREEN;   //Beethro will stop talking.
			AUDIENCE_DB(MID_WinLevel12_2);
			BEETHRO_DB(MID_WinLevel12_3);
			AUDIENCE_DB(MID_WinLevel12_4);

			if (HasPlayerVisitedRoom(168L))
			{
				BEETHRO_DB(MID_WinLevel12_5);
				AUDIENCE_DB(MID_WinLevel12_6);
				BEETHRO_DB(MID_WinLevel12_7);
			}
			else
			{
				BEETHRO_DB(MID_WinLevel12_8);
				AUDIENCE_DB(MID_WinLevel12_9);
				BEETHRO_DB(MID_WinLevel12_10);
				AUDIENCE_DB(MID_WinLevel12_11);
				BEETHRO_DB(MID_WinLevel12_12);
			}

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 13.
		case SC_Level13:
			if (!ShowDemoSceneForRoom(200L, 30000L))	//13:2S2W
			{
				BEETHRO_DB(MID_WinLevel13_1);
				AUDIENCE_DB(MID_WinLevel13_2);
				BEETHRO_DB(MID_WinLevel13_3);
				AUDIENCE_DB(MID_WinLevel13_4);
				BEETHRO_DB(MID_WinLevel13_5);
				AUDIENCE_DB(MID_WinLevel13_6);
				BEETHRO_DB(MID_WinLevel13_7);
				AUDIENCE_DB(MID_WinLevel13_8);

				this->eNextCommandWaitFor = WW_ScrollText;
				break;
			}

			ShowRoomStart(192L);	//L13:Entrance

			BEETHRO_DB(MID_WinLevel13_9);
			AUDIENCE_DB(MID_WinLevel13_10);
			BEETHRO_DB(MID_WinLevel13_11);

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		//Level 14.
		case SC_Level14:
			//Note that B will say same thing whether or not demo shows.  Player
			//may have skipped over level 14.
			ShowDemoSceneForRoom(216L, 20000L);
			
			AUDIENCE_DB(MID_WinLevel14_1);
			BEETHRO_DB(MID_WinLevel14_2);
			AUDIENCE_DB(MID_WinLevel14_3);
			BEETHRO_DB(MID_WinLevel14_4);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 15.
		case SC_Level15:
			if (!ShowDemoSceneForRoom(228L, 60000L))	//15:1S2W
			{
				//Just go on to level 16.  Player may have skipped over level 15.
				//Or player may have not conquered this room.  Level 15 will let
				//you leave early.
				this->eCommand = SC_Level16;
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			{
				const bool bSkippedLevel14 = !FindHighlightDemoForRoom(216L); //14:1S
				if (bSkippedLevel14)
				{
					BEETHRO_DB(MID_WinLevel15_1);
				}
			}
			BEETHRO_DB(MID_WinLevel15_2);
			AUDIENCE_DB(MID_WinLevel15_3);
			BEETHRO_DB(MID_WinLevel15_4);
			AUDIENCE_DB(MID_WinLevel15_5);
			BEETHRO_DB(MID_WinLevel15_6);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 16.
		case SC_Level16:
      {
			if (!InitRoom241Info()) //16:2S1E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			
			ShowDemoScene(this->Room241Info.dwDemoID, this->Room241Info.FirstStep);
			BEETHRO_DB(MID_WinLevel16_1);
         WSTRING wStr = g_pTheDB->GetMessageText(MID_WinLevel16_2);
         wStr += wszSpace;
			switch (this->Room241Info.wEnterDirection)
			{
				case N: wStr += g_pTheDB->GetMessageText(MID_North); break;
				case S: wStr += g_pTheDB->GetMessageText(MID_South); break;
				case W: wStr += g_pTheDB->GetMessageText(MID_West); break;
				default: ASSERTP(false, "Bad entrance to room 241."); break;
			}
         wStr += wszPeriod;
         wStr += wszCRLF;
         AUDIENCE(wStr.c_str());
			BEETHRO_DB(MID_WinLevel16_3);
			AUDIENCE_DB(MID_WinLevel16_4);

			switch (this->Room241Info.wEnterDirection)
			{
				case N: BEETHRO_DB(MID_WinLevel16_5); break;
				case S: BEETHRO_DB(MID_WinLevel16_6); break;
				case W: BEETHRO_DB(MID_WinLevel16_7); break;
			}

			AUDIENCE_DB(MID_WinLevel16_8);
			BEETHRO_DB(MID_WinLevel16_9);
			AUDIENCE_DB(MID_WinLevel16_10);
			BEETHRO_DB(MID_WinLevel16_11);
			AUDIENCE_DB(MID_WinLevel16_12);
			BEETHRO_DB(MID_WinLevel16_13);

			this->eNextCommandWaitFor = WW_ScrollText;
      }
		break;

		case SC_Level16b:
		{
			//Figure out which chamber was entered second.
			const UINT wSecondChamber = 
				(this->Room241Info.wNorthwestSequenceNo == 1) * NW +
				(this->Room241Info.wNortheastSequenceNo == 1) * NE +
				(this->Room241Info.wSouthwestSequenceNo == 1) * SW +
				(this->Room241Info.wSoutheastSequenceNo == 1) * SE;

         WSTRING wStr = g_pTheDB->GetMessageText(MID_WinLevel16_14);
         wStr += wszSpace;
			switch (wSecondChamber)
			{
				case NW: 
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northwest);
               wStr += g_pTheDB->GetMessageText(MID_NorthWest);
				break;
				
				case NE: 
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northeast);
               wStr += g_pTheDB->GetMessageText(MID_NorthEast);
				break;
				
				case SW:
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southwest);
               wStr += g_pTheDB->GetMessageText(MID_SouthWest);
				break;
				
				case SE:
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southeast);
               wStr += g_pTheDB->GetMessageText(MID_SouthEast);
				break;

				default: ASSERTP(false, "Unexpected chamber entry sequence."); break;
			}
         wStr += wszSpace;
         wStr += g_pTheDB->GetMessageText(MID_WinLevel16_15);
         wStr += wszCRLF;

			BEETHRO(wStr.c_str());

			//I'm not waiting for the demo to end--the flow of the dialogue
			//is more important here.
			this->eNextCommandWaitFor = WW_ScrollText;
		}
		break;

		case SC_Level16c:
		{
			//This will stop the demo from playing, but leave it on its last step.
			const UINT wEndTurnNo = this->pCurrentGame->wTurnNo + 1;
			if (wEndTurnNo < this->wDemoEndTurnNo) 
				this->wDemoEndTurnNo = wEndTurnNo;

			BEETHRO_DB(MID_WinLevel16_16);

			const UINT wFirstChamber = 
				(this->Room241Info.wNorthwestSequenceNo == 0) * NW +
				(this->Room241Info.wNortheastSequenceNo == 0) * NE +
				(this->Room241Info.wSouthwestSequenceNo == 0) * SW +
				(this->Room241Info.wSoutheastSequenceNo == 0) * SE;

			WSTRING wstrMessage = g_pTheDB->GetMessageText(MID_WinLevel16_17);
         wstrMessage += wszSpace;
			switch (wFirstChamber)
			{
				case NW:	wstrMessage += g_pTheDB->GetMessageText(MID_NorthWest); break;
				case NE: wstrMessage += g_pTheDB->GetMessageText(MID_NorthEast); break;
				case SW: wstrMessage += g_pTheDB->GetMessageText(MID_SouthWest); break;
				case SE: wstrMessage += g_pTheDB->GetMessageText(MID_SouthEast); break;
				default: break;   //might occur if demo is broken, but we have to allow for that...
			}
         wstrMessage += wszSpace;
			wstrMessage +=	g_pTheDB->GetMessageText(MID_WinLevel16_18);
         wstrMessage +=	wszCRLF;
			AUDIENCE(wstrMessage.c_str());

			BEETHRO_DB(MID_WinLevel16_19);

			//Demo has already ended.
			this->eNextCommandWaitFor = WW_ScrollText;
		}
		break;

		case SC_Level16d:
		{
			const UINT wFirstChamber = 
				(this->Room241Info.wNorthwestSequenceNo == 0) * NW +
				(this->Room241Info.wNortheastSequenceNo == 0) * NE +
				(this->Room241Info.wSouthwestSequenceNo == 0) * SW +
				(this->Room241Info.wSoutheastSequenceNo == 0) * SE;

			WSTRING wstrFirstChamber;
			switch (wFirstChamber)
			{
				case NW:
					wstrFirstChamber = g_pTheDB->GetMessageText(MID_NorthWest);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northwest);
				break;

				case NE:
					wstrFirstChamber = g_pTheDB->GetMessageText(MID_NorthEast);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northeast);
				break;

				case SW:
					wstrFirstChamber = g_pTheDB->GetMessageText(MID_SouthWest);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southwest);
				break;

				case SE:
					wstrFirstChamber = g_pTheDB->GetMessageText(MID_SouthEast);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southeast);
				break;

				default: break;   //might happen if demo is broken, so have to handle it...
			}
							
			WSTRING wstrMessage = g_pTheDB->GetMessageText(MID_WinLevel16_20);
         wstrMessage += wszSpace;
			wstrMessage +=	wstrFirstChamber;
         wstrMessage += wszSpace;
			wstrMessage +=	g_pTheDB->GetMessageText(MID_WinLevel16_21);

			BEETHRO(wstrMessage.c_str());

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		}
		break;

		case SC_Level16e:
		{
			const UINT wSecondChamber = 
				(this->Room241Info.wNorthwestSequenceNo == 1) * NW +
				(this->Room241Info.wNortheastSequenceNo == 1) * NE +
				(this->Room241Info.wSouthwestSequenceNo == 1) * SW +
				(this->Room241Info.wSoutheastSequenceNo == 1) * SE;

			WSTRING wstrSecondChamber;
			switch (wSecondChamber)
			{
				case NW:
					wstrSecondChamber = g_pTheDB->GetMessageText(MID_NorthWest);
					ShowDemoScene(this->Room241Info.dwDemoID, 
							this->Room241Info.Northwest);
				break;
				
				case NE:
					wstrSecondChamber = g_pTheDB->GetMessageText(MID_NorthEast);
					ShowDemoScene(this->Room241Info.dwDemoID, 
							this->Room241Info.Northeast);
				break;
				
				case SW:
					wstrSecondChamber = g_pTheDB->GetMessageText(MID_SouthWest);
					ShowDemoScene(this->Room241Info.dwDemoID, 
							this->Room241Info.Southwest);
				break;
				
				case SE:
					wstrSecondChamber = g_pTheDB->GetMessageText(MID_SouthEast);
					ShowDemoScene(this->Room241Info.dwDemoID, 
							this->Room241Info.Southeast);
				break;

				default: ASSERTP(false, "Unexpected chamber entry sequence (2)."); break;
			}

			WSTRING wstrMessage = g_pTheDB->GetMessageText(MID_WinLevel16_22);
         wstrMessage += wszSpace;
			wstrMessage += wstrSecondChamber;
         wstrMessage += wszSpace;
			wstrMessage +=	g_pTheDB->GetMessageText(MID_WinLevel16_23);
         wstrMessage += wszSpace;
			wstrMessage +=	wstrSecondChamber;
         wstrMessage += wszSpace;
			wstrMessage +=	g_pTheDB->GetMessageText(MID_WinLevel16_24);

			BEETHRO(wstrMessage.c_str());

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		}
		break;

		case SC_Level16f:
		{
			const UINT wThirdChamber = 
				(this->Room241Info.wNorthwestSequenceNo == 2) * NW +
				(this->Room241Info.wNortheastSequenceNo == 2) * NE +
				(this->Room241Info.wSouthwestSequenceNo == 2) * SW +
				(this->Room241Info.wSoutheastSequenceNo == 2) * SE;

			WSTRING wstrThirdChamber;
			switch (wThirdChamber)
			{
				case NW:
					wstrThirdChamber = g_pTheDB->GetMessageText(MID_NorthWest);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northwest);
				break;

				case NE:
					wstrThirdChamber = g_pTheDB->GetMessageText(MID_NorthEast);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northeast);
				break;

				case SW:
					wstrThirdChamber = g_pTheDB->GetMessageText(MID_SouthWest);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southwest);
				break;

				case SE:
					wstrThirdChamber = g_pTheDB->GetMessageText(MID_SouthEast);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southeast);
				break;

				default: ASSERTP(false, "Unexpected chamber entry sequence (3)."); break;
			}

			WSTRING wstrMessage = g_pTheDB->GetMessageText(MID_WinLevel16_25);
         wstrMessage += wszSpace;
			wstrMessage += wstrThirdChamber;
         wstrMessage += wszSpace;
			wstrMessage += g_pTheDB->GetMessageText(MID_WinLevel16_26);

			BEETHRO(wstrMessage.c_str());

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		}
		break;

		case SC_Level16g:
		{
			const UINT wLastChamber = 
				(this->Room241Info.wNorthwestSequenceNo == 3) * NW +
				(this->Room241Info.wNortheastSequenceNo == 3) * NE +
				(this->Room241Info.wSouthwestSequenceNo == 3) * SW +
				(this->Room241Info.wSoutheastSequenceNo == 3) * SE;

			WSTRING wstrLastChamber;
			WSTRING wstrLastMonster = g_pTheDB->GetMessageText(MID_WinLevel16_27);
			switch (wLastChamber)
			{
				case NW:
					wstrLastChamber = g_pTheDB->GetMessageText(MID_NorthWest);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northwest);
				break;

				case NE:
					wstrLastChamber = g_pTheDB->GetMessageText(MID_NorthEast);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Northeast);
				break;

				case SW:
					wstrLastChamber = g_pTheDB->GetMessageText(MID_SouthWest);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southwest);
				break;

				case SE:
					wstrLastChamber = g_pTheDB->GetMessageText(MID_SouthEast);
					wstrLastMonster = g_pTheDB->GetMessageText(MID_WinLevel16_28);
					ShowDemoScene(this->Room241Info.dwDemoID,
							this->Room241Info.Southeast);
				break;

				default: ASSERTP(false, "Unexpected chamber entry sequence (4)."); break;
			}

			WSTRING wstrMessage = g_pTheDB->GetMessageText(MID_WinLevel16_29);
         wstrMessage += wszSpace;
			wstrMessage += wstrLastChamber;
         wstrMessage += wszSpace;
			wstrMessage += g_pTheDB->GetMessageText(MID_WinLevel16_30);
         wstrMessage += wszSpace;
			wstrMessage += wstrLastMonster;
         wstrMessage += wszPeriod;
         wstrMessage += wszCRLF;
         wstrMessage += wszCRLF;
			wstrMessage += g_pTheDB->GetMessageText(MID_WinLevel16_31);

			BEETHRO(wstrMessage.c_str());

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		}
		break;

		//Level 17.
		case SC_Level17:
			if (!ShowDemoSceneForRoom(247L, 15000L)) //17:1N
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel17_1);
			AUDIENCE_DB(MID_WinLevel17_2);
			BEETHRO_DB(MID_WinLevel17_3);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 18.
		case SC_Level18:
			if (!ShowRoomStart(265L))	//18:1E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel18_1);
			AUDIENCE_DB(MID_WinLevel18_2);
			BEETHRO_DB(MID_WinLevel18_3);
			AUDIENCE_DB(MID_WinLevel18_4);
			BEETHRO_DB(MID_WinLevel18_5);

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_Level18b:
			if (!ShowDemoSceneForRoom(265L, 10000L)) //18:1E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel18_6);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 19.
		case SC_Level19:
			if (!ShowDemoSceneForRoom(278L, 15000L)) //19:2S2E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel19_1);
			BEFORE_NEW_SCREEN;

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 20.
		case SC_Level20:
			GoToScreen(SCR_WinAudience);
			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_Level20b:
			BEETHRO_DB(MID_WinLevel20_1);
			BEFORE_NEW_SCREEN;

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_Level20c:
			if (!ShowDemoSceneForRoom(290L, 30000L)) //20:Entrance.
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			GoToScreen(SCR_WinRoom);
			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_Level20d:
			BEETHRO_DB(MID_WinLevel20_2);
			AUDIENCE_DB(MID_WinLevel20_3);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level20e:
			if (!ShowDemoSceneForRoom(281L, 20000L)) //20:3N1E
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel20_4);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 21.
		case SC_Level21:
			if (!ShowDemoSceneForRoom(292L, 40000L)) //21:1N1W
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel21_1);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 22.
		case SC_Level22:
			if (!ShowDemoSceneForRoom(308L, 10000L)) //22:1W
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel22_1);
			AUDIENCE_DB(MID_WinLevel22_2);
			BEETHRO_DB(MID_WinLevel22_3);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 23.
		case SC_Level23:
			if (!ShowDemoSceneForRoom(322L, 30000L)) //23:1S1W
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel23_1);
			AUDIENCE_DB(MID_WinLevel23_2);
			BEETHRO_DB(MID_WinLevel23_3);
			AUDIENCE_DB(MID_WinLevel23_4);
			BEETHRO_DB(MID_WinLevel23_5);
			AUDIENCE_DB(MID_WinLevel23_6);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 24.
		case SC_Level24:
			if (!ShowDemoSceneForRoom(332L, 20000L)) //24:1W
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}

			BEETHRO_DB(MID_WinLevel24_1);
			AUDIENCE_DB(MID_WinLevel24_2);
			BEETHRO_DB(MID_WinLevel24_3);

			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//Level 25.
		case SC_Level25:
			if (!InitRoom344Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowRoomStart(344L)) ASSERTP(false, "Show start for room 344 failed.");

			BEETHRO_DB(MID_WinLevel25_1);

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_Level25b:
			if (!ShowDemoScene(this->Room344Info.dwDemoID, this->Room344Info.Scene))
				ASSERTP(false, "Show demo scene for room 344 failed.");

			BEETHRO_DB(MID_WinLevel25_2);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25c:
			if (!InitRoom341Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room341Info.dwDemoID, this->Room341Info.Scene))
				ASSERTP(false, "Show demo screen for room 341 failed.");

			BEETHRO_DB(MID_WinLevel25_3);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25d:
			if (!InitRoom340Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room340Info.dwDemoID, this->Room340Info.Scene))
				ASSERTP(false, "Show demo screen for room 340 failed.");

			BEETHRO_DB(MID_WinLevel25_4);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25e:
			if (!InitRoom339Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room339Info.dwDemoID, this->Room339Info.Scene))
				ASSERTP(false, "Show demo scene for room 339 failed.");

			BEETHRO_DB(MID_WinLevel25_5);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25f:
			if (!InitRoom342Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room342Info.dwDemoID, this->Room342Info.Scene))
				ASSERTP(false, "Show demo scene for room 342 failed.");

			BEETHRO_DB(MID_WinLevel25_6);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25g:
			if (!InitRoom345Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room345Info.dwDemoID, this->Room345Info.Scene))
				ASSERTP(false, "Show demo scene for room 345 failed.");

			BEETHRO_DB(MID_WinLevel25_7);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25h:
			if (!InitRoom348Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room348Info.dwDemoID, this->Room348Info.Scene))
				ASSERTP(false, "Show demo scene for room 348 failed.");

			BEETHRO_DB(MID_WinLevel25_8);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25i:
			if (!InitRoom349Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room349Info.dwDemoID, this->Room349Info.Scene))
				ASSERTP(false, "Show demo scene for room 349 failed.");

			BEETHRO_DB(MID_WinLevel25_9);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25j:
			if (!InitRoom350Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room350Info.dwDemoID, this->Room350Info.Scene))
				ASSERTP(false, "Show demo scene for room 350 failed.");

			BEETHRO_DB(MID_WinLevel25_10);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25k:
			if (!InitRoom347Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room347Info.dwDemoID, this->Room347Info.Scene))
				ASSERTP(false, "Show demo scene for room 347 failed.");

			BEETHRO_DB(MID_WinLevel25_11);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25l:
			if (!InitRoom346Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room346Info.dwDemoID, this->Room346Info.Scene))
				ASSERTP(false, "Show demo scene for room 346 failed.");

			BEETHRO_DB(MID_WinLevel25_12);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25m:
			if (!InitRoom343Info())
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			if (!ShowDemoScene(this->Room343Info.dwDemoID, 
					this->Room343Info.FirstStepToBeforeDeath))
				ASSERTP(false, "Show demo scene for room 343 failed.");

			BEETHRO_DB(MID_WinLevel25_13);
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_Level25n:
			if (!ShowDemoScene(this->Room343Info.dwDemoID, 
					this->Room343Info.NeatherDies))
				ASSERTP(false, "Show demo scene for room 343 failed.");
			
			BEETHRO_DB(MID_WinLevel25_14);
			BEFORE_NEW_SCREEN;
						
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		case SC_NeatherSpeech:
			ShowPic("LastWords", 17, 264, 170, 207);
			GoToScreen(SCR_WinPic);

			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_NeatherSpeechB:
			BEETHRO_DB(MID_WinNeatherSpeech);
			BEFORE_NEW_SCREEN;

			this->eNextCommandWaitFor = WW_ScrollText;
		break;
		
		case SC_NeatherSpeechC:
			if (!InitRoom343Info() || //Have to reinit because I left screen.
					!ShowDemoScene(this->Room343Info.dwDemoID, 
					this->Room343Info.AfterDeath))
				ASSERTP(false, "Show demo scene for room 343 failed.");

			GoToScreen(SCR_WinRoom);

			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_NeatherSpeechD:
			BEETHRO_DB(MID_WinEnd1);
			AUDIENCE_DB(MID_WinEnd2);
			BEETHRO_DB(MID_WinEnd3);
			AUDIENCE_DB(MID_WinEnd4);
			BEETHRO_DB(MID_WinEnd5);
			BEFORE_NEW_SCREEN;
			
			this->eNextCommandWaitFor = WW_ScrollTextAndDemoEnd;
		break;

		//The "end".
		case SC_TheEnd:
			ShowPic("TheEnd", 438, 12, 190, 234);
			GoToScreen(SCR_WinPic);

			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_TheEndB:
			BEETHRO_DB(MID_WinEnd6);
			AUDIENCE_DB(MID_WinEnd7);
			BEETHRO_DB(MID_WinEnd8);
         BEETHRO(wszSpace);
         BEETHRO(wszSpace);
			AUDIENCE_DB(MID_WinEnd9);
			BEETHRO_DB(MID_WinEnd10);
         BEETHRO(wszSpace);
         BEETHRO(wszSpace);
			AUDIENCE_DB(MID_WinEnd11);
			BEETHRO_DB(MID_WinEnd12);
			AUDIENCE_DB(MID_WinEnd13);
			BEETHRO_DB(MID_WinEnd14);
			BEFORE_NEW_SCREEN;

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		//The door.
		case SC_TheDoor:
			if (!ShowRoomStart(146L))	//10:3N
			{
				this->eCommand = SC_Error; //Error--finish story early.
				this->eNextCommandWaitFor = WW_Nothing;
				return;
			}
			GoToScreen(SCR_WinRoom);

			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_TheDoorB:
			BEETHRO_DB(MID_WinTheDoor1);

			AUDIENCE_DB(MID_WinTheDoor2);
			BEFORE_NEW_SCREEN;
			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_TheDoorC:
			ShowPic("dd1", 11, 12, 186, 124);
			GoToScreen(SCR_WinPic);
			this->eNextCommandWaitFor = WW_Nothing;
		break;		

		case SC_TheDoorD:
			BEETHRO_DB(MID_WinTheDoor3);

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_TheDoorE:
			ShowPic("dd2");
			BEETHRO_DB(MID_WinTheDoor4);

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_TheDoorF:
			ShowPic("dd3");
			BEETHRO_DB(MID_WinTheDoor5);
         BEETHRO(wszSpace);
         BEETHRO(wszSpace);
			
			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_TheDoorG:
			ShowPic("dd4");
			BEETHRO_DB(MID_WinTheDoor6);
			AUDIENCE_DB(MID_WinTheDoor7);
			this->eNextCommandWaitFor = WW_ScrollText;
		break;
		
		case SC_TheDoorH:
			ShowPic("dd5");
			BEETHRO_DB(MID_WinTheDoor8);
			AUDIENCE_DB(MID_WinTheDoor9);
			BEETHRO_DB(MID_WinTheDoor10);
			AUDIENCE_DB(MID_WinTheDoor11);
			BEETHRO_DB(MID_WinTheDoor12);
			BEFORE_NEW_SCREEN;
			
			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_TheDoorI:
			GoToScreen(SCR_WinAudience);
			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_TheDoorJ:
			BEETHRO_DB(MID_WinTheDoor13);
			AUDIENCE_DB(MID_WinTheDoor14);
			BEETHRO_DB(MID_WinTheDoor15);
			AUDIENCE_DB(MID_WinTheDoor16);
			BEETHRO_DB(MID_WinTheDoor17);
			AUDIENCE_DB(MID_WinTheDoor18);
			BEFORE_NEW_SCREEN;
			BEFORE_NEW_SCREEN;
			BEFORE_NEW_SCREEN;

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_EndOfScript: //Last command.
			//Win sequence is finished.
			ResetScript();
			GoToScreen(SCR_Credits);
		return;

		//If an error occurred in one of the operations, I have Beethro finish
		//the story early.
		case SC_Error:
			GoToScreen(SCR_WinAudience);
			this->eNextCommandWaitFor = WW_Nothing;
		break;

		case SC_ErrorB:
			BEETHRO_DB(MID_WinError1);
			AUDIENCE_DB(MID_WinError2);
			BEETHRO_DB(MID_WinError3);
			AUDIENCE_DB(MID_WinError4);
			BEETHRO_DB(MID_WinError5);
			AUDIENCE_DB(MID_WinError6);
			BEETHRO_DB(MID_WinError7);
			AUDIENCE_DB(MID_WinError8);
			BEETHRO_DB(MID_WinError9);
         BEFORE_NEW_SCREEN;

			this->pScrollingText->AddText(
               g_pTheDB->GetMessageText(MID_WinFailedToRetrieveDemo), F_Sign);

			this->eNextCommandWaitFor = WW_ScrollText;
		break;

		case SC_ErrorC:
			//Win sequence is finished.
			ResetScript();
			GoToScreen(SCR_Credits);
		return;

		default:
			ASSERTP(false, "Unexpected command#."); break;
	}

	//Increment the command by 1.  The casting and assignment business is to
	//keep the compiler from complaining.
	{
		int nCommand = static_cast<int>(this->eCommand);
		this->eCommand = static_cast<SCRIPTCOMMAND>(++nCommand);
	}

#	undef BEFORE_NEW_SCREEN
#	undef AUDIENCE
#	undef BEETHRO
#	undef AUDIENCE_DB
#	undef BEETHRO_DB
}

//Add new methods before the previous method, not here.

// $Log: WinScreen.cpp,v $
// Revision 1.46  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.45  2003/08/19 04:38:04  mrimer
// Removed assertions for broken demos that we have no power over.
//
// Revision 1.44  2003/07/31 23:22:52  erikh2000
// Added some more robustness to the scene selection code to deal with demos that are no longer compatible.
//
// Revision 1.43  2003/07/22 19:00:28  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.42  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.41  2003/07/02 15:30:17  mrimer
// Relaxed constraints for InitRoom109Info() so that end sequence won't exit if an older (incompatible) demo doesn't play the room (kill the serpent) successfully.
//
// Revision 1.40  2003/07/01 20:24:55  mrimer
// Moved all end of game text into DB.
//
// Revision 1.39  2003/06/26 11:44:15  schik
// VS.NET compatibility fixes
//
// Revision 1.38  2003/06/20 00:42:49  mrimer
// Added more robust handling of broken saved games.
//
// Revision 1.37  2003/06/18 04:05:56  mrimer
// Fixed bug: end game sequence freezes after showing level 13.
//
// Revision 1.36  2003/06/15 00:16:06  mrimer
// Reinserted compiled out end script.  Updated strings to work with Unicode fonts.
//
// Revision 1.35  2003/06/06 18:11:29  mrimer
// Revised frame rate delay.
//
// Revision 1.34  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.33  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.32  2003/05/19 20:29:28  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.31  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.30  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.29  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.28  2003/02/16 20:32:20  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.27  2003/01/09 22:41:32  erikh2000
// Added an #include.
//
// Revision 1.26  2002/12/22 02:28:28  mrimer
// Revised swordsman vars.
//
// Revision 1.25  2002/11/18 17:55:43  mrimer
// Removed frame rate delay to maximize animation speed.
//
// Revision 1.24  2002/11/15 02:54:44  mrimer
// Fixed win sequence logic and dialog in a few places.
//
// Revision 1.23  2002/10/22 22:38:40  erikh2000
// Fixed a bug with the level 10 exit room demo not playing.
// Added more lines after text that comes before a screen change.
// Made the room info routines more robust.
// Fixed a bug that was leaving demo playback in a paused state.
//
// Revision 1.22  2002/10/21 20:30:00  mrimer
// Added OnMouseDown(), PauseAnimation(), UnpauseAnimation().
// Now pause scrolling while a demo is being loaded and prepared.
// Fixed dialog error.
// Made some vars const.
//
// Revision 1.21  2002/10/21 09:35:02  erikh2000
// Fixed "the the" error.
//
// Revision 1.20  2002/10/17 17:23:13  mrimer
// Fixed a typo.
//
// Revision 1.19  2002/10/10 01:02:44  erikh2000
// Code to start music playing has moved to CWinStartScreen.
//
// Revision 1.18  2002/10/09 22:16:40  erikh2000
// Finished dialogue for script commands.
//
// Revision 1.17  2002/10/09 00:57:48  erikh2000
// Finished script command logic.  Dialogue remains to be finished.
//
// Revision 1.16  2002/10/04 17:53:41  mrimer
// Added pause and speed control to this screen.
// Added mouse cursor display logic.
//
// Revision 1.15  2002/10/03 02:49:14  erikh2000
// Added a comment.
//
// Revision 1.14  2002/10/01 22:57:49  erikh2000
// Added a lot more script commands.
// Wrote a method to encapsulate some of the common code for getting a highlight room demo's current game.
// Wrote a method to show a picture on the new SCR_WinPic screen.
//
// Revision 1.13  2002/09/06 20:12:59  erikh2000
// Added more commands to script.
// Wrote method to show start of room.
//
// Revision 1.12  2002/09/05 18:28:17  erikh2000
// Added method to determine if player has visited a room.
// Scroll text wait criteria now waits for all text to be shown, instead of all text to scroll off the widget.
// Added more of the script to DoNextCommand().
//
// Revision 1.11  2002/09/04 22:33:03  erikh2000
// Added code for playing demos.
//
// Revision 1.10  2002/09/03 21:43:33  erikh2000
// Got the first part of the script working--Beethro talks, audience talks, text scrolls, the screen switches from audience to room view, a room is shown in the room widget.
//
// Revision 1.9  2002/08/23 23:57:51  erikh2000
// Renamed a constant.
//
// Revision 1.8  2002/08/01 17:27:13  mrimer
// Revised some screen transition and music setting logic.
//
// Revision 1.7  2002/07/29 17:14:17  mrimer
// Added SetForActivate().
//
// Revision 1.6  2002/07/26 19:18:54  mrimer
// Added transition to Credits screen when finished.
//
// Revision 1.5  2002/07/11 20:59:48  mrimer
// Implemented base class animation/event handling.
//
// Revision 1.4  2002/07/10 04:16:20  erikh2000
// Minimal fix to get past a compile error.
//
// Revision 1.3  2002/04/10 00:29:41  erikh2000
// Removed unnecessary #includes.
//
// Revision 1.2  2002/04/09 10:05:41  erikh2000
// Fixed revision log macro.
//
