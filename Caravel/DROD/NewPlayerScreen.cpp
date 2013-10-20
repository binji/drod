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
 * Contributor(s): Mike Rimer (mrimer), Matt Schikore (schik)
 *
 * ***** END LICENSE BLOCK ***** */

#include "NewPlayerScreen.h"
#include "GameScreen.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/TextBoxWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/DbXML.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

const DWORD TAG_CANCEL = 1091L;
const DWORD TAG_IMPORT = 1092L;
const DWORD TAG_NAME = 1093L;

//
//Protected methods.
//

//*****************************************************************************
CNewPlayerScreen::CNewPlayerScreen() : CDrodScreen(SCR_NewPlayer)
   , pPlayerBox(NULL)
   , pOKButton(NULL), pImportPlayerButton(NULL)
   , pNameWidget(NULL)
//Constructor.
{
}

//*****************************************************************************
CNewPlayerScreen::~CNewPlayerScreen()
//Destructor.
{
}

//*****************************************************************************
bool CNewPlayerScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{ 
	ASSERT(!this->bIsLoaded);
	
	this->pPlayerBox = new CDialogWidget(0L, 0, 0, 300, 170);
	this->pPlayerBox->AddWidget(
			new CLabelWidget(0L, 20, 8, 260, 100,
					F_Message, g_pTheDB->GetMessageText(MID_NewPlayerDialogPrompt)), 1);

   //Name entry field (gets focus).
   this->pNameWidget = new CTextBoxWidget(TAG_NAME, 50, 100, 200, 25);
   this->pPlayerBox->AddWidget(this->pNameWidget);

	//Buttons
	this->pOKButton = new CButtonWidget(
			TAG_OK, 15, 135, 60, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	this->pPlayerBox->AddWidget(this->pOKButton);
	this->pImportPlayerButton = new CButtonWidget(
			TAG_IMPORT, 220, 135, 70, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Import));
	this->pPlayerBox->AddWidget(this->pImportPlayerButton);
	this->pPlayerBox->AddWidget(new CButtonWidget(
			TAG_CANCEL, 90, 135, 70, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Cancel)));

	AddWidget(this->pPlayerBox,true);
	this->pPlayerBox->Center();
	this->pPlayerBox->Hide();

   this->pPlayerBox->SetEnterText(TAG_NAME);

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//*****************************************************************************
void CNewPlayerScreen::Unload()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{ 
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	this->bIsLoaded = false;
}

//*****************************************************************************
void CNewPlayerScreen::Paint(
//Overridable method to paint the screen.  
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	SDL_Surface *pScreenSurface = GetDestSurface();
	SDL_Rect rect = {0, 0, pScreenSurface->w, pScreenSurface->h};
	static const SURFACECOLOR Black = {0, 0, 0};
	DrawFilledRect(rect, Black);

	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
bool CNewPlayerScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
   g_pTheSound->PlaySong(SONGID_INTRO);

	ShowCursor();

   //Need to enter this screen if no players exist yet.
	const DWORD dwPlayerID = g_pTheDB->GetPlayerID();
	return dwPlayerID == 0;
}

//*****************************************************************************
void CNewPlayerScreen::OnBetweenEvents()
//Bring up player selection dialog right away.
//As soon as it closes, return from this screen.
{
   DWORD dwTagNo;
   do {
      dwTagNo = this->pPlayerBox->Display();
		switch (dwTagNo)
		{
		case TAG_OK:
			//Create a user with the selected name
         AddPlayer();
         Deactivate();
			break;

      case TAG_CANCEL:
         if (ShowYesNoMessage(MID_ReallyQuit) != TAG_NO)
            GoToScreen(SCR_None);
         break;

		case TAG_IMPORT:
		{
         //Ensure highlight room ID list is compiled.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetLoadedScreen(SCR_Game));
         if (!pGameScreen) break;

         //Import a player data file.
         DWORD dwImportedPlayerID;
			Import(EXT_PLAYER, dwImportedPlayerID);
         if (CDbXML::WasImportSuccessful() && dwImportedPlayerID)
			{
            Deactivate();
			}
		}
		break;
      }
   } while (!IsDeactivating());

   const DWORD dwPlayerID = g_pTheDB->GetPlayerID();
   if (dwPlayerID)
      HiResPrompt();
}

//
// Private methods
//

//*****************************************************************************
DWORD CNewPlayerScreen::AddPlayer()
//Add a local player to the DB.
//
//Returns: ID of new player added, else 0L if none
{
	//Add player to DB.
	CDbPlayer *pPlayer = g_pTheDB->Players.GetNew();
	pPlayer->NameText = this->pNameWidget->GetText();
	pPlayer->EMailText = wszEmpty;
	pPlayer->Update();
	const DWORD dwPlayerID = pPlayer->dwPlayerID;
	delete pPlayer;
	g_pTheDB->Commit();

	return dwPlayerID;
}

//*****************************************************************************
void CNewPlayerScreen::HiResPrompt()
//Warn new user about using high resolutions in windowed mode.
{
	if (!IsFullScreen())
	{
		SDL_Rect **modes=SDL_ListModes(NULL, SDL_FULLSCREEN);
		// SDL_ListModes is allowed to return NULL if there are no
		// modes available, or (SDL_Rect**)-1 if any resolution is
		// allowed.
		if (modes && modes != (SDL_Rect**)-1)	//In some cases, full screen modes are not available
		{
			for (int i=0; modes[i]; ++i)
				if (modes[i]->w > 800 && modes[i]->h > 600)
				{
					ShowOkMessage(MID_HighResWarning);
					break;
				}
		}
	}
}

//*****************************************************************************
void CNewPlayerScreen::SetPlayerHold(const DWORD dwPlayerID) const
//Set active hold to the last one player was playing.
{
	const DWORD dwSavedGameID = g_pTheDB->SavedGames.FindByContinueLatest(dwPlayerID);
	const DWORD dwHoldID = g_pTheDB->SavedGames.GetHoldIDofSavedGame(dwSavedGameID);
	if (dwHoldID)
		g_pTheDB->SetHoldID(dwHoldID);
}

// $Log: NewPlayerScreen.cpp,v $
// Revision 1.39  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.38  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.37  2003/08/09 20:08:34  mrimer
// Fixed bug: cancelling import changes selected ID.
//
// Revision 1.36  2003/07/29 03:52:34  mrimer
// Fix for import of player demos.
//
// Revision 1.35  2003/07/15 00:31:00  mrimer
// Fixed bug: not getting result of import properly.
//
// Revision 1.34  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.33  2003/07/07 23:27:27  mrimer
// Now text entry box starts with focus.
//
// Revision 1.32  2003/07/03 08:08:42  mrimer
// Removed uneeded methods.  Fixed a couple bugs.
//
// Revision 1.31  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
// Revision 1.30  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.29  2003/06/26 17:51:03  mrimer
// Added description of selected player's current location.  Revised method structure.
//
// Revision 1.28  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.27  2003/06/21 10:40:55  mrimer
// Added SetPlayerHold() to default the active hold to the latest the selected player played.
//
// Revision 1.26  2003/06/17 23:13:59  mrimer
// Code maintenance -- ShowTextInputMessage() now requires text entry by default.
//
// Revision 1.25  2003/06/12 21:47:28  mrimer
// Set default export file name to something more intuitive.
//
// Revision 1.24  2003/06/10 01:24:10  mrimer
// Fixed some bugs.
//
// Revision 1.23  2003/06/09 19:29:54  mrimer
// Paint screen before bringing up add player dialog.
//
// Revision 1.22  2003/05/29 20:08:47  mrimer
// Added Cancel button.
//
// Revision 1.21  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.20  2003/05/23 21:43:04  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.19  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.18  2003/05/21 03:07:39  mrimer
// Fixed some logic bugs.
//
// Revision 1.17  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.16  2003/04/29 11:15:31  mrimer
// Improved context-specific file selection.
//
// Revision 1.15  2003/04/28 22:58:28  mrimer
// Refactored export file selection code into CScreen::ExportSelectFile().
//
// Revision 1.14  2003/04/28 22:27:08  mrimer
// Dialog now correctly exits on <ESC> when a player is already selected.
//
// Revision 1.13  2003/04/26 17:15:57  mrimer
// Fixed some bugs with file selection.
//
// Revision 1.12  2003/04/21 21:23:32  mrimer
// Fixed a var type.
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
// Revision 1.7  2003/01/04 23:09:13  mrimer
// Updated file export interface.
//
// Revision 1.6  2002/12/22 02:35:12  mrimer
// Added import and export interface.
// Revised Load() and Unload() for more efficient widget handling.  Added SetForActivate().
// Fixed crash when no full screen modes are available.
//
// Revision 1.5  2002/11/15 02:36:24  mrimer
// Added multiple player support, including: adding, deleting and selecting players.
//
// Revision 1.4  2002/10/17 17:22:58  mrimer
// Added warning about small text when playing in high resolutions, hint to use full screen mode.
//
// Revision 1.3  2002/10/04 18:01:27  mrimer
// Refactored quit logic into OnQuit().
//
// Revision 1.2  2002/09/05 20:44:00  erikh2000
// Added a destructor with some cleanup code.
//
// Revision 1.1  2002/09/05 18:53:40  mrimer
// Initial check-in.
//
