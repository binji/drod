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

#include "ButtonWidget.h"
#include "BitmapManager.h"
#include "FontManager.h"
#include "EventHandlerWidget.h"
#include "Sound.h"

#include <BackEndLib/Types.h>
#include <BackEndLib/Assert.h>
#include <BackEndLib/Ports.h>

//Source coords and dimensions within parts surface.
const UINT X_LEFT_BEVEL = 136;
const UINT Y_TOP_BEVEL = 0;
const UINT CX_CORNER = 1;
const UINT CY_CORNER = 1;
const UINT CX_CENTER = 13;
const UINT CY_CENTER = 13;
const UINT X_RIGHT_BEVEL = X_LEFT_BEVEL + CX_CORNER + CX_CENTER;
const UINT Y_BOTTOM_BEVEL = Y_TOP_BEVEL + CY_CORNER + CY_CENTER;
const UINT X_CENTER = X_LEFT_BEVEL + CX_CORNER;
const UINT Y_CENTER = Y_TOP_BEVEL + CY_CORNER;

//
//Public methods.
//

//******************************************************************************
CButtonWidget::CButtonWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	const WCHAR *pwczSetCaption)			//(in)	Caption for button.  "" = none.
	: CFocusWidget(WT_Button, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
   , bIsPressed(false)
{
	this->pwczCaption = NULL;
	SetCaption(pwczSetCaption);

	//Search for a (the first) hotkey.
	const UINT nLength = WCSlen(pwczSetCaption);
	for (UINT nIndex=0; nIndex<nLength; nIndex++)
		if (pwczSetCaption[nIndex] == L'&')
		{
			ASSERT(nIndex+1 < nLength);
			AddHotkey(static_cast<SDLKey>(WCv(towlower(pwczSetCaption[nIndex+1]))), dwSetTagNo);	//no uppercase mappings exist
			break;
		}
}

//******************************************************************************
CButtonWidget::~CButtonWidget()
//Destructor.
//
{
   delete[] this->pwczCaption;
}


//******************************************************************************
void CButtonWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the widget.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
{
	const SDLKey key = KeyboardEvent.keysym.sym;

	switch (key) {
		case SDLK_SPACE:
		case SDLK_RETURN:
		{
			Press();
			Paint();
			g_pTheSound->PlaySoundEffect(SOUNDLIB::SEID_BUTTON);
			SDL_Delay(200);
			Unpress();
			Paint();
			SDL_Delay(50);

			//Call OnClick() notifier.
			CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
			if (pEventHandler) pEventHandler->OnClick(this->dwTagNo);
		}
		return;

		default: break;
   }
}

//******************************************************************************
void CButtonWidget::HandleDrag(
//Handles mouse motion occurring after a mousedown event in this widget.
//
//Params:
	const SDL_MouseMotionEvent &Motion)	//(in) Event to handle.
{
	//The mouse button goes up or down depending on whether the cursor is
	//inside the widget.
	bool bWasPressed = this->bIsPressed;
	if (ContainsCoords(Motion.x, Motion.y))
		Press();
	else
		Unpress();

	//If there was a change, repaint the button.
	if (this->bIsPressed != bWasPressed)
	{
		if (this->bIsPressed) g_pTheSound->PlaySoundEffect(SOUNDLIB::SEID_BUTTON);
		Paint();
	}
}

//******************************************************************************
void CButtonWidget::HandleMouseDown(
//Handles mouse down event.
//
//Params:
	const SDL_MouseButtonEvent &/*Button*/)	//(in) Event to handle.
{
	//The mouse button goes down.
	bool bWasPressed = this->bIsPressed;
	Press();

	//If there was a change, repaint the button.
	if (this->bIsPressed != bWasPressed)
	{
		g_pTheSound->PlaySoundEffect(SOUNDLIB::SEID_BUTTON);
		Paint();
	}
}

//******************************************************************************
void CButtonWidget::HandleMouseUp(
//Handles mouse up event.
//
//Params:
	const SDL_MouseButtonEvent &/*Button*/)	//(in) Event to handle.
{
	//The mouse button goes down.
	bool bWasPressed = this->bIsPressed;
	Unpress();

	//If there was a change, repaint the button.
	if (this->bIsPressed != bWasPressed)
		Paint();
}

//******************************************************************************
void CButtonWidget::SetCaption(
//Sets caption that appears on button.
//
//Params:
	const WCHAR *pwczSetCaption)	//(in)	New text.
{
	ASSERT(pwczSetCaption);

	delete[] this->pwczCaption;
	
	this->pwczCaption = new WCHAR[WCSlen(pwczSetCaption) + 1];
	WCScpy(this->pwczCaption, pwczSetCaption);
}

//******************************************************************************
void CButtonWidget::Paint(
//Paint widget area.
//
//Params:
	bool bUpdateRect)			//(in)	If true (default) and destination
								//		surface is the screen, the screen
								//		will be immediately updated in
								//		the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

		ASSERT(this->w > CX_CORNER * 2);
	ASSERT(this->h > CY_CORNER * 2);

	if (this->bIsPressed)
	{
		DrawPressed();
	} else {
		DrawNormal();
	}

	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//
//Private methods.
//

//******************************************************************************
void CButtonWidget::DrawButtonText(
//Draw button text.
//
//Params:
	const UINT wOffset)	//(in) Number of pixels to drop text.
{
	ASSERT(this->pwczCaption);

	UINT wTextW, wTextH;
	int xDraw, yDraw;
	const UINT buttonFont = IsEnabled() ? FONTLIB::F_Button : FONTLIB::F_Button_Disabled;

	//Get a copy of caption text without any ampersands.
	WCHAR *pwczPlainCaption = new WCHAR[WCSlen(this->pwczCaption) + 1];
	if (!pwczPlainCaption) {ASSERTP(false, "Alloc failed."); return;}
	const WCHAR *pwczRead = this->pwczCaption;
	WCHAR *pwczWrite = pwczPlainCaption;
	while (*pwczRead != '\0')
	{
		if (*pwczRead != '&')
			*(pwczWrite++) = *pwczRead;
		++pwczRead;
	}
	pWCv(pwczWrite) = '\0';

	//Get dimensions of plain caption to be used for centering drawn text.
	g_pTheFM->GetTextWidthHeight(buttonFont, pwczPlainCaption, wTextW, wTextH);
	if (wTextH + wOffset > this->h) return; //No room for text.
	if (wTextW > this->w)
		xDraw = this->x;
	else
		xDraw = this->x + (this->w - wTextW) / 2;
	yDraw = this->y + (this->h - wTextH) / 2;

	//Draw the text.
	if (IsEnabled())
		g_pTheFM->DrawHotkeyTextToLine(buttonFont, pwczCaption,
			xDraw, yDraw + wOffset, this->w, GetDestSurface());
	else
		g_pTheFM->DrawTextToRect(buttonFont, pwczPlainCaption,
			xDraw, yDraw + wOffset, this->w, this->h, GetDestSurface());

	delete[] pwczPlainCaption;
}

//******************************************************************************
void CButtonWidget::DrawFocused(
//Draw button with focus.
//
//Params:
	const UINT wOffset)	//(in) Number of pixels to offset focus indicator.
{
	const SURFACECOLOR FocusColor = GetSurfaceColor(GetDestSurface(), RGB_FOCUS);
	static const UINT wIndent = 2;	//# pixels in from beveled edge that box shows
	const UINT DRAWX = this->x;
	const UINT DRAWY = this->y + wOffset;
	const SDL_Rect rect = {DRAWX + CX_CORNER + wIndent, DRAWY + wIndent + 1,
		this->w - (wIndent + CX_CORNER) * 2, this->h - (wIndent + CY_CORNER) * 2};

	//draw box inside button edge
	DrawRect(rect,FocusColor);
}

//******************************************************************************
void CButtonWidget::DrawNormal(void)
//Draw button in its normal state.
{
	SDL_Surface *pDestSurface = GetDestSurface();

	//Dest coords and dimensions.
	UINT xLeftBevel = this->x;
	UINT xRightBevel = this->x + this->w - CX_CORNER;
	UINT yTopBevel = this->y;
	UINT yBottomBevel = this->y + this->h - CY_CORNER;
	UINT xCenter = xLeftBevel + CX_CORNER;
	UINT yCenter = yTopBevel + CY_CORNER;

	//Draw top-left corner.
	SDL_Rect src = {X_LEFT_BEVEL, Y_TOP_BEVEL, CX_CORNER, CY_CORNER};
	SDL_Rect dest = {xLeftBevel, yTopBevel, CX_CORNER, CY_CORNER};
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw top-right corner.
	src.x = X_RIGHT_BEVEL;
	dest.x = xRightBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-right corner.
	src.y = Y_BOTTOM_BEVEL;
	dest.y = yBottomBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-left corner.
	src.x = X_LEFT_BEVEL;
	dest.x = xLeftBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom bevel.
	src.x = X_CENTER;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw top bevel.
	dest.y = yTopBevel;
	src.y = Y_TOP_BEVEL;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw left bevel.
	dest.x = xLeftBevel;
	src.x = X_LEFT_BEVEL;
	src.y = Y_CENTER;
	src.w = dest.w = CX_CORNER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw right bevel.
	dest.x = xRightBevel;
	src.x = X_RIGHT_BEVEL;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw center.
	src.x = X_CENTER;
	src.y = Y_CENTER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		src.w = dest.w = CX_CENTER;
		for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
		{
			if (dest.x + CX_CENTER > xRightBevel)
				dest.w = src.w = xRightBevel - dest.x; //Clip the blit to remaining width.
			if (dest.y + CY_CENTER > yBottomBevel)
				dest.h = src.h = yBottomBevel - dest.y; //Clip the blit to remaining height.
			SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
		}
	}

	if (IsSelected())
		DrawFocused(0);

	DrawButtonText(0);
}

//******************************************************************************
void CButtonWidget::DrawPressed(void)
//Draw button in its pressed state.
{
	SDL_Surface *pDestSurface = GetDestSurface();

	//Dest coords and dimensions.
	UINT xLeftBevel = this->x;
	UINT xRightBevel = this->x + this->w - CX_CORNER;
	UINT yTopBevel = this->y;
	UINT yBottomBevel = this->y + this->h - CY_CORNER;
	UINT xCenter = xLeftBevel + CX_CORNER;
	UINT yCenter = yTopBevel + CY_CORNER;

	//If the button bevel is enlarged beyond 1 pixel, then it is no longer
	//visually correct to blit the same top-right and bottom-left corner as
	//DrawNormal() does.
	ASSERT(CX_CORNER == 1 && CY_CORNER == 1);

	//Draw top-left corner--use bottom-right corner for source.
	SDL_Rect src = {X_RIGHT_BEVEL, Y_BOTTOM_BEVEL, CX_CORNER, CY_CORNER};
	SDL_Rect dest = {xLeftBevel, yTopBevel, CX_CORNER, CY_CORNER};
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw top-right corner.
	src.y = Y_TOP_BEVEL;
	dest.x = xRightBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-right corner--use top-left corner for source.
	src.x = X_LEFT_BEVEL;
	dest.y = yBottomBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-left corner.
	src.y = Y_BOTTOM_BEVEL;
	dest.x = xLeftBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw top bevel--use bottom bevel for source.
	src.x = X_CENTER;
	dest.y = yTopBevel;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw bottom bevel--use top bevel for source.
	src.y = Y_TOP_BEVEL;
	dest.y = yBottomBevel;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw left bevel--use right bevel for source.
	dest.x = xLeftBevel;
	src.x = X_RIGHT_BEVEL;
	src.y = Y_CENTER;
	src.w = dest.w = CX_CORNER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw right bevel--use left bevel for source.
	dest.x = xRightBevel;
	src.x = X_LEFT_BEVEL;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw top row of center.
	src.x = X_CENTER;
	src.y = Y_CENTER + CY_CENTER - 1;
	src.h = dest.h = 1;
	src.w = dest.w = CX_CENTER;
	dest.y = yCenter;
	for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the blit to remaining width.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw rest of center.
	src.y = Y_CENTER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter + 1; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		src.w = dest.w = CX_CENTER;
		for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
		{
			if (dest.x + CX_CENTER > xRightBevel)
				dest.w = src.w = xRightBevel - dest.x; //Clip the blit to remaining width.
			if (dest.y + CY_CENTER > yBottomBevel)
				dest.h = src.h = yBottomBevel - dest.y; //Clip the blit to remaining height.
			SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
		}
	}

	if (IsSelected())
		DrawFocused(1);

	//Draw text -- one pixel lower.
	DrawButtonText(1);
}

// $Log: ButtonWidget.cpp,v $
// Revision 1.11  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.10  2003/09/16 16:02:55  schik
// Fixed memory leak
//
// Revision 1.9  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.8  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/07/09 04:53:01  schik
// Fixed a small memory leak
//
// Revision 1.6  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/26 17:20:27  mrimer
// Removed some includes.  Revised some code.
//
// Revision 1.4  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.2  2003/05/23 21:41:23  mrimer
// Added portability for APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.23  2003/05/04 00:04:02  mrimer
// Fixed some var types.
//
// Revision 1.22  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.21  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.20  2002/08/23 23:26:19  erikh2000
// Sound effect now plays when button is pressed.
//
// Revision 1.19  2002/07/19 20:38:36  mrimer
// Revised HandleKeyDown() to return no type.
//
// Revision 1.18  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.17  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.16  2002/06/14 02:29:30  erikh2000
// Made disabled buttons drawn without hotkey-colored text.
//
// Revision 1.15  2002/06/14 00:44:44  erikh2000
// Changed static surfacecolor declarations to const to prevent errors.
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
// Changed focus rect color to subtler dark brown.
//
// Revision 1.14  2002/06/13 21:07:51  erikh2000
// Made depressed button paint focus rect offseted down instead of down and right.
//
// Revision 1.13  2002/06/11 22:57:45  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.12  2002/06/07 17:49:41  mrimer
// Added SDL_Delay in the click graphic.
//
// Revision 1.11  2002/06/06 00:01:14  mrimer
// Refined focus drawing code.
//
// Revision 1.10  2002/06/05 03:11:00  mrimer
// Added focus graphic.
//
// Revision 1.9  2002/06/03 22:58:06  mrimer
// Added/refined focusability.
//
// Revision 1.8  2002/06/02 22:19:23  erikh2000
// Fixed problem getting text centered.
//
// Revision 1.7  2002/05/31 23:46:16  mrimer
// Added hotkey support.
//
// Revision 1.6  2002/05/25 04:31:09  mrimer
// Added DrawHotkeyTextToLine().
// Consolidated specified SDL_Colors to .h file.
//
// Revision 1.5  2002/05/20 18:37:41  mrimer
// Added disabled state to CButtonWidget.
//
// Revision 1.4  2002/05/12 03:13:44  erikh2000
// Added code to load/unload child widgets.
//
// Revision 1.3  2002/04/29 00:06:19  erikh2000
// Renamed "depressed" to "pressed" for consistency.
//
// Revision 1.2  2002/04/16 10:40:19  erikh2000
// Added text caption to button painting.
//
// Revision 1.1  2002/04/13 19:28:51  erikh2000
// Initial check-in.
//
