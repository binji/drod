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
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef RESTORESCREEN_H
#define RESTORESCREEN_H

#include "DrodScreen.h"
#include "RoomWidget.h"
#include "MapWidget.h"
#include <FrontEndLib/ListBoxWidget.h>
#include <FrontEndLib/ScalerWidget.h>
#include "../DRODLib/CurrentGame.h"
#include <BackEndLib/Types.h>

using namespace std;

typedef struct tagCheckpointButton
{
	CWidget *	pButton;
	DWORD		dwSavedGameID;
} CHECKPOINT;

typedef list<void *> CHECKPOINT_LIST; //"void *" s/b "CHECKPOINT *", but it gave me annoying warnings.

class CRestoreScreen : public CScreen
{
protected:
	friend class CDrodScreenManager;

	CRestoreScreen();

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	void		ChooseCheckpoint(const DWORD dwTagNo);
	void		ChooseLevelLatest(const DWORD dwLevelID);
	void		ChooseLevelSavedGame(const DWORD dwSavedGameID);
	void		ChooseLevelStart(const DWORD dwLevelID);
	void		ChooseRoomLatest(const DWORD dwRoomX, const DWORD dwRoomY);
	void		ChooseRoomSavedGame(const DWORD dwSavedGameID);
	void		ChooseRoomStart(const DWORD dwRoomX, const DWORD dwRoomY);
	virtual void   OnClick(const DWORD dwTagNo);
	virtual void   OnSelectChange(const DWORD dwTagNo);
	virtual void   Paint(bool bUpdateRect=true);
	void		PopulateLevelListBoxFromSavedGames();
	void		SetCheckpoints();
	bool		SetWidgets();
	void		ShowCheckpointButtonsForSavedGame(const DWORD dwSavedGameID);
   void     UpdateWidgets();

	DWORD			dwSelectedSavedGameID;
	DWORD			dwLastGameID;
   UINT        wConqueredRooms;

	CCurrentGame *	pCurrentRestoreGame;
	CRoomWidget *	pRoomWidget;
	CScalerWidget *	pScaledRoomWidget;
	CMapWidget *	pMapWidget;
	CListBoxWidget *pLevelListBoxWidget;
	SDL_Surface *	pBackgroundSurface;

	CHECKPOINT_LIST Checkpoints;
};

#endif //...#ifndef RESTORESCREEN_H

// $Log: RestoreScreen.h,v $
// Revision 1.21  2003/08/25 22:06:58  mrimer
// Added UpdateWidgets(), and simplified saved game representation.
//
// Revision 1.20  2003/07/17 01:54:38  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.19  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.18  2003/05/30 04:04:49  mrimer
// Fixed some bugs in the restore logic (again, for the last time).
//
// Revision 1.17  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.16  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.15  2003/05/21 03:06:44  mrimer
// Suppressed "Really Restore?" prompt when it doesn't relate to the selected level.
//
// Revision 1.14  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.13  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.12  2002/11/15 02:32:47  mrimer
// Made several parameters const.
//
// Revision 1.11  2002/07/20 23:16:54  erikh2000
// Screen now uses new CScalerWidget to draw scaled room.
//
// Revision 1.10  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.9  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.8  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.7  2002/06/05 23:57:33  mrimer
// Removed unneeded virtual functions.
//
// Revision 1.6  2002/05/12 03:18:50  erikh2000
// Many changes--basically bug fixes and checkpoint restore interface.
//
// Revision 1.5  2002/05/10 22:39:04  erikh2000
// Many changes related to logic for updating widgets.
//
// Revision 1.4  2002/04/29 00:17:20  erikh2000
// Several additions and changes that make the screen do more than previously.  You can now restore games from beginning of any explored level or room.
//
// Revision 1.3  2002/04/25 09:29:09  erikh2000
// Finished widget placement code in Load().
//
// Revision 1.2  2002/04/19 21:57:11  erikh2000
// RestoreScreen now loads and displays a monolithic 640x480 background.
//
// Revision 1.1  2002/04/10 00:26:30  erikh2000
// Initial check-in.
//
