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

#ifndef WINAUDIENCESCREEN_H
#define WINAUDIENCESCREEN_H

#include "WinScreen.h"

#include <BackEndLib/Assert.h>

class CWinAudienceScreen : public CWinScreen
{
protected:
	friend class CDrodScreenManager;

	CWinAudienceScreen() : CWinScreen(SCR_WinAudience),
		lLastPupilMove(0), lLastMouthMove(0), lLastArmMove(0), bMouthOpen(false)
	{
		ResetPupils();
	}

	virtual void Paint(bool bUpdateRect=true);

	virtual void DrawBeethroTalking();
	virtual void DrawBeethroSilent();

	virtual bool Load();
	virtual void Unload();

private:
	void			DrawPupils();
	void			DrawRovingPupils();
	void			DrawArm();
	void			DrawSpeechSquare();
	void			ResetArm() {nArmFrame = 0;}
	void			ResetPupils();

	SDL_Rect CurLeftPupilDest, CurRightPupilDest;
	Uint32 lLastPupilMove, lLastMouthMove, lLastArmMove;
	bool bMouthOpen;
	int nArmFrame;
};

#endif //...#ifndef WINAUDIENCESCREEN_H

// $Log: WinAudienceScreen.h,v $
// Revision 1.9  2003/07/17 01:54:38  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.8  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.7  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.6  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.5  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.4  2002/09/04 22:31:14  erikh2000
// Removed virtual ShowDemoScene() method.
//
// Revision 1.3  2002/09/04 21:47:56  mrimer
// Moved Animate() code into base win screen class.
//
// Revision 1.2  2002/09/03 21:43:33  erikh2000
// Got the first part of the script working--Beethro talks, audience talks, text scrolls, the screen switches from audience to room view, a room is shown in the room widget.
//
// Revision 1.1  2002/07/11 20:58:37  mrimer
// Initial check-in.
//
