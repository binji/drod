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

#include "RoomScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include "MapWidget.h"
#include "RoomWidget.h"
#include <FrontEndLib/LabelWidget.h>

#include <BackEndLib/Wchar.h>

//*****************************************************************************
CRoomScreen::CRoomScreen(
//Base constructor.
//
//Params:
	const SCREENTYPE eSetType)
	: CDrodScreen(eSetType)
	, pMapWidget(NULL)
	, pScrollLabel(NULL)
	, pGraphicsSurface(NULL)
	, pwczSignText(NULL)
	, bIsScrollVisible(false)
{
}

//*****************************************************************************
void CRoomScreen::SetMusicStyle(
//Changes the music to match style.  If music is already matching style, nothing
//will happen.
//
//Params:
	const UINT wStyle)
{
	switch (wStyle)
	{
		case 1: g_pTheSound->PlaySong(SONGID_GAME1); break;
		case 2: g_pTheSound->PlaySong(SONGID_GAME2); break;
		case 3: g_pTheSound->PlaySong(SONGID_GAME3); break;
		case 4: g_pTheSound->PlaySong(SONGID_GAME4); break;
		case 5: g_pTheSound->PlaySong(SONGID_GAME5); break;
		case 6: g_pTheSound->PlaySong(SONGID_GAME6); break;
		case 7: g_pTheSound->PlaySong(SONGID_GAME7); break;
		case 8: g_pTheSound->PlaySong(SONGID_GAME8); break;
		case 9: g_pTheSound->PlaySong(SONGID_GAME9); break;

		default:
			ASSERTP(false, "Bad style#.");
	}
}

//*****************************************************************************
bool CRoomScreen::Load(
//Load resources for screen.
//Should only be called by Load method of derived class.
//
//Returns:
//True if successful, false if not.
//
//Params:
	CCurrentGame *pCurrentGame)	//(in)
{
	static const int X_MAP = 13;
	static const int Y_MAP = 364;
	static const UINT CX_MAP = 75;
	static const UINT CY_MAP = 81;
	static const int X_SCROLL_LABEL = 8;
	static const int Y_SCROLL_LABEL = 136;
	static const UINT CX_SCROLL_LABEL = 83;
	static const UINT CY_SCROLL_LABEL = 201;

	ASSERT(!this->bIsLoaded);

	//Load graphics bitmap.
	this->pGraphicsSurface = g_pTheBM->GetBitmapSurface("GameScreen");
	if (!this->pGraphicsSurface) return false;

	//Add widgets.
	this->pMapWidget = new CMapWidget(TAG_MAP, X_MAP, Y_MAP, CX_MAP, CY_MAP,
			pCurrentGame);
	this->pMapWidget->Disable();
	AddWidget(this->pMapWidget);

	this->pScrollLabel = new CLabelWidget(0L, X_SCROLL_LABEL, Y_SCROLL_LABEL,
			CX_SCROLL_LABEL, CY_SCROLL_LABEL, F_Scroll, wszEmpty);
	AddWidget(this->pScrollLabel);
	this->pScrollLabel->Hide();

	//this->bIsLoaded is set in derived class
	return true;
}

//*****************************************************************************
void CRoomScreen::Unload()
//Unload resources for screen.
//Should only be called by Unload method of derived class.
{
	ASSERT(this->bIsLoaded);

	//Free graphics surface.
	if (this->pGraphicsSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("GameScreen");
		this->pGraphicsSurface = false;
	}

	//this->bIsLoaded is reset in derived class
}

//*****************************************************************************
void CRoomScreen::SetSignText(
//Set text that appears on sign.
//
//Params:
	const WCHAR *pwczSetText)	//(in)	New text.  NULL will make the sign
								//		disappear on next paint.
{
	if (this->pwczSignText)
	{
		delete [] this->pwczSignText;
		this->pwczSignText = NULL;
	}

	if (pwczSetText)
	{
		const UINT dwTextLen = WCSlen(pwczSetText);
		if (dwTextLen)
		{
			this->pwczSignText = new WCHAR[dwTextLen + 1];
			if (this->pwczSignText) WCScpy(this->pwczSignText, pwczSetText);
		}
	}
}

//*****************************************************************************
void CRoomScreen::PaintBackground()
//Paint background.
{
	static SDL_Rect src = {0, 0, this->w, this->h};
	static SDL_Rect dest = {0, 0, this->w, this->h};
	SDL_BlitSurface(this->pGraphicsSurface, &src, GetDestSurface(), &dest);
}

//*****************************************************************************
void CRoomScreen::PaintScroll()
//Paint the scroll.
{
	static const int X_SCROLL = 4;
	static const int Y_SCROLL = 117;
	static const UINT CX_SCROLL = 97;
	static const UINT CY_SCROLL = 238;
	static const int X_SRC_SCROLL = 641;
	static const int Y_SRC_SCROLL = 1;
	static SDL_Rect ScreenRect = {X_SCROLL, Y_SCROLL, CX_SCROLL, CY_SCROLL};
	static SDL_Rect ScrollRect = {X_SRC_SCROLL, Y_SRC_SCROLL, CX_SCROLL, CY_SCROLL};

	SDL_Surface *pDestSurface = GetDestSurface();

	if (this->bIsScrollVisible)
	{
		SDL_BlitSurface(this->pGraphicsSurface, &ScrollRect,
				pDestSurface, &ScreenRect);
		this->pScrollLabel->Show();
		this->pScrollLabel->Paint();
	}
	else
	{
		SDL_BlitSurface(this->pGraphicsSurface, &ScreenRect,
				pDestSurface, &ScreenRect);
		this->pScrollLabel->Hide();
	}

	UpdateRect(ScreenRect);
}

//*****************************************************************************
void CRoomScreen::PaintSign()
//Paint the sign.
{
	SDL_Surface *pDestSurface = GetDestSurface();

	static const UINT CX_LEFT_SIGN = 41;
	static const UINT CX_MIDDLE_SIGN = 30;
	static const UINT CX_RIGHT_SIGN = 43;
	static const UINT CX_SIGN = 534;
	static const UINT CY_SIGN = 23;
	static const int X_LEFT_SIGN_SRC = 182;
	static const int X_MIDDLE_SIGN_SRC = 224;
	static const int X_RIGHT_SIGN_SRC = 255;
	static const int Y_SIGN_SRC = 482;
	static const int X_SIGN = 102;
	static const int Y_SIGN = 1;
	static SDL_Rect LeftSignSource = {X_LEFT_SIGN_SRC, Y_SIGN_SRC, CX_LEFT_SIGN, CY_SIGN};
	static SDL_Rect MiddleSignSource = {X_MIDDLE_SIGN_SRC, Y_SIGN_SRC, CX_MIDDLE_SIGN, CY_SIGN};
	static SDL_Rect RightSignSource = {X_RIGHT_SIGN_SRC, Y_SIGN_SRC, CX_RIGHT_SIGN, CY_SIGN};
	static SDL_Rect EntireSign = {X_SIGN, Y_SIGN, CX_SIGN, CY_SIGN};

	//Blit background over the entire possible area the sign could cover.
	SDL_BlitSurface(this->pGraphicsSurface, &EntireSign, pDestSurface, &EntireSign);

	//Is there text to display?
	if (this->pwczSignText) //Yes.
	{
		UINT wMiddleCount, wTextWidth, wTextHeight;

		//Figure out how wide it will be.
		g_pTheFM->GetTextWidthHeight(F_Sign, this->pwczSignText, wTextWidth, wTextHeight);
		ASSERT(wTextWidth > 0);

		//Figure how many middle sign parts will be needed to display the text.
		wMiddleCount = (wTextWidth / CX_MIDDLE_SIGN);
		if (wTextWidth % CX_MIDDLE_SIGN != 0) ++wMiddleCount; //Round up.

		//If the text is too large to fit, then exit without displaying it.
		UINT wSignWidth = CX_LEFT_SIGN + (wMiddleCount * CX_MIDDLE_SIGN) +
				CX_RIGHT_SIGN;
		if (wSignWidth > CX_SIGN)
		{
         //Sign width too large -- truncate sign text to fit.
         wSignWidth = CX_SIGN;
         wMiddleCount = (CX_SIGN - CX_LEFT_SIGN - CX_RIGHT_SIGN) / CX_MIDDLE_SIGN;
		}

		//Blit left part of sign.
		SDL_Rect Dest = {X_SIGN + ((CX_SIGN - wSignWidth) / 2), Y_SIGN,
				CX_LEFT_SIGN, CY_SIGN};
		Uint32 TransparentColor = SDL_MapRGB(this->pGraphicsSurface->format, 226, 0, 0);
		SDL_SetColorKey(this->pGraphicsSurface, SDL_SRCCOLORKEY, TransparentColor);
		SDL_BlitSurface(this->pGraphicsSurface, &LeftSignSource, pDestSurface, &Dest);
		SDL_SetColorKey(this->pGraphicsSurface, 0, TransparentColor);

		//Blit middle parts of sign.
		Dest.x += CX_LEFT_SIGN;
		Dest.w = CX_MIDDLE_SIGN;
		for (UINT wI = 0; wI < wMiddleCount; ++wI)
		{
			SDL_BlitSurface(this->pGraphicsSurface, &MiddleSignSource, pDestSurface, &Dest);
			Dest.x += CX_MIDDLE_SIGN;
		}

		//Blit right part of sign.
		Dest.w = CX_RIGHT_SIGN;
		SDL_SetColorKey(this->pGraphicsSurface, SDL_SRCCOLORKEY, TransparentColor);
		SDL_BlitSurface(this->pGraphicsSurface, &RightSignSource, pDestSurface, &Dest);

		//Draw text on sign.
		int xText = X_SIGN + ((CX_SIGN - wTextWidth) / 2);
      if (xText < (int)(X_SIGN + CX_LEFT_SIGN/2)) xText = X_SIGN + CX_LEFT_SIGN/2;
		int yText = Y_SIGN + ((CY_SIGN - wTextHeight) / 2);
		if (yText < (int)Y_SIGN) yText = Y_SIGN;
		g_pTheFM->DrawTextXY(F_Sign, this->pwczSignText, pDestSurface, xText, yText,
            CX_SIGN - CX_LEFT_SIGN);

		SDL_SetColorKey(this->pGraphicsSurface, 0, TransparentColor);
	}

	UpdateRect(EntireSign);
}

//*****************************************************************************
void CRoomScreen::InitKeysymToCommandMap(
//Set the keysym-to-command map with values from player settings that will determine
//which commands correspond to which keys.
//
//Params:
	const CDbPackedVars &PlayerSettings)	//(in)	Player settings to load from.
{
	//Clear the map.
	memset(this->KeysymToCommandMap, CMD_UNSPECIFIED,
			sizeof(this->KeysymToCommandMap));

	//Get values from current player settings.
	int nKey = PlayerSettings.GetVar("MoveNorthwest", SDLK_KP7);
	this->KeysymToCommandMap[nKey] = CMD_NW;
	nKey = PlayerSettings.GetVar("MoveNorth", SDLK_KP8);
	this->KeysymToCommandMap[nKey] = CMD_N;
	nKey = PlayerSettings.GetVar("MoveNortheast", SDLK_KP9);
	this->KeysymToCommandMap[nKey] = CMD_NE;
	nKey = PlayerSettings.GetVar("MoveWest", SDLK_KP4);
	this->KeysymToCommandMap[nKey] = CMD_W;
	nKey = PlayerSettings.GetVar("Wait", SDLK_KP5);
	this->KeysymToCommandMap[nKey] = CMD_WAIT;
	nKey = PlayerSettings.GetVar("MoveEast", SDLK_KP6);
	this->KeysymToCommandMap[nKey] = CMD_E;
	nKey = PlayerSettings.GetVar("MoveSouthwest", SDLK_KP1);
	this->KeysymToCommandMap[nKey] = CMD_SW;
	nKey = PlayerSettings.GetVar("MoveSouth", SDLK_KP2);
	this->KeysymToCommandMap[nKey] = CMD_S;
	nKey = PlayerSettings.GetVar("MoveSoutheast", SDLK_KP3);
	this->KeysymToCommandMap[nKey] = CMD_SE;
	nKey = PlayerSettings.GetVar("SwingCounterclockwise", SDLK_q);
	this->KeysymToCommandMap[nKey] = CMD_CC;
	nKey = PlayerSettings.GetVar("SwingClockwise", SDLK_w);
	this->KeysymToCommandMap[nKey] = CMD_C;
	nKey = PlayerSettings.GetVar("Restart", SDLK_r);
	this->KeysymToCommandMap[nKey] = CMD_RESTART;
}

//*****************************************************************************
int CRoomScreen::FindKey(
//Returns:
//Key index mapped to command.
	const int nCommand) const	//(in) Command to find mapping for.
{
	for (int nIndex=0; nIndex<SDLK_LAST; nIndex++)
		if (this->KeysymToCommandMap[nIndex] == nCommand)
			return nIndex;
	ASSERTP(false, "Failed to find key mapping.");
	return -1;
}

// $Log: RoomScreen.cpp,v $
// Revision 1.13  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.12  2003/11/16 03:17:13  mrimer
// Fixed bug: room coords scroll disappears when text is too long.
//
// Revision 1.11  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.10  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.8  2003/06/09 19:30:26  mrimer
// Fixed some level editor bugs.
//
// Revision 1.7  2003/06/06 18:21:31  mrimer
// Revised some calls.
//
// Revision 1.6  2003/05/23 21:43:04  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.5  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.4  2003/05/04 00:04:02  mrimer
// Fixed some var types.
//
// Revision 1.3  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.2  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.1  2002/11/15 02:28:15  mrimer
// Initial check-in.  (Code refactored from CGameScreen.)
//
