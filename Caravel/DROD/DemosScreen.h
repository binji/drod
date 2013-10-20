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

#ifndef DEMOSSCREEN_H
#define DEMOSSCREEN_H

#include "DrodScreen.h"

#include <BackEndLib/Wchar.h>
#include <BackEndLib/Types.h>

#include <string>
using namespace std;

class CCurrentGame;
class CFrameWidget;
class CLabelWidget;
class CListBoxWidget;
class COptionButtonWidget;
class CRoomWidget;
class CScalerWidget;

class CDemosScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CDemosScreen();
   virtual ~CDemosScreen();

	virtual bool   Load();
	virtual void   Unload();
	virtual bool   SetForActivate();

private:
	bool		GetItemTextForDemo(DWORD dwDemoID,	WSTRING &wstrText) const;
	void		OnClick(const DWORD dwTagNo);
	void		OnSelectChange(const DWORD dwTagNo);
	virtual void	Paint(bool bUpdateRect = true);
	void		PopulateDemoListBox();
	void		SetWidgetsToDemo(DWORD dwDemoID);
	
	SDL_Surface *		pBackgroundSurface;
	CListBoxWidget *	pDemoListBoxWidget;
	CRoomWidget *		pRoomWidget;
	CScalerWidget *		pScaledRoomWidget;
	CCurrentGame *		pDemoCurrentGame;
	CLabelWidget *		pAuthorWidget, *pCreatedWidget, *pDurationWidget;
	CLabelWidget *		pDescriptionWidget;
	COptionButtonWidget *pOptionButton;
	CLabelWidget *		pLBoxHeaderWidget;
	CLabelWidget *		pNoDemoWidget;
	CFrameWidget *		pDetailsFrame;
};

#endif //...#ifndef DEMOSSCREEN_H

// $Log: DemosScreen.h,v $
// Revision 1.20  2003/07/31 21:34:14  mrimer
// Added demo duration display.
//
// Revision 1.19  2003/07/24 21:30:11  mrimer
// Added virtual destructor.
//
// Revision 1.18  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.17  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.16  2003/05/23 21:43:02  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.15  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.14  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.13  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.12  2003/04/29 11:13:18  mrimer
// Added PopulateDemoListBox().  Implemented demo import.
//
// Revision 1.11  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.10  2003/02/16 20:32:17  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.9  2002/11/15 02:47:50  mrimer
// Added capability of selecting a recorded demo for public viewing.
//
// Revision 1.8  2002/07/20 23:07:47  erikh2000
// Changed code to use new CScalerWidget to draw scaled room.
//
// Revision 1.7  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.6  2002/06/21 03:31:56  erikh2000
// Made some cosmetic changes involving what widgets are shown when no demo is selected.
//
// Revision 1.5  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.4  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.3  2002/06/13 22:57:47  erikh2000
// Changed code so that it receives dates in a wstring instead of a preallocated wchar buffer.
// Changed method for concatenating item text to return a wstring.
//
// Revision 1.2  2002/06/09 01:11:48  mrimer
// Added help button.
// Rearranged includes.
//
// Revision 1.1  2002/05/15 01:22:33  erikh2000
// Initial check-in.
//
