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
 * Portions created by the Initial Developer are Copyright (C) 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributors:
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "HoldSelectScreen.h"
#include "DrodFontManager.h"
#include "GameScreen.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/DbXML.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

const DWORD TAG_LIST_BOX = 1090L;
const DWORD TAG_CANCEL = 1091L;
const DWORD TAG_EXPORT = 1092L;
const DWORD TAG_IMPORT = 1093L;
const DWORD TAG_DELETEHOLD = 1094L;

//
//Public methods.
//

//*****************************************************************************
CHoldSelectScreen::CHoldSelectScreen()
   : CDrodScreen(SCR_HoldSelect)
	, pHoldBox(NULL)
	, pHoldListBoxWidget(NULL)
   , pHoldDesc(NULL), pAuthorName(NULL)
	, pOKButton(NULL), pExportButton(NULL), pDeleteButton(NULL)
//Constructor
{
}

//*****************************************************************************
CHoldSelectScreen::~CHoldSelectScreen()
//Destructor.
{
}

//*****************************************************************************
bool CHoldSelectScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
   static const UINT CY_SPACE = 8;

   static const UINT CX_DIALOG = 300;

   static const int X_HOLDLISTBOX = 35;
   static const int Y_HOLDLISTBOX = 35;
   static const UINT CX_HOLDLISTBOX = CX_DIALOG - 2*X_HOLDLISTBOX;
   static const UINT CY_HOLDLISTBOX = 156;   //11 items

   static const int X_LABEL = 15;
   static const UINT CX_LABEL = CX_DIALOG - 2*X_LABEL;
   static const int Y_AUTHOR_LABEL = Y_HOLDLISTBOX + CY_HOLDLISTBOX + CY_SPACE/2;
   static const UINT CY_AUTHOR_LABEL = 16 + CY_SPACE/2;
   static const int Y_DESC_LABEL = Y_AUTHOR_LABEL + CY_AUTHOR_LABEL;
   static const UINT CY_DESC_LABEL = 67;  //5 lines of text

   static const UINT CX_BUTTON = 60;
   static const UINT CY_BUTTON = CY_STANDARD_BUTTON;
   static const UINT CX_BUTTON_SPACE = 10;
   static const UINT CX_DELETEHOLD_BUTTON = 80;
   static const int X_OK_BUTTON = 40;
   static const int Y_OK_BUTTON = Y_DESC_LABEL + CY_DESC_LABEL + CY_SPACE/2;
   static const int X_CANCEL_BUTTON = X_OK_BUTTON + CX_BUTTON + CX_BUTTON_SPACE;
   static const int Y_CANCEL_BUTTON = Y_OK_BUTTON;
   static const int X_DELETEHOLD_BUTTON = X_CANCEL_BUTTON + CX_BUTTON + CX_BUTTON_SPACE;
   static const int Y_DELETE_BUTTON = Y_OK_BUTTON;
   static const int X_EXPORT_BUTTON = 80;
   static const int Y_EXPORT_BUTTON = Y_OK_BUTTON + CY_BUTTON + CY_SPACE;
   static const int X_IMPORT_BUTTON = 160;
   static const int Y_IMPORT_BUTTON = Y_EXPORT_BUTTON;

   static const UINT CY_DIALOG = Y_IMPORT_BUTTON + CY_BUTTON + CY_SPACE;
   ASSERT(CY_DIALOG <= this->h);

	ASSERT(!this->bIsLoaded);

	this->pHoldBox = new CDialogWidget(0L, 0, 0, CX_DIALOG, CY_DIALOG, true);

   //Labels.
	this->pHoldBox->AddWidget(new CLabelWidget(0L, 80, 8, 140, 25,
					F_Message, g_pTheDB->GetMessageText(MID_SelectHoldPrompt)));

	this->pAuthorName = new CLabelWidget(0L, X_LABEL, Y_AUTHOR_LABEL, CX_LABEL,
         CY_AUTHOR_LABEL, F_Small, wszEmpty);
	this->pAuthorName->SetAlign(CLabelWidget::TA_CenterGroup);
	this->pHoldBox->AddWidget(this->pAuthorName);
	this->pHoldDesc = new CLabelWidget(0L, X_LABEL, Y_DESC_LABEL, CX_LABEL,
         CY_DESC_LABEL, F_Small, wszEmpty);
	this->pHoldDesc->SetAlign(CLabelWidget::TA_CenterGroup);
	this->pHoldBox->AddWidget(this->pHoldDesc);

   //Buttons.
	this->pOKButton = new CButtonWidget(
			TAG_OK, X_OK_BUTTON, Y_OK_BUTTON, CX_BUTTON, CY_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	this->pHoldBox->AddWidget(this->pOKButton);
	CButtonWidget *pCancelButton = new CButtonWidget(
			TAG_CANCEL, X_CANCEL_BUTTON, Y_CANCEL_BUTTON, CX_BUTTON, CY_BUTTON,
			g_pTheDB->GetMessageText(MID_Cancel));
	this->pHoldBox->AddWidget(pCancelButton);
	this->pDeleteButton = new CButtonWidget(TAG_DELETEHOLD,
         X_DELETEHOLD_BUTTON, Y_DELETE_BUTTON, CX_DELETEHOLD_BUTTON, CY_BUTTON,
         g_pTheDB->GetMessageText(MID_DeleteHold));
	this->pHoldBox->AddWidget(this->pDeleteButton);

	this->pExportButton = new CButtonWidget(
			TAG_EXPORT, X_EXPORT_BUTTON, Y_EXPORT_BUTTON, CX_BUTTON, CY_BUTTON,
			g_pTheDB->GetMessageText(MID_Export));
	this->pHoldBox->AddWidget(this->pExportButton);
	CButtonWidget *pImportButton = new CButtonWidget(
			TAG_IMPORT, X_IMPORT_BUTTON, Y_IMPORT_BUTTON, CX_BUTTON, CY_BUTTON,
			g_pTheDB->GetMessageText(MID_Import));
	this->pHoldBox->AddWidget(pImportButton);

   //Hold list.
	this->pHoldListBoxWidget = new CListBoxWidget(TAG_LIST_BOX,
			X_HOLDLISTBOX, Y_HOLDLISTBOX, CX_HOLDLISTBOX, CY_HOLDLISTBOX, true);
	this->pHoldBox->AddWidget(this->pHoldListBoxWidget);

   AddWidget(this->pHoldBox);
	this->pHoldBox->Center();

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//*****************************************************************************
void CHoldSelectScreen::Unload()
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
void CHoldSelectScreen::Paint(
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
bool CHoldSelectScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	PopulateHoldListBox();

	//Select current hold.
   const DWORD dwHoldID = g_pTheDB->GetHoldID();
	this->pHoldListBoxWidget->SelectItem(dwHoldID);

	ShowCursor();

	return true;
}

//*****************************************************************************
void CHoldSelectScreen::OnBetweenEvents()
//Bring up hold selection dialog right away.
//As soon as it closes, return from this screen.
{
	//Select hold.
	const DWORD dwCurrentHoldID = g_pTheDB->GetHoldID();
	DWORD dwHoldID = GetSelectedItem();
	GetHoldID(dwHoldID);

	if (dwCurrentHoldID != dwHoldID)
	{
		//Unload any game being played in another hold.
		CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
				g_pTheSM->GetLoadedScreen(SCR_Game));
		pGameScreen->UnloadGame();

		g_pTheDB->SetHoldID(dwHoldID);
	}

   if (GetDestScreenType() != SCR_None)
	   GoToScreen(SCR_Return);
}

//
// Private methods
//

//*****************************************************************************
void CHoldSelectScreen::GetHoldID(
//Params:
	DWORD &dwHoldID)	//(in/out)
{
   DWORD dwSelectedHoldID=dwHoldID;
   DWORD dwTagNo;
	do
	{
		if (this->pHoldListBoxWidget->GetItemCount() == 0)
		{
         ASSERT(!dwSelectedHoldID);
			this->pOKButton->Disable();
			this->pExportButton->Disable();
         this->pDeleteButton->Disable();
		} else {
			this->pOKButton->Enable();
         this->pDeleteButton->Enable();
         SetHoldDesc(dwSelectedHoldID);
		}

		dwTagNo = this->pHoldBox->Display();
      dwSelectedHoldID = GetSelectedItem();
		switch (dwTagNo)
		{
			case TAG_OK:
				dwHoldID = dwSelectedHoldID;
				break;

			case TAG_DELETEHOLD:
			   //Delete hold on confirmation.
			   if (ShowYesNoMessage(MID_DeleteHoldPrompt) == TAG_YES)
			   {
               const bool bDeletingActiveHold = dwHoldID == dwSelectedHoldID;
               SetCursor(CUR_Wait);
				   g_pTheDB->Holds.Delete(dwSelectedHoldID);
					PopulateHoldListBox();
					this->pHoldListBoxWidget->SelectItem(0);
				   dwSelectedHoldID = this->pHoldListBoxWidget->GetSelectedItem();
               if (bDeletingActiveHold)
                  dwHoldID = dwSelectedHoldID;
               SetHoldDesc(dwSelectedHoldID);
               SetCursor();
					Paint();
			   }
			   break;

			case TAG_EXPORT:
			{
				//Export the selected hold.
				if (dwSelectedHoldID)
				{
					CDbHold *pHold = g_pTheDB->Holds.GetByID(dwSelectedHoldID);
					if (pHold)
					{
						//Check whether rooms in hold are solvable.
						if (!pHold->VerifySolvability())
							if (ShowYesNoMessage(MID_SolvabilityUnknown) != TAG_YES)
                     {
                        delete pHold;
								break;
                     }

                 	//Default filename is hold name.
						WSTRING wstrExportFile = (WSTRING)pHold->NameText;
						if (ExportSelectFile(MID_SaveHoldPath, wstrExportFile, EXT_HOLD))
						{
							//Write the hold file.
							SetCursor(CUR_Wait);
							ShowStatusMessage(MID_Exporting);
							const bool bResult = CDbXML::ExportXML(ViewTypeStr(V_Holds),
									p_HoldID, dwSelectedHoldID, wstrExportFile.c_str());
							HideStatusMessage();
							SetCursor();
							ShowOkMessage(bResult ? MID_HoldFileSaved :
									MID_HoldFileNotSaved);
						}
						delete pHold;
					}
				}
			}
			break;

			case TAG_IMPORT:
			{
				//Import a hold data file.
            DWORD dwImportedHoldID;
				Import(EXT_HOLD, dwImportedHoldID);
				if (CDbXML::WasImportSuccessful() && dwImportedHoldID)
				{
               //Select imported hold.
               dwSelectedHoldID = dwImportedHoldID;
					PopulateHoldListBox();  //Update in case a hold was added.
					this->pHoldListBoxWidget->SelectItem(dwSelectedHoldID);
				   dwSelectedHoldID = this->pHoldListBoxWidget->GetSelectedItem();
               SetHoldDesc(dwSelectedHoldID);
					Paint();
				}
			}
			break;

         case TAG_CANCEL:
		   case TAG_ESCAPE:
		   case TAG_QUIT:
				return;
		}
	} while (dwTagNo != TAG_OK	|| !dwHoldID);
}

//*****************************************************************************
void CHoldSelectScreen::OnSelectChange(
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
         const DWORD dwHoldID = GetSelectedItem();
         SetHoldDesc(dwHoldID);
			Paint();
		}
		break;
	}
}

//*****************************************************************************
void CHoldSelectScreen::SetHoldDesc(
//Updates name/description fields of hold.
//Sets export permissions.
//
//Params:
	const DWORD dwHoldID)			//(in)	Hold.
{
	CDbHold* pHold = g_pTheDB->Holds.GetByID(dwHoldID);
	if (!pHold)
		this->pExportButton->Disable();
	else
   {
		const DWORD dwAuthorID = pHold->dwPlayerID;
		CDbPlayer *pPlayer = g_pTheDB->Players.GetByID(dwAuthorID);
		ASSERT(pPlayer);
		this->pHoldDesc->SetText(pHold->DescriptionText);
		this->pAuthorName->SetText(pPlayer->NameText);
		delete pPlayer;
		delete pHold;

		//Player can only publish hold they authored.
		if (dwAuthorID == g_pTheDB->GetPlayerID())
			this->pExportButton->Enable();
		else
			this->pExportButton->Disable();
      // Player cannot delete official holds
      if (dwHoldID < 100) {
         this->pDeleteButton->Disable();
      } else {
         this->pDeleteButton->Enable();
      }
	}
}

//*****************************************************************************
DWORD	CHoldSelectScreen::GetSelectedItem()
//Returns: tag of item selected in the list box widget.
{
	return this->pHoldListBoxWidget->GetSelectedItem();
}

//*****************************************************************************
void CHoldSelectScreen::PopulateHoldListBox() const
//Put the holds in the DB into the hold list box.
{
	this->pHoldListBoxWidget->Clear();

	BEGIN_DBREFCOUNT_CHECK;
	{
		CDbHold *pHold = g_pTheDB->Holds.GetFirst();
		while (pHold)
		{
         WSTRING wStr = static_cast<WSTRING>(pHold->NameText);
         //Add indicator if current player has beaten hold.
	      const DWORD dwEndHoldID = g_pTheDB->SavedGames.FindByEndHold(pHold->dwHoldID);
         if (dwEndHoldID)
         {
            wStr += wszSpace;
            wStr += wszLeftParen;
            wStr += wszAsterisk;
            wStr += wszRightParen;
         }
			this->pHoldListBoxWidget->AddItem(pHold->dwHoldID, wStr.c_str());
			delete pHold;
			pHold = g_pTheDB->Holds.GetNext();
		}
	}
	END_DBREFCOUNT_CHECK;
}

// $Log: HoldSelectScreen.cpp,v $
// Revision 1.40  2004/01/02 01:00:50  mrimer
// Alphabetized hold/player lists.
//
// Revision 1.39  2003/08/09 20:08:34  mrimer
// Fixed bug: cancelling import changes selected ID.
//
// Revision 1.38  2003/08/09 00:38:53  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.37  2003/08/06 01:34:16  mrimer
// Added indicator to hold name when it's been beaten.
//
// Revision 1.36  2003/07/31 15:46:54  schik
// No longer allows user to delete an official hold
//
// Revision 1.35  2003/07/25 23:24:13  mrimer
// Refined widget layout on dialog.  Enlarged hold list and description area.
//
// Revision 1.34  2003/07/24 04:03:41  mrimer
// Enabled double-clicking the list-box on the dialog to exit it with OK return value.
//
// Revision 1.33  2003/07/22 19:00:24  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.32  2003/07/19 21:23:57  mrimer
// Put initial focus on OK button.
//
// Revision 1.31  2003/07/19 02:23:10  mrimer
// Updated DB view access to use enum references rather than strings.
//
// Revision 1.30  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.29  2003/07/13 06:45:31  mrimer
// Removed unused parameter from CDbXML::Export() call.
//
// Revision 1.28  2003/07/12 22:16:02  mrimer
// Clicking X while on this screen now brings up quit dialog and closes app.
//
// Revision 1.27  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.26  2003/07/09 21:11:29  mrimer
// Revised Import API.
//
// Revision 1.25  2003/07/03 21:57:55  mrimer
// Enabled <Escape> to return from screen.
//
// Revision 1.24  2003/07/01 20:33:39  mrimer
// Added delete button member var.  Initialize member vars in constructor.  Disable delete hold button when not applicable.  Show hourglass cursor while deleting hold.
//
// Revision 1.23  2003/06/21 10:40:00  mrimer
// Fixed crash: delete active hold, then press cancel.
//
// Revision 1.22  2003/06/17 22:10:35  mrimer
// Fixed Delete hold button width.
//
// Revision 1.21  2003/06/17 18:27:52  mrimer
// Added delete hold button.
//
// Revision 1.20  2003/06/12 21:47:28  mrimer
// Set default export file name to something more intuitive.
//
// Revision 1.19  2003/06/12 18:07:41  mrimer
// Fixed display logic bug.
//
// Revision 1.18  2003/06/09 23:56:09  mrimer
// Fixed a memory leak.
//
// Revision 1.17  2003/05/29 19:14:31  mrimer
// Added an assertion.
//
// Revision 1.16  2003/05/28 23:11:52  erikh2000
// TA_* constants are used differently.
//
// Revision 1.15  2003/05/25 22:48:11  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.14  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.13  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.12  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.11  2003/04/29 11:15:33  mrimer
// Improved context-specific file selection.
//
// Revision 1.10  2003/04/28 22:58:28  mrimer
// Refactored export file selection code into CScreen::ExportSelectFile().
//
// Revision 1.9  2003/04/26 17:15:57  mrimer
// Fixed some bugs with file selection.
//
// Revision 1.8  2003/04/24 22:50:25  mrimer
// Added export confirmation when all rooms in hold are not proven solvable.
//
// Revision 1.7  2003/04/15 15:00:52  mrimer
// Set hourglass cursor during hold export.
//
// Revision 1.6  2003/04/08 13:08:28  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.5  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.4  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.3  2003/02/16 20:32:19  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.2  2003/01/04 23:09:13  mrimer
// Updated file export interface.
//
// Revision 1.1  2002/12/22 02:26:45  mrimer
// Initial check-in.
//
// Revision 1.1  2002/11/15 02:42:36  mrimer
// Initial check-in.
//
