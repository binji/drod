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
 * Portions created by the Initial Developer are Copyright (C) 
 * 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#include "LevelStartScreen.h"
#include "DrodFontManager.h"
#include "GameScreen.h"

#include "../DRODLib/CurrentGame.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbLevels.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Wchar.h>

const UINT CX_LEFTRIGHT_MARGIN = 50;
const UINT CY_HOLDNAME = 45;
const UINT CY_LEVELNAME = 45;
const UINT CY_DIVIDER_SPACE = 10;
const UINT CY_AUTHOR = 24;
const UINT CY_CREATED = CY_AUTHOR;
const UINT CY_TOPTEXT = CY_HOLDNAME + CY_LEVELNAME + CY_AUTHOR + CY_CREATED + CY_DIVIDER_SPACE;
const UINT CY_DESCRIPTION = CScreen::CY_SCREEN - (CY_TOPTEXT + CY_DIVIDER_SPACE);

//
//Protected methods.
//

//************************************************************************************
CLevelStartScreen::CLevelStartScreen(void) : CDrodScreen(SCR_LevelStart)
	, pHoldNameWidget(NULL), pLevelNameWidget(NULL), pCreatedWidget(NULL)
	, pAuthorWidget(NULL), pDescriptionWidget(NULL)
//Constructor.
{ }

//*****************************************************************************
void CLevelStartScreen::Paint(
//Overridable method to paint the screen.  
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
//
//As a placeholder, it justs draws a black screen to show something upon
//arrival to the screen.
{
	SDL_Surface *pScreenSurface = GetDestSurface();
	SDL_Rect rect = {0, 0, pScreenSurface->w, pScreenSurface->h};
	static const SURFACECOLOR Black = {0, 0, 0};
	DrawFilledRect(rect, Black);

	//Vertically center all the stuff.
	UINT wIgnored, wTextH;
	this->pDescriptionWidget->GetTextWidthHeight(wIgnored, wTextH);
	const int yTop = (CY_SCREEN - (CY_TOPTEXT + wTextH + CY_DIVIDER_SPACE)) / 3;	//slightly above center
	const int Y_DIVIDER = yTop + CY_TOPTEXT;

	//Move text to correct vertical position.
	this->pHoldNameWidget->Move(this->pHoldNameWidget->GetX(),yTop);
	this->pLevelNameWidget->Move(this->pLevelNameWidget->GetX(),yTop + CY_HOLDNAME);
	this->pCreatedWidget->Move(this->pCreatedWidget->GetX(),yTop + CY_HOLDNAME + CY_LEVELNAME);
	this->pAuthorWidget->Move(this->pAuthorWidget->GetX(),Y_DIVIDER - CY_DIVIDER_SPACE - CY_AUTHOR);
	this->pDescriptionWidget->Move(this->pDescriptionWidget->GetX(),Y_DIVIDER + CY_DIVIDER_SPACE);

	//Draw dividers.
	DrawDivider(Y_DIVIDER);
	const int nLowerDividerY = Y_DIVIDER + CY_DIVIDER_SPACE + wTextH + CY_DIVIDER_SPACE;
	if (nLowerDividerY < CY_SCREEN)
		DrawDivider(nLowerDividerY);

	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//************************************************************************************
bool CLevelStartScreen::Load(void)
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{ 
	ASSERT(!this->bIsLoaded);

	const int X_AUTHOR = CX_LEFTRIGHT_MARGIN;
	const UINT CX_AUTHOR = CX_SCREEN - (CX_LEFTRIGHT_MARGIN * 2);

	const int X_CREATED = X_AUTHOR;
	const UINT CX_CREATED = CX_AUTHOR;

	const int X_HOLDNAME = X_AUTHOR;
	const UINT CX_HOLDNAME = CX_AUTHOR;

	const int X_LEVELNAME = X_AUTHOR;
	const UINT CX_LEVELNAME = CX_AUTHOR;

	const int X_DESCRIPTION = X_AUTHOR;
	const UINT CX_DESCRIPTION = CX_AUTHOR;

	this->pHoldNameWidget = new CLabelWidget(0L, X_HOLDNAME, 0, 
			CX_HOLDNAME, CY_HOLDNAME, F_LevelName, wszEmpty);
	this->pHoldNameWidget->SetAlign(CLabelWidget::TA_CenterGroup);
	AddWidget(this->pHoldNameWidget);

	this->pLevelNameWidget = new CLabelWidget(0L, X_LEVELNAME, 0, 
			CX_LEVELNAME, CY_LEVELNAME, F_LevelName, wszEmpty);
	this->pLevelNameWidget->SetAlign(CLabelWidget::TA_CenterGroup);
	AddWidget(this->pLevelNameWidget);

	this->pCreatedWidget = new CLabelWidget(0L, X_CREATED, 0, CX_CREATED,
			CY_CREATED, F_LevelInfo, wszEmpty);
	this->pCreatedWidget->SetAlign(CLabelWidget::TA_CenterGroup);
	AddWidget(this->pCreatedWidget);

	this->pAuthorWidget = new CLabelWidget(0L, X_AUTHOR, 0, CX_AUTHOR, CY_AUTHOR,
			F_LevelInfo, wszEmpty);
	this->pAuthorWidget->SetAlign(CLabelWidget::TA_CenterGroup);
	AddWidget(this->pAuthorWidget);

	this->pDescriptionWidget = new CLabelWidget(0L, X_DESCRIPTION, 0, 
			CX_DESCRIPTION, CY_DESCRIPTION, F_LevelDescription, wszEmpty);
	this->pDescriptionWidget->SetAlign(CLabelWidget::TA_CenterGroup);
	AddWidget(this->pDescriptionWidget);
	
	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//************************************************************************************
void CLevelStartScreen::Unload(void)
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{ 
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	this->bIsLoaded = false;
}

//************************************************************************************
bool CLevelStartScreen::SetForActivate(void)
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Get level that the game screen is on.
	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetLoadedScreen(SCR_Game));
	const CCurrentGame *pCurrentGame = pGameScreen->GetCurrentGame();
	const DWORD dwLevelID = pCurrentGame->pLevel->dwLevelID;
	CDbLevel *pLevel = g_pTheDB->Levels.GetByID(dwLevelID);
	CDbHold *pHold = g_pTheDB->Holds.GetByID(pLevel->dwHoldID);
	if (!pLevel || !pHold) {ASSERTP(false, "Couldn't retrieve level or hold."); return false;}
	
	//Set label texts.
	this->pHoldNameWidget->SetText(pHold->NameText);
	this->pLevelNameWidget->SetText(pLevel->NameText);
	
	WSTRING wstrCreated = (WSTRING)CDbMessageText(MID_LevelCreated);
	wstrCreated += wszSpace;
	pLevel->Created.GetLocalFormattedText(DF_LONG_DATE, wstrCreated);
	this->pCreatedWidget->SetText(wstrCreated.c_str());

	WSTRING wstrAuthor = (WSTRING)CDbMessageText(MID_LevelBy);
	wstrAuthor += wszSpace;
	wstrAuthor += pLevel->GetAuthorText();
	this->pAuthorWidget->SetText(wstrAuthor.c_str());

	this->pDescriptionWidget->SetText(pLevel->DescriptionText);

	delete pLevel;
	delete pHold;

	//This screen should always return to the game screen.
	if (g_pTheSM->GetReturnScreenType() != SCR_Game)
		g_pTheSM->InsertReturnScreen(SCR_Game);

	//Start music playing.
	pGameScreen->SetMusicStyle(pGameScreen->GetCurrentGame()->pRoom->wStyle);

	return true;
}

//
//Private methods.
//

//******************************************************************************
void CLevelStartScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD dwTagNo,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	CScreen::OnKeyDown(dwTagNo, Key);
	if (IsDeactivating()) return;

   //Changing screen size shouldn't advance to next screen.
	switch (Key.keysym.sym)
	{
      case SDLK_RALT: case SDLK_LALT:
         return;
      case SDLK_RETURN:
         if (Key.keysym.mod & KMOD_ALT)
			   return;
		   break;
	   case SDLK_F10:
         return;
      default: break;
   }

	GoToScreen(SCR_Return);
}

//******************************************************************************
void CLevelStartScreen::OnMouseDown(
//Called when widget receives SDL_MOUSEBUTTONDOWN event.
//
//Params:
	const DWORD /*dwTagNo*/,				//(in)	Widget that event applied to.
	const SDL_MouseButtonEvent &/*Button*/)	//(in)	Event.
{
	GoToScreen(SCR_Return);
}

//******************************************************************************
void CLevelStartScreen::DrawDivider(
//Draws a divider at a specified row.
//
//Params:
	const int nY)	//(in)	Row to draw at.
{
	const SURFACECOLOR Gray = GetSurfaceColor(GetDestSurface(), 102, 102, 102);

	DrawRow(CX_LEFTRIGHT_MARGIN, nY, CX_SCREEN - (CX_LEFTRIGHT_MARGIN * 2), Gray);
}

// $Log: LevelStartScreen.cpp,v $
// Revision 1.21  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.20  2003/07/22 19:00:24  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.19  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.18  2003/06/06 18:22:53  mrimer
// Changing window size no longer goes to the next screen.
//
// Revision 1.17  2003/05/28 23:11:52  erikh2000
// TA_* constants are used differently.
//
// Revision 1.16  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.15  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.14  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.13  2003/05/19 20:29:28  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.12  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.11  2003/04/08 13:08:28  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.10  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.9  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.8  2003/02/16 20:32:19  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.7  2002/11/22 02:17:31  mrimer
// Added hold name above level name.
//
// Revision 1.6  2002/10/10 01:00:26  erikh2000
// Removed a dead comment.
//
// Revision 1.5  2002/07/19 20:40:31  mrimer
// Changed to display text vertically centered.
//
// Revision 1.4  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.3  2002/07/03 22:03:02  mrimer
// Increased description and created by font sizes.
//
// Revision 1.2  2002/06/23 10:54:02  erikh2000
// The level start screen now works completely.
//
// Revision 1.1  2002/06/21 22:31:17  erikh2000
// Initial check-in.
//
