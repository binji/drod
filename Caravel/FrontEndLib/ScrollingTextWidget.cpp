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

#include "ScrollingTextWidget.h"

#include <BackEndLib/Assert.h>

#include <SDL_ttf.h>

//
//Public methods.
//

//******************************************************************************
CScrollingTextWidget::CScrollingTextWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	const UINT wPixelsPerSecond)		//(in)	Scroll rate.
	: CWidget(WT_ScrollingText, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, bPaused(false)
{
	this->pBackgroundSurface = NULL;
	SetScrollRate(wPixelsPerSecond);
	ClearText();
}

//******************************************************************************
void CScrollingTextWidget::ClearText(void)
//Erases all text.
{
	ClearChildren();
	ScrollAbsolute(0, 0);
	this->nNextLabelY = this->h;
	this->dwLastScroll = 0L;
}

//******************************************************************************
bool CScrollingTextWidget::IsAllTextShown(void) const
//Has all text been shown inside of the widget?  If there is still text waiting
//below the widget's scrolled view then the answer is "no".
//
//Returns:
//True if all text has been shown, false if not.
{
	//Amount of space to have below text before this returns true.
	static const UINT BUFFER_PIXELS = 15;

	if (!ContainsText())
		//Either no text has been added, or it has all scrolled past the top.
		return true;

	//Get the lowest text label.  It will be the last child of this widget
	//because of how AddText() works.
	WIDGET_ITERATOR iLowestLabel = this->Children.end();
	--iLowestLabel;
	CLabelWidget *pLowestLabel = DYN_CAST(CLabelWidget*, CWidget*, *iLowestLabel);

	return (pLowestLabel->GetY() + (int) pLowestLabel->GetH() +
		   this->nChildrenScrollOffsetY < static_cast<int>(this->y + this->h - BUFFER_PIXELS));
}

//******************************************************************************
void CScrollingTextWidget::Paint(
//Paint text inside of the widget area.
//
//Params:
	bool bUpdateRect)				//(in)	If true (default) and destination
									//		surface is the screen, the screen
									//		will be immediately updated in
									//		the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

	SDL_Surface *pDestSurface = GetDestSurface();

	if (this->pBackgroundSurface)
	{
		SDL_Rect BgSrc = {this->nBackgroundX, this->nBackgroundY, this->w, this->h};
		SDL_Rect BgDest = {this->x, this->y, this->w, this->h};
		if (BgSrc.x == -1) BgSrc.x = this->x;
		if (BgSrc.y == -1) BgSrc.y = this->y;
		SDL_BlitSurface(this->pBackgroundSurface, &BgSrc, pDestSurface, &BgDest);
	}

	PaintChildren();

	if (bUpdateRect) UpdateRect();
}

//******************************************************************************
void CScrollingTextWidget::AddText(
//Add new text item to list.
//
//Params:
	const WCHAR *pwczText,              //(in)
	const UINT eFontType,               //(in)
    CLabelWidget::TEXTALIGN eTextAlign) //(in)
{
	//If the last label is showing on the widget, put the next label just past
	//the bottom of the widget area, instead of right underneath the
	//already-showing last label.  It looks a little jarring to have it pop up
	//out of no where.
	if (
			//Cheap check--scrolling must occur before above condition can be true.
			this->nChildrenScrollOffsetY != 0 &&

			//The last label has been completely shown but has not scrolled off the
			//top yet.
			ContainsText() && IsAllTextShown())
		this->nNextLabelY = (int) this->h - this->nChildrenScrollOffsetY;

	//Add a new last label underneath any existing labels.
	CLabelWidget *pLabel = new CLabelWidget(0, 0, this->nNextLabelY,
			this->w, this->h, eFontType, pwczText, true);
	pLabel->SetAlign(eTextAlign);
	AddWidget(pLabel, true);

	//Height of label was resized to fit the text.  Increment nNextLabelY so
	//the next label will go underneath this label.
	this->nNextLabelY += pLabel->GetH();
}

//******************************************************************************
CLabelWidget *CScrollingTextWidget::GetLastShownLabel(void) const
//Gets the last (lowest) label that is currently shown inside the widget.
//
//Returns:
//Pointer to label or NULL if no label is showing.
{
	if (!this->Children.size()) return NULL; //No labels.

	//Looking at labels from last to first.  Labels are ordered by their
	//y coord because of how AddText() works.
	WIDGET_ITERATOR iSeek = this->Children.end();
	while (iSeek != this->Children.begin())
	{
		--iSeek;
		if ((*iSeek)->GetY() + this->nChildrenScrollOffsetY < static_cast<int>(this->y + this->h))
			//Found the lowest showing label.
			return DYN_CAST(CLabelWidget*, CWidget*, *iSeek);
	}

	//No labels are showing.
	return NULL;
}

//******************************************************************************
void CScrollingTextWidget::SetBackground(
//Sets background image that goes underneath text.
//
//Params:
	SDL_Surface *pSetSurface,	//(in)	Pointer to source surface used for future
								//		blits.  Caller must make sure the surface
								//		is available for life of this object or
								//		make another call to SetBackground() when
								//		the surface becomes unavailable.  Pass NULL
								//		to specify that no background surface be
								//		used.
	const int nSetX,			//(in)	Source coords to blit from source surface.
	const int nSetY)			//		If -1 (default), then the coords of this 
								//		widget will be used.
{
	//Make sure blit coords are not out-of-bounds.
	ASSERT(nSetX >= -1 && static_cast<int>(nSetX + this->w) < pSetSurface->w);
	ASSERT(nSetY >= -1 && static_cast<int>(nSetY + this->h) < pSetSurface->h);

	this->pBackgroundSurface = pSetSurface;
	this->nBackgroundX = nSetX;
	this->nBackgroundY = nSetY;
}

//******************************************************************************
void CScrollingTextWidget::ScrollText()
//Scroll text up according to how much time has passed since last scrolling.
{
	if (!this->Children.size()) return;
	if (this->bPaused) return;

	//Determine number of pixels to scroll up.
	const DWORD dwNow = SDL_GetTicks();
	if (!this->dwLastScroll)
		this->dwLastScroll = SDL_GetTicks();
	const UINT wPixels = (this->wMSPerPixel == 0) ? 0 :
			(dwNow - this->dwLastScroll) / this->wMSPerPixel;
	if (wPixels)
	{
		Scroll(0, -wPixels);
		this->dwLastScroll += wPixels * this->wMSPerPixel;
	}

	//Remove any labels that have scrolled off the top.
	WIDGET_ITERATOR iSeek = this->Children.begin();
	while (iSeek != this->Children.end())
	{
		WIDGET_ITERATOR i = iSeek;
		++iSeek;
		if ((*i)->GetY() + (int) (*i)->GetH() + this->nChildrenScrollOffsetY < 
				this->y)
			RemoveWidget(*i);
	}

	if (!this->Children.size())
	{
		//Reset scrolling settings.
		ClearText();
	}
}

//******************************************************************************
void CScrollingTextWidget::SetScrollRate(
//Sets scrolling text rate.
//A value of 0 pauses scrolling.
//
//Params:
	const UINT wPixelsPerSecond)
{
	if (wPixelsPerSecond == 0)
	{
		this->bPaused = true;
		return;
	}
	if (this->bPaused)
	{
		this->bPaused = false;
		this->dwLastScroll = 0L;	//reset after pause
	}
	if (wPixelsPerSecond)
		this->wMSPerPixel = 1000 / wPixelsPerSecond;
	else
		this->wMSPerPixel = 1000;
}

// $Log: ScrollingTextWidget.cpp,v $
// Revision 1.5  2003/07/22 18:42:19  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.4  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/05/28 23:00:41  erikh2000
// Using TA_* constants differently.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.14  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.13  2002/10/21 20:27:40  mrimer
// Added buffer to IsAllTextShown().
//
// Revision 1.12  2002/10/21 09:34:06  erikh2000
// Fixed division by zero bug causing intermittent crashes in win sequence.
//
// Revision 1.11  2002/10/09 00:55:47  erikh2000
// Fixed error causing labels to not be removed when they scroll off top.
//
// Revision 1.10  2002/10/04 17:54:26  mrimer
// Added pause control.
//
// Revision 1.9  2002/09/06 20:08:46  erikh2000
// When text is added while the end of the current text is showing inside the widget, the new text will be positioned just underneath the widget viewing area--it looks smoother.
//
// Revision 1.8  2002/09/05 18:24:06  erikh2000
// Added method to check that all check has been shown.  This typically happens before ContainsText() returns false and there is still text showing in the widget.
//
// Revision 1.7  2002/09/04 21:55:27  mrimer
// Added a comment.
//
// Revision 1.6  2002/09/04 21:47:14  mrimer
// Added real time scrolling speed parameter.
//
// Revision 1.5  2002/09/03 21:39:47  erikh2000
// Changed the implementation to use child labels to store and render texts, instead of one SDL surface.
// Added a method used to figure out what the last shown text is.
//
// Revision 1.4  2002/08/23 23:42:57  erikh2000
// Prefixed a var with "this->" to show it is a class member.
//
// Revision 1.3  2002/07/23 20:14:02  mrimer
// Added intermediate surface to avoid rerendering text.
// Removed list structure.
// More revisions to get it working.
//
// Revision 1.2  2002/07/22 21:03:00  mrimer
// Added TextBelowWidgetArea().  Fixed some bugs.
//
// Revision 1.1  2002/07/11 20:58:37  mrimer
// Initial check-in.
//
