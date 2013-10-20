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
 * Portions created by the Initial Developer are Copyright (C) 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributors:
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef HOLDSELECTSCREEN_H
#define HOLDSELECTSCREEN_H

#include "DrodScreen.h"

class CButtonWidget;
class CLabelWidget;
class CListBoxWidget;
class CHoldSelectScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CHoldSelectScreen();
	virtual ~CHoldSelectScreen();

	virtual void   Paint(bool bUpdateRect=true);
	virtual void   OnBetweenEvents();
	DWORD		GetSelectedItem();

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	virtual void   OnSelectChange(const DWORD dwTagNo);
	void		PopulateHoldListBox() const;
   void     SetHoldDesc(const DWORD dwHoldID);
	void		GetHoldID(DWORD &dwHoldID);

	CDialogWidget *pHoldBox;
	CListBoxWidget *pHoldListBoxWidget;
	CLabelWidget *pHoldDesc;
	CLabelWidget *pAuthorName;
	CButtonWidget *pOKButton, *pExportButton, *pDeleteButton;
};

#endif

// $Log: HoldSelectScreen.h,v $
// Revision 1.7  2003/07/24 04:03:41  mrimer
// Enabled double-clicking the list-box on the dialog to exit it with OK return value.
//
// Revision 1.6  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.5  2003/07/01 20:33:39  mrimer
// Added delete button member var.  Initialize member vars in constructor.  Disable delete hold button when not applicable.  Show hourglass cursor while deleting hold.
//
// Revision 1.4  2003/06/12 18:07:41  mrimer
// Fixed display logic bug.
//
// Revision 1.3  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.2  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.1  2002/12/22 02:26:45  mrimer
// Initial check-in.
//
// Revision 1.1  2002/11/15 02:42:36  mrimer
// Initial check-in.
//
