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

#include "MapWidget.h"
#include "RoomWidget.h"
#include "DrodSound.h"
#include <FrontEndLib/EventHandlerWidget.h>

#include "../DRODLib/GameConstants.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbRooms.h"
#include "../DRODLib/DbLevels.h"
#include <BackEndLib/Types.h>
#include <BackEndLib/Assert.h>
#include <BackEndLib/Files.h>

//Map color indexes.
enum MapColor {
	MAP_BLACK,
	MAP_WHITE,
	MAP_LTYELLOW,
	MAP_RED,
	MAP_LTGREEN,
	MAP_LTCYAN,
	MAP_DKCYAN,
	MAP_BLUE,
	MAP_LTBLUE,
	MAP_MAGENTA,
	MAP_LTGREY,
	MAP_DKCYAN2,
	MAP_GRAY,

	MAP_COLOR_COUNT
};

static SURFACECOLOR m_arrColor[MAP_COLOR_COUNT];

//
//Public methods.
//

//*****************************************************************************
CMapWidget::CMapWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	const CCurrentGame *pSetCurrentGame)	//(in) Game to use for drawing the map.
	: CFocusWidget(WT_Map, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
   , bVacantRoom(false)
	, dwLevelID(0L)
	, bIsLevelComplete(false)
	, pCurrentGame(pSetCurrentGame)
	, pLevel(NULL)
	, pMapSurface(NULL)
	, dwSelectedRoomX(0L), dwSelectedRoomY(0L)
	, dwLeftRoomX(0L), dwTopRoomY(0L), dwRightRoomX(0L), dwBottomRoomY(0L)
   , wLastSrcX(0), wLastSrcY(0), wBorderW(0), wBorderH(0)
	, bScrollHorizontal(false), bScrollVertical(false)
	, bEditing(false), bCopyingRoom(false)
	, bDeletingRoom(false)
	, pRoom(NULL)
{
	//Can't display a map smaller than one room.
	ASSERT(wSetW >= CDrodBitmapManager::DISPLAY_COLS);
	ASSERT(wSetH >= CDrodBitmapManager::DISPLAY_ROWS);
}

//*****************************************************************************
CMapWidget::~CMapWidget()
//Destructor.
{ 
	ASSERT(!this->bIsLoaded);
}

//*****************************************************************************
void CMapWidget::ClearMap()
//Reset the level so that the map draws nothing.
{
	this->pLevel = NULL;
	Paint();
}

//*****************************************************************************
void CMapWidget::GetSelectedRoomXY(DWORD &dwRoomX, DWORD &dwRoomY) const
{
	dwRoomX = this->dwSelectedRoomX;
	dwRoomY = this->dwSelectedRoomY;
}

//******************************************************************************
void CMapWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the widget.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)	//(in) Event to handle.
//
//Returns:
//True.
{
	const SDLKey key = KeyboardEvent.keysym.sym;
	DWORD dwRoomX = this->dwSelectedRoomX, dwRoomY = this->dwSelectedRoomY;

	switch (key) {
		//Moving selection to an adjacent room
		case SDLK_DOWN: case SDLK_KP2:
			if (dwRoomY < (this->bEditing ? this->dwBottomRoomY + 1 : this->dwBottomRoomY))
				++dwRoomY;
			break;
		case SDLK_LEFT: case SDLK_KP4:
			if (dwRoomX > (this->bEditing ? this->dwLeftRoomX - 1 : this->dwLeftRoomX))
				--dwRoomX;
			break;
		case SDLK_RIGHT: case SDLK_KP6:
			if (dwRoomX < (this->bEditing ? this->dwRightRoomX + 1 : this->dwRightRoomX))
				++dwRoomX;
			break;
		case SDLK_UP: case SDLK_KP8:
			if (dwRoomY > (this->bEditing ? this->dwRightRoomX - 1 : this->dwTopRoomY))
				--dwRoomY;
			break;

		//Cutting, copying and pasting selected room
		case SDLK_x:
		case SDLK_c:
			if (!this->bEditing || !this->pLevel) return;
			if (KeyboardEvent.keysym.mod & KMOD_CTRL)
			{
				CDbRoom *pCutRoom = this->pLevel->Rooms.GetByCoords(
						this->pLevel->dwLevelID, dwRoomX, dwRoomY);
				if (pCutRoom)
				{
					//A room exists here to cut/copy -- save it.
					delete this->pRoom;
					this->pRoom = pCutRoom;
					this->bCopyingRoom = (key == SDLK_c);	//Ctrl-C copies
					g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);
				}
			}
			return;
		case SDLK_v:
			this->bDeletingRoom = false;
			if (!this->bEditing || !this->pLevel || !this->pRoom) return;
			if (KeyboardEvent.keysym.mod & KMOD_CTRL)
			{
				if (this->pRoom->dwRoomX == dwRoomX && this->pRoom->dwRoomY == dwRoomY)
					return;	//putting room at same spot -- don't need to do anything

				this->bDeletingRoom = !this->bVacantRoom;
				//PasteRoom() called through owner screen.
			}
			return;

		default:
			return;
	}

	if (SelectRoomIfValid(dwRoomX,dwRoomY))
	{
		Paint();
		
		//Call OnSelectChange() notifier.
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());
	}
}

//******************************************************************************
void CMapWidget::HandleMouseDown(
//Processes a mouse event within the scope of the widget.
//
//Params:
	const SDL_MouseButtonEvent &MouseButtonEvent)	//(in) Event to handle.
{
	const DWORD dwPrevRoomX = this->dwSelectedRoomX,
			dwPrevRoomY = this->dwSelectedRoomY;

	SelectRoomAtCoords(MouseButtonEvent.x, MouseButtonEvent.y);
	Paint();

	if (dwPrevRoomX != this->dwSelectedRoomX || 
			dwPrevRoomY != this->dwSelectedRoomY)
	{
		//Call OnSelectChange() notifier.
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->OnSelectChange(GetTagNo());
	}
}

//*****************************************************************************
void CMapWidget::Resize(UINT wSetW, UINT wSetH)
//Override CWidget::Resize() to keep map surface set up correctly for new
//size.
{
	//Can't display a map smaller than one room.
	ASSERT(wSetW >= CDrodBitmapManager::DISPLAY_COLS);
	ASSERT(wSetH >= CDrodBitmapManager::DISPLAY_ROWS);
	
	this->w = wSetW;
	this->h = wSetH;
	ClipWHToDest();
	
	//Reload the map surface.
	if (this->pMapSurface) 
	{
		SDL_FreeSurface(this->pMapSurface);
		this->pMapSurface = NULL;
	}
	LoadMapSurface();
}

//*****************************************************************************
bool CMapWidget::LoadFromCurrentGame(
//Sets current game used by map.
//
//Params:
	const CCurrentGame *pSetCurrentGame)	//(in) New current game.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pSetCurrentGame);
	this->pCurrentGame = pSetCurrentGame;
	this->pLevel = this->pCurrentGame->pLevel;	//easy access
	this->bEditing = false;

	//Reload the map surface.
	if (this->pMapSurface)
	{
		SDL_FreeSurface(this->pMapSurface);
		this->pMapSurface = NULL;
	}
	if (!LoadMapSurface())
		return false;

	//Set vars that show current state of map compared to current game.
	this->dwLevelID = this->pLevel->dwLevelID;
	this->bIsLevelComplete = this->pCurrentGame->IsCurrentLevelComplete();
	this->ExploredRooms = this->pCurrentGame->ExploredRooms;
	this->ConqueredRooms = this->pCurrentGame->ConqueredRooms;

	//Select the current room.
	SelectRoom(this->pCurrentGame->pRoom->dwRoomX, this->pCurrentGame->pRoom->dwRoomY);

	return true;
}

//*****************************************************************************
bool CMapWidget::LoadFromLevel(
//Sets current game used by map.
//
//Params:
	CDbLevel *pLevel)	//(in) Level to load from.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pLevel);
	this->pLevel = pLevel;
	this->pCurrentGame = NULL;
	this->bEditing = true;

	//Reload the map surface.
	if (this->pMapSurface)
	{
		SDL_FreeSurface(this->pMapSurface);
		this->pMapSurface = NULL;
	}
	if (!LoadMapSurface()) 
		return false;

	//Set vars that show current state of map compared to current game.
	this->dwLevelID = pLevel->dwLevelID;
	this->bIsLevelComplete = false;	//show original state of level

	//Select the first room in the level.
	CDbRoom *pRoom = this->pLevel->Rooms.GetFirst();
	if (pRoom)
	{
		SelectRoom(pRoom->dwRoomX, pRoom->dwRoomY);
		delete pRoom;
	}

	//Can't cut room from a different level, only copy.
	if (this->pRoom)
		this->bCopyingRoom = true;

	return true;
}

//*****************************************************************************
void CMapWidget::UpdateFromCurrentGame()
//Update data so that it matches current game.
{
	ASSERT(this->pCurrentGame);
	ASSERT(this->pMapSurface);
	
	UpdateMapSurface();
	SelectRoom(this->pCurrentGame->pRoom->dwRoomX, this->pCurrentGame->pRoom->dwRoomY);
}

//*****************************************************************************
void CMapWidget::UpdateFromCurrentLevel()
//Update map to reflect the latest in-game level state.
{
   // No current game if using editor
   if (this->pCurrentGame)
      this->pLevel = this->pCurrentGame->pLevel;   //make sure level pointer is current
   LoadMapSurface();
}

//*****************************************************************************
void CMapWidget::Paint(
//Blits the map to the widget area on the screen surface.
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

	if (!this->pLevel)
	{
		DrawPlaceholder();
		return;
	}

	ASSERT(this->pMapSurface);

	//Figure part of map surface that will go in widget area.
	if (this->bScrollHorizontal)
	{
		const UINT wCenterX = this->wBorderW + (this->dwSelectedRoomX - this->dwLeftRoomX) *
				CDrodBitmapManager::DISPLAY_COLS + (CDrodBitmapManager::DISPLAY_COLS / 2);
		this->wLastSrcX = wCenterX - (this->w / 2);
	}
	else
		this->wLastSrcX = 0;
	if (this->bScrollVertical)
	{
		const UINT wCenterY = this->wBorderH + (this->dwSelectedRoomY - this->dwTopRoomY) * 
				CDrodBitmapManager::DISPLAY_ROWS + (CDrodBitmapManager::DISPLAY_ROWS / 2);
		this->wLastSrcY = wCenterY - (this->h / 2);
	}
	else
		this->wLastSrcY = 0;

	//Blit map surface to screen.
	SDL_Surface *pDestSurface = GetDestSurface();
	SDL_Rect src = {this->wLastSrcX, this->wLastSrcY, this->w, this->h};
	SDL_Rect dest = {this->x, this->y, this->w, this->h};
	SDL_BlitSurface(this->pMapSurface, &src, pDestSurface, &dest);

	//Draw selected room rectangle if widget area is large enough.
	if (this->w >= CDrodBitmapManager::DISPLAY_COLS + 2 &&
			this->h >= CDrodBitmapManager::DISPLAY_ROWS + 2)
	{
		//Figure coords for rectangle.
		dest.w = CDrodBitmapManager::DISPLAY_COLS + 2;
		dest.h = CDrodBitmapManager::DISPLAY_ROWS + 2;
		if (bScrollHorizontal)
			dest.x = this->x + (this->w - CDrodBitmapManager::DISPLAY_COLS) / 2 - 1;
		else
			dest.x = this->x + this->wBorderW + 
				((this->dwSelectedRoomX - this->dwLeftRoomX) * 
				CDrodBitmapManager::DISPLAY_COLS) - 1;
		if (bScrollVertical)
			dest.y = this->y + (this->h - CDrodBitmapManager::DISPLAY_ROWS) / 2 - 1;
		else
			dest.y = this->y + this->wBorderH + 
				((this->dwSelectedRoomY - this->dwTopRoomY) * 
				CDrodBitmapManager::DISPLAY_ROWS) - 1;


		//Draw two rectangles around selected room to make it 2 pixels thick.
		BYTE boxColor = (IsSelected() || !this->bEditing ? MAP_LTCYAN : MAP_LTGREY);
		DrawRect(dest, m_arrColor[boxColor]);
		--dest.x;
		--dest.y;
		dest.w += 2;
		dest.h += 2;
		DrawRect(dest, m_arrColor[boxColor]);
	}

	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CMapWidget::PaintClipped(
//
//Params:
	const int /*nX*/, const int /*nY*/, const UINT /*wW*/, const UINT /*wH*/,
	const bool /*bUpdateRect*/)
{
	//Paint() uses direct access to pixels, so it can't be clipped with
	//SDL_SetClipRect().  Either you need to write PaintClipped() or change
	//the situation which causes this widget to be clipped.
	ASSERTP(false, "Widget can't be painted clipped.");
}

//*****************************************************************************
bool CMapWidget::PasteRoom()
//Pastes this->pRoom to the selected room spot on the map.
//It might have a room there or not.  If so, delete the old one.
//If entrance room was cut-and-pasted, update level's entrance room position.
//
//Returns: whether a room was pasted
{
	this->bDeletingRoom = false;
	if (!this->bEditing || !this->pLevel || !this->pRoom) return false;

	const DWORD dwRoomX = this->dwSelectedRoomX, dwRoomY = this->dwSelectedRoomY;
	DWORD dwSX, dwSY;

	if (this->pRoom->dwRoomX == dwRoomX && this->pRoom->dwRoomY == dwRoomY)
		return false;	//putting room at same spot -- don't need to do anything

	//Check whether level entrance is being moved.
	this->pLevel->GetStartingRoomCoords(dwSX,dwSY);
	const bool bMovingLevelEntrance = !this->bCopyingRoom &&
			(dwRoomX == dwSX && dwRoomY == dwSY);
   const bool bChangingLevels = this->pLevel->dwLevelID != this->pRoom->dwLevelID;

	if (this->bVacantRoom)
	{
		//Place room at empty spot.
		this->bVacantRoom = false;
		if (!this->bCopyingRoom)
		{
			//Move room.
			this->pRoom->dwRoomX = dwRoomX;
			this->pRoom->dwRoomY = dwRoomY;
         if (bChangingLevels) this->pRoom->UpdateExitIDs();
			this->pRoom->Update();
			this->bCopyingRoom = true;	//future pastes will now copy room
		} else {
			//Copy room.
			CDbRoom *pNewRoom = this->pRoom->MakeCopy();
			if (!pNewRoom) return false;
			pNewRoom->dwRoomX = dwRoomX;
			pNewRoom->dwRoomY = dwRoomY;
			pNewRoom->dwLevelID = this->dwLevelID;	//might be from another level
			pNewRoom->dwRoomID = 0L;	//so new room gets added to DB
         if (bChangingLevels) pNewRoom->UpdateExitIDs();
			pNewRoom->Update();
			delete pNewRoom;
		}
	} else {
		CDbRoom *pRoomHere = this->pLevel->Rooms.GetByCoords(
				this->pLevel->dwLevelID, dwRoomX, dwRoomY);
		ASSERT(pRoomHere);
		if (!this->bCopyingRoom)
		{
			//Move room.  Delete room at this spot.
			this->pLevel->Rooms.Delete(pRoomHere->dwRoomID);
			this->pRoom->dwRoomX = dwRoomX;
			this->pRoom->dwRoomY = dwRoomY;
         if (bChangingLevels) this->pRoom->UpdateExitIDs();
			this->pRoom->Update();
			this->bCopyingRoom = true;	//future pastes will now copy room
		} else {
			//Copy other room's data into this room.
			const DWORD dwRoomID = pRoomHere->dwRoomID;
			pRoomHere->MakeCopy(*this->pRoom);
			pRoomHere->dwRoomX = dwRoomX;	//keep same coords and ID
			pRoomHere->dwRoomY = dwRoomY;
			pRoomHere->dwLevelID = this->dwLevelID;	//might be from another level
			pRoomHere->dwRoomID = dwRoomID;
         if (bChangingLevels) pRoomHere->UpdateExitIDs();
			pRoomHere->Update();
		}
		delete pRoomHere;
	}

	//If entrance room was moved, update level entrance room coords.
	if (bMovingLevelEntrance)
	{
		this->pLevel->dwStartingRoomX = dwRoomX;
		this->pLevel->dwStartingRoomY = dwRoomY;
		this->pLevel->Update();
	}

	g_pTheSound->PlaySoundEffect(SEID_DOOROPEN);

	//Update map.
	LoadMapSurface();

	//Call OnSelectChange() notifier.
	CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
	if (pEventHandler) pEventHandler->OnSelectChange(this->GetTagNo());

	return true;
}

//
//Protected methods.
//

//*****************************************************************************
bool CMapWidget::Load()
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	
	//Load from the current game if possible.
	if (this->pCurrentGame)
		this->bIsLoaded = LoadFromCurrentGame(this->pCurrentGame);
	else
		this->bIsLoaded = true;

	//Load any children widgets.
	if (this->bIsLoaded)
		this->bIsLoaded = LoadChildren();

	return this->bIsLoaded;
}

//*****************************************************************************
void CMapWidget::Unload()
//Unload resources for the widget.
{
	ASSERT(this->bIsLoaded);

	//Unload any children widgets.
	UnloadChildren();

	if (this->pMapSurface) 
	{
		SDL_FreeSurface(this->pMapSurface);
		this->pMapSurface = NULL;
	}
	
	delete this->pRoom;

	this->bIsLoaded = false;
}

//*****************************************************************************
void CMapWidget::SetDarkenedRooms(
//Set rooms that will appear as darkened.
//
//Params:
	CIDList &RoomIDs)	//(in) Rooms to darken.
{
	this->DarkenedRooms = RoomIDs;

	LoadFromCurrentGame(this->pCurrentGame);
	SelectRoom(this->dwSelectedRoomX, this->dwSelectedRoomY);
}

//*****************************************************************************
void CMapWidget::SetDestSurface(
//Calls CWidget method and fixes this object after the new surface has been set.
//
//Params:
	SDL_Surface *pNewSurface)
{
	CWidget::SetDestSurface(pNewSurface);
	if (IsLoaded()) 
		//The new surface may have different surface colors.
		InitMapColors();
}

//
//Private methods.
//

//*****************************************************************************
bool CMapWidget::IsAdjacentToValidRoom(
//Returns: true if room coords are adjacent to an existing room, else false
//
//Accepts:
  const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Room #
{
	if (g_pTheDB->Rooms.FindIDAtCoords(this->pLevel->dwLevelID, dwRoomX-1, dwRoomY))
		return true;
	if (g_pTheDB->Rooms.FindIDAtCoords(this->pLevel->dwLevelID, dwRoomX+1, dwRoomY))
		return true;
	if (g_pTheDB->Rooms.FindIDAtCoords(this->pLevel->dwLevelID, dwRoomX, dwRoomY-1))
		return true;
	if (g_pTheDB->Rooms.FindIDAtCoords(this->pLevel->dwLevelID, dwRoomX, dwRoomY+1))
		return true;

	//No room exists on any side.
	return false;
}

//*****************************************************************************
void CMapWidget::SelectRoom(
//Sets the room that will be selected.
//When using the editor, a vacant spot can be selected.
//
//Accepts:
  const DWORD dwRoomX, const DWORD dwRoomY)
{
	ASSERT(this->bEditing || dwRoomX >= this->dwLeftRoomX);
	ASSERT(this->bEditing || dwRoomY >= this->dwTopRoomY);
	ASSERT(this->bEditing || dwRoomX <= this->dwRightRoomX);
	ASSERT(this->bEditing || dwRoomY <= this->dwBottomRoomY);
	this->bVacantRoom = false;	//set to true elsewhere

	this->dwSelectedRoomX = dwRoomX;
	this->dwSelectedRoomY = dwRoomY;
}

//*****************************************************************************
bool CMapWidget::SelectRoomIfValid(
//If room exists and has been explored, select it.
//
//Returns: true if valid room was selected, else false
//
//Accepts:
  const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Room #
{
	//General bounds check.
	if (!this->bEditing &&
			(dwRoomX < this->dwLeftRoomX || dwRoomY < this->dwTopRoomY ||
			dwRoomX > this->dwRightRoomX ||	dwRoomY > this->dwBottomRoomY))
		return false;

	//Does room exist?
   if (!this->pLevel) return false;
	const DWORD dwRoomID = g_pTheDB->Rooms.FindIDAtCoords(this->pLevel->dwLevelID,
			dwRoomX, dwRoomY);
	if (dwRoomID) //Yes.
	{
		//Has it been explored?
		if (this->bEditing || this->ExploredRooms.IsIDInList(dwRoomID)) //Yes.
		{
			//Select the room.
			SelectRoom(dwRoomX, dwRoomY);
			return true;
		}
	} else {
		if (this->bEditing && IsAdjacentToValidRoom(dwRoomX, dwRoomY))
		{
			//Select vacant spot anyway.  New room can be placed there.
			SelectRoom(dwRoomX, dwRoomY);
			this->bVacantRoom = true;
			return true;
		}
	}
	return false;
}

//*****************************************************************************
void CMapWidget::SelectRoomAtCoords(
//Sets the selected room to one currently displayed within widget area 
//containing specified coords.  If coords are invalid, room selection will not
//change.
//
//Accepts:
  const int nX, const int nY)	//(in) Coords within widget area.
{
	ASSERT(ContainsCoords(nX, nY));
	if (!this->pMapSurface) return;

	//Figure where coords are in relation to source surface.
	const UINT wSrcX = this->wLastSrcX + (nX - this->x);
	const UINT wSrcY = this->wLastSrcY + (nY - this->y);
	ASSERT(static_cast<int>(wSrcX) < this->pMapSurface->w);
	ASSERT(static_cast<int>(wSrcY) < this->pMapSurface->h);

	//Figure which room corresponds to source coords.
	const int offsetX = (int)wSrcX - (int)this->wBorderW;
	const int offsetY = (int)wSrcY - (int)this->wBorderH;
	DWORD dwRoomX = (int)this->dwLeftRoomX + (offsetX / (int)CDrodBitmapManager::DISPLAY_COLS),
			dwRoomY = (int)this->dwTopRoomY + (offsetY / (int)CDrodBitmapManager::DISPLAY_ROWS);
	if (offsetX < 0) --dwRoomX;
	if (offsetY < 0) --dwRoomY;

	if (this->dwSelectedRoomX != dwRoomX || this->dwSelectedRoomY != dwRoomY)
		SelectRoomIfValid(dwRoomX, dwRoomY);
}

//**********************************************************************************
void CMapWidget::InitMapColors()
//Set values in color array used for drawing map pixels.
//
//Returns:
//Pointer to new structure of NULL for failure.
{
	//One call is enough, since m_arrColor is static module-scoped.
	static bool bInitOnce = false;
	if (bInitOnce) return;
	bInitOnce = true;

	//Get surface-writable bytes for the colors used for
	//map pixels.
	m_arrColor[MAP_BLACK] =		GetSurfaceColor(this->pMapSurface, 0,  0,  0);
	m_arrColor[MAP_WHITE] =		GetSurfaceColor(this->pMapSurface, 255,255,255);
	m_arrColor[MAP_LTYELLOW] =	GetSurfaceColor(this->pMapSurface, 255,255,0);
	m_arrColor[MAP_RED] =		GetSurfaceColor(this->pMapSurface, 255,0,  0);
	m_arrColor[MAP_LTGREEN] =	GetSurfaceColor(this->pMapSurface, 0,  255,0);
	m_arrColor[MAP_LTCYAN] =	GetSurfaceColor(this->pMapSurface, 0,  255,255);
	m_arrColor[MAP_DKCYAN] =	GetSurfaceColor(this->pMapSurface, 0,  128,128);
	m_arrColor[MAP_BLUE] =		GetSurfaceColor(this->pMapSurface, 0,  0,  128);
	m_arrColor[MAP_LTBLUE] =	GetSurfaceColor(this->pMapSurface, 0,  0,  255);
	m_arrColor[MAP_MAGENTA] =	GetSurfaceColor(this->pMapSurface, 128,0,  128);
	m_arrColor[MAP_LTGREY] =	GetSurfaceColor(this->pMapSurface, 200,200,200);
	m_arrColor[MAP_DKCYAN2] =	GetSurfaceColor(this->pMapSurface, 0,  64, 64);
	m_arrColor[MAP_GRAY] =		GetSurfaceColor(this->pMapSurface, 128,128,128);
}

//*****************************************************************************
bool CMapWidget::LoadMapSurface()
//Creates map surface resource containing representation of all rooms in level.
{
	bool bSuccess = true;

	//Load every room in the current level.
	list<CDbRoom *> DrawRooms;
	CDbRoom *pRoom = this->pLevel->Rooms.GetFirst();
	if (!pRoom)
	{
		//No rooms to display.
		DrawPlaceholder();
		return true;
	}
	this->dwLeftRoomX = this->dwRightRoomX = pRoom->dwRoomX;
	this->dwBottomRoomY = this->dwTopRoomY = pRoom->dwRoomY;
	while (pRoom)
	{
		//Look for expansion of level boundaries.
		if (pRoom->dwRoomX < this->dwLeftRoomX)
			this->dwLeftRoomX = pRoom->dwRoomX;
		else if (pRoom->dwRoomX > this->dwRightRoomX)
			this->dwRightRoomX = pRoom->dwRoomX;
		if (pRoom->dwRoomY < this->dwTopRoomY)
			this->dwTopRoomY = pRoom->dwRoomY;
		else if (pRoom->dwRoomY > this->dwBottomRoomY)
			this->dwBottomRoomY = pRoom->dwRoomY;

		//Keep the rooms that have been explored in a list.
		if (this->bEditing)
			DrawRooms.push_back(pRoom);
		else
		{
			if (this->pCurrentGame->IsRoomAtCoordsExplored(pRoom->dwRoomX, 
					pRoom->dwRoomY))
				DrawRooms.push_back(pRoom);
			else
				delete pRoom;
		}

		//Get next room in level.
		pRoom = this->pLevel->Rooms.GetNext();
	}

   //Currently, a room's y-coordinate contains a pseudo-level encoding in its 100s place.
   //Therefore, a level's rooms should not have y-coords that overflow into to the next level's room coords.
   ASSERT(this->dwBottomRoomY - this->dwTopRoomY + 1 <= 100);

	//Determine dimensions of map for level and border size.  Border is so 
	//that I can scroll map to edge rooms without worrying about the display.
	const UINT wMapW = CDrodBitmapManager::DISPLAY_COLS *
			(this->dwRightRoomX - this->dwLeftRoomX + 1) + 1;
	const UINT wMapH = CDrodBitmapManager::DISPLAY_ROWS *
			(this->dwBottomRoomY - this->dwTopRoomY + 1) + 1;
	//always scroll so as to be able to add empty rooms on the edges in the level editor.
	this->bScrollHorizontal = (wMapW > this->w) || this->bEditing;
	this->bScrollVertical = (wMapH > this->h) || this->bEditing;
	if (this->bScrollHorizontal)
		this->wBorderW = this->w - (this->bEditing ? 0 : CDrodBitmapManager::DISPLAY_COLS);
	else
		this->wBorderW = (this->w - wMapW) / 2;
	if (this->bScrollVertical)
		this->wBorderH = this->h - (this->bEditing ? 0 : CDrodBitmapManager::DISPLAY_ROWS);
	else
		this->wBorderH = (this->h - wMapH) / 2;

   // Delete the surface if it exists
	if (this->pMapSurface)
      SDL_FreeSurface(this->pMapSurface);

   //Create the surface
   this->pMapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			wMapW + (this->wBorderW * 2), wMapH + (this->wBorderH * 2),
			24, 0, 0, 0, 0);
	if (!this->pMapSurface)
	{
        CFiles Files;
		Files.AppendErrorLog("CMapWidget::LoadMapSurface()--SDL_CreateRGBSurface() failed.");
		bSuccess = false;
		goto Cleanup;
	}

	//Get colors for drawing on map surface.
	InitMapColors();

	//Fill map surface with dark cyan.
	SDL_FillRect(this->pMapSurface,NULL,m_arrColor[MAP_DKCYAN].byt3 << 16 |
			m_arrColor[MAP_DKCYAN].byt2 << 8 | m_arrColor[MAP_DKCYAN].byt1);

	{	
		//Draw each room onto the map.
		for( list<CDbRoom *>::const_iterator iSeek = DrawRooms.begin();
			iSeek != DrawRooms.end(); ++iSeek)
		{
			DrawMapSurfaceFromRoom(*iSeek);
		}
	}

Cleanup:
	for( list<CDbRoom *>::const_iterator iSeek = DrawRooms.begin();
		iSeek != DrawRooms.end(); ++iSeek)
	{
		delete *iSeek;
	}
	return bSuccess;
}

//*****************************************************************************
void CMapWidget::UpdateMapSurface()
//Updates map surface with any differences between map and current game.
{
	bool bDrawCurrentRoom = false;

	//If level changed, then a complete reload is needed.
	ASSERT(this->pCurrentGame);
	if (this->pCurrentGame->pLevel->dwLevelID != this->dwLevelID)
	{
		LoadFromCurrentGame(this->pCurrentGame);
		return;
	}

	//See if all the current game conquered rooms are shown in map.
	if (this->pCurrentGame->ConqueredRooms.GetSize() != 
			this->ConqueredRooms.GetSize())
	{
		IDNODE *pConquered = this->pCurrentGame->ConqueredRooms.Get(0);
		while (pConquered)
		{
			if (!this->ConqueredRooms.IsIDInList(pConquered->dwID))
			{
				//A new conquered room to update my map with.
				this->ConqueredRooms.Add(pConquered->dwID);
				if (this->pCurrentGame->pRoom->dwRoomID == pConquered->dwID)
					bDrawCurrentRoom = true;
				else
				{
					//Load the room in.
					CDbRoom *pTempRoom = this->pLevel->Rooms.GetByID(
							pConquered->dwID);
					if (pTempRoom)
					{
						DrawMapSurfaceFromRoom(pTempRoom);
						delete pTempRoom;
					}
					else
						ASSERTP(false, "Failed to retrieve room");
				}
			}
			pConquered = pConquered->pNext;
		} //...while(pConquered)
	} //...if conquered room list sizes don't match.
	
	//See if all the current game explored rooms are shown in map.
	if (this->pCurrentGame->ExploredRooms.GetSize() != 
			this->ExploredRooms.GetSize())
	{
		IDNODE *pExplored = this->pCurrentGame->ExploredRooms.Get(0);
		while (pExplored)
		{
			if (!this->ExploredRooms.IsIDInList(pExplored->dwID))
			{
				//A new explored room to update my map with.
				this->ExploredRooms.Add(pExplored->dwID);

				//Draw the room if it isn't in the conquered list (already drawn).
				if (!this->ConqueredRooms.IsIDInList(pExplored->dwID))
				{
					if (this->pCurrentGame->pRoom->dwRoomID == pExplored->dwID)
						bDrawCurrentRoom = true;
					else
					{
						//Load the room in.
						CDbRoom *pTempRoom = this->pLevel->Rooms.GetByID(
								pExplored->dwID);
						if (pTempRoom)
						{
							DrawMapSurfaceFromRoom(pTempRoom);
							delete pTempRoom;
						}
						else
							ASSERTP(false, "Failed to retrieve room.");
					}
				}
			}
			pExplored = pExplored->pNext;
		}
	}

	//New room is the current room.
	if (bDrawCurrentRoom)
	{
		DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
	}

	//Is level complete in current game, but not map?
	if (this->pCurrentGame->IsCurrentLevelComplete() && !this->bIsLevelComplete)
	{
		this->bIsLevelComplete = true;
		RemoveBlueDoorsFromMapSurface();
	}
}

//*****************************************************************************
void CMapWidget::DrawMapSurfaceFromRoom(
//Loads a room from disk into a specified position in the map surface.
//
//Params:
  const CDbRoom *pRoom)	//(in)	Contains coords of room to update on map
						//		as well as the squares of the room to use
						//		in determining pixels.
{
	ASSERT(pRoom);

	//Get variables that affect how map pixels are set.
	//When there is no current game, then show everything fully.
	const bool bConquered = (this->pCurrentGame ? this->pCurrentGame->
			IsRoomAtCoordsConquered(pRoom->dwRoomX, pRoom->dwRoomY) : true);
	const bool bCompleted = (this->pCurrentGame ? this->pCurrentGame->
			IsCurrentLevelComplete() : false);
	const bool bDarkened  = (this->pCurrentGame ?
			this->DarkenedRooms.IsIDInList(pRoom->dwRoomID) : false);
   const bool bPendingExit = (this->pCurrentGame ?
         pRoom->dwRoomID == this->pCurrentGame->pRoom->dwRoomID &&   //only applies to current room
         this->pCurrentGame->IsCurrentRoomPendingExit() : false);

	//Set colors in map surface to correspond to squares in the room.
	LockMapSurface();

	SURFACECOLOR Color;
	static const UINT wBPP = this->pMapSurface->format->BytesPerPixel;
	const DWORD dwRowOffset = this->pMapSurface->pitch - (CDrodBitmapManager::DISPLAY_COLS * wBPP);
	Uint8 *pSeek = GetRoomStart(pRoom->dwRoomX, pRoom->dwRoomY);
	Uint8 *pStop = pSeek + (this->pMapSurface->pitch * CDrodBitmapManager::DISPLAY_ROWS);
	UINT wSquareIndex=0;

	//Each iteration draws one row.
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		Uint8 *pEndOfRow = pSeek + (CDrodBitmapManager::DISPLAY_COLS * wBPP);

		//Each iteration draws one pixel.
		while (pSeek != pEndOfRow)
		{
			Color = GetMapColorFromTile(
				(unsigned char) pRoom->pszOSquares[wSquareIndex],
				(unsigned char) pRoom->pszTSquares[wSquareIndex],
				bConquered, bCompleted, bDarkened, bPendingExit);
			pSeek[0] = Color.byt1;
			pSeek[1] = Color.byt2;
			pSeek[2] = Color.byt3;

			++wSquareIndex;
			pSeek += wBPP;
		}
		pSeek += dwRowOffset;
	}
			
	UnlockMapSurface();
}

//*****************************************************************************
inline void CMapWidget::LockMapSurface()
//Simplified call to SDL_LockSurface().
{
	if (SDL_MUSTLOCK(this->pMapSurface)) 
	{
		if ( SDL_LockSurface(this->pMapSurface) <= 0 ) ASSERTP(false, "Failed to lock surface.");
	}
}

//*****************************************************************************
inline void CMapWidget::UnlockMapSurface()
//Simplified call to SDL_LockSurface().
{
	if (SDL_MUSTLOCK(this->pMapSurface)) SDL_UnlockSurface(this->pMapSurface);
}

//*****************************************************************************
inline SURFACECOLOR CMapWidget::GetMapColorFromTile(
//Returns the map image color that corresponds to a given tile#.
//
//Accepts:
	const UINT wOpaqueTile, 
	const UINT wTransparentTile,
	const bool bRoomConquered, 
	const bool bLevelComplete,
	const bool bDarkened,
   const bool bPendingExit)
{
	if (wTransparentTile == T_TAR)
		return m_arrColor[MAP_MAGENTA];

	switch (wOpaqueTile)
	{
		case T_WALL: case T_WALL_B:
			return bDarkened ? m_arrColor[MAP_DKCYAN2] : m_arrColor[MAP_BLACK];
		case T_STAIRS: case T_DOOR_Y:
			return m_arrColor[MAP_LTYELLOW];
		case T_DOOR_M:
			if (bRoomConquered) 
				return bDarkened ? m_arrColor[MAP_LTGREY] : m_arrColor[MAP_WHITE];
			else
				return m_arrColor[MAP_LTGREEN];
		case T_DOOR_C:
			if (bLevelComplete)
				return bDarkened ? m_arrColor[MAP_DKCYAN] :
						(bRoomConquered ? m_arrColor[MAP_WHITE] :
                  (bPendingExit ? m_arrColor[MAP_LTGREEN] : m_arrColor[MAP_RED]));
			else
				return m_arrColor[MAP_LTCYAN];
		case T_PIT:
			return m_arrColor[MAP_BLUE];
		case T_OB_1:
			return m_arrColor[MAP_GRAY];
		default:
         return bDarkened ? m_arrColor[MAP_DKCYAN] :
               (bRoomConquered ? m_arrColor[MAP_WHITE] : 
               (bPendingExit ? m_arrColor[MAP_LTGREEN] : m_arrColor[MAP_RED]));
   } 
}

//*****************************************************************************
inline Uint8 *CMapWidget::GetRoomStart(
//Gets location in map surface pixels for top lefthand corner of a room.
//
//Accepts:
	const DWORD dwRoomX, const DWORD dwRoomY)
{
	ASSERT(dwRoomX >= this->dwLeftRoomX);
	ASSERT(dwRoomY >= this->dwTopRoomY);
	ASSERT(dwRoomX <= this->dwRightRoomX);
	ASSERT(dwRoomY <= this->dwBottomRoomY);
	ASSERT(this->pMapSurface);
	
	//Figure x and y inside of pixel map.
	const DWORD x = (dwRoomX - this->dwLeftRoomX) *
			CDrodBitmapManager::DISPLAY_COLS + this->wBorderW;
	const DWORD y = (dwRoomY - this->dwTopRoomY) *
			CDrodBitmapManager::DISPLAY_ROWS + this->wBorderH;

	//Figure address of pixel.
	return (Uint8 *)this->pMapSurface->pixels + (y * this->pMapSurface->pitch) + 
			(x * this->pMapSurface->format->BytesPerPixel);
}

//*****************************************************************************
void CMapWidget::RemoveBlueDoorsFromMapSurface()
{
	const SURFACECOLOR LtCyan = m_arrColor[MAP_LTCYAN];
	const SURFACECOLOR White = m_arrColor[MAP_WHITE];

	LockMapSurface();

	static UINT wBPP = this->pMapSurface->format->BytesPerPixel;
	const UINT wRowW = ((this->dwRightRoomX - this->dwLeftRoomX) *
			CDrodBitmapManager::DISPLAY_COLS * wBPP);
	UINT wRowOffset = this->pMapSurface->pitch - wRowW;
	Uint8 *pSeek = GetRoomStart(this->dwLeftRoomX, this->dwTopRoomY);
	Uint8 *pStop = pSeek + ((this->dwBottomRoomY - this->dwTopRoomY) * 
			CDrodBitmapManager::DISPLAY_ROWS * this->pMapSurface->pitch);

	//Eaach iteration draws one row.
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		Uint8 *pEndOfRow = pSeek + wRowW;

		//Each iteration draws or skips over one pixel.
		while (pSeek != pEndOfRow)
		{
			if (pSeek[0] == LtCyan.byt1 &&
					pSeek[1] == LtCyan.byt2 &&
					pSeek[2] == LtCyan.byt3) //Found blue door.
			{
				//Remove it by redrawing with white pixel.
				pSeek[0] = White.byt1;
				pSeek[1] = White.byt2;
				pSeek[2] = White.byt3;
			}
			pSeek += wBPP;
		}

		pSeek += wRowOffset;
	}

	UnlockMapSurface();
}

// $Log: MapWidget.cpp,v $
// Revision 1.49  2004/08/10 02:04:58  mrimer
// Fixed scroll text copy bugs when a room is copied.
//
// Revision 1.48  2004/01/02 00:59:12  mrimer
// Changed cut/copy room sound to be distinct from cut/copy level sound.
//
// Revision 1.47  2003/11/09 05:21:44  mrimer
// Fixed bug: stair destinations not changed when room is moved to new hold.
//
// Revision 1.46  2003/10/27 20:21:39  mrimer
// Fixed cosmetic bug: Blue doors turn red on map when room is cleared.
//
// Revision 1.45  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.44  2003/07/29 03:53:18  mrimer
// Fixed crash: room query when no level is specified.
//
// Revision 1.43  2003/07/16 07:42:45  mrimer
// Added an assertion: assuming maps are < 100 rooms tall.
//
// Revision 1.42  2003/07/10 03:42:52  schik
// Now frees and re-allocates the map surface, as Mike says the map size
// can change and this it how it should be done.
//
// Revision 1.41  2003/07/10 01:32:08  schik
// Only creates a new surface if there isn't a surface created yet.
//
// Revision 1.40  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.39  2003/06/23 04:14:56  schik
// Fixed crash when changing rooms in the editor or exiting the editor
//
// Revision 1.38  2003/06/22 05:58:00  mrimer
// Fixed a bug.
//
// Revision 1.37  2003/06/20 23:55:24  mrimer
// Fixed a bug.
//
// Revision 1.36  2003/06/18 21:21:37  schik
// Added visual cue when room is in conquered state.
//
// Revision 1.35  2003/05/28 23:13:11  erikh2000
// Methods of CFiles are called differently.
//
// Revision 1.34  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.33  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.32  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.31  2003/05/03 23:31:29  mrimer
// Added ClearMap().
//
// Revision 1.30  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.29  2002/12/22 02:36:49  mrimer
// Refined room cutting/copying for moving rooms between levels.  Fixed color problem with room highlighting box.
//
// Revision 1.28  2002/11/22 22:02:42  mrimer
// Implemented cutting, copying, and pasting rooms.
// Added UpdateFromCurrentLevel().
//
// Revision 1.27  2002/11/22 02:32:07  mrimer
// Added obstacle drawing to map.  Fixed drawing bug for small levels.
//
// Revision 1.26  2002/11/15 02:38:00  mrimer
// Added support for use in level editor.
//
// Revision 1.25  2002/09/24 21:38:49  mrimer
// Revised calls to CFiles::AppendErrorLog().
//
// Revision 1.24  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.23  2002/07/20 23:14:32  erikh2000
// Revised keydown handler to call an event notifier when selection changes, and to not repaint itself when no new selection is made.
// Fixed a problem in Unload with a bad pointer.
//
// Revision 1.22  2002/07/19 20:38:37  mrimer
// Revised HandleKeyDown() to return no type.
//
// Revision 1.21  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.20  2002/07/03 22:06:05  mrimer
// Changed darkened room color (DKCYAN and DKCYAN2).
// Various optimizations.
// Added const to formal parameters.
//
// Revision 1.19  2002/06/21 05:18:34  mrimer
// Revised includes.
//
// Revision 1.18  2002/06/20 00:55:54  erikh2000
// If widget doesn't have a current game set, it will draw a placeholder rect.
//
// Revision 1.17  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.16  2002/06/15 18:33:57  erikh2000
// Paint() exits early if current game is not set.
//
// Revision 1.15  2002/06/14 00:55:47  erikh2000
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
// Renamed ClipWHToScreen() call to new ClipWHToDest() method.
// Wrote SetDestSurface() override that recalcs cached surface colors on each call.
//
// Revision 1.14  2002/06/11 22:54:25  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.13  2002/06/07 17:52:23  mrimer
// Fixed mouse click handling.
//
// Revision 1.12  2002/06/06 00:03:00  mrimer
// Added mouseless UI.  Added focus graphic.
//
// Revision 1.11  2002/06/05 03:17:43  mrimer
// Added widget focusability and keyboard control.
// Added focus graphic.
//
// Revision 1.10  2002/05/21 21:35:29  erikh2000
// Changed color-setting code.
//
// Revision 1.9  2002/05/12 03:17:46  erikh2000
// Fixed bug preventing unconquered and darkened rooms from being displayed with dark red.
//
// Revision 1.8  2002/05/10 22:38:06  erikh2000
// Added ability to draw some rooms in a darkened state.
//
// Revision 1.7  2002/04/29 00:14:39  erikh2000
// Added methods used for selecting rooms by clicking on the map widget.
//
// Revision 1.6  2002/04/19 21:43:49  erikh2000
// Removed references to ScreenConstants.h.
//
// Revision 1.5  2002/04/14 00:32:41  erikh2000
// Changed way that colors are calculated.
// Added PaintClipped() method that fires an assertian.
//
// Revision 1.4  2002/04/13 19:44:24  erikh2000
// Added new type parameter to CWidget construction call.
//
// Revision 1.3  2002/04/09 10:05:38  erikh2000
// Fixed revision log macro.
//
