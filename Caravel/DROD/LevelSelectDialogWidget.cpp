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
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "LevelSelectDialogWidget.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>
#include <FrontEndLib/OptionButtonWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/DbLevels.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Wchar.h>

//NOTE: tag #'s should not conflict with other widgets on screen
const DWORD TAG_LEVELS_LISTBOX = 997;
const DWORD TAG_ALL_LEVELS_OPTIONBOX = 998;
const DWORD TAG_CANCEL = 999;

//*****************************************************************************
CLevelSelectDialogWidget::CLevelSelectDialogWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	const int nSetX, const int nSetY)			//		constructor.
	: CDialogWidget(dwSetTagNo, nSetX, nSetY, 330, 235, true)   //double-click on list box disables
	, pLevelListBoxWidget(NULL)
	, pSourceLevel(NULL)
	, pPromptLabel(NULL)
{
}

//*****************************************************************************
void CLevelSelectDialogWidget::EnableCancelButton(const bool bFlag)
{
   CButtonWidget *pCancelButton = static_cast<CButtonWidget*>(GetWidget(TAG_CANCEL));
   if (bFlag)
      pCancelButton->Enable();
   else
      pCancelButton->Disable();
}

//*****************************************************************************
bool CLevelSelectDialogWidget::Load()
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	this->pPromptLabel = new CLabelWidget(0L, 25, 10, 290, 30, F_Small, wszEmpty);
	AddWidget(this->pPromptLabel);

   //OK button gets default focus
	CButtonWidget *pOKButton = new CButtonWidget(
			TAG_OK, 100, 205, 60, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	AddWidget(pOKButton);
	CButtonWidget *pCancelButton = new CButtonWidget(
			TAG_CANCEL, 170, 205, 60, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Cancel));
	AddWidget(pCancelButton);

	this->pLevelListBoxWidget = new CListBoxWidget(TAG_LEVELS_LISTBOX,
			15, 45, 300, 130);
	AddWidget(this->pLevelListBoxWidget);

//We're disallowing exits to point to levels in other holds for this release.
#ifdef AFTERVERSION1_6
   COptionButtonWidget *pAllLevelsOptionButton = new COptionButtonWidget(
			TAG_ALL_LEVELS_OPTIONBOX, 20, 180, 300, CY_STANDARD_OPTIONBUTTON,
			g_pTheDB->GetMessageText(MID_ShowAllLevels), false);
	AddWidget(pAllLevelsOptionButton);
#endif

   return CWidget::Load();
}

//*****************************************************************************
void CLevelSelectDialogWidget::OnClick(
//Handles click event.
//
//Params:
	const DWORD dwTagNo)				//(in)	Widget that received event.
{
	CDialogWidget::OnClick(dwTagNo);

	switch (dwTagNo)
	{
		case TAG_ALL_LEVELS_OPTIONBOX:
		{
			//Update level list box.
			const DWORD dwLevelID = GetSelectedItem();
			PopulateLevelList();
			SelectItem(dwLevelID);
			Paint();
		}
		break;
	}
}

//*****************************************************************************
void CLevelSelectDialogWidget::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD dwTagNo,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	CDialogWidget::OnKeyDown(dwTagNo, Key);

	switch (dwTagNo)
	{
		case TAG_ALL_LEVELS_OPTIONBOX:
		{
			//Update level list box.
			const DWORD dwLevelID = GetSelectedItem();
			PopulateLevelList();
			SelectItem(dwLevelID);
			Paint();
		}
		break;
	}
}

//*****************************************************************************
void CLevelSelectDialogWidget::PopulateLevelList()
//Populate list box with all active levels.
//List can be toggled between levels in this hold and all levels.
{
//!!
#ifdef AFTERVERSION1_6
	const bool bShowAllLevels = DYN_CAST(COptionButtonWidget*, CWidget*, 
         GetWidget(TAG_ALL_LEVELS_OPTIONBOX))->IsChecked();
#else
   const bool bShowAllLevels = false;
#endif

	this->pLevelListBoxWidget->Clear();

	BEGIN_DBREFCOUNT_CHECK;
	{
		//Default choice.
		this->pLevelListBoxWidget->AddItem(0L, g_pTheDB->GetMessageText(MID_DefaultExit));

		CDbHold *pHold;
		CDbLevel *pLevel;
		if (bShowAllLevels)
		{
			//Get all levels in DB.  Sort by hold.
         CDb db;
			pHold = db.Holds.GetFirst();
			while (pHold)
			{
            //Allow selecting levels from hold only if player has access rights.
            if (db.Holds.PlayerCanEditHold(pHold->dwHoldID))
            {
				   static const WCHAR wszHoldSep[] = {W_t(' '),W_t(':'),W_t(' '),W_t(0)};
				   WSTRING wHoldName = (WSTRING)pHold->NameText;
				   wHoldName += wszHoldSep;
				   db.Levels.FilterBy(pHold->dwHoldID);
				   pLevel = db.Levels.GetFirst();
				   while (pLevel)
				   {
					   ASSERT(pLevel->dwLevelID);

					   WSTRING wText = wHoldName;
					   wText += pLevel->NameText;
					   pLevelListBoxWidget->AddItem(pLevel->dwLevelID, wText.c_str());
					   delete pLevel;
					   pLevel = db.Levels.GetNext();
				   }
            }
				delete pHold;
				pHold = db.Holds.GetNext();
			}
		} else {
			//Get the levels in the current level's hold.
			if (!this->pSourceLevel) return;

			CDbHold *pHold = g_pTheDB->Holds.GetByID(this->pSourceLevel->dwHoldID);
			pLevel = pHold->Levels.GetFirst();
			while (pLevel)
			{
				ASSERT(pLevel->dwLevelID);
				pLevelListBoxWidget->AddItem(pLevel->dwLevelID, pLevel->NameText);
				delete pLevel;
				pLevel = pHold->Levels.GetNext();
			}
			delete pHold;
		}
	}
	END_DBREFCOUNT_CHECK;
}

//*****************************************************************************
DWORD CLevelSelectDialogWidget::GetSelectedItem() const
{
	return this->pLevelListBoxWidget->GetSelectedItem();
}

//*****************************************************************************
void CLevelSelectDialogWidget::SelectItem(
	const DWORD dwTagNo)	//(in)
{
	this->pLevelListBoxWidget->SelectItem(dwTagNo);
}


//*****************************************************************************
void CLevelSelectDialogWidget::SetPrompt(
	const MESSAGE_ID messageID)
{
	this->pPromptLabel->SetText(g_pTheDB->GetMessageText(messageID));
}

//*****************************************************************************
void CLevelSelectDialogWidget::SetSourceLevel(
	CDbLevel *pLevel)	//(in)
{
	this->pSourceLevel = pLevel;
}

// $Log: LevelSelectDialogWidget.cpp,v $
// Revision 1.18  2003/07/31 21:33:42  mrimer
// Placed default focus on OK.
//
// Revision 1.17  2003/07/24 04:03:41  mrimer
// Enabled double-clicking the list-box on the dialog to exit it with OK return value.
//
// Revision 1.16  2003/07/19 02:19:20  mrimer
// Disabled level exits to other holds until next release.
//
// Revision 1.15  2003/07/16 07:43:25  mrimer
// Revised selecting stair destination UI.
//
// Revision 1.14  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.13  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.12  2003/06/30 19:32:45  mrimer
// Now only showing levels on holds to which current player has access rights.
//
// Revision 1.11  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.9  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.8  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.7  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.6  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
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
// Revision 1.2  2003/01/08 00:56:37  mrimer
// Prepended hold name to level name when all levels are shown.
//
// Revision 1.1  2002/12/22 02:26:45  mrimer
// Initial check-in.
//
