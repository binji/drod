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

#include "WinPicScreen.h"
#include "DrodBitmapManager.h"
#include <FrontEndLib/Fade.h>

#include <BackEndLib/Assert.h>

//
//Protected methods.
//

//******************************************************************************
void CWinPicScreen::FadeToNewPic(void)
//Fade from the current screen to the currently loaded picture.
{
	CFade fade(GetDestSurface(), this->pWinSurface);
	fade.FadeBetween();
}

//******************************************************************************
bool CWinPicScreen::Load(void)
//Loads resources for the screen.
{
	ASSERT(!this->bIsLoaded);

	//Handle case where screen is being reloaded after an unload, and a picture
	//surface needs to be restored.
	ASSERT(!this->pWinSurface);
	if (this->strPicName.size())
	{
		this->pWinSurface = g_pTheBM->GetBitmapSurface(this->strPicName.c_str());
		if (!this->pWinSurface)
		{
			this->strPicName = "";
			return false;
		}
	}

	this->pScrollingText = new CScrollingTextWidget(0L, 0, 0, 1, 1);
	if (this->pWinSurface)
		this->pScrollingText->SetBackground(this->pWinSurface);
	this->pScrollingText->SetScrollRate(15);
	AddWidget(this->pScrollingText);
	
	//Load children last.
	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//******************************************************************************
void CWinPicScreen::Unload(void)
//Unloads resources for the screen.
{
	ASSERT(this->bIsLoaded);

	//Unload children first.
	UnloadChildren();

	//Release surface for picture.
	if (this->pWinSurface)
	{
		g_pTheBM->ReleaseBitmapSurface(this->strPicName.c_str());
		this->pWinSurface = NULL;
	}

	this->bIsLoaded = false;
}

//******************************************************************************
bool CWinPicScreen::LoadPic(
//Load a picture so that it is displayed on the screen.
//
//Params:
	const char *pszPicName)	//Name of picture to load.
{
	ASSERT(strlen(pszPicName));

	//Release existing surface for picture.
	if (this->pWinSurface)
	{
		ASSERT(this->strPicName.size());
		g_pTheBM->ReleaseBitmapSurface(&*this->strPicName.begin());
	}

	//Load new pic.
	this->strPicName = pszPicName;
	this->pWinSurface = g_pTheBM->GetBitmapSurface(pszPicName);
	if (!this->pWinSurface)
	{
		this->strPicName = "";
		return false;
	}
	else if (this->pWinSurface->w != CX_SCREEN || this->pWinSurface->h != CY_SCREEN)
	{
		//Pic dimensions don't match screen.
		g_pTheBM->ReleaseBitmapSurface(this->strPicName.c_str());
		this->strPicName = "";
		this->pWinSurface = NULL;
		return false;
	}

	//Scrolling text needs pointer to new background surface.
	this->pScrollingText->SetBackground(this->pWinSurface);

	//Success.
	return true;
}

//******************************************************************************
void CWinPicScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool bUpdateRect)	//(in)	If true (default) and destination
							//		surface is the screen, the screen
							//		will be immediately updated in
							//		the widget's rect.
{
	PaintBackground();
	PaintChildren();

	if (bUpdateRect) UpdateRect();
}

//******************************************************************************
void CWinPicScreen::SetScrollingTextRect(
//Moves and resizes the scrolling text widget to match a new rect.
//
//Params:
	const SDL_Rect &NewRect)	//(in)	Rect for scrolling text widget.
{
	SDL_Surface *pDestSurface = GetDestSurface();

	//Erase scrolling text rect from screen at current position.
	SDL_Rect OldRect;
	this->pScrollingText->GetRect(OldRect);
	SDL_BlitSurface(this->pWinSurface, &OldRect, pDestSurface, &OldRect);
	
	//Move and resize scrolling text widget to new rect.
	this->pScrollingText->Move(NewRect.x, NewRect.y);
	this->pScrollingText->Resize(NewRect.w, NewRect.h);
}

// $Log: WinPicScreen.cpp,v $
// Revision 1.7  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.6  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.5  2003/04/08 13:08:29  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.4  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.3  2002/10/10 01:01:53  erikh2000
// Removed dead code.
//
// Revision 1.2  2002/10/02 21:43:00  mrimer
// Updated screen fade calls.
//
// Revision 1.1  2002/10/01 22:58:39  erikh2000
// Initial check-in.
//
