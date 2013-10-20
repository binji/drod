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

#include "DrodScreen.h"
#include "DrodSound.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbPlayers.h"

//*****************************************************************************
CDrodScreen::CDrodScreen(
//Constructor.
//
//Params:
	const SCREENTYPE eSetType)			//(in)	Type of screen this is.
	: CScreen(eSetType)
	, pLevelBox(NULL)
{
}

//*****************************************************************************
void CDrodScreen::EnablePlayerSettings(
//Set game parameters according to player settings.
//
//Params:
   const DWORD dwPlayerID) //(in) Player
{
	CDbPlayer *pPlayer = g_pTheDB->Players.GetByID(dwPlayerID);
   if (!pPlayer)
   {
      ASSERTP(false, "Could not retrieve player data.");
      return;
   }

	g_pTheSound->EnableSoundEffects(pPlayer->Settings.GetVar("SoundEffects", true) != 0);
	g_pTheSound->SetSoundEffectsVolume(pPlayer->Settings.GetVar("SoundEffectsVolume", (BYTE)127));

	g_pTheSound->EnableMusic(pPlayer->Settings.GetVar("Music", true) != 0);
	g_pTheSound->SetMusicVolume(pPlayer->Settings.GetVar("MusicVolume", (BYTE)127));

   SetFullScreen(pPlayer->Settings.GetVar("Fullscreen", false) != 0);

   //RepeatRate is queried in CGameScreen::ApplyPlayerSettings().

   delete pPlayer;
}
   
//*****************************************************************************
bool CDrodScreen::SelectLevelID(
//Displays a dialog box with a list box of all active levels.
//Hitting OK will set the parameter to the selected level's ID.
//Hitting Cancel will leave it unchanged.
//
//Pre-cond: this->pLevelBox was loaded
//
//Returns: whether OK button was clicked (false implies Cancel)
//
//Params:
	CDbLevel *pLevel,		//(in)
	DWORD &dwLevelTagNo,	//(in/out)	ID of selected level on OK.
	const MESSAGE_ID messagePromptID,	//(in)
   bool bEnableCancel)   //(in) default = true
{
	this->pLevelBox->SetPrompt(messagePromptID);
	this->pLevelBox->SetSourceLevel(pLevel);
	this->pLevelBox->PopulateLevelList();

	//Select current choice.
	this->pLevelBox->SelectItem(dwLevelTagNo);

   this->pLevelBox->EnableCancelButton(bEnableCancel);
	this->pLevelBox->Show();
	const DWORD dwTagNo = this->pLevelBox->Display();
	this->pLevelBox->Hide();
	Paint();

	//Get selected value.
	if (dwTagNo == TAG_OK)
		dwLevelTagNo = this->pLevelBox->GetSelectedItem();
	return dwTagNo == TAG_OK;
}

// $Log: DrodScreen.cpp,v $
// Revision 1.5  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.4  2003/07/16 07:43:25  mrimer
// Revised selecting stair destination UI.
//
// Revision 1.3  2003/07/03 08:03:18  mrimer
// Added EnablePlayerSettings().
//
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
