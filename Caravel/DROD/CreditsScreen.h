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

#ifndef CREDITSSCREEN_H
#define CREDITSSCREEN_H

#include "DrodScreen.h"
#include <FrontEndLib/LabelWidget.h>

#include <BackEndLib/Types.h>

#include <vector>

using namespace std;

class CScrollingTextWidget;
class CCreditsScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CCreditsScreen();

	virtual bool   Load();
	virtual void   Paint(bool bUpdateRect=true);
	virtual bool   SetForActivate();
	virtual void   Unload();
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
	virtual void	OnBetweenEvents();
	virtual bool	OnQuit();

private:
	void		AddLyricLabels();
	void		HideLyricLabels();
	void		ShowLyricLabel(UINT wLabelNo);

	SDL_Surface *				pBackgroundSurface;
	CScrollingTextWidget *		pScrollingText;
	vector<CLabelWidget *>		LyricLabels;

	static float			fScrollRateMultiplier;
	static UINT				wNormalScrollRate;
};

#endif //...#ifndef CREDITSSCREEN_H

// $Log: CreditsScreen.h,v $
// Revision 1.10  2003/08/16 00:39:49  mrimer
// Tweaking method definition.
//
// Revision 1.9  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.8  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.7  2003/05/23 21:43:02  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.6  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.5  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.4  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.3  2002/10/04 17:53:41  mrimer
// Added pause and speed control to this screen.
// Added mouse cursor display logic.
//
// Revision 1.2  2002/08/23 23:28:19  erikh2000
// Added credits text.
// Added "end of the game" song with synched lyric labels.
//
// Revision 1.1  2002/07/19 20:32:39  mrimer
// Initial check-in.
//
