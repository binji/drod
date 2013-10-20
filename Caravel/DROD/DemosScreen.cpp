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

#include "DemosScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include "Browser.h"
#include "DemoScreen.h"
#include "GameScreen.h"
#include "RoomWidget.h"
#include <FrontEndLib/FrameWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>
#include <FrontEndLib/OptionButtonWidget.h>
#include <FrontEndLib/ScalerWidget.h>
#include <FrontEndLib/ButtonWidget.h>

#include "../DRODLib/CueEvents.h"
#include "../DRODLib/CurrentGame.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbXML.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>
#include <BackEndLib/Ports.h>

//Widget tag constants.
const DWORD TAG_WATCH = 1000;
const DWORD TAG_DELETE = 1001;
const DWORD	TAG_HELP = 1002;
const DWORD TAG_RETURN = 1003;
const DWORD TAG_DEMO_LBOX = 1004;
const DWORD TAG_DISPLAYOPTION = 1005;
const DWORD TAG_EXPORT = 1006;
const DWORD TAG_IMPORT = 1007;

const UINT MAXLEN_ITEMTEXT = 100;

//
//Protected methods.
//

//************************************************************************************
CDemosScreen::CDemosScreen() : CDrodScreen(SCR_Demos)
	, pBackgroundSurface(NULL)
	, pDemoListBoxWidget(NULL)
	, pRoomWidget(NULL)
	, pScaledRoomWidget(NULL)
	, pDemoCurrentGame(NULL)
	, pAuthorWidget(NULL), pCreatedWidget(NULL), pDurationWidget(NULL)
   , pDescriptionWidget(NULL)
	, pLBoxHeaderWidget(NULL), pNoDemoWidget(NULL)
	, pDetailsFrame(NULL)
//Constructor.
{
	SetKeyRepeat(66);
}

//************************************************************************************
CDemosScreen::~CDemosScreen()
{
   delete this->pDemoCurrentGame;
}

//************************************************************************************
bool CDemosScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	const UINT CX_SPACE = 8;
	const UINT CY_SPACE = 8;
	const UINT CX_TITLE = 100;
	const UINT CY_TITLE = 32;
	const UINT CY_TITLE_SPACE = 8;
	const int X_TITLE = (this->w - CX_TITLE) / 2;
	const int Y_TITLE = CY_TITLE_SPACE;
	
	const UINT CY_WATCH = CY_STANDARD_BUTTON;
	const int Y_WATCH = CY_SCREEN - CY_SPACE - CY_WATCH;
	const UINT CX_WATCH = 50;
	const int X_WATCH = CX_SPACE;

	const UINT CY_DELETE = CY_WATCH;
	const int Y_DELETE = Y_WATCH;
	const UINT CX_DELETE = CX_WATCH;
	const int X_DELETE = X_WATCH + CX_WATCH + CX_SPACE;

	const int X_HELP_BUTTON = X_DELETE + CX_DELETE + CX_SPACE;
	const int Y_HELP_BUTTON = Y_WATCH;
	const UINT CX_HELP_BUTTON = CX_WATCH;
	const UINT CY_HELP_BUTTON = CY_WATCH;

	const UINT CY_EXPORT = CY_WATCH;
	const int Y_EXPORT = Y_WATCH;
	const UINT CX_EXPORT = CX_WATCH;
	const int X_EXPORT = X_HELP_BUTTON + CX_HELP_BUTTON + CX_SPACE;

	const int X_IMPORT = X_EXPORT + CX_EXPORT + CX_SPACE;

	const UINT CY_RETURN = CY_WATCH;
	const int Y_RETURN = Y_WATCH;
	const UINT CX_RETURN = 120;
	const int X_RETURN = CX_SCREEN - CX_SPACE - CX_RETURN;

	const UINT CY_LBOX_HEADER = 15;
	const UINT CX_LBOX_HEADER = ((CX_SCREEN - CX_SPACE) / 2) - CX_SPACE;
	const int X_LBOX_HEADER = CX_SPACE;
	const int Y_LBOX_HEADER = Y_TITLE + CY_TITLE + CY_TITLE_SPACE;
	
	const UINT CX_DEMO_LBOX = CX_LBOX_HEADER;
	const int X_DEMO_LBOX = X_LBOX_HEADER;
	const UINT CY_DEMO_LBOX = CY_SCREEN - CY_SPACE - CY_WATCH - CY_SPACE - 
			Y_LBOX_HEADER - CY_LBOX_HEADER;
	const int Y_DEMO_LBOX = Y_LBOX_HEADER + CY_LBOX_HEADER;

	const UINT CX_DETAILS_FRAME = CX_DEMO_LBOX;
	const int Y_DETAILS_FRAME = Y_LBOX_HEADER;
	const UINT CY_DETAILS_FRAME = CY_SCREEN - CY_SPACE - CY_STANDARD_BUTTON - CY_SPACE - 
			Y_DETAILS_FRAME;
	const int X_DETAILS_FRAME = X_DEMO_LBOX + CX_DEMO_LBOX + CX_SPACE;

	//Details frame widgets.
	const UINT CX_MINIROOM = CX_DETAILS_FRAME - CX_SPACE - CX_SPACE;
	const UINT CY_MINIROOM = (CX_MINIROOM * CDrodBitmapManager::CY_ROOM) /
			CDrodBitmapManager::CX_ROOM;
	const int X_MINIROOM = CX_SPACE;
	const int Y_MINIROOM = CY_DETAILS_FRAME - CY_SPACE - CY_MINIROOM;

	const int X_AUTHOR_LABEL = X_MINIROOM;
	const int Y_AUTHOR_LABEL = CY_SPACE;
	const UINT CX_AUTHOR_LABEL = 100;
	const UINT CY_AUTHOR_LABEL = 15;

	const int X_AUTHOR = X_AUTHOR_LABEL + CX_AUTHOR_LABEL;
	const int Y_AUTHOR = Y_AUTHOR_LABEL;
	const UINT CX_AUTHOR = CX_DETAILS_FRAME - CX_SPACE - X_AUTHOR_LABEL - CX_AUTHOR_LABEL;
	const UINT CY_AUTHOR = CY_AUTHOR_LABEL;

	const int X_CREATED_LABEL = X_AUTHOR_LABEL;
	const UINT CX_CREATED_LABEL = CX_AUTHOR_LABEL;
	const UINT CY_CREATED_LABEL = CY_AUTHOR_LABEL;
	const int Y_CREATED_LABEL = Y_AUTHOR_LABEL + CY_AUTHOR_LABEL;

	const int X_CREATED = X_AUTHOR;
	const int Y_CREATED = Y_CREATED_LABEL;
	const UINT CX_CREATED = CX_AUTHOR;
	const UINT CY_CREATED = CY_CREATED_LABEL;

	const int X_DURATION_LABEL = X_AUTHOR_LABEL;
	const UINT CX_DURATION_LABEL = CX_AUTHOR_LABEL;
	const UINT CY_DURATION_LABEL = CY_AUTHOR_LABEL;
	const int Y_DURATION_LABEL = Y_CREATED_LABEL + CY_CREATED_LABEL;

	const int X_DURATION = X_AUTHOR;
	const int Y_DURATION = Y_DURATION_LABEL;
	const UINT CX_DURATION = CX_AUTHOR;
	const UINT CY_DURATION = CY_DURATION_LABEL;

	const int X_DESC_LABEL = X_AUTHOR_LABEL;
	const UINT CX_DESC_LABEL = CX_AUTHOR_LABEL;
	const UINT CY_DESC_LABEL = CY_AUTHOR_LABEL;
	const int Y_DESC_LABEL = Y_DURATION_LABEL + CY_DURATION_LABEL;

	const int X_DESC = X_AUTHOR;
	const int Y_DESC = Y_DESC_LABEL;
	const UINT CX_DISPLAYOPTION = CX_AUTHOR;
	const UINT CY_DISPLAYOPTION = CY_STANDARD_OPTIONBUTTON;
	const UINT CX_DESC = CX_AUTHOR;
	const UINT CY_DESC = Y_MINIROOM - CY_DISPLAYOPTION - Y_DESC;
	const int X_DISPLAYOPTION = X_AUTHOR_LABEL;
	const int Y_DISPLAYOPTION = Y_DESC_LABEL + CY_DESC;

	const int X_NODEMO_LABEL = X_AUTHOR_LABEL;
	const int Y_NODEMO_LABEL = Y_AUTHOR_LABEL;
	const UINT CX_NODEMO_LABEL = CX_DETAILS_FRAME - CX_SPACE - CX_SPACE;
	const UINT CY_NODEMO_LABEL = CY_AUTHOR_LABEL;

	ASSERT(!this->bIsLoaded);
	
	//Load background graphic.
	ASSERT(!this->pBackgroundSurface);
	this->pBackgroundSurface = g_pTheBM->GetBitmapSurface("Background");
	if (!this->pBackgroundSurface) return false;

	//
	//Add widgets to screen.
	//
	CButtonWidget *pButton;

	//Title.
	AddWidget(new CLabelWidget(0L, X_TITLE, Y_TITLE,
			CX_TITLE, CY_TITLE, F_Title, g_pTheDB->GetMessageText(MID_Demos)));

	//Demo list box.
	this->pLBoxHeaderWidget = new CLabelWidget(0L, X_LBOX_HEADER,
			Y_LBOX_HEADER, CX_LBOX_HEADER, CY_LBOX_HEADER, F_Small, wszEmpty);
	AddWidget(this->pLBoxHeaderWidget);
	this->pDemoListBoxWidget = new CListBoxWidget(TAG_DEMO_LBOX,
			X_DEMO_LBOX, Y_DEMO_LBOX, CX_DEMO_LBOX, CY_DEMO_LBOX);
	AddWidget(this->pDemoListBoxWidget);

	//Details frame.
	this->pDetailsFrame = new CFrameWidget(0L, X_DETAILS_FRAME,
			Y_DETAILS_FRAME, CX_DETAILS_FRAME, CY_DETAILS_FRAME,
			g_pTheDB->GetMessageText(MID_Details));
	AddWidget(this->pDetailsFrame);

	//Details frame widgets.
	this->pDetailsFrame->AddWidget(new CLabelWidget(0L, X_AUTHOR_LABEL, Y_AUTHOR_LABEL,
					CX_AUTHOR_LABEL, CY_AUTHOR_LABEL, F_Small,
					g_pTheDB->GetMessageText(MID_Author)));
	this->pAuthorWidget = new CLabelWidget(0L, X_AUTHOR, Y_AUTHOR,
			CX_AUTHOR, CY_AUTHOR, F_Small, wszEmpty);
	this->pDetailsFrame->AddWidget(this->pAuthorWidget);
	this->pDetailsFrame->AddWidget(new CLabelWidget(0L, X_CREATED_LABEL, Y_CREATED_LABEL,
					CX_CREATED_LABEL, CY_CREATED_LABEL, F_Small,
					g_pTheDB->GetMessageText(MID_Created)));
	this->pCreatedWidget = new CLabelWidget(0L, X_CREATED, Y_CREATED,
			CX_CREATED, CY_CREATED, F_Small, wszEmpty);
	this->pDetailsFrame->AddWidget(this->pCreatedWidget);
	this->pDetailsFrame->AddWidget(new CLabelWidget(0L, X_DURATION_LABEL, Y_DURATION_LABEL,
					CX_DURATION_LABEL, CY_DURATION_LABEL, F_Small,
					g_pTheDB->GetMessageText(MID_Duration)));
	this->pDurationWidget = new CLabelWidget(0L, X_DURATION, Y_DURATION,
			CX_DURATION, CY_DURATION, F_Small, wszEmpty);
	this->pDetailsFrame->AddWidget(this->pDurationWidget);
	this->pDetailsFrame->AddWidget(new CLabelWidget(0L, X_DESC_LABEL, Y_DESC_LABEL,
					CX_DESC_LABEL, CY_DESC_LABEL, F_Small,
					g_pTheDB->GetMessageText(MID_Description)));
	this->pDescriptionWidget = new CLabelWidget(0L, X_DESC, Y_DESC,
			CX_DESC, CY_DESC, F_Small, wszEmpty);
	this->pDetailsFrame->AddWidget(this->pDescriptionWidget);

	this->pOptionButton = new COptionButtonWidget(
			TAG_DISPLAYOPTION, X_DISPLAYOPTION, Y_DISPLAYOPTION, CX_DISPLAYOPTION,
			CY_DISPLAYOPTION,	g_pTheDB->GetMessageText(MID_DisplayDemo), false);
	this->pDetailsFrame->AddWidget(this->pOptionButton);
	
	this->pScaledRoomWidget = new CScalerWidget(0L, X_MINIROOM, Y_MINIROOM, 
			CX_MINIROOM, CY_MINIROOM);
	this->pDetailsFrame->AddWidget(this->pScaledRoomWidget);
	this->pRoomWidget = new CRoomWidget(0L, 0, 0, CDrodBitmapManager::CX_ROOM,
			CDrodBitmapManager::CY_ROOM, NULL);
	this->pScaledRoomWidget->AddScaledWidget(this->pRoomWidget);
	
	this->pNoDemoWidget = new CLabelWidget(0L, X_NODEMO_LABEL, 
			Y_NODEMO_LABEL, CX_NODEMO_LABEL, CY_NODEMO_LABEL, F_Small,
			g_pTheDB->GetMessageText(MID_NoDemoSpecified));
	this->pDetailsFrame->AddWidget(this->pNoDemoWidget);
	this->pNoDemoWidget->Hide();

	//Bottom buttons.
	pButton = new CButtonWidget(TAG_WATCH, X_WATCH, Y_WATCH,
			CX_WATCH, CY_WATCH, g_pTheDB->GetMessageText(MID_Watch));
	AddWidget(pButton);
	pButton = new CButtonWidget(TAG_DELETE, X_DELETE, Y_DELETE,
			CX_DELETE, CY_DELETE, g_pTheDB->GetMessageText(MID_Delete));
	AddWidget(pButton);
	
	pButton = new CButtonWidget(TAG_HELP, X_HELP_BUTTON, Y_HELP_BUTTON,
			CX_HELP_BUTTON, CY_HELP_BUTTON, g_pTheDB->GetMessageText(MID_Help));
	AddWidget(pButton);
	AddHotkey(SDLK_F1,TAG_HELP);

	pButton = new CButtonWidget(TAG_EXPORT, X_EXPORT, Y_EXPORT,
		CX_EXPORT, CY_EXPORT, g_pTheDB->GetMessageText(MID_Export));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_IMPORT, X_IMPORT, Y_EXPORT,
		CX_EXPORT, CY_EXPORT, g_pTheDB->GetMessageText(MID_Import));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_RETURN, X_RETURN, Y_RETURN,
			CX_RETURN, CY_RETURN, g_pTheDB->GetMessageText(MID_ReturnToGame));
	AddWidget(pButton);

	//Load children widgets.
	this->bIsLoaded = LoadChildren();

	return this->bIsLoaded;
}

//************************************************************************************
void CDemosScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pBackgroundSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Background");
		this->pBackgroundSurface = NULL;
	}

	delete this->pDemoCurrentGame;
	this->pDemoCurrentGame = NULL;

	this->bIsLoaded = false;
}

//******************************************************************************
bool CDemosScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	this->pLBoxHeaderWidget->SetText(wszEmpty);
	this->pAuthorWidget->SetText(wszEmpty);
	this->pCreatedWidget->SetText(wszEmpty);
	this->pDurationWidget->SetText(wszEmpty);
	this->pDescriptionWidget->SetText(wszEmpty);

	//List box header to show level position description.
	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetLoadedScreen(SCR_Game));
	const CCurrentGame *pCurrentGame = pGameScreen->GetCurrentGame();
	WSTRING wstrHeader = (const WCHAR *)CDbMessageText(MID_DemosFor);
   wstrHeader += wszSpace;
	pCurrentGame->pRoom->GetLevelPositionDescription(wstrHeader);
	this->pLBoxHeaderWidget->SetText(wstrHeader.c_str());

	PopulateDemoListBox();

	return true;
}

//
//Private methods.
//

//******************************************************************************
void CDemosScreen::OnClick(
//Handles click events.
//
//Params:
	DWORD dwTagNo)								//(in) Widget receiving event.
{
	switch (dwTagNo)
	{
		case TAG_HELP:
			SetFullScreen(false);
			ShowHelp("demos.html");
		break;

		case TAG_WATCH:
		{
			const DWORD dwDemoID = this->pDemoListBoxWidget->GetSelectedItem();
			CDemoScreen *pDemoScreen = DYN_CAST(CDemoScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Demo));
         pDemoScreen->SetReplayOptions(true);
			if (!pDemoScreen->LoadDemoGame(dwDemoID))
				break;
			GoToScreen(SCR_Demo);
		}
		return;

		case TAG_DELETE:
		{
			DWORD dwDemoID = this->pDemoListBoxWidget->GetSelectedItem();
			if (dwDemoID)
			{
				const DWORD dwRet = ShowYesNoMessage(MID_ReallyDeleteDemo);
				if (dwRet == TAG_QUIT)
				{
					if (OnQuit())
						return;
				}
				else if (dwRet == TAG_YES)
				{
					//Delete the demo.
					g_pTheDB->Demos.Delete(dwDemoID);

					//Update widgets to show one less demo.
					this->pDemoListBoxWidget->RemoveItem(dwDemoID);
					dwDemoID = this->pDemoListBoxWidget->GetSelectedItem();
					SetWidgetsToDemo(dwDemoID);
					Paint();
				}
			}
		}
		break;

		case TAG_EXPORT:
		{
			const DWORD dwDemoID = this->pDemoListBoxWidget->GetSelectedItem();
			if (dwDemoID)
			{
				CDbDemo *pDemo = g_pTheDB->Demos.GetByID(dwDemoID);
				if (pDemo)
				{
               //Default filename is demo description.
					WSTRING wstrExportFile = (WSTRING)pDemo->DescriptionText;
					if (ExportSelectFile(MID_SaveDemoPath, wstrExportFile, EXT_DEMO))
					{
						//Write the demo file.
						if (CDbXML::ExportXML(ViewTypeStr(V_Demos), p_DemoID, dwDemoID,
								wstrExportFile.c_str()))
							ShowOkMessage(MID_DemoFileSaved);
						else
							ShowOkMessage(MID_DemoFileNotSaved);
					}
					delete pDemo;
				}
			}
		}
		break;

		case TAG_IMPORT:
		{
			//Import a demo data file.
         DWORD dwIgnored;
         Import(EXT_DEMO, dwIgnored);
			if (CDbXML::WasImportSuccessful() && dwIgnored)
			{
				//Update in case a demo was added.
				PopulateDemoListBox();
				Paint();
			}
		}
		break;

		case TAG_RETURN:
			Deactivate();
		return;

		case TAG_DISPLAYOPTION:
			//Toggle demo display at title screen.
			const DWORD dwDemoID = this->pDemoListBoxWidget->GetSelectedItem();
			CDbDemo *pDemo = g_pTheDB->Demos.GetByID(dwDemoID);
         ASSERT(pDemo);
			if (this->pOptionButton->IsChecked())
			{
				pDemo->wShowSequenceNo = g_pTheDB->Demos.GetNextSequenceNo();
				pDemo->Update();
			}
			else
				g_pTheDB->Demos.RemoveShowSequenceNo(pDemo->wShowSequenceNo);
			delete pDemo;
		break;
	}
}

//******************************************************************************
void CDemosScreen::OnSelectChange(
//Handles selection changes.
//
//Params:
	const DWORD dwTagNo) //(in)	Widget affected by event.
{
	if (dwTagNo == TAG_DEMO_LBOX)
	{
		const DWORD dwDemoID = this->pDemoListBoxWidget->GetSelectedItem();
		if (dwDemoID) 
		{
			SetWidgetsToDemo(dwDemoID);
			Paint();
		}
	}
}

//************************************************************************************
bool CDemosScreen::GetItemTextForDemo(
//Gets text used to summarize one demo in the list box.
//
//Params:
	DWORD dwDemoID,		//(in)	Demo to summarize.
	WSTRING &wstrText)	//(out)	Receives item text.
//
//Returns:
//True if demo is not hidden, false if demo is hidden.
const
{
	ASSERT(dwDemoID);

	//Get the demo.
	CDbDemo *pDemo = g_pTheDB->Demos.GetByID(dwDemoID);
	if (!pDemo) return false;
	if (pDemo->bIsHidden) {delete pDemo; return false;}

	//Get saved game for demo.
	CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetByID(pDemo->dwSavedGameID);
	if (!pSavedGame) {delete pDemo; return false;}

	//Copy date/time to beginning of item.
	pSavedGame->Created.GetLocalFormattedText(DF_SHORT_DATE | DF_SHORT_TIME, wstrText);
	delete pSavedGame;

	//Copy description to item.
	const WCHAR *pwczDescription = pDemo->DescriptionText;
	UINT wDescriptionLen = WCSlen(pwczDescription);
	if (wDescriptionLen)
	{
		wstrText += wszSpace;
		wstrText += wszQuote;
		if (wstrText.size() + wDescriptionLen + 1 /* """ */ > MAXLEN_ITEMTEXT)
		{
			//Append truncated description.
			UINT wCopyLen = MAXLEN_ITEMTEXT - wstrText.size() - 4; // "..."" 
			wstrText.append(pwczDescription, wCopyLen);
			wstrText += wszElipsis;
			wstrText += wszQuote;
		}
		else
		{
			//Append full description.
			wstrText += pwczDescription;
			wstrText += wszQuote;
		}
	}
	delete pDemo;
	return true;
}

//************************************************************************************
void CDemosScreen::Paint(
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
void CDemosScreen::PopulateDemoListBox()
//Compile a list of all demos recorded in this room.
//Demos from all players are shown, unless they belong to another player and
//are marked for show from the title screen.
{
	this->pDemoListBoxWidget->Clear();

	//Get room that the game screen is on.
	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetLoadedScreen(SCR_Game));
	const CCurrentGame *pCurrentGame = pGameScreen->GetCurrentGame();
   ASSERT(pCurrentGame);
	const DWORD dwRoomID = pCurrentGame->pRoom->dwRoomID;
   const DWORD dwPlayerID = pCurrentGame->dwPlayerID;
   //Get demos in current room.
   CDb db;
	db.Demos.FilterByRoom(dwRoomID);

   ASSERT(pCurrentGame->pHold);
	const bool bPlayerIsHoldAuthor =
         g_pTheDB->GetPlayerID() == pCurrentGame->pHold->dwPlayerID;

	//Add items for demos into list box.
	WSTRING wstrItemText;
	CDbDemo *pDemo = db.Demos.GetFirst();
	while (pDemo)
	{
      if (pDemo->wShowSequenceNo)
      {
         CDbSavedGame *pSavedGame = db.SavedGames.GetByID(pDemo->dwSavedGameID);
         ASSERT(pSavedGame);
         if (pSavedGame->dwPlayerID != dwPlayerID && !bPlayerIsHoldAuthor)
         {
            //Show demo made by another player.
            //Can't access this demo unless the hold is owned by the player.
            delete pSavedGame;
            delete pDemo;
		      pDemo = db.Demos.GetNext();
            continue;
         }
         delete pSavedGame;
      }
		wstrItemText = wszEmpty;
		if (GetItemTextForDemo(pDemo->dwDemoID, wstrItemText))
			this->pDemoListBoxWidget->AddItem(pDemo->dwDemoID, wstrItemText.c_str());
      delete pDemo;
		pDemo = db.Demos.GetNext();
	}
	if (this->pDemoListBoxWidget->GetItemCount()==0)
		this->pDemoListBoxWidget->AddItem(0L, g_pTheDB->GetMessageText(MID_NoDemosForRoom));
	this->pDemoListBoxWidget->SelectLine(0);

	//Set room and other widgets to demo.
	const DWORD dwDemoID = this->pDemoListBoxWidget->GetSelectedItem();
	SetWidgetsToDemo(dwDemoID);

	SelectFirstWidget(false);
}

//*****************************************************************************
void CDemosScreen::SetWidgetsToDemo(
//Sets widgets to correspond to a specified demo.
//
//Params:
	DWORD dwDemoID)	//(in)	If 0L then no demo will be selected.
{
	CWidget *pButton;

	if (dwDemoID == 0L)	//No demo selected.
	{
		this->pDetailsFrame->HideChildren();
		this->pNoDemoWidget->Show();
		
		pButton = GetWidget(TAG_DELETE);
		pButton->Disable();
		pButton = GetWidget(TAG_WATCH);
		pButton->Disable();
		pButton = GetWidget(TAG_EXPORT);
		pButton->Disable();

		return;
	}
	
	//
	//A demo is selected.
	//
	
	this->pDetailsFrame->ShowChildren();
	this->pNoDemoWidget->Hide();

	pButton = GetWidget(TAG_DELETE);
	pButton->Enable();
	pButton = GetWidget(TAG_WATCH);
	pButton->Enable();
	pButton = GetWidget(TAG_EXPORT);
	pButton->Enable();

	//Get saved game for the demo.
	CDbDemo *pDemo = g_pTheDB->Demos.GetByID(dwDemoID);
	if (!pDemo) {ASSERTP(false, "Could not retrieve demo."); return;} //Probably corrupted db.
	const DWORD dwSavedGameID = pDemo->dwSavedGameID;

	//Get new current game.
	CCueEvents Ignored;
	delete this->pDemoCurrentGame;
	this->pDemoCurrentGame = g_pTheDB->GetSavedCurrentGame(dwSavedGameID, Ignored, true);
	if (this->pDemoCurrentGame)
	{
		//Load room widget from current game.
		this->pDemoCurrentGame->SetTurn(pDemo->wBeginTurnNo, Ignored);
		this->pRoomWidget->LoadFromCurrentGame(this->pDemoCurrentGame);
	}

	//Set text for labels.
	WSTRING wstrCreated;
	this->pDemoCurrentGame->Created.GetLocalFormattedText(DF_LONG_DATE | DF_SHORT_TIME,
			wstrCreated);
	this->pCreatedWidget->SetText(wstrCreated.c_str());

	//Get author text from a couple of lookups.
	const WCHAR *pwczAuthor = pDemo->GetAuthorText();
	this->pAuthorWidget->SetText((pwczAuthor) ? pwczAuthor : wszEmpty);
	
   //Get demo duration from a couple of lookups.
   WSTRING wstrDuration;
   const UINT wMoves = pDemo->wEndTurnNo - pDemo->wBeginTurnNo + 1;
   if (pDemo->GetTimeElapsed(wstrDuration))
   {
      wstrDuration += wszSpace;
      wstrDuration += wszSpace;
   }
   WCHAR dummy[20];
   wstrDuration += _ltoW(wMoves, dummy, 10);
   wstrDuration += wszSpace;
   wstrDuration += g_pTheDB->GetMessageText(wMoves == 1 ? MID_Move : MID_Moves);
   this->pDurationWidget->SetText(wstrDuration.c_str());

	//Set description text from narrative analysis of the demo.
	WSTRING wstrNarrationText;
	pDemo->GetNarrationText(wstrNarrationText);
	this->pDescriptionWidget->SetText(wstrNarrationText.c_str());

	//Set display option box.
	this->pOptionButton->SetChecked(pDemo->wShowSequenceNo != 0);

	//Enable option: setting demos for display from title screen
	//only if current player authored this hold.
	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetLoadedScreen(SCR_Game));
	const DWORD dwAuthorID = pGameScreen->GetCurrentGame()->pHold->dwPlayerID;
	if (g_pTheDB->GetPlayerID() == dwAuthorID)
		this->pOptionButton->Show();
	else
		this->pOptionButton->Hide();

	delete pDemo;
}

// $Log: DemosScreen.cpp,v $
// Revision 1.66  2004/05/20 17:44:15  mrimer
// Fixed bug: other player's demo disappears when you mark it for show in your hold.
//
// Revision 1.65  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.64  2003/09/22 16:51:15  mrimer
// Fixed off by one error.
//
// Revision 1.63  2003/08/09 20:08:34  mrimer
// Fixed bug: cancelling import changes selected ID.
//
// Revision 1.62  2003/08/09 00:38:53  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.61  2003/08/01 00:42:54  mrimer
// Enlarged space for demo description text.
//
// Revision 1.60  2003/07/31 21:49:02  schik
// Added changing of playback speed of demos (except those run from the title screen)
//
// Revision 1.59  2003/07/31 21:34:14  mrimer
// Added demo duration display.
//
// Revision 1.58  2003/07/24 21:30:10  mrimer
// Added virtual destructor.
//
// Revision 1.57  2003/07/22 18:31:35  mrimer
// Added a space to a string.  Changed risky reinterpret casts to DYN_CAST.
//
// Revision 1.56  2003/07/19 02:23:10  mrimer
// Updated DB view access to use enum references rather than strings.
//
// Revision 1.55  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.54  2003/07/13 06:45:31  mrimer
// Removed unused parameter from CDbXML::Export() call.
//
// Revision 1.53  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.52  2003/07/09 21:11:53  mrimer
// Revised import API.
//
// Revision 1.51  2003/06/25 03:22:50  mrimer
// Fixed potential db access bugs (filtering).
//
// Revision 1.50  2003/06/17 22:11:32  mrimer
// Relaxed the demo filter criteria to include demos made by other players (except show demos).
//
// Revision 1.49  2003/06/12 21:47:27  mrimer
// Set default export file name to something more intuitive.
//
// Revision 1.48  2003/05/25 22:48:11  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.47  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.46  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.45  2003/05/08 22:04:41  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.44  2003/04/29 11:13:15  mrimer
// Added PopulateDemoListBox().  Implemented demo import.
//
// Revision 1.43  2003/04/28 22:58:28  mrimer
// Refactored export file selection code into CScreen::ExportSelectFile().
//
// Revision 1.42  2003/04/26 17:15:58  mrimer
// Fixed some bugs with file selection.
//
// Revision 1.41  2003/04/08 13:08:26  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.40  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.39  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.38  2003/02/16 20:32:17  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.37  2003/01/08 00:59:12  mrimer
// Change "display demo from title" option to only be available for hold author.
//
// Revision 1.36  2003/01/04 23:09:13  mrimer
// Updated file export interface.
//
// Revision 1.35  2002/12/22 02:46:03  mrimer
// Revised filtering to handle multiple players.  Removed BETA conditional compiles.
//
// Revision 1.34  2002/11/15 02:47:50  mrimer
// Added capability of selecting a recorded demo for public viewing.
//
// Revision 1.33  2002/10/04 22:07:23  mrimer
// Added link to demos help page.
//
// Revision 1.32  2002/10/04 18:03:02  mrimer
// Refactored quit logic into OnQuit().
//
// Revision 1.31  2002/09/01 00:06:33  erikh2000
// Fixed a memory leak that occurs when player cancels demo export.
//
// Revision 1.30  2002/08/31 03:05:23  erikh2000
// Removed some testing code.
//
// Revision 1.29  2002/08/30 20:01:19  mrimer
// NULLed all object pointers in the constructor.
//
// Revision 1.28  2002/08/29 09:20:30  erikh2000
// Wrote code to export demo file when export button is pressed.
//
// Revision 1.27  2002/08/28 22:20:55  mrimer
// Reinserted export button with BETA conditional compile.
//
// Revision 1.26  2002/07/22 18:44:09  mrimer
// Set cursor repeat rate to standard speed on this screen.
//
// Revision 1.25  2002/07/20 23:07:47  erikh2000
// Changed code to use new CScalerWidget to draw scaled room.
//
// Revision 1.24  2002/07/17 20:46:01  erikh2000
// Added some testing code to play the most interesting scene from a demo.
//
// Revision 1.23  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.22  2002/06/25 05:42:24  mrimer
// Made call to StopRepeat().
//
// Revision 1.21  2002/06/22 05:57:20  erikh2000
// Fixed an error causing a couple of buttons to draw before the fade in transition.
//
// Revision 1.20  2002/06/21 22:31:23  mrimer
// Set focus to "Return" button when all demos are deleted.
//
// Revision 1.19  2002/06/21 05:02:51  mrimer
// Added InitFocus() call.
//
// Revision 1.18  2002/06/21 03:31:55  erikh2000
// Made some cosmetic changes involving what widgets are shown when no demo is selected.
//
// Revision 1.17  2002/06/20 04:11:57  erikh2000
// Made narration always appear for demo description.
// Changed calls to methods that now take wstring parameters.
//
// Revision 1.16  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.15  2002/06/16 22:11:53  erikh2000
// Added code to loaded text from DB.
//
// Revision 1.14  2002/06/16 06:24:22  erikh2000
// Wrote code to delete a demo in response to "delete" button click.
//
// Revision 1.13  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.12  2002/06/14 18:26:05  mrimer
// Fixed Activate so OnMouseButtonDown() doesn't set dwTagNo.
//
// Revision 1.11  2002/06/14 02:31:16  erikh2000
// Removed export button.
// Fixed some small bugs.
// Watch and delete buttons are now disabled when there are no demos.
//
// Revision 1.10  2002/06/14 00:46:26  erikh2000
// Changed call to obsolete GetScreenSurfaceColor() method so that it gets color from dest surface instead.
//
// Revision 1.9  2002/06/13 22:57:47  erikh2000
// Changed code so that it receives dates in a wstring instead of a preallocated wchar buffer.
// Changed method for concatenating item text to return a wstring.
//
// Revision 1.8  2002/06/13 21:52:26  mrimer
// Finished specifying focusable widgets.
// Minor fixes.
//
// Revision 1.7  2002/06/11 23:00:03  mrimer
// Now use MessageText loading instead of literals.
//
// Revision 1.6  2002/06/11 21:15:57  mrimer
// Updated surface handling.
//
// Revision 1.5  2002/06/09 06:38:26  erikh2000
// Changed code to use new message text class.
// Author label now gets set from demo author.
//
// Revision 1.4  2002/06/09 01:11:48  mrimer
// Added help button.
// Rearranged includes.
//
// Revision 1.3  2002/06/05 03:13:26  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.2  2002/05/21 21:33:55  erikh2000
// Fixed a widget dimensions error.
// Demo description will now use a generated narration text by default.
//
// Revision 1.1  2002/05/15 01:22:33  erikh2000
// Initial check-in.
//
