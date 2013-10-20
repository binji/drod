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
 * 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef NEWPLAYERSCREEN_H
#define NEWPLAYERSCREEN_H

#include "DrodScreen.h"
#include "../DRODLib/DbPlayers.h"

class CListBoxWidget;
class CButtonWidget;
class CDialogWidget;
class CLabelWidget;
class CNewPlayerScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CNewPlayerScreen();
	virtual ~CNewPlayerScreen();

	virtual void OnBetweenEvents();
	virtual void Paint(bool bUpdateRect=true);

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	DWORD		AddPlayer();
   void     HiResPrompt();
   void     SetPlayerHold(const DWORD dwPlayerID) const;

	CDialogWidget *pPlayerBox;
	CButtonWidget *pOKButton, *pImportPlayerButton;
   CTextBoxWidget *pNameWidget;
};

#endif //...#ifndef NEWPLAYERSCREEN_H

// $Log: NewPlayerScreen.h,v $
// Revision 1.14  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.13  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.12  2003/07/03 08:03:41  mrimer
// Removed unneeded methods.
//
// Revision 1.11  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
// Revision 1.10  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/06/26 17:46:33  mrimer
// Added display of selected player's current location.
//
// Revision 1.8  2003/06/21 10:40:55  mrimer
// Added SetPlayerHold() to default the active hold to the latest the selected player played.
//
// Revision 1.7  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.6  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.5  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.4  2002/12/22 02:27:32  mrimer
// Added SetForActivate().  Added some widget vars.
//
// Revision 1.3  2002/11/15 02:36:24  mrimer
// Added multiple player support, including: adding, deleting and selecting players.
//
// Revision 1.2  2002/09/05 20:44:00  erikh2000
// Added a destructor with some cleanup code.
//
// Revision 1.1  2002/09/05 18:53:40  mrimer
// Initial check-in.
//
