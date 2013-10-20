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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "SettingsScreen.h"
#include "DrodBitmapManager.h"
#include "Browser.h"
#include "DrodScreenManager.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include "GameScreen.h"
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/FrameWidget.h>
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/OptionButtonWidget.h>
#include <FrontEndLib/SliderWidget.h>
#include <FrontEndLib/TextBoxWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/GameConstants.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

//Widget tag constants.
const DWORD TAG_NW_BUTTON = 1001;
const DWORD TAG_N_BUTTON = 1002;
const DWORD TAG_NE_BUTTON = 1003;
const DWORD TAG_W_BUTTON = 1004;
const DWORD TAG_WAIT_BUTTON = 1005;
const DWORD TAG_E_BUTTON = 1006;
const DWORD TAG_SW_BUTTON = 1007;
const DWORD TAG_S_BUTTON = 1008;
const DWORD TAG_SE_BUTTON = 1009;
const DWORD TAG_C_BUTTON = 1010;
const DWORD TAG_CC_BUTTON = 1011;
const DWORD TAG_RESTART_BUTTON = 1012;
const DWORD TAG_REPEATRATE = 1013;

const DWORD TAG_NW_LABEL = 1021;
const DWORD TAG_N_LABEL = 1022;
const DWORD TAG_NE_LABEL = 1023;
const DWORD TAG_W_LABEL = 1024;
const DWORD TAG_WAIT_LABEL = 1025;
const DWORD TAG_E_LABEL = 1026;
const DWORD TAG_SW_LABEL = 1027;
const DWORD TAG_S_LABEL = 1028;
const DWORD TAG_SE_LABEL = 1029;
const DWORD TAG_C_LABEL = 1030;
const DWORD TAG_CC_LABEL = 1031;
const DWORD TAG_RESTART_LABEL = 1032;
const DWORD TAG_REPEATRATE_LABEL = 1033;

const DWORD TAG_USE_FULLSCREEN = 1040;
const DWORD TAG_ENABLE_SOUNDEFF = 1041;
const DWORD TAG_ENABLE_MUSIC = 1042;
const DWORD TAG_SOUNDEFF_VOLUME = 1043;
const DWORD TAG_MUSIC_VOLUME = 1044;

const DWORD TAG_SHOWCHECKPOINTS = 1050;
const DWORD TAG_SAVEONCONQUER = 1051;
const DWORD TAG_SAVEONDIE = 1052;

const DWORD TAG_NAME = 1060;

const DWORD TAG_AUTOSAVE = 1070;
const DWORD TAG_ITEMTIPS = 1071;

const DWORD TAG_CANCEL = 1091;
const DWORD TAG_HELP = 1092;

const UINT MAXLEN_COMMANDNAME = 23;
static const char COMMANDNAME_ARRAY[DCMD_Count][MAXLEN_COMMANDNAME + 1] =
{
	"MoveNorthwest",
	"MoveNorth",
	"MoveNortheast",
	"MoveWest",
	"Wait",
	"MoveEast",
	"MoveSouthwest",
	"MoveSouth",
	"MoveSoutheast",
	"SwingClockwise",
	"SwingCounterclockwise",
	"Restart"
};

//
//Protected methods.
//

//************************************************************************************
CSettingsScreen::CSettingsScreen()
//Constructor.
	: CDrodScreen(SCR_Settings)
	, pBackgroundSurface(NULL)
   , pDialogBox(NULL)
	, pCurrentPlayer(NULL)
   , pNameWidget(NULL)
{ 
	SetKeyRepeat(66);
}

//************************************************************************************
CSettingsScreen::~CSettingsScreen()
{
	delete this->pCurrentPlayer;
}

//************************************************************************************
bool CSettingsScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	static const UINT CX_SPACE = 8;
	static const UINT CY_SPACE = 8;
	static const UINT CX_TITLE = 100;
	static const UINT CY_TITLE = 32;
	static const UINT CY_TITLE_SPACE = 8;
	static const int X_TITLE = (this->w - CX_TITLE) / 2;
	static const int Y_TITLE = CY_TITLE_SPACE;

	static const UINT CX_OKAY_BUTTON = 70;
	static const UINT CY_OKAY_BUTTON = CY_STANDARD_BUTTON;
	static const UINT CX_CANCEL_BUTTON = CX_OKAY_BUTTON;
	static const UINT CY_CANCEL_BUTTON = CY_OKAY_BUTTON;
	static const UINT CX_HELP_BUTTON = CX_OKAY_BUTTON;
	static const UINT CY_HELP_BUTTON = CY_OKAY_BUTTON;
	static const int X_CANCEL_BUTTON = (this->w - CY_CANCEL_BUTTON) / 2;
	static const int X_OKAY_BUTTON = X_CANCEL_BUTTON - CX_OKAY_BUTTON - CX_SPACE;
	static const int X_HELP_BUTTON = X_CANCEL_BUTTON + CX_CANCEL_BUTTON + CX_SPACE;
	static const int Y_OKAY_BUTTON = this->h - CY_SPACE - CY_OKAY_BUTTON;
	static const int Y_CANCEL_BUTTON = Y_OKAY_BUTTON;
	static const int Y_HELP_BUTTON = Y_OKAY_BUTTON;

	// Right half of screen. //

	//Commands frame and children.
	static const UINT CX_OFFCENTER = 40;
	static const UINT CX_COMMANDS_FRAME = (this->w - (CX_SPACE * 4)) / 2 - CX_OFFCENTER;	//narrower
	static const UINT CY_COMMANDS_FRAME = (this->h - CY_TITLE_SPACE - CY_TITLE -
			CY_TITLE_SPACE - CY_STANDARD_BUTTON - CY_SPACE * 2);
	static const int X_COMMANDS_FRAME = (this->w + CX_SPACE * 2) / 2 + CX_OFFCENTER;
	static const int Y_COMMANDS_FRAME = Y_TITLE + CY_TITLE + CY_TITLE_SPACE;
	static const UINT CX_CMD_BUTTON = (CX_COMMANDS_FRAME + CX_SPACE * 5) / 2;
	static const UINT CY_CMD_BUTTON = CY_STANDARD_BUTTON;
	static const int X_CMD_BUTTON = CX_SPACE * 2;
	static const int Y_CMD_BUTTON = CY_SPACE;
	static const int X_CMD_LABEL = X_CMD_BUTTON + CX_CMD_BUTTON + CX_SPACE * 3/2;
	static const int Y_CMD_LABEL = Y_CMD_BUTTON;
	static const UINT CX_CMD_LABEL = CX_CMD_BUTTON;
	static const UINT CY_CMD_LABEL = CY_CMD_BUTTON;

	//Repeat rate.
	static const int Y_SLOW_LABEL = CY_CMD_BUTTON * (DCMD_Count+1) + CY_SPACE;
	static const UINT CX_SLOW_LABEL = 25;
	static const UINT CY_SLOW_LABEL = 18;
	static const int Y_FAST_LABEL = Y_SLOW_LABEL;
	static const UINT CX_FAST_LABEL = 25;
	static const UINT CY_FAST_LABEL = CY_SLOW_LABEL;
	static const int X_FAST_LABEL = CX_COMMANDS_FRAME - CX_SPACE * 2 - CX_FAST_LABEL;
	static const int X_REPEATRATE_LABEL = CX_SPACE * 2;
	static const int Y_REPEATRATE_LABEL = Y_SLOW_LABEL + CY_SLOW_LABEL;
	static const UINT CX_REPEATRATE_LABEL = 60;
	static const UINT CY_REPEATRATE_LABEL = 23;
	static const int X_REPEATRATE = X_REPEATRATE_LABEL + CX_REPEATRATE_LABEL + CX_SPACE;
	static const UINT CX_REPEATRATE = CX_COMMANDS_FRAME - X_REPEATRATE - CX_SPACE * 2;
	static const UINT CY_REPEATRATE = CY_STANDARD_SLIDER;
	static const int Y_REPEATRATE = Y_REPEATRATE_LABEL +
			(CY_REPEATRATE_LABEL - CY_REPEATRATE) / 2;
	static const int X_SLOW_LABEL = X_REPEATRATE;

	// Left half of screen. //

	//Personal frame and children.
	static const int X_PERSONAL_FRAME = CX_SPACE;
	static const int Y_PERSONAL_FRAME = Y_COMMANDS_FRAME;
	static const UINT CX_PERSONAL_FRAME = (this->w - (CX_SPACE * 3)) / 2 + CX_OFFCENTER;	//wider
	static const int X_NAME_LABEL = CX_SPACE;
	static const int Y_NAME_LABEL = CY_SPACE;
	static const UINT CX_NAME_LABEL = 40;
	static const UINT CY_NAME_LABEL = 23;
	static const int X_NAME = X_NAME_LABEL + CX_NAME_LABEL + CX_SPACE;
	static const int Y_NAME = Y_NAME_LABEL;
	static const UINT CX_NAME = CX_PERSONAL_FRAME - X_NAME - CX_SPACE;
	static const UINT CY_NAME = CY_NAME_LABEL;
	static const UINT CY_PERSONAL_FRAME = CY_NAME_LABEL + CY_SPACE * 3;

	//Graphics and sound (GAS) frame and children.
	static const UINT CX_GAS_FRAME = CX_PERSONAL_FRAME;
	static const int X_GAS_FRAME = X_PERSONAL_FRAME;
	static const int Y_GAS_FRAME = Y_PERSONAL_FRAME + CY_PERSONAL_FRAME + CY_SPACE;
	//Graphics
	static const UINT CX_USEFULLSCREEN = 150;
	static const UINT CY_USEFULLSCREEN = CY_STANDARD_OPTIONBUTTON;
	static const int X_USEFULLSCREEN = CX_SPACE;
	static const int Y_USEFULLSCREEN = CY_SPACE;
	//Sound
	static const int Y_QUIET_LABEL = Y_USEFULLSCREEN + CY_USEFULLSCREEN - CY_SPACE;
	static const UINT CX_QUIET_LABEL = 25;
	static const UINT CY_QUIET_LABEL = 18;
	static const int Y_LOUD_LABEL = Y_QUIET_LABEL;
	static const UINT CX_LOUD_LABEL = 25;
	static const UINT CY_LOUD_LABEL = CY_QUIET_LABEL;
	static const int X_LOUD_LABEL = CX_GAS_FRAME - CX_SPACE - CX_LOUD_LABEL;
	static const int X_ENABLE_SOUNDEFF = CX_SPACE;
	static const int Y_ENABLE_SOUNDEFF = Y_QUIET_LABEL + CY_QUIET_LABEL;
	static const UINT CX_ENABLE_SOUNDEFF = CX_USEFULLSCREEN;
	static const UINT CY_ENABLE_SOUNDEFF = CY_STANDARD_OPTIONBUTTON;
	static const int X_ENABLE_MUSIC = X_ENABLE_SOUNDEFF;
	static const int Y_ENABLE_MUSIC = Y_ENABLE_SOUNDEFF + CY_STANDARD_OPTIONBUTTON + CY_SPACE;
	static const UINT CX_ENABLE_MUSIC = CX_ENABLE_SOUNDEFF;
	static const UINT CY_ENABLE_MUSIC = CY_ENABLE_SOUNDEFF;
	static const int X_SOUNDEFF_VOLUME = X_ENABLE_SOUNDEFF + CX_ENABLE_SOUNDEFF + CX_SPACE;
	static const UINT CX_SOUNDEFF_VOLUME = CX_GAS_FRAME - X_SOUNDEFF_VOLUME - CX_SPACE;
	static const UINT CY_SOUNDEFF_VOLUME = CY_STANDARD_SLIDER;
	static const int Y_SOUNDEFF_VOLUME = Y_ENABLE_SOUNDEFF +
			(CY_ENABLE_SOUNDEFF - CY_SOUNDEFF_VOLUME) / 2;
	static const UINT CX_MUSIC_VOLUME = CX_SOUNDEFF_VOLUME;
	static const UINT CY_MUSIC_VOLUME = CY_STANDARD_SLIDER;
	static const int X_MUSIC_VOLUME = X_SOUNDEFF_VOLUME;
	static const int Y_MUSIC_VOLUME = Y_ENABLE_MUSIC + (CY_ENABLE_MUSIC - CY_MUSIC_VOLUME) / 2;
	static const int X_QUIET_LABEL = X_SOUNDEFF_VOLUME;

	static const UINT CY_GAS_FRAME = Y_MUSIC_VOLUME + CY_MUSIC_VOLUME + CY_SPACE;

	//Special frame and children.
	static const UINT CX_STANDARD_OPTIONBUTTON = CY_STANDARD_OPTIONBUTTON;
	static const int X_SPECIAL_FRAME = X_GAS_FRAME;
	static const int Y_SPECIAL_FRAME = Y_GAS_FRAME + CY_GAS_FRAME + CY_SPACE;
	static const UINT CX_SPECIAL_FRAME = CX_GAS_FRAME;
	static const int X_SHOWCHECKPOINTS = CX_SPACE;
	static const int Y_SHOWCHECKPOINTS = CY_SPACE;
	static const UINT CX_SHOWCHECKPOINTS = CX_SPECIAL_FRAME - CX_STANDARD_OPTIONBUTTON - CY_SPACE;
	static const UINT CY_SHOWCHECKPOINTS = CY_STANDARD_OPTIONBUTTON;
	static const int X_SAVEONCONQUER = CX_SPACE;
	static const int Y_SAVEONCONQUER = Y_SHOWCHECKPOINTS + CY_SHOWCHECKPOINTS + CY_SPACE;
	static const UINT CX_SAVEONCONQUER = CX_SHOWCHECKPOINTS;
	static const UINT CY_SAVEONCONQUER = CY_STANDARD_OPTIONBUTTON;
	static const int X_SAVEONDIE = CX_SPACE;
	static const int Y_SAVEONDIE = Y_SAVEONCONQUER + CY_SAVEONCONQUER + CY_SPACE;
	static const UINT CX_SAVEONDIE = CX_SHOWCHECKPOINTS;
	static const UINT CY_SAVEONDIE = CY_STANDARD_OPTIONBUTTON;
	static const UINT CY_SPECIAL_FRAME = Y_SAVEONDIE + CY_SAVEONDIE + CY_SPACE;

	//Editor frame and children.
	static const int X_EDITOR_FRAME = X_GAS_FRAME;
	static const int Y_EDITOR_FRAME = Y_SPECIAL_FRAME + CY_SPECIAL_FRAME + CY_SPACE;
	static const UINT CX_EDITOR_FRAME = CX_GAS_FRAME;
	static const UINT CY_EDITOR_FRAME = CY_SCREEN - CY_SPACE - CY_STANDARD_BUTTON - CY_SPACE -
			Y_EDITOR_FRAME;
	static const int X_AUTOSAVE = CX_SPACE;
	static const int Y_AUTOSAVE = CY_SPACE;
	static const UINT CX_AUTOSAVE = CX_EDITOR_FRAME - CX_STANDARD_OPTIONBUTTON - CY_SPACE;
	static const UINT CY_AUTOSAVE = CY_STANDARD_OPTIONBUTTON;
	static const int X_ITEMTIPS = CX_SPACE;
	static const int Y_ITEMTIPS = Y_AUTOSAVE + CY_AUTOSAVE + CY_SPACE;
	static const UINT CX_ITEMTIPS = CX_AUTOSAVE;
	static const UINT CY_ITEMTIPS = CY_STANDARD_OPTIONBUTTON;

	CSliderWidget *pSliderWidget;
	COptionButtonWidget *pOptionButton;
	CButtonWidget *pButton;

	ASSERT(!this->bIsLoaded);
	
	//Load background graphic.
	ASSERT(!this->pBackgroundSurface);
	this->pBackgroundSurface = g_pTheBM->GetBitmapSurface("Background");
	if (!this->pBackgroundSurface) return false;

	//
	//Add widgets to screen.
	//
	
	//Title.
	AddWidget(new CLabelWidget(0L, X_TITLE, Y_TITLE,
			CX_TITLE, CY_TITLE, F_Title, g_pTheDB->GetMessageText(MID_Settings)) );

	//Personal frame.
	CFrameWidget *pPersonalFrame = new CFrameWidget(0L, X_PERSONAL_FRAME, Y_PERSONAL_FRAME,
			CX_PERSONAL_FRAME, CY_PERSONAL_FRAME, g_pTheDB->GetMessageText(MID_Personal));
	AddWidget(pPersonalFrame);

	//Personal frame children.
	pPersonalFrame->AddWidget(
			new CLabelWidget(0L, X_NAME_LABEL, Y_NAME_LABEL,
					CX_NAME_LABEL, CY_NAME_LABEL, F_Message, g_pTheDB->GetMessageText(MID_Name)) );
	this->pNameWidget = new CTextBoxWidget(TAG_NAME, X_NAME, Y_NAME, CX_NAME, CY_NAME, 30);
	pPersonalFrame->AddWidget(this->pNameWidget);

	//Graphics and sound (GAS) frame.
	CFrameWidget *pGasFrame = new CFrameWidget(0L, X_GAS_FRAME, Y_GAS_FRAME,
			CX_GAS_FRAME, CY_GAS_FRAME, g_pTheDB->GetMessageText(MID_GraphicsAndSound));
	AddWidget(pGasFrame);

	//Graphics children.
	pOptionButton = new COptionButtonWidget(TAG_USE_FULLSCREEN, X_USEFULLSCREEN,
			Y_USEFULLSCREEN, CX_USEFULLSCREEN, CY_USEFULLSCREEN,
			g_pTheDB->GetMessageText(MID_UseFullScreen), false);
	pGasFrame->AddWidget(pOptionButton);

	//Sound children.
	pOptionButton = new COptionButtonWidget(TAG_ENABLE_SOUNDEFF,
			X_ENABLE_SOUNDEFF, Y_ENABLE_SOUNDEFF, CX_ENABLE_SOUNDEFF,
			CY_ENABLE_SOUNDEFF, g_pTheDB->GetMessageText(MID_PlaySoundEffects), false);
	pGasFrame->AddWidget(pOptionButton);

	pSliderWidget = new CSliderWidget(TAG_SOUNDEFF_VOLUME, X_SOUNDEFF_VOLUME,
			Y_SOUNDEFF_VOLUME, CX_SOUNDEFF_VOLUME, CY_SOUNDEFF_VOLUME, 127);
	pGasFrame->AddWidget(pSliderWidget);

	pOptionButton = new COptionButtonWidget(TAG_ENABLE_MUSIC, X_ENABLE_MUSIC,
			Y_ENABLE_MUSIC, CX_ENABLE_MUSIC, CY_ENABLE_MUSIC,
			g_pTheDB->GetMessageText(MID_PlayMusic), false);
	pGasFrame->AddWidget(pOptionButton);

	pSliderWidget = new CSliderWidget(TAG_MUSIC_VOLUME, X_MUSIC_VOLUME,
			Y_MUSIC_VOLUME, CX_MUSIC_VOLUME, CY_MUSIC_VOLUME, 127);
	pGasFrame->AddWidget(pSliderWidget);

	pGasFrame->AddWidget(
			new CLabelWidget(0L, X_QUIET_LABEL, Y_QUIET_LABEL,
					CX_QUIET_LABEL, CY_QUIET_LABEL, F_Small, g_pTheDB->GetMessageText(MID_Quiet)) );
	pGasFrame->AddWidget(
			new CLabelWidget(0L, X_LOUD_LABEL, Y_LOUD_LABEL,
					CX_LOUD_LABEL, CY_LOUD_LABEL, F_Small, g_pTheDB->GetMessageText(MID_Loud)) );

	//Special frame.
	CFrameWidget *pSpecialFrame = new CFrameWidget(0L, X_SPECIAL_FRAME, Y_SPECIAL_FRAME,
			CX_SPECIAL_FRAME, CY_SPECIAL_FRAME, g_pTheDB->GetMessageText(MID_Special));
	AddWidget(pSpecialFrame);
	pOptionButton = new COptionButtonWidget(TAG_SHOWCHECKPOINTS, X_SHOWCHECKPOINTS,
					Y_SHOWCHECKPOINTS, CX_SHOWCHECKPOINTS, CY_SHOWCHECKPOINTS,
					g_pTheDB->GetMessageText(MID_ShowCheckpoints), false);
	pSpecialFrame->AddWidget(pOptionButton);

	pOptionButton = new COptionButtonWidget(TAG_SAVEONCONQUER, X_SAVEONCONQUER,
					Y_SAVEONCONQUER, CX_SAVEONCONQUER, CY_SAVEONCONQUER,
					g_pTheDB->GetMessageText(MID_AutoSaveDemoOnConquer), 
					false);
	pSpecialFrame->AddWidget(pOptionButton);

	pOptionButton = new COptionButtonWidget(TAG_SAVEONDIE, X_SAVEONDIE,
					Y_SAVEONDIE, CX_SAVEONDIE, CY_SAVEONDIE,
					g_pTheDB->GetMessageText(MID_AutoSaveDemoOnDie), 
					false);
	pSpecialFrame->AddWidget(pOptionButton);

	//Editor frame.
	CFrameWidget *pEditorFrame = new CFrameWidget(0L, X_EDITOR_FRAME, Y_EDITOR_FRAME,
			CX_EDITOR_FRAME, CY_EDITOR_FRAME, g_pTheDB->GetMessageText(MID_Editor));
	AddWidget(pEditorFrame);

	pOptionButton = new COptionButtonWidget(TAG_AUTOSAVE, X_AUTOSAVE,
					Y_AUTOSAVE, CX_AUTOSAVE, CY_AUTOSAVE,
					g_pTheDB->GetMessageText(MID_AutoSave), true);
	pEditorFrame->AddWidget(pOptionButton);

	pOptionButton = new COptionButtonWidget(TAG_ITEMTIPS, X_ITEMTIPS,
					Y_ITEMTIPS, CX_ITEMTIPS, CY_ITEMTIPS,
					g_pTheDB->GetMessageText(MID_ItemTips), true);
	pEditorFrame->AddWidget(pOptionButton);

	//Commands frame.
	CWidget *pCommandFrame = AddWidget(
			new CFrameWidget(0L, X_COMMANDS_FRAME, Y_COMMANDS_FRAME,
			CX_COMMANDS_FRAME, CY_COMMANDS_FRAME, g_pTheDB->GetMessageText(MID_Commands)) );

  //Command buttons.
  const UINT BUTTON_TAG_COUNT = 12;
  const DWORD ButtonTags[BUTTON_TAG_COUNT] = {TAG_NW_BUTTON, TAG_N_BUTTON, TAG_NE_BUTTON, 
      TAG_W_BUTTON, TAG_WAIT_BUTTON, TAG_E_BUTTON, TAG_SW_BUTTON, TAG_S_BUTTON, 
      TAG_SE_BUTTON, TAG_C_BUTTON, TAG_CC_BUTTON, TAG_RESTART_BUTTON};
  for (UINT wButtonTagI = 0; wButtonTagI < BUTTON_TAG_COUNT; ++wButtonTagI)
  {
    MESSAGE_ID eMsgID = static_cast<MESSAGE_ID>( MID_MoveNorthwest + wButtonTagI);
    pButton = new CButtonWidget(ButtonTags[wButtonTagI],
        X_CMD_BUTTON, Y_CMD_BUTTON + (CY_CMD_BUTTON + 1) * wButtonTagI,
        CX_CMD_BUTTON, CY_CMD_BUTTON, g_pTheDB->GetMessageText(eMsgID) );
    pCommandFrame->AddWidget(pButton);
  }
	
	//Command labels.
#	define NewLabelForCommand(labelTag) \
		this->pCommandLabelWidgets[labelTag - TAG_NW_LABEL] = new CLabelWidget(labelTag,\
				X_CMD_LABEL, Y_CMD_LABEL + (CY_CMD_LABEL + 1) * (labelTag - TAG_NW_LABEL),\
				CX_CMD_LABEL, CY_CMD_LABEL, F_Small,\
				wszEmpty);\
		pCommandFrame->AddWidget(this->pCommandLabelWidgets[labelTag - TAG_NW_LABEL])

	NewLabelForCommand(TAG_NW_LABEL);
	NewLabelForCommand(TAG_N_LABEL);
	NewLabelForCommand(TAG_NE_LABEL);
	NewLabelForCommand(TAG_W_LABEL);
	NewLabelForCommand(TAG_WAIT_LABEL);
	NewLabelForCommand(TAG_E_LABEL);
	NewLabelForCommand(TAG_SW_LABEL);
	NewLabelForCommand(TAG_S_LABEL);
	NewLabelForCommand(TAG_SE_LABEL);
	NewLabelForCommand(TAG_C_LABEL);
	NewLabelForCommand(TAG_CC_LABEL);
	NewLabelForCommand(TAG_RESTART_LABEL);

#	undef NewLabelForCommand

	//Repeat rate.
	pCommandFrame->AddWidget(
			new CLabelWidget(0L, X_REPEATRATE_LABEL, Y_REPEATRATE_LABEL,
					CX_REPEATRATE_LABEL, CY_REPEATRATE_LABEL, F_Small,
					g_pTheDB->GetMessageText(MID_RepeatRate)) );
	pSliderWidget = new CSliderWidget(TAG_REPEATRATE, X_REPEATRATE,
			Y_REPEATRATE, CX_REPEATRATE, CY_REPEATRATE, 127);
	pCommandFrame->AddWidget(pSliderWidget);
	pCommandFrame->AddWidget(
			new CLabelWidget(0L, X_SLOW_LABEL, Y_SLOW_LABEL,
					CX_SLOW_LABEL, CY_SLOW_LABEL, F_Small, g_pTheDB->GetMessageText(MID_Slow)) );
	pCommandFrame->AddWidget(
			new CLabelWidget(0L, X_FAST_LABEL, Y_FAST_LABEL,
					CX_FAST_LABEL, CY_FAST_LABEL, F_Small, g_pTheDB->GetMessageText(MID_Fast)) );


	//Okay, cancel and help buttons.
	pButton = new CButtonWidget(TAG_OK, X_OKAY_BUTTON, Y_OKAY_BUTTON,
				CX_OKAY_BUTTON, CY_OKAY_BUTTON, g_pTheDB->GetMessageText(MID_Okay));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_CANCEL, X_CANCEL_BUTTON, Y_CANCEL_BUTTON,
				CX_CANCEL_BUTTON, CY_CANCEL_BUTTON, g_pTheDB->GetMessageText(MID_Cancel));
	AddWidget(pButton);
	AddHotkey(SDLK_ESCAPE,TAG_CANCEL); 
	pButton = new CButtonWidget(TAG_HELP, X_HELP_BUTTON, Y_HELP_BUTTON, 
						   CX_HELP_BUTTON, CY_HELP_BUTTON, g_pTheDB->GetMessageText(MID_Help)); 
	AddWidget(pButton); 
	AddHotkey(SDLK_F1,TAG_HELP);

	//Dialog box.
	const int CX_MESSAGE = 200;
	const int CY_MESSAGE = 100;
	const UINT CX_MESSAGE_BUTTON = 80;
	this->pDialogBox = new CKeypressDialogWidget(0L,
			0, 0, CX_MESSAGE, CY_MESSAGE);

	this->pDialogBox->AddWidget(
			new CLabelWidget(0L, 17, CY_SPACE, CX_MESSAGE - 20, 20,
					F_Small, g_pTheDB->GetMessageText(MID_GetKeyCommand)));
	pButton = new CButtonWidget(TAG_CANCEL,
			(CX_MESSAGE-CX_MESSAGE_BUTTON) / 2,
			CY_MESSAGE-CY_STANDARD_BUTTON - CY_SPACE,
			CX_MESSAGE_BUTTON, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_CancelNoHotkey));
	this->pDialogBox->AddWidget(pButton);
	this->pDialogBox->Hide();
	AddWidget(pDialogBox);
	this->pDialogBox->Center();


	//Load children widgets.
	this->bIsLoaded = LoadChildren();

	return this->bIsLoaded;
}

//************************************************************************************
void CSettingsScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pBackgroundSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Background");
		this->pBackgroundSurface = NULL;
	}

	delete this->pCurrentPlayer;
	this->pCurrentPlayer = NULL;

	this->bIsLoaded = false;
}

//******************************************************************************
bool CSettingsScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Load player data from database.
	delete this->pCurrentPlayer;
	this->pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	if (!this->pCurrentPlayer) return false;
	SetUnspecifiedPlayerSettings(this->pCurrentPlayer->Settings);
	UpdateWidgetsFromPlayerData(this->pCurrentPlayer);

	SelectFirstWidget(false);

	return true;
}

//
//Private methods.
//

//************************************************************************************
void CSettingsScreen::SynchScreenSizeWidget()
//Should be called when screen size changes.
{
	COptionButtonWidget *pOptionButton =
		static_cast<COptionButtonWidget *>(
		GetWidget(TAG_USE_FULLSCREEN));

	if (IsFullScreen() != pOptionButton->IsChecked())
	{
		pOptionButton->SetChecked(IsFullScreen());
		pOptionButton->Paint();
	}
}

//*****************************************************************************
void CSettingsScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD dwTagNo,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	CScreen::OnKeyDown(dwTagNo,Key);

	switch (Key.keysym.sym)
	{
		case SDLK_F10:
			//Keep screen size setting synchronized.
			SynchScreenSizeWidget();
		break;

      default: break;
	}
}

//************************************************************************************
void CSettingsScreen::OnClick(const DWORD dwTagNo)
//Called when widget receives click.
{
	//Conversion macros.
#	define BUTTONTAG_TO_DCMD(t)		(static_cast<DCMD>((t) - TAG_NW_BUTTON))
#	define DCMD_TO_LABELTAG(d)		(TAG_NE_LABEL + (d))
#	define KEY_TO_MID(k)			    (MESSAGE_ID)((long) MID_UNKNOWN + (k))

	COptionButtonWidget *pOptionButton;

	switch (dwTagNo) {
					
		case TAG_ESCAPE:
		case TAG_CANCEL:
			RestorePlayerSettings();
			GoToScreen(SCR_Return);
		return;

		case TAG_OK:
      {
         //Verify all settings are valid.
	      CTextBoxWidget *pTextBox = static_cast<CTextBoxWidget *>(
			      GetWidget(TAG_NAME));
	      if (WCSlen(pTextBox->GetText()) == 0)
         {
            ShowOkMessage(MID_EmptyNameError);
            break;
         }
			if (!AllCommandsAreAssignedToKeys(this->pCurrentPlayer->Settings))
			{
            ShowOkMessage(MID_DefineAllKeys);
            break;
         }
			UpdatePlayerDataFromWidgets(this->pCurrentPlayer);
			this->pCurrentPlayer->Update();
			GoToScreen(SCR_Return);
			return;
      }

		case TAG_HELP:
			SetFullScreen(false);
			ShowHelp("settings.html"); 
			SynchScreenSizeWidget();   //need to keep synched
		break;

		case TAG_NW_BUTTON: case TAG_N_BUTTON: case TAG_NE_BUTTON:
		case TAG_W_BUTTON: case TAG_WAIT_BUTTON: case TAG_E_BUTTON:
		case TAG_SW_BUTTON: case TAG_S_BUTTON: case TAG_SE_BUTTON:
		case TAG_C_BUTTON: case TAG_CC_BUTTON: case TAG_RESTART_BUTTON:
		{
			DCMD eCommand = BUTTONTAG_TO_DCMD(dwTagNo);

			SDLKey newKey, currentKey;
			currentKey = static_cast<SDLKey>(this->pCurrentPlayer->Settings.GetVar(
				COMMANDNAME_ARRAY[eCommand], SDLK_UNKNOWN));
			if (GetCommandKeyRedefinition(eCommand, currentKey, newKey))
			{
				if (newKey != currentKey)
				{
					//Overwritten key commands set to undefined.
					for (int nCommand = DCMD_NW; nCommand < DCMD_Count; ++nCommand)
					{
						if (this->pCurrentPlayer->Settings.GetVar(
							COMMANDNAME_ARRAY[nCommand], 0)==newKey)
						{
							this->pCurrentPlayer->Settings.SetVar(
								COMMANDNAME_ARRAY[nCommand], SDLK_UNKNOWN);
							static_cast<CLabelWidget *>(
								this->pCommandLabelWidgets[nCommand])->
								SetText(g_pTheDB->GetMessageText(KEY_TO_MID(SDLK_UNKNOWN)));
						}
					}

					//Update current player settings for this command to newKey.
					this->pCurrentPlayer->Settings.SetVar(
							COMMANDNAME_ARRAY[eCommand], newKey);

					//Update label of command that was changed.
					static_cast<CLabelWidget *>(
							this->pCommandLabelWidgets[eCommand])->
								SetText(g_pTheDB->GetMessageText(KEY_TO_MID(newKey)));

					Paint();
				}
			} else {
				if (OnQuit())
					return;
			}
		}
		break;

		case TAG_SCREENSIZE:
			//Keep screen size button synchronized.
			SynchScreenSizeWidget();
		break;

		case TAG_ENABLE_SOUNDEFF:
			pOptionButton = static_cast<COptionButtonWidget *>(
				GetWidget(dwTagNo));
			g_pTheSound->EnableSoundEffects(pOptionButton->IsChecked());
			g_pTheSound->PlaySoundEffect(SEID_CLEAR);	//play sample sound
		break;

		case TAG_ENABLE_MUSIC:
			pOptionButton = static_cast<COptionButtonWidget *>(
				GetWidget(dwTagNo));
			g_pTheSound->EnableMusic(pOptionButton->IsChecked());
			if (pOptionButton->IsChecked())
				g_pTheSound->PlaySong(SONGID_INTRO);
		break;
	}	//switch dwTagNo

#	undef BUTTONTAG_TO_DCMD
#	undef DCMD_TO_LABELTAG
#	undef KEY_TO_MID
}

//************************************************************************************
void CSettingsScreen::OnDragUp(const DWORD dwTagNo, const SDL_MouseButtonEvent &/*Button*/)
//Called when widget being dragged is released.
{
	COptionButtonWidget *pOptionButton;
	CSliderWidget *pSliderWidget;

	switch (dwTagNo) {
		case TAG_SOUNDEFF_VOLUME:
			pOptionButton = static_cast<COptionButtonWidget *>(
				GetWidget(TAG_ENABLE_SOUNDEFF));
			if (!pOptionButton->IsChecked())
			{
				pOptionButton->SetChecked(true);
				pOptionButton->Paint();
				g_pTheSound->EnableSoundEffects(true);
			}
			pSliderWidget = static_cast<CSliderWidget *>(
				GetWidget(dwTagNo));
			g_pTheSound->SetSoundEffectsVolume(pSliderWidget->GetValue());
			g_pTheSound->PlaySoundEffect(SEID_CLEAR);	//play sample sound
		break;

		case TAG_MUSIC_VOLUME:
			pOptionButton = static_cast<COptionButtonWidget *>(
				GetWidget(TAG_ENABLE_MUSIC));
			if (!pOptionButton->IsChecked())
			{
				pOptionButton->SetChecked(true);
				pOptionButton->Paint();
				g_pTheSound->EnableMusic(true);
				g_pTheSound->PlaySong(SONGID_INTRO);
			}
			pSliderWidget = static_cast<CSliderWidget *>(
				GetWidget(dwTagNo));
			g_pTheSound->SetMusicVolume(pSliderWidget->GetValue());
		break;
   }
}

//************************************************************************************
void CSettingsScreen::RestorePlayerSettings()
//Upon cancelling, restore settings to the way they were.
{
	//Reload player data from database.  This instance of CDbPlayer won't have
	//changes made on this screen.
	CDbPlayer *pPlayer = g_pTheDB->GetCurrentPlayer();
   ASSERT(pPlayer);

	COptionButtonWidget *pOptionButton;
	CSliderWidget *pSliderWidget;

	UpdateWidgetsFromPlayerData(pPlayer);
	delete pPlayer;

	pOptionButton = static_cast<COptionButtonWidget *>(
		GetWidget(TAG_ENABLE_SOUNDEFF));
	g_pTheSound->EnableSoundEffects(pOptionButton->IsChecked());
	pSliderWidget = static_cast<CSliderWidget *>(
		GetWidget(TAG_SOUNDEFF_VOLUME));
	g_pTheSound->SetSoundEffectsVolume(pSliderWidget->GetValue());

	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_ENABLE_MUSIC));
	g_pTheSound->EnableMusic(pOptionButton->IsChecked());

	pSliderWidget = static_cast<CSliderWidget *>(
							GetWidget(TAG_MUSIC_VOLUME));
	g_pTheSound->SetMusicVolume(pSliderWidget->GetValue());
}

//************************************************************************************
void CSettingsScreen::SetUnspecifiedPlayerSettings(
//Any player setting that is unspecified will get a default value.  This is where the
//default settings for a player are defined.
//
//Params:
	CDbPackedVars	&Settings)	//(in/out)	When received, param may contain any number
								//			of vars that match or don't match expected
								//			vars.  Returned with all expected vars set.
{
	//Set-if-missing macros.
#	define SETMISSING(name, value) if (!Settings.DoesVarExist(name)) Settings.SetVar(name, value);

	SETMISSING("Language", 1);
	
	SETMISSING("Fullscreen", false);

	SETMISSING("Music", true);
	SETMISSING("SoundEffects", true);
	SETMISSING("MusicVolume", (BYTE)127);
	SETMISSING("SoundEffectsVolume", (BYTE)127);
	
	SETMISSING("MoveNorthwest", SDLK_KP7);
	SETMISSING("MoveNorth", SDLK_KP8);
	SETMISSING("MoveNortheast", SDLK_KP9);
	SETMISSING("MoveWest", SDLK_KP4);
	SETMISSING("Wait", SDLK_KP5);
	SETMISSING("MoveEast", SDLK_KP6);
	SETMISSING("MoveSouthwest", SDLK_KP1);
	SETMISSING("MoveSouth", SDLK_KP2);
	SETMISSING("MoveSoutheast", SDLK_KP3);
	SETMISSING("SwingClockwise", SDLK_w);
	SETMISSING("SwingCounterclockwise", SDLK_q);
	SETMISSING("Restart", SDLK_r);

	SETMISSING("ShowCheckpoints", true);
	SETMISSING("AutoSaveOptions", ASO_DEFAULT);

	SETMISSING("AutoSave", true);
	SETMISSING("ItemTips", true);

	SETMISSING("RepeatRate", (BYTE)127);

#	undef SETMISSING
}

//************************************************************************************
void CSettingsScreen::UpdateWidgetsFromPlayerData(
//Synchronizes all the widgets on the screen with player data.
//
//Params:
	CDbPlayer *pPlayer)	//(in)	Player data.
{
	//Personal settings.
	this->pNameWidget->SetText(pPlayer->NameText);
	
	//Video settings.
	COptionButtonWidget *pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_USE_FULLSCREEN));
	pOptionButton->SetChecked(pPlayer->Settings.GetVar("Fullscreen", false));

	//Sound settings.
	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_ENABLE_SOUNDEFF));
	pOptionButton->SetChecked(pPlayer->Settings.GetVar("SoundEffects", true));
	
	BYTE bytVolume = pPlayer->Settings.GetVar("SoundEffectsVolume", (BYTE)127);
	CSliderWidget *pSliderWidget = static_cast<CSliderWidget *>(
			GetWidget(TAG_SOUNDEFF_VOLUME));
	pSliderWidget->SetValue(bytVolume);

	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_ENABLE_MUSIC));
	pOptionButton->SetChecked(pPlayer->Settings.GetVar("Music", true));
	
	bytVolume = pPlayer->Settings.GetVar("MusicVolume", (BYTE)127);
	pSliderWidget = static_cast<CSliderWidget *>(
			GetWidget(TAG_MUSIC_VOLUME));
	pSliderWidget->SetValue(bytVolume);
	
	//Special settings.
	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_SHOWCHECKPOINTS));
	pOptionButton->SetChecked(pPlayer->Settings.GetVar("ShowCheckpoints", true));
	
	const DWORD dwAutoSaveOptions = pPlayer->Settings.GetVar("AutoSaveOptions", 
			ASO_DEFAULT);
	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_SAVEONCONQUER));
	pOptionButton->SetChecked((dwAutoSaveOptions & ASO_CONQUERDEMO) == ASO_CONQUERDEMO);
	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_SAVEONDIE));
	pOptionButton->SetChecked((dwAutoSaveOptions & ASO_DIEDEMO) == ASO_DIEDEMO);

	//Editor settings.
	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_AUTOSAVE));
	pOptionButton->SetChecked(pCurrentPlayer->Settings.GetVar("AutoSave", true));

	pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_ITEMTIPS));
	pOptionButton->SetChecked(pCurrentPlayer->Settings.GetVar("ItemTips", true));

	//Command settings.
	for (int nCommand = DCMD_NW; nCommand < DCMD_Count; ++nCommand)
	{
		int nKey = pPlayer->Settings.GetVar(COMMANDNAME_ARRAY[nCommand], SDLK_UNKNOWN);
		((CLabelWidget *)pCommandLabelWidgets[nCommand])->SetText(
			g_pTheDB->GetMessageText( (MESSAGE_ID)((long)MID_UNKNOWN + nKey) ) );
	}

	bytVolume = pPlayer->Settings.GetVar("RepeatRate", (BYTE)127);
	pSliderWidget = static_cast<CSliderWidget *>(
			GetWidget(TAG_REPEATRATE));
	pSliderWidget->SetValue(bytVolume);
}

//************************************************************************************
void CSettingsScreen::UpdatePlayerDataFromWidgets(
//Synchronizes player data with widgets on screen.
//
//Params:
	CDbPlayer *pPlayer)	//(in/out)	Accepts loaded player, returns with members updated.
{
	//Personal settings.
	CTextBoxWidget *pTextBox = DYN_CAST(CTextBoxWidget*, CWidget*,
			GetWidget(TAG_NAME));
	pPlayer->NameText = pTextBox->GetText();
		
	//Video settings.
	COptionButtonWidget *pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_USE_FULLSCREEN));
	pPlayer->Settings.SetVar("Fullscreen", pOptionButton->IsChecked());
	SetFullScreen(pOptionButton->IsChecked());
	
	//Sound settings.
	pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_ENABLE_SOUNDEFF));
	pPlayer->Settings.SetVar("SoundEffects", pOptionButton->IsChecked());
		
	CSliderWidget *pSliderWidget = DYN_CAST(CSliderWidget*, CWidget*,
			GetWidget(TAG_SOUNDEFF_VOLUME));
	pPlayer->Settings.SetVar("SoundEffectsVolume", pSliderWidget->GetValue());

	pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_ENABLE_MUSIC));
	pPlayer->Settings.SetVar("Music", pOptionButton->IsChecked());
	
	pSliderWidget = DYN_CAST(CSliderWidget*, CWidget*,
			GetWidget(TAG_MUSIC_VOLUME));
	pPlayer->Settings.SetVar("MusicVolume", pSliderWidget->GetValue());
	
	//Special settings.
	DWORD dwAutoSaveOptions = ASO_ROOMBEGIN | ASO_LEVELBEGIN | ASO_HIGHLIGHTDEMO;
	pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_SHOWCHECKPOINTS));
	pPlayer->Settings.SetVar("ShowCheckpoints", pOptionButton->IsChecked());
	if (pOptionButton->IsChecked()) dwAutoSaveOptions |= ASO_CHECKPOINT;
	
	pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_SAVEONCONQUER));
	if (pOptionButton->IsChecked()) dwAutoSaveOptions |= ASO_CONQUERDEMO;
	pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_SAVEONDIE));
	if (pOptionButton->IsChecked()) dwAutoSaveOptions |= ASO_DIEDEMO;
	pPlayer->Settings.SetVar("AutoSaveOptions", dwAutoSaveOptions);
	
	//Editor settings.
	pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_AUTOSAVE));
	pCurrentPlayer->Settings.SetVar("AutoSave", pOptionButton->IsChecked());
	pOptionButton = DYN_CAST(COptionButtonWidget*, CWidget*,
			GetWidget(TAG_ITEMTIPS));
	pCurrentPlayer->Settings.SetVar("ItemTips", pOptionButton->IsChecked());


	//Command settings--these were updated in response to previous UI events, 
	//so nothing to do here.
	pSliderWidget = DYN_CAST(CSliderWidget*, CWidget*, GetWidget(TAG_REPEATRATE));
	pPlayer->Settings.SetVar("RepeatRate", pSliderWidget->GetValue());
}

//************************************************************************************
void CSettingsScreen::Paint(
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

//************************************************************************************
bool CSettingsScreen::GetCommandKeyRedefinition(
//Returns false if SDL_Quit/ALT-F4 occurred, true otherwise. 
//
//Params:
	const DCMD eCommand,		//(in) Command being redefined
	const SDLKey CurrentKey,	//(in)
	SDLKey &NewKey)				//(out)
{
#	define KEY_TO_MID(k)			    (MESSAGE_ID)((long)MID_UNKNOWN + (k))
#	define DCMD_TO_BUTTONMID(d)		(MESSAGE_ID)(MID_MoveNorthwest + (d))

	const int CX_MESSAGE = 200;	//NOTE: keep same as above
	UINT cxDraw, cyDraw;
	MESSAGE_ID eButtonMID = DCMD_TO_BUTTONMID(eCommand);
	g_pTheFM->GetTextWidthHeight(F_Small, g_pTheDB->GetMessageText(eButtonMID), cxDraw, cyDraw);

	CLabelWidget *pCommandLabel = new CLabelWidget(
			0L, (CX_MESSAGE-cxDraw)/2, 35, cxDraw, cyDraw,
			F_Small, g_pTheDB->GetMessageText(eButtonMID));

	this->pDialogBox->AddWidget(pCommandLabel,true);
	this->pDialogBox->Show();

	DWORD dwRetTagNo;
	SDLKey DialogKey;
	bool bInvalidKey; 
	do 
	{
		dwRetTagNo = this->pDialogBox->Display();
		if (dwRetTagNo == TAG_QUIT || dwRetTagNo == TAG_CANCEL ||
				dwRetTagNo == TAG_ESCAPE) break;
		DialogKey = this->pDialogBox->GetKey();
		bInvalidKey = (DialogKey >= SDLK_F1 && DialogKey <= SDLK_F15);
		if (bInvalidKey)
         ShowOkMessage(MID_InvalidCommandKey);
	}
	while (bInvalidKey);

	this->pDialogBox->Hide();
	this->pDialogBox->RemoveWidget(pCommandLabel);
	Paint();

	if (dwRetTagNo == TAG_QUIT)
		return false;
	if (dwRetTagNo == TAG_CANCEL || dwRetTagNo == TAG_ESCAPE)
	{
		NewKey = CurrentKey;
		return true;
	}

	NewKey = DialogKey;
	return true;

#	undef DCMD_TO_BUTTONMID
#	undef KEY_TO_MID
}

//*****************************************************************************
bool CSettingsScreen::AllCommandsAreAssignedToKeys(
//Returns whether all commands are assigned to keys.
//(Assuming the policy that each command be assigned to a unique key is enforced.)
//
//Params:
	const CDbPackedVars &Settings) //(in)	Player settings.
const
{
	for (int nCommand = DCMD_NW; nCommand < DCMD_Count; ++nCommand)
	{
		if (Settings.GetVar(COMMANDNAME_ARRAY[nCommand], SDLK_UNKNOWN) == SDLK_UNKNOWN)
			return false;
	}
	return true;
}

// $Log: SettingsScreen.cpp,v $
// Revision 1.69  2005/03/15 21:43:44  mrimer
// Compiler tweaks.
//
// Revision 1.68  2003/09/11 02:29:12  mrimer
// Fixed bug: can erase player name.  Removed unneeded error message methods.
//
// Revision 1.67  2003/07/25 23:23:18  mrimer
// Made const widget position vars static.
//
// Revision 1.66  2003/07/22 18:29:04  mrimer
// Fixed bug: clicking "Help" brings up key redefine dialog.
//
// Revision 1.65  2003/07/22 01:01:08  erikh2000
// Removed e-mail related widgets.
//
// Revision 1.64  2003/07/03 08:13:03  mrimer
// Added destructor, and more vars in initializer list.
//
// Revision 1.63  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.62  2003/06/26 17:59:38  mrimer
// Added checks for valid active player.
//
// Revision 1.61  2003/06/16 18:49:20  mrimer
// Fixed incorrect slider behavior.
//
// Revision 1.60  2003/06/09 23:55:46  mrimer
// Removed the call to RemoveFocusWidget() on the key def dialog.
//
// Revision 1.59  2003/06/09 19:28:14  mrimer
// Fixed key redefinition bug.
//
// Revision 1.58  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.57  2003/05/23 21:43:04  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.56  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.55  2003/05/19 20:29:28  erikh2000
// Fixes to make warnings go away.
//
// Revision 1.54  2003/05/08 23:26:14  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.53  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.52  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.51  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.50  2002/11/18 18:28:35  mrimer
// Added editor settings frame and key repeat slider.  Combined graphics and sound frames.
//
// Revision 1.49  2002/11/15 03:02:57  mrimer
// Fixed some widget logic and dimensions.  Removed unneeded includes.
//
// Revision 1.48  2002/10/17 17:19:54  mrimer
// Added SynchScreenSizeWidget() to keep size setting current with actual screen size.
//
// Revision 1.47  2002/10/04 18:01:26  mrimer
// Refactored quit logic into OnQuit().
//
// Revision 1.46  2002/09/27 17:48:43  mrimer
// Tweaking.
//
// Revision 1.45  2002/09/24 21:33:10  mrimer
// Removed superfluous, erroneous hotkey mappings.
//
// Revision 1.44  2002/08/23 23:44:02  erikh2000
// Changed calls to match renamed members of CSound.
//
// Revision 1.43  2002/07/22 18:44:09  mrimer
// Set cursor repeat rate to standard speed on this screen.
//
// Revision 1.42  2002/07/22 01:54:47  erikh2000
// Put in correct filename for help HTML page, even though the page doesn't exist yet.
//
// Revision 1.41  2002/07/10 04:15:51  erikh2000
// Added auto-save option for saving highlight demos to player setting.
//
// Revision 1.40  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.39  2002/06/25 05:47:54  mrimer
// Added calls to StopRepeat().
//
// Revision 1.38  2002/06/21 05:23:19  mrimer
// Replaced MoveFocus() call with InitFocus().
//
// Revision 1.37  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.36  2002/06/16 22:17:29  erikh2000
// Made changes to how auto-save options are saved and loaded.
//
// Revision 1.35  2002/06/16 06:47:50  erikh2000
// Fixed bug where unspecified commands weren't detected after clicking "okay".
//
// Revision 1.34  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.33  2002/06/13 21:47:33  mrimer
// Dialog widget tweaking.
//
// Revision 1.32  2002/06/13 21:09:44  erikh2000
// Fixed a bug where unspecified keys were not being detected.
//
// Revision 1.31  2002/06/11 22:51:44  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.30  2002/06/11 17:47:22  mrimer
// Added DoAction(), RestorePlayerSettings().
// Implemented volume/sound behavior.
// Improved key repeat behavior.  Some tweaking.
//
// Revision 1.29  2002/06/10 23:25:27  mrimer
// Readded overwritten key commands get set to "undefined".
// Added ENTER hotkey for Okay button on key undefined error message.
// Music volume fix in UpdateWidgetsFromPlayerData().
//
// Revision 1.28  2002/06/09 18:38:20  erikh2000
// Made special frame extend to bottom of screen.
//
// Revision 1.27  2002/06/09 06:29:59  erikh2000
// Settings load and save from database instead of DROD.ini.
//
// Revision 1.26  2002/06/07 22:58:49  mrimer
// Added help button.
// Show dialog error message when invalid command key is entered.
//
// Revision 1.25  2002/06/07 18:20:16  mrimer
// Made Commands frame dynamically fit the number of command buttons.
//
// Revision 1.24  2002/06/06 00:00:06  mrimer
// Tweaked event handling.
//
// Revision 1.23  2002/06/05 19:52:06  mrimer
// Moved focus, hotkey, and key repeat handling to CScreen.
//
// Revision 1.22  2002/06/05 03:20:00  mrimer
// Implemented mouseless UI.  Moved focus vars to CScreen.
//
// Revision 1.21  2002/06/03 22:58:06  mrimer
// Added/refined focusability.
//
// Revision 1.20  2002/06/01 04:57:47  mrimer
// Finished implementing hotkeys.
//
// Revision 1.19  2002/05/31 23:46:16  mrimer
// Added hotkey support.
//
// Revision 1.18  2002/05/25 04:31:09  mrimer
// Added DrawHotkeyTextToLine().
// Consolidated specified SDL_Colors to .h file.
//
// Revision 1.17  2002/05/24 14:14:36  mrimer
// Focus widgets now inherit from CFocusWidget.
//
// Revision 1.16  2002/05/24 13:55:12  mrimer
// Added using TAB to switch focus to next widget.
//
// Revision 1.15  2002/05/23 22:45:28  mrimer
// Made bCommandsAltered local to Activate().
//
// Revision 1.14  2002/05/23 22:28:22  mrimer
// Load and save keyboard commands from DROD.ini.
//
// Revision 1.13  2002/05/21 21:42:16  mrimer
// Implemented TextBoxWidget.
//
// Revision 1.12  2002/05/21 18:11:24  mrimer
// Added changing command key mappings in settings screen.
//
// Revision 1.11  2002/05/21 13:50:32  mrimer
// Added GetCommandKeyRedefinition().  Selecting command buttons calls it.
//
// Revision 1.10  2002/05/20 18:36:09  mrimer
// Cleaned up pointers.  Get default command label messages from DB.
//
// Revision 1.9  2002/05/20 17:46:03  mrimer
// Added Personal box with name and email to settings screen.
//
// Revision 1.8  2002/05/17 09:25:00  erikh2000
// Changed title font to epilog.
//
// Revision 1.7  2002/05/12 03:22:02  erikh2000
// Wrote code to add widgets for new "Video" frame on settings screen.
// Mostly all the UI text is loaded from localizable message texts now.
//
// Revision 1.6  2002/05/10 22:43:09  erikh2000
// Reenabled "really quit?" messages.
//
// Revision 1.5  2002/04/29 00:21:33  erikh2000
// Disabled "really quit?" dialog because I was having trouble clicking on it with the cursor gone.
//
// Revision 1.4  2002/04/25 18:21:05  mrimer
// Added quit confirmation message and window resizing event.
//
// Revision 1.3  2002/04/24 08:16:12  erikh2000
// Finished widget-loading code for the screen.
//
// Revision 1.2  2002/04/19 21:58:17  erikh2000
// SettingsScreen now loads and displays a monolithic 640x480 background.
// Widgets to display title and command-redefinition interface are present.
//
// Revision 1.1  2002/04/10 00:26:30  erikh2000
// Initial check-in.
//
