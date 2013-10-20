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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//ScreenManager.h
//Declarations for CScreenManager.
//CScreenManager loads and unloads screens, with knowledge of specific screen
//types.  It directs execution to an input loop in an appropriate CScreen class.

#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "Screen.h"
#include <BackEndLib/MessageIDs.h>
#include <string>

using std::string;

//Screen (surface) transition effects.
//Add other transition effects here as needed.
enum TRANSTYPE {
	Fade,		//Smooth color transition from one screen to another.
	Cut,		//New screen shown without transition.
	Pan		//Slide from one screen to another.
};

//Values needed here, but can't place them in a final enumeration yet (i.e.,
//there will be more screens used in the app).
//Give these values to the corresponding names in the final enumeration.
namespace SCREENLIB
{
   enum SCREENTYPE {
      SCR_None = 0,		//Not an actual screen--indicates no screen or application exit.
	   SCR_Return = 1    //Not an actual screen--indicates the screen previously visited.
   };
};

//*****************************************************************************
class CScreen;
class CScreenManager
{
public:
	CScreenManager(SDL_Surface *pSetScreenSurface);
	virtual ~CScreenManager();

	virtual UINT Init() {return 0L;}

	SDL_Cursor *	GetCursor(const UINT cursorType) const
			{return this->pCursor[cursorType];}

	//Specifies the transition to use on the next screen change.  After that
	//screen change, the transition type will go back to the default type.
	void			SetDestTransition(const TRANSTYPE eTransType)
			{this->eTransition = eTransType;}

	UINT		ActivateScreen(const UINT eScreen);
	CScreen *		GetLoadedScreen(const UINT eScreen) const;
	CScreen *		GetScreen(const UINT eScreen);
	UINT		GetReturnScreenType() const;
	void			InsertReturnScreen(const UINT eScreen);
	bool			IsScreenLoaded(const UINT eScreen) const;
	void			ChangeReturnScreen(const UINT eScreen);
	void			ClearReturnScreens();
	void			RemoveReturnScreen();
    virtual void    GetScreenName(const UINT eScreen, string &strName) const = 0;

protected:
	virtual void FreeCursors() {}
	virtual CScreen *	GetNewScreen(const UINT eScreen)=0;
	CScreen *	LoadScreen(const UINT eScreen);
	void		UnloadAllScreens();

	SDL_Cursor*		LoadCursor(const char* cursorName);
	virtual bool		LoadCursors()=0;

	SDL_Cursor **		pCursor;
	list<CScreen *>		LoadedScreens;
	list<UINT>	ReturnScreenList;

	//Screen transition effects.
	TRANSTYPE eTransition;

	PREVENT_DEFAULT_COPY(CScreenManager);
};

//Define global pointer to the one and only CScreenManager object.
#ifndef INCLUDED_FROM_SCREENMANAGER_CPP
	extern CScreenManager *g_pTheSM;
#endif

#endif //...#ifndef SCREENMANAGER_H

// $Log: ScreenManager.h,v $
// Revision 1.4  2003/10/07 21:11:31  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.3  2003/07/12 01:12:00  mrimer
// Revised screen/cursor naming in FrontEndLib.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
