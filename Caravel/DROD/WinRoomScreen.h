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

#ifndef WINROOMSCREEN_H
#define WINROOMSCREEN_H

#include "WinScreen.h"
#include "../DRODLib/CueEvents.h"

class CRoomWidget;
class CWinRoomScreen : public CWinScreen
{
protected:
	friend class CDrodScreenManager;
	friend class CWinScreen;

	CWinRoomScreen()
      : CWinScreen(SCR_WinRoom)
      , pRoomWidget(NULL)
		, dwNextCommandTime(0L), pCurrentCommand(NULL)
		, lLastPupilMove(0), lLastMouthMove(0), bMouthOpen(false)
	{
		ResetPupils();
	}

   virtual ~CWinRoomScreen() {}

	virtual void	DrawBeethroTalking();
	virtual void	DrawBeethroSilent();
	virtual bool	Load();
	bool			LoadDemoScene(const DWORD dwDemoID, CDemoScene &Scene);
	bool			LoadRoomStart(const DWORD dwRoomID);
	virtual void	OnBetweenEvents();
	virtual void	Paint(bool bUpdateRect=true);
	virtual void	Unload();

	CRoomWidget *	pRoomWidget;

private:
	void			DrawEyes();
	void			DrawPupils();
	void			DrawRovingPupils();
	void			ProcessCommand(int nCommand);
	void			ProcessCueEventsBeforeRoomDraw(CCueEvents &CueEvents);
	void			ResetPupils();

	DWORD			dwNextCommandTime;
	COMMANDNODE *	pCurrentCommand;
	SDL_Rect		CurLeftPupilDest, CurRightPupilDest;
	Uint32			lLastPupilMove, lLastMouthMove;
	bool			bMouthOpen;
};

#endif //...#ifndef WINROOMSCREEN_H

// $Log: WinRoomScreen.h,v $
// Revision 1.11  2003/07/17 01:54:38  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.10  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.8  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.7  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.6  2002/09/06 20:09:23  erikh2000
// Added method to show room start in the room widget.
//
// Revision 1.5  2002/09/04 22:31:57  erikh2000
// Added code to play demos.
//
// Revision 1.4  2002/09/04 21:47:56  mrimer
// Moved Animate() code into base win screen class.
//
// Revision 1.3  2002/09/03 23:50:47  mrimer
// Added DrawEyes().
//
// Revision 1.2  2002/09/03 21:43:33  erikh2000
// Got the first part of the script working--Beethro talks, audience talks, text scrolls, the screen switches from audience to room view, a room is shown in the room widget.
//
// Revision 1.1  2002/07/11 20:58:37  mrimer
// Initial check-in.
//
