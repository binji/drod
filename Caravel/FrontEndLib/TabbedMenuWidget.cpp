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

#include "TabbedMenuWidget.h"
#include "BitmapManager.h"
#include "EventHandlerWidget.h"
#include "Sound.h"

//Standard size of tabs on menu.
const UINT CX_STANDARD_MENUTAB = 31;
const UINT CY_STANDARD_MENUTAB = 24;

//Background menu color.
const Uint32 BGColor = 0xff << 16 | 0xfa << 8 | 0xcd;	//pale yellow

const SURFACECOLOR Black = {0, 0, 0};

//
//Public methods.
//

//******************************************************************************
CTabbedMenuWidget::CTabbedMenuWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	const int nSetX, const int nSetY,		//		constructor.
	const UINT wSetW, const UINT wSetH,		//
	const UINT wTabs)								//(in)	Number of tabs.
	: CFocusWidget(WT_TabbedMenu, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, wNumTabs(wTabs)
	, wSelectedTab(0)
{
	ASSERT(wTabs > 0);

	this->pwTabTileNo = new UINT[wTabs];
	this->plChildWidgetsForTab = new list<CWidget *>[wTabs];
}

//******************************************************************************
CTabbedMenuWidget::~CTabbedMenuWidget()
//Destructor.
{
	delete[] this->pwTabTileNo;
	delete[] this->plChildWidgetsForTab;
}

//******************************************************************************
void CTabbedMenuWidget::SetTabTile(
//Sets the tile to be displayed on the specified tab.
//
//Params:
	const UINT wTab, const UINT wTileNo)
{
	ASSERT(wTab < this->wNumTabs);
	this->pwTabTileNo[wTab] = wTileNo;
}

//******************************************************************************
CWidget*	CTabbedMenuWidget::AddWidgetToTab(
//Adds a widget to menu corresponding to certain tag
//as well as to list of children.
//
//Params:
	CWidget *pNewWidget,
	const UINT wTab,	//(in) to which tab
	bool bLoad)	//(in) default = false
{
	ASSERT(wTab < this->wNumTabs);
	this->plChildWidgetsForTab[wTab].push_back(pNewWidget);
	if (wTab == this->wSelectedTab)
		pNewWidget->Show();
	else
		pNewWidget->Hide();

	return CWidget::AddWidget(pNewWidget,bLoad);
}

//******************************************************************************
void CTabbedMenuWidget::Paint(
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

	ASSERT(this->w >= CX_STANDARD_MENUTAB && this->h >= CY_STANDARD_MENUTAB);

	SDL_Surface *pSurface = GetDestSurface();

	//Draw menu area.
	//Fill menu surface.
	SDL_Rect rect = {this->x, this->y + CY_STANDARD_MENUTAB, this->w,
			this->h - CY_STANDARD_MENUTAB};
	SDL_FillRect(pSurface,&rect,BGColor);
	DrawRect(rect,Black);

	//Draw tabs.
	DrawTabs();

	//Draw with or without focus.
	if (this->IsSelected())
		DrawFocused();

	//Draw children in active menu.
	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//
//Protected methods.
//

//******************************************************************************
void CTabbedMenuWidget::HandleKeyDown(
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
		//Move between tabs.
		case SDLK_LEFT:
		case SDLK_KP4:
		{
			if (this->wSelectedTab > 0)
				SelectTab(this->wSelectedTab-1);
		}
		return;
		case SDLK_RIGHT:
		case SDLK_KP6:
		{
			if (this->wSelectedTab < this->wNumTabs - 1)
				SelectTab(this->wSelectedTab+1);
		}
		return;

      default: break;
	}
}

//******************************************************************************
void CTabbedMenuWidget::HandleMouseDown(
//Handles a mouse down event.
//All this should do is select a tag.
//
//Params:
	const SDL_MouseButtonEvent &Button)	//(in)	Event to handle.
{
	//Select tab.
	if (Button.y <= this->y + CY_STANDARD_MENUTAB)
	{
		const UINT wTabClicked = (Button.x - this->x) / CX_STANDARD_MENUTAB;
		//Select tab if click occurred inside its region.
		if (wTabClicked < this->wNumTabs)
			SelectTab(wTabClicked);
	}
}

//
//Private methods.
//

//******************************************************************************
void CTabbedMenuWidget::SelectTab(
//Select a tab and make widgets associated with it visible.
//
//Params:
	const UINT wTab)
{
	ASSERT(wTab < this->wNumTabs);

	if (this->wSelectedTab == wTab) return;

	g_pTheSound->PlaySoundEffect(SOUNDLIB::SEID_READ);

	//Make widgets associated with old tab invisible.
   WIDGET_ITERATOR iSeek;
	for (iSeek = this->plChildWidgetsForTab[this->wSelectedTab].begin(); 
			iSeek != this->plChildWidgetsForTab[this->wSelectedTab].end(); ++iSeek)
	{
		(*iSeek)->Hide();
	}

	this->wSelectedTab = wTab;

	//Make widgets associated with new tab visible.
	for (iSeek = this->plChildWidgetsForTab[this->wSelectedTab].begin();
			iSeek != this->plChildWidgetsForTab[this->wSelectedTab].end(); ++iSeek)
	{
		(*iSeek)->Show();
	}

	//Call OnSelectChange() notifier.
	CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
	if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());

	Paint();
}

//******************************************************************************
void CTabbedMenuWidget::DrawTabs(void)
//Draw tabs on top.
{
	static const UINT wXTile = 9;	//tile position on tab
	static const UINT wYTile = 5;

	SDL_Surface *pSurface = GetDestSurface();
	const SURFACECOLOR SBGColor = GetSurfaceColor(pSurface, 0xff, 0xfa, 0xcd);

	for (UINT wTab=0; wTab<this->wNumTabs; wTab++)
	{
		//Fill tab surface.
		const int xTab = this->x + CX_STANDARD_MENUTAB * wTab;
		SDL_Rect rect = {xTab, this->y, CX_STANDARD_MENUTAB,
				CY_STANDARD_MENUTAB};
		SDL_FillRect(pSurface,&rect,BGColor);

		//Draw edge of tab.
		DrawRect(rect,Black);

		//Show selected tab.
		if (wTab == this->wSelectedTab)
		{
			DrawRow(xTab+1, this->y+CY_STANDARD_MENUTAB, CX_STANDARD_MENUTAB-2,
					SBGColor);
			DrawRow(xTab+1, this->y+CY_STANDARD_MENUTAB-1, CX_STANDARD_MENUTAB-2,
					SBGColor);
		}

		//Draw tile.
		g_pTheBM->BlitTileImage(this->pwTabTileNo[wTab],
				xTab + wXTile, this->y + wYTile, pSurface);
	}
}

//******************************************************************************
void CTabbedMenuWidget::DrawFocused(void)
//Draw focus.
{
	const SURFACECOLOR FocusColor = GetSurfaceColor(GetDestSurface(), RGB_FOCUS);
	static const UINT wXIndent = 7;	//# pixels in from widget edge
	static const UINT wYIndent = 3;
	static const UINT CX_FOCUS = CBitmapManager::CX_TILE + 4;
	static const UINT CY_FOCUS = CBitmapManager::CY_TILE + 4;
	static const UINT DRAWY = this->y + wYIndent;
	const UINT DRAWX = this->x + wXIndent +
			CX_STANDARD_MENUTAB * this->wSelectedTab;
	const SDL_Rect rect = {DRAWX, DRAWY, CX_FOCUS, CY_FOCUS};

	//Draw box around tile.
	DrawRect(rect,FocusColor);
}

// $Log: TabbedMenuWidget.cpp,v $
// Revision 1.2  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.1  2003/05/22 21:49:31  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
