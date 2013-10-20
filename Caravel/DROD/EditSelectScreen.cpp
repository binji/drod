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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "EditSelectScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodScreenManager.h"
#include "DrodSound.h"
#include "Browser.h"
#include "EditRoomScreen.h"
#include "GameScreen.h"

#include "MapWidget.h"
#include "EditRoomWidget.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/FrameWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>
#include <FrontEndLib/OptionButtonWidget.h>
#include <FrontEndLib/ScalerWidget.h>
#include <FrontEndLib/TextBoxWidget.h>

#include "../DRODLib/CueEvents.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbHolds.h"
#include "../DRODLib/DbLevels.h"
#include "../DRODLib/DBProps.h"
#include "../DRODLib/DbRooms.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

//Widget tags.
const DWORD TAG_NEW_LEVEL = 1013;
const DWORD TAG_NEW_HOLD = 1014;
const DWORD TAG_RENAME_HOLD = 1015;
const DWORD TAG_RENAME_LEVEL = 1016;
const DWORD TAG_REDESC_HOLD = 1017;
const DWORD TAG_REDESC_LEVEL = 1018;
const DWORD TAG_ENDING_MESSAGE = 1019;

const DWORD TAG_LEVEL_AUTHOR_LABEL = 1020;
const DWORD TAG_LEVEL_DATE_LABEL = 1021;
const DWORD TAG_POSITION_LABEL = 1022;
const DWORD TAG_ROOM_IS_REQUIRED = 1023;
const DWORD TAG_WHO_CAN_EDIT_LBOX = 1024;

const DWORD TAG_HOLD_LBOX = 1030;
const DWORD TAG_LEVEL_LBOX = 1031;
const DWORD TAG_STYLE_LBOX = 1032;

const DWORD TAG_COPY_HOLD = 1033;
const DWORD TAG_DELETE_HOLD = 1034;
const DWORD TAG_FIRST_LEVEL = 1035;
const DWORD TAG_DELETE_LEVEL = 1036;

const DWORD TAG_MINIROOM = 1040;

const DWORD TAG_EDIT = 1091;
const DWORD TAG_CANCEL = 1092;
const DWORD TAG_HELP = 1093;

//
//Public methods.
//

//*****************************************************************************
void CEditSelectScreen::SetToCopiedHold(
//Updates widgets to point to new copy of hold made in the Edit Room screen.
//
//Params:
   CDbHold *pHold, CDbLevel *pLevel)
{
   delete this->pSelectedHold;
   this->pSelectedHold = g_pTheDB->Holds.GetByID(pHold->dwHoldID);
   delete this->pSelectedLevel;
   this->pSelectedLevel = g_pTheDB->Levels.GetByID(pLevel->dwLevelID);

	PopulateHoldListBox();
	PopulateLevelListBox();
   this->pHoldListBoxWidget->SelectItem(pHold->dwHoldID);

	//Update map.
	DWORD dwX, dwY;
	this->pSelectedLevel->GetStartingRoomCoords(dwX,dwY);
	this->pMapWidget->LoadFromLevel(this->pSelectedLevel);
}

//
//Protected methods.
//

//*****************************************************************************
CEditSelectScreen::CEditSelectScreen()
	: CDrodScreen(SCR_EditSelect)
	, pLevelListBoxWidget(NULL), pHoldListBoxWidget(NULL)
	, pStyleListBoxWidget(NULL)
	, pSelectedHold(NULL), pSelectedLevel(NULL), pSelectedRoom(NULL)
	, pLevelCopy(NULL)
	, bCopyingLevel(false)
	, pMapWidget(NULL)
	, pRoomWidget(NULL)
	, pScaledRoomWidget(NULL)
	, pBackgroundSurface(NULL)
//Constructor.
{
	SetKeyRepeat(66);
}

//*****************************************************************************
bool CEditSelectScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	
	//Load background graphic.
	ASSERT(!this->pBackgroundSurface);
	this->pBackgroundSurface = g_pTheBM->GetBitmapSurface("Background");
	if (!this->pBackgroundSurface) return false;

	//Title bar
	static const UINT CX_SPACE = 8;
	static const UINT CY_SPACE = 8;
	static const UINT CX_TITLE = 100;
	static const UINT CY_TITLE = 32;
	static const UINT CY_TITLE_SPACE = 8;
	static const int X_TITLE = (this->w - CX_TITLE) / 2;
	static const int Y_TITLE = CY_TITLE_SPACE;

	//Buttons
	static const UINT CX_EDIT_BUTTON = 70;
	static const UINT CY_EDIT_BUTTON = CY_STANDARD_BUTTON;
	static const UINT CX_CANCEL_BUTTON = 110;
	static const UINT CY_CANCEL_BUTTON = CY_STANDARD_BUTTON;
	static const UINT CX_HELP_BUTTON = CX_EDIT_BUTTON;
	static const UINT CY_HELP_BUTTON = CY_STANDARD_BUTTON;
	static const int X_HELP_BUTTON = this->w - CX_HELP_BUTTON - CX_SPACE;
	static const int X_CANCEL_BUTTON = X_HELP_BUTTON - CX_CANCEL_BUTTON - CX_SPACE;
	static const int X_EDIT_BUTTON = X_CANCEL_BUTTON - CX_EDIT_BUTTON - CX_SPACE;
	static const int Y_EDIT_BUTTON = this->h - CY_SPACE - CY_EDIT_BUTTON;
	static const int Y_CANCEL_BUTTON = Y_EDIT_BUTTON;
	static const int Y_HELP_BUTTON = Y_EDIT_BUTTON;

	//Mini-room widget has strict proportions and its dimensions will define 
	//placement of most everything else.
	static const UINT CY_MINIROOM = CDrodBitmapManager::CY_ROOM / 2;
	//Width of mini-room must be proportional to regular room display.
	static const UINT CX_MINIROOM = CY_MINIROOM * CDrodBitmapManager::CX_ROOM /
			CDrodBitmapManager::CY_ROOM;
	static const int Y_MINIROOM = Y_EDIT_BUTTON - CY_SPACE - CY_MINIROOM;
	static const int X_MINIROOM = this->w - CX_SPACE - CX_MINIROOM;
	static const UINT CX_ROOM_REQUIRED_BUTTON = 107;
	static const UINT CX_POSITION_LABEL = CX_MINIROOM - CX_ROOM_REQUIRED_BUTTON;
	static const UINT CY_ROOM_REQUIRED_BUTTON = CY_STANDARD_OPTIONBUTTON;
	static const UINT CY_POSITION_LABEL = CY_ROOM_REQUIRED_BUTTON;
	static const int X_POSITION_LABEL = X_MINIROOM;
	static const int Y_POSITION_LABEL = Y_MINIROOM - CY_POSITION_LABEL;
	static const int X_ROOM_REQUIRED_BUTTON = X_POSITION_LABEL + CX_POSITION_LABEL;
	static const int Y_ROOM_REQUIRED_BUTTON = Y_POSITION_LABEL;

	//Map
	static const UINT CX_MAP = this->w - CX_SPACE - CX_MINIROOM - CX_SPACE * 3;
	static const UINT CY_MAP = CY_MINIROOM - CY_SPACE;
	static const int X_MAP = CX_SPACE;
	static const int Y_MAP = this->h- CY_MAP - CY_SPACE;

	//Hold selection.
	static const int X_CHOOSE_HOLD_LABEL = CX_SPACE;
	static const int Y_CHOOSE_HOLD_LABEL = Y_TITLE + CY_TITLE + CY_TITLE_SPACE/2;
	static const UINT CX_CHOOSE_HOLD_LABEL = 80;
	static const UINT CY_CHOOSE_HOLD_LABEL = CY_STANDARD_BUTTON;
	static const int X_NEW_HOLD = X_CHOOSE_HOLD_LABEL + CX_CHOOSE_HOLD_LABEL + CX_SPACE;
	static const int Y_NEW_HOLD = Y_CHOOSE_HOLD_LABEL - 2;
	static const UINT CX_NEW_HOLD = 70;
	static const UINT CY_NEW_HOLD = CY_STANDARD_BUTTON;
	static const int X_HOLD_LBOX = X_CHOOSE_HOLD_LABEL;
	static const int Y_HOLD_LBOX = Y_CHOOSE_HOLD_LABEL + CY_CHOOSE_HOLD_LABEL + 1;
	static const UINT CX_HOLD_LBOX = X_NEW_HOLD + CX_NEW_HOLD - X_HOLD_LBOX + 1;
	static const UINT CY_HOLD_LBOX = 128;

	//Level selection.
	static const int X_CHOOSE_LEVEL_LABEL = X_NEW_HOLD + CX_NEW_HOLD + CX_SPACE * 2;
	static const int Y_CHOOSE_LEVEL_LABEL = Y_CHOOSE_HOLD_LABEL;
	static const UINT CX_CHOOSE_LEVEL_LABEL = 85;
	static const UINT CY_CHOOSE_LEVEL_LABEL = CY_STANDARD_BUTTON;
	static const UINT CX_NEW_LEVEL = CX_NEW_HOLD;
	static const UINT CY_NEW_LEVEL = CY_STANDARD_BUTTON;
	static const int X_NEW_LEVEL = X_CHOOSE_LEVEL_LABEL + CX_CHOOSE_LEVEL_LABEL + CX_SPACE;
	static const int Y_NEW_LEVEL = Y_CHOOSE_LEVEL_LABEL - 2;
	static const int X_LEVEL_LBOX = X_CHOOSE_LEVEL_LABEL;
	static const int Y_LEVEL_LBOX = Y_CHOOSE_LEVEL_LABEL + CY_CHOOSE_LEVEL_LABEL + 1;
	static const UINT CX_LEVEL_LBOX = X_NEW_LEVEL + CX_NEW_LEVEL - X_LEVEL_LBOX + 1;
	static const UINT CY_LEVEL_LBOX = CY_HOLD_LBOX;

	//Style selection.
	static const UINT CX_CHOOSE_STYLE_LABEL = 85;
	static const int X_CHOOSE_STYLE_LABEL = this->w - CX_CHOOSE_STYLE_LABEL - CX_SPACE;
	static const int Y_CHOOSE_STYLE_LABEL = Y_CHOOSE_HOLD_LABEL;
	static const UINT CY_CHOOSE_STYLE_LABEL = CY_STANDARD_BUTTON;
	static const int X_STYLE_LBOX = X_CHOOSE_STYLE_LABEL;
	static const int Y_STYLE_LBOX = Y_CHOOSE_STYLE_LABEL + CY_CHOOSE_STYLE_LABEL + 1;
	static const UINT CX_STYLE_LBOX = X_CHOOSE_STYLE_LABEL + CX_CHOOSE_STYLE_LABEL - X_STYLE_LBOX + 1;
	static const UINT CY_STYLE_LBOX = CY_HOLD_LBOX;

	//Hold settings.
	static const UINT CX_RENAME_HOLD = 70;
	static const int X_RENAME_HOLD = X_CHOOSE_HOLD_LABEL + 5;
	static const int Y_RENAME_HOLD = Y_HOLD_LBOX + CY_HOLD_LBOX + 5;
	static const UINT CY_RENAME_HOLD = CY_STANDARD_BUTTON;
	static const UINT CX_REDESC_HOLD = CX_RENAME_HOLD;
	static const int X_REDESC_HOLD = X_RENAME_HOLD + CX_RENAME_HOLD + CX_SPACE;
	static const int Y_REDESC_HOLD = Y_RENAME_HOLD;
	static const UINT CY_REDESC_HOLD = CY_STANDARD_BUTTON;

	static const int X_COPY_HOLD = X_CHOOSE_HOLD_LABEL;
	static const int Y_COPY_HOLD = Y_RENAME_HOLD + CY_RENAME_HOLD + 5;
	static const UINT CX_COPY_HOLD = 50;
	static const UINT CY_COPY_HOLD = CY_STANDARD_BUTTON;
	static const int X_DELETE_HOLD = X_COPY_HOLD + CX_COPY_HOLD + 5;
	static const int Y_DELETE_HOLD = Y_COPY_HOLD;
	static const UINT CX_DELETE_HOLD = CX_COPY_HOLD;
	static const UINT CY_DELETE_HOLD = CY_STANDARD_BUTTON;
	static const int X_ENDING_MESSAGE = X_DELETE_HOLD + CX_DELETE_HOLD + 5;
	static const int Y_ENDING_MESSAGE = Y_COPY_HOLD;
	static const UINT CX_ENDING_MESSAGE = CX_COPY_HOLD;
	static const UINT CY_ENDING_MESSAGE = CY_STANDARD_BUTTON;

   //Hold settings (Editing privileges).
	static const int X_HOLD_FRAME = X_NEW_LEVEL + CX_NEW_LEVEL + CX_SPACE * 2;
	static const int Y_HOLD_FRAME = Y_CHOOSE_HOLD_LABEL + CY_SPACE;
	static const int CX_HOLD_FRAME = X_CHOOSE_STYLE_LABEL - X_HOLD_FRAME - CX_SPACE;

	static const int X_WHO_CAN_EDIT = CX_SPACE;
	static const UINT CX_WHO_CAN_EDIT = CX_HOLD_FRAME - 2 * X_WHO_CAN_EDIT;
	static const UINT CY_WHO_CAN_EDIT = 20;
	static const int Y_WHO_CAN_EDIT = CY_SPACE/2;
	static const int X_WHO_CAN_EDIT_LBOX = X_WHO_CAN_EDIT;
	static const int Y_WHO_CAN_EDIT_LBOX = Y_WHO_CAN_EDIT + CY_WHO_CAN_EDIT;
	static const UINT CX_WHO_CAN_EDIT_LBOX = X_WHO_CAN_EDIT + CX_WHO_CAN_EDIT - X_WHO_CAN_EDIT_LBOX + 1;
	static const UINT CY_WHO_CAN_EDIT_LBOX = 44;

   static const int CY_HOLD_FRAME = Y_WHO_CAN_EDIT_LBOX + CY_WHO_CAN_EDIT_LBOX + CY_SPACE;

	//Level settings.
	static const UINT CX_RENAME_LEVEL = 70;
	static const int X_RENAME_LEVEL = X_CHOOSE_LEVEL_LABEL + 7;
	static const int Y_RENAME_LEVEL = Y_LEVEL_LBOX + CY_LEVEL_LBOX + 5;
	static const UINT CY_RENAME_LEVEL = CY_STANDARD_BUTTON;
	static const UINT CX_REDESC_LEVEL = CX_RENAME_LEVEL;
	static const int X_REDESC_LEVEL = X_RENAME_LEVEL + CX_RENAME_LEVEL + CX_SPACE;
	static const int Y_REDESC_LEVEL = Y_RENAME_LEVEL;
	static const UINT CY_REDESC_LEVEL = CY_STANDARD_BUTTON;
	static const int X_FIRST_LEVEL = X_RENAME_LEVEL;
	static const int Y_FIRST_LEVEL = Y_RENAME_LEVEL + CY_RENAME_LEVEL + 5;
	static const UINT CX_FIRST_LEVEL = CX_RENAME_LEVEL;
	static const UINT CY_FIRST_LEVEL = CY_STANDARD_BUTTON;
	static const int X_DELETE_LEVEL = X_REDESC_LEVEL;
	static const int Y_DELETE_LEVEL = Y_FIRST_LEVEL;
	static const UINT CX_DELETE_LEVEL = CX_RENAME_LEVEL;
	static const UINT CY_DELETE_LEVEL = CY_STANDARD_BUTTON;

	static const int X_LEVEL_FRAME = X_NEW_LEVEL + CX_NEW_LEVEL + CX_SPACE * 2;
	static const int Y_LEVEL_FRAME = Y_HOLD_FRAME + CY_HOLD_FRAME + CY_SPACE;
	static const int CX_LEVEL_FRAME = X_CHOOSE_STYLE_LABEL - X_LEVEL_FRAME - CX_SPACE;
	static const int CY_LEVEL_FRAME = Y_POSITION_LABEL - Y_LEVEL_FRAME;
	static const int X_LEVEL_AUTHOR_LABEL = CX_SPACE;
	static const int Y_LEVEL_AUTHOR_LABEL = CY_SPACE;
	static const UINT CX_LEVEL_AUTHOR_LABEL = 40;
	static const UINT CY_LEVEL_AUTHOR_LABEL = 20;
	static const int X_LEVEL_DATE_LABEL = X_LEVEL_AUTHOR_LABEL;
	static const int Y_LEVEL_DATE_LABEL = Y_LEVEL_AUTHOR_LABEL + CY_LEVEL_AUTHOR_LABEL;
	static const UINT CX_LEVEL_DATE_LABEL = CX_LEVEL_AUTHOR_LABEL;
	static const UINT CY_LEVEL_DATE_LABEL = CY_LEVEL_AUTHOR_LABEL;

	static const int X_LEVEL_AUTHOR = X_LEVEL_AUTHOR_LABEL + CX_LEVEL_AUTHOR_LABEL + CX_SPACE;
	static const int Y_LEVEL_AUTHOR = Y_LEVEL_AUTHOR_LABEL;
	static const UINT CX_LEVEL_AUTHOR = X_CHOOSE_STYLE_LABEL - X_LEVEL_AUTHOR - CX_SPACE;
	static const UINT CY_LEVEL_AUTHOR = CY_LEVEL_AUTHOR_LABEL + 2;
	static const int X_LEVEL_DATE = X_LEVEL_AUTHOR;
	static const int Y_LEVEL_DATE = Y_LEVEL_DATE_LABEL;
	static const UINT CX_LEVEL_DATE = CX_LEVEL_AUTHOR;
	static const UINT CY_LEVEL_DATE = CY_LEVEL_DATE_LABEL + 2;

	CButtonWidget *pButton;

	//Title.
	AddWidget(new CLabelWidget(0L, X_TITLE, Y_TITLE,
			CX_TITLE, CY_TITLE, F_Title, g_pTheDB->GetMessageText(MID_Editor)));

	//Hold selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_HOLD_LABEL, Y_CHOOSE_HOLD_LABEL,
				CX_CHOOSE_HOLD_LABEL, CY_CHOOSE_HOLD_LABEL, F_Header,
				g_pTheDB->GetMessageText(MID_ChooseHold)));
	pButton = new CButtonWidget(TAG_NEW_HOLD, X_NEW_HOLD, Y_NEW_HOLD,
				CX_NEW_HOLD, CY_NEW_HOLD, g_pTheDB->GetMessageText(MID_NewHold));
	AddWidget(pButton);
	this->pHoldListBoxWidget = new CListBoxWidget(TAG_HOLD_LBOX,
			X_HOLD_LBOX, Y_HOLD_LBOX, CX_HOLD_LBOX, CY_HOLD_LBOX, true);
	AddWidget(this->pHoldListBoxWidget);

	//Hold selection area.
	pButton = new CButtonWidget(TAG_RENAME_HOLD, X_RENAME_HOLD, Y_RENAME_HOLD,
				CX_RENAME_HOLD, CY_RENAME_HOLD, g_pTheDB->GetMessageText(MID_Rename));
	pButton->Disable();
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_REDESC_HOLD, X_REDESC_HOLD, Y_REDESC_HOLD,
				CX_REDESC_HOLD, CY_REDESC_HOLD, g_pTheDB->GetMessageText(MID_Describe));
	pButton->Disable();
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_COPY_HOLD, X_COPY_HOLD, Y_COPY_HOLD,
				CX_COPY_HOLD, CY_COPY_HOLD, g_pTheDB->GetMessageText(MID_Copy));
	pButton->Disable();
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_DELETE_HOLD, X_DELETE_HOLD, Y_DELETE_HOLD,
				CX_DELETE_HOLD, CY_DELETE_HOLD, g_pTheDB->GetMessageText(MID_DeleteNoHotkey));
	pButton->Disable();
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_ENDING_MESSAGE, X_ENDING_MESSAGE, Y_ENDING_MESSAGE,
				CX_ENDING_MESSAGE, CY_ENDING_MESSAGE, g_pTheDB->GetMessageText(MID_EndHoldMessage));
	pButton->Disable();
	AddWidget(pButton);

	//Level selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_LEVEL_LABEL, Y_CHOOSE_LEVEL_LABEL,
				CX_CHOOSE_LEVEL_LABEL, CY_CHOOSE_LEVEL_LABEL, F_Header,
				g_pTheDB->GetMessageText(MID_ChooseLevel)));
	pButton = new CButtonWidget(TAG_NEW_LEVEL, X_NEW_LEVEL, Y_NEW_LEVEL,
				CX_NEW_LEVEL, CY_NEW_LEVEL, g_pTheDB->GetMessageText(MID_NewLevel));
	pButton->Disable();
	AddWidget(pButton);
	this->pLevelListBoxWidget = new CListBoxWidget(TAG_LEVEL_LBOX,
			X_LEVEL_LBOX, Y_LEVEL_LBOX, CX_LEVEL_LBOX, CY_LEVEL_LBOX);
	AddWidget(this->pLevelListBoxWidget);

	pButton = new CButtonWidget(TAG_RENAME_LEVEL, X_RENAME_LEVEL, Y_RENAME_LEVEL, 
				CX_RENAME_LEVEL, CY_RENAME_LEVEL, g_pTheDB->GetMessageText(MID_Rename));
	pButton->Disable();
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_REDESC_LEVEL, X_REDESC_LEVEL, Y_REDESC_LEVEL,
				CX_REDESC_LEVEL, CY_REDESC_LEVEL, g_pTheDB->GetMessageText(MID_Describe));
	pButton->Disable();
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_FIRST_LEVEL, X_FIRST_LEVEL, Y_FIRST_LEVEL,
				CX_FIRST_LEVEL, CY_FIRST_LEVEL, g_pTheDB->GetMessageText(MID_MakeFirstLevel));
	pButton->Disable();
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_DELETE_LEVEL, X_DELETE_LEVEL, Y_DELETE_LEVEL,
				CX_DELETE_LEVEL, CY_DELETE_LEVEL, g_pTheDB->GetMessageText(MID_DeleteNoHotkey));
	pButton->Disable();
	AddWidget(pButton);

	//Hold settings area.
	CFrameWidget *pHoldSettingsFrame = new CFrameWidget(0L, X_HOLD_FRAME,
			Y_HOLD_FRAME, CX_HOLD_FRAME, CY_HOLD_FRAME,
			g_pTheDB->GetMessageText(MID_HoldSettings));
	AddWidget(pHoldSettingsFrame);

	pHoldSettingsFrame->AddWidget(new CLabelWidget(0L,
			X_WHO_CAN_EDIT, Y_WHO_CAN_EDIT, CX_WHO_CAN_EDIT, CY_WHO_CAN_EDIT,
         F_Small, g_pTheDB->GetMessageText(MID_WhoCanEdit)));
   CListBoxWidget *pWhoCanEditListBox = new CListBoxWidget(TAG_WHO_CAN_EDIT_LBOX,
			X_WHO_CAN_EDIT_LBOX, Y_WHO_CAN_EDIT_LBOX, CX_WHO_CAN_EDIT_LBOX,
         CY_WHO_CAN_EDIT_LBOX);
	pHoldSettingsFrame->AddWidget(pWhoCanEditListBox);
   pWhoCanEditListBox->AddItem(CDbHold::Anyone, g_pTheDB->GetMessageText(MID_Anyone));
	pWhoCanEditListBox->AddItem(CDbHold::YouAndConquerors, g_pTheDB->GetMessageText(MID_YouAndConquerors));
	pWhoCanEditListBox->AddItem(CDbHold::OnlyYou, g_pTheDB->GetMessageText(MID_OnlyYou));

	//Level settings area.
   CFrameWidget *pLevelSettingsFrame = new CFrameWidget(0L, X_LEVEL_FRAME,
			Y_LEVEL_FRAME, CX_LEVEL_FRAME, CY_LEVEL_FRAME,
			g_pTheDB->GetMessageText(MID_LevelSettings));
	AddWidget(pLevelSettingsFrame);

	pLevelSettingsFrame->AddWidget(new CLabelWidget(0L, X_LEVEL_AUTHOR_LABEL,
			Y_LEVEL_AUTHOR_LABEL, CX_LEVEL_AUTHOR_LABEL, CY_LEVEL_AUTHOR_LABEL,
			F_Small, g_pTheDB->GetMessageText(MID_LevelBy)));
	pLevelSettingsFrame->AddWidget(new CLabelWidget(0L, X_LEVEL_DATE_LABEL,
			Y_LEVEL_DATE_LABEL, CX_LEVEL_DATE_LABEL, CY_LEVEL_DATE_LABEL, F_Small,
			g_pTheDB->GetMessageText(MID_LevelCreated)));
	pLevelSettingsFrame->AddWidget(new CLabelWidget(TAG_LEVEL_AUTHOR_LABEL,
			X_LEVEL_AUTHOR, Y_LEVEL_AUTHOR, CX_LEVEL_AUTHOR, CY_LEVEL_AUTHOR,
			F_Small, wszEmpty));
	pLevelSettingsFrame->AddWidget(new CLabelWidget(TAG_LEVEL_DATE_LABEL,
			X_LEVEL_DATE, Y_LEVEL_DATE, CX_LEVEL_DATE, CY_LEVEL_DATE, F_Small,
			wszEmpty));

	//Room style selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_STYLE_LABEL, Y_CHOOSE_STYLE_LABEL,
				CX_CHOOSE_STYLE_LABEL, CY_CHOOSE_STYLE_LABEL, F_Header,
				g_pTheDB->GetMessageText(MID_RoomStyle)));
	this->pStyleListBoxWidget = new CListBoxWidget(TAG_STYLE_LBOX,
			X_STYLE_LBOX, Y_STYLE_LBOX, CX_STYLE_LBOX, CY_STYLE_LBOX);
	AddWidget(this->pStyleListBoxWidget);

	//Room selection area.
	this->pMapWidget = new CMapWidget(TAG_MAP, X_MAP, Y_MAP, CX_MAP, CY_MAP, NULL);
	AddWidget(this->pMapWidget);

	//Room view.
	this->pScaledRoomWidget = new CScalerWidget(TAG_MINIROOM, X_MINIROOM, Y_MINIROOM, 
			CX_MINIROOM, CY_MINIROOM);
	AddWidget(this->pScaledRoomWidget);
	this->pRoomWidget = new CEditRoomWidget(0L, 0, 0, CDrodBitmapManager::CX_ROOM,
			CDrodBitmapManager::CY_ROOM);
	this->pScaledRoomWidget->AddScaledWidget(this->pRoomWidget);
	AddWidget(new CLabelWidget(TAG_POSITION_LABEL, X_POSITION_LABEL, Y_POSITION_LABEL, 
				CX_POSITION_LABEL, CY_POSITION_LABEL, F_Small, wszEmpty));
	AddWidget(new COptionButtonWidget(TAG_ROOM_IS_REQUIRED,
			X_ROOM_REQUIRED_BUTTON, Y_ROOM_REQUIRED_BUTTON, CX_ROOM_REQUIRED_BUTTON,
			CY_ROOM_REQUIRED_BUTTON, g_pTheDB->GetMessageText(MID_RoomIsRequired), false));

	//Edit, cancel and help buttons.
	pButton = new CButtonWidget(TAG_EDIT, X_EDIT_BUTTON, Y_EDIT_BUTTON,
				CX_EDIT_BUTTON, CY_EDIT_BUTTON, g_pTheDB->GetMessageText(MID_EditRoom));
	pButton->Disable();
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_CANCEL, X_CANCEL_BUTTON, Y_CANCEL_BUTTON, 
				CX_CANCEL_BUTTON, CY_CANCEL_BUTTON, g_pTheDB->GetMessageText(MID_ReturnToGame));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_HELP, X_HELP_BUTTON, Y_HELP_BUTTON, 
				CX_HELP_BUTTON, CY_HELP_BUTTON, g_pTheDB->GetMessageText(MID_Help));
	AddWidget(pButton);
	AddHotkey(SDLK_F1,TAG_HELP);

	//Level list dialog box.
	this->pLevelBox = new CLevelSelectDialogWidget(0L);
	AddWidget(this->pLevelBox);
	this->pLevelBox->Center();
	this->pLevelBox->Hide();

	//Load children widgets.
	this->bIsLoaded = LoadChildren();

	//Prepare style selection list box once.
	PopulateStyleListBox();

	return this->bIsLoaded;
}

//*****************************************************************************
void CEditSelectScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pBackgroundSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Background");
		this->pBackgroundSurface = NULL;
	}

   FreeMembers();

	this->bIsLoaded = false;
}

//*****************************************************************************
void CEditSelectScreen::FreeMembers()
//Release DB members.
{
	delete this->pSelectedHold;
	this->pSelectedHold = NULL;
	delete this->pSelectedLevel;
	this->pSelectedLevel = NULL;
	delete this->pSelectedRoom;
	this->pSelectedRoom = NULL;

   delete this->pLevelCopy;
	this->pLevelCopy = NULL;

   //Clear widgets.
   this->pHoldListBoxWidget->Clear();
   pLevelListBoxWidget->Clear();
   this->pRoomWidget->ResetRoom();
   this->pMapWidget->ClearMap();
	COptionButtonWidget *pOpButton = static_cast<COptionButtonWidget *>
			(GetWidget(TAG_ROOM_IS_REQUIRED));
	pOpButton->SetChecked(false);
   this->pStyleListBoxWidget->SelectLine(0);
}

//*****************************************************************************
bool CEditSelectScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Load EditRoomScreen now to be able to access its level/room pointers.
	if (!g_pTheSM->GetScreen(SCR_EditRoom))
	{
		ShowOkMessage(MID_CouldNotLoadResources);
		return false;
	}

	//Get widgets and current hold/level/room views ready.
	if (!SetWidgets()) return false;

	SelectFirstWidget(false);

	return true;
}

//
//Private methods.
//

//*****************************************************************************
void CEditSelectScreen::OnClick(
//Called when widget receives a click event.
//
//Params:
	const DWORD dwTagNo) //(in)	Widget that event applied to.
{
	DWORD dwAnswerTagNo;
	WSTRING wstr;

	switch (dwTagNo)
	{
		case TAG_ESCAPE:
		case TAG_CANCEL:
        {
			CDbBase::Commit();
            FreeMembers();
			GoToScreen(SCR_Return);
        }
		break;

		case TAG_EDIT:
		case TAG_MINIROOM:
			if (this->pSelectedRoom)
			{
				CDbBase::Commit();

				CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen *, CScreen *,
						g_pTheSM->GetLoadedScreen(SCR_EditRoom));
				if (pEditRoomScreen->SetRoom(this->pSelectedRoom->dwRoomID))
            {
				   delete this->pSelectedRoom;
				   this->pSelectedRoom = NULL;	//will be set upon return from EditRoomScreen.
				   GoToScreen(SCR_EditRoom);
            }
			}
		break;

		case TAG_HELP:
			SetFullScreen(false);
			ShowHelp("editselect.html");
		break;

		case TAG_NEW_HOLD:
			AddHold();
			Paint();
		break;
		case TAG_NEW_LEVEL:
			AddLevel();
			Paint();
		break;

		case TAG_RENAME_HOLD:
         if (!ModifyHold()) break;
			wstr = this->pSelectedHold->NameText;
			dwAnswerTagNo = ShowTextInputMessage(MID_NameHold, wstr);
			if (dwAnswerTagNo == TAG_OK)
			{
				this->pSelectedHold->NameText = wstr.c_str();
				this->pSelectedHold->Update();
            this->pHoldListBoxWidget->SetSelectedItemText(wstr.c_str());
				this->pHoldListBoxWidget->Paint();
			}
		break;
		case TAG_RENAME_LEVEL:
			if (!this->pSelectedLevel) break;
         ASSERT(this->pSelectedHold);
         if (!ModifyHold()) break;
			wstr = this->pSelectedLevel->NameText;
			dwAnswerTagNo = ShowTextInputMessage(MID_NameLevel, wstr);
			if (dwAnswerTagNo == TAG_OK)
			{
				this->pSelectedLevel->NameText = wstr.c_str();
				this->pSelectedLevel->Update();
            this->pLevelListBoxWidget->SetSelectedItemText(wstr.c_str());
				this->pLevelListBoxWidget->Paint();
			}
		break;

		case TAG_REDESC_HOLD:
			ASSERT(this->pSelectedHold);
         if (!ModifyHold()) break;
			wstr = this->pSelectedHold->DescriptionText;
			dwAnswerTagNo = ShowTextInputMessage(MID_DescribeHold, wstr, true);
			if (dwAnswerTagNo == TAG_OK)
			{
				this->pSelectedHold->DescriptionText = wstr.c_str();
				this->pSelectedHold->Update();
			}
		break;
		case TAG_REDESC_LEVEL:
			if (!this->pSelectedLevel) break;
         ASSERT(this->pSelectedHold);
         if (!ModifyHold()) break;
			wstr = this->pSelectedLevel->DescriptionText;
			dwAnswerTagNo = ShowTextInputMessage(MID_DescribeLevel, wstr, true);
			if (dwAnswerTagNo == TAG_OK)
			{
				this->pSelectedLevel->DescriptionText = wstr.c_str();
				this->pSelectedLevel->Update();
			}
		break;

      case TAG_COPY_HOLD:
         CopyHold();
      break;
      case TAG_FIRST_LEVEL:
         MakeLevelFirst();
      break;

      case TAG_DELETE_HOLD:
         DeleteHold();
      break;
      case TAG_DELETE_LEVEL:
         DeleteLevel();
      break;

      case TAG_ENDING_MESSAGE:
			ASSERT(this->pSelectedHold);
         if (!ModifyHold()) break;
			wstr = this->pSelectedHold->EndHoldText;
			dwAnswerTagNo = ShowTextInputMessage(MID_EndHoldPrompt, wstr, true);
			if (dwAnswerTagNo == TAG_OK)
			{
				this->pSelectedHold->EndHoldText = wstr.c_str();
				this->pSelectedHold->Update();
			}
      break;

      case TAG_ROOM_IS_REQUIRED:
		{
			COptionButtonWidget *pOpButton = DYN_CAST(COptionButtonWidget*,
               CWidget*, GetWidget(TAG_ROOM_IS_REQUIRED));
         if (!ModifyHold())
         {
            pOpButton->SetChecked(this->pSelectedRoom->bIsRequired);
            pOpButton->Paint();
         } else {
			   this->pSelectedRoom->bIsRequired = pOpButton->IsChecked();
			   this->pSelectedRoom->Update();
         }
		}
		break;
	}

	SetWidgetStates();
}

//*****************************************************************************
void CEditSelectScreen::OnKeyDown(
//Handles a keydown event.
//
//Params:
	const DWORD dwTagNo, const SDL_KeyboardEvent &Key)
{
	CScreen::OnKeyDown(dwTagNo, Key);

	switch (Key.keysym.sym)
	{
		case SDLK_DELETE:
		{
			CWidget *pWidget = GetSelectedWidget();
			switch (pWidget->GetTagNo())
			{
			case TAG_MAP:
				{
					if (!this->pSelectedRoom) break;
               if (!ModifyHold()) break;
					//Not allowed to delete level entrance room.
					DWORD dSX, dSY;
					this->pSelectedLevel->GetStartingRoomCoords(dSX, dSY);
					CDbRoom *pRoom = g_pTheDB->Rooms.GetByCoords(
							this->pSelectedLevel->dwLevelID,	dSX, dSY);
					if (pRoom->dwRoomID == this->pSelectedRoom->dwRoomID)
						ShowOkMessage(MID_CantDeleteEntranceRoom);
					else
						if (ShowYesNoMessage(MID_DeleteRoomPrompt) == TAG_YES)
						{
							g_pTheDB->Rooms.Delete(this->pSelectedRoom->dwRoomID);

                     //Update hold's timestamp.
                     this->pSelectedHold->Update();

                     //Reload level
							SelectLevel(this->pSelectedLevel->dwLevelID);
							Paint();
						}
					delete pRoom;
				}
				break;
			}
			SetWidgetStates();
		}
		break;

		//Cutting, copying and pasting selected level
		case SDLK_x:
		case SDLK_c:
			if (Key.keysym.mod & KMOD_CTRL)
			{
				CWidget *pWidget = GetSelectedWidget();
            ASSERT(pWidget);
				switch (pWidget->GetTagNo())
				{
					case TAG_LEVEL_LBOX:
					{
			         if (!this->pSelectedLevel) break;
						//Save a copy of level being cut/copied.
						CDbLevel *pCutLevel = this->pSelectedHold->Levels.GetByID(
								this->pSelectedLevel->dwLevelID);
						delete this->pLevelCopy;
						this->pLevelCopy = pCutLevel;
						this->bCopyingLevel = (Key.keysym.sym == SDLK_c);	//Ctrl-C copies

						g_pTheSound->PlaySoundEffect(SEID_TRAPDOOR);
					}
               break;
				}
			}
			return;
		case SDLK_v:
			if (Key.keysym.mod & KMOD_CTRL)
			{
				CWidget *pWidget = GetSelectedWidget();
				switch (pWidget->GetTagNo())
				{
					case TAG_HOLD_LBOX: break;

					case TAG_LEVEL_LBOX:
					{
						//Paste a level.
						g_pTheSound->PlaySoundEffect(SEID_DOOROPEN);
						const bool bUpdate = PasteLevel();
						if (bUpdate)
							Paint();
					}
					break;

					case TAG_MAP:
					{
						//Paste a room.
                  if (!ModifyHold()) break;
						if (this->pMapWidget->IsDeletingRoom())
							if (ShowYesNoMessage(MID_DeleteRoomPrompt) != TAG_YES)
								break;
						const bool bUpdate = this->pMapWidget->PasteRoom();
						if (bUpdate)
						{
                     //Update hold's timestamp.
                     this->pSelectedHold->Update();

							DWORD dwRoomX, dwRoomY;
							this->pMapWidget->GetSelectedRoomXY(dwRoomX, dwRoomY);
							SelectRoom(dwRoomX, dwRoomY);
							Paint();
						}
					}
					break;
				}
			}
		break;

      default:
         if (IsDeactivating())
            FreeMembers();
      break;
	}
}

//*****************************************************************************
void CEditSelectScreen::OnSelectChange(
//Handles a selection change.
//
//Params:
	const DWORD dwTagNo) //(in) Widget event applies to.
{
	switch (dwTagNo)
	{
		case TAG_HOLD_LBOX:
			SelectHold(this->pHoldListBoxWidget->GetSelectedItem());
			Paint();
		break;

		case TAG_LEVEL_LBOX:
			SelectLevel(this->pLevelListBoxWidget->GetSelectedItem());
			Paint();
		break;

		case TAG_STYLE_LBOX:
         if (!ModifyHold())
         {
            this->pStyleListBoxWidget->SelectItem(this->pSelectedRoom->wStyle);
         } else {
			   SetRoomStyle((UINT)(this->pStyleListBoxWidget->GetSelectedItem()));
			   if (this->pSelectedRoom) this->pSelectedRoom->Update();
         }
			Paint();
		break;

      case TAG_WHO_CAN_EDIT_LBOX:
      {
         CListBoxWidget *pWhoCanEditListBox = DYN_CAST(CListBoxWidget *, CWidget *,
			      GetWidget(TAG_WHO_CAN_EDIT_LBOX));

         //Only the hold author can change editing privileges.
         if (this->pSelectedHold->dwPlayerID == g_pTheDB->GetPlayerID())
         {
            this->pSelectedHold->editingPrivileges = (CDbHold::EditAccess)
                  pWhoCanEditListBox->GetSelectedItem();
            this->pSelectedHold->Update();
         } else {
            pWhoCanEditListBox->SelectItem(
                  (DWORD)this->pSelectedHold->editingPrivileges);
            pWhoCanEditListBox->Paint();
         }
      }
      break;

      case TAG_MAP:
		{
			DWORD dwRoomX, dwRoomY;
			this->pMapWidget->Paint();
			this->pMapWidget->GetSelectedRoomXY(dwRoomX, dwRoomY);
			if (this->pMapWidget->bVacantRoom)
			{
				if (ModifyHold() && ShowYesNoMessage(MID_AddRoomPrompt) == TAG_YES)
            {
					if (AddRoom(dwRoomX,dwRoomY))
               {
                  //Update hold's timestamp.
                  ASSERT(this->pSelectedHold);
                  this->pSelectedHold->Update();
               }
            } else {
               //Put the map selection back on the current room.
               if (this->pSelectedRoom)
                  this->pMapWidget->SelectRoom(this->pSelectedRoom->dwRoomX,
                        this->pSelectedRoom->dwRoomY);
            }
			}
			else
				SelectRoom(dwRoomX, dwRoomY);
			Paint();
		}
		break;
	}
}

//*****************************************************************************
bool CEditSelectScreen::SetWidgets()
//Set up widgets and data used by them when user enters edit screen.
//Should only be called by SetForActivate().
//
//Returns:
//True if successful, false if not.
{
	if (!this->pSelectedHold)
	{
		//Select entrance room for first level of current hold, if possible.
		//(Level selection box is populated here.)
		if (!SelectFirstHold())
			return false;	//no holds are accessable to player
								//and they don't want to create their own hold
	} else {
		if (!this->pSelectedLevel)
		{
			Paint();	//display screen first
			if (AddLevel() == 0)
				return false;
		}

		//Ensure level/map is current.
		ASSERT(this->pSelectedLevel);
		SelectLevel(this->pSelectedLevel->dwLevelID);	//reload

		//Ensure map and room are synched to room in edit screen if returning from there.
		CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen *, CScreen *,
				g_pTheSM->GetLoadedScreen(SCR_EditRoom));
		const DWORD dwRoomID = pEditRoomScreen->GetRoomID();
		if (dwRoomID)
			SetSelectedRoom(g_pTheDB->Rooms.GetByID(dwRoomID));
	}

	SetWidgetStates();

	return true;
}

//*****************************************************************************
void CEditSelectScreen::SetWidgetStates()
//Set button states depending on what data are available.
{
	CButtonWidget *pButton, *pButton2, *pButton3, *pButton4, *pButton5, *pButton6;
	COptionButtonWidget *pOpButton;

	//Hold
	pButton = static_cast<CButtonWidget *>(GetWidget(TAG_RENAME_HOLD));
	pButton2 = static_cast<CButtonWidget *>(GetWidget(TAG_REDESC_HOLD));
	pButton3 = static_cast<CButtonWidget *>(GetWidget(TAG_NEW_LEVEL));
   pButton4 = static_cast<CButtonWidget *>(GetWidget(TAG_COPY_HOLD));
   pButton5 = static_cast<CButtonWidget *>(GetWidget(TAG_DELETE_HOLD));
   pButton6 = static_cast<CButtonWidget *>(GetWidget(TAG_ENDING_MESSAGE));
	if (this->pSelectedHold)
	{
		pButton->Enable();
		pButton2->Enable();
		pButton3->Enable();
		pButton4->Enable();
      // Do not allow the user to delete an official hold
      if (pSelectedHold->dwHoldID < 100)
		   pButton5->Disable();
      else 
         pButton5->Enable();
		pButton6->Enable();
	} else {
		pButton->Disable();
		pButton2->Disable();
		pButton3->Disable();
		pButton4->Disable();
		pButton5->Disable();
		pButton6->Disable();
	}

	//Level
	pButton = static_cast<CButtonWidget *>(GetWidget(TAG_RENAME_LEVEL));
	pButton2 = static_cast<CButtonWidget *>(GetWidget(TAG_REDESC_LEVEL));
	pButton3 = static_cast<CButtonWidget *>(GetWidget(TAG_FIRST_LEVEL));
	pButton4 = static_cast<CButtonWidget *>(GetWidget(TAG_DELETE_LEVEL));
	if (this->pSelectedLevel)
	{
		pButton->Enable();
		pButton2->Enable();
		pButton3->Enable();
		pButton4->Enable();
	} else {
		pButton->Disable();
		pButton2->Disable();
		pButton3->Disable();
		pButton4->Disable();
	}

	//Room
	pButton = static_cast<CButtonWidget *>(GetWidget(TAG_EDIT));
	pOpButton = static_cast<COptionButtonWidget *>(GetWidget(TAG_ROOM_IS_REQUIRED));
	if (this->pSelectedRoom)
	{
		pButton->Enable();
		pOpButton->Enable();
		pOpButton->SetChecked(this->pSelectedRoom->bIsRequired);
	} else {
		pButton->Disable();
		pOpButton->Disable();
		pOpButton->SetChecked(false);
	}
}

//*****************************************************************************
DWORD CEditSelectScreen::AddHold()
//Inserts a new hold into the DB.
//
//Returns: new hold's ID#
{
	WSTRING wstrName;
	DWORD dwTagNo = ShowTextInputMessage(MID_NameHold, wstrName);
	if (dwTagNo != TAG_OK)
		return 0L;
	WSTRING wstrDescription;
	dwTagNo = ShowTextInputMessage(MID_DescribeHold, wstrDescription, true);
	if (dwTagNo != TAG_OK)
		return 0L;

	CDbHold *pHold;
	pHold = g_pTheDB->Holds.GetNew();

	//Set members that correspond to database fields.
	pHold->NameText = wstrName.c_str();
	pHold->DescriptionText = wstrDescription.c_str();
	pHold->dwPlayerID = g_pTheDB->GetPlayerID();

	//Save the new hold.
	if (!pHold->Update())
	{
		delete pHold;
		ShowOkMessage(MID_HoldNotSaved);
		return 0L;
	}

	//Add to hold list.
	const DWORD dwHoldID = pHold->dwHoldID;
	this->pHoldListBoxWidget->AddItem(dwHoldID, pHold->NameText);
	this->pHoldListBoxWidget->SelectItem(dwHoldID);
	Paint();

	//Add first level to hold.
	delete this->pSelectedHold;
	this->pSelectedHold = pHold;
	if (!SelectFirstLevel())
		return 0L;

	delete pHold;
	this->pSelectedHold = NULL;

	SelectHold(dwHoldID);

	return dwHoldID;
}

//*****************************************************************************
bool CEditSelectScreen::SelectFirstHold()
//Populate hold list box with list of all viewable/editable holds.
//Select the current hold, if possible.  If its editing privileges
//are restricted, select the first hold in the hold list box.
//If there aren't any accessable holds, have the player create one.
//
//Returns: true either if player can select a hold to edit,
//				or creates their own hold, else false.
{
	const DWORD dwCurrentHoldID = g_pTheDB->GetHoldID();

	PopulateHoldListBox();

	//Select current hold, if possible.
	const DWORD dwSelectingHoldID = g_pTheDB->Holds.PlayerCanEditHold(dwCurrentHoldID) ?
			dwCurrentHoldID : this->pHoldListBoxWidget->GetSelectedItem();

	if (dwSelectingHoldID)
		return SelectHold(dwSelectingHoldID);

	//Force player to create their own hold.
	//(Otherwise the screen is useless.)
   this->pLevelListBoxWidget->Clear();
	Paint();	//display screen first
	return AddHold() != 0;
}

//*****************************************************************************
bool CEditSelectScreen::SelectHold(
//Select hold by ID.
//Update level selection list.
//
//Params:
	const DWORD dwHoldID)	//(in) Chosen hold ID.
{
	if (!dwHoldID) return false;	//can happen if no holds in DB

	delete this->pSelectedHold;
	this->pSelectedHold = g_pTheDB->Holds.GetByID(dwHoldID);
	ASSERT(this->pSelectedHold);

	this->pHoldListBoxWidget->SelectItem(dwHoldID);

   //Set hold editing privileges.
   CListBoxWidget *pWhoCanEditListBox = DYN_CAST(CListBoxWidget *, CWidget *,
			GetWidget(TAG_WHO_CAN_EDIT_LBOX));
   pWhoCanEditListBox->SelectItem((DWORD)this->pSelectedHold->editingPrivileges);

	PopulateLevelListBox();

	return SelectFirstLevel();
}

//*****************************************************************************
DWORD CEditSelectScreen::AddLevel()
//Inserts a new level into the current hold in the DB.
//
//Returns: new level's ID#
{
	if (!this->pSelectedHold)
		return 0L;

   if (!ModifyHold())
      return 0L;

	WSTRING wstrName;
	DWORD dwTagNo = ShowTextInputMessage(MID_NameLevel, wstrName);
	if (dwTagNo != TAG_OK)
		return 0L;
	WSTRING wstrDescription;
	dwTagNo = ShowTextInputMessage(MID_DescribeLevel, wstrDescription, true);
	if (dwTagNo != TAG_OK)
		return 0L;

	//Get new level.
	CDbLevel *pLevel = this->pSelectedHold->Levels.GetNew();

	//Set members that correspond to database fields.
	//Note: pLevel->dwHoldID was already set to match its containing hold
	//in the call to CDbLevels::GetNew().
	pLevel->dwPlayerID = g_pTheDB->GetPlayerID();

	pLevel->NameText = wstrName.c_str();
	pLevel->DescriptionText = wstrDescription.c_str();
	//Default values.
	pLevel->wX = CDrodBitmapManager::DISPLAY_COLS/2;
	pLevel->wY = CDrodBitmapManager::DISPLAY_ROWS/2;
	pLevel->wO = SE;

	//Save the new level.
	pLevel->dwHoldID = this->pSelectedHold->dwHoldID;
	if (!pLevel->Update())
	{
		delete pLevel;
		ShowOkMessage(MID_LevelNotSaved);
		return 0L;
	}
   //Insert level into hold.
   this->pSelectedHold->InsertLevel(pLevel);

	//Add to level list box.
	const DWORD dwLevelID = pLevel->dwLevelID;
	this->pLevelListBoxWidget->AddItem(dwLevelID, pLevel->NameText);
	this->pLevelListBoxWidget->SelectItem(dwLevelID);

	//Add entrance room to level.
	delete this->pSelectedLevel;
	this->pSelectedLevel = pLevel;
	SelectLevelEntranceRoom();

	delete pLevel;
	this->pSelectedLevel = NULL;

	SelectLevel(dwLevelID);

	return dwLevelID;
}

//*****************************************************************************
bool CEditSelectScreen::SelectFirstLevel()
//Select first level in hold.
//If there are no levels, add one.
//
//Returns: whether there is a level to select, or a level was added
{
	ASSERT(this->pSelectedHold);
	delete this->pSelectedLevel;
	this->pSelectedLevel = this->pSelectedHold->Levels.GetFirst();

	if (!this->pSelectedLevel)
	{
		//Add first level.
		Paint();	//display screen first
		const bool bLevelExists = AddLevel() != 0;
      if (!bLevelExists)
      {
         //Level wasn't added -- reset all relevant widgets.
         this->pLevelListBoxWidget->Clear();
	      delete this->pSelectedLevel;
	      this->pSelectedLevel = NULL;
	      delete this->pSelectedRoom;
	      this->pSelectedRoom = NULL;
         this->pRoomWidget->ResetRoom();
         this->pMapWidget->ClearMap();
      }
      return bLevelExists;
	} else {
		SelectLevel(this->pSelectedLevel->dwLevelID);
		return true;
	}
}

//*****************************************************************************
void CEditSelectScreen::SelectLevel(
//Select level by ID.
//
//Params:
	const DWORD dwLevelID)	//(in) Chosen level ID.
{
	ASSERT(this->pSelectedHold);
	delete this->pSelectedLevel;
	this->pSelectedLevel = this->pSelectedHold->Levels.GetByID(dwLevelID);
	ASSERT(this->pSelectedLevel);

	//Update author and date labels.
	CLabelWidget *pLabel = DYN_CAST(CLabelWidget *, CWidget *,
			GetWidget(TAG_LEVEL_AUTHOR_LABEL) );
	pLabel->SetText(this->pSelectedLevel->GetAuthorText());
	pLabel = DYN_CAST(CLabelWidget *, CWidget *,
			GetWidget(TAG_LEVEL_DATE_LABEL) );
	WSTRING wstrCreated;
	this->pSelectedLevel->Created.GetLocalFormattedText(DF_LONG_DATE, wstrCreated);
	pLabel->SetText(wstrCreated.c_str());

	//Update map.
	DWORD dwX, dwY;
	this->pSelectedLevel->GetStartingRoomCoords(dwX,dwY);
	this->pMapWidget->LoadFromLevel(this->pSelectedLevel);

	SelectLevelEntranceRoom();
}

//*****************************************************************************
DWORD CEditSelectScreen::AddRoom(
//Inserts a new room into the current level at given position in the DB.
//
//Returns: new room's ID#, or 0 if failed
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of chosen room.
{
	if (!this->pSelectedHold || !this->pSelectedLevel)
		return 0L;
   if (!ModifyHold())
      return 0L;
	const DWORD dwRoomAtCoords = this->pSelectedLevel->FindRoomIDAtCoords(dwRoomX,dwRoomY);
   if (dwRoomAtCoords)
   {
      //Room already exists here.  Just return it.
      //Update widgets.
	   this->pMapWidget->LoadFromLevel(this->pSelectedLevel);
	   SelectRoom(dwRoomX,dwRoomY);
		return dwRoomAtCoords;	
   }

   //Get new room.
   CDbRoom *pRoom = this->pSelectedLevel->Rooms.GetNew();

   //Set members that correspond to database fields.
   //Note: pRoom->dwLevelID was already set to match its containing level
	//in the call to CDbRooms::GetNew().
	pRoom->dwRoomX = dwRoomX;
	pRoom->dwRoomY = dwRoomY;
	pRoom->wRoomCols = CDrodBitmapManager::DISPLAY_COLS;
	pRoom->wRoomRows = CDrodBitmapManager::DISPLAY_ROWS;
	pRoom->wStyle = (UINT)(this->pStyleListBoxWidget->GetSelectedItem());
	pRoom->bIsRequired = true;

	//Make room empty.
	const DWORD dwSquareCount = pRoom->wRoomCols * pRoom->wRoomRows;
	pRoom->pszOSquares = new char[dwSquareCount + 1];
	if (!pRoom->pszOSquares)
	{
		delete pRoom;
		return 0L;
	}
	pRoom->pszTSquares = new char[dwSquareCount + 1];
	if (!pRoom->pszTSquares)
	{
		delete pRoom;
		return 0L;
	}
	memset(pRoom->pszOSquares, T_FLOOR, dwSquareCount * sizeof(char));
	memset(pRoom->pszTSquares, T_EMPTY, dwSquareCount * sizeof(char));
	pRoom->pszOSquares[dwSquareCount] = 0;
	pRoom->pszTSquares[dwSquareCount] = 0;

	//Add proper edges to the room.
	CEditRoomScreen::FillInRoomEdges(pRoom);

	//Save the new room.
	if (!pRoom->Update())
	{
		ShowOkMessage(MID_RoomNotSaved);
		delete pRoom;
		return 0L;
	}

	const DWORD dwRoomID = pRoom->dwRoomID;
	delete pRoom;

	//Update map.
	this->pMapWidget->LoadFromLevel(this->pSelectedLevel);
	SelectRoom(dwRoomX,dwRoomY);

	return dwRoomID;
}

//*****************************************************************************
void CEditSelectScreen::CopyHold()
//Make duplicate copy of selected hold.
{
	if (!this->pSelectedHold)
      return;
   if (ShowYesNoMessage(MID_CopyHoldPrompt) != TAG_YES)
      return;

	g_pTheSound->PlaySoundEffect(SEID_MIMIC);
   SetCursor(CUR_Wait);

   CDbHold *pNewHold = this->pSelectedHold->MakeCopy();
   const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
   ASSERT(dwCurrentPlayerID);
   if (this->pSelectedHold->dwPlayerID != dwCurrentPlayerID)
      pNewHold->ChangeAuthor(dwCurrentPlayerID);
   else
   {
      //Alter hold name to indicate this one's a copy.
	   WSTRING holdName = (WSTRING)pNewHold->NameText;
      holdName += wszSpace;
	   holdName += wszLeftParen;
      holdName += g_pTheDB->GetMessageText(MID_Copy);
	   holdName += wszRightParen;
	   pNewHold->NameText = holdName.c_str();
      pNewHold->Update();
   }

   PopulateHoldListBox();
   SelectHold(pNewHold->dwHoldID);
   delete pNewHold;
   Paint();
   SetCursor();
}

//*****************************************************************************
void CEditSelectScreen::DeleteHold()
{
	if (!this->pSelectedHold)
      return;
	if (ShowYesNoMessage(MID_DeleteHoldPrompt) != TAG_YES)
      return;

	g_pTheSound->PlaySoundEffect(SEID_TRAPDOOR);
   SetCursor(CUR_Wait);

   const bool bCurrentHoldDeleted =
         g_pTheDB->GetHoldID() == this->pSelectedHold->dwHoldID;
   g_pTheDB->Holds.Delete(this->pSelectedHold->dwHoldID);
   if (bCurrentHoldDeleted)
	   g_pTheDB->SetHoldID(0L);
	CDbBase::Commit();
	FreeMembers();
	Paint();
   SetCursor();
	if (!SelectFirstHold())
	{
		//No holds player can (or wants to) edit -- go to title screen.
		GoToScreen(SCR_Return);
	}
	Paint();
}

//*****************************************************************************
void CEditSelectScreen::DeleteLevel()
//Deletes the currently selected level from its hold, upon user confirmation.
//User specifies where to reroute all stairs that previously went to this level.
{
   if (!this->pSelectedLevel)
      return;
   ASSERT(this->pSelectedHold);
   if (!ModifyHold())
      return;
	if (ShowYesNoMessage(MID_DeleteLevelPrompt) != TAG_YES)
      return;

   SetCursor(CUR_Wait);
   g_pTheDB->Levels.Delete(this->pSelectedLevel->dwLevelID);

   //Select replacement destination level *after* this level has been removed
   //from list of possibilities.  Disable cancelling operation at this point.
   DWORD dwNewLevelID = 0;
	if (!SelectLevelID(this->pSelectedLevel, dwNewLevelID, MID_DestLevelPrompt, false))
      return;

   this->pSelectedHold->RemoveLevel(this->pSelectedLevel->dwLevelID,
			dwNewLevelID);
   this->pSelectedHold->Update();
	delete this->pSelectedLevel;
	this->pSelectedLevel = NULL;

   PopulateLevelListBox();
   SelectFirstLevel();
	Paint();
   SetCursor();
}

//*****************************************************************************
void CEditSelectScreen::MakeLevelFirst()
//Confirm making this level the first one in the hold.
{
   if (!ModifyHold())
      return;
	if (ShowYesNoMessage(MID_MakeHoldEntranceLevel) != TAG_YES)
      return;

   this->pSelectedHold->dwLevelID =
			this->pSelectedLevel->dwLevelID;
	this->pSelectedHold->Update();

	g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);
}

//*****************************************************************************
bool CEditSelectScreen::ModifyHold()
//If hold being modified is owned by the current player, modifications are
//allowed.  Otherwise, the user is prompted to make a modified copy of the hold.
//If they agree, then the copy is created and selected, and the current player
//becomes the author (allowing future modifications to be made).
//
//Returns: whether modifying current hold is allowed
{
   if (!this->pSelectedHold)
      return false;
   const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
   ASSERT(dwCurrentPlayerID);
   if (this->pSelectedHold->dwPlayerID == dwCurrentPlayerID)
      return true;   //the hold author is making changes -- always allowed

   if (ShowYesNoMessage(MID_CopyHoldPrompt) != TAG_YES)
      return false;

   SetCursor(CUR_Wait);

   CDbHold *pNewHold = this->pSelectedHold->MakeCopy();
   pNewHold->ChangeAuthor(dwCurrentPlayerID);
   PopulateHoldListBox();
   SelectHold(pNewHold->dwHoldID);
   delete pNewHold;

   Paint();
   SetCursor();

   return false;
}

//*****************************************************************************
void CEditSelectScreen::SelectLevelEntranceRoom()
//Selects the entrance room for the level.
//If there is no entrance room, add one.
{
	if (!this->pSelectedLevel) return;
   ASSERT(this->pSelectedHold);

	DWORD dSX, dSY;
	this->pSelectedLevel->GetStartingRoomCoords(dSX, dSY);

	delete this->pSelectedRoom;
	this->pSelectedRoom = g_pTheDB->Rooms.GetByCoords(
			this->pSelectedLevel->dwLevelID,	dSX, dSY);

	if (!this->pSelectedRoom)
	{
		//Add entrance room to level.
		const DWORD dwLevelID = this->pSelectedLevel->dwLevelID;
		const DWORD dwRoomX = 50, dwRoomY = dwLevelID * 100 + 50;
		const DWORD dwEntranceRoomID = AddRoom(dwRoomX, dwRoomY);
      if (dwEntranceRoomID)
      {
		   this->pSelectedLevel->dwRoomID = dwEntranceRoomID;
		   this->pSelectedLevel->Update();
      } else {
         //Room couldn't be created (probably corrupted DB).
         ASSERTP(false, "Failed to create room.");
         this->pRoomWidget->ResetRoom();
      }
	}
	else
		SelectRoom(dSX, dSY);
}

//*****************************************************************************
void CEditSelectScreen::SelectRoom(
//Selects room at coords and updates display.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of chosen room.
{
	ASSERT(this->pSelectedLevel);

   //Reset room pointer so room widget isn't left with dangling pointer to old room.
   this->pRoomWidget->ResetRoom();

	//Find the room.
	CDbRoom *pRoom = g_pTheDB->Rooms.GetByCoords(
			this->pSelectedLevel->dwLevelID, dwRoomX, dwRoomY);
	if (pRoom)
		SetSelectedRoom(pRoom);

	SetWidgetStates();
}

//*****************************************************************************
void CEditSelectScreen::SetSelectedRoom(
//Selects room and updates display.
//
//Params:
	CDbRoom *pRoom)	//(in)
{
	ASSERT(pRoom);
	delete this->pSelectedRoom;
	this->pSelectedRoom = pRoom;

	//Update map widget.
	this->pMapWidget->SelectRoom(pRoom->dwRoomX, pRoom->dwRoomY);

	//Update the room widget with new room.
	this->pRoomWidget->LoadFromRoom(pRoom);
	this->pRoomWidget->Paint();

	//Select room style from the list box.
	this->pStyleListBoxWidget->SelectItem(this->pSelectedRoom->wStyle);
	CRoomScreen::SetMusicStyle(this->pSelectedRoom->wStyle);

	//Update room label.
	WSTRING wstrDesc;
	pRoom->GetLevelPositionDescription(wstrDesc);
	CLabelWidget *pRoomLabel = DYN_CAST(CLabelWidget *, CWidget *,
			GetWidget(TAG_POSITION_LABEL) );
	pRoomLabel->SetText(wstrDesc.c_str());
}

//*****************************************************************************
void CEditSelectScreen::SetRoomStyle(
//Sets the style for the selected room and updates display and music.
//
//Params:
	const UINT wStyleID)	//(in) Chosen style ID.
{
	if (!this->pSelectedRoom) return;

	this->pSelectedRoom->wStyle = wStyleID;
	//Update the room widget with new style.
	this->pRoomWidget->LoadFromRoom(this->pSelectedRoom);
	this->pRoomWidget->Paint();

	CRoomScreen::SetMusicStyle(wStyleID);
}

//*****************************************************************************
void CEditSelectScreen::Paint(
//Paint the screen.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	SDL_BlitSurface(this->pBackgroundSurface, NULL, GetDestSurface(), NULL);

	PaintChildren();
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CEditSelectScreen::PopulateHoldListBox()
//Puts all holds player has completed or authored into list box.
{
	BEGIN_DBREFCOUNT_CHECK;
	this->pHoldListBoxWidget->Clear();

	//Get holds in DB.
	CDbHold *pHold = g_pTheDB->Holds.GetFirst();

	while (pHold)
	{
		if (g_pTheDB->Holds.PlayerCanEditHold(pHold->dwHoldID))
			this->pHoldListBoxWidget->AddItem(pHold->dwHoldID, pHold->NameText);
		delete pHold;
		pHold = g_pTheDB->Holds.GetNext();
	}
	END_DBREFCOUNT_CHECK;
}

//*****************************************************************************
void CEditSelectScreen::PopulateLevelListBox()
//Puts levels of current hold into list box.
{
	BEGIN_DBREFCOUNT_CHECK;
	this->pLevelListBoxWidget->Clear();
	
	if (!this->pSelectedHold) return;

	//Get levels.
	CDbLevel *pLevel = this->pSelectedHold->Levels.GetFirst();

	while (pLevel)
	{
		this->pLevelListBoxWidget->AddItem(pLevel->dwLevelID, pLevel->NameText);
		delete pLevel;
		pLevel = this->pSelectedHold->Levels.GetNext();
	}
	END_DBREFCOUNT_CHECK;
}

//*****************************************************************************
void CEditSelectScreen::PopulateStyleListBox()
//Puts available styles into list box.
{
	this->pStyleListBoxWidget->Clear();

	//Get styles (all the ones which have a corresponding file).
	UINT wStyleNo = 1;
	WCHAR *wName;
	do {
		wName = g_pTheBM->DoesStyleExist(wStyleNo);
		if (wName)
		{
			this->pStyleListBoxWidget->AddItem( wStyleNo, g_pTheDB->GetMessageText(
          static_cast<MESSAGE_ID>(MID_STYLE_1 + wStyleNo-1)) );
			delete[] wName;
			++wStyleNo;
		}
		else break;
	} while (true);
}

//*****************************************************************************
bool CEditSelectScreen::PasteLevel()
//Pastes this->pLevelCopy to the selected spot in the current hold's level list.
//If no hold is selected, do nothing.
//If entrance level was cut-and-pasted, update hold's first level ID.
//If level is being placed as the entrance level, update new hold's first level ID.
//
//Returns: whether a level was pasted
{
	if (!this->pLevelCopy || !this->pSelectedHold) return false;
   if (!ModifyHold()) return false;

   ShowCursor();

	//Place level at selected spot.
	DWORD dwLevelID;
   const bool bDifferentHold = this->pSelectedHold->dwHoldID != this->pLevelCopy->dwHoldID;
	if (!this->bCopyingLevel)
	{
		dwLevelID = this->pLevelCopy->dwLevelID;
		DWORD dwNewLevelID;
		const bool bPaste = SelectLevelID(this->pSelectedLevel, dwNewLevelID,
				MID_DestLevelPrompt);
		if (!bPaste) return false;
      SetCursor(CUR_Wait);
      if (!bDifferentHold)
      {
		   this->pSelectedHold->RemoveLevel(dwLevelID,dwNewLevelID);
         this->pSelectedHold->Update();
      } else {
         CDbHold *pHold = g_pTheDB->Holds.GetByID(this->pLevelCopy->dwHoldID);
         pHold->RemoveLevel(dwLevelID,dwNewLevelID);
         pHold->Update();
         delete pHold;
      }

		//Move level.
		this->pLevelCopy->dwHoldID = this->pSelectedHold->dwHoldID;
      this->pLevelCopy->dwLevelIndex = 0;
		this->pSelectedHold->InsertLevel(this->pLevelCopy,
            this->pLevelListBoxWidget->GetSelectedItem());
		this->pLevelCopy->Update();
      if (bDifferentHold)
         CDbLevels::UpdateExitIDs(this->pLevelCopy->dwLevelID, this->pSelectedHold->dwHoldID);   //reset exits
		this->bCopyingLevel = true;	//future pastes will now copy level
	} else {
		//Copy level.
      SetCursor(CUR_Wait);
		CDbLevel *pNewLevel = this->pLevelCopy->MakeCopy(this->pSelectedHold->dwHoldID);
      if (!pNewLevel)
         return false;
      dwLevelID = pNewLevel->dwLevelID;

      //Insert level duplicate into current hold.
      pNewLevel->dwLevelIndex = 0;
      this->pSelectedHold->InsertLevel(pNewLevel);
		pNewLevel->Update();
      if (bDifferentHold)
         CDbLevels::UpdateExitIDs(pNewLevel->dwLevelID, this->pSelectedHold->dwHoldID);   //reset exits

      delete pNewLevel;
	}

   //Must reload the hold, level and its rooms (synchronize the data).
   SelectHold(this->pSelectedHold->dwHoldID);
	this->pLevelListBoxWidget->SelectItem(dwLevelID);
	SelectLevel(dwLevelID);

   SetCursor();

	return true;
}

// $Log: EditSelectScreen.cpp,v $
// Revision 1.61  2004/01/26 15:06:17  mrimer
// Fixed bug: can select level as new destination following deletion.
//
// Revision 1.60  2004/01/03 00:03:56  mrimer
// Fixed bug: half-deleted levels cause crash.
//
// Revision 1.59  2004/01/02 01:00:50  mrimer
// Alphabetized hold/player lists.
//
// Revision 1.58  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.57  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.56  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.55  2003/09/03 21:39:50  erikh2000
// Changed FlushWrites() calls to Commit(), since redundant FlushWrites() was removed.
//
// Revision 1.54  2003/08/15 20:15:42  mrimer
// Moved hold editing privileges into a new "Hold Settings" frame widget.
//
// Revision 1.53  2003/08/12 18:50:47  mrimer
// Fixed bug: removing hold's entrance level and selecting no repacement level corrupts hold.
//
// Revision 1.52  2003/08/11 12:48:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.51  2003/08/06 01:08:23  mrimer
// Fixed bug: level exit IDs not being updated on copy.
//
// Revision 1.50  2003/08/05 17:02:41  mrimer
// Fixed bug: renaming hold/level changes selections.
//
// Revision 1.49  2003/08/01 23:59:56  mrimer
// Fixed bugs relating to making a hold copy from the room editor.
//
// Revision 1.48  2003/07/31 21:32:05  mrimer
// Fixed bug: can't duplicate level if original is selected.
//
// Revision 1.47  2003/07/31 15:46:54  schik
// No longer allows user to delete an official hold
//
// Revision 1.46  2003/07/26 08:59:07  mrimer
// Fixed a cursor problem.
//
// Revision 1.45  2003/07/25 22:43:51  mrimer
// Made hold description textbox multi-lined when new hold is created.
//
// Revision 1.44  2003/07/21 22:08:42  mrimer
// Added ModifyHold().  Now require making personal copy of hold before modifying anything.  Made hold desc. text box multi-line.
//
// Revision 1.43  2003/07/19 21:25:54  mrimer
// Fixed bug: level list not updating when level is pasted.
//
// Revision 1.42  2003/07/19 02:20:38  mrimer
// Added hold "Ending" button.  Removed unobvious shortcut keys for editor functions that now have buttons.
//
// Revision 1.41  2003/07/17 01:53:28  mrimer
// Add " (Copy)" to the name of a duplicated hold.
//
// Revision 1.40  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.39  2003/07/16 07:47:07  mrimer
// Fixed bugs: hourglass while deleting levels; level copying faulty.
//
// Revision 1.38  2003/07/15 00:35:41  mrimer
// Fixed bug: widgets not updating when a modified copy of a hold is created.  Added SetToCopiedHold().
// Added buttons for hold and level ops: Refactored code into CopyHold(), DeleteHold(), DeleteLevel(), and MakeLevelFirst().
//
// Revision 1.37  2003/07/09 21:13:12  mrimer
// Fixed a couple bugs.
//
// Revision 1.36  2003/07/07 23:28:44  mrimer
// Moved room IsRequired button.  Fixed level copy and display bugs.
//
// Revision 1.35  2003/07/06 04:56:10  mrimer
// Update hold's timestamp when something inside it (associated with it) is modified.
//
// Revision 1.34  2003/07/05 02:36:41  mrimer
// Made room handling code more robust.
//
// Revision 1.33  2003/07/03 08:06:09  mrimer
// Fixed an assertion.
//
// Revision 1.32  2003/07/02 02:18:44  mrimer
// Fixed bug: empty hold's first level ID not being set when level is pasted.
//
// Revision 1.31  2003/07/02 01:53:57  mrimer
// Fixed bug: widgets should be reset when no level is selected.
//
// Revision 1.30  2003/07/01 23:45:49  mrimer
// Fixed bug: wrong hold selected in hold list.
//
// Revision 1.29  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.28  2003/06/27 17:27:55  mrimer
// Added hold editing privilege handling.
//
// Revision 1.27  2003/06/26 17:55:16  mrimer
// Fixed bug: previous player's editing screen not being refreshed.
//
// Revision 1.26  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.25  2003/06/17 23:13:58  mrimer
// Code maintenance -- ShowTextInputMessage() now requires text entry by default.
//
// Revision 1.24  2003/06/13 06:31:08  mrimer
// Fixed a display bug.
//
// Revision 1.23  2003/06/09 23:53:28  mrimer
// Added FreeMembers().  Reset hold/level/room vars on exiting the editor.
//
// Revision 1.22  2003/06/09 19:30:26  mrimer
// Fixed some level editor bugs.
//
// Revision 1.21  2003/05/25 22:48:11  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.20  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.19  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.18  2003/05/19 20:29:26  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.17  2003/05/08 23:26:14  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.16  2003/05/08 22:04:41  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.15  2003/05/03 23:39:25  mrimer
// Restricted hold viewing/editing privileges to the hold author, or players that completed the hold.
// Enforced non-empty naming of holds and levels.  Disallowed remaining on the edit select screen when no holds/levels exist to edit.
//
// Revision 1.14  2003/04/28 22:26:02  mrimer
// Fixed a bug.
//
// Revision 1.13  2003/04/28 14:24:17  mrimer
// Fixed bug with IsRequired room flag.
//
// Revision 1.12  2003/04/17 21:08:29  mrimer
// Updated UI to handle room's new IsRequired property.  Removed level's old roomsNeededToComplete property.
//
// Revision 1.11  2003/04/08 13:08:26  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.10  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.9  2003/02/24 17:16:04  erikh2000
// Replaced obsolete CDbBase::GetStorage() call to use CDbBase::GetView().
//
// Revision 1.8  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.7  2003/02/16 20:32:18  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.6  2003/01/08 00:57:47  mrimer
// Fixed bugs with pasting levels.
//
// Revision 1.5  2002/12/22 02:42:57  mrimer
// Implemented level cut, copy and paste.  Added making a level the hold's entrance level.
//
// Revision 1.4  2002/11/23 00:36:33  mrimer
// Now selects active hold on activation.
//
// Revision 1.3  2002/11/22 22:05:04  mrimer
// Added support for cutting, copying and pasting rooms from the map.
// Renamed OnKeyUp() to OnKeyDown().
//
// Revision 1.2  2002/11/22 02:35:52  mrimer
// Revised SetWidgets() to fix some synchronization bugs.
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
