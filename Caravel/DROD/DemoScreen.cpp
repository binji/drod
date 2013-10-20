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

#include "DemoScreen.h"
#include "DrodScreenManager.h"
#include "MapWidget.h"
#include "RoomWidget.h"
#include "../DRODLib/CueEvents.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbCommands.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>

const DWORD FIRST_COMMAND_DELAY = 500;
const DWORD LAST_COMMAND_DELAY = 500;
const DWORD ANIMATE_DELAY = 33;

float				CDemoScreen::fScrollRateMultiplier = 1.0;
UINT				CDemoScreen::wNormalScrollRate = 33L;

//
//CDemoScreen public methods.
//

//*****************************************************************************
bool CDemoScreen::LoadDemoGame(
//Loads current game and playback info from a demo.
//
//Params:
	const DWORD dwDemoID)	//(in)	Demo to load.
//
//Returns:
//True if successful, false if not.
{
	//Load the demo.
	delete this->pDemo;
	this->pDemo = g_pTheDB->Demos.GetByID(dwDemoID);
	if (!this->pDemo) return false;

	//Load demo's saved game for CGameScreen.
	if (!CGameScreen::LoadSavedGame(this->pDemo->dwSavedGameID, true)) return false;
	CGameScreen::pCurrentGame->SetAutoSaveOptions(ASO_NONE);

	//Rewind current game to beginning of demo.
	CCueEvents Ignored;
	CGameScreen::pCurrentGame->SetTurn(this->pDemo->wBeginTurnNo, Ignored);

	//Update game screen widgets for new room and current game.
	if (!CGameScreen::pMapWidget->LoadFromCurrentGame(CGameScreen::pCurrentGame) ||
			!CGameScreen::pRoomWidget->LoadFromCurrentGame(CGameScreen::pCurrentGame)) 
		return false;
	CGameScreen::SetSignTextToCurrentRoom();
	
	return true;
}

//
//CDemoScreen protected methods.
//

//*****************************************************************************
CDemoScreen::CDemoScreen()
   : CGameScreen(SCR_Demo)
   , pCurrentCommand(NULL)
   , dwNextAnimateTime(0L), dwNextCommandTime(0L)
   , pDemo(NULL), bCanChangeSpeed(false)
//Constructor.
{
}

//*****************************************************************************
CDemoScreen::~CDemoScreen()
//Destructor.
{
	delete this->pDemo;
}

//******************************************************************************
bool CDemoScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Check that caller loaded a current game and demo.
	if (!CGameScreen::pCurrentGame || !this->pDemo)
	{
		ASSERTP(false, "Current game and demo not loaded."); 
		return false;
	}

	//Set frame rate as high as needed for smooth animations.
	SetBetweenEventsInterval(12);

	//Get first command.
	this->pCurrentCommand = 
			CGameScreen::pCurrentGame->Commands.Get(this->pDemo->wBeginTurnNo);
	this->dwNextAnimateTime = SDL_GetTicks() + ANIMATE_DELAY;
	this->dwNextCommandTime = SDL_GetTicks() + FIRST_COMMAND_DELAY;

	return true;
}

//
//Private methods.
//

//*****************************************************************************
void CDemoScreen::OnKeyDown(
//Use the CScreen handler instead of CGameScreen's, which would process game
//commands in response to key pressed.
//
//Params:
	const DWORD dwTagNo, const SDL_KeyboardEvent &Key)	//(in)
{
	CScreen::OnKeyDown(dwTagNo, Key);
	if (IsDeactivating())
		return;

	//Hitting a key ends the demo.
	switch (Key.keysym.sym)
	{
      case SDLK_KP2: case SDLK_DOWN:
			this->bPaused = false;
			if (this->fScrollRateMultiplier < 20.0f) this->fScrollRateMultiplier *= 1.1f;
         break;
      case SDLK_KP8: case SDLK_UP:
			this->bPaused = false;
			if (this->fScrollRateMultiplier > 0.15f) this->fScrollRateMultiplier *= 0.9f;
         break;
      case SDLK_SPACE:
			this->bPaused = !this->bPaused;
         break;
      case SDLK_RETURN:
         if (Key.keysym.mod & KMOD_ALT &&	!GetHotkeyTag(Key.keysym.sym))
            //going to next case
		case SDLK_F10:
			//don't do anything here
			break;
		default:
			Deactivate();
			break;
	}
}

//******************************************************************************
void CDemoScreen::OnMouseUp(
//Handling mouse clicks.
//
//Params:
	const DWORD /*dwTagNo*/,	const SDL_MouseButtonEvent &/*Button*/)
{
	//Mouse click ends the demo.
	Deactivate();
}

//*****************************************************************************
void CDemoScreen::OnBetweenEvents()
//Called between events.
{
	//Animate the game screen.
	CGameScreen::OnBetweenEvents();

	//Process next command if it's time.
	Uint32 dwNow = SDL_GetTicks();
	if (dwNow >= this->dwNextCommandTime)
	{
		if (!this->pCurrentCommand)  //End of demo.
		{
			Deactivate();
			return;
		}
		ProcessCommand(this->pCurrentCommand->bytCommand);
		
		//Check for last turn in demo.
		if (CGameScreen::pCurrentGame->wTurnNo - 1 == this->pDemo->wEndTurnNo)
		{
			this->pCurrentCommand = NULL;
		}
		else	//Get next turn.
		{
			this->pCurrentCommand = CGameScreen::pCurrentGame->Commands.GetNext();			
			if (!this->pCurrentCommand && this->pDemo->dwNextDemoID) //Multi-room demo.
			{
				//Load next demo and get first command.
				if (!LoadDemoGame(this->pDemo->dwNextDemoID)) //Load failed.
				{
					Deactivate();
					return;
				}
				this->pCurrentCommand = CGameScreen::pCurrentGame->Commands.GetFirst();
			}
		}

		this->dwNextCommandTime = dwNow + ((this->pCurrentCommand == NULL) ?
				LAST_COMMAND_DELAY :
      static_cast<DWORD>(this->pCurrentCommand->byt10msElapsedSinceLast * 10 *
            (this->bCanChangeSpeed ? this->fScrollRateMultiplier : 1.0f)));
	}
}

void  CDemoScreen::SetReplayOptions(bool bChangeSpeed)
{
   this->bCanChangeSpeed = bChangeSpeed;
   this->fScrollRateMultiplier = 1.0f;
}

// $Log: DemoScreen.cpp,v $
// Revision 1.26  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.25  2003/09/22 20:37:47  mrimer
// Fixed bug: mouse click registered on two screens.
//
// Revision 1.24  2003/08/11 12:48:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.23  2003/07/31 21:49:02  schik
// Added changing of playback speed of demos (except those run from the title screen)
//
// Revision 1.22  2003/07/24 19:48:53  mrimer
// Now constructor calls base constructor and inits member vars in initializer list.
//
// Revision 1.21  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.20  2003/06/06 18:11:30  mrimer
// Revised frame rate delay.
//
// Revision 1.19  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.18  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.17  2003/05/08 22:04:41  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.16  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.15  2002/11/22 02:40:07  mrimer
// Removed animation delay to maximize frame rate.
//
// Revision 1.14  2002/10/21 20:47:23  mrimer
// A standard keypress or mouse click will end demo playback.
//
// Revision 1.13  2002/10/11 01:55:16  erikh2000
// Removed testing code.
//
// Revision 1.12  2002/09/04 22:29:46  erikh2000
// Added a "this->" in front of a variable for clarity.
//
// Revision 1.11  2002/09/01 00:06:00  erikh2000
// Changed reference to "wTurnsTaken" to "wTurnNo" which is incremented more often.
//
// Revision 1.10  2002/07/22 02:48:52  erikh2000
// Made face and room widgets animated.
//
// Revision 1.9  2002/07/17 20:44:14  erikh2000
// Added a temporary method used for displaying a demo scene (range of commands) on the demo screen.
//
// Revision 1.8  2002/07/09 22:35:47  mrimer
// Revised #includes.
//
// Revision 1.7  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.6  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.5  2002/06/15 18:37:05  erikh2000
// Fixed places where a CDbBase-derived class was not being deleted.
//
// Revision 1.4  2002/06/11 21:15:29  mrimer
// Removed specific level style music playing during demo.
//
// Revision 1.3  2002/05/21 21:32:37  erikh2000
// Fixed demo screen so that it could load a demo where the player dies at the end of it.
//
// Revision 1.2  2002/05/15 01:29:31  erikh2000
// Changed call to "SetType" to renamed "SetScreenType".
//
// Revision 1.1  2002/05/15 01:22:33  erikh2000
// Initial check-in.
//
