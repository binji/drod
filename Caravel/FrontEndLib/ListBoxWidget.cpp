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

#include "ListBoxWidget.h"
#include "EventHandlerWidget.h"
#include "BitmapManager.h"
#include "FontManager.h"
#include "Inset.h"
#include <BackEndLib/Types.h>
#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

//Dimensions of areas drawn for widget.
static const UINT CX_UP = 12;
static const UINT CY_UP = 12;
static const UINT CX_BACK = 12;
static const UINT CY_BACK = 22;
static const UINT CX_POS = 12;
static const UINT CY_POS = 12;
static const UINT CX_DOWN = 12;
static const UINT CY_DOWN = 12;

//
//Public methods.
//

//******************************************************************************
CListBoxWidget::CListBoxWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,					//(in)	Required params for CWidget
	const int nSetX, const int nSetY,	//		constructor.
	const UINT wSetW, const UINT wSetH,	//
   const bool bSortAlphabetically)     //[default=false]
	: CFocusWidget(WT_ListBox, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, wTopLineNo(0), wSelectedLineNo(0)
	, wDisplayLineCount(0)
	, eLastClickResult(LBCR_Nothing)
	, bIsUpButtonPressed(false)
	, bIsDownButtonPressed(false)
	, nPosClickY(0)
   , wPosClickTopLineNo(0)
	, bClickedSelection(false)
   , bSortAlphabetically(bSortAlphabetically)
{
	CLEAR_RECT(this->PosRect);
	CLEAR_RECT(this->UpRect);
	CLEAR_RECT(this->DownRect);
	CLEAR_RECT(this->ItemsRect);
	CLEAR_RECT(this->ScrollRect);

	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::AddItem(
//Add an item to end of list box items.
//
//Params:
	const DWORD dwSetKey,				//(in)	Arbitrary key associated with
								//		text.  Should be unique to all other keys.
	const WCHAR *pwczSetText)	//(in)	Text to display on screen.
{
	//Create new item.
	LBOX_ITEM *pNew = new LBOX_ITEM;
	pNew->dwKey = dwSetKey;
	pNew->pwczText = new WCHAR[WCSlen(pwczSetText) + 1];
	if (!pNew->pwczText) 
	{
		ASSERTP(false,"Bad pwczSetText or alloc failed.");
		return;
	}
	WCScpy(pNew->pwczText, pwczSetText);

	//Add it to list.
   if (this->bSortAlphabetically)
   {
      //Insert alphabetically.
      list<LBOX_ITEM *>::iterator iter = this->Items.begin();
      while (iter != this->Items.end() && WCSicmp((*iter)->pwczText,pNew->pwczText) < 0)
         ++iter;
      this->Items.insert(iter, pNew);
   }
   else
	   this->Items.push_back(pNew);

	//Recalc areas of widget since they may have changed.
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::RemoveItem(
//Remove an existing item from the list box items.
//
//Params:
	const DWORD dwKey)	//(in)	Unique key indicating which item to remove.
{
	//Find the line corresponding to the key.
	UINT wSeekLineNo = 0;
	for(list<LBOX_ITEM *>::const_iterator iSeek = this->Items.begin();
		iSeek != this->Items.end(); ++iSeek)
	{
		if ((*iSeek)->dwKey == dwKey) //Found item.
		{
			//Remove from list.
			this->Items.remove(*iSeek);
			if (wSeekLineNo <= this->wSelectedLineNo)
			{
				if (this->wSelectedLineNo > 0) --this->wSelectedLineNo;
			}
			break;
		}
		++wSeekLineNo;
	}

	//Recalc areas of widget since they may have changed.
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::SelectLine(
//Selects a line in the list box.
//
//Params:
	const UINT wLineNo)	//(in)	Line to select.
{
	ASSERT(wLineNo == 0 || wLineNo < this->Items.size());

	this->wSelectedLineNo = wLineNo;

	//If selected line is not in current page, scroll the list box to show it.
	if (this->wSelectedLineNo < this->wTopLineNo)
		this->wTopLineNo = this->wSelectedLineNo;
	else if (this->wTopLineNo + this->wDisplayLineCount <= this->wSelectedLineNo)
		this->wTopLineNo = this->wSelectedLineNo - this->wDisplayLineCount + 1;

	//Recalc areas of widget since they may have changed.
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::SelectItem(
//Selects an item in the list box.
//
//Params:
	const DWORD dwKey)	//(in)	Indicates which item to select.
{
	//Find the line corresponding to the key.
	UINT wSeekLineNo = 0;
	for(list<LBOX_ITEM *>::const_iterator iSeek = this->Items.begin();
		iSeek != this->Items.end(); ++iSeek)
	{
		if ((*iSeek)->dwKey == dwKey) //Found item.
		{
			SelectLine(wSeekLineNo);
			return;
		}
		++wSeekLineNo;
	}
}

//******************************************************************************
void CListBoxWidget::DragPosButton(
//Updates position button and list box position based on Y coordinate from mouse
//dragging after a click on the position button.
//
//Param:
	const int nY)	//(in)	Vertical mouse coord.
{
	//Shouldn't be showing the pos button if less than two pages of items.
	ASSERT(Items.size() > this->wDisplayLineCount);

	//Figure difference in lines from click coord to current coord.
	const int nMoveY = nY - this->nPosClickY;
	const double dblLinesToPixel = (double) Items.size() / (double) this->ScrollRect.h;
	const int nMoveLines = (int)((double)(nMoveY) * dblLinesToPixel);
	if (nMoveLines == 0) return;

	//Move the top line to new location.
	if ((int) (this->wPosClickTopLineNo + nMoveLines + this->wDisplayLineCount) > 
			(int)(this->Items.size()))
		this->wTopLineNo = this->Items.size() - this->wDisplayLineCount;
	else if ((int) this->wPosClickTopLineNo + nMoveLines < 0)
		this->wTopLineNo = 0;
	else
		this->wTopLineNo = this->wPosClickTopLineNo + nMoveLines;

	//Recalc areas of widget since they may have changed.
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::ScrollDownOneLine()
//Scroll list box down one line.
{
	if (this->wTopLineNo + this->wDisplayLineCount < this->Items.size())
		++this->wTopLineNo;

	//Recalc areas of widget since they may have changed.
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::ScrollDownOnePage()
//Scroll list box down one page.
{
	if (this->wTopLineNo + (this->wDisplayLineCount * 2) < this->Items.size())
		this->wTopLineNo += this->wDisplayLineCount;
	else
		this->wTopLineNo = this->Items.size() - this->wDisplayLineCount;
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::ScrollUpOnePage()
//Scroll list box up one page.
{
	if (this->wTopLineNo >= this->wDisplayLineCount)
		this->wTopLineNo -= this->wDisplayLineCount;
	else
		this->wTopLineNo = 0;
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::ScrollUpOneLine()
//Scroll list box up one line.
{
	if (this->wTopLineNo > 0)
		--this->wTopLineNo;

	//Recalc areas of widget since they may have changed.
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::Clear()
//Clear the list box so that it contains no items.
{
	for (list<LBOX_ITEM *>::const_iterator iSeek = this->Items.begin();
			iSeek != this->Items.end(); ++iSeek)
	{
		if ((*iSeek)->pwczText) delete[] (*iSeek)->pwczText;
		delete *iSeek;
	}
	this->Items.clear();

   this->wSelectedLineNo = 0;

	//Recalc areas of widget since they may have changed.
	CalcAreas();
}

//******************************************************************************
void CListBoxWidget::Paint(
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

	//Coords withing parts surface.
	static const int X_UP = 154;
	static const int Y_UP = 15;
	static const int X_UP_D = 154;
	static const int Y_UP_D = 27;
	static const int X_DOWN = 166;
	static const int Y_DOWN = 15;
	static const int X_DOWN_D = 166;
	static const int Y_DOWN_D = 27;
	static const int X_POS = 178;
	static const int Y_POS = 22;
	static const int X_BACK = 178;
	static const int Y_BACK = 0;

	static const UINT CX_ITEM_INDENT = 2;
	
	//Draw inset area where text appears.
	SDL_Surface *pDestSurface = GetDestSurface();
	DrawInset(this->x, this->y, this->w, this->h, this->pPartsSurface, 
			pDestSurface);

	//Draw scroll bar if needed.
	const bool bDrawScrollBar = (this->Items.size() > this->wDisplayLineCount);
	if (bDrawScrollBar)
	{
		//Draw up button.
		SDL_Rect src = { (this->bIsUpButtonPressed) ? X_UP_D : X_UP, 
			(this->bIsUpButtonPressed) ? Y_UP_D : Y_UP, CX_UP, CY_UP};
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &this->UpRect);

		//Draw down button.
		src.x = (this->bIsDownButtonPressed) ? X_DOWN_D : X_DOWN;
		src.y = (this->bIsDownButtonPressed) ? Y_DOWN_D : Y_DOWN;
		src.w = CX_DOWN;
		src.h = CY_DOWN;
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &this->DownRect);

		//Draw scroll bar background between the two buttons.
		SDL_Rect dest = {this->ScrollRect.x, this->ScrollRect.y, CX_BACK, CY_BACK};
		src.x = X_BACK;
		src.y = Y_BACK;
		src.w = CX_BACK;
		src.h = dest.h = CY_BACK;		
		for (; dest.y < this->DownRect.y; dest.y += CY_BACK)
		{
			if (dest.y + CY_BACK > static_cast<UINT>(this->DownRect.y))
            dest.h = src.h = this->DownRect.y - dest.y; //Clip the blit to remaining height.
			SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
		}

		//Draw position button if it will fit in space between up and down buttons.
		if (this->ScrollRect.h >= CY_POS)
		{
			src.x = X_POS;
			src.y = Y_POS;
			src.w = CX_POS;

			//Draw position button.
			if (this->PosRect.h == CY_POS) //Draw position button at its smallest height.
			{
				src.h = CY_POS;
				SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &(this->PosRect));
			}
			else	//Draw stretched position button.
			{
				const UINT CY_TOP_BEVEL = 3;
				const UINT CY_BOTTOM_BEVEL = 3;
				const UINT CY_CENTER = 6;

				//Draw top bevel.
				dest.x = this->PosRect.x;
				dest.y = this->PosRect.y;
				dest.w = this->PosRect.w;
				src.h = dest.h = CY_TOP_BEVEL;
				SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);

				//Draw stretched center of position button.
				src.y += CY_TOP_BEVEL;
				src.h = dest.h = CY_CENTER;
				int yBottomBevel = this->PosRect.y + this->PosRect.h - CY_BOTTOM_BEVEL;
				for (dest.y += CY_TOP_BEVEL; dest.y < yBottomBevel; 
						dest.y += CY_CENTER)
				{
					if (dest.y + CY_CENTER >= (UINT) yBottomBevel)
						//Clip the blit to remaining height.
						dest.h = src.h = yBottomBevel - dest.y; 
					SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
				}

				//Draw bottom bevel.
				src.h = dest.h = CY_BOTTOM_BEVEL;
				src.y += CY_CENTER;
				dest.y = yBottomBevel;
				SDL_BlitSurface(pPartsSurface, &src, pDestSurface, &dest);
			}
		}
	}

	//If no items, then I'm done drawing.
	if (this->Items.size() == 0) return;
	
	//Find top item.
	list<LBOX_ITEM *>::const_iterator iListItem = this->Items.begin();
	for (UINT wI = 0; wI < this->wTopLineNo; ++wI) ++iListItem;

	//Draw item text.
	const UINT wStopLineNo = this->wTopLineNo + wDisplayLineCount;
	const int xDraw = this->x + CX_INSET_BORDER + CX_ITEM_INDENT;
	int yDraw = this->ItemsRect.y = this->y + CY_INSET_BORDER;
	const UINT cxDraw = this->w - (CX_INSET_BORDER * 2) - CX_ITEM_INDENT -
			(bDrawScrollBar * CX_UP);
	for (UINT wLineNo = this->wTopLineNo;
			wLineNo < wStopLineNo && iListItem != this->Items.end();
			++wLineNo, ++iListItem)
	{
		ASSERT(yDraw < static_cast<int>(this->y + this->h - CY_INSET_BORDER));

		//If this is selected item, draw solid rect underneath.
		if (wLineNo == this->wSelectedLineNo)
		{
			const SURFACECOLOR BackColor =
					GetSurfaceColor(pDestSurface, 190, 181, 165);
			SDL_Rect ItemRect = {xDraw - CX_ITEM_INDENT, yDraw, 
					cxDraw + CX_ITEM_INDENT, CY_LBOX_ITEM};
			DrawFilledRect(ItemRect, BackColor);

			//Draw focus box.
			if (IsSelected())
			{
				const SURFACECOLOR FocusColor =
						GetSurfaceColor(GetDestSurface(), RGB_FOCUS);
				DrawRect(ItemRect,FocusColor);
			}
		}

		//Draw text for one item.
		const UINT eDrawFont = (wLineNo == this->wSelectedLineNo) ? 
				FONTLIB::F_SelectedListBoxItem : FONTLIB::F_ListBoxItem;
					//Draw selected line text.
		const int DY_TEXT = -2; //Because this particular font is always too far down.
		g_pTheFM->DrawTextXY(eDrawFont, (*iListItem)->pwczText, pDestSurface,
				xDraw, yDraw + DY_TEXT, cxDraw, CY_LBOX_ITEM);

		yDraw += CY_LBOX_ITEM;
	}

	PaintChildren();
	if (bUpdateRect) UpdateRect();
}

//******************************************************************************
DWORD CListBoxWidget::GetSelectedItem() const
//Returns key corresponding to selected item, or 0L if none.
{
	//Find selected item.
	list<LBOX_ITEM *>::const_iterator iListItem = this->Items.begin();
	for (UINT wI = 0; wI < this->wSelectedLineNo; ++wI) ++iListItem;

	return (iListItem == this->Items.end()) ? 0L : (*iListItem)->dwKey;
}

//******************************************************************************
const WCHAR* CListBoxWidget::GetSelectedItemText() const
//Returns text corresponding to selected item, or "" if none.
{
	//Find selected item.
	list<LBOX_ITEM *>::const_iterator iListItem = this->Items.begin();
	for (UINT wI = 0; wI < this->wSelectedLineNo; ++wI) ++iListItem;

	return (iListItem == this->Items.end()) ? wszEmpty : (*iListItem)->pwczText;
}

//******************************************************************************
void CListBoxWidget::SetSelectedItemText(const WCHAR *pwczSetText)
{
	//Find selected item.
	list<LBOX_ITEM *>::const_iterator iListItem = this->Items.begin();
	for (UINT wI = 0; wI < this->wSelectedLineNo; ++wI) ++iListItem;
   if (iListItem == this->Items.end()) return;

	(*iListItem)->pwczText = new WCHAR[WCSlen(pwczSetText) + 1];
	if (!(*iListItem)->pwczText) 
	{
		ASSERTP(false,"Bad pwczSetText or alloc failed.(2)");
		return;
	}
	WCScpy((*iListItem)->pwczText, pwczSetText);
}

//******************************************************************************
LBOX_CLICK_RESULT CListBoxWidget::ClickAtCoords(
//Updates list box in response to a mouse click at specified coords.
//
//Params:
	const int nX, const int nY)	//(in) Click coords.
//
//Returns:
//An LBCR_* constant indicating what happened.
{
	this->bClickedSelection = false;

	//Check for click on position button.
	if (IS_IN_RECT(nX, nY, this->PosRect))
	{
		this->wPosClickTopLineNo = this->wTopLineNo;
		this->nPosClickY = nY;
		return LBCR_PosButton;
	}

	//Check for click inside scroll area, but not on position button since I just
	//checked that.
	if (IS_IN_RECT(nX, nY, this->ScrollRect))
	{
		if (nY < this->PosRect.y)
		{
			ScrollUpOnePage();
			return LBCR_PageUp;
		}
		else
		{
			ScrollDownOnePage();
			return LBCR_PageDown;
		}
	}

	//Check for click inside scroll bar up button.
	if (IS_IN_RECT(nX, nY, this->UpRect))
	{
		ScrollUpOneLine();
		return LBCR_UpButton;
	}

	//Check for click inside scroll bar down button.
	if (IS_IN_RECT(nX, nY, this->DownRect))
	{
		ScrollDownOneLine();
		return LBCR_DownButton;
	}

	//Check for click inside list items.
	if (IS_IN_RECT(nX, nY, this->ItemsRect))
	{
	   //One of the list items was clicked.
      if (GetItemCount() > 0)
	      this->bClickedSelection = true;

		const UINT wPrevSelectedLineNo = this->wSelectedLineNo;
		this->wSelectedLineNo = this->wTopLineNo + 
				((nY - this->ItemsRect.y) / CY_LBOX_ITEM);
		if (this->wSelectedLineNo >= this->Items.size())
			this->wSelectedLineNo = this->Items.size() - 1;

		if (this->wSelectedLineNo != wPrevSelectedLineNo)
		{
			CalcAreas();
			return LBCR_NewSelection;
		}
	}

	//Click did not cause an update of the widget.
	return LBCR_Nothing;
}

//*****************************************************************************
bool CListBoxWidget::ClickedSelection() const
//Returns: whether any list item was clicked on
{
	return this->bClickedSelection;
}

//*****************************************************************************
void CListBoxWidget::Move(
//Move widget, and recalc clicking areas.
//
//Params:
	const int nSetX, const int nSetY)	//(in)
{
	CWidget::Move(nSetX, nSetY);

	CalcAreas();
}
	 
//
//Protected methods.
//

//******************************************************************************
void CListBoxWidget::HandleDrag(
//Handles a mouse motion event while the button is down.
//
//Params:
	const SDL_MouseMotionEvent &Motion)
{
	if (this->eLastClickResult == LBCR_PosButton)
	{
		DragPosButton(Motion.y);
		Paint();
	}
}

//******************************************************************************
void CListBoxWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the widget.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
//
//Returns:
//True.
{
	UINT wPrevSelectedLine = this->wSelectedLineNo;
	UINT wNewSelectedLine = wPrevSelectedLine;

	switch (KeyboardEvent.keysym.sym) {
		case SDLK_UP: case SDLK_KP8:
			if (wNewSelectedLine > 0) --wNewSelectedLine;
		break;

		case SDLK_DOWN: case SDLK_KP2:
			if (wNewSelectedLine < this->Items.size() - 1) ++wNewSelectedLine;
		break;

		case SDLK_HOME:
			wNewSelectedLine = 0;
		break;
			
		case SDLK_END:
			wNewSelectedLine = this->Items.size() - 1;
		break;

		case SDLK_PAGEUP:
			if ((int) (wNewSelectedLine) - int(this->wDisplayLineCount) > 0)
				wNewSelectedLine -= this->wDisplayLineCount;
			else
				wNewSelectedLine = 0;
		break;

		case SDLK_PAGEDOWN:
			if ((int) (wNewSelectedLine) + int(this->wDisplayLineCount) <
						(int)this->Items.size() - 1)
				wNewSelectedLine += this->wDisplayLineCount;
			else
				wNewSelectedLine = this->Items.size() - 1;
		break;

		default:
		return;
	}

	if (wPrevSelectedLine != wNewSelectedLine)
	{
		//Paint a newly selected line.
		SelectLine(wNewSelectedLine);
		Paint();

		//Call OnSelectChange() notifier.
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());
	}
}

//******************************************************************************
void CListBoxWidget::HandleMouseDown(
//Handles a mouse down event.
//
//Params:
	const SDL_MouseButtonEvent &MouseButtonEvent)	//(in) Event to handle.
{
	if (this->eLastClickResult != LBCR_Nothing)
   {
      //Can happen if a selection change was made, causing a dialog box to
      //pop up before the mouse up event is received.
      this->bIsDownButtonPressed = false;
      this->bIsUpButtonPressed = false;
   }

	this->eLastClickResult = ClickAtCoords(MouseButtonEvent.x, MouseButtonEvent.y);
	switch (this->eLastClickResult)
	{
		case LBCR_Nothing: 
		return; //No changes to paint.

		case LBCR_UpButton:
			this->bIsUpButtonPressed = true;
		break;

		case LBCR_DownButton:
			this->bIsDownButtonPressed = true;
		break;

		default:
		break;
	}
	Paint();

	if (this->eLastClickResult == LBCR_NewSelection)
	{
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());
	}
}

//******************************************************************************
void CListBoxWidget::HandleMouseDownRepeat(
//Handles a mouse down repeat event.  This is caused when the user holds the mouse
//button down for a period of time.
//
//Params:
	const SDL_MouseButtonEvent &Button)	//(in) Mouse down event to handle.
{
	//Just repeat the results of last click result if coords are still in
	//correct area.
	switch (this->eLastClickResult)
	{
		case LBCR_UpButton:
			ASSERT(this->bIsUpButtonPressed);
			if (IS_IN_RECT(Button.x, Button.y, this->UpRect))
				ScrollUpOneLine();
		break;

		case LBCR_DownButton:
			ASSERT(this->bIsDownButtonPressed);
			if (IS_IN_RECT(Button.x, Button.y, this->DownRect))
				ScrollDownOneLine();
		break;

		case LBCR_PageDown:
		case LBCR_PageUp:
			if (IS_IN_RECT(Button.x, Button.y, this->ScrollRect) &&
					!IS_IN_RECT(Button.x, Button.y, this->PosRect))
			{
				if (Button.y < this->PosRect.y)
					ScrollUpOnePage();
				else
					ScrollDownOnePage();
			}
		break;
		
		default://Nothing happened.
		return; 
	}
	Paint();
}

//******************************************************************************
void CListBoxWidget::HandleMouseUp(
//Handles a mouse up event.
//
//Params:
	const SDL_MouseButtonEvent &/*Button*/)	//(in) Event to handle.
{
	ASSERT(!(this->bIsUpButtonPressed && this->bIsDownButtonPressed));

	//Unpress anything that is pressed.
	if (this->bIsUpButtonPressed)
	{
		ASSERT(this->eLastClickResult == LBCR_UpButton);
		this->bIsUpButtonPressed = false;
	}
	else if (this->bIsDownButtonPressed)
	{
		ASSERT(this->eLastClickResult == LBCR_DownButton);
		this->bIsDownButtonPressed = false;
	}
	Paint();

	//Reset click result.
	this->eLastClickResult = LBCR_Nothing;
}

//******************************************************************************
void CListBoxWidget::HandleMouseWheel(
//Handles a mouse down event.
//
//Params:
	const SDL_MouseButtonEvent &MouseButtonEvent)	//(in) Event to handle.
{
	if (MouseButtonEvent.button == SDL_BUTTON_WHEELUP)
		ScrollUpOneLine();
	else if (MouseButtonEvent.button == SDL_BUTTON_WHEELDOWN)
		ScrollDownOneLine();

	Paint();

	if (this->eLastClickResult == LBCR_NewSelection)
	{
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());
	}
}

//
//Private methods.
//

//*****************************************************************************
void CListBoxWidget::CalcAreas()
//Calculate coords and dimensions of areas within list box.
{
	ASSERT(this->h > CY_INSET_BORDER + CY_UP + CY_DOWN + CY_INSET_BORDER);	
	ASSERT(this->w > CX_INSET_BORDER + CX_UP + CX_INSET_BORDER);

	//Figure out if I need to draw a scroll bar.
	this->wDisplayLineCount = (this->h - (CY_INSET_BORDER * 2)) / CY_LBOX_ITEM;
	bool bDrawScrollBar = (this->Items.size() > this->wDisplayLineCount);
	if (!bDrawScrollBar) this->wTopLineNo = 0;
	
	//Calc scroll bar if needed.
	if (!bDrawScrollBar)
	{
		CLEAR_RECT(this->UpRect);
		CLEAR_RECT(this->DownRect);
	}
	else
	{
		//Calc up button.
		SET_RECT(this->UpRect, this->x + this->w - CX_UP - CX_INSET_BORDER, 
				this->y + CY_INSET_BORDER, CX_UP, CY_UP);
		
		//Calc down button.
		SET_RECT(this->DownRect, this->x + this->w - CX_UP - CX_INSET_BORDER, 
				this->y + this->h - CY_INSET_BORDER - CY_DOWN, CX_UP, CY_UP);
		
		//Calc scroll bar background between the two buttons.
		this->ScrollRect.x = this->DownRect.x;
		this->ScrollRect.y = this->y + CY_INSET_BORDER + CY_UP;
		this->ScrollRect.w = CX_BACK;
		this->ScrollRect.h = this->DownRect.y - this->y - CY_INSET_BORDER - CY_UP;
				
		//Will position button fit in space between up and down buttons?
		if (this->ScrollRect.h < CY_POS)	//No.
			CLEAR_RECT(this->PosRect);
		else							//Yes.
		{
			//Figure percentage position button is from top of list box.
			double dblPosPercent = (double) this->wTopLineNo / 
					(double) (this->Items.size() - wDisplayLineCount);

			//Calc position button.
			this->PosRect.h = (Uint16)(this->ScrollRect.h * ((double) this->wDisplayLineCount / 
					(double) this->Items.size()));
			if (this->PosRect.h < CY_POS) this->PosRect.h = CY_POS;
			this->PosRect.w = CX_POS;
			this->PosRect.x = this->ScrollRect.x;
			this->PosRect.y = this->y + CY_INSET_BORDER + CY_UP + 
					(Sint16)(dblPosPercent * (this->ScrollRect.h - this->PosRect.h));
		}
	}

	//If no items, then no items rect.
	if (this->Items.size() == 0) 
	{
		CLEAR_RECT(this->ItemsRect);
	}
	else
	{
		//Calc item rect.
		this->ItemsRect.x = this->x + CX_INSET_BORDER;
		this->ItemsRect.y = this->y + CY_INSET_BORDER;
		this->ItemsRect.w = this->w - (CX_INSET_BORDER * 2) - (bDrawScrollBar ?
				CX_UP : 0);
		this->ItemsRect.h = this->wDisplayLineCount * CY_LBOX_ITEM;
	}
}

// $Log: ListBoxWidget.cpp,v $
// Revision 1.14  2004/05/20 17:37:41  mrimer
// Tweaking.
//
// Revision 1.13  2004/01/02 01:09:59  mrimer
// Added optional alphabetical sorting of list items.
//
// Revision 1.12  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.11  2003/09/11 02:03:39  mrimer
// Fixed a bug.
//
// Revision 1.10  2003/08/29 16:50:31  mrimer
// Fixed bug: Click on empty list registers a selection made
//
// Revision 1.9  2003/08/22 04:19:29  mrimer
// Moved call to Clear() from Unload() to the (new) destructor.
//
// Revision 1.8  2003/08/05 17:02:06  mrimer
// Added SetSelectedItemText().
//
// Revision 1.7  2003/07/21 22:01:12  mrimer
// Changed an assertion to code handling the eventuality.
//
// Revision 1.6  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.5  2003/07/02 01:05:59  mrimer
// Changed widget to render only a single line of text for each item.
//
// Revision 1.4  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/06/09 23:51:48  mrimer
// Made some const method vars static.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.27  2003/05/19 20:40:30  erikh2000
// Fixes for warnings.
//
// Revision 1.26  2003/04/17 21:06:13  mrimer
// Added HandleMouseWheel().
//
// Revision 1.25  2003/04/17 18:55:38  schik
// Mouse Scroll Wheel now works as expected in the listbox.
//
// Revision 1.24  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.23  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.22  2003/01/08 00:53:43  mrimer
// Added ClickedSelection().
//
// Revision 1.21  2002/12/22 02:22:12  mrimer
// Added GetSelectedItemText().
//
// Revision 1.20  2002/11/15 03:08:58  mrimer
// Fixed some minor bugs.
// Moved member initialization in constructor's initialization list.
// Made some vars and parameters const.
//
// Revision 1.19  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.18  2002/07/20 23:10:47  erikh2000
// Revised the keydown handler to support other keys and paint more quickly.
//
// Revision 1.17  2002/07/19 20:38:37  mrimer
// Revised HandleKeyDown() to return no type.
//
// Revision 1.16  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.15  2002/06/21 18:32:31  mrimer
// Fixed list box event behavior.
//
// Revision 1.14  2002/06/16 06:24:52  erikh2000
// Added method for removing list box items.
//
// Revision 1.13  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.12  2002/06/14 00:52:47  erikh2000
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
//
// Revision 1.11  2002/06/11 22:55:52  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.10  2002/06/09 06:42:28  erikh2000
// Changed a param to const because its use is read-only.
//
// Revision 1.9  2002/06/07 17:52:23  mrimer
// Fixed mouse click handling.
//
// Revision 1.8  2002/06/06 00:02:20  mrimer
// Finished keyboard handling.  Added focus graphic.
//
// Revision 1.7  2002/06/05 03:17:42  mrimer
// Added widget focusability and keyboard control.
// Added focus graphic.
//
// Revision 1.6  2002/05/21 21:35:29  erikh2000
// Changed color-setting code.
//
// Revision 1.5  2002/05/15 01:26:46  erikh2000
// Added new methods to select a line and get number if items in list box.
//
// Revision 1.4  2002/05/12 03:16:49  erikh2000
// Made a method for returning selected item handle situation where there are no items.
//
// Revision 1.3  2002/05/10 22:36:41  erikh2000
// Removed widget area calculation code from paint code and put it in a separate routine.
//
// Revision 1.2  2002/04/29 00:13:14  erikh2000
// Implemented the class.
//
// Revision 1.1  2002/04/25 09:28:28  erikh2000
// Initial check-in.
//
