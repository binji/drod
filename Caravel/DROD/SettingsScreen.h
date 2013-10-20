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

#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include "DrodScreen.h"

#include <FrontEndLib/TextBoxWidget.h>
#include <FrontEndLib/KeypressDialogWidget.h>
#include "../DRODLib/DbPlayers.h"
#include <BackEndLib/Types.h>

//Definable command constants.  Must be in same order as associated widgets and
//their tags.
enum DCMD
{
	DCMD_NW = 0,
	DCMD_N,
	DCMD_NE,
	DCMD_W,
	DCMD_Wait,
	DCMD_E,
	DCMD_SW,
	DCMD_S,
	DCMD_SE,
	DCMD_C,
	DCMD_CC,
	DCMD_Restart,

	DCMD_Count
};

//*****************************************************************************************
class CSettingsScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CSettingsScreen();
   virtual ~CSettingsScreen();

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	bool		AllCommandsAreAssignedToKeys(const CDbPackedVars &Settings) const;
	bool		GetCommandKeyRedefinition(const DCMD eCommand, const SDLKey CurrentKey, SDLKey &NewKey);
	virtual void OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
	virtual void OnClick(const DWORD dwTagNo);
	virtual void OnDragUp(const DWORD dwTagNo, const SDL_MouseButtonEvent &Button);
	virtual void Paint(bool bUpdateRect=true);
	void		RestorePlayerSettings();
	void		SetUnspecifiedPlayerSettings(CDbPackedVars &Settings);
	void		SynchScreenSizeWidget();
	void		UpdatePlayerDataFromWidgets(CDbPlayer *pPlayer);
	void		UpdateWidgetsFromPlayerData(CDbPlayer *pPlayer);

	SDL_Surface *			pBackgroundSurface;
	CKeypressDialogWidget *	pDialogBox;
	CWidget	*				pCommandLabelWidgets[DCMD_Count];
	CDbPlayer *				pCurrentPlayer;

	CTextBoxWidget * pNameWidget;
};

#endif //...#ifndef SETTINGSSCREEN_H

// $Log: SettingsScreen.h,v $
// Revision 1.29  2003/09/11 02:29:12  mrimer
// Fixed bug: can erase player name.  Removed unneeded error message methods.
//
// Revision 1.28  2003/07/22 01:01:08  erikh2000
// Removed e-mail related widgets.
//
// Revision 1.27  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.26  2003/07/03 08:04:13  mrimer
// Added destructor.
//
// Revision 1.25  2003/06/16 20:27:19  mrimer
// Added OnDragUp().
//
// Revision 1.24  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.23  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.22  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.21  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.20  2002/10/17 17:19:54  mrimer
// Added SynchScreenSizeWidget() to keep size setting current with actual screen size.
//
// Revision 1.19  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.18  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.17  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.16  2002/06/11 17:43:31  mrimer
// Separated widget behavior into DoAction().
// Added RestorePlayerSettings().
//
// Revision 1.15  2002/06/09 06:29:59  erikh2000
// Settings load and save from database instead of DROD.ini.
//
// Revision 1.14  2002/06/07 22:56:55  mrimer
// Added InvalidCommandKeyError().
//
// Revision 1.13  2002/06/07 18:20:16  mrimer
// Made Commands frame dynamically fit the number of command buttons.
//
// Revision 1.12  2002/06/05 03:08:26  mrimer
// Refactored focus info into CScreen.
//
// Revision 1.11  2002/06/03 22:56:29  mrimer
// Added widget focusability.
//
// Revision 1.10  2002/05/24 14:14:37  mrimer
// Focus widgets now inherit from CFocusWidget.
//
// Revision 1.9  2002/05/24 13:55:13  mrimer
// Added using TAB to switch focus to next widget.
//
// Revision 1.8  2002/05/23 22:45:28  mrimer
// Made bCommandsAltered local to Activate().
//
// Revision 1.7  2002/05/23 22:28:22  mrimer
// Load and save keyboard commands from DROD.ini.
//
// Revision 1.6  2002/05/21 21:42:16  mrimer
// Implemented TextBoxWidget.
//
// Revision 1.5  2002/05/21 18:11:24  mrimer
// Added changing command key mappings in settings screen.
//
// Revision 1.4  2002/05/21 13:50:32  mrimer
// Added GetCommandKeyRedefinition().  Selecting command buttons calls it.
//
// Revision 1.3  2002/04/29 00:21:34  erikh2000
// Disabled "really quit?" dialog because I was having trouble clicking on it with the cursor gone.
//
// Revision 1.2  2002/04/19 21:58:17  erikh2000
// SettingsScreen now loads and displays a monolithic 640x480 background.
// Widgets to display title and command-redefinition interface are present.
//
// Revision 1.1  2002/04/10 00:26:30  erikh2000
// Initial check-in.
//
