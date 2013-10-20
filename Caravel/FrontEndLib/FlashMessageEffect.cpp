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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "FlashMessageEffect.h"
#include "FontManager.h"

#include <BackEndLib/Assert.h>

//********************************************************************************
CFlashMessageEffect::CFlashMessageEffect(
//Constructor.
//
//
//Params:
	CWidget *pSetWidget,	//(in)	Should be a room widget.
	const WCHAR *text,		//(in)  Text to display
	const Uint32 wDuration)	//(in)  How long to display (in milliseconds)
	: CEffect(pSetWidget)
{
	ASSERT(pSetWidget->GetType() == WT_Room);

	pSetWidget->GetRect(screenRect);

	this->wstrText = text;
	
	this->wDuration = wDuration;

	this->wAnimFrame = 0;

	this->dwLastFrame = this->dwStartTime = SDL_GetTicks();
}

//********************************************************************************
bool CFlashMessageEffect::Draw(SDL_Surface* pDestSurface)
//Draws a pulsing message in the middle of the parent widget.
{
	const UINT eDrawFont = (this->wAnimFrame == 0 ?
			FONTLIB::F_FlashMessage_1 : FONTLIB::F_FlashMessage_2);
	//End after duration has elapsed.
	const Uint32 dwNow = SDL_GetTicks();
	if (dwNow - dwStartTime > wDuration)
		return false;

	UINT cxDraw, cyDraw;
	g_pTheFM->GetTextWidthHeight(eDrawFont, this->wstrText.c_str(),
			cxDraw, cyDraw);

	//Center text in widget.
	const UINT xDraw = (this->screenRect.w - cxDraw) / 2;
	const UINT yDraw = (this->screenRect.h - cyDraw) / 2;

   if (!pDestSurface)
      pDestSurface = GetDestSurface();

	g_pTheFM->DrawTextXY(eDrawFont, this->wstrText.c_str(), pDestSurface,
		this->screenRect.x + xDraw, this->screenRect.y + yDraw);

	//Specify area of effect.
	this->rAreaOfEffect.x = this->screenRect.x + xDraw;
	this->rAreaOfEffect.y = this->screenRect.y + yDraw;
	this->rAreaOfEffect.w = cxDraw;
	this->rAreaOfEffect.h = cyDraw;

	//Keep displaying if duration has not elapsed.
	if (dwNow - this->dwLastFrame > 250)
	{
		this->dwLastFrame = dwNow;
		this->wAnimFrame = (this->wAnimFrame == 0 ? 1 : 0);
	}

	return true;
}

// $Log: FlashMessageEffect.cpp,v $
// Revision 1.4  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
//
// Revision 1.3  2003/06/09 19:22:15  mrimer
// Added optional max width/height params to FM::DrawTextXY().
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.9  2003/04/08 13:08:27  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.8  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.7  2002/10/11 15:30:43  mrimer
// Fixed display bugs.
//
// Revision 1.6  2002/10/10 21:15:20  mrimer
// Modified to support optimized room drawing.
//
// Revision 1.5  2002/06/21 05:05:38  mrimer
// Changed DrawText call to DrawTextXY.
//
// Revision 1.4  2002/06/16 22:13:46  erikh2000
// Changed a param to const since it was read-only.
//
// Revision 1.3  2002/06/11 22:59:00  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.2  2002/05/17 00:47:10  mrimer
// Make copy of specified text string.
//
// Revision 1.1  2002/05/16 23:08:27  mrimer
// Initial checkin.
//
