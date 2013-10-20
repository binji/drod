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

#include "WinRoomScreen.h"
#include "DrodSound.h"
#include "RoomWidget.h"
#include "TarStabEffect.h"
#include "BloodEffect.h"
#include "TrapdoorFallEffect.h"
#include "CheckpointEffect.h"
#include "DebrisEffect.h"
#include <FrontEndLib/BumpObstacleEffect.h>

#include "../DRODLib/MonsterFactory.h"
#include "../DRODLib/Db.h"
#include <BackEndLib/Assert.h>

#include <SDL.h>

#include <algorithm>
#if defined _MSC_VER
#	define	Min	_MIN
#	define	Max	_MAX
#else
#	define	Min	std::min
#	define	Max	std::max
#endif

//Graphic image locations/dimensions.
const UINT CX_HEAD = 82;
const UINT CY_HEAD = 146;
const UINT X_HEADDEST = 543;
const UINT Y_HEADDEST = 0;
const UINT X_OPENHEADSRC = 1;
const UINT Y_OPENHEADSRC = 17;
const UINT X_CLOSEDHEADSRC = 84;
const UINT Y_CLOSEDHEADSRC = 17;
SDL_Rect HeadDest = {X_HEADDEST, Y_HEADDEST, CX_HEAD, CY_HEAD};
SDL_Rect OpenHeadSrc = {X_OPENHEADSRC, Y_OPENHEADSRC, CX_HEAD, CY_HEAD};
SDL_Rect ClosedHeadSrc = {X_CLOSEDHEADSRC, Y_CLOSEDHEADSRC, CX_HEAD, CY_HEAD};

const UINT X_ROOM = 0;
const UINT Y_ROOM = 16;
SDL_Rect RoomDest = {X_ROOM, Y_ROOM, CDrodBitmapManager::CX_ROOM,
		CDrodBitmapManager::CY_ROOM};

const UINT CX_PUPIL = 4;
const UINT CY_PUPIL = 4;
const UINT X_PUPILSRC = 167;
const UINT Y_PUPILSRC = 17;
SDL_Rect PupilSrc = {X_PUPILSRC, Y_PUPILSRC, CX_PUPIL, CY_PUPIL};

const UINT CX_EYESRC = 9;
const UINT CY_EYESRC = 3;
const UINT X_EYESRC = 172;
const UINT Y_EYESRC = 17;
const UINT X_LEFTEYEDEST = 555;
const UINT Y_LEFTEYEDEST = 76;
const UINT X_RIGHTEYEDEST = 580;
const UINT Y_RIGHTEYEDEST = 77;
SDL_Rect EyeSrc = {X_EYESRC, Y_EYESRC, CX_EYESRC, CY_EYESRC};
SDL_Rect LeftEyeDest = {X_LEFTEYEDEST, Y_LEFTEYEDEST, CX_EYESRC, CY_EYESRC};
SDL_Rect RightEyeDest = {X_RIGHTEYEDEST, Y_RIGHTEYEDEST, CX_EYESRC, CY_EYESRC};
SDL_Rect LeftPupilDest = {X_LEFTEYEDEST, Y_LEFTEYEDEST+CY_EYESRC/2, CX_PUPIL, CY_PUPIL};
SDL_Rect RightPupilDest = {X_RIGHTEYEDEST, Y_RIGHTEYEDEST+CY_EYESRC/2, CX_PUPIL, CY_PUPIL};

//
//Protected methods.
//

//******************************************************************************
void CWinRoomScreen::OnBetweenEvents()
{
	CWinScreen::OnBetweenEvents();

	if (!this->pCurrentCommand) return; //End of demo or none loaded.

	//Handle case when only move is to exit room.
	if (this->wDemoEndTurnNo == 0)
	{
		this->pCurrentGame->wTurnNo = 1;
		this->pCurrentCommand = NULL;
		return;
	}

	//Process next command if it's time.
	const Uint32 dwNow = SDL_GetTicks();
	if (dwNow >= this->dwNextCommandTime)
	{
		ProcessCommand(this->pCurrentCommand->bytCommand);

		//Check for last turn in demo.
		if (this->pCurrentGame->wTurnNo - 1 == this->wDemoEndTurnNo)
			this->pCurrentCommand = NULL;
		else	//Get next turn.
		{
			this->pCurrentCommand = this->pCurrentGame->Commands.GetNext();			
			if (this->pCurrentCommand)
				this->dwNextCommandTime = dwNow +
						(DWORD)(10.0 * this->pCurrentCommand->byt10msElapsedSinceLast
						/ this->fScrollRateMultiplier);	//synch demo with text scroll speed
		}
	}
}

//*****************************************************************************
void CWinRoomScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool bUpdateRect)	//(in)	If true (default) and destination
							//		surface is the screen, the screen
							//		will be immediately updated in
							//		the widget's rect.
{
	PaintBackground();

	if (this->pRoomWidget->GetCurrentGame())
		this->pRoomWidget->ResetForPaint();
	PaintChildren();

	if (IsBeethroTalking())
		DrawBeethroTalking();
	else
		DrawBeethroSilent();
	
	if (bUpdateRect) UpdateRect();
}

//******************************************************************************
bool CWinRoomScreen::Load()
//Load resources for the screen.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	ASSERT(!this->pWinSurface);

	this->pWinSurface = g_pTheBM->GetBitmapSurface("RoomView");
	if (!this->pWinSurface) return false;

	this->pRoomWidget = new CRoomWidget(0L, X_ROOM, Y_ROOM,
			CDrodBitmapManager::CX_ROOM, CDrodBitmapManager::CY_ROOM, NULL);
	AddWidget(this->pRoomWidget);

	this->pScrollingText = new CScrollingTextWidget(0L, 534, 159, 104, 319);
	this->pScrollingText->SetBackground(this->pWinSurface);
	this->pScrollingText->SetScrollRate(20);
	AddWidget(this->pScrollingText);

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//******************************************************************************
void CWinRoomScreen::Unload()
//Unload resources for the screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pWinSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("RoomView");
		this->pWinSurface = NULL;
	}

	this->bIsLoaded = false;
}

//******************************************************************************
bool CWinRoomScreen::LoadDemoScene(
//Load a scene from a demo in the room widget.
//
//Params:
	const DWORD dwDemoID,	//(in)	Demo to show.
	CDemoScene &Scene)		//(in)	Scene to show.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwDemoID);
	ASSERT(Scene.wBeginTurnNo <= Scene.wEndTurnNo);

	//Load the demo.
	CDbDemo *pDemo = g_pTheDB->Demos.GetByID(dwDemoID);
	if (!pDemo) {ASSERTP(false, "Failed to retrieve demo."); return false;}
	ASSERT(pDemo->wBeginTurnNo <= Scene.wBeginTurnNo);
	ASSERT(pDemo->wEndTurnNo >= Scene.wEndTurnNo);
	
	//Free an already-loaded game.
	if (this->pCurrentGame)
	{
		delete this->pCurrentGame;
		this->pCurrentGame = NULL;
	}

	//Load the game.
	CCueEvents Ignored;
	this->pCurrentGame = g_pTheDB->GetSavedCurrentGame(
			pDemo->dwSavedGameID, Ignored, true);
	if (!this->pCurrentGame) {ASSERTP(false, "Failed to retrieve current game."); delete pDemo; return false;}
	this->pCurrentGame->SetAutoSaveOptions(ASO_NONE); //No auto-saving for playback.
	this->pCurrentGame->SetTurn(Scene.wBeginTurnNo, Ignored);
	
	//Get the first command.
	this->pCurrentCommand = this->pCurrentGame->Commands.Get(Scene.wBeginTurnNo);
	ASSERT(this->pCurrentCommand);
	delete pDemo;

	//Update the room widget with new current game.
	if (!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		{ASSERTP(false, "Failed to load room from current game."); return false;}

	return true;
}

//******************************************************************************
bool CWinRoomScreen::LoadRoomStart(
//Load a room start saved game so it shows in the room widget.
//
//Params:
	const DWORD dwRoomID)		//(in)	Room to show.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwRoomID);

	//Find room start saved game for room.
	DWORD dwRoomSavedGameID = 0L;
	{
		CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(dwRoomID);
		if (!pRoom) return false;

		CDbSavedGame *pSavedGame = pRoom->SavedGames.GetFirst();
		while (pSavedGame)
		{
			if (pSavedGame->eType == ST_RoomBegin) 
			{
				dwRoomSavedGameID = pSavedGame->dwSavedGameID;
				delete pSavedGame;
				break;
			}
			delete pSavedGame;
			pSavedGame = pRoom->SavedGames.GetNext();
		}
		delete pRoom;
	}
	if (!dwRoomSavedGameID)
		{ASSERTP(false, "Bad saved game."); return false;}

	//Free an already-loaded game.
	delete this->pCurrentGame;
	this->pCurrentGame = NULL;

	//Load the game.
	CCueEvents Ignored;
	this->pCurrentGame = g_pTheDB->GetSavedCurrentGame(dwRoomSavedGameID, Ignored, true);
	if (!this->pCurrentGame) {ASSERTP(false, "Failed to retrieve saved current game."); return false;}
	this->pCurrentGame->SetAutoSaveOptions(ASO_NONE); //No auto-saving for playback.
		
	//Set current command to NULL to avoid any demo playback behaviour.
	this->pCurrentCommand = NULL;
	
	//Update the room widget with new current game.
	if (!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		{ASSERTP(false, "Failed to load from current game."); return false;}

	return true;
}

//
//Private methods.
//

//*****************************************************************************
void CWinRoomScreen::ProcessCommand(int nCommand)
//Processes game command, making calls to update game data and respond to cue 
//events.
{
	CCueEvents CueEvents;
	ASSERT(nCommand != CMD_RESTART);
	
	//Send command to current game and get cue events list back.
	ASSERT(this->pCurrentGame->bIsGameActive); //We should have reloaded a game before getting here.
	this->pCurrentGame->ProcessCommand(nCommand, CueEvents);
	
	ASSERT(!CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied));

	ProcessCueEventsBeforeRoomDraw(CueEvents);
	
	//Redraw the room.
	this->pRoomWidget->Paint();
}

//***************************************************************************************
void CWinRoomScreen::ProcessCueEventsBeforeRoomDraw(
//This is a copy-and-paste job from CGameScreen::ProcessCueEventsBeforeRoomDraw().
//See revision history of that routine for record of contributions.  The general
//differences are described below in case someone needs to do another copy-and-paste.
//Yes, the idea occurred to write a routine shared between the two screens, but
//all things considered, it is probably not worth the effort.
//
//Differences:
//1. Leaving room and dieing are unexpected events that fire assertians.
//2. Screen to go to next is not specified in return value.
//3. CMapWidget and sign updating code removed.
//
//Params:
	CCueEvents &CueEvents) //(in)
{
	ASSERT(!CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom));

	//Channel n+1 -- Swordsman's voice.
	if (CueEvents.HasOccurred(CID_HitObstacle))
	{
		g_pTheSound->PlaySoundEffect(SEID_OOF);
		CMoveCoord *pMoveCoord = DYN_CAST(CMoveCoord*, CAttachableObject*,
				CueEvents.GetFirstPrivateData(CID_HitObstacle));
		this->pRoomWidget->RemoveTLayerEffectsOfType(EBUMPOBSTACLE);
		this->pRoomWidget->AddTLayerEffect(new CBumpObstacleEffect(this->pRoomWidget, 
				pMoveCoord->wCol, pMoveCoord->wRow, pMoveCoord->wO));
	}
	else if (CueEvents.HasOccurred(CID_Scared))
		g_pTheSound->PlaySoundEffect(SEID_SCARED);
	else if (CueEvents.HasOccurred(CID_SwordsmanTired))
		g_pTheSound->PlaySoundEffect(SEID_TIRED);
	else if (CueEvents.HasOccurred(CID_AllMonstersKilled) &&
		!CueEvents.HasOccurred(CID_NeatherExitsRoom)) //Beethro isn't happy about the 'Neather getting away.
	{
		g_pTheSound->PlaySoundEffect(SEID_CLEAR);
	}

	//Channel n+2 -- 'Neather's voice.
	if (CueEvents.HasOccurred(CID_NeatherScared))
		g_pTheSound->PlaySoundEffect(SEID_NSCARED);
	else if (CueEvents.HasOccurred(CID_NeatherFrustrated))
		g_pTheSound->PlaySoundEffect(SEID_NFRUSTRATED);
   else if (CueEvents.HasOccurred(CID_NeatherLaughing))
		g_pTheSound->PlaySoundEffect(SEID_NLAUGHING);

	//Channel n+3.
	if (CueEvents.HasOccurred(CID_MonsterDiedFromStab))
	{
		bool bLastBrain=false;
		g_pTheSound->PlaySoundEffect(SEID_SPLAT);
		CMonster *pMonster = DYN_CAST(CMonster*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab));
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
			pMonster = DYN_CAST(CMonster*, CAttachableObject*, CueEvents.GetNextPrivateData());
		}
		if (bLastBrain) g_pTheSound->PlaySoundEffect(SEID_LASTBRAIN);
	}
	if (CueEvents.HasOccurred(CID_TarDestroyed))
	{
		g_pTheSound->PlaySoundEffect(SEID_STABTAR);
		CMoveCoord *pCoord = DYN_CAST(CMoveCoord*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_TarDestroyed));
		while (pCoord)
		{
			this->pRoomWidget->AddTLayerEffect(
					new CTarStabEffect(this->pRoomWidget, *pCoord));
			pCoord = DYN_CAST(CMoveCoord*, CAttachableObject*, CueEvents.GetNextPrivateData());
		}
	}
	if (CueEvents.HasOccurred(CID_SnakeDiedFromTruncation))
	{
		g_pTheSound->PlaySoundEffect(SEID_SPLAT);
		CMonster *pMonster = DYN_CAST(CMonster*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_SnakeDiedFromTruncation));
		while (pMonster)
		{
			CMoveCoord coord(pMonster->wX,pMonster->wY,pMonster->wO);
			this->pRoomWidget->AddTLayerEffect(
					new CBloodEffect(this->pRoomWidget, coord));
			pMonster = DYN_CAST(CMonster*, CAttachableObject*, CueEvents.GetNextPrivateData());
		}
	}

	//Channel n+4.
	if (CueEvents.HasOccurred(CID_OrbActivated))
	{
		g_pTheSound->PlaySoundEffect(SEID_ORBHIT);
		COrbData *pOrbData = DYN_CAST(COrbData*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_OrbActivated));
		while (pOrbData)
		{
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData);
			pOrbData = DYN_CAST(COrbData*, CAttachableObject*, CueEvents.GetNextPrivateData());
		}
	}
	else if (CueEvents.HasOccurred(CID_OrbActivatedByMimic))
	{
		g_pTheSound->PlaySoundEffect(SEID_ORBHITMIMIC);
		COrbData *pOrbData = DYN_CAST(COrbData*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_OrbActivatedByMimic));
		while (pOrbData)
		{
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData);
			pOrbData = DYN_CAST(COrbData*, CAttachableObject*, CueEvents.GetNextPrivateData());
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

	if (CueEvents.HasOccurred(CID_TrapDoorRemoved))
	{
		g_pTheSound->PlaySoundEffect(SEID_TRAPDOOR);
		CCoord *pCoord = DYN_CAST(CCoord*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_TrapDoorRemoved) );
		while (pCoord)
		{
			this->pRoomWidget->AddTLayerEffect(
					new CTrapdoorFallEffect(this->pRoomWidget, *pCoord));
			pCoord = DYN_CAST(CCoord*, CAttachableObject*, CueEvents.GetNextPrivateData());
		}
	}
	if (CueEvents.HasOccurred(CID_CheckpointActivated))
	{
		CCoord *pCoord = DYN_CAST(CCoord*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_CheckpointActivated));
		this->pRoomWidget->AddTLayerEffect(
				new CCheckpointEffect(this->pRoomWidget, *pCoord));
		g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);
	}
	if (CueEvents.HasOccurred(CID_RedDoorsOpened) || CueEvents.HasOccurred(CID_GreenDoorsOpened))
		g_pTheSound->PlaySoundEffect(SEID_DOOROPEN);
	if (CueEvents.HasOccurred(CID_CrumblyWallDestroyed))
	{
		g_pTheSound->PlaySoundEffect(SEID_BREAKWALL);
		CMoveCoord *pCoord = DYN_CAST(CMoveCoord*, CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_CrumblyWallDestroyed));
		while (pCoord)
		{
			this->pRoomWidget->AddTLayerEffect(
					new CDebrisEffect(this->pRoomWidget, *pCoord));
			pCoord = DYN_CAST(CMoveCoord*, CAttachableObject*, CueEvents.GetNextPrivateData());
		}
	}

	ASSERT(!CueEvents.HasOccurred(CID_ExitLevelPending));

	//Do an update of tile image arrays.
	if (CueEvents.HasOccurred(CID_Plots))	//Plots occurred.
		this->pRoomWidget->UpdateFromPlots();
}

//******************************************************************************
void CWinRoomScreen::DrawBeethroTalking()
//Mouth moves, eyes wander
{
	SDL_Surface *pDestSurface = GetDestSurface();
	//Draw talking mouth.
	const Uint32 curTime = SDL_GetTicks();
	if (curTime - this->lLastMouthMove > 300)
	{
		if (!this->bPaused)
			this->bMouthOpen = !this->bMouthOpen;
		this->lLastMouthMove = curTime;
		SDL_BlitSurface(this->pWinSurface, (this->bMouthOpen ? &OpenHeadSrc
				: &ClosedHeadSrc), pDestSurface, &HeadDest);
		UpdateRect(HeadDest);
	}

	DrawEyes();
	DrawRovingPupils();
}

//******************************************************************************
void CWinRoomScreen::DrawBeethroSilent()
//Mouth not moving, eyes fixed
{
	SDL_BlitSurface(this->pWinSurface, &ClosedHeadSrc, GetDestSurface(),
			&HeadDest);
	UpdateRect(HeadDest);

	//Draw eyes and stationary pupils.
	DrawEyes();
	ResetPupils();
	DrawPupils();
}

//******************************************************************************
void CWinRoomScreen::DrawEyes()
//Blit Beethro's eyes.
{
	SDL_Surface *pDestSurface = GetDestSurface();
	SDL_BlitSurface(this->pWinSurface, &EyeSrc, pDestSurface, &LeftEyeDest);
	SDL_BlitSurface(this->pWinSurface, &EyeSrc, pDestSurface, &RightEyeDest);
}

//******************************************************************************
void CWinRoomScreen::DrawPupils()
//Draw pupils on Beethro's eyes at current position.
{
	SDL_Surface *pDestSurface = GetDestSurface();

	SDL_SetClipRect(pDestSurface,&LeftEyeDest);
	SDL_BlitSurface(this->pWinSurface, &PupilSrc, pDestSurface, &CurLeftPupilDest);
	UpdateRect(LeftEyeDest);

   SDL_SetClipRect(pDestSurface,&RightEyeDest);
	SDL_BlitSurface(this->pWinSurface, &PupilSrc, pDestSurface, &CurRightPupilDest);
	UpdateRect(RightEyeDest);

   SDL_SetClipRect(pDestSurface,NULL);
}

//******************************************************************************
void CWinRoomScreen::DrawRovingPupils()
//Randomly roving eyes.
{
	//Move eyes every 500ms.
	const Uint32 curTime = SDL_GetTicks();
	if (curTime - this->lLastPupilMove > 500 && !this->bPaused)
	{
		this->lLastPupilMove = curTime;

		const int xRand = (((int)rand()%3)-1);
		const int yRand = (((int)rand()%3)-1);
		CurLeftPupilDest.x += xRand;
		CurLeftPupilDest.y += yRand;
		CurRightPupilDest.x += xRand;
		CurRightPupilDest.y += yRand;
		//Bounds checking.  Let eyes go past bounds by a pixel or two.
		{
			//Left eye.
			const Sint16 xLeft=LeftEyeDest.x-1;
			const Sint16 xRight=LeftEyeDest.x+LeftEyeDest.w-2;
			CurLeftPupilDest.x = min(max(CurLeftPupilDest.x,xLeft),xRight);
			const Sint16 xTop=LeftEyeDest.y-1;
			const Sint16 xBot=LeftEyeDest.y+LeftEyeDest.h-2;
			CurLeftPupilDest.y = min(max(CurLeftPupilDest.y,xTop),xBot);
		}
		{
			//Right eye.
			const Sint16 xLeft=RightEyeDest.x-1;
			const Sint16 xRight=RightEyeDest.x+RightEyeDest.w-2;
			CurRightPupilDest.x = min(max(CurRightPupilDest.x,xLeft),xRight);
			const Sint16 xTop=RightEyeDest.y-1;
			const Sint16 xBot=RightEyeDest.y+RightEyeDest.h-2;
			CurRightPupilDest.y = min(max(CurRightPupilDest.y,xTop),xBot);
		}
	}

	DrawPupils();
}

//******************************************************************************
void CWinRoomScreen::ResetPupils()
//Put pupils in position looking at audience.
{
	CurLeftPupilDest = LeftPupilDest;
	CurRightPupilDest = RightPupilDest;
}

// $Log: WinRoomScreen.cpp,v $
// Revision 1.25  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.24  2003/07/22 19:00:28  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.23  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.22  2003/07/01 20:26:48  mrimer
// Made repainting more efficient to reduce flicker.  Revised some code.
//
// Revision 1.21  2003/06/06 18:12:40  mrimer
// Removed some unneeded assertions.
//
// Revision 1.20  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.19  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.18  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.17  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.16  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.15  2003/01/09 22:41:28  erikh2000
// Added an #include.
//
// Revision 1.14  2002/11/15 02:57:02  mrimer
// Revised some code.  Added handling of CID_OrbActivatedByMimic cue event.  Removed plot history stuff.
//
// Revision 1.13  2002/10/21 21:07:48  mrimer
// Fixed assertion on demo playback when there are no moves in a demo.
//
// Revision 1.12  2002/10/09 00:56:56  erikh2000
// Removed commented out code.
//
// Revision 1.11  2002/10/04 17:57:12  mrimer
// Added pause control to animation.
//
// Revision 1.10  2002/10/01 22:55:27  erikh2000
// Fixed a problem with played back current games resaving saved games.
//
// Revision 1.9  2002/10/01 16:28:04  mrimer
// Enhanced mimic placement effect.
//
// Revision 1.8  2002/09/14 21:41:31  mrimer
// Added code to remove duplicate CBumpObstacleEffect's.
//
// Revision 1.7  2002/09/06 20:09:22  erikh2000
// Added method to show room start in the room widget.
//
// Revision 1.6  2002/09/05 18:26:08  erikh2000
// Fixed a bug where most interesting scene selection was showing a sequence that began too early.
// Slowed down text scroll speed.
//
// Revision 1.5  2002/09/04 22:31:57  erikh2000
// Added code to play demos.
//
// Revision 1.4  2002/09/04 21:47:56  mrimer
// Moved Animate() code into base win screen class.
//
// Revision 1.3  2002/09/03 23:51:55  mrimer
// Sped up screen animation by removing unnecessary UpdateRect's, etc.
//
// Revision 1.2  2002/09/03 21:43:33  erikh2000
// Got the first part of the script working--Beethro talks, audience talks, text scrolls, the screen switches from audience to room view, a room is shown in the room widget.
//
// Revision 1.1  2002/07/11 20:58:37  mrimer
// Initial check-in.
//
