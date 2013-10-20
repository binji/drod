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

//NOTE: The import code is full of correct assumptions about what DROD 1.5 data will contain.  It shouldn't
//be reused for 1.6 imports.

#ifndef SETUPSCREEN_H
#define SETUPSCREEN_H

#include "DrodScreen.h"
#include "../DRODLib/Db.h"
#include <FrontEndLib/ProgressBarWidget.h>

class CSetupScreen : public CDrodScreen
{
    friend class CDrodScreenManager;

private:
    SDL_Surface *			pBackgroundSurface;
    CProgressBarWidget *    pProgressWidget;

protected:
	CSetupScreen() : CDrodScreen(SCR_Setup), pBackgroundSurface(NULL), pProgressWidget(NULL) { }
	virtual ~CSetupScreen() { }

   virtual bool   Load();
   virtual void   OnBetweenEvents();
   virtual void   OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
   virtual bool   OnQuit();
   virtual bool   SetForActivate();
   virtual void   Unload();

public:
    virtual void  Paint(bool bUpdateRect=true);

private:
    bool            StartImport(const WCHAR *pszSourcePath);
    void            DoWork();
    DWORD           ImportMessage(DWORD dwSourceMessageID) const;

private:
    class CWork
    {
    public:
        typedef enum tagStatus {NotStarted = 0, InProgress, Completed, Failed} STATUS;
        typedef enum tagStep {OpenDROD1_5, CopyPlayers, CopySavedGames, CopyDemos,
                SaveAll, CloseDROD1_5, Done} STEP;

        CWork() :
            eStep(OpenDROD1_5),
            eStatus(NotStarted),
            wPercentComplete(0), 
            pDROD1_5Stream(NULL),
            pDROD1_5Storage(NULL),
            pSourcePlayersView(NULL),
            pDestPlayersView(NULL),
            dwPlayersCopied(0),
            pSourceSavedGamesView(NULL),
            pDestSavedGamesView(NULL),
            dwSavedGamesCopied(0),
            pSourceDemosView(NULL),
            pDestDemosView(NULL),
            dwDemosCopied(0),
            dwAddEndHoldSavePlayerID(0)
         { }
        ~CWork() 
        {
            delete pSourceDemosView;
            delete pDestDemosView;
            delete pSourceSavedGamesView;
            delete pDestSavedGamesView;
            delete pSourcePlayersView;
            delete pDestPlayersView;
            delete pDROD1_5Storage;
            delete pDROD1_5Stream;
        }
        void SetNextStep()
        {
            this->eStep = static_cast<STEP>( static_cast<int>(this->eStep) + 1 );
        }

        STEP        eStep;
        STATUS      eStatus;
        UINT        wPercentComplete;
        WSTRING     wstrSourcePath;

        CGameStream *   pDROD1_5Stream;
        c4_Storage *    pDROD1_5Storage;

        c4_View *       pSourcePlayersView;
        c4_View *       pDestPlayersView;
        DWORD           dwPlayersCopied;

        c4_View *       pSourceSavedGamesView;
        c4_View *       pDestSavedGamesView;
        DWORD           dwSavedGamesCopied;

        c4_View *       pSourceDemosView;
        c4_View *       pDestDemosView;
        DWORD           dwDemosCopied;

        DWORD            dwAddEndHoldSavePlayerID;
    };
    CWork Work;
};

#endif //...#ifndef SETUPCREEN_H

// $Log: SetupScreen.h,v $
// Revision 1.8  2003/07/22 18:28:14  mrimer
// Removed mandatory between frame delay to speed process.  Sped up message text lookup.
//
// Revision 1.7  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.6  2003/06/30 19:31:46  mrimer
// Add an EndHold saved game on upgrade if player has saved game in final room.
//
// Revision 1.5  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/09 22:19:47  mrimer
// Removed unneeded var.
//
// Revision 1.3  2003/06/03 06:24:40  mrimer
// Fixed bugs relating to player ID import.
//
// Revision 1.2  2003/05/28 23:06:01  erikh2000
// Setup screen now fully functional.  It upgrades data from v1.5 to v1.6.
//
// Revision 1.1  2003/05/25 22:47:33  erikh2000
// Added setup screen--doesn't do anything yet.
//
