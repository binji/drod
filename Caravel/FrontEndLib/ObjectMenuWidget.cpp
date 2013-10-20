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

#include "ObjectMenuWidget.h"
#include "BitmapManager.h"
#include "EventHandlerWidget.h"
#include <FrontEndLib/Sound.h>

const UINT CX_FOCUS_BUFFER = 2;
const UINT CY_FOCUS_BUFFER = 2;

//
//Public methods.
//

//******************************************************************************
CObjectMenuWidget::CObjectMenuWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	const int nSetX, const int nSetY,		//		constructor.
	const UINT wSetW, const UINT wSetH,		//
	const UINT wGapX, const UINT wGapY,		//(in)	Size of gaps between objects.
	const UINT wFloorTile)						//(in)   Image # of floor tile
	: CFocusWidget(WT_ObjectMenu, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, wGapX(wGapX)
	, wGapY(wGapY)
	, wFloorTile(wFloorTile)
	, pEraseSurface(NULL)
	, bSelectionDrawn(false)
{
	ASSERT(wSetW >= (UINT)CBitmapManager::CX_TILE);
	ASSERT(wSetH >= (UINT)CBitmapManager::CY_TILE);
	ASSERT(wGapX < wSetW/2);
	ASSERT(wGapY < wSetH/2);
	this->iterSelectedObject = this->lObjects.end();	//not a valid object
}

//******************************************************************************
CObjectMenuWidget::~CObjectMenuWidget()
//Destructor.
{
   for (list<MENUOBJECT *>::iterator i = this->lObjects.begin(); i != this->lObjects.end(); i++) {
      delete *i;
   }
   this->lObjects.clear();
}

//******************************************************************************
UINT CObjectMenuWidget::GetSelectedObject()
//Returns the ID of the currently selected object.
{
	if (this->iterSelectedObject == this->lObjects.end())
		return NO_SELECTED_OBJECT;
	else
		return (*this->iterSelectedObject)->wObjectNo;
}

//******************************************************************************
void CObjectMenuWidget::AddObject(
//Adds a widget to menu corresponding to certain tag
//as well as to list of children.
//
//Params:
	const UINT wObjectNo,	//(in) object id
	const UINT wXSize, const UINT wYSize,	//(in) # tiles to show for display
	const UINT* paTiles)		//(in) literal tiles to draw
{
	ASSERT(wXSize > 0);
	ASSERT(wYSize > 0);
	ASSERT(wXSize*CBitmapManager::CX_TILE < this->w - CX_FOCUS_BUFFER*2);
	ASSERT(wYSize*CBitmapManager::CY_TILE < this->h - CY_FOCUS_BUFFER*2);
	ASSERT(paTiles);

	MENUOBJECT *pObject = new MENUOBJECT;
	pObject->paTiles = (unsigned int*)paTiles;
	pObject->wObjectNo = wObjectNo;
	pObject->wXSize = wXSize;
	pObject->wYSize = wYSize;

	//Find next available spot to put object.
	int nX, nY;

	if (this->lObjects.empty())
	{
		//First object.  Place at top.
		pObject->nX = CX_FOCUS_BUFFER;
		pObject->nY = CY_FOCUS_BUFFER;
	} else {
		//Put after last object.
		MENUOBJECT *pLastObject = this->lObjects.back();
		nX = pLastObject->nX + pLastObject->wXSize*CBitmapManager::CX_TILE + this->wGapX;
		if (nX + wXSize*CBitmapManager::CX_TILE <= this->w - CX_FOCUS_BUFFER)
		{
			//Object will fit on current row.
			pObject->nX = nX;
			pObject->nY = pLastObject->nY;
		} else {
			//Object must be placed at beginning of next row.
			pObject->nX = CX_FOCUS_BUFFER;
			//Scan bottom row for lowest point and place below that.
			OBJECT_RITERATOR pTrav = this->lObjects.rbegin();
			//Calculate bottom of last object.
			int nMinX=nX, nMaxY = (*pTrav)->nY + (*pTrav)->wYSize*CBitmapManager::CY_TILE;
			while (pTrav != this->lObjects.rend())
			{
				//Have we finished the row we started on?
				if ((*pTrav)->nX >= nMinX)
					break;
				else
					nMinX = (*pTrav)->nX;
				//Calculate bottom of object.
				nY = (*pTrav)->nY + (*pTrav)->wYSize*CBitmapManager::CY_TILE; 
				if (nY > nMaxY)
					nMaxY = nY;
				++pTrav;
			}

			pObject->nY = nMaxY + this->wGapY;
			ASSERT(pObject->nY + wYSize*CBitmapManager::CY_TILE < this->h);
		}
	}

	this->lObjects.push_back(pObject);

	//If this was the first object added, select it now.
	if (this->iterSelectedObject == this->lObjects.end())
	{
		SetSelectedObject(this->lObjects.begin());
		this->iterPrevSelectedObject = this->lObjects.begin();
	}
}

//******************************************************************************
void CObjectMenuWidget::Paint(
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

	SDL_Surface *pSurface = GetDestSurface();
	MENUOBJECT *pObject;

   //Erase selection at previous location.
	if (this->bSelectionDrawn)
	{
		pObject = (*this->iterPrevSelectedObject);
		const UINT DRAWX = this->x + pObject->nX - 2;
		const UINT DRAWY = this->y + pObject->nY - 2;
		const UINT CX_FOCUS = pObject->wXSize * CBitmapManager::CX_TILE + 4;
		const UINT CY_FOCUS = pObject->wYSize * CBitmapManager::CY_TILE + 4;
		SDL_Rect SelectionScreenRect = {DRAWX, DRAWY, CX_FOCUS, CY_FOCUS};
		SDL_Rect EraseRect = {0, 0, CX_FOCUS, CY_FOCUS};
		SDL_BlitSurface(this->pEraseSurface, &EraseRect, 
				pSurface, &SelectionScreenRect);
	}

	//Draw objects in menu.
	UINT wX, wY, wIndex;
	for (OBJECT_ITERATOR iSeek = this->lObjects.begin(); 
			iSeek != this->lObjects.end(); ++iSeek)
	{
		pObject = (*iSeek);
		wIndex = 0;
		for (wY=0; wY<pObject->wYSize; wY++)
			for (wX=0; wX<pObject->wXSize; wX++)
			{
				//Draw floor under transparent objects.
				g_pTheBM->BlitLayeredTileImage(this->wFloorTile,
						pObject->paTiles[wIndex++],
						this->x + pObject->nX + wX*CBitmapManager::CX_TILE,
						this->y + pObject->nY + wY*CBitmapManager::CY_TILE, pSurface, 255);
			}
	}

	//Highlight selected object.
	if (this->iterSelectedObject != this->lObjects.end())
	{
		pObject = (*this->iterSelectedObject);
		const UINT DRAWX = this->x + pObject->nX - 2;
		const UINT DRAWY = this->y + pObject->nY - 2;
		const UINT CX_FOCUS = pObject->wXSize * CBitmapManager::CX_TILE + 4;
		const UINT CY_FOCUS = pObject->wYSize * CBitmapManager::CY_TILE + 4;
		SDL_Rect SelectionScreenRect = {DRAWX, DRAWY, CX_FOCUS, CY_FOCUS};

		//Save spot where selection box is drawn.
		SDL_Rect SelectionRect = {0, 0, CX_FOCUS, CY_FOCUS};
		SDL_BlitSurface(pSurface, &SelectionScreenRect,
				this->pEraseSurface, &SelectionRect);
		this->bSelectionDrawn = true;
		this->iterPrevSelectedObject = this->iterSelectedObject;

		//Draw box on edge of selected object, with or without focus.
		//(Drawing outside of object would require saving a separate surface
		//to erase the box, and we don't really need to do that.)
		const SURFACECOLOR SelectedObjectHighlight = (IsSelected() ?
				GetSurfaceColor(pSurface, 0, 0, 0) :
				GetSurfaceColor(pSurface, 64, 64, 64));
		DrawRect(SelectionScreenRect,SelectedObjectHighlight);
	}

	//Draw children in active menu.
	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//
//Protected methods.
//

//******************************************************************************
void CObjectMenuWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the widget.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
//
//Returns:
//True.
{
	const SDLKey key = KeyboardEvent.keysym.sym;

	if (this->iterSelectedObject == this->lObjects.end())
		return;

	switch (key) {
		//Move between objects.
		case SDLK_UP:
		case SDLK_KP8:

		case SDLK_LEFT:
		case SDLK_KP4:
			if (this->iterSelectedObject != this->lObjects.begin())
			{
				SetSelectedObject(--this->iterSelectedObject);
				Paint();
			}
		break;

		case SDLK_DOWN:
		case SDLK_KP2:

		case SDLK_RIGHT:
		case SDLK_KP6:
			if (&(*this->iterSelectedObject) != &this->lObjects.back())
			{
				SetSelectedObject(++this->iterSelectedObject);
				Paint();
			}
		break;

		default: break;
   }
}

//******************************************************************************
void CObjectMenuWidget::HandleMouseDown(
//Handles a mouse down event.
//All this should do is select a tag.
//
//Params:
	const SDL_MouseButtonEvent &Button)	//(in)	Event to handle.
{
	MENUOBJECT *pObject;

	//Return id# of object clicked.
	UINT wX, wY;
	for (OBJECT_ITERATOR iSeek = this->lObjects.begin(); 
			iSeek != this->lObjects.end(); ++iSeek)
	{
		pObject = (*iSeek);
		wX = this->x + pObject->nX;
		wY = this->y + pObject->nY;
		//If in bounds of this object, it was clicked.
		if (Button.x >= wX && Button.y >= wY &&
				Button.x < wX + pObject->wXSize*CBitmapManager::CX_TILE &&
				Button.y < wY + pObject->wYSize*CBitmapManager::CY_TILE)
		{
			SetSelectedObject(iSeek);
			Paint();
		}
	}

	//No object was clicked on.  Do nothing.
}

//******************************************************************************
bool CObjectMenuWidget::Load(void)
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);

	//Create surface to save screen surface bits where selection is drawn.
	this->pEraseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			this->GetW(), this->GetH(), 24, 0, 0, 0, 0);	//max possible size
	if (!this->pEraseSurface) return false;

	//Success.
	this->bIsLoaded = true;
	return true;
}

//******************************************************************************
void CObjectMenuWidget::Unload(void)
//Unload resources for widget.
{
	if (this->pEraseSurface)
	{
		SDL_FreeSurface(this->pEraseSurface);
		this->pEraseSurface = NULL;
	}

	this->bIsLoaded = false;
}

//
//Private methods.
//

//******************************************************************************
void CObjectMenuWidget::SetSelectedObject(
//Sets the selected object on the menu.  Updates the view.
//
//Params:
	OBJECT_ITERATOR const &newSelectedObject)	//(in) an object in this->lObjects
{
	ASSERT(*newSelectedObject);
	this->iterSelectedObject = newSelectedObject;

	//Call OnSelectChange() notifier.
	CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
	if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());
}

// $Log: ObjectMenuWidget.cpp,v $
// Revision 1.5  2003/09/16 16:03:16  schik
// Fixed memory leak
//
// Revision 1.4  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/06/16 18:47:00  mrimer
// Fixed a display bug.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.2  2002/11/22 02:16:59  mrimer
// Made selection border look better.
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
