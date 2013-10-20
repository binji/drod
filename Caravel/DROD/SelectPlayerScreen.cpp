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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "SelectPlayerScreen.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include "GameScreen.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/DbSavedGames.h"
#include "../DRODLib/DbXML.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

const DWORD TAG_LIST_BOX = 1090L;
const DWORD TAG_CANCEL = 1091L;
const DWORD TAG_NEWPLAYER = 1092L;
const DWORD TAG_DELETEPLAYER = 1093L;
const DWORD TAG_EXPORT = 1094L;
const DWORD TAG_IMPORT = 1095L;

bool CSelectPlayerScreen::bFirst = true;

//
//Protected methods.
//

//*****************************************************************************
CSelectPlayerScreen::CSelectPlayerScreen() : CDrodScreen(SCR_SelectPlayer)
   , pPlayerBox(NULL)
   , pPlayerListBoxWidget(NULL)
   , pOKButton(NULL), pNewPlayerButton(NULL), pDeletePlayerButton(NULL)
   , pExportPlayerButton(NULL), pImportPlayerButton(NULL)
   , pPlayerHoldLabel(NULL), pPlayerPositionLabel(NULL)
//Constructor.
{
}

//*****************************************************************************
CSelectPlayerScreen::~CSelectPlayerScreen()
//Destructor.
{
}

//*****************************************************************************
bool CSelectPlayerScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{ 
	ASSERT(!this->bIsLoaded);
	
	this->pPlayerBox = new CDialogWidget(0L, 0, 0, 300, 270, true);
	this->pPlayerBox->AddWidget(
			new CLabelWidget(0L, 100, 8, 100, 25,
					F_Message, g_pTheDB->GetMessageText(MID_SelectPlayerPrompt)));

   //Player position labels.
   this->pPlayerHoldLabel = new CLabelWidget(0, 20, 165, 260, 20, F_Small, NULL);
   this->pPlayerHoldLabel->SetAlign(CLabelWidget::TA_CenterGroup);
   this->pPlayerBox->AddWidget(this->pPlayerHoldLabel);
   this->pPlayerPositionLabel = new CLabelWidget(0, 20, 185, 260, 20, F_Small, NULL);
   this->pPlayerPositionLabel->SetAlign(CLabelWidget::TA_CenterGroup);
   this->pPlayerBox->AddWidget(this->pPlayerPositionLabel);

	//Buttons
	this->pOKButton = new CButtonWidget(
			TAG_OK, 15, 210, 60, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	this->pPlayerBox->AddWidget(this->pOKButton);
	this->pNewPlayerButton = new CButtonWidget(
			TAG_NEWPLAYER, 85, 210, 95, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_NewPlayer));
	this->pPlayerBox->AddWidget(this->pNewPlayerButton);
	this->pDeletePlayerButton = new CButtonWidget(
			TAG_DELETEPLAYER, 190, 210, 95, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_DeletePlayer));
	this->pPlayerBox->AddWidget(this->pDeletePlayerButton);
	this->pExportPlayerButton = new CButtonWidget(
			TAG_EXPORT, 25, 240, 70, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Export));
	this->pPlayerBox->AddWidget(this->pExportPlayerButton);
	this->pImportPlayerButton = new CButtonWidget(
			TAG_IMPORT, 115, 240, 70, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Import));
	this->pPlayerBox->AddWidget(this->pImportPlayerButton);
	this->pPlayerBox->AddWidget(new CButtonWidget(
			TAG_CANCEL, 200, 240, 70, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Cancel)));

	//Player list.
	this->pPlayerListBoxWidget = new CListBoxWidget(TAG_LIST_BOX,
			25, 35, 250, 125, true);
	this->pPlayerBox->AddWidget(pPlayerListBoxWidget);

	AddWidget(this->pPlayerBox,true);
	this->pPlayerBox->Center();
	this->pPlayerBox->Hide();

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//*****************************************************************************
void CSelectPlayerScreen::Unload()
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
void CSelectPlayerScreen::Paint(
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
bool CSelectPlayerScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
   const bool bFirstTime = CSelectPlayerScreen::bFirst;
   if (bFirstTime)
      g_pTheSound->PlaySong(SONGID_INTRO);

	ShowCursor();

	//Find out whether any local player records exist.
	const DWORD dwPlayerID = g_pTheDB->GetPlayerID();
	if (dwPlayerID)
   {
      //Some players exist.  Select one.
      if (bFirstTime)
      {
         SetPlayerHold(dwPlayerID);
         SelectPlayer();
	      CSelectPlayerScreen::bFirst = false;
         return false;  //Don't have to enter screen.
      }
      return true;
   }

	//No players exist -- add one (show player selection dialog).
   SelectPlayer();
	CSelectPlayerScreen::bFirst = false;
   if (!IsDeactivating())
      HiResPrompt();

   //Can leave this screen now.
	return false;
}

//*****************************************************************************
void CSelectPlayerScreen::OnBetweenEvents()
//Bring up player selection dialog right away.
//As soon as it closes, return from this screen.
{
   SelectPlayer();

   if (!IsDeactivating())
	   GoToScreen(SCR_Return);
}

//*****************************************************************************
void CSelectPlayerScreen::OnSelectChange(
//Handles a selection change.
//
//Params:
	const DWORD dwTagNo)			//(in)	Widget that event applied to.
{
	switch (dwTagNo)
	{
		case TAG_LIST_BOX:
		{
			//Update the description label.
         const DWORD dwPlayerID = GetSelectedItem();
         SetPlayerDesc(dwPlayerID);
		}
		break;

      default: break;
	}
}

//
// Private methods
//

//*****************************************************************************
DWORD CSelectPlayerScreen::AddPlayer()
//Add a local player to the DB.
//
//Returns: ID of new player added, else 0L if none
{
	WSTRING wstrPlayerName;
	const DWORD dwTagNo = ShowTextInputMessage(
		g_pTheDB->GetMessageText(MID_NewPlayerPrompt), wstrPlayerName);

	if (dwTagNo == TAG_OK)
	{
      CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();

		//Add player to DB.
		CDbPlayer *pPlayer = g_pTheDB->Players.GetNew();
		pPlayer->NameText = wstrPlayerName.c_str();
		pPlayer->EMailText = wszEmpty;
      if (pCurrentPlayer)
      {
         //By default, give new player same settings as current player.
         pPlayer->Settings = pCurrentPlayer->Settings;
         delete pCurrentPlayer;
      }
		pPlayer->Update();
		const DWORD dwPlayerID = pPlayer->dwPlayerID;
		delete pPlayer;
		g_pTheDB->Commit();

		return dwPlayerID;
	}

	return 0L;
}

//*****************************************************************************
DWORD CSelectPlayerScreen::GetSelectedItem()
//Returns: tag of item selected in the list box widget.
{
	return this->pPlayerListBoxWidget->GetSelectedItem();
}

//*****************************************************************************
void CSelectPlayerScreen::HiResPrompt()
//Warn new user about using high resolutions in windowed mode.
{
	if (!IsFullScreen())
	{
		SDL_Rect **modes=SDL_ListModes(NULL, SDL_FULLSCREEN);
		if (modes)	//In some cases, full screen modes are not available
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
void CSelectPlayerScreen::SelectPlayer()
{
	DWORD dwPlayerID;
	const DWORD dwCurrentPlayerID = dwPlayerID = g_pTheDB->GetPlayerID();

	this->pPlayerBox->Show();
	do {
		SetPlayerID(dwPlayerID);
		if (!dwPlayerID)
			if (OnQuit())
				return;
	} while (!dwPlayerID);
	this->pPlayerBox->Hide();

	if (dwPlayerID != dwCurrentPlayerID)
	{
		//Disable any previous player's game.
		CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
				g_pTheSM->GetLoadedScreen(SCR_Game));
		pGameScreen->UnloadGame();

		//Set player, and find latest hold player had saved in.
		g_pTheDB->SetPlayerID(dwPlayerID);
      SetPlayerHold(dwPlayerID);
   }
}

//*****************************************************************************
void CSelectPlayerScreen::SetPlayerDesc(const DWORD dwPlayerID)
//Sets the labels to display the indicated player's
//current location in the game, based on the latest continue slot.
{
   static const WCHAR wszSignSep[] = { W_t(':'), W_t(' '), W_t(0) };

   if (dwPlayerID)
   {
	   const DWORD dwSavedGameID = g_pTheDB->SavedGames.FindByContinueLatest(dwPlayerID);
      CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetByID(dwSavedGameID);
      if (pSavedGame)
      {
         ASSERT(pSavedGame->dwPlayerID == dwPlayerID);
         CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(pSavedGame->dwRoomID);
         delete pSavedGame;
         if (pRoom)
         {
            WSTRING wstrRoomPosDesc;
            pRoom->GetLevelPositionDescription(wstrRoomPosDesc);
	         CDbLevel *pLevel = g_pTheDB->Levels.GetByID(pRoom->dwLevelID);
            delete pRoom;
            if (pLevel)
            {
	            WSTRING wstrLevelPosDesc = (const WCHAR *)pLevel->NameText;
	            wstrLevelPosDesc += wszSignSep;
               wstrLevelPosDesc += wstrRoomPosDesc;
               this->pPlayerPositionLabel->SetText(wstrLevelPosDesc.c_str());

               CDbHold *pHold = g_pTheDB->Holds.GetByID(pLevel->dwHoldID);
	            delete pLevel;
	            if (pHold)
               {
                  this->pPlayerHoldLabel->SetText(pHold->NameText);
                  delete pHold;
                  Paint();
                  return;
               }
            }
         }
      }
   }

   //If any of the above steps failed, blank the fields.
   this->pPlayerHoldLabel->SetText(NULL);
   this->pPlayerPositionLabel->SetText(NULL);
   Paint();
}

//*****************************************************************************
void CSelectPlayerScreen::SetPlayerHold(const DWORD dwPlayerID) const
//Set active hold to the last one player was playing.
{
	const DWORD dwSavedGameID = g_pTheDB->SavedGames.FindByContinueLatest(dwPlayerID);
	const DWORD dwHoldID = g_pTheDB->SavedGames.GetHoldIDofSavedGame(dwSavedGameID);
	if (dwHoldID)
		g_pTheDB->SetHoldID(dwHoldID);
}

//*****************************************************************************
void CSelectPlayerScreen::SetPlayerID(
//If this is the first time this screen is entered (on startup)
//and there is only one local player in the DB, then automatically select them.
//
//Otherwise, displays a dialog box with a list box of all players in the DB.
//
//Hitting OK will set the parameter to the selected player's ID and exit.
//Hitting New Player will prompt for a new player name and add them to the DB.
//Hitting Delete Player will delete the selected player upon confirmation.
//
//Params:
	DWORD &dwPlayerID)	//(in/out)	ID of selected player on OK.
{
	//Get the local players in the DB.
	PopulatePlayerListBox(this->pPlayerListBoxWidget);

	//Automatically select only player on startup.
	if (CSelectPlayerScreen::bFirst &&
			this->pPlayerListBoxWidget->GetItemCount() == 1)
		return;

	//Select current choice.
	this->pPlayerListBoxWidget->SelectItem(dwPlayerID);
   SetPlayerDesc(dwPlayerID);

	DWORD dwTagNo;
	do
	{
		if (pPlayerListBoxWidget->GetItemCount() == 0)
		{
			this->pOKButton->Disable();
			this->pDeletePlayerButton->Disable();
			this->pExportPlayerButton->Disable();
		} else {
			this->pOKButton->Enable();
			this->pDeletePlayerButton->Enable();
			this->pExportPlayerButton->Enable();
		}

		dwTagNo = this->pPlayerBox->Display();
		switch (dwTagNo)
		{
		case TAG_OK:
			//Get selected value.
			dwPlayerID = this->pPlayerListBoxWidget->GetSelectedItem();
         EnablePlayerSettings(dwPlayerID);
			break;
		case TAG_NEWPLAYER:
      {
			//Add new player and select it.
			const DWORD dwNewPlayerID = AddPlayer();
			if (dwNewPlayerID)
			{
            dwPlayerID = dwNewPlayerID;
				PopulatePlayerListBox(this->pPlayerListBoxWidget);
				this->pPlayerListBoxWidget->SelectItem(dwPlayerID);
            SetPlayerDesc(dwPlayerID);
			}
			break;
      }
		case TAG_DELETEPLAYER:
			//Delete player on confirmation.
			if (ShowYesNoMessage(MID_DeletePlayerPrompt) == TAG_YES)
			{
            SetCursor(CUR_Wait);
				g_pTheDB->Players.Delete(this->pPlayerListBoxWidget->GetSelectedItem());
				PopulatePlayerListBox(this->pPlayerListBoxWidget);
				this->pPlayerListBoxWidget->SelectLine(0);
				dwPlayerID = this->pPlayerListBoxWidget->GetSelectedItem();
            SetCursor();
			}
			break;

		case TAG_EXPORT:
		{
			const DWORD dwPlayerID = this->pPlayerListBoxWidget->GetSelectedItem();
			if (dwPlayerID)
			{
				CDbPlayer *pPlayer = g_pTheDB->Players.GetByID(dwPlayerID);
				if (pPlayer)
				{
               //Default filename is player name.
					WSTRING wstrExportFile = (WSTRING)pPlayer->NameText;
					if (ExportSelectFile(MID_SavePlayerPath,wstrExportFile, EXT_PLAYER))
					{
						//Write the player file.
                  SetCursor(CUR_Wait);
						ShowStatusMessage(MID_Exporting);
						const bool bResult = CDbXML::ExportXML(ViewTypeStr(V_Players),
								p_PlayerID, dwPlayerID, wstrExportFile.c_str());
						HideStatusMessage();
                  SetCursor();
						ShowOkMessage(bResult ? MID_PlayerFileSaved :
								MID_PlayerFileNotSaved);
					}
					delete pPlayer;
				}
			}
		}
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
            //Select the imported player.
            dwPlayerID = dwImportedPlayerID;
				//Update in case a player was added.
				PopulatePlayerListBox(this->pPlayerListBoxWidget);
				this->pPlayerListBoxWidget->SelectItem(dwPlayerID);
            SetPlayerDesc(dwPlayerID);
			}
		}
		break;

		case TAG_CANCEL:
		case TAG_ESCAPE:
		case TAG_QUIT:
			return;
		}
	} while (dwTagNo != TAG_OK	|| !dwPlayerID);
}

//*****************************************************************************
void CSelectPlayerScreen::PopulatePlayerListBox(
//Puts levels of current hold into list box.
//
//Params:
	CListBoxWidget *pPlayerListBoxWidget)	//(in/out)
const
{
	BEGIN_DBREFCOUNT_CHECK;
	pPlayerListBoxWidget->Clear();

	//Get holds in DB.
	{
      CDb db;
		db.Players.FilterByLocal();
		CDbPlayer *pPlayer = db.Players.GetFirst();
		while (pPlayer)
		{
			pPlayerListBoxWidget->AddItem(pPlayer->dwPlayerID, pPlayer->NameText);
			delete pPlayer;
			pPlayer = db.Players.GetNext();
		}
	}
	END_DBREFCOUNT_CHECK;
}

// $Log: SelectPlayerScreen.cpp,v $
// Revision 1.21  2004/01/02 01:00:50  mrimer
// Alphabetized hold/player lists.
//
// Revision 1.20  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.19  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.18  2003/08/11 12:48:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.17  2003/08/09 20:08:34  mrimer
// Fixed bug: cancelling import changes selected ID.
//
// Revision 1.16  2003/07/29 03:52:34  mrimer
// Fix for import of player demos.
//
// Revision 1.15  2003/07/24 04:03:41  mrimer
// Enabled double-clicking the list-box on the dialog to exit it with OK return value.
//
// Revision 1.14  2003/07/22 19:00:27  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.13  2003/07/19 02:23:10  mrimer
// Updated DB view access to use enum references rather than strings.
//
// Revision 1.12  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/07/13 06:45:31  mrimer
// Removed unused parameter from CDbXML::Export() call.
//
// Revision 1.10  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.9  2003/07/11 17:30:05  mrimer
// Now OK button receives initial focus.
//
// Revision 1.8  2003/07/11 13:21:00  mrimer
// Added check: whether there is a current player when copying player settings to new player.
//
// Revision 1.7  2003/07/11 05:47:06  mrimer
// When new player is created, they are now given the same settings as the current player by default.
//
// Revision 1.6  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/07/09 21:11:29  mrimer
// Revised Import API.
//
// Revision 1.4  2003/07/07 23:22:22  mrimer
// Added hourglass cursor during player deletion.
//
// Revision 1.3  2003/07/03 21:46:58  mrimer
// Fixed bug: going to player selection screen changes selected hold.
//
// Revision 1.2  2003/07/03 08:10:16  mrimer
// Put this screen back in the list of startup screens (and does not activate with only one player).
//
// Revision 1.1  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
//
