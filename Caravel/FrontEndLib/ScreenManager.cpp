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

#define INCLUDED_FROM_SCREENMANAGER_CPP
#include "ScreenManager.h"
#undef INCLUDED_FROM_SCREENMANAGER_CPP

#include "BitmapManager.h"
#include "Widget.h"

#include "Fade.h"
#include "Pan.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>

typedef list<CScreen *>::const_iterator LOADEDSCREENS_ITERATOR;

//Holds the only instance of CScreenManager for the app.
CScreenManager *g_pTheSM = NULL;

//*****************************************************************************
CScreenManager::CScreenManager(
//Constructor.
//
//Params:
	SDL_Surface *pSetScreenSurface) //(in)	The screen surface.
	: pCursor(NULL)
   , eTransition(Fade)
{
	ASSERT(pSetScreenSurface);
		
	//CScreenManager should only be instanced one time during the life
	//of the app.
	static bool bIsInstanced = false;
	ASSERT(!bIsInstanced);
	bIsInstanced = true;

	this->eTransition = Fade;

	SetWidgetScreenSurface(pSetScreenSurface);
	this->ReturnScreenList.push_back(SCREENLIB::SCR_None);
}

//*****************************************************************************
CScreenManager::~CScreenManager() 
//Destructor.
{
	FreeCursors();
	UnloadAllScreens();
}

//*****************************************************************************
void CScreenManager::ChangeReturnScreen(
//Sets the screen that the currently activated screen will return to if 
//it indicates SCR_Return for post-activation destination.  The return screen
//that was previously indicated will not be kept in the return sequence.
//
//Params:
	const UINT eScreen)	//(in)	Screen to return to.
{
	ASSERT(this->ReturnScreenList.size());

	//Get rid of the old return screen.
	this->ReturnScreenList.pop_back();
	
	//Add the new return screen.
	this->ReturnScreenList.push_back(eScreen);
}

//*****************************************************************************
void CScreenManager::ClearReturnScreens()
//Removes all screens in the return queue.
{
	while (GetReturnScreenType() != SCREENLIB::SCR_None)
		RemoveReturnScreen();
}

//*****************************************************************************
void CScreenManager::InsertReturnScreen(
//Sets the screen that the currently activated screen will return to if 
//it indicates SCR_Return for post-activation destination.  This return screen
//will be inserted after the current return screen.
//
//Params:
	const UINT eScreen)	//(in)	Screen to return to.
{
	//Shouldn't insert the same return screen.
	ASSERT(eScreen != this->ReturnScreenList.back());
	
	//Add the new return screen.
	this->ReturnScreenList.push_back(eScreen);
}

//*****************************************************************************
void CScreenManager::RemoveReturnScreen()
//Removes the screen that the currently activated screen will return to if
//it indicates SCR_Return for post-activation destination.  The return screen
//that preceded the current return screen will become the new return screen.
{
	ASSERT(this->ReturnScreenList.size());

	//Get rid of the old return screen.
	this->ReturnScreenList.pop_back();
}

//*****************************************************************************
UINT CScreenManager::GetReturnScreenType()
//Returns type of return screen.
const
{
	return (this->ReturnScreenList.size() == 0)
      ? static_cast<UINT>(SCREENLIB::SCR_None)
		: this->ReturnScreenList.back();
}

//*****************************************************************************
UINT CScreenManager::ActivateScreen(const UINT eScreen)
//After a screen is activated, the CScreen class for that screen will be
//responsible for handling input and output.  The method exits after player
//or other event causes the activated screen to exit.
//
//Returns:
//The next screen to activate.
{
	//It is illegal to make a reentrant call to ActivateScreen(), because only
	//one screen can be active at a time.
	static bool bInActivateScreen = false;
	ASSERT(!bInActivateScreen);
	bInActivateScreen = true;

	//Get a loaded screen.
	UINT eNextScreen;
	CScreen *pScreen = GetLoadedScreen(eScreen);
	if (!pScreen) pScreen = LoadScreen(eScreen);
	if (!pScreen)
	{
		//Bad unexpected error--set next screen to return, and player may still
		//be able to get to other places.
		eNextScreen = SCREENLIB::SCR_Return;
	}
	else
	{
		ASSERT(pScreen->bIsLoaded); //Make sure overrided CScreen::Load() set member.

		//A newly activated screen's destination is always SCR_Return until activation
		//code sets it to something else.
		pScreen->SetDestScreenType(SCREENLIB::SCR_Return);

		//Before painting anything, give the destination screen a chance to update,
		//which could change what is painted.
		if (pScreen->SetForActivate())
		{
			//Create a temporary screen-sized surface.
			SDL_Surface *pNewSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
				CScreen::CX_SCREEN, CScreen::CY_SCREEN, 24, 0, 0, 0, 0);
			ASSERT(pNewSurface);
			//Make the destination screen paint to the temp surface.
			pScreen->SetDestSurface(pNewSurface);
			//Paint the destination screen to the temp surface.
			pScreen->Paint();
			//Make destination screen point back to the screen surface.
			pScreen->SetDestSurface(0L);
			
			//Animate transition to dest screen.
            //this->eTransition = Cut; //!!
			switch (this->eTransition)
			{
				case Cut:
					pScreen->Paint();
				break;
				
				case Fade:
				{
					CFade fade(pScreen->GetDestSurface(), pNewSurface);
					fade.FadeBetween();
				}
				break;

				case Pan:
				{
					CPan pan(pScreen->GetDestSurface(), pNewSurface,
							(PanDirection)(rand() % PanCount));
					pan.PanBetween();
				}
				break;

				default:
					ASSERTP(false, "Bad transition type.");
				break;
			}
			SDL_FreeSurface(pNewSurface);

			//Reset to default screen transition.
			this->eTransition = Fade;

			//Activate the screen.  Execution returns when the screen is exited.
			pScreen->Activate();
		}
		//else pre-activation tasks failed (go to the indicated screen).
		eNextScreen = pScreen->GetDestScreenType();
	}

	//Figure out which screen is next.
	switch (eNextScreen)
	{
		case 0:	//SCR_None:		//Exit app.
		break;

		case 1:	//SCR_Return:	//The screen before this one.
			eNextScreen = this->ReturnScreenList.back();
			this->ReturnScreenList.pop_back();
		break;

		default:			//Another screen.
			//Push this screen onto the return list, so that player can return to
			//it from the next screen.  
			this->ReturnScreenList.push_back(eScreen);
		break;
	}

	bInActivateScreen = false;
	return eNextScreen;
}

//*****************************************************************************
CScreen * CScreenManager::GetLoadedScreen(
//Gets a screen that was previously loaded.
//
//Params:
	const UINT eScreen) //(in)	Screen to get.
//
//Returns:
//Pointer to screen or NULL if screen is not loaded.	
const
{
	//Each iteration looks at one loaded screen for a match.
	for (LOADEDSCREENS_ITERATOR iSeek = this->LoadedScreens.begin();
		iSeek != this->LoadedScreens.end(); ++iSeek)
	{
		if ((*iSeek)->eType == eScreen) 
			return *iSeek; //Found it.
	}

	//No match.
	return NULL;
}

//*****************************************************************************
CScreen * CScreenManager::GetScreen(
//Gets a screen, loading it first if necessary.
//
//Params:
	const UINT eScreen)	//(in)	Screen to get.
//
//Returns:
//Pointer to screen or NULL if screen failed to load.
{
	CScreen *pRet = GetLoadedScreen(eScreen);
	if (pRet) 
		return pRet;
	else
		return LoadScreen(eScreen);
}

//*****************************************************************************
bool CScreenManager::IsScreenLoaded(const UINT eScreen) const 
{
	return (GetLoadedScreen(eScreen)!=NULL);
}

//
//CScreenManager private methods.
//

//*****************************************************************************
SDL_Cursor* CScreenManager::LoadCursor(const char* cursorName)
//Loads special cursor used by app.
//
//Returns:
//Pointer to cursor if successful, else NULL.
{
	//Get bitmap containing cursor pixels.
	SDL_Surface *pCursorBMP = g_pTheBM->GetBitmapSurface(cursorName);
	if (!pCursorBMP) return NULL;
	const UINT wBPP = pCursorBMP->format->BytesPerPixel;
	ASSERT(wBPP==3);

	//Translate BMP pixels into cursor arrays.
	ASSERT(pCursorBMP->w % 8 == 0 && pCursorBMP->w > 0 && pCursorBMP->w <= 64);
	ASSERT(pCursorBMP->h > 0 && pCursorBMP->h <= 64);
	const UINT wArraySize = ((pCursorBMP->w / 8) * pCursorBMP->h);
	Uint8 *pMask = new Uint8[wArraySize];
	Uint8 *pData = new Uint8[wArraySize];
	Uint8 *pSrc = static_cast<Uint8 *>(pCursorBMP->pixels);
	
	SDL_Cursor *pCursor = NULL;
	if (pMask && pData)
	{
		//Each element converts 8 source pixels to one element of the mask
		//and data arrays.
		for (UINT wI = 0; wI < wArraySize; ++wI)
		{
			pData[wI] =
				((pSrc[0] == 0) * 128) +
				((pSrc[wBPP * 1] == 0) * 64) +
				((pSrc[wBPP * 2] == 0) * 32) +
				((pSrc[wBPP * 3] == 0) * 16) +
				((pSrc[wBPP * 4] == 0) * 8) +
				((pSrc[wBPP * 5] == 0) * 4) +
				((pSrc[wBPP * 6] == 0) * 2) +
				((pSrc[wBPP * 7] == 0) * 1);

			pMask[wI] =
				((pSrc[0] != 128) * 128) +
				((pSrc[wBPP * 1] != 128) * 64) +
				((pSrc[wBPP * 2] != 128) * 32) +
				((pSrc[wBPP * 3] != 128) * 16) +
				((pSrc[wBPP * 4] != 128) * 8) +
				((pSrc[wBPP * 5] != 128) * 4) +
				((pSrc[wBPP * 6] != 128) * 2) +
				((pSrc[wBPP * 7] != 128) * 1);

			pSrc += (wBPP * 8);
		}

		//Create the cursor.
		pCursor = SDL_CreateCursor(pData, pMask, pCursorBMP->w, 
				pCursorBMP->h, 0, 0);
	}

	delete [] pMask;
	delete [] pData;
	g_pTheBM->ReleaseBitmapSurface(cursorName);

	return pCursor;
}

//*****************************************************************************
CScreen * CScreenManager::LoadScreen(
//Load a screen.
//
//Params:
	const UINT eScreen)	//(in)	Screen to load.
//
//Returns:
//Pointer to screen or NULL if the screen could not be loaded.
{
	//Should not load a screen twice.
	ASSERT(GetLoadedScreen(eScreen)==NULL);

	//Load the screen.
	CScreen *pScreen = GetNewScreen(eScreen);
	if (!pScreen->Load())
	{
		delete pScreen;
		return NULL;
	}
	ASSERT(pScreen->bIsLoaded); //Make sure overrided CScreen::Load() set member.

	//Add screen to loaded screens lists.
	this->LoadedScreens.push_back(pScreen);

	//Success.
	return pScreen;
}

//*****************************************************************************
void CScreenManager::UnloadAllScreens()
//Unload all the screens.
{
	//Each iteration unloads a screen.
	for (LOADEDSCREENS_ITERATOR iSeek = this->LoadedScreens.begin();
		iSeek != this->LoadedScreens.end(); ++iSeek)
	{
		CScreen *pDelete = *iSeek;
		pDelete->Unload();
		delete pDelete;
	}

	//Clear the list.
	this->LoadedScreens.clear();
}

// $Log: ScreenManager.cpp,v $
// Revision 1.9  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.8  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/07/12 01:12:00  mrimer
// Revised screen/cursor naming in FrontEndLib.
//
// Revision 1.6  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.5  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/04 05:34:43  mrimer
// Fixed a large memory leak.
//
// Revision 1.3  2003/05/30 04:06:23  mrimer
// Cleaned up some code.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
