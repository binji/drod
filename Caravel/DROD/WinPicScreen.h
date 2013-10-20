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
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef WINPICSCREEN_H
#define WINPICSCREEN_H

#include "WinScreen.h"
#include <SDL.h>

class CWinPicScreen : public CWinScreen
{
protected:
	friend class CDrodScreenManager;
	friend class CWinScreen;

	CWinPicScreen(void) : CWinScreen(SCR_WinPic) { }

	void			FadeToNewPic(void);
	virtual bool	Load(void);
	bool			LoadPic(const char *pszBMPName);
	virtual void	Paint(bool bUpdateRect=true);
	void			SetScrollingTextRect(const SDL_Rect &NewRect);
	virtual void	Unload(void);

private:
	virtual void	DrawBeethroTalking(void) { }
	virtual void	DrawBeethroSilent(void) { }
	
	string			strPicName;
};

#endif //...#ifndef WINPICSCREEN_H

// $Log: WinPicScreen.h,v $
// Revision 1.5  2003/07/17 01:54:38  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.4  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.3  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.2  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.1  2002/10/01 22:58:39  erikh2000
// Initial check-in.
//
