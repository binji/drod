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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef ROOMSCREEN_H
#define ROOMSCREEN_H

#include "DrodScreen.h"
#include "DrodScreenManager.h"
#include "../DRODLib/DbPackedVars.h"

//Base class for displaying a room,
//including basically everything that goes on "gamescreen.bmp"
//(i.e. RoomWidget, Scroll, Map, and Sign).

const DWORD TAG_ROOM = 1000;
const DWORD TAG_MAP = 1001;

class CMapWidget;
class CRoomWidget;
class CLabelWidget;
class CCurrentGame;
class CRoomScreen : public CDrodScreen
{
public:
	virtual bool   Load(CCurrentGame *pCurrentGame);
	virtual void   Unload();
	static void		SetMusicStyle(const UINT wStyle);

protected:
	friend class CScreenManager;

	CRoomScreen(const SCREENTYPE eSetType);
   virtual ~CRoomScreen() { }

	int		FindKey(const int nCommand) const;
	void		HideScroll() {this->bIsScrollVisible = false; PaintScroll();}
	void		InitKeysymToCommandMap(const CDbPackedVars &PlayerSettings);
	void		PaintBackground();
	void		PaintScroll();
	void		PaintSign();
	void		SetSignText(const WCHAR *pwczSetText);
	void		ShowScroll() {this->bIsScrollVisible = true; PaintScroll();}

	//These are accessed by CDemoScreen.
	CMapWidget *		pMapWidget;
	CLabelWidget *		pScrollLabel;

	SDL_Surface *		pGraphicsSurface;
	WCHAR *				pwczSignText;

	bool					bIsScrollVisible;

	int					KeysymToCommandMap[SDLK_LAST];
};

#endif //...#ifndef ROOMSCREEN_H

// $Log: RoomScreen.h,v $
// Revision 1.8  2003/07/24 19:46:24  mrimer
// Added empty virtual desctuctors.
//
// Revision 1.7  2003/07/17 01:54:38  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.6  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.5  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.4  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.3  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.2  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.1  2002/11/15 02:28:15  mrimer
// Initial check-in.  (Code refactored from CGameScreen.)
//
