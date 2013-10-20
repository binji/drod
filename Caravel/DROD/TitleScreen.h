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

#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#include "DrodScreen.h"

#include <BackEndLib/Types.h>
#include <BackEndLib/IDList.h>

//Menu selection.
enum TitleSelection
{
	MNU_CONTINUE=0,
	MNU_RESTORE,
	MNU_SETTINGS,
	MNU_HELP,
	MNU_DEMO,
	MNU_QUIT,
	MNU_BUILD,
	MNU_WHO,
	MNU_WHERE,

	MNU_COUNT,

	MNU_UNSPECIFIED = -1
};

class CTitleScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CTitleScreen();

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	void		Animate();
	void		AnimateFlag();
	void		AnimateWaves();
	void		BlitEmptyRestoreBrick() const;
	
	void		DrawMainHead(int nX, int nY, UINT wO);
	void		DrawSecondHead(int nX, int nY, UINT wO);
	void		DrawThirdHead(int nX, int nY, UINT wO);
	void		DrawFourthHead(int nX, int nY, UINT wO);
	void		DrawFifthHead(int nX, int nY, UINT wO);	
	
	void		DrawBrick(TitleSelection wMenuPos);
	void		DrawHighlightedBrick(TitleSelection wMenuPos);
	void		DrawPushedBrick(TitleSelection wMenuPos);

	TitleSelection		GetMenuPosFromCoords(int nX, int nY) const;
	DWORD		GetNextDemoID();
	void		HighlightMenuItem(TitleSelection wMenuPos);
	virtual void	Paint(bool bUpdateRect=true);
	void		PaintHeads();
	SCREENTYPE	ProcessMenuSelection(TitleSelection wMenuPos);
	
	void		LoadDemos();

	virtual void	OnBetweenEvents();
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &KeyboardEvent);
	virtual void	OnMouseMotion(const DWORD dwTagNo, const SDL_MouseMotionEvent &MotionEvent);
	virtual void	OnMouseUp(const DWORD dwTagNo, const SDL_MouseButtonEvent &ButtonEvent);

	void		SetSavedGameExists();

	SDL_Surface *	pTitleSurface;
	SDL_Surface *	pHeadsSurface;

	//Demo display.
	CIDList	ShowSequenceDemoIDs;
	IDNODE *	pCurrentDemoID;
	DWORD		dwCurrentDemoHoldID;	//demos are shown from the selected hold only

	int		nHeadOffsetX;
	UINT		wHeadsO;
	TitleSelection	wSelectedPos;
	DWORD		dwFirstPaint;
	bool		bSavedGameExists;
};

#endif //...#ifndef TITLESCREEN_H

// $Log: TitleScreen.h,v $
// Revision 1.22  2003/07/17 01:54:38  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.21  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.20  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.19  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.18  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.17  2003/04/13 02:03:18  mrimer
// Changed show demo selection to only show demos from the current hold.
// Will reload the demo show list when a hold author plays their hold (possibly altering the show list).
//
// Revision 1.16  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.15  2003/02/01 23:53:53  erikh2000
// Fixed errors in rect coordinates.
// Changed hot keys to match button graphics.
//
// Revision 1.14  2002/12/22 02:17:00  mrimer
// Removed hold selection code to CHoldSelectScreen.
//
// Revision 1.13  2002/11/22 02:13:17  mrimer
// Added SetSavedGameExists() to handle saved games for multiple holds.  Updated hold selection code.
//
// Revision 1.12  2002/11/15 02:17:08  mrimer
// Added TitleSelection enumeration.  Made virtual methods explicit.
//
// Revision 1.11  2002/08/25 20:15:50  erikh2000
// Added code to draw empty restore brick.
//
// Revision 1.10  2002/07/19 20:27:23  mrimer
// Added bSavedGameExists.
//
// Revision 1.9  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.8  2002/06/21 01:28:00  erikh2000
// Title screen now plays demos from show sequence.
//
// Revision 1.7  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.6  2002/06/07 17:46:43  mrimer
// Fixed mouse button handling.
//
// Revision 1.5  2002/06/02 10:48:24  erikh2000
// Brand new title screen--extensive changes.
//
// Revision 1.4  2002/05/21 21:38:28  erikh2000
// Added stub method to return next demo ID in sequence.
//
// Revision 1.3  2002/04/19 21:44:52  erikh2000
// Renamed SDL event handling methods to prevent override compile error.
//
// Revision 1.2  2002/04/09 10:05:41  erikh2000
// Fixed revision log macro.
//
