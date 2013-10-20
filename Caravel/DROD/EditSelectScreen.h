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

#ifndef EDITSELECTSCREEN_H
#define EDITSELECTSCREEN_H

#include "DrodScreen.h"

class CDbHold;
class CDbLevel;
class CDbRoom;
class CListBoxWidget;
class CMapWidget;
class CEditRoomWidget;
class CScalerWidget;
class CEditSelectScreen : public CDrodScreen
{
public:
   void     SetToCopiedHold(CDbHold *pHold, CDbLevel *pLevel);

protected:
	friend class CDrodScreenManager;

	CEditSelectScreen();

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();
   void     FreeMembers();

private:
	virtual void	OnClick(const DWORD dwTagNo);
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
	virtual void	OnSelectChange(const DWORD dwTagNo);
	virtual void	Paint(bool bUpdateRect=true);

	DWORD		AddHold();
   void     CopyHold();
   void     DeleteHold();
   bool     ModifyHold();
	bool		SelectFirstHold();
	bool		SelectHold(const DWORD dwHoldID);

	DWORD		AddLevel();
   void     DeleteLevel();
   void     MakeLevelFirst();
	bool		SelectFirstLevel();
	void		SelectLevel(const DWORD dwLevelID);

	DWORD		AddRoom(const DWORD dwRoomX, const DWORD dwRoomY);
	void		SelectLevelEntranceRoom();
	void		SelectRoom(const DWORD dwRoomX, const DWORD dwRoomY);
	void		SetSelectedRoom(CDbRoom *pRoom);
	void		SetRoomStyle(const UINT wStyleID);

	bool		PasteLevel();
	void		PopulateLevelListBox();
	void		PopulateHoldListBox();
	void		PopulateStyleListBox();
	bool		SetWidgets();
	void		SetWidgetStates();

	//List of holds/levels/styles.
	CListBoxWidget *	pLevelListBoxWidget;
	CListBoxWidget *	pHoldListBoxWidget;
	CListBoxWidget *	pStyleListBoxWidget;

	//Currently selected hold/level/room.
	CDbHold *			pSelectedHold;
	CDbLevel *			pSelectedLevel;
	CDbRoom *			pSelectedRoom;

	CDbLevel *			pLevelCopy;	//level being cut/copied
	bool					bCopyingLevel;	//whether being copied, or cut

	//Level/room display.
	CMapWidget *		pMapWidget;
	CEditRoomWidget *	pRoomWidget;
	CScalerWidget *	pScaledRoomWidget;

	SDL_Surface *		pBackgroundSurface;
};

#endif //...#ifndef EDITSELECTSCREEN_H

// $Log: EditSelectScreen.h,v $
// Revision 1.14  2003/07/21 22:08:42  mrimer
// Added ModifyHold().  Now require making personal copy of hold before modifying anything.  Made hold desc. text box multi-line.
//
// Revision 1.13  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.12  2003/07/15 00:35:41  mrimer
// Fixed bug: widgets not updating when a modified copy of a hold is created.  Added SetToCopiedHold().
// Added buttons for hold and level ops: Refactored code into CopyHold(), DeleteHold(), DeleteLevel(), and MakeLevelFirst().
//
// Revision 1.11  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.10  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.8  2003/06/09 23:53:28  mrimer
// Added FreeMembers().  Reset hold/level/room vars on exiting the editor.
//
// Revision 1.7  2003/06/09 19:30:28  mrimer
// Fixed some level editor bugs.
//
// Revision 1.6  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.5  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.4  2003/05/03 23:39:25  mrimer
// Restricted hold viewing/editing privileges to the hold author, or players that completed the hold.
// Enforced non-empty naming of holds and levels.  Disallowed remaining on the edit select screen when no holds/levels exist to edit.
//
// Revision 1.3  2002/12/22 02:23:34  mrimer
// Added level cut, copy, and paste support.
//
// Revision 1.2  2002/11/22 22:01:36  mrimer
// Renamed OnKeyUp() to OnKeyDown().
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
