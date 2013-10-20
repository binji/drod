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

#include "WinStartScreen.h"
#include "GameScreen.h"
#include "DrodFontManager.h"
#include "DrodScreenManager.h"
#include "DrodSound.h"

#include "../DRODLib/Db.h"
#include "../DRODLib/DbPlayers.h"
#include "../Texts/MIDs.h"

const DWORD TAG_YOUWON_LABEL = 1001;
const DWORD TAG_INFO_LABEL = 1002;

const UINT CX_LEFTRIGHT_MARGIN = 65;

const int X_YOUWON = CX_LEFTRIGHT_MARGIN;
const int Y_YOUWON = 0;
const UINT CY_YOUWON = 45;

const UINT CY_LABEL_SPACING = 4;
const int X_INFO = X_YOUWON;
const int Y_INFO = Y_YOUWON + CY_YOUWON + CY_LABEL_SPACING;

//
//Protected methods.
//

//************************************************************************************
CWinStartScreen::CWinStartScreen(void) : CDrodScreen(SCR_WinStart)
//Constructor.
{ }

//*****************************************************************************
void CWinStartScreen::Paint(
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
	const SURFACECOLOR Black = {0, 0, 0};
	DrawFilledRect(rect, Black);

	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//************************************************************************************
bool CWinStartScreen::Load(void)
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{ 
	ASSERT(!this->bIsLoaded);

   static const UINT CX_YOUWON = CX_SCREEN - (CX_LEFTRIGHT_MARGIN * 2);
   static const UINT CX_INFO = CX_YOUWON;
   static const UINT CY_INFO = 1;

   //Add "You won" label.
	CLabelWidget *pYouWonLabel = new CLabelWidget(TAG_YOUWON_LABEL, X_YOUWON, Y_YOUWON,
			CX_YOUWON, CY_YOUWON, F_LevelName, wszEmpty);
	pYouWonLabel->SetAlign(CLabelWidget::TA_CenterGroup);
	AddWidget(pYouWonLabel);

	//Add info label underneath it, resizing to fit.
	CLabelWidget *pInfoLabel = new CLabelWidget(TAG_INFO_LABEL, X_INFO, Y_INFO, 
			CX_INFO, CY_INFO, F_LevelDescription, wszEmpty, true);
	AddWidget(pInfoLabel);

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//************************************************************************************
void CWinStartScreen::Unload(void)
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
bool CWinStartScreen::SetForActivate(void)
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	const bool bDugansDungeon = (g_pTheDB->GetHoldID() == HOLD_DUGANS_DUNGEON);
	
	//Always play music for screens in win sequence, if possible, unless user
	//really wants sound off (i.e. sound effects are turned off too).
	CDbPlayer *pPlayer = g_pTheDB->GetCurrentPlayer();
	if (pPlayer->Settings.GetVar("SoundEffects", true) && bDugansDungeon)
		g_pTheSound->EnableMusic(true);
	delete pPlayer;

	g_pTheSound->PlaySong(SONGID_WINGAME);

	//Return to the title screen for escape.
	g_pTheSM->ClearReturnScreens();
	g_pTheSM->InsertReturnScreen(SCR_Title);

   //Prepare "You Won" text.
   CDbHold *pHold = g_pTheDB->Holds.GetByID(g_pTheDB->GetHoldID());
   WSTRING wstrYouWon = g_pTheDB->GetMessageText(MID_YouConquered);
   wstrYouWon += wszSpace;
   wstrYouWon += pHold->NameText;
   wstrYouWon += wszExclamation;

   WSTRING wTmp;
   if (bDugansDungeon)
   {
      wTmp = g_pTheDB->GetMessageText(MID_WinDugansDungeon);
   }
   else
   {
      //Show customized end hold message, if defined.
      wTmp = (WSTRING)pHold->EndHoldText;
      if (pHold->EndHoldText.IsEmpty())
      {
         //Show canned end message.
         wTmp = g_pTheDB->GetMessageText(MID_WinHomemadeDungeon1);
      
         CDbPlayer *pAuthor = g_pTheDB->Players.GetByID(pHold->dwPlayerID);
         ASSERT(pAuthor);
         wTmp += pAuthor->NameText;
         delete pAuthor;

         if (pHold->editingPrivileges == CDbHold::YouAndConquerors)
         {
            wTmp += g_pTheDB->GetMessageText(MID_WinHomemadeDungeonCanEdit1);
            wTmp += wszSpace;
            wTmp += pHold->NameText;
            wTmp += wszSpace;
            wTmp += g_pTheDB->GetMessageText(MID_WinHomemadeDungeonCanEdit2);
         } else {
            wTmp += wszQuote;
            wTmp += wszPeriod;
         }
      }
   }
   delete pHold;

	//Now that I know combined height of two labels, center them vertically.
   CLabelWidget *pYouWonLabel = DYN_CAST(CLabelWidget *, CWidget*, GetWidget(TAG_YOUWON_LABEL));
   ASSERT(pYouWonLabel);
   CLabelWidget *pInfoLabel = DYN_CAST(CLabelWidget *, CWidget*, GetWidget(TAG_INFO_LABEL));
   ASSERT(pInfoLabel);
   pYouWonLabel->SetText(wstrYouWon.c_str(), true);
   pInfoLabel->SetText(wTmp.c_str(), true);
	const int cyLabels = pYouWonLabel->GetH() + CY_LABEL_SPACING + pInfoLabel->GetH();
	const int yCenterOffset = (CY_SCREEN - cyLabels) / 2;
	pYouWonLabel->Move(X_YOUWON, Y_YOUWON + yCenterOffset);
	pInfoLabel->Move(X_INFO, pYouWonLabel->GetY() + pYouWonLabel->GetH() + CY_LABEL_SPACING);
	
	return true;
}

//
//Private methods.
//

//******************************************************************************
void CWinStartScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD dwTagNo,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	CScreen::OnKeyDown(dwTagNo, Key);
	if (IsDeactivating()) return;

	const bool bDugansDungeon = (g_pTheDB->GetHoldID() == HOLD_DUGANS_DUNGEON);
	GoToScreen(bDugansDungeon ? SCR_WinAudience : SCR_Return);
}

//******************************************************************************
void CWinStartScreen::OnMouseDown(
//Called when widget receives SDL_MOUSEBUTTONDOWN event.
//
//Params:
	const DWORD dwTagNo,				//(in)	Widget that event applied to.
	const SDL_MouseButtonEvent &Button)	//(in)	Event.
{
	CScreen::OnMouseDown(dwTagNo, Button);
	const bool bDugansDungeon = (g_pTheDB->GetHoldID() == HOLD_DUGANS_DUNGEON);
	GoToScreen(bDugansDungeon ? SCR_WinAudience : SCR_Return);
}

// $Log: WinStartScreen.cpp,v $
// Revision 1.18  2004/02/17 14:35:36  mrimer
// Reenable muted music only if ending for Dugan's Dungeon will be shown.
//
// Revision 1.17  2003/08/01 17:21:22  mrimer
// Changed static_casts to DYN_CAST.
//
// Revision 1.16  2003/07/19 02:17:51  mrimer
// Added display of personalized ending text.
//
// Revision 1.15  2003/07/01 20:24:55  mrimer
// Moved all end of game text into DB.
//
// Revision 1.14  2003/06/27 18:27:37  mrimer
// Changed end message to only mention editing the hold if it has privileges set for conquerors.
//
// Revision 1.13  2003/06/22 05:58:00  mrimer
// Fixed a bug.
//
// Revision 1.12  2003/06/20 00:42:23  mrimer
// Fixed text misalignment.
//
// Revision 1.11  2003/06/16 18:48:44  mrimer
// Fixed bug: end text not being updated for completing two+ holds.
//
// Revision 1.10  2003/06/09 21:47:39  erikh2000
// Changed end-of-hold texts.
//
// Revision 1.9  2003/05/28 23:11:52  erikh2000
// TA_* constants are used differently.
//
// Revision 1.8  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.7  2003/05/19 20:29:29  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.6  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.5  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.4  2003/02/16 20:32:20  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.3  2002/11/22 02:27:37  mrimer
// Added preliminary text for completing new holds.
//
// Revision 1.2  2002/11/18 17:26:52  mrimer
// Fixed bug and spelling error.
//
// Revision 1.1  2002/10/10 01:03:51  erikh2000
// Initial check-in.
//
