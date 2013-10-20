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

#ifndef SELECTPLAYERSCREEN_H
#define SELECTPLAYERSCREEN_H

#include "DrodScreen.h"
#include "../DRODLib/DbPlayers.h"

class CListBoxWidget;
class CButtonWidget;
class CDialogWidget;
class CLabelWidget;
class CSelectPlayerScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CSelectPlayerScreen();
	virtual ~CSelectPlayerScreen();

	virtual void OnBetweenEvents();
   virtual void OnSelectChange(const DWORD dwTagNo);
	virtual void Paint(bool bUpdateRect=true);

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	DWORD		AddPlayer();
   DWORD    GetSelectedItem();
   void     HiResPrompt();
   void     SelectPlayer();
	void		SetPlayerID(DWORD &dwPlayerID);
   void     SetPlayerHold(const DWORD dwPlayerID) const;
   void     SetPlayerDesc(const DWORD dwPlayerID);
	void		PopulatePlayerListBox(CListBoxWidget *pPlayerListBoxWidget) const;

	CDialogWidget *pPlayerBox;
	CListBoxWidget *pPlayerListBoxWidget;
	CButtonWidget *pOKButton, *pNewPlayerButton, *pDeletePlayerButton,
			*pExportPlayerButton, *pImportPlayerButton;
	CLabelWidget *pPlayerHoldLabel, *pPlayerPositionLabel;
	static bool bFirst;	//whether first time entered
};

#endif //...#ifndef NEWPLAYERSCREEN_H

// $Log: SelectPlayerScreen.h,v $
// Revision 1.3  2003/07/17 01:54:38  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.2  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.1  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
//
