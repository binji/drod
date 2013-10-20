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

#include "Widget.h"
#include "BitmapManager.h"
#include "EventHandlerWidget.h"
#include "Screen.h"
#include <BackEndLib/Assert.h>

#include <SDL.h>

#include <list>

SDL_Surface *CWidget::pPartsSurface = NULL;
UINT CWidget::wPartsSurfaceRefs = 0;

static SDL_Surface *m_pScreenSurface = NULL;

//***************************************************************************
void SetWidgetScreenSurface(
//Widgets will work after you say what the screen surface is.
//
//Params:
	SDL_Surface *pSetScreenSurface) //(in)
{
	ASSERT(pSetScreenSurface);
	m_pScreenSurface = pSetScreenSurface;
}

//***************************************************************************
SDL_Surface * GetWidgetScreenSurface()
//Returns the screen surface that is the default destination surface for
//all widgets.
//
//NOTE: Don't use this for drawing operations inside a widget.  That will work
//fine until we want the widget to draw on a non-screen surface.  Use the 
//surface returned from CWidget::GetDestSurface() instead.
{
	return m_pScreenSurface;
}

//
//CWidget protected methods.
//

//***************************************************************************
CWidget::CWidget(
//Constructor.
//
//Params:
	WIDGETTYPE eSetType,	//(in)	Type of widget.
	DWORD dwSetTagNo,		//(in)	Arbitrary ID for this widget.
	int nSetX, int nSetY,	//(in)	Rect that widget's paint area lies
	UINT wSetW, UINT wSetH)	//		within.
	: bIsLoaded(false)
	, bIsEnabled(true)

	, dwTagNo(dwSetTagNo)
	, x(nSetX)
	, y(nSetY)
	, w(wSetW)
	, h(wSetH)
	, nChildrenScrollOffsetX(0), nChildrenScrollOffsetY(0)
   , eType(eSetType)
	, pParent(NULL)
	, bIsVisible(true)

	, pHotkeys(NULL)
	, nHotkeys(0)

	, pSurface(NULL)	//Destination surface is the screen.
{
	//Check for reserved tag usage.
	ASSERT(!IS_RESERVED_TAG(dwSetTagNo));
}

//***************************************************************************
CWidget::~CWidget()
//Destructor.
{
	ASSERT(!this->bIsLoaded);
	ClearChildren();
	RemoveAllHotkeys();
}

//***************************************************************************
void CWidget::ClearChildren()
//Clear all the widget children.
{
	//Delete the CWidgets.
	for (WIDGET_ITERATOR iSeek = this->Children.begin();
			iSeek != this->Children.end(); ++iSeek)
	{
		if ((*iSeek)->bIsLoaded) (*iSeek)->Unload();
		delete *iSeek;
	}

	//Clear the list.
	this->Children.clear();
}

//***************************************************************************
void CWidget::GetScrollOffset(
//Gets the offset that should be applied to the position of this widget and 
//its children.  If you add the offset to the x,y members, you will have the
//position of the widget on the screen.  The widget may be outside of the
//screen or the clipping area of its parent.
//
//Params:
	int &nOffsetX, int &nOffsetY)	//(out)	Returns offsets.
const
{
	nOffsetX = nOffsetY = 0;

	//Add scroll offsets from all parents of this widget.
	const CWidget *pSeekParent = this->pParent;
	while (pSeekParent)
	{
		nOffsetX += pSeekParent->nChildrenScrollOffsetX;
		nOffsetY += pSeekParent->nChildrenScrollOffsetY;
		pSeekParent = pSeekParent->pParent;
	}
}

//***************************************************************************
bool CWidget::IsScrollOffset()
//Is there a scroll offset for this widget?  If caller is planning on using
//the offsets, just call GetScrollOffset() without calling this method.  Use
//this method if you just want to make a quick check.
//
//Returns:
//True if scrolling offsets are specified for any parent of this widget.
const
{
	const CWidget *pSeekParent = this->pParent;
	while (pSeekParent)
	{
		if (pSeekParent->nChildrenScrollOffsetX ||
				pSeekParent->nChildrenScrollOffsetY)
			return true; //Found at least one offset.
		pSeekParent = pSeekParent->pParent;
	}

	//No scrolling offsets found.
	return false;
}

//***************************************************************************
CWidget * CWidget::GetFirstSibling()
//Get the first sibling of this widget.
{
	if (!this->pParent) 
	{
		ASSERT(this->eType == WT_Screen);	//If fires, caller is probably trying
											//to get sibling of a widget that 
											//hasn't been added to a parent yet.
		return this;
	}

	ASSERT(this->pParent->Children.size());

	return *(this->pParent->Children.begin());
}

//***************************************************************************
CWidget * CWidget::GetLastSibling()
//Get the last sibling of this widget.
{
	if (!this->pParent) 
	{
		ASSERT(this->eType == WT_Screen);	//If fires, caller is probably trying
											//to get sibling of a widget that 
											//hasn't been added to a parent yet.
		return this;
	}

	ASSERT(this->pParent->Children.size());

	WIDGET_ITERATOR iRet = this->pParent->Children.end();
	return *(--iRet);
}

//***************************************************************************
CWidget * CWidget::GetPrevSibling()
//Get the previous sibling of this widget.
{
	if (!this->pParent) 
	{
		ASSERT(this->eType == WT_Screen);	//If fires, caller is probably trying
											//to get sibling of a widget that 
											//hasn't been added to a parent yet.
		return this;
	}

	ASSERT(this->pParent->Children.size());

	if (this->pParent->Children.size()==1) return this; //Widget is an only-child.

	for (WIDGET_ITERATOR iSeek = this->pParent->Children.begin(); 
			iSeek != this->pParent->Children.end(); ++iSeek)
	{
		if (*iSeek == this) //Found this widget in parent's child list.
		{
			//Get previous widget.
			if (iSeek == this->pParent->Children.begin())
				return NULL;
			else
				return *(--iSeek);
		}
	}

	ASSERTP(false, "Parent's child list or this widget's parent pointer is incorrect.");
	return NULL;
}

//***************************************************************************
CWidget * CWidget::GetNextSibling()
//Get the next sibling of this widget.
{
	if (!this->pParent) 
	{
		ASSERT(this->eType == WT_Screen);	//If fires, caller is probably trying
											//to get sibling of a widget that 
											//hasn't been added to a parent yet.
		return this;
	}

	ASSERT(this->pParent->Children.size());

	if (this->pParent->Children.size()==1) return this; //Widget is an only-child.

	for (WIDGET_ITERATOR iSeek = this->pParent->Children.begin(); 
			iSeek != this->pParent->Children.end(); ++iSeek)
	{
		if (*iSeek == this) //Found this widget in parent's child list.
		{
			//Get next widget.
			if (++iSeek == this->pParent->Children.end())
				return NULL;
			else
				return *iSeek;
		}
	}

	ASSERTP(false, "Parent's child list or this widget's parent pointer is incorrect.");
	return NULL;
}

//***************************************************************************
CEventHandlerWidget * CWidget::GetParentEventHandlerWidget() const
{
	if (!this->pParent) 
		return NULL; //This widget doesn't have an event handler.
	else
		return this->pParent->GetEventHandlerWidget();
}

//***************************************************************************
CEventHandlerWidget * CWidget::GetEventHandlerWidget() const
//Find event-handler widget that handles events for this widget.
//
//Returns:
//Pointer to a event-handling widget or NULL if none found.
{
	//Check against known list of widget types that are event handlers.
	if (
			this->eType == WT_Screen || 
			this->eType == WT_Dialog)
		//This widget is an event-handler.
		return (CEventHandlerWidget *)(this);
	
	//Check parent(s).
	return GetParentEventHandlerWidget();
}

//***************************************************************************
void CWidget::SetParent(
//Sets parent widget of this widget.
//
//Params:
	CWidget *const pSetParent)	//(in)
{
	ASSERT(pSetParent);
	this->pParent = pSetParent;
}

//***************************************************************************
void CWidget::PaintChildren(
//Paint all the visible child widgets.
//
//Params:
	bool bUpdateRects)	//(in)	If true, the SDL_UpdateRect will be called
						//		for each painted widget.  This will immediately
						//		update the screen if destination surface is
						//		pointing to it.  Default is false.
{
	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
			iSeek != this->Children.end(); ++iSeek)
	{
		if ((*iSeek)->IsVisible()) 
		{
			//Check for a completely out-of-bounds widget.
			if (!(*iSeek)->OverlapsRect(this->x + nOffsetX, this->y + nOffsetY, 
					this->w, this->h))
				continue;

			//Check for clipping.
			if ((*iSeek)->IsInsideOfRect(this->x + nOffsetX, 
					this->y + nOffsetY, this->w, this->h))
				(*iSeek)->Paint(bUpdateRects);
			else
				(*iSeek)->PaintClipped(this->x + nOffsetX, this->y + nOffsetY, 
						this->w, this->h, bUpdateRects);
		}
	}
}

//*****************************************************************************
bool CWidget::Load()
//Overridable method that loads resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	
	//Load bitmaps containing parts needed to depict common widgets.
   if (!CWidget::pPartsSurface)
   {
      ASSERT(CWidget::wPartsSurfaceRefs == 0);
      CWidget::pPartsSurface = g_pTheBM->GetBitmapSurface("Dialog");
      if (!CWidget::pPartsSurface)
         return false;
   }
   ++CWidget::wPartsSurfaceRefs;

	this->bIsLoaded = LoadChildren();
	
	return true;
}

//*****************************************************************************
void CWidget::Unload()
//Overridable method that unloads resources for the widget.
{
	ASSERT(this->bIsLoaded);
	
	UnloadChildren();
	
   ASSERT(CWidget::wPartsSurfaceRefs > 0);
   --CWidget::wPartsSurfaceRefs;
   if (CWidget::wPartsSurfaceRefs == 0)
   {
      ASSERT(this->pPartsSurface);
		g_pTheBM->ReleaseBitmapSurface("Dialog");
      CWidget::pPartsSurface = NULL;
	}

   this->bIsLoaded = false;
}

//***************************************************************************
bool CWidget::LoadChildren()
//Load all the widget children.
//
//Returns:
//True if all children were loaded, false if one load failed.
{
	//Load the CWidgets.
	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
			iSeek != this->Children.end(); ++iSeek)
	{
		if (!(*iSeek)->IsLoaded())
		{
			if (!(*iSeek)->Load()) return false;
			ASSERT((*iSeek)->IsLoaded()); //Check that derived classes 
										//are setting bIsLoaded.
		}
	}

	///All widgets loaded.
	return true;
}

//***************************************************************************
void CWidget::UnloadChildren()
//Unload all the widget children.
{
	//Unload the CWidgets.
	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
			iSeek != this->Children.end(); ++iSeek)
	{
		if ((*iSeek)->IsLoaded())
		{
			(*iSeek)->Unload();
			ASSERT(!(*iSeek)->IsLoaded());	//Check that derived classes 
											//are clearing bIsLoaded.
		}
	}
}

//
//CWidget public methods.
//

#define PPIXEL(col,row) ( (Uint8 *)pDestSurface->pixels + \
	((row) * pDestSurface->pitch) + \
	((col) * pDestSurface->format->BytesPerPixel) )

//***************************************************************************
void CWidget::Scroll(
//Sets offset that children are moved by when painted.  Used for scrolling
//children inside of the widget.  If children will be moving outside of widget
//area, you should make sure that all children have a working PaintClipped()
//method.
//
//The return parameters values of GetScrollOffset() for children of this 
//widget will be affected by this call.
//
//Params:
	int dx,			//(in)	Offsets to apply to children.  
	int dy)			//		Use negative numbers for left
					//		and up offsets.
{
	this->nChildrenScrollOffsetX += dx;
	this->nChildrenScrollOffsetY += dy;
}

//***************************************************************************
void CWidget::ScrollAbsolute(
//Sets offset that children are moved by when painted.  Used for scrolling
//children inside of the widget.  If children will be moving outside of widget
//area, you should make sure that all children have a working PaintClipped()
//method.
//
//The return parameters values of GetScrollOffset() for children of this 
//widget will be affected by this call.
//
//Params:
	int nSetOffsetX,	//(in)	Offsets to apply to children.  
	int nSetOffsetY)	//		Use negative numbers for left
						//		and up offsets.  0,0 removes all offsets.
{
	this->nChildrenScrollOffsetX = nSetOffsetX;
	this->nChildrenScrollOffsetY = nSetOffsetY;
}

//***************************************************************************
void CWidget::GetRectContainingChildren(
//Gets the smallest rect that will contain all children of this widget.
//
//Params:
	SDL_Rect &ChildContainerRect)	//(out)
const
{
	//Is there at least one widget?
	WIDGET_ITERATOR iSeek = this->Children.begin();
	SDL_Rect ChildRect;
	if (iSeek == this->Children.end())
	{
		//No--Set the container to zero width/depth and return early.
		SET_RECT(ChildContainerRect, 0, 0, 0, 0);
		return;
	}
	else
	{
		//Yes--Set the container to the first child.
		(*iSeek)->GetRect(ChildRect);
		COPY_RECT(ChildRect, ChildContainerRect);
	}

	//For any remaining children, expand boundaries of container to include them.
	for (++iSeek; //Advance to second child.
			iSeek != this->Children.end(); ++iSeek)
	{
		(*iSeek)->GetRect(ChildRect);

		//Expand left of container to include child if needed.
		int nDiffX = ChildContainerRect.x - ChildRect.x;
		if (nDiffX > 0)
		{
			ChildContainerRect.x -= nDiffX;
			ChildContainerRect.w += nDiffX;
		}

		//Expand top of container to include child if needed.
		int nDiffY = ChildContainerRect.y - ChildRect.y;
		if (nDiffY > 0)
		{
			ChildContainerRect.y -= nDiffY;
			ChildContainerRect.h += nDiffY;
		}

		//Expand right of container to include child if needed.
		int nChildRightX = ChildRect.x + ChildRect.w;
		int nContainerRightX = ChildContainerRect.x + ChildContainerRect.w;
		if (nChildRightX > nContainerRightX)
			ChildContainerRect.w += (nChildRightX - nContainerRightX);

		//Expand bottom of container to include child if needed.
		int nChildBottomY = ChildRect.y + ChildRect.h;
		int nContainerBottomY = ChildContainerRect.y + ChildContainerRect.h;
		if (nChildBottomY > nContainerBottomY)
			ChildContainerRect.h += (nChildBottomY - nContainerBottomY);
	}
}

//***************************************************************************
void CWidget::HideChildren()
//Hides all children of this widget, but not the widget itself.  If you wish
//to hide both, simply call Hide().
{
	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
			iSeek != this->Children.end(); ++iSeek)
	{
		(*iSeek)->Hide();
	}
}

//***************************************************************************
void CWidget::ShowChildren()
//Shows all children of this widget, but not the widget itself.  If you wish
//to show both, simply call Show().
{
	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
			iSeek != this->Children.end(); ++iSeek)
	{
		(*iSeek)->Show();
	}
}

//***************************************************************************
void CWidget::DrawPlaceholder()
//Draws a solid rect over the widget area.
{
	SURFACECOLOR Color = GetSurfaceColor(GetDestSurface(), RGB_PLACEHOLDER);
	SDL_Rect rect = {this->x, this->y, this->w, this->h};
	DrawFilledRect(rect, Color);
}

//*****************************************************************************
void CWidget::Center()
//Centers this widget inside its parent area.
{
   ASSERT(this->pParent);
	const int nMoveX = (static_cast<int>(this->pParent->w) - static_cast<int>(this->w)) / 2;
	const int nMoveY = (static_cast<int>(this->pParent->h) - static_cast<int>(this->h)) / 2;
	Move(nMoveX, nMoveY);
}

//*****************************************************************************
bool CWidget::ContainsCoords(
//Does the widget's area contain specified coordinates?
//
//Params:
	const int nX, const int nY) //(in) Coords to compair against.
//
//Returns:
//True if it does, false if not.
const
{
	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	return (nX >= this->x + nOffsetX && nY >= this->y + nOffsetY &&
			nX < this->x + nOffsetX + static_cast<int>(this->w) && nY < this->y +
         nOffsetY + static_cast<int>(this->h));
}

//*****************************************************************************
bool CWidget::IsInsideOfParent()
//Is the widget's area completely inside of its parent?
//
//Returns:
//True if it is, false if not.
const
{
	SDL_Rect ParentRect;
	this->pParent->GetRect(ParentRect);
	return IsInsideOfRect(ParentRect.x, ParentRect.y, 
			ParentRect.w, ParentRect.h);
}

//*****************************************************************************
bool CWidget::OverlapsRect(
//Does any part of this widget overlap a specified rectangle?
//
//Params:
	const int nX, const int nY,
	const UINT wW, const UINT wH) //(in) Rect to compare against.
//
//Returns:
//True if it does, false if not.
const
{
	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	return (!(
	
			//Rect is too far left to overlap.
			(int)(nX + wW) < this->x + nOffsetX ||

			//Rect is too far right.
			nX > (int)(this->x + nOffsetX + this->w) ||

			//Rect is too far up.
			(int)(nY + wH) < this->y + nOffsetY ||

			//Rect is too far down.
			nY > (int)(this->y + nOffsetY + this->h)) );
}

//*****************************************************************************
bool CWidget::IsInsideOfRect(
//Is the widget's area completely inside of a specified rectangle?
//
//Params:
	int nX, int nY, UINT wW, UINT wH)	//(in) Rect to compare against.
//
//Returns:
//True if it is, false if not.
const
{
	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	if (this->x + nOffsetX < nX ||
			this->x + nOffsetX + this->w > nX + wW ||
			this->y + nOffsetY < nY ||
			this->y + nOffsetY + this->h > nY + wH)
		return false;
	else
		return true;
}
	
//*****************************************************************************
void CWidget::PaintClipped(
//Paint widget inside of a clipped area.  PaintChildren() will automatically
//call this for children with rects outside of the widget area.  You should
//override this method if your widget uses direct pixel access and is expected
//to be clipped.
//
//Params:
	const int nX, const int nY,   //(in)	The rect to clip using screen coords.
   const UINT wW, const UINT wH,	//(in)	The rect to clip using screen coords.
	const bool bUpdateRect)			//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	SDL_Rect ClipRect = {nX, nY, wW, wH};
	
	SDL_Surface *pDestSurface = GetDestSurface();
	SDL_SetClipRect(pDestSurface, &ClipRect);
	Paint(bUpdateRect);
	SDL_SetClipRect(pDestSurface, NULL);
}

//*****************************************************************************
void CWidget::PaintClippedInsideParent(
//Calls PaintClipped() with parent widget rect.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	PaintClipped(pParent->x, pParent->y, pParent->w, pParent->h, bUpdateRect);
}

//*****************************************************************************
SDL_Surface * CWidget::GetDestSurface() const
//Get surface that should be used for all drawing operations involving this
//widget.
{
	//If surface is NULL, return the screen surface, otherwise return the
	//explicitly set surface.
	return (this->pSurface == NULL) ?
		m_pScreenSurface : this->pSurface;
}

//*****************************************************************************
void CWidget::SetDestSurface(
//Set surface that should be used for all drawing operations involving this
//widget.  Child widgets will have their surfaces set as well.
//
//Params:
	SDL_Surface *pSetSurface)	//(in)	New destination surface.  NULL indicates 
								//		that screen surface will be used.
{
	//Set this widget's dest surface (and children).
	this->pSurface = pSetSurface;
	SetChildrenDestSurface(pSetSurface);
}

//*****************************************************************************
void CWidget::SetChildrenDestSurface(
//Same as SetDestSurface(), but only changes dest surface of children.
//
//Params:
	SDL_Surface *pSetSurface)	//(in)	New destination surface.  NULL indicates 
								//		that screen surface will be used.
{
	//Set children.
	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
				iSeek != this->Children.end(); ++iSeek)
		(*iSeek)->SetDestSurface(pSetSurface);
}

//*****************************************************************************
void CWidget::UpdateRect() const
//Updates dest surface for this widget's rect.
{
   if (IsLocked()) return; //never call SDL_UpdateRect when surface is locked

   int nUpdateX, nUpdateY, nW = this->w, nH = this->h;
	GetScrollOffset(nUpdateX, nUpdateY);
   nUpdateX += this->x;
   nUpdateY += this->y;

   //Bounds checks.  Crop update to screen area.
   if (nUpdateX < 0)
   {
      nW += nUpdateX;
      nUpdateX = 0;
   }
   if (nUpdateY < 0)
   {
      nH += nUpdateY;
      nUpdateY = 0;
   }
   if (nUpdateX + nW > CScreen::CX_SCREEN)
      nW = CScreen::CX_SCREEN - nUpdateX;
	if (nUpdateY + nH > CScreen::CY_SCREEN)
      nH = CScreen::CY_SCREEN - nUpdateY;

   SDL_UpdateRect(GetDestSurface(), nUpdateX, nUpdateY, nW, nH);
}

//*****************************************************************************
void CWidget::Move(
//
//Params:
	const int nSetX, const int nSetY)	//(in)
{
	int dx, dy;
	if (this->pParent)
	{
		dx = nSetX - (this->x - this->pParent->x);
		dy = nSetY - (this->y - this->pParent->y);
	}
	else
	{
		dx = nSetX - this->x;
		dy = nSetY - this->y;
	}
	this->x += dx; 
	this->y += dy;
	
	//Move children.
	if (dx || dy)
	{
		for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
				iSeek != this->Children.end(); ++iSeek)
		{
			(*iSeek)->Move((*iSeek)->x - this->x + dx, 
					(*iSeek)->y - this->y + dy);
		}
	}
}

//*****************************************************************************
void CWidget::Resize(
//
//Params:
	const UINT wSetW, const UINT wSetH)	//(in)
{
	this->w	= wSetW;
	this->h	= wSetH;
}

//*****************************************************************************
void CWidget::DrawFilledRect(
//Draws a filled rectangle to the destination surface.
//
//Params:
	const SDL_Rect &rect,		//(in)
	const SURFACECOLOR &Color,	//(in)
   SDL_Surface *pDestSurface) //(in) [default = NULL]
{
	if (!pDestSurface)
      pDestSurface = GetDestSurface();
	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	ASSERT(wBPP == 3);

	//Calc offset between end of a row and beginning of next.
	const DWORD dwRowOffset = pDestSurface->pitch - (rect.w * wBPP);

	//Calc location of topleft pixel.
	Uint8 *pSeek = (Uint8 *)(pDestSurface->pixels) + 
			(rect.y * pDestSurface->pitch) + (rect.x * wBPP);

	//Calc location of bottomleft pixel plus one row too far.
	const Uint8 *pEndOfSeek = pSeek + (rect.h * pDestSurface->pitch);

	//Lock surface.
	LockDestSurface(pDestSurface);

	//Each iteration fills one row of pixels.
	while (pSeek != pEndOfSeek)
	{
		ASSERT(pSeek < pEndOfSeek);
		const Uint8 *pEndOfRow = pSeek + (rect.w * wBPP);

		//Each iteration sets 3 bytes (1 pixel).
		while (pSeek != pEndOfRow)
		{
			pSeek[0] = Color.byt1;
			pSeek[1] = Color.byt2;
			pSeek[2] = Color.byt3;
			pSeek += wBPP;
		}

		//Advance to beginning of next row.
		pSeek += dwRowOffset;
	}

	//Unlock surface.
	UnlockDestSurface(pDestSurface);
}

//*****************************************************************************
void CWidget::DrawRect(
//Draws a rectangle (unfilled) to the dest surface.
//
//Params:
	const SDL_Rect &rect,		//(in)
	const SURFACECOLOR &Color,	//(in)
   SDL_Surface *pDestSurface) //(in) [default = NULL]
{
	if (!pDestSurface)
      pDestSurface = GetDestSurface();
	static UINT wBPP = pDestSurface->format->BytesPerPixel;
	ASSERT(wBPP == 3);

	LockDestSurface(pDestSurface);

	//Draw top row.
	Uint8 *pSeek = PPIXEL(rect.x, rect.y);
	Uint8 *pStop = pSeek + (rect.w * wBPP);
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		pSeek[0] = Color.byt1;
		pSeek[1] = Color.byt2;
		pSeek[2] = Color.byt3;
		pSeek += wBPP;
	}

	if (rect.h >= 2)
	{
		//Draw bottom row.
		pSeek = PPIXEL(rect.x, rect.y + rect.h - 1);
		pStop = pSeek + (rect.w * wBPP);
		while (pSeek != pStop)
		{
			ASSERT(pSeek < pStop);
			pSeek[0] = Color.byt1;
			pSeek[1] = Color.byt2;
			pSeek[2] = Color.byt3;
			pSeek += wBPP;
		}
		
		//Draw left column.
		pSeek = PPIXEL(rect.x, rect.y + 1);
		pStop = pSeek + (rect.h - 2) * pDestSurface->pitch;
		while (pSeek != pStop)
		{
			ASSERT(pSeek < pStop);
			pSeek[0] = Color.byt1;
			pSeek[1] = Color.byt2;
			pSeek[2] = Color.byt3;
			pSeek += pDestSurface->pitch;
		}
		
		//Draw right column.
		pSeek = PPIXEL(rect.x + rect.w - 1, rect.y + 1);
		pStop = pSeek + (rect.h - 2) * pDestSurface->pitch;
		while (pSeek != pStop)
		{
			ASSERT(pSeek < pStop);
			pSeek[0] = Color.byt1;
			pSeek[1] = Color.byt2;
			pSeek[2] = Color.byt3;
			pSeek += pDestSurface->pitch;
		}
	}

	//Unlock screen.
	UnlockDestSurface(pDestSurface);
}

//*****************************************************************************
void CWidget::DrawCol(
//Draws a column (vertical line) to the screen.
//
//Params:
	int nX, int nY,				//(in)	Coords for top of column.
	UINT wH,					//(in)	Height of column.
	const SURFACECOLOR &Color,	//(in)	Color of column pixels.
   SDL_Surface *pDestSurface) //(in)  default == NULL
{
   if (!pDestSurface)
	   pDestSurface = GetDestSurface();
	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	ASSERT(wBPP == 3);

	LockDestSurface(pDestSurface);

	//Draw column.
	Uint8 *pSeek = PPIXEL(nX, nY);
	Uint8 *pStop = pSeek + (pDestSurface->pitch * wH);
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		pSeek[0] = Color.byt1;
		pSeek[1] = Color.byt2;
		pSeek[2] = Color.byt3;
		pSeek += pDestSurface->pitch;
	}

	UnlockDestSurface(pDestSurface);
}

//*****************************************************************************
void CWidget::DrawRow(
//Draws a row (horizontal line) to the screen.
//
//Params:
	int nX, int nY,				//(in)	Coords for left of row.
	UINT wW,					//(in)	Width of row.
	const SURFACECOLOR &Color,	//(in)	Color of row pixels.
   SDL_Surface *pDestSurface) //(in)  default == NULL
{
   if (!pDestSurface)
	   pDestSurface = GetDestSurface();
	static UINT wBPP = pDestSurface->format->BytesPerPixel;
	ASSERT(wBPP == 3);

	LockDestSurface(pDestSurface);

	//Draw column.
	Uint8 *pSeek = PPIXEL(nX, nY);
	Uint8 *pStop = pSeek + (wBPP * wW);
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		pSeek[0] = Color.byt1;
		pSeek[1] = Color.byt2;
		pSeek[2] = Color.byt3;
		pSeek += wBPP;
	}

	UnlockDestSurface(pDestSurface);
}

//***************************************************************************
void CWidget::GetDestSurfaceColor(
//Gets bytes that can be written to destination surface to produce a pixel of 
//specified color.
//
//Params:
	Uint8 bytRed,		//(in)	Desired color.
	Uint8 bytGreen,		//
	Uint8 bytBlue,		//
	SURFACECOLOR &Color)//(out)	Returns 3 bytes in the structure.
const
{
	Color = GetSurfaceColor(GetDestSurface(), bytRed, bytGreen, bytBlue);
}

//***************************************************************************
CWidget *CWidget::AddWidget(
//Add a child widget to this widget.
//
//Params:
	CWidget *pWidget,	//(in)	Widget to add.
	bool bLoad)			//(in)	If true widget will be loaded.  False is default.
//
//Returns:
//Pointer to new widget (same as in param) of NULL if a load failed.  Caller
//should not delete the widget after a successful or unsuccessful call.
{
	ASSERT(pWidget);

	ASSERT(pWidget->GetTagNo() == 0L || pWidget->GetTagNo() == TAG_OK ||
			GetWidget(pWidget->GetTagNo()) == NULL);

	//Add focusable or animated widgets to lists.
	if (pWidget->IsFocusable() || pWidget->IsAnimated())
	{
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) 
		{
			if (pWidget->IsFocusable())
				pEventHandler->AddFocusWidget(pWidget);
			if (pWidget->IsAnimated())
				pEventHandler->AddAnimatedWidget(pWidget);
		}
	}

	//Move child widget relative to parent.
	pWidget->SetParent(this);
	pWidget->Move(pWidget->x, pWidget->y);
	this->Children.push_back(pWidget);

	if (bLoad && !pWidget->Load()) 
		return NULL;
	else
		return pWidget;
}

//***************************************************************************
void CWidget::RemoveWidget(
//Remove a child widget from this widget.
//
//Params:
	CWidget *pRemoveWidget)
{
	ASSERT(GetWidget(pRemoveWidget->GetTagNo()) != NULL); //Is widget a child?

	//Remove focusable or animated widgets from lists.
	if (pRemoveWidget->IsFocusable() || pRemoveWidget->IsAnimated())
	{
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) 
		{
			if (pRemoveWidget->IsFocusable())
				pEventHandler->RemoveFocusWidget(pRemoveWidget);
			if (pRemoveWidget->IsAnimated())
				pEventHandler->RemoveAnimatedWidget(pRemoveWidget);
		}
	}

	//Remove widget from child list.
	this->Children.remove(pRemoveWidget);

	//Free the widget and children's resources.
	pRemoveWidget->Unload();
	delete pRemoveWidget;
}

//***************************************************************************
CWidget * CWidget::GetWidget(
//Get a specific widget.
//
//Params:
	const DWORD dwFindTagNo,	//(in)	Identifies widget to get.
	const bool bFindVisibleOnly)	//(in)	Widget must be visible (default=false).
{
	CWidget *pFromChild;
	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
			iSeek != this->Children.end(); ++iSeek)
	{
		if (!bFindVisibleOnly || (*iSeek)->IsVisible())
		{
			if ((*iSeek)->GetTagNo() == dwFindTagNo)
				return *iSeek; //Found it.
			else
			{
				//Look recursively for the tag.
				pFromChild = (*iSeek)->GetWidget(dwFindTagNo);
				if (pFromChild) return pFromChild;
			}
		}
	}

	//No match.
	return NULL;
}

//***************************************************************************
CWidget * CWidget::GetWidgetContainingCoords(
//Gets widget that contains coords.  Recursively called on child widgets
//to get the widget with a smaller area containing the coords.  If two or more
//widget siblings contain the coords, then the first one found will be returned.
//Hidden and disabled widgets are ignored.
//
//Params:
	int nX, int nY,			//(in)	Coords to check against.
	WIDGETTYPE eWidgetType)	//(in)	Type of widget required for a match.  If
							//		WT_Unspecified, criteria will not be applied.
//
//Returns:
//Found widget or NULL if no match.
const
{
	//Each iteration looks at one child of this widget for a match.
	CWidget *pFromChild;
	for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
			iSeek != this->Children.end(); ++iSeek)
	{
		//Filter by visibility.
		if (!(*iSeek)->bIsVisible || !(*iSeek)->bIsEnabled) continue;

		//Check coords against widget.
		if ((*iSeek)->ContainsCoords(nX, nY)) //Found a match.
		{
			//Look recursively for an even closer match.
			pFromChild = (*iSeek)->GetWidgetContainingCoords(nX, nY, 
					eWidgetType);
			if (pFromChild && 
					(eWidgetType == WT_Unspecified || pFromChild->eType == eWidgetType) )
				return pFromChild;	//Matching child widget.
			else if ((eWidgetType == WT_Unspecified || (*iSeek)->eType == eWidgetType) )
				return *iSeek;		//No matching child widget so use this one.
		}
	}

	//No match.
	return NULL;
}

//*****************************************************************************
Uint32 CWidget::IsLocked() const
//Whether surface is locked.
{
	return GetDestSurface()->locked;
}

//*****************************************************************************
SDL_Surface *CWidget::LockDestSurface(SDL_Surface *pDestSurface) //(in) [default = NULL])
//Simplified call to SDL_LockSurface().
//
//Returns:
//The dest surface.  Caller could have also gotten it from GetDestSurface(), 
//but it is returned here for convenience.
{
   if (!pDestSurface)
	   pDestSurface = GetDestSurface();
	if (SDL_MUSTLOCK(pDestSurface)) 
	{
		while ( SDL_LockSurface(pDestSurface) < 0 ) 
			SDL_Delay(100);
	}
	return pDestSurface;
}

//*****************************************************************************
void CWidget::UnlockDestSurface(SDL_Surface *pDestSurface) //(in) [default = NULL]
//Simplified call to SDL_UnlockSurface().
{
   if (!pDestSurface)
	   pDestSurface = GetDestSurface();
	if (SDL_MUSTLOCK(pDestSurface)) SDL_UnlockSurface(pDestSurface);
}

//
//CWidget private methods.
//

//***************************************************************************
void CWidget::ClipWHToDest()
//Clip width and height of widget area to fit inside dest surface.
{
	SDL_Surface *pDestSurface = GetDestSurface();
	if (this->x + static_cast<int>(this->w) > pDestSurface->w)
		this->w = pDestSurface->w - this->x;
	if (this->y + static_cast<int>(this->h) > pDestSurface->h)
		this->h = pDestSurface->h - this->y;
}

//***************************************************************************
void CWidget::AddHotkey(
//Adds hotkey with given key and tag to this object.
//
//Params:
	const SDLKey key,	//(in)
	const DWORD tag)	//(in)
{
	UINT nIndex;

	//Is key already set as a hotkey?
	for (nIndex = 0; nIndex<this->nHotkeys; nIndex++)
		if (this->pHotkeys[nIndex].key == key)
		{
			//Change this key to new tag
			this->pHotkeys[nIndex].tag = tag;
			return;
		}

	//Add new hotkey assignment.
	++this->nHotkeys;
	this->pHotkeys = (HOTKEY*)realloc(this->pHotkeys,this->nHotkeys * sizeof(HOTKEY));
	this->pHotkeys[nIndex].key = key;
	this->pHotkeys[nIndex].tag = tag;
}

//***************************************************************************
DWORD CWidget::GetHotkeyTag(
//Returns: tag attached to hotkey 'key' for this object if it exists, else 0.
//
//Params:
	const SDLKey key)	//(in)
{
	UINT nIndex;
	DWORD tag;

	//Only check active widgets.
	if (IsActive())
	{
		//Is key set as a hotkey?
		for (nIndex = 0; nIndex<this->nHotkeys; nIndex++)
			if (this->pHotkeys[nIndex].key == key)
				return this->pHotkeys[nIndex].tag;

		//Recursively check childrens' hotkeys.
		for (WIDGET_ITERATOR iSeek = this->Children.begin(); 
				iSeek != this->Children.end(); ++iSeek)
		{
			tag = (*iSeek)->GetHotkeyTag(key);
			if (tag) return tag;
		}
	}

	//No hotkey mappings found for this key.
	return 0;
}

//***************************************************************************
void CWidget::RemoveHotkey(
//Removes hotkey with given tag from this object.
//
//Params:
	const DWORD tag)	//(in)
{
	UINT nIndex;

	//Find tag.
	for (nIndex = 0; nIndex<this->nHotkeys; nIndex++)
		if (this->pHotkeys[nIndex].tag == tag)
		{
			//Remove hotkey.
			//Slide further ones up in the array one spot.
			while (nIndex<this->nHotkeys-1)
			{
				this->pHotkeys[nIndex] = this->pHotkeys[nIndex+1];
				++nIndex;
			}
			//Remove last (empty) spot.
			--this->nHotkeys;
			this->pHotkeys = (HOTKEY*)realloc(this->pHotkeys,this->nHotkeys * sizeof(HOTKEY));
			break;
		}
}

//***************************************************************************
void CWidget::RemoveAllHotkeys()
//Deallocates all hotkeys in this object.
{
	this->nHotkeys = 0;
	free(this->pHotkeys);
	this->pHotkeys = NULL;
}

// $Log: Widget.cpp,v $
// Revision 1.13  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.12  2003/10/01 15:57:30  mrimer
// Fixed UpdateRect() method inheritance from CWidget to CScreen.
//
// Revision 1.11  2003/10/01 15:35:29  mrimer
// Fixed potential bug in UpdateRect().
//
// Revision 1.10  2003/08/16 00:47:04  mrimer
// Augmented surface methods with optional surface parameter.
// Fixed occasional bug in DrawFilledRect().
//
// Revision 1.9  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.8  2003/07/09 11:56:51  schik
// Made PaintClipped() arguments consistent among derived classes (VS.NET warnings)
//
// Revision 1.7  2003/07/07 23:38:46  mrimer
// Made parameter to SetParent() non-const.
//
// Revision 1.6  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/09 19:22:16  mrimer
// Added optional max width/height params to FM::DrawTextXY().
//
// Revision 1.4  2003/06/06 18:08:42  mrimer
// Added some SDL checks.  Fixed some assertion bugs.
//
// Revision 1.3  2003/05/28 22:58:49  erikh2000
// Corrected an error in calculated widget offsets.
// Added WT_ProgressBar.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.40  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.39  2002/12/22 02:16:04  mrimer
// Added GetParentEventHandlerWidget().
//
// Revision 1.38  2002/11/15 02:57:40  mrimer
// Made some vars and parameters const.
//
// Revision 1.37  2002/10/21 20:17:20  mrimer
// Modified GetWidget() to optionally retrieve only visible widgets.
//
// Revision 1.36  2002/09/30 18:42:00  mrimer
// Tweaking.
//
// Revision 1.35  2002/09/27 18:27:19  mrimer
// Fixed previous commit.
//
// Revision 1.34  2002/09/27 17:48:15  mrimer
// Fixed tag number checking.
//
// Revision 1.33  2002/09/04 20:40:15  mrimer
// Moved UpdateRect(rect) form CWidget to CScreen.
//
// Revision 1.32  2002/09/03 23:50:26  mrimer
// Overloaded UpdateRect to update only part of the screen.
//
// Revision 1.31  2002/09/03 21:41:48  erikh2000
// ClearChildren() is more robust.
// Scrolling offsets can be applied to all children of a widget and several methods taken them into account.
//
// Revision 1.30  2002/08/28 20:35:28  mrimer
// Added bounds checking in UpdateRect().
//
// Revision 1.29  2002/07/22 17:35:09  mrimer
// Made DrawRect() logic more robust.
//
// Revision 1.28  2002/07/20 23:20:16  erikh2000
// A widget can now be animated.  An animated widget is automatically added to an event handler's animation list.  The event handler calls a new HandleAnimate() method between events.
// Changed LockDestSurface() to return a point to dest surface for convenience.
// Added new documentation to top of Widget.h.
//
// Revision 1.27  2002/07/05 10:40:04  erikh2000
// Wrote usage comments.
// Added overridable event-handling methods.
// Changed AddWidget() and RemoveWidget() so that they affect the focus list.
// Wrote some methods for accessing sibling widgets.
//
// Revision 1.26  2002/06/21 03:32:24  erikh2000
// Added ShowChildren() and HideChildren() methods.
//
// Revision 1.25  2002/06/20 00:57:51  erikh2000
// Added method to draw a placeholder rect over the widget area.
//
// Revision 1.24  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.23  2002/06/14 18:30:49  mrimer
// Added assertion of child tag # uniqueness in AddWidget().
//
// Revision 1.22  2002/06/14 05:09:16  erikh2000
// Fixed error where widgets are randomly disabled.
//
// Revision 1.21  2002/06/14 02:33:37  erikh2000
// GetWidgetContainingCoords() now ignores disabled widgets.
//
// Revision 1.20  2002/06/14 01:08:13  erikh2000
// Made several changes to CWidget methods that make them use destination surface instead of screen surface.  Renamed some methods to reflect these changes.
// Made CWidget::pSurface value of NULL represent the screen surface, so that a Widget dest surface for the screen will not become invalid during fullscreen/windowed switches.
//
// Revision 1.19  2002/06/11 22:48:06  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.18  2002/06/07 22:59:16  mrimer
// Rearranged includes.
//
// Revision 1.17  2002/06/05 03:22:23  mrimer
// Added IsActive() call.
//
// Revision 1.16  2002/06/01 04:57:47  mrimer
// Finished implementing hotkeys.
//
// Revision 1.15  2002/05/31 23:46:16  mrimer
// Added hotkey support.
//
// Revision 1.14  2002/05/21 21:39:10  erikh2000
// Changed color-setting code.
//
// Revision 1.13  2002/05/12 03:24:38  erikh2000
// Change AddWidget() so that you could optionally add a widget and load it in one call.
// Added RemoveWidget() method.
// Added a second reserved tag--TAG_ESCAPE.
//
// Revision 1.12  2002/05/10 22:44:08  erikh2000
// Renamed "InitWidget()" to "SetWidgetScreenSurface()" to more accurately describe usage.
//
// Revision 1.11  2002/04/29 00:39:56  erikh2000
// Renamed TAG_SDLQUIT to TAG_QUIT.
//
// Revision 1.10  2002/04/25 18:34:58  mrimer
// Removed duplicate UpdateRect().
//
// Revision 1.9  2002/04/25 18:30:21  mrimer
// Added functions to switch between windowed and full-screen modes.
//
// Revision 1.8  2002/04/24 08:16:56  erikh2000
// Fixed an error where child widgets were being given wrong coords.
//
// Revision 1.7  2002/04/19 21:47:19  erikh2000
// Added GetWidgetContainingCoords() method.
//
// Revision 1.6  2002/04/14 00:41:24  erikh2000
// Added DrawCol() and DrawRow() methods.
// Changed parameters of methods to SURFACECOLOR for consistency.
// Moved GetSurfaceColor() to a mainline function in Colors.cpp, and created a new GetScreenSurfaceColor() method that calls it.
//
// Revision 1.5  2002/04/13 19:34:17  erikh2000
// Widget position is now set with relative coords and child widgets are moved with widget.
// Widget has a type variable that must be set at construction.
// Added some coordinate/rect calculation methods.
// Added clipping during paints.
//
// Revision 1.4  2002/04/12 22:53:48  erikh2000
// Twiddling.
//
// Revision 1.3  2002/04/11 10:13:19  erikh2000
// Added delay and retry logic to LockScreen().
//
// Revision 1.2  2002/04/09 10:05:41  erikh2000
// Fixed revision log macro.
//
