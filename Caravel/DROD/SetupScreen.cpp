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
 * 2003 Caravel Software. All Rights Reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#include "SetupScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "../DRODLib/dbprops1_5.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbCommands.h"
#include "../DRODLib/GameConstants.h"
#include "../Texts/MIDs.h"
#include <FrontEndLib/LabelWidget.h>
#include <BackEndLib/Files.h>

//**************************************************************************************
inline void InsertDemoCommand(CDbCommands &commands, const UINT wIndex, const BYTE command)
{
   COMMANDNODE *pCommand = commands.Get(wIndex);
   COMMANDNODE *pNewCommand = new COMMANDNODE;
   pNewCommand->bytCommand = command;
   pNewCommand->byt10msElapsedSinceLast = 0;
   pNewCommand->pNext = pCommand->pNext;
   pCommand->pNext = pNewCommand;
}

//**********************************************************************************************************
bool CSetupScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
    const UINT CX_SPACE = 8;
	const UINT CY_SPACE = 8;
	const UINT CX_TITLE = 200;
	const UINT CY_TITLE = 32;
	const UINT CY_TITLE_SPACE = 8;
	const int X_TITLE = (this->w - CX_TITLE) / 2;
	const int Y_TITLE = CY_TITLE_SPACE;

    const UINT CX_PROGRESS = (this->w - CX_SPACE - CX_SPACE);
    const UINT CY_PROGRESS = 30;
    const int X_PROGRESS = CX_SPACE;
    const int Y_PROGRESS = (this->h - CY_SPACE - CY_PROGRESS);

//    const UINT CX_EXPLANATION = 500;
//    const int X_EXPLANATION = (this->w - CX_EXPLANATION) / 2;
//    const int Y_EXPLANATION = Y_TITLE + CY_TITLE + CY_SPACE;
//    const UINT CY_EXPLANATION = this->h - Y_EXPLANATION - CY_SPACE - CY_PROGRESS - CY_SPACE;

    //Load background graphic.
	ASSERT(!this->pBackgroundSurface);
	this->pBackgroundSurface = g_pTheBM->GetBitmapSurface("Background");
	if (!this->pBackgroundSurface) return false;

    //Title.
	CLabelWidget *pLabel = new CLabelWidget(0L, X_TITLE, Y_TITLE,
			CX_TITLE, CY_TITLE, F_Title, g_pTheDB->GetMessageText(MID_UpgradingData));
    pLabel->SetAlign(CLabelWidget::TA_CenterGroup);
    AddWidget(pLabel);

    //Progress bar.
    this->pProgressWidget = new CProgressBarWidget(0L, X_PROGRESS, Y_PROGRESS, CX_PROGRESS, CY_PROGRESS, 0);
    this->pProgressWidget->Hide(false);
    AddWidget(this->pProgressWidget);

    this->bIsLoaded = LoadChildren();
    return this->bIsLoaded;
}

//**********************************************************************************************************
void CSetupScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pBackgroundSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Background");
		this->pBackgroundSurface = NULL;
	}

	this->bIsLoaded = false;
}

//**********************************************************************************************************
void CSetupScreen::Paint(
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

//**********************************************************************************************************
void CSetupScreen::OnBetweenEvents()
{
    this->pProgressWidget->SetValue( this->Work.wPercentComplete * 255 / 100 );
    
    switch (this->Work.eStatus)
    {
        case CWork::NotStarted:
        {
            CFiles Files;
            WSTRING wstrSourcePath;//, wstrDestPath = Files.GetDatPath();

            SetCursor(CUR_Wait);

            //Get import path from drod.ini.
            {
                string strSourcePath;
                Files.GetGameProfileString("Setup", "ImportPath", strSourcePath);
                AsciiToUnicode(strSourcePath.c_str(), wstrSourcePath);
            }

            //Show intro message and see if user wants to run upgrade.
            //Upgrade can only be performed on the first run.
            {
               string strStatus;
               SetCursor(CUR_Select);
               Files.GetGameProfileString("Setup", "Status", strStatus);
               while (ShowYesNoMessage(MID_RunUpgrade)==TAG_NO)
               {
                  if (ShowYesNoMessage(MID_ReallyCancelUpgradeData)==TAG_YES)
                  {
                     CFiles Files;
                     Files.WriteGameProfileString("Setup", "Status", "cancelled");
                     Deactivate();
                     return;
                  }
               }
               SetCursor(CUR_Wait);
            }
            
            if (!StartImport(wstrSourcePath.c_str()))
            {
                SetCursor(CUR_Select);
                ShowOkMessage(MID_UpgradeDataFailed);
                Deactivate();
            }

            //Show widgets, now that importing is underway.
            this->pProgressWidget->Show();
        }
        return;

        case CWork::InProgress:
            DoWork();
        return;

        case CWork::Completed:
        {
            CFiles Files;
            Files.WriteGameProfileString("Setup", "Status", "complete");
            Deactivate();
            SetCursor();
        }
        return;

        case CWork::Failed:
        {
            CFiles Files;
            SetCursor(CUR_Select);
            ShowOkMessage(MID_UpgradeDataFailed);
            Deactivate();
        }
        return;

        default:
        ASSERTP(false, "Bad work state.");
    }
}

//**********************************************************************************************************
bool CSetupScreen::OnQuit()
//Called when SDL_QUIT event is received.
//Returns whether Quit action was confirmed.
{
   SetCursor(CUR_Select);
   const bool bExit = (ShowYesNoMessage(MID_ReallyCancelUpgradeData) != TAG_NO);
   SetCursor(CUR_Wait);
   if (bExit)
   {
      HideStatusMessage();
      GoToScreen(SCR_None);
      return true;
   }
   return false;
}

//**********************************************************************************************************
void CSetupScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD /*dwTagNo*/,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	switch (Key.keysym.sym)
	{
        case SDLK_RETURN:
         if (Key.keysym.mod & KMOD_ALT &&	!GetHotkeyTag(Key.keysym.sym))
            //going to next case
		case SDLK_F10:
			ToggleScreenSize();
		break;

		case SDLK_F4:
			if (Key.keysym.mod & KMOD_ALT)
				OnQuit();
		break;

		case SDLK_ESCAPE:
        {
            SetCursor(CUR_Select);
            if (ShowYesNoMessage(MID_ReallyCancelUpgradeData) != TAG_NO)
            {
                HideStatusMessage();
			    Deactivate();
            }
            else
                SetCursor(CUR_Wait);
        }
		break;

      default: break;
	}
}

//**********************************************************************************************************
bool CSetupScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Set frame rate high to speed import.
	SetBetweenEventsInterval(0);

   return true;
}

//
//CSetupScreen private methods.
//

//**********************************************************************************************************
bool CSetupScreen::StartImport(const WCHAR *pwzSourcePath)
{
    if (!pwzSourcePath || !*pwzSourcePath) return false;

    if (!g_pTheDB->IsOpen()) return false;

    ASSERT(!this->Work.wPercentComplete);
    this->Work.wstrSourcePath = pwzSourcePath;
    this->Work.wPercentComplete = 1;
    this->Work.eStatus = CWork::InProgress;

    return true;
}

//**********************************************************************************************************
void CSetupScreen::DoWork()
//Do a little bit of the importing work, save results, and exit.  The method is called several times
//to complete the import.
{
   static const DWORD dwDugansDungeonLastRoomID = 342;

    bool bSuccess = true;
    switch (Work.eStep)
    {
        case CWork::OpenDROD1_5:
        {
            ShowStatusMessage(MID_Opening1_5Data);

            //Check for presence of 1.5 dat.
            WSTRING wstr1_5Filepath = Work.wstrSourcePath.c_str();
            {
                static const WCHAR szDROD1_5Dat[] = {W_t('d'),W_t('r'),W_t('o'),W_t('d'),W_t('1'),W_t('_'),W_t('5'),W_t('.'),W_t('d'),W_t('a'),W_t('t'),W_t(0)};
                wstr1_5Filepath += wszSlash;
                wstr1_5Filepath += szDROD1_5Dat;
                if (!CFiles::DoesFileExist(wstr1_5Filepath.c_str())) {bSuccess = false; break;}
            }

            //Open it.
            Work.pDROD1_5Stream = new CGameStream(wstr1_5Filepath.c_str());
            if (!Work.pDROD1_5Stream) {bSuccess = false; break;}
            Work.pDROD1_5Storage = new c4_Storage;
            if (!Work.pDROD1_5Storage || !Work.pDROD1_5Storage->LoadFrom(*(Work.pDROD1_5Stream)))
                {bSuccess = false; break;}

            Work.wPercentComplete = 5;
            Work.SetNextStep();
        }
        break;

        case CWork::CopyPlayers:
        {
            if (!Work.pDestPlayersView) //1st call w/this step.
            {
                ShowStatusMessage(MID_CopyingPlayerSettings); 

                Work.pDestPlayersView = new c4_View(g_pTheDB->GetView(ViewTypeStr(V_Players)));

                //Remove all player records.
                Work.pDestPlayersView->RemoveAll();

                ASSERT(!Work.pSourcePlayersView);
                Work.pSourcePlayersView = new c4_View(Work.pDROD1_5Storage->View(ViewTypeStr(V_Players)));
            }
            const DWORD dwPlayerCount = Work.pSourcePlayersView->GetSize();
            if (Work.dwPlayersCopied == dwPlayerCount)
            {
                //Set incremented ID to match
                g_pTheDB->SetIncrementedID(p_PlayerID, dwPlayerCount);

                delete Work.pDestPlayersView; Work.pDestPlayersView = NULL;
                delete Work.pSourcePlayersView; Work.pSourcePlayersView = NULL;

                Work.wPercentComplete = 10;
                Work.SetNextStep();
            }
            else
            {
                //Import all player records.
                //Non-local players keep their original creation time (for GUID).
                CDate CreatedTime, now;
                CDate OrigCreatedTime(1997, 1, 1);
                c4_RowRef SourceRow = Work.pSourcePlayersView->GetAt(Work.dwPlayersCopied);
                CreatedTime = ns1_5::p_IsLocal(SourceRow) != 0 ? now : OrigCreatedTime;

                const DWORD dwNameMessageID = ImportMessage(ns1_5::p_NameMessageID(SourceRow));
                const DWORD dwOriginalNameMessageID = ImportMessage(ns1_5::p_NameMessageID(SourceRow));
                const DWORD dwEMailMessageID = ImportMessage(ns1_5::p_EMailMessageID(SourceRow));

                Work.pDestPlayersView->Add(
		                p_PlayerID[                     ns1_5::p_PlayerID(SourceRow) ] +
		                p_IsLocal[                      ns1_5::p_IsLocal(SourceRow) ] +
		                p_NameMessageID[                dwNameMessageID ] +
		                p_EMailMessageID[               dwEMailMessageID ] +
		                p_GID_OriginalNameMessageID[    dwOriginalNameMessageID ] +
		                p_GID_Created[                  (time_t)(CreatedTime) + Work.dwPlayersCopied] +
		                p_LastUpdated[                  (time_t)(CreatedTime) + Work.dwPlayersCopied] +
		                p_Settings[                     ns1_5::p_Settings(SourceRow) ] );

                Work.wPercentComplete = 5 + (++(Work.dwPlayersCopied) * 4 / dwPlayerCount);
            }
        }
        break;

        case CWork::CopySavedGames:
        {
            if (!Work.pDestSavedGamesView) //1st call w/this step.
            {
                ShowStatusMessage(MID_CopyingSavedGames);

                Work.pDestSavedGamesView = new c4_View(g_pTheDB->GetView(ViewTypeStr(V_SavedGames)));
                Work.pDestSavedGamesView->RemoveAll();
                ASSERT(!Work.pSourceSavedGamesView);
                Work.pSourceSavedGamesView = new c4_View(Work.pDROD1_5Storage->View(ViewTypeStr(V_SavedGames)));
            }
            const DWORD dwSavedGameCount = Work.pSourceSavedGamesView->GetSize();
            if (Work.dwSavedGamesCopied == dwSavedGameCount)
            {
                //Set incremented ID to match
                {
                    c4_View SourceIncrementedIDs = Work.pDROD1_5Storage->View("IncrementedIDs");
                    c4_RowRef SourceRow = SourceIncrementedIDs.GetAt(0);
                    g_pTheDB->SetIncrementedID(p_SavedGameID, ns1_5::p_SavedGameID(SourceRow));
                }

                delete Work.pDestSavedGamesView; Work.pDestSavedGamesView = NULL;
                delete Work.pSourceSavedGamesView; Work.pSourceSavedGamesView = NULL;

                //Add the EndHold saved game after all other save records.
                if (Work.dwAddEndHoldSavePlayerID)
                {
                   CDbSavedGame *pEndHoldSavedGame = g_pTheDB->SavedGames.GetNew();
                   pEndHoldSavedGame->dwPlayerID = Work.dwAddEndHoldSavePlayerID;
                   pEndHoldSavedGame->eType = ST_EndHold;
                   pEndHoldSavedGame->bIsHidden = true;
                   pEndHoldSavedGame->dwRoomID = dwDugansDungeonLastRoomID;
                   pEndHoldSavedGame->Update();
                   delete pEndHoldSavedGame;
                }

                Work.wPercentComplete = 65;
                Work.SetNextStep();
            }
            else
            {
                c4_RowRef SourceRow = Work.pSourceSavedGamesView->GetAt(Work.dwSavedGamesCopied);

                c4_View ExploredRooms = ns1_5::p_ExploredRooms(SourceRow);
                c4_View ConqueredRooms = ns1_5::p_ConqueredRooms(SourceRow);

                //Fix broken demos 1 and 4.
                //Also need to change endTurnNo in the corresponding demo record.
                CDbCommands commands;
                commands = ns1_5::p_Commands(SourceRow);
                switch (Work.dwSavedGamesCopied)
                {
                    case 1:
                    {
                        static const UINT wIndexNo[] = {31, 62, 92, 409, 410, 411, 412, 413, 414, 415, 416};
                        static const BYTE commandNo[] = {CMD_WAIT, CMD_WAIT, CMD_WAIT, CMD_CC, CMD_CC, CMD_N, CMD_NE, CMD_C, CMD_C, CMD_SE, CMD_SE};
                        for (UINT wIndex=0; wIndex<11; ++wIndex)
                            InsertDemoCommand(commands, wIndexNo[wIndex], commandNo[wIndex]);
                        commands.Add(CMD_N, 0);
                        break;
                    }
                    case 4:
                    {
                       InsertDemoCommand(commands, 0, CMD_WAIT);
                       break;
                    }
                }
                DWORD dwCommandsSize;
	            BYTE *pbytCommands = commands.GetPackedBuffer(dwCommandsSize);
	            c4_Bytes CommandsBytes(pbytCommands, dwCommandsSize);

                Work.pDestSavedGamesView->Add(
                    p_SavedGameID[        ns1_5::p_SavedGameID(SourceRow) ] +
                    p_PlayerID[           ns1_5::p_PlayerID(SourceRow) ] +
                    p_RoomID[             ns1_5::p_RoomID(SourceRow) ] +
                    p_Type[               ns1_5::p_Type(SourceRow) ] +
                    p_CheckpointX[        ns1_5::p_CheckpointX(SourceRow) ] +
                    p_CheckpointY[        ns1_5::p_CheckpointY(SourceRow) ] +
                    p_IsHidden[           ns1_5::p_IsHidden(SourceRow) ] +
                    p_LastUpdated[        ns1_5::p_LastUpdated(SourceRow) ] +
                    p_StartRoomX[         ns1_5::p_StartRoomX(SourceRow) ] +
                    p_StartRoomY[         ns1_5::p_StartRoomY(SourceRow) ] +
                    p_StartRoomO[         ns1_5::p_StartRoomO(SourceRow) ] +
                    p_ExploredRooms[      ExploredRooms ] +
                    p_ConqueredRooms[     ConqueredRooms ] +
                    p_Created[            ns1_5::p_Created(SourceRow) ] +
                    p_Commands[           CommandsBytes ] );

                //Add "End Hold" saved record if a saved game exists for the final room.
                if (static_cast<DWORD>(ns1_5::p_RoomID(SourceRow)) == dwDugansDungeonLastRoomID)
                   Work.dwAddEndHoldSavePlayerID = static_cast<DWORD>(ns1_5::p_PlayerID(SourceRow));

                Work.wPercentComplete = 10 + (++(Work.dwSavedGamesCopied) * 50 / dwSavedGameCount);
            }
        }
        break;

        case CWork::CopyDemos:
        {
            if (!Work.pDestDemosView) //1st call w/this step.
            {
                ShowStatusMessage(MID_CopyingDemos); 

                Work.pDestDemosView = new c4_View(g_pTheDB->GetView(ViewTypeStr(V_Demos)));
                Work.pDestDemosView->RemoveAll();
                ASSERT(!Work.pSourceDemosView);
                Work.pSourceDemosView = new c4_View(Work.pDROD1_5Storage->View(ViewTypeStr(V_Demos)));
            }
            DWORD dwDemoCount = Work.pSourceDemosView->GetSize();
            if (Work.dwDemosCopied == dwDemoCount)
            {
                //Set incremented ID to match
                {
                    c4_View SourceIncrementedIDs = Work.pDROD1_5Storage->View("IncrementedIDs");
                    c4_RowRef SourceRow = SourceIncrementedIDs.GetAt(0);
                    g_pTheDB->SetIncrementedID(p_DemoID, ns1_5::p_DemoID(SourceRow));
                }
                delete Work.pDestDemosView; Work.pDestDemosView = NULL;
                delete Work.pSourceDemosView; Work.pSourceDemosView = NULL;

                Work.wPercentComplete = 90;
                Work.SetNextStep();
            }
            else
            {
                c4_RowRef SourceRow = Work.pSourceDemosView->GetAt(Work.dwDemosCopied);

                const DWORD dwDescriptionMessageID = ImportMessage(ns1_5::p_DescriptionMessageID(SourceRow));

                //Fix broken demos 1 and 4 (due to changed tar mother and roach queen spawning).
                const DWORD dwSavedGameID = ns1_5::p_SavedGameID(SourceRow);
                UINT wEndTurnNo = ns1_5::p_EndTurnNo(SourceRow);
                switch (dwSavedGameID)
                {
                   case 10001:
                      wEndTurnNo += 12;
                      break;
                   case 10004:
                      ++wEndTurnNo;
                      break;
                }

                Work.pDestDemosView->Add(
			              p_DemoID[                 ns1_5::p_DemoID(SourceRow) ] +
			              p_SavedGameID[            dwSavedGameID ] +
			              p_DescriptionMessageID[   dwDescriptionMessageID ] +
			              p_IsHidden[               ns1_5::p_IsHidden(SourceRow) ] + 
			              p_ShowSequenceNo[         ns1_5::p_ShowSequenceNo(SourceRow) ] +
			              p_BeginTurnNo[            ns1_5::p_BeginTurnNo(SourceRow) ] +
			              p_EndTurnNo[              wEndTurnNo ] +
			              p_NextDemoID[             ns1_5::p_NextDemoID(SourceRow) ] +
			              p_Checksum[               ns1_5::p_Checksum(SourceRow) ]);

                Work.wPercentComplete = 65 + (++(Work.dwDemosCopied) * 20 / dwDemoCount);
            }
        }
        break;

        case CWork::SaveAll:
        {
            ShowStatusMessage(MID_SavingChanges); 

            g_pTheDB->Commit();

            //Remove imported local player if name field is empty.
            CDbPlayer *pPlayer = g_pTheDB->Players.GetByID(1);
            if (pPlayer)
            {
               if (WCSlen((const WCHAR *)pPlayer->NameText)==0)
               {
                  g_pTheDB->Players.Delete(1, false);
                  g_pTheDB->Commit();
               }
               delete pPlayer;
            }
            g_pTheDB->SetPlayerID(0);

            Work.wPercentComplete = 95;
            Work.SetNextStep();
        }
        break;

        case CWork::CloseDROD1_5:
        {
            ShowStatusMessage(MID_Closing1_5Data);

            delete Work.pDROD1_5Storage; Work.pDROD1_5Storage = NULL;
            delete Work.pDROD1_5Stream; Work.pDROD1_5Stream = NULL;

            Work.wPercentComplete = 99;
            Work.SetNextStep();
        }
        break;

        case CWork::Done:
        {
            Work.wPercentComplete = 100;
            Work.eStatus = CWork::Completed;
        }
        break;

        default:
            ASSERTP(false, "Bad work state.");
            bSuccess = false;
    }

    if (!bSuccess)
    {
        g_pTheDB->Rollback();
        Work.eStatus = CWork::Failed;
    }
}

//**********************************************************************************************************
DWORD CSetupScreen::ImportMessage(
//Imports one message text from 1.5 source storage to 1.6 dest storage.  A different
//MessageID and MessageTextID will be assigned.
//
//Params:
    const DWORD dwSourceMessageID)        //(in)  Message in 1.5 data to import.
//
//Returns:
//MessageID in dest storage of new message.
const
{
    c4_View SourceView = Work.pDROD1_5Storage->View("MessageTexts");

    //Find message to import.  I'm going to assume that there is only one messagetext 
    //(language) for the message.
    c4_RowRef SourceRow = SourceView.GetAt(0);
    const long lRowCount = SourceView.GetSize();
    ASSERT(lRowCount > 0);
    //To speed up search, start looking where last search left off.
    static long lRowNo = 0;
    ASSERT(lRowNo < lRowCount);
    const long lStartRowNo = lRowNo;
    bool bWrapped = false;
    do {
        SourceRow = SourceView.GetAt(lRowNo);
        if (dwSourceMessageID == static_cast<DWORD>(ns1_5::p_MessageID(SourceRow)))
           break;
        ++lRowNo;
        if (lRowNo == lRowCount)
        {
           lRowNo = 0;
           bWrapped = true;
        }
    } while (lRowNo != lStartRowNo);
    const bool bInsertEmpty = (bWrapped && (lRowNo == lStartRowNo));

    CDbMessageText NewText;
    if (bInsertEmpty)
        NewText = wszEmpty;
    else
    {
        c4_Bytes MessageTextBytes(ns1_5::p_MessageText(SourceRow));
        NewText = (const WCHAR *)(MessageTextBytes.Contents());
    }
    return NewText.Flush();
}

// $Log: SetupScreen.cpp,v $
// Revision 1.20  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.19  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.18  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.17  2003/08/18 22:42:30  erikh2000
// Fixed problem with 1.5 imported demos out-of-synch with new spawn timing.
//
// Revision 1.16  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.15  2003/07/31 15:56:19  mrimer
// Changed data upgrade to only allow upgrading on the first run.
//
// Revision 1.14  2003/07/22 18:28:14  mrimer
// Removed mandatory between frame delay to speed process.  Sped up message text lookup.
//
// Revision 1.13  2003/07/19 02:23:10  mrimer
// Updated DB view access to use enum references rather than strings.
//
// Revision 1.12  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/07/03 21:46:18  mrimer
// Fixed upgrade bug (DD player creation time being set incorrectly) causing corrupted export data.
//
// Revision 1.10  2003/06/30 19:31:46  mrimer
// Add an EndHold saved game on upgrade if player has saved game in final room.
//
// Revision 1.9  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.8  2003/06/13 00:21:58  mrimer
// Non-local player records now keep their original creation timestamp on import (to preserve GUIDs).
//
// Revision 1.7  2003/06/09 23:55:07  mrimer
// Refined deleting the empty player record.
//
// Revision 1.6  2003/06/09 22:18:40  mrimer
// Fixed import bugs with fixed drod1_5.dat.
//
// Revision 1.5  2003/06/09 19:29:02  mrimer
// Added checks for importing only demos/saved games belonging to the local 1.5 player.
//
// Revision 1.4  2003/06/03 06:24:40  mrimer
// Fixed bugs relating to player ID import.
//
// Revision 1.3  2003/06/01 23:15:09  erikh2000
// Changed the drod.ini value checks a little.
//
// Revision 1.2  2003/05/28 23:06:01  erikh2000
// Setup screen now fully functional.  It upgrades data from v1.5 to v1.6.
//
