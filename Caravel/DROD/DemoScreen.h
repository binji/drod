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

#ifndef DEMOSCREEN_H
#define DEMOSCREEN_H

#include "../DRODLib/DbDemos.h"
#include <BackEndLib/Types.h>

#include "GameScreen.h"

//***************************************************************************************
class CDemoScreen : public CGameScreen
{
public:
	bool		LoadDemoGame(const DWORD dwDemoID);
   void     SetReplayOptions(bool bChangeSpeed);

protected:
	friend class CDrodScreenManager;

	CDemoScreen();
	virtual ~CDemoScreen();

	virtual bool   SetForActivate();

private:
	virtual void   OnBetweenEvents();
	virtual void   OnKeyDown(const DWORD dwTagNo,
			const SDL_KeyboardEvent &KeyboardEvent);
	virtual void   OnMouseUp(const DWORD dwTagNo,
			const SDL_MouseButtonEvent &Button);

	COMMANDNODE *	pCurrentCommand;
	DWORD			dwNextAnimateTime, dwNextCommandTime;
	CDbDemo *		pDemo;

   bool                 bCanChangeSpeed;
	static float			fScrollRateMultiplier;
	static UINT				wNormalScrollRate;
};

#endif //...#ifndef DEMOSCREEN_H

// $Log: DemoScreen.h,v $
// Revision 1.15  2003/09/22 20:37:48  mrimer
// Fixed bug: mouse click registered on two screens.
//
// Revision 1.14  2003/07/31 21:49:02  schik
// Added changing of playback speed of demos (except those run from the title screen)
//
// Revision 1.13  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.12  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.11  2003/05/23 21:43:02  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.10  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.9  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.8  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.7  2002/10/21 20:47:23  mrimer
// A standard keypress or mouse click will end demo playback.
//
// Revision 1.6  2002/10/11 01:55:16  erikh2000
// Removed testing code.
//
// Revision 1.5  2002/07/22 01:00:29  erikh2000
// Made destructor declaration virtual just so that the virtual status is more obvious.
//
// Revision 1.4  2002/07/17 20:44:14  erikh2000
// Added a temporary method used for displaying a demo scene (range of commands) on the demo screen.
//
// Revision 1.3  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.2  2002/06/15 18:37:05  erikh2000
// Fixed places where a CDbBase-derived class was not being deleted.
//
// Revision 1.1  2002/05/15 01:22:33  erikh2000
// Initial check-in.
//
