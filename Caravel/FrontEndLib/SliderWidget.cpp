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

#include "SliderWidget.h"
#include "BitmapManager.h"
#include <BackEndLib/Assert.h>

const UINT CX_SLIDER = 7;
const UINT CY_SLIDER = 14;

//
//Public methods.
//

//******************************************************************************
CSliderWidget::CSliderWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	BYTE bytSetValue)						//(in)	0 = left, 255 = right.
	: CFocusWidget(WT_Slider, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, bytValue(bytSetValue), bytPrevValue(bytSetValue)
	, bWasSliderDrawn(false)
	, bFocusRegionsSaved(false)
	, pEraseSurface(NULL)
{
	this->pFocusSurface[0] = this->pFocusSurface[1] = NULL;
}

//******************************************************************************
void CSliderWidget::HandleDrag(
//Handle mouse dragging of the slider.
//
//Params:
	const SDL_MouseMotionEvent &Motion)	//(in)	Event to handle.
{
	SetToX(Motion.x);
	Paint();
}

//******************************************************************************
void CSliderWidget::HandleMouseDown(
//Handle mousedown on the slider.
//
//Params:
	const SDL_MouseButtonEvent &Button) //(in)	Event to handle.
{
	SetToX(Button.x);
	Paint();
}

//******************************************************************************
void CSliderWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the widget.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
//
//Returns:
//True.
{
	const SDLKey key = KeyboardEvent.keysym.sym;
	const BYTE incAmount = (BYTE)8;

	switch (key) {
		case SDLK_LEFT: case SDLK_KP4:
			//slide left
			if (this->bytValue > 0)
			{
				if (KeyboardEvent.keysym.mod & KMOD_CTRL)
				{	//all the way to left
					this->bytValue = 0;
				} else {
					if (this->bytValue < incAmount)
						this->bytValue = 0;
					else
						this->bytValue -= incAmount;
				}
				Paint();
			}
			return;
		case SDLK_RIGHT: case SDLK_KP6:
			//slide right
			if (this->bytValue < 255)
			{
				if (KeyboardEvent.keysym.mod & KMOD_CTRL)
				{	//all the way to right
					this->bytValue = 255;
				} else {
					if (this->bytValue > 255 - incAmount)
						this->bytValue = 255;
					else
						this->bytValue += incAmount;
				}
				Paint();
			}
			return;

         default: break;
	}
}

//******************************************************************************
void CSliderWidget::Paint(
//Paint widget area.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	ASSERT(static_cast<UINT>(this->w) >= CX_SLIDER);
	ASSERT(static_cast<UINT>(this->y) >= CX_SLIDER);

	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

	static const int X_SLIDER = 147;
	static const int Y_SLIDER = 23;
	static SDL_Rect SliderRect = {X_SLIDER, Y_SLIDER, CX_SLIDER, CY_SLIDER};
	static SDL_Rect EraseRect = {0, 0, CX_SLIDER, CY_SLIDER};
	SDL_Rect ScreenRect = {0, 0, CX_SLIDER, CY_SLIDER};
	SDL_Rect FocusScreenRect = {this->x + 1, 0, this->w-2, 1};
	SDL_Rect FocusRect = {0, 0, this->w-2, 1};

	//Calc surface colors if needed.
	SDL_Surface *pDestSurface = GetDestSurface();
	const SURFACECOLOR Light = GetSurfaceColor(pDestSurface, 225, 214, 192), 
			Dark = GetSurfaceColor(pDestSurface, 145, 132, 109);
	
	//Save spots where focus lines are drawn.
	if (!this->bFocusRegionsSaved)
	{
		FocusScreenRect.y = this->y;
		SDL_BlitSurface(pDestSurface, &FocusScreenRect,
				this->pFocusSurface[0], &FocusRect);
		FocusScreenRect.y = this->y + this->h - 1;
		SDL_BlitSurface(pDestSurface, &FocusScreenRect,
				this->pFocusSurface[1], &FocusRect);
		this->bFocusRegionsSaved = true;
	}

	//Erase slider at previous location.
	ScreenRect.y = this->y + (this->h - CY_SLIDER) / 2;
	if (this->bWasSliderDrawn)
	{
		ScreenRect.x = static_cast<Sint16>( this->x + 1 +
				((double) this->bytPrevValue / 255.0) * (this->w - CX_SLIDER - 2) );
		SDL_BlitSurface(this->pEraseSurface, &EraseRect,
            pDestSurface, &ScreenRect);
	}

	//Draw the line that slider travels over.
	DrawCol(this->x, this->y, this->h, Dark);
	DrawCol(this->x + 1, this->y, this->h, Light);
	DrawCol(this->x + this->w - 2, this->y, this->h, Dark);
	DrawCol(this->x + this->w - 1, this->y, this->h, Light);
	int yLine = this->y + this->h / 2;
	DrawRow(this->x + 1, yLine - 1, this->w - 2, Dark);
	DrawRow(this->x + 1, yLine, this->w - 2, Light);

	//Copy underneath slider area to surface that can be used to erase it later.
	ScreenRect.x = static_cast<Sint16>( this->x + 1 +
			((double) this->bytValue / 255.0) * (this->w - CX_SLIDER - 2) );
	SDL_BlitSurface(pDestSurface, &ScreenRect, this->pEraseSurface, &EraseRect);

	//Draw the slider in its current location.
	SDL_BlitSurface(this->pPartsSurface, &SliderRect, pDestSurface, &ScreenRect);
	this->bWasSliderDrawn = true;
	this->bytPrevValue = this->bytValue;	//save position thumb was drawn at

	PaintChildren();

	if (IsSelected())
		DrawFocused();
	else
	{
		//erase focus lines
		FocusScreenRect.y = this->y;
		SDL_BlitSurface(this->pFocusSurface[0], &FocusRect, pDestSurface, &FocusScreenRect);
		FocusScreenRect.y = this->y + this->h - 1;
		SDL_BlitSurface(this->pFocusSurface[1], &FocusRect, pDestSurface, &FocusScreenRect);
	}

	if (bUpdateRect) UpdateRect();
}

//******************************************************************************
void CSliderWidget::SetValue(
//Set value of slider, which affects its position.
//
//Params:
	const BYTE bytSetValue)	//(in)	New value.
{
	this->bytValue = bytSetValue;
}

//******************************************************************************
void CSliderWidget::SetToX(
//Set value of slider based on x coordinate on screen.
//
//Params:
	const int nX)	//(in)	x coordinate.
{
	if (nX < static_cast<int>(this->x))
      SetValue(0);
	else if (nX >= static_cast<int>(this->x + this->w - CX_SLIDER))
		SetValue(255);
	else
	{
		const BYTE bytNewValue = static_cast<BYTE>( (double)(nX - this->x) /
				(double)(this->w - CX_SLIDER) * 255.0 );
		SetValue(bytNewValue);
	}
}

//
//Protected methods.
//

//******************************************************************************
bool CSliderWidget::Load()
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	//Create surface to save screen surface bits of an area before slider
	//is blitted to it.
	this->pEraseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			CX_SLIDER, CY_SLIDER, 24, 0, 0, 0, 0);
	if (!this->pEraseSurface) return false;

	//Create surface to save screen surface bits where focus is drawn.
	this->pFocusSurface[0] = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			this->w-2, 1, 24, 0, 0, 0, 0);
	this->pFocusSurface[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			this->w-2, 1, 24, 0, 0, 0, 0);
	if (!this->pFocusSurface[0] || !this->pFocusSurface[1]) return false;

   return CWidget::Load();
}

//******************************************************************************
void CSliderWidget::Unload()
//Unload resources for widget.
{
	if (this->pEraseSurface)
	{
		SDL_FreeSurface(this->pEraseSurface);
		this->pEraseSurface = NULL;
	}

	if (this->pFocusSurface[0])
	{
		SDL_FreeSurface(this->pFocusSurface[0]);
		this->pFocusSurface[0] = NULL;
		SDL_FreeSurface(this->pFocusSurface[1]);
		this->pFocusSurface[1] = NULL;
	}

   CWidget::Unload();
}

//
//Private methods.
//

//******************************************************************************
void CSliderWidget::DrawFocused()
//Draw focus.
{
	const SURFACECOLOR FocusColor = GetSurfaceColor(GetDestSurface(), RGB_FOCUS);
	const SDL_Rect rect = {this->x, this->y, this->w, this->h};

	DrawRect(rect,FocusColor);
}

// $Log: SliderWidget.cpp,v $
// Revision 1.4  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.3  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.13  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.12  2002/11/22 02:28:54  mrimer
// Removed unneeded code and includes.  Revised constructor initialization list.
//
// Revision 1.11  2002/09/03 21:30:34  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.10  2002/07/19 20:38:37  mrimer
// Revised HandleKeyDown() to return no type.
//
// Revision 1.9  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.8  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.7  2002/06/14 01:04:56  erikh2000
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
// Changed static surfacecolor declarations to const to prevent errors.
// Changed focus rect color to subtler dark brown.
//
// Revision 1.6  2002/06/11 22:49:59  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.5  2002/06/07 16:23:24  mrimer
// Rearranged includes to remove warning.
//
// Revision 1.4  2002/06/06 00:01:14  mrimer
// Refined focus drawing code.
//
// Revision 1.3  2002/06/05 03:17:43  mrimer
// Added widget focusability and keyboard control.
// Added focus graphic.
//
// Revision 1.2  2002/05/21 13:58:01  mrimer
// Fixed off by 1 error at right end.
//
// Revision 1.1  2002/04/24 08:11:14  erikh2000
// Initial check-in.
//
