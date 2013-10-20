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

#include "OptionButtonWidget.h"
#include "BitmapManager.h"
#include "FontManager.h"
#include "EventHandlerWidget.h"
#include "Sound.h"

//Source coords and dimensions within parts surface.
const int X_OB_C = 136;
const int Y_OB_C = 15;
const int X_OB_UC = 136;
const int Y_OB_UC = 26;
const UINT CX_OB = 11;
const UINT CY_OB = 11;
const UINT CX_FOCUS = CX_OB + 4;	//2 pixel buffer
const UINT CY_FOCUS = CY_OB + 4;

//
//Public methods.
//

//******************************************************************************
COptionButtonWidget::COptionButtonWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,					//(in)	Required params for CWidget 
	const int nSetX, const int nSetY,	//		constructor.
	const UINT wSetW, const UINT wSetH,	//
	const WCHAR *pwczSetCaption,			//(in)	Caption for button.  "" = none.
	const bool bSetChecked,					//(in)	Button begins checked?
	const bool bWhiteText)					//(in)	Make text white (default=false).
	: CFocusWidget(WT_OptionButton, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, bFocusRegionsSaved(false)
	, bIsChecked(bSetChecked)
	, bWhiteText(bWhiteText)
	, pwczCaption(NULL)
{
	SetCaption(pwczSetCaption);
}

//******************************************************************************
COptionButtonWidget::~COptionButtonWidget()
//Destructor.
//
{
   delete[] pwczCaption;
}

//******************************************************************************
void COptionButtonWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the widget.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
//
//Returns:
//True.
{
	const SDLKey key = KeyboardEvent.keysym.sym;

	switch (key) {
		case SDLK_SPACE:
			ToggleCheck(true);
		break;

      default: break;
	}
}

//******************************************************************************
void COptionButtonWidget::HandleMouseDown(
//Handles a mouse down event.
//
//Params:
	const SDL_MouseButtonEvent &/*Button*/)	//(in)	Event to handle.
{
	ToggleCheck(true);
}

//******************************************************************************
void COptionButtonWidget::ToggleCheck(
//Changes checked status.
//
//Params:
	const bool bShowEffects)	//(in) default = false
{
	this->bIsChecked = !this->bIsChecked;

	if (bShowEffects)
	{
		g_pTheSound->PlaySoundEffect(SOUNDLIB::SEID_BUTTON);
		Paint();
	}
}

//******************************************************************************
void COptionButtonWidget::SetCaption(
//Sets caption that appears on button.
//
//Params:
	const WCHAR *pwczSetCaption)	//(in)	New text.
{
	ASSERT(pwczSetCaption);

	delete [] this->pwczCaption;
	
	this->pwczCaption = new WCHAR[WCSlen(pwczSetCaption) + 1];
	WCScpy(this->pwczCaption, pwczSetCaption);
}



//******************************************************************************
void COptionButtonWidget::Paint(
//Paint widget area.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

	SDL_Rect FocusScreenRect = {this->x, this->y + (this->h - CY_OB) / 2 - 2,
			CX_FOCUS, CY_FOCUS};
	SDL_Rect FocusRect = {0, 0, CX_FOCUS, CY_FOCUS};

	ASSERT(this->w >= CX_OB && this->h >= CY_OB);

	//Save spot where focus box is drawn.
	SDL_Surface *pDestSurface = GetDestSurface();
	if (!this->bFocusRegionsSaved)
	{
		SDL_BlitSurface(pDestSurface, &FocusScreenRect,
				this->pFocusSurface, &FocusRect);
		this->bFocusRegionsSaved = true;
	}

	//Draw with or without focus.
	if (this->IsSelected())
		DrawFocused();
	else
		SDL_BlitSurface(this->pFocusSurface, &FocusRect, pDestSurface, &FocusScreenRect);

	//Draw checkbox.
	if (this->bIsChecked)
		DrawChecked();
	else
		DrawUnchecked();

	//Draw caption.
	if (this->pwczCaption)
	{
		const UINT CX_CAPTION_SPACE = 6;	//4 + 2 pixels for focus graphic
		UINT wIgnored, wTextHeight;
      g_pTheFM->GetTextRectHeight(FONTLIB::F_Button, 
				this->pwczCaption, this->w - CX_OB - CX_CAPTION_SPACE, wIgnored,
            wTextHeight);
		if (wTextHeight <= this->h)
		{
			g_pTheFM->DrawTextToRect(
					this->bWhiteText ? FONTLIB::F_ButtonWhite : FONTLIB::F_Button,
					this->pwczCaption,
					this->x + CX_OB + CX_CAPTION_SPACE,
					this->y + ((this->h - wTextHeight) / 2),
					this->w - CX_OB - CX_CAPTION_SPACE, wTextHeight,
					GetDestSurface());
		}
	}

	PaintChildren();

	if (bUpdateRect) UpdateRect();
}

//
//Protected methods.
//

//******************************************************************************
bool COptionButtonWidget::Load(void)
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	//Create surface to save screen surface bits where focus is drawn.
	this->pFocusSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			CX_FOCUS, CY_FOCUS, 24, 0, 0, 0, 0);
	if (!this->pFocusSurface) return false;

   return CWidget::Load();
}

//******************************************************************************
void COptionButtonWidget::Unload(void)
//Unload resources for widget.
{
	if (this->pFocusSurface)
	{
		SDL_FreeSurface(this->pFocusSurface);
		this->pFocusSurface = NULL;
	}

   CWidget::Unload();
}

//
//Private methods.
//

//******************************************************************************
void COptionButtonWidget::DrawChecked(void)
//Draw button in its checked state.
{
	static SDL_Rect Src = {X_OB_C, Y_OB_C, CX_OB, CY_OB};
	static SDL_Rect Dest = {0, 0, CX_OB, CY_OB};
	Dest.x = this->x + 2;
	Dest.y = this->y + (this->h - CY_OB) / 2;
	SDL_BlitSurface(this->pPartsSurface, &Src, GetDestSurface(), &Dest);
}

//******************************************************************************
void COptionButtonWidget::DrawUnchecked(void)
//Draw button in its unchecked state.
{
	static SDL_Rect Src = {X_OB_UC, Y_OB_UC, CX_OB, CY_OB};
	static SDL_Rect Dest = {0, 0, CX_OB, CY_OB};
	Dest.x = this->x + 2;
	Dest.y = this->y + (this->h - CY_OB) / 2;
	SDL_BlitSurface(this->pPartsSurface, &Src, GetDestSurface(), &Dest);
}

//******************************************************************************
void COptionButtonWidget::DrawFocused(void)
//Draw focus.
{
	const SURFACECOLOR FocusColor = GetSurfaceColor(GetDestSurface(), RGB_FOCUS);
	const UINT DRAWX = this->x;
	const UINT DRAWY = this->y + (this->h - CY_OB) / 2 - 2;
	const SDL_Rect rect = {DRAWX, DRAWY, CX_FOCUS, CY_FOCUS};

	//draw box around checkbox
	DrawRect(rect,FocusColor);
}

// $Log: OptionButtonWidget.cpp,v $
// Revision 1.5  2003/09/16 19:03:11  schik
// Fixed memory leak
//
// Revision 1.4  2003/07/19 02:13:00  mrimer
// Modified API for a method in CFontManager.
//
// Revision 1.3  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.2  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.15  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.14  2002/11/15 02:35:23  mrimer
// Revised display options and constructor.
//
// Revision 1.13  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.12  2002/08/23 23:40:35  erikh2000
// Sound effect now plays when button is toggled.
//
// Revision 1.11  2002/07/19 20:38:37  mrimer
// Revised HandleKeyDown() to return no type.
//
// Revision 1.10  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.9  2002/06/23 10:55:43  erikh2000
// Revised calls to CFontManager::GetTextRectHeight() to include new param.
//
// Revision 1.8  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.7  2002/06/14 00:57:18  erikh2000
// Changed focus rect color to subtler dark brown.
//
// Revision 1.6  2002/06/11 22:54:25  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.5  2002/06/06 00:01:14  mrimer
// Refined focus drawing code.
//
// Revision 1.4  2002/06/05 03:17:43  mrimer
// Added widget focusability and keyboard control.
// Added focus graphic.
//
// Revision 1.3  2002/06/03 22:58:06  mrimer
// Added/refined focusability.
//
// Revision 1.2  2002/04/24 08:14:05  erikh2000
// Implemented the class.
//
// Revision 1.1  2002/04/23 03:13:39  erikh2000
// Initial check-in.
//
