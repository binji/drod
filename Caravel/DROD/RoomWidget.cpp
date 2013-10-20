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

#include "RoomWidget.h"
#include "DrodBitmapManager.h"
#include "DrodScreen.h"
#include "NeatherStrikesOrbEffect.h"
#include "RoomEffectList.h"
#include "StrikeOrbEffect.h"
#include "TileImageCalcs.h"
#include "TileImageConstants.h"
#include <FrontEndLib/Bolt.h>
#include <FrontEndLib/FrameRateEffect.h>
#include <FrontEndLib/Pan.h>
#include <FrontEndLib/ShadeEffect.h>

#include "../DRODLib/CurrentGame.h"
#include "../DRODLib/GameConstants.h"
#include "../DRODLib/Mimic.h"
#include "../DRODLib/Monster.h"
#include "../DRODLib/MonsterFactory.h"
#include "../DRODLib/Neather.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/Db.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Files.h>

#define IS_COLROW_IN_DISP(c,r) \
		(static_cast<UINT>(c) < CDrodBitmapManager::DISPLAY_COLS && \
		static_cast<UINT>(r) < CDrodBitmapManager::DISPLAY_ROWS)

//change frame once every 5 seconds, on average
const int MONSTER_ANIMATION_DELAY = 5;

//
//Public methods.
//

//*****************************************************************************
CRoomWidget::CRoomWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH,					//
	const CCurrentGame *pSetCurrentGame)	//(in)	Game to use for drawing the
											//		room.
	: CWidget(WT_Room, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, pRoomSnapshotSurface(NULL)

	, dwRoomX(0L), dwRoomY(0L)
	, wStyle((UINT)-1)
	, wShowCol(0), wShowRow(0)

   , pCurrentGame(pSetCurrentGame)
	, pRoom(NULL)
	, pwOSquareTI(NULL), pwTSquareTI(NULL), pwMSquareTI(NULL), pbEdges(NULL)

	, pBoltPartsSurface(NULL)

	, bShowingSwordsman(true)
	, bShowCheckpoints(true)
	, bShowFrameRate(false)

	, dwLastDrawSquareInfoUpdateCount(0L)
   , dwLastAnimationFrame(SDL_GetTicks())

	, bAllDirty(true)
	, bWasPlacingMimic(false)
   , bWasInvisible(false)
	, pTileInfo(NULL)
	, wLastOrientation(0), wLastX(0), wLastY(0)
	, bLastRaised(false)

	, CX_TILE(CBitmapManager::CX_TILE), CY_TILE(CBitmapManager::CY_TILE)
{
   this->pLastLayerEffects = new CRoomEffectList(this);
	this->pTLayerEffects = new CRoomEffectList(this);
}

//*****************************************************************************
void CRoomWidget::AddLastLayerEffect(
//Adds an effect to the list of effects drawn after the last layer of the room.
//
//Params:
	CEffect *pEffect)	//(in)	Effect to add.  CRoomWidget will take ownership
						//		of the pointer, and caller shouldn't delete.
{
	ASSERT(pEffect);
	this->pLastLayerEffects->AddEffect(pEffect);
}

//*****************************************************************************
void CRoomWidget::AddTLayerEffect(
//Adds an effect to the list of effects drawn after the transparent layer of 
//the room.
//
//Params:
	CEffect *pEffect)	//(in)	Effect to add.  CRoomWidget will take ownership
						//		of the pointer, and caller shouldn't delete.
{
	ASSERT(pEffect);
	this->pTLayerEffects->AddEffect(pEffect);
}

//*****************************************************************************
void CRoomWidget::AddShadeEffect(
//Adds a Shade effect of given color to room tile.
//
//Params:
	const UINT wX, const UINT wY, const SURFACECOLOR &Color)	//(in)
{
	CCoord Coord(wX,wY);
	AddLastLayerEffect(new CShadeEffect(this, Coord, Color));
}

//*****************************************************************************
void CRoomWidget::AddStrikeOrbEffect(
//Add a strike orb effect to room.
//
//Params:
	const COrbData &SetOrbData,	//(in) Orb to be struck.
   bool bDrawOrb)
{
	AddLastLayerEffect(
		new CStrikeOrbEffect(this, SetOrbData, this->pBoltPartsSurface, bDrawOrb));
}

//*****************************************************************************
void CRoomWidget::ClearEffects(
//Clears all effects in the room.
//If bKeepFrameRate is true, then update screen areas also.
//
//Params:
	const bool bKeepFrameRate)	//(in)	If true (default), the frame rate
										//			effect will persist.
{
	//Keep room synched.
	if (this->pCurrentGame)
		if (this->pCurrentGame->pRoom != this->pRoom)
			this->pRoom = this->pCurrentGame->pRoom;

	//Clear layer lists.
	this->pTLayerEffects->Clear(bKeepFrameRate);
	this->pLastLayerEffects->Clear(bKeepFrameRate);

	//Add frame rate effect back in if appropriate.
	if (bKeepFrameRate && this->bShowFrameRate)
		AddLastLayerEffect(new CFrameRateEffect(this));
}

//*****************************************************************************
UINT * CRoomWidget::GetMonsterTile(
//
//Params:
	const UINT wCol, const UINT wRow)	//(in) Position of square
//Returns:
//Pointer to animation frame for this square
{
	ASSERT(this->pwMSquareTI);
	ASSERT(wCol < this->pRoom->wRoomCols);
	ASSERT(wRow < this->pRoom->wRoomRows);
	return (this->pwMSquareTI + ARRAYINDEX(wCol,wRow));
}

//*****************************************************************************
bool CRoomWidget::LoadFromCurrentGame(
//Loads widget from current game.
//
//Params:
	const CCurrentGame *pSetCurrentGame)
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pSetCurrentGame);
	this->pCurrentGame = pSetCurrentGame;
	this->pRoom = pSetCurrentGame->pRoom;	//quick access

	//Update vars used for comparison of widget to current game.
	this->wStyle = this->pRoom->wStyle;
	this->dwRoomX = this->pRoom->dwRoomX;
	this->dwRoomY = this->pRoom->dwRoomY;

	ClearEffects();

	//Load tile images.
	if (!g_pTheDBM->LoadTileImagesForStyle(this->wStyle)) return false;

	//Set tile image arrays to new current room.
	ResetForPaint();

	return true;
}

//*****************************************************************************
void CRoomWidget::HideFrameRate()
//Hides frame rate.
{
	ASSERT(this->bShowFrameRate);

	this->pLastLayerEffects->RemoveEffectsOfType(EFRAMERATE);
	this->bShowFrameRate = false;
}

//*****************************************************************************
void CRoomWidget::ShowFrameRate()
//Shows frame rate as a counter drawn on top everything.
{
	ASSERT(!this->bShowFrameRate);

	AddLastLayerEffect(new CFrameRateEffect(this));
	this->bShowFrameRate = true;
}

//*****************************************************************************
void CRoomWidget::ToggleFrameRate()
//Shows frame rate if it is hidden, and hides it if it is showing.
{
	if (this->bShowFrameRate)
		HideFrameRate();
	else
		ShowFrameRate();
}

//*****************************************************************************
void CRoomWidget::ShowRoomTransition(
//Changes view from old room to current one.
//
//Params:
	const UINT wExitOrientation)	//(in) direction of exit
{
	if (IsValidOrientation(wExitOrientation) && wExitOrientation != NO_ORIENTATION)
	{
		//Show a smooth transition between rooms.
		PanDirection panDirection;
		switch (wExitOrientation)
		{
			case N: panDirection = PanNorth;	break;
			case S: panDirection = PanSouth;	break;
			case E: panDirection = PanEast;	break;
			case W: panDirection = PanWest;	break;
			default: ASSERTP(false, "Bad pan direction.");	break;
		}

		//Image of room being left.
		SDL_Surface *pOldRoomSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
				this->w, this->h, 24, 0, 0, 0, 0);
		//Image of room being entered.
		SDL_Surface *pNewRoomSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
				this->w, this->h, 24, 0, 0, 0, 0);
		SDL_Rect rect = {this->x, this->y, this->w, this->h};
		SDL_Rect tempRect = {0, 0, this->w, this->h};

		//If for some reason a surface couldn't be allocated,
		//the new room will be shown without any transition effect.
		if (pOldRoomSurface && pNewRoomSurface)
		{
			//Take snapshot of room being left.
 			SDL_BlitSurface(GetDestSurface(), &rect, pOldRoomSurface, &tempRect);

			//Render image of new room (update this->pRoomSnapshotSurface).
			ClearEffects();
			UpdateFromCurrentGame();
 			SDL_BlitSurface(this->pRoomSnapshotSurface, &rect,
						pNewRoomSurface, &tempRect);

			//Slide from the old to new room.
			static const Uint32 dwPanDuration = 600;
			CPan pan(pOldRoomSurface, pNewRoomSurface, panDirection);
			const DWORD dwStart = SDL_GetTicks();
			while (true)
			{
				const Uint32 dwNow = SDL_GetTicks();
				pan.IncrementPan((dwNow - dwStart) / (float)dwPanDuration);
				SDL_BlitSurface(pOldRoomSurface, &tempRect,
						GetDestSurface(), &rect);
				UpdateRect();
				if (dwNow - dwStart > dwPanDuration)
				{
					//Done panning.
					SDL_FreeSurface(pOldRoomSurface);
					SDL_FreeSurface(pNewRoomSurface);
					return;
				}
			}
		} else {
			if (pOldRoomSurface)
				SDL_FreeSurface(pOldRoomSurface);
		}
	}

	//Just show new room.
	ClearEffects();
	UpdateFromCurrentGame();
	Paint();
}

//*****************************************************************************
void CRoomWidget::UpdateFromCurrentGame()
//Update the room widget so that it is ready to display the room from
//the current game.
{
	//If the room changed, then get the new one.
	ASSERT(this->pCurrentGame);
	if (this->pCurrentGame->pRoom != this->pRoom)
		this->pRoom = this->pCurrentGame->pRoom;

	//Room widget can only display standard-sized rooms now.
	ASSERT(this->pRoom->wRoomCols == CDrodBitmapManager::DISPLAY_COLS);
	ASSERT(this->pRoom->wRoomRows == CDrodBitmapManager::DISPLAY_ROWS);

	if (this->wStyle != this->pRoom->wStyle)
	{
		this->wStyle = this->pRoom->wStyle;
		if (!g_pTheDBM->LoadTileImagesForStyle(this->wStyle)) ASSERTP(false, "Failed to load tile images.");
	}

	DirtyRoom();
	if (this->dwRoomX != this->pRoom->dwRoomX ||
		this->dwRoomY != this->pRoom->dwRoomY)
	{
		//Prepare new room.
		this->dwRoomX = this->pRoom->dwRoomX;
		this->dwRoomY = this->pRoom->dwRoomY;
		if (!UpdateDrawSquareInfo()) ASSERTP(false, "Failed to update draw square info.");
	} else {
		//Just redraw the (same) room.
		Repaint(this->wShowCol, this->wShowRow);
	}
}

//*****************************************************************************
void CRoomWidget::UpdateFromPlots()
//Refresh the tile image arrays after plots have been made.
{
	//Next call will recalc all the tile images instead of just ones affected
	//by plots made.  But this is not a bottleneck so a plot history
	//does not have to be maintained.
	UpdateDrawSquareInfo();
}

//*****************************************************************************
void CRoomWidget::ResetForPaint()
//Reset the object so that the next paint will draw everything fresh with no
//assumptions about what is already drawn in the widget area.
{
	this->bAllDirty = true;

	//Keep room synched.
	if (this->pCurrentGame)
		if (this->pCurrentGame->pRoom != this->pRoom)
			this->pRoom = this->pCurrentGame->pRoom;

	UpdateDrawSquareInfo();
}

//*****************************************************************************
void CRoomWidget::BlitDirtyRoomTiles()
//Redraw all tiles on screen that need refreshing.
{
	const UINT wStartPos = this->wShowRow * this->pRoom->wRoomCols + this->wShowCol;
	const UINT wRowOffset = this->pRoom->wRoomCols - CDrodBitmapManager::DISPLAY_COLS;
	const UINT wXEnd = this->wShowCol + CDrodBitmapManager::DISPLAY_COLS;
	const UINT wYEnd = this->wShowRow + CDrodBitmapManager::DISPLAY_ROWS;

	SDL_Surface *pDestSurface = GetDestSurface();
	SDL_Rect src = {0, 0, CX_TILE, CY_TILE};
	SDL_Rect dest = {0, 0, CX_TILE, CY_TILE};
	TILEINFO *pbMI = this->pTileInfo + wStartPos;
	UINT wX, wY;
	for (wY = this->wShowRow; wY < wYEnd; ++wY)
	{
		for (wX = this->wShowCol; wX < wXEnd; ++wX)
		{
			if ((pbMI++)->dirty)
			{
				src.x = dest.x = this->x + (wX - this->wShowCol) * CX_TILE;
				src.y = dest.y = this->y + (wY - this->wShowRow) * CY_TILE;
				SDL_BlitSurface(this->pRoomSnapshotSurface, &src, pDestSurface, &dest);
			}
		}
		pbMI += wRowOffset;
	}
}

//*****************************************************************************
void CRoomWidget::DrawTileEdges(
//Draws black edges around a tile if needed.
//
//Params:
	const UINT wX, const UINT wY,	//(in) Tile coords.
	const EDGES *pbE,					//(in) Edge data for tile.
   SDL_Surface *pDestSurface)    //(in) Where to draw edges.
{
   static const SURFACECOLOR Black = {0, 0, 0};
	static const int CX_TILE = CBitmapManager::CX_TILE;
	static const int CY_TILE = CBitmapManager::CY_TILE;
	static const UINT DISPLAY_COLS = CDrodBitmapManager::DISPLAY_COLS;
	static const UINT DISPLAY_ROWS = CDrodBitmapManager::DISPLAY_ROWS;

	if (pbE->drawNorthEdge)
		DrawRow(this->x + wX * CX_TILE, this->y + wY * CY_TILE, CX_TILE, Black,
            pDestSurface);
	//join some cases at the corner so it looks better
	else if (wY)
	{
		if (wX)
		{
			if ((pbE - DISPLAY_COLS)->drawWestEdge && (pbE - 1)->drawNorthEdge)
				DrawRow(this->x + wX * CX_TILE, this->y + wY * CY_TILE, 1, Black,
                  pDestSurface);
		}
		if (wX < DISPLAY_COLS-1)
		{
			if ((pbE - DISPLAY_COLS)->drawEastEdge && (pbE + 1)->drawNorthEdge)
				DrawRow(this->x + (wX+1) * CX_TILE - 1,
						this->y + wY * CY_TILE, 1, Black, pDestSurface);
		}
	}

	if (pbE->drawSouthEdge)
		DrawRow(this->x + wX * CX_TILE, this->y + (wY+1) * CY_TILE - 1, CX_TILE,
            Black, pDestSurface);
	//join some cases at the corner so it looks better
	else if (wY < DISPLAY_ROWS-1)
	{
		if (wX)
		{
			if ((pbE + DISPLAY_COLS)->drawWestEdge && (pbE - 1)->drawSouthEdge)
				DrawRow(this->x + wX * CX_TILE, this->y + (wY+1) * CY_TILE - 1, 1,
                  Black, pDestSurface);
		}
		if (wX < DISPLAY_COLS-1)
		{
			if ((pbE + DISPLAY_COLS)->drawEastEdge && (pbE + 1)->drawSouthEdge)
				DrawRow(this->x + (wX+1) * CX_TILE - 1, this->y + (wY+1) *
						CY_TILE - 1, 1, Black, pDestSurface);
		}
	}

	if (pbE->drawWestEdge)
		DrawCol(this->x + wX * CX_TILE, this->y + wY * CY_TILE, CY_TILE, Black,
            pDestSurface);

	if (pbE->drawEastEdge)
		DrawCol(this->x + (wX+1) * CX_TILE - 1, this->y + wY * CY_TILE, CY_TILE,
            Black, pDestSurface);
}

//*****************************************************************************
void CRoomWidget::Repaint(
//Repaints the "physical room" (the o- and t-layers) within the given rectangle.
//Uses "smart" room pre-rendering, only painting tiles that have changed since
//last call.
//
//Params:
	int wCol, int wRow,		//(in) top-left tile coords
	int wWidth, int wHeight)
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	const UINT wRowOffset = this->pRoom->wRoomCols - wWidth;
	const UINT wStartPos = wRow * this->pRoom->wRoomCols + wCol;
	const UINT wXEnd = wCol + wWidth;
	const UINT wYEnd = wRow + wHeight;

	SDL_Surface *pDestSurface = this->pRoomSnapshotSurface;	//draw to here

	//Check for state changes that will affect entire room.
	const bool bIsPlacingMimic = this->pCurrentGame->swordsman.bIsPlacingMimic;
	const bool bIsInvisible = !this->pCurrentGame->swordsman.bIsVisible;
	const bool bPlayerIsDying = this->pCurrentGame->SwordsmanIsDying();
	const bool bStateChanged = (this->bWasPlacingMimic != bIsPlacingMimic) ||
      (this->bWasInvisible && !bIsInvisible) || bPlayerIsDying;
	if (bStateChanged)
		this->bAllDirty = true;

#define DrawRoomTile(wX,wY,wTileImageNo) g_pTheBM->BlitTileImage(\
		wTileImageNo, this->x + wX * CX_TILE,\
		this->y + wY * CY_TILE, pDestSurface, 255)
#define DrawLayeredRoomTiles(wX, wY, wBottomTileImageNo, wTopTileImageNo) \
		g_pTheBM->BlitLayeredTileImage(wBottomTileImageNo, wTopTileImageNo,\
		this->x + wX * CX_TILE,\
		this->y + wY * CY_TILE, pDestSurface, 255)

	UINT *pwOTI = this->pwOSquareTI + wStartPos;
	UINT *pwTTI = this->pwTSquareTI + wStartPos;
	const EDGES *pbE = this->pbEdges + wStartPos;
	const TILEINFO *pbMI = this->pTileInfo + wStartPos;
	UINT wX, wY;

	//Note: Don't need to lock this surface.
	ASSERT(!SDL_MUSTLOCK(pDestSurface));
	bool bIsCheckpoint;
	for (wY = wRow; wY < wYEnd; ++wY)
	{
		for (wX = wCol; wX < wXEnd; ++wX)
		{
			if (this->bAllDirty || pbMI->dirty)
			{
				bIsCheckpoint = this->bShowCheckpoints &&
						this->pRoom->GetOSquare(wX, wY)==T_CHECKPOINT;
				
				//If there are no edges or checkpoint, can optimize tile drawing.
				if (!(pbE->drawEastEdge ||
						pbE->drawNorthEdge || pbE->drawSouthEdge || pbE->drawWestEdge)
					&& !bIsCheckpoint)
				{
					if (*pwTTI != TI_TEMPTY)
					{
						//Draw o- and t-layers simultaneously.
						DrawLayeredRoomTiles(wX, wY, *pwOTI, *pwTTI);
					}
					else
					{
						//Only draw opaque layer.
						DrawRoomTile(wX, wY, *pwOTI);
					}
				} else {
					//1. Draw opaque layer.
					DrawRoomTile(wX, wY, *pwOTI);

					//Draw checkpoint on top of floor.
					if (bIsCheckpoint)
					{
						DrawRoomTile(wX, wY, TI_CHECKPOINT);
					}

					//2. Draw outline around squares that need it.
					DrawTileEdges(wX, wY, pbE, pDestSurface);

					//3. Draw transparent layer.
					if (*pwTTI != TI_TEMPTY)
					{
                  DrawRoomTile(wX, wY, *pwTTI);
					}
				}
			}

			//Advance all pointers to next square.
			++pwOTI;
			++pwTTI;
			++pbMI;
			++pbE;
		}
		pwOTI += wRowOffset;
		pwTTI += wRowOffset;
		pbMI += wRowOffset;
		pbE += wRowOffset;
	}

	//4. When action is frozen, draw the following here.
	if (((!this->bWasPlacingMimic || this->bAllDirty) && bIsPlacingMimic)
			|| bPlayerIsDying)
	{
		//4a. Effects that go on top of room image, under monsters/swordsman.
		this->pTLayerEffects->DrawEffects(true, pDestSurface);	//freeze effects

		//4b. Draw monsters (not killing player).
		DrawMonsters(this->pRoom->pFirstMonster, pDestSurface,
            bIsPlacingMimic || bPlayerIsDying);

		//4c. Mimic placement effects:
		//Make room black-and-white, and draw (unmoving) swordsman on top.
		if (bIsPlacingMimic && !bPlayerIsDying)
		{
			BAndWRect(pDestSurface, this->wShowCol, this->wShowRow);

			ASSERT(this->bShowingSwordsman);
			DrawSwordsman(this->pCurrentGame->swordsman, pDestSurface);
		}
	}
	this->bWasPlacingMimic = bIsPlacingMimic;
	this->bWasInvisible = bIsInvisible;

#undef DrawRoomTile
#undef DrawLayeredRoomTiles
}

//*****************************************************************************
void CRoomWidget::Paint(
//Plots current room to display.
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

	if (!this->pRoom)
	{
		DrawPlaceholder();
		return;
	}

   ASSERT(this->pCurrentGame);
   //Keep room pointer synched.
	if (this->pRoom != this->pCurrentGame->pRoom)
		this->pRoom = this->pCurrentGame->pRoom;

	SDL_Surface *pDestSurface = GetDestSurface();
	const bool bPlayerIsAlive = !this->pCurrentGame->SwordsmanIsDying();
	const bool bIsPlacingMimic = this->pCurrentGame->swordsman.bIsPlacingMimic;
	const UINT wTurn = this->pCurrentGame->wSpawnCycleCount;
	TILEINFO *pbMI;

	//Do player and monsters need to be repainted?
	const bool bMoveMade = (wTurn != this->wLastTurn);
	if (bMoveMade)
		this->wLastTurn = wTurn;

	//1. Blit pre-rendered room.
	SDL_Rect src = {this->x, this->y, this->w, this->h};
	SDL_Rect dest = {this->x, this->y, this->w, this->h};
	ASSERT(this->pRoomSnapshotSurface);
	if (this->bAllDirty)
	{
		DirtySpriteTiles();	//keep information current

		//Blit entire room.
		SDL_BlitSurface(this->pRoomSnapshotSurface, &src, pDestSurface, &dest);
	} else {
		//Blit only dirtied room tiles to save time.
		const bool bIsVisible = (this->pCurrentGame ?
				this->pCurrentGame->swordsman.bIsVisible : true);
		src.w = dest.w = CX_TILE;
		src.h = dest.h = CY_TILE;

		if (bMoveMade)
		{
			//Dirty modified positions of player/monsters to synchronize room image.
			DirtySpriteTiles();
		} else {
			//Animate monsters between moves.
			if (!bIsPlacingMimic && bPlayerIsAlive)
				AnimateMonsters();
		}

		//Touch up tiles where effects were drawn last frame.
		DirtyEffectTiles(bIsPlacingMimic && bPlayerIsAlive,
				!bIsVisible,bMoveMade);

		//Redraw all tiles that have changed.
		BlitDirtyRoomTiles();
	}

	//2. Draw mimic cursor only.
	//(The action is frozen and everything else has been drawn already.)
	if (bIsPlacingMimic && bPlayerIsAlive)
	{
		DrawMimicCursor(this->pCurrentGame->swordsman.wMimicCursorX,
				this->pCurrentGame->swordsman.wMimicCursorY, pDestSurface);
	} else {
		//3. When action is not frozen, draw the following here.
		if (bPlayerIsAlive)
		{
			//3a. Draw effects that go on top of room image, under monsters/swordsman.
			this->pTLayerEffects->DrawEffects();

			//3b. Repaint player/monsters.
			if (this->bAllDirty)
			{
				//Draw monsters (that aren't killing swordsman).
				DrawMonsters(this->pRoom->pFirstMonster, pDestSurface, false);
			} else {
				//Paint monsters whose tiles have been repainted.
				//Also check whether monster is raised up and tile above is dirty.
				//This signifies the monster's image has been clipped on top
				//and must be repainted.
				CMonster *pMonster = this->pRoom->pFirstMonster;
				while (pMonster)
				{
					pbMI = this->pTileInfo + ARRAYINDEX(pMonster->wX,
							pMonster->wY);
					if (pMonster->wType == M_MIMIC)
					{
						//Check whether mimic's sword square is dirty also.
						CMimic *const pMimic = DYN_CAST(CMimic*, CMonster*, pMonster);
						const UINT wSX = pMimic->GetSwordX();
						const UINT wSY = pMimic->GetSwordY();
						if (IS_COLROW_IN_DISP(wSX, wSY))
						{
							if (this->pTileInfo[ARRAYINDEX(wSX, wSY)].dirty ||
									(pbMI->raised && wSY > 0 &&
									this->pTileInfo[ARRAYINDEX(wSX, wSY-1)].dirty))
							{
								DrawMonster(pMonster, this->pRoom, pDestSurface, false);
								pMonster = pMonster->pNext;
								continue;
							}
						}
					}
					if (pbMI->dirty || (pbMI->raised && pMonster->wY > 0 &&
								this->pTileInfo[ARRAYINDEX(pMonster->wX,
										pMonster->wY-1)].dirty))
						DrawMonster(pMonster, this->pRoom, pDestSurface, false);
					
					pMonster = pMonster->pNext;
				}
			}
		}

		//4. Draw swordsman.
		if (this->bShowingSwordsman)
			DrawSwordsman(this->pCurrentGame->swordsman, pDestSurface);

		//5. If monster is killing swordsman, draw it on top.
		if (!bPlayerIsAlive)
		{
			//If swordsman is dead only by monster moving onto the same square, draw it.
			CMonster *const pMonsterOnPlayer =
					this->pRoom->GetMonsterAtSquare(this->pCurrentGame->swordsman.wX,
					this->pCurrentGame->swordsman.wY);
			if (pMonsterOnPlayer)
				DrawMonster(pMonsterOnPlayer, this->pRoom, pDestSurface, true);
         else {
            //If killed by mimic's sword, draw the mimic.
	         UINT wSX, wSY;
	         CMonster *pSeek = this->pRoom->pFirstMonster;
	         while (pSeek)
	         {
		         if (pSeek->wType == M_MIMIC)
		         {
			         CMimic *pMimic = DYN_CAST(CMimic*, CMonster*, pSeek);
			         wSX = pMimic->GetSwordX();
			         wSY = pMimic->GetSwordY();
			         if (wSX == this->pCurrentGame->swordsman.wX &&
                        wSY == this->pCurrentGame->swordsman.wY)
                  {
				         DrawMonster(pSeek, this->pRoom, pDestSurface, true);
                     break;   //only one mimic can be stabbing swordsman
                  }
		         }
		         pSeek = pSeek->pNext;
	         }
         }
		}
	}

	//6. Draw effects that go on top of everything else drawn in the room.
	this->pLastLayerEffects->DrawEffects();

	//Everything should have been (re)painted by now.
	//Undirty all the tiles.
	this->bAllDirty = false;
	pbMI = this->pTileInfo;
	TILEINFO *const pbMIStop = pbMI +
			CDrodBitmapManager::DISPLAY_ROWS * CDrodBitmapManager::DISPLAY_COLS;
	while (pbMI != pbMIStop)
		(pbMI++)->dirty = 0;

	//Paint widget children on top of everything.
	PaintChildren();

	//Put it up on the screen.
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CRoomWidget::PaintClipped(int /*nX*/, int /*nY*/, UINT /*wW*/, UINT /*wH*/,
		const bool /*bUpdateRect*/)
{
	//Paint() uses direct access to pixels, so it can't be clipped with
	//SDL_SetClipRect().  Either you need to write PaintClipped() or change
	//the situation which causes this widget to be clipped.
	ASSERTP(false, "Can't paint clipped.");
}

//*****************************************************************************
void CRoomWidget::RemoveLastLayerEffectsOfType(
//Removes all last-layer effects of given type in the room.
//
//Params:
	const UINT eEffectType)	//(in) Type of effect to remove.
{
	this->pLastLayerEffects->RemoveEffectsOfType(eEffectType);
}

//*****************************************************************************
void CRoomWidget::RemoveTLayerEffectsOfType(
//Removes all t-layer effects of given type in the room.
//
//Params:
	const UINT eEffectType)	//(in) Type of effect to remove.
{
	this->pTLayerEffects->RemoveEffectsOfType(eEffectType);
}

//*****************************************************************************
void CRoomWidget::GetSquareRect(
//Get rect on screen surface of a specified square.
//
//Params:
	UINT wCol, UINT wRow,	//(in)	Square to get rect for.
	SDL_Rect &SquareRect)	//(out)	Receives rect.
const
{
	ASSERT(IS_COLROW_IN_DISP(wCol, wRow));
	SET_RECT(SquareRect, this->x + (CX_TILE * wCol),
			this->y + (CY_TILE * wRow), CX_TILE,
			CY_TILE);
}

//*****************************************************************************
UINT CRoomWidget::SwitchAnimationFrame(
//Switch the monster animation frame at specified tile.
//
//Params:
	const UINT wCol, const UINT wRow)	//(in)	Room square.
{
	ASSERT(wCol < this->pRoom->wRoomCols);
	ASSERT(wRow < this->pRoom->wRoomRows);
	TILEINFO *const pTile = this->pTileInfo + ARRAYINDEX(wCol,wRow);
	return (UINT)(pTile->animFrame = 1 - pTile->animFrame);	//switch frame
}

//
//Protected methods.
//

//*****************************************************************************
CRoomWidget::~CRoomWidget()
//Destructor.  Use Unload() for cleanup.
{ 
	ASSERT(!this->bIsLoaded);
   delete this->pLastLayerEffects;
	delete this->pTLayerEffects;
}

//*****************************************************************************
bool CRoomWidget::Load()
//Load resources for CRoomWidget.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	ASSERT(!this->pRoomSnapshotSurface);
	ASSERT(!this->pBoltPartsSurface);

	if (this->pCurrentGame)
	{
		if (!LoadFromCurrentGame(this->pCurrentGame)) return false;
	}

	//To store unchanging room image.
	//It doesn't need to be this large, but it fixes some surface offset issues.
	//Only the area corresponding to the location of this widget will be used.
	this->pRoomSnapshotSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
		CScreen::CX_SCREEN, CScreen::CY_SCREEN, 24, 0, 0, 0, 0);
	if (!this->pRoomSnapshotSurface) return false;

	//Get bolts parts surface.
	this->pBoltPartsSurface = g_pTheBM->GetBitmapSurface("Bolts");
	if (!this->pBoltPartsSurface) return false;
	static Uint32 TransparentColor = SDL_MapRGB(this->pBoltPartsSurface->format, 
			192, 192, 192);
	SDL_SetColorKey(this->pBoltPartsSurface, SDL_SRCCOLORKEY, TransparentColor);

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//*****************************************************************************
void CRoomWidget::DeleteArrays()
//Deallocate arrays.
{
	delete[] this->pwTSquareTI;
	this->pwTSquareTI = NULL;
	
	delete[] this->pwOSquareTI;
	this->pwOSquareTI = NULL;

	delete[] this->pwMSquareTI;
	this->pwMSquareTI = NULL;

	delete[] this->pbEdges;
	this->pbEdges = NULL;

	delete[] this->pTileInfo;
	this->pTileInfo = NULL;
}

//*****************************************************************************
void CRoomWidget::Unload()
//Unload resource for CRoomWidget.
{
	UnloadChildren();

	ClearEffects(false);

	if (this->pRoomSnapshotSurface)
	{
		SDL_FreeSurface(this->pRoomSnapshotSurface);
		this->pRoomSnapshotSurface = NULL;
	}

	if (this->pBoltPartsSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Bolts");
		this->pBoltPartsSurface = NULL;
	}

	DeleteArrays();

	this->bIsLoaded = false;
}

//*****************************************************************************
void CRoomWidget::BoundsCheckRect(
//Performs bounds checking on a room rectangle.
//
//Params:
	int &wCol, int &wRow,			//(in/out)	Top-left tile.
	int &wWidth, int &wHeight)		//(in/out)	dimensions, in tiles
const
{
	ASSERT(wWidth > 0 && wHeight > 0);	//avoid wasteful calls
	ASSERT(wCol + wWidth > 0 && wRow + wHeight > 0);
	//Room dimensions.
	const int wRW = this->pRoom->wRoomCols;
	const int wRH = this->pRoom->wRoomRows;
	ASSERT(wCol < wRW && wRow < wRH);

	//Perform bounds checking.
	if (wCol < 0) {wWidth += wCol; wCol = 0;}
	if (wRow < 0) {wHeight += wRow; wRow = 0;}
	if (wCol + wWidth >= wRW) {wWidth = wRW - wCol;}
	if (wRow + wHeight >= wRH) {wHeight = wRH - wRow;}
}

//*****************************************************************************
void CRoomWidget::BAndWRect(
//Sets a rectangle of tiles in room to black-and-white.
//
//Params:
	SDL_Surface *pDestSurface,	//(in)	Surface to draw to.
	int wCol, int wRow,			//(in)	Top-left tile.
	int wWidth, int wHeight)	//(in)	dimensions, in tiles
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	g_pTheBM->BAndWRect(this->x + wCol * CX_TILE,
			this->y + wRow * CY_TILE,
			wWidth * CX_TILE, wHeight * CY_TILE,
			pDestSurface);
}
//*****************************************************************************
void CRoomWidget::DarkenRect(
//Darkens a rectangle of tiles in room by given percent.
//
//Params:
	SDL_Surface *pDestSurface,	//(in)	Surface to draw to.
	const float fLightPercent,	//(in)	% of color value to retain
	int wCol, int wRow,			//(in)	Top-left tile.
	int wWidth, int wHeight)	//(in)	dimensions, in tiles
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	g_pTheBM->DarkenRect(this->x + wCol * CX_TILE, this->y + wRow * CY_TILE,
			wWidth * CX_TILE, wHeight * CY_TILE, fLightPercent, pDestSurface);
}

//*****************************************************************************
void CRoomWidget::ShadeRect(
//Shades a rectangle of tiles in room with given color.
//
//Params:
	SDL_Surface *pDestSurface,	//(in)	Surface to draw to.
	const SURFACECOLOR Color,	//(in)	Color to shade with
	int wCol, int wRow,			//(in)	Top-left tile.
	int wWidth, int wHeight)	//(in)	dimensions, in tiles
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	g_pTheBM->ShadeRect(this->x + wCol * CX_TILE, this->y + wRow * CY_TILE,
			wWidth * CX_TILE, wHeight * CY_TILE, Color, pDestSurface);
}

//*****************************************************************************
void CRoomWidget::DirtyTileRect(
//Mark all tiles between these two points as dirty.
//
//Params:
	const int x1, const int y1, const int x2, const int y2)	//(in)
{
	//Find min/max coords.
	int xMin = min(x1,x2);
	int xMax = max(x1,x2);
	int yMin = min(y1,y2);
	int yMax = max(y1,y2);

	//Only dirty tiles in visible room area.
	if (xMin < 0) xMin = 0;
	if (yMin < 0) yMin = 0;
	if (xMax >= static_cast<int>(CDrodBitmapManager::DISPLAY_COLS)) xMax = CDrodBitmapManager::DISPLAY_COLS-1;
	if (yMax >= static_cast<int>(CDrodBitmapManager::DISPLAY_ROWS)) yMax = CDrodBitmapManager::DISPLAY_ROWS-1;

	this->pLastLayerEffects->DirtyTilesInRect(xMin,yMin,xMax,yMax);
}

//*****************************************************************************
void CRoomWidget::DirtyEffectTiles(
//Dirties all room tiles within the effects' area.
//
//Params:
	const bool bDrawMimicCursor,	//(in)
	const bool bInvisible,			//(in)
	const bool bMoveMade)			//(in)
{
	this->pLastLayerEffects->DirtyTiles();
	this->pTLayerEffects->DirtyTiles();
	if (bDrawMimicCursor)
		DrawMimicCursor(this->pCurrentGame->swordsman.wMimicCursorX,
				this->pCurrentGame->swordsman.wMimicCursorY, GetDestSurface(), true);
	if (bInvisible)
		DrawInvisibilityRange(GetDestSurface(), true);
	if (bMoveMade && this->pTLayerEffects->ContainsEffectOfType(ENEATHERHITORB))
	{
		//Removing this effect here dirties the old 'Neather tiles.
		this->pTLayerEffects->RemoveEffectsOfType(ENEATHERHITORB);
	}
}

//*****************************************************************************
void CRoomWidget::DirtySpriteTiles()
//Dirties room tiles at modified positions of player/monsters
//to synchronize room image with current game state.
{
	CCoordStack swordCoords;
	TILEINFO *pTInfo = this->pTileInfo;
	UINT *pwM = this->pwMSquareTI;
	CMonster *pMonster;
	bool bStateChanged, bRaised;
	bool bPositionChanged, bOrientationChanged = false;
	UINT wX, wY, wTile;
	UINT wPX, wPY;
	UINT wSX, wSY;

	//Process monsters.
	for (wY = 0; wY < CDrodBitmapManager::DISPLAY_ROWS; wY++)
		for (wX = 0; wX < CDrodBitmapManager::DISPLAY_COLS; wX++)
		{
			//Calculate image state for this tile.
			pTInfo->sword = 0;
			pMonster = this->pRoom->GetMonsterAtSquare(wX, wY);
			if (pMonster)
			{
				if (pMonster->wType == M_MIMIC)
				{
					CMimic *pMimic = DYN_CAST(CMimic*, CMonster*, pMonster);
					wSX = pMimic->GetSwordX();
					wSY = pMimic->GetSwordY();
					if (IS_COLROW_IN_DISP(wSX, wSY))
					{
						//Sword stats for square
						swordCoords.Push(wSX, wSY);
						pTInfo->sword = 1;
					}
				}
				wTile = GetTileImageForMonster(pMonster, pTInfo->animFrame);
			} else {
				wTile = 0;
			}

			//Dirty tile if current state differs from recorded value.
			if (*pwM != wTile)
			{
				//Update image state.  Dirty tile.
				*pwM = wTile;
				pTInfo->dirty = 1;
         }

         //Dirty tile above where a raised monster is/was.
			if (IS_COLROW_IN_DISP(wX,wY-1))
				this->pTileInfo[ARRAYINDEX(wX,wY-1)].dirty = 1;

			//Advance to next square.
			++pTInfo;
			++pwM;
		}

	if (this->bShowingSwordsman)
	{
		//Gather swordsman stats.
		wPX = this->pCurrentGame->swordsman.wX;
		wPY = this->pCurrentGame->swordsman.wY;
		bPositionChanged = (wPX != this->wLastX || wPY != this->wLastY);
		bOrientationChanged = this->pCurrentGame->swordsman.wO != this->wLastOrientation;

		//Redraw swordsman if he changed state.
		bRaised = DrawRaised(this->pRoom->GetOSquare(wPX, wPY));
		bStateChanged = (bPositionChanged || bOrientationChanged ||
				bRaised != this->bLastRaised || !this->pCurrentGame->swordsman.bIsVisible);
		if (bStateChanged)
		{
			//Dirty tile player is on.
			//Dirty tile above also if player was standing on something.
			this->pTileInfo[ARRAYINDEX(wPX,wPY)].dirty = 1;
			if (bRaised || (this->bLastRaised && !bPositionChanged) &&
					IS_COLROW_IN_DISP(wPX,wPY-1))
				this->pTileInfo[ARRAYINDEX(wPX,wPY-1)].dirty = 1;

			if (bPositionChanged)
			{
				//Player moved -- dirty tile he used to be on.
				this->pTileInfo[ARRAYINDEX(this->wLastX,this->wLastY)].dirty = 1;
				if (this->bLastRaised && IS_COLROW_IN_DISP(this->wLastX,this->wLastY-1))
					this->pTileInfo[ARRAYINDEX(this->wLastX,this->wLastY-1)].dirty = 1;
			}
		}
		//Swordsman's sword.
		wSX = this->pCurrentGame->swordsman.wSwordX;
		wSY = this->pCurrentGame->swordsman.wSwordY;
		if (IS_COLROW_IN_DISP(wSX, wSY))
		{
			swordCoords.Push(wSX, wSY);
			pTInfo = this->pTileInfo + ARRAYINDEX(wSX,wSY);
			pTInfo->sword = 1;
			if (!this->pCurrentGame->swordsman.bIsVisible)	//invisible -- always update
				this->pTileInfo[ARRAYINDEX(wSX,wSY)].dirty = 1;
		}
		//Player state should be synchronized now.
		this->wLastX = wPX;
		this->wLastY = wPY;
		this->wLastOrientation = this->pCurrentGame->swordsman.wO;
		this->bLastRaised = bRaised;
	}

	//Process swords (all are gathered now).
	//Should work even if two swords are ever on the same tile.
	//S1. Erase swords at old positions.
	if (bOrientationChanged)
	{
		//Swords changed direction.  Repaint all of them.
		while (this->lastSwordCoords.GetSize())
		{
			this->lastSwordCoords.Pop(wX,wY);
			this->pTileInfo[ARRAYINDEX(wX,wY)].dirty = 1;
			//Erase above it in case it was raised.
			if (IS_COLROW_IN_DISP(wX,wY-1))
				this->pTileInfo[ARRAYINDEX(wX,wY-1)].dirty = 1;
		}
	} else {
		//Repaint tiles that don't have swords in old position any more.
		while (this->lastSwordCoords.GetSize())
		{
			this->lastSwordCoords.Pop(wX,wY);
			pTInfo = this->pTileInfo + ARRAYINDEX(wX,wY);
			if (!pTInfo->sword)
			{
				//Sword not here any more -- dirty tile.
				pTInfo->dirty = 1;
				if (IS_COLROW_IN_DISP(wX,wY-1))
					this->pTileInfo[ARRAYINDEX(wX,wY-1)].dirty = 1;
			}
		}
	}
	//S2. Draw swords at new positions.
	while (swordCoords.GetSize())
	{
		swordCoords.Pop(wX,wY);
		//Only repaint if sword is no longer here.
		if (!this->lastSwordCoords.IsMember(wX,wY))
		{
			this->lastSwordCoords.Push(wX,wY);	//save for next turn
			this->pTileInfo[ARRAYINDEX(wX,wY)].dirty = 1;
			if (IS_COLROW_IN_DISP(wX,wY-1))
				this->pTileInfo[ARRAYINDEX(wX,wY-1)].dirty = 1;
		}
	}
}

//*****************************************************************************
void CRoomWidget::DrawMonsters(
//Draws monsters not on the player -- this case is handled separately.
//
//Params:
	CMonster *const pMonsterList,	//(in)	Monsters to draw.
   SDL_Surface *pDestSurface,
	const bool bActionIsFrozen)	//(in)	Whether action is currently stopped.
{
	CMonster *pMonster = pMonsterList;
	const UINT wPX = this->pCurrentGame->swordsman.wX;
	const UINT wPY = this->pCurrentGame->swordsman.wY;

	while (pMonster)
	{
		//Draw monster if it is not on player.
		if ((pMonster->wX != wPX || pMonster->wY != wPY))
			DrawMonster(pMonster, this->pRoom, pDestSurface, bActionIsFrozen);

		pMonster = pMonster->pNext;
	}
}

//*****************************************************************************
void CRoomWidget::AnimateMonsters()
//Randomly change monsters' animation frame.
{
	CMonster *pMonster = this->pRoom->pFirstMonster;
	UINT wX, wY;

	//Animate monsters in real time.
	const Uint32 dwNow=SDL_GetTicks();
	Uint32 dwTimeElapsed = dwNow - this->dwLastAnimationFrame;
	if (dwTimeElapsed==0)
		dwTimeElapsed=1;
	const Uint32 dwRandScalar=MONSTER_ANIMATION_DELAY * 1000 / dwTimeElapsed;

	while (pMonster)
	{
		if (RAND(dwRandScalar) == 0)
		{
			wX = pMonster->wX;	//shorthand
			wY = pMonster->wY;
			SwitchAnimationFrame(wX, wY);
			this->pTileInfo[ARRAYINDEX(wX, wY)].dirty = 1;
		}
		pMonster = pMonster->pNext;
	}
	this->dwLastAnimationFrame=dwNow;
}

//*****************************************************************************
void CRoomWidget::DrawMonster(
//Draws a monster.
//
//Params:
	CMonster *const pMonster,		//(in)	Monster to draw.
	CDbRoom *const pRoom,
	SDL_Surface *pDestSurface,
	const bool bActionIsFrozen)	//(in)	Whether action is currently stopped.
{
	bool bDrawRaised =
			DrawRaised(pRoom->GetOSquare(pMonster->wX, pMonster->wY));
	switch (pMonster->wType)
	{
	case M_MIMIC:
		DrawMimic(DYN_CAST(const CMimic*, CMonster *, pMonster), bDrawRaised, pDestSurface);
		break;
	case M_NEATHER:
		DrawNeather(DYN_CAST(CNeather*, CMonster *, pMonster), bDrawRaised, pDestSurface);
		break;
	case M_SERPENT:
		bDrawRaised = false;
		//when killing player, draw whole serpent here
		if (bActionIsFrozen && pMonster->wX == this->pCurrentGame->swordsman.wX &&
				pMonster->wY == this->pCurrentGame->swordsman.wY)
			DrawSerpent(pMonster,pDestSurface);
		//no break
	default:
		const UINT wTileImageNo = GetTileImageForMonster(pMonster,
				this->pTileInfo[ARRAYINDEX(pMonster->wX, pMonster->wY)].animFrame);
		DrawTileImage(pMonster->wX, pMonster->wY, wTileImageNo, bDrawRaised,
				pDestSurface);
		break;
	}
}

//*****************************************************************************
void CRoomWidget::DrawTileImage(
//Blits a tile graphic to a specified square of the room.
//
//Params:
		const UINT wCol, const UINT wRow,	//(in)	Destination square coords.
		const UINT wTileImageNo,			//(in)	Indicates which tile image to blit.
		const bool bDrawRaised,			//(in)	Draw tile raised above surface?
		SDL_Surface *pDestSurface,		//(in)	Surface to draw to.
		const Uint8 nOpacity)	//(in)	Tile opacity (0 = transparent, 255 = opaque).
{
	ASSERT(IS_COLROW_IN_DISP(wCol, wRow));
	ASSERT(wTileImageNo < TI_COUNT);
	ASSERT(wTileImageNo != TI_TEMPTY); //Wasteful to make this call for empty blit.

	const UINT wX = this->x + (CX_TILE * wCol);
	const UINT wY = this->y + (CY_TILE * wRow);
	if (bDrawRaised)
	{
		static const UINT CY_RAISED = 4;
		if (wRow == 0)
		{
			SDL_Rect ClipRect;
			GetRect(ClipRect);
			SDL_SetClipRect(pDestSurface, &ClipRect);
			g_pTheBM->BlitTileImage(wTileImageNo, wX, wY - CY_RAISED,
					pDestSurface,nOpacity, true);
			SDL_SetClipRect(pDestSurface, NULL);
		}
		else
		{
			g_pTheBM->BlitTileImage(wTileImageNo, wX, wY - CY_RAISED,
					pDestSurface,nOpacity);
		}
	}
	else
	{
		g_pTheBM->BlitTileImage(wTileImageNo, wX, wY,
				pDestSurface,nOpacity);
	}
}

//*****************************************************************************
void CRoomWidget::DrawBoltInRoom(
//Draws a lightning bolt between the given coords.
//
//Params:
	const int xS, const int yS, const int xC, const int yC)
{
	static SDL_Rect RoomRect;
	SDL_Surface *pDestSurface = GetDestSurface();
	GetRect(RoomRect);
	SDL_SetClipRect(pDestSurface, &RoomRect);
	DrawBolt(xS, yS, xC, yC, CDrodBitmapManager::DISPLAY_COLS,
			this->pBoltPartsSurface, pDestSurface);
	SDL_SetClipRect(pDestSurface, NULL);
}

//*****************************************************************************
void CRoomWidget::DrawInvisibilityRange(
//Show effective range of invisibility effect by highlighting area around
//swordsman.
//
//Params:
	SDL_Surface *pDestSurface,		//(in)	Surface to draw to.
	const bool bDirtyTilesOnly)	//(in) Whether to just mark which tiles
											//are affected or draw the effect.
{
	ASSERT(this->pCurrentGame);
	static const int numTiles = DEFAULT_SMELL_RANGE*2 + 1;
	const int x1 = this->pCurrentGame->swordsman.wX - DEFAULT_SMELL_RANGE;
	const int y1 = this->pCurrentGame->swordsman.wY - DEFAULT_SMELL_RANGE;
	const int x2 = this->pCurrentGame->swordsman.wX + DEFAULT_SMELL_RANGE;
	const int y2 = this->pCurrentGame->swordsman.wY + DEFAULT_SMELL_RANGE;
   static const float fShadingFactor = 0.80f;

	if (bDirtyTilesOnly)
	{
		DirtyTileRect(x1-1,y1-1,x2+1,y2+1);	//one tile buffer in case swordsman moved
	} else {
		if (this->bAllDirty)
			DarkenRect(pDestSurface, fShadingFactor, x1, y1, numTiles, numTiles);
		else
		{
			//Darken only dirty tiles.
			int wX, wY;
			for (wY = y1; wY <= y2; ++wY)
				for (wX = x1; wX <= x2; ++wX)
				{
					if (IS_COLROW_IN_DISP(wX,wY))
						if (this->bAllDirty || this->pTileInfo[ARRAYINDEX(
								(UINT)wX, (UINT)wY)].dirty)
							g_pTheBM->DarkenTile(this->x + wX * CX_TILE,
									this->y + wY * CY_TILE, fShadingFactor, pDestSurface);
				}
		}
	}
}

//*****************************************************************************
void CRoomWidget::DrawSwordsman(
//Draws swordsman along with his sword to game screen.
//
//Params:
	const CSwordsman &swordsman,	//(in) swordsman data
	SDL_Surface *pDestSurface)		//(in)	Surface to draw to.
{
	ASSERT(IsValidOrientation(swordsman.wO));
	ASSERT(IS_COLROW_IN_DISP(swordsman.wX, swordsman.wY));
	ASSERT(this->pRoom);

	static const UINT SMAN_TI[] = {TI_SMAN_YNW, TI_SMAN_YN, TI_SMAN_YNE,
			TI_SMAN_YW, TI_TEMPTY, TI_SMAN_YE,
			TI_SMAN_YSW, TI_SMAN_YS, TI_SMAN_YSE};
	static const UINT SMANI_TI[] = {TI_SMAN_IYNW, TI_SMAN_IYN, TI_SMAN_IYNE,
			TI_SMAN_IYW, TI_TEMPTY, TI_SMAN_IYE,
			TI_SMAN_IYSW, TI_SMAN_IYS, TI_SMAN_IYSE};
	static const UINT SWORD_TI[] = {TI_SWORD_YNW, TI_SWORD_YN, TI_SWORD_YNE,
			TI_SWORD_YW, TI_TEMPTY, TI_SWORD_YE,
			TI_SWORD_YSW, TI_SWORD_YS, TI_SWORD_YSE};
	static const UINT SWORDI_TI[] = {TI_SWORD_IYNW, TI_SWORD_IYN, TI_SWORD_IYNE,
			TI_SWORD_IYW, TI_TEMPTY, TI_SWORD_IYE,
			TI_SWORD_IYSW, TI_SWORD_IYS, TI_SWORD_IYSE};

	//Get tile image for swordsman and sword from visible or invisible array.
	static Uint8 nOpacity = 255;
	UINT wSManTI, wSwordTI;
	if (swordsman.bIsVisible)
	{
		wSManTI = SMAN_TI[swordsman.wO];
		wSwordTI = SWORD_TI[swordsman.wO];
		nOpacity = 255;
	} else {
		DrawInvisibilityRange(pDestSurface);

		//Show invisible swordsman fading in and out.
		static bool bFadeIn = false;
		if (this->pRoom->SomeMonsterCanSmellSwordsman())
		{
			wSManTI = SMAN_TI[swordsman.wO];	//show Beethro solid when sensed
			wSwordTI = SWORD_TI[swordsman.wO];
		} else {
			wSManTI = SMANI_TI[swordsman.wO];
			wSwordTI = SWORDI_TI[swordsman.wO];
		}

		//Fade in and out.
		if (nOpacity < 50)	//don't make too faint
			bFadeIn = true;
		else if (nOpacity > 160)	//don't make too dark
			bFadeIn = false;
		if (bFadeIn)
			nOpacity += 2;
		else
			nOpacity -= 8;
	}

	//Draw swordsman raised above floor?
	const bool bDrawRaised = DrawRaised(this->pRoom->GetOSquare(swordsman.wX,
			swordsman.wY));

	//Blit the swordsman.
	if (this->bAllDirty || this->pTileInfo[
			ARRAYINDEX(swordsman.wX,swordsman.wY)].dirty ||
			(bDrawRaised && swordsman.wY > 0 &&
					this->pTileInfo[ARRAYINDEX(swordsman.wX, swordsman.wY-1)].dirty))
		DrawTileImage(swordsman.wX, swordsman.wY, wSManTI, bDrawRaised,
				pDestSurface, nOpacity);

	//See if sword square within display.
	if (IS_COLROW_IN_DISP(swordsman.wSwordX, swordsman.wSwordY))
		if (this->bAllDirty || this->pTileInfo[ARRAYINDEX(swordsman.wSwordX,
				swordsman.wSwordY)].dirty ||
				(bDrawRaised && swordsman.wSwordY > 0 &&
				this->pTileInfo[ARRAYINDEX(swordsman.wSwordX,
						swordsman.wSwordY-1)].dirty))
			DrawTileImage(swordsman.wSwordX, swordsman.wSwordY, wSwordTI,
					bDrawRaised, pDestSurface, nOpacity);
}

//*****************************************************************************
void CRoomWidget::DrawMimic(
//Draws mimic along with his sword to game screen.
//
//Params:
	const CMimic *pMimic,		//(in)	Pointer to CMimic monster.
	const bool bDrawRaised,		//(in)	Draw mimic raised above floor?
	SDL_Surface *pDestSurface,	//(in)	Surface to draw to.
	const Uint8 nOpacity)		//(in)	Opacity of mimic.
{
	ASSERT(IsValidOrientation(pMimic->wO));
	ASSERT(IS_COLROW_IN_DISP(pMimic->wX, pMimic->wY));

	static const UINT MIMIC_TI[] = {TI_MIMIC_NW, TI_MIMIC_N, TI_MIMIC_NE,
			TI_MIMIC_W, TI_TEMPTY, TI_MIMIC_E,
			TI_MIMIC_SW, TI_MIMIC_S, TI_MIMIC_SE};
	static const UINT MIMIC_SWORD_TI[] = {
			TI_MIMIC_SWORD_NW, TI_MIMIC_SWORD_N, TI_MIMIC_SWORD_NE,
			TI_MIMIC_SWORD_W, TI_TEMPTY, TI_MIMIC_SWORD_E,
			TI_MIMIC_SWORD_SW, TI_MIMIC_SWORD_S, TI_MIMIC_SWORD_SE};

	//Get tile image from array.
	const UINT wMimicTI = MIMIC_TI[pMimic->wO];

	//Blit the mimic.
	DrawTileImage(pMimic->wX, pMimic->wY, wMimicTI, bDrawRaised, pDestSurface,
		nOpacity);

	//See if mimic sword square within display.
	const UINT wMSwordX = pMimic->GetSwordX(), wMSwordY = pMimic->GetSwordY();
	if (IS_COLROW_IN_DISP(wMSwordX, wMSwordY))
	{
		//Get mimic sword tile image from array.
		const UINT wSwordTI = MIMIC_SWORD_TI[pMimic->wO];
		DrawTileImage(wMSwordX, wMSwordY, wSwordTI, bDrawRaised, pDestSurface,
			nOpacity);
 	}
}

//*****************************************************************************
void CRoomWidget::DrawMimicCursor(
//Draws a mimic cursor to a specified square of the room.
//
//Params:
	const UINT wCol, const UINT wRow,		//(in)	Destination square coords.
	SDL_Surface *pDestSurface,	//(in)	Surface to draw to.
	const bool bDirtyTilesOnly)	//(in) Just mark which tiles are affected.
{
	ASSERT(IS_COLROW_IN_DISP(wCol, wRow));
	ASSERT(this->pCurrentGame);

	const int xPixel = this->x + wCol * CX_TILE;
	const int yPixel = this->y + wRow * CY_TILE;
	const bool bObstacle = this->pRoom->
			DoesSquareContainMimicPlacementObstacle(wCol, wRow);

	//Draw cursor.
	if (!bDirtyTilesOnly)
	{
		//Show illegal placement tile.
		if (bObstacle)
      {
         static const SURFACECOLOR Red = {255, 0, 0};
			g_pTheBM->ShadeTile(xPixel,yPixel,Red,GetDestSurface());
      } else {
			//Fade in and out.
			static Uint8 nOpacity = 160;
			static bool bFadeIn = false;
			if (nOpacity < 50)	//don't make too faint
				bFadeIn = true;
			else if (nOpacity > 160)	//don't make too dark
				bFadeIn = false;
			if (bFadeIn)
				nOpacity += 6;
			else
				nOpacity -= 4;

			CMimic Mimic; //This mimic is disconnected from the current game.
			Mimic.wO = this->pCurrentGame->swordsman.wO;
			Mimic.wX = wCol;
			Mimic.wY = wRow;
			DrawMimic(&Mimic, true, pDestSurface, nOpacity);
		}
	}

	//Draw bolt from swordsman to cursor.
	static const UINT CX_TILE_HALF = CX_TILE / 2;
	static const UINT CY_TILE_HALF = CY_TILE / 2;
	const UINT wSX = this->pCurrentGame->swordsman.wX;
	const UINT wSY = this->pCurrentGame->swordsman.wY;
	const int xS = this->x + wSX * CX_TILE + CX_TILE_HALF;
	const int yS = this->y + wSY * CY_TILE + CY_TILE_HALF;
	const int xC = xPixel + CX_TILE_HALF;
	const int yC = yPixel + CY_TILE_HALF;

	//Dirty tiles where cursor was and is now.
	if (bDirtyTilesOnly)
   {
	   DirtyTileRect(wCol-2,wRow-3,wCol+2,wRow+2);	//cursor, ~two tile buffer

      //Determine bolt bounding box.
      int xMin = min(wSX,wCol) - CX_TILE;	//provide some buffer for random bolt
	   int xMax = max(wSX,wCol) + CX_TILE;
	   int yMin = min(wSY,wRow) - CY_TILE;
	   int yMax = max(wSY,wRow) + CY_TILE;
		//Make buffer of sufficient size for all cases.
		while (xMax - xMin < 6 * CX_TILE)
		{
			xMin -= CX_TILE;
			xMax += CX_TILE;
		}
		while (yMax - yMin < 6 * CY_TILE)
		{
			yMin -= CY_TILE;
			yMax += CY_TILE;
		}
	   DirtyTileRect(xMin,yMin,xMax,yMax);	//bolt
   } else {
		DrawBoltInRoom(xS, yS, xC, yC);
   }
}

//*****************************************************************************
void CRoomWidget::DrawNeather(
//Draws Neather (possibly along with his hammer banging an orb).
//
//Params:
	CNeather *pNeather,			//(in)	Pointer to CNeather monster.
	const bool bDrawRaised,		//(in)	Draw Neather raised above floor?
	SDL_Surface *pDestSurface)	//(in)	Surface to draw to.
{
	//If Neather is banging an orb with his hammer, use an effect to draw it.
	if (pNeather->bStrikingOrb)
	{
		if (this->bAddNEffect)
		{
			this->bAddNEffect = false;	//don't allow adding effect twice
			CCoord coord(pNeather->wX,pNeather->wY);
			CNeatherStrikesOrbEffect *pNE = new CNeatherStrikesOrbEffect(this,coord);
			this->pTLayerEffects->AddEffect(pNE);
		}
	} else {
		this->bAddNEffect = true;
		this->pTLayerEffects->RemoveEffectsOfType(ENEATHERHITORB);
	}

	if (!this->pTLayerEffects->ContainsEffectOfType(ENEATHERHITORB))
	{
		//Draw Neather normally.
		DrawTileImage(pNeather->wX, pNeather->wY,
				GetTileImageForMonster(pNeather,0), bDrawRaised, pDestSurface);
	}
}

//*****************************************************************************************
void CRoomWidget::DrawSerpent(
// Starting after head, traverse and draw room tiles to the tail.
// Assumes a valid serpent.
//
//Params:
	CMonster *pMonster,			//(in)	Pointer to CSerpent monster.
	SDL_Surface *pDestSurface)	//(in)	Surface to draw to.
{
	ASSERT(pMonster->wType == M_SERPENT);
	ASSERT(this->pRoom);
	UINT wX = pMonster->wX, wY = pMonster->wY;
	int dx = -(int)nGetOX(pMonster->wO);
	int dy = -(int)nGetOY(pMonster->wO);
	UINT tile;
	int t;
	while (true)
	{
		//Draw each tile along serpent.
		ASSERT ((dx == 0) != (dy == 0));	//no diagonals
		wX += dx;
		wY += dy;
		tile = this->pRoom->GetTSquare(wX, wY);
		ASSERT(bIsSerpent(tile));
		DrawTileImage(wX, wY, GetTileImageForTileNo(tile), false, pDestSurface);
		//Go to next piece.
		switch (tile) 
		{
			case T_SNK_EW: case T_SNK_NS: break;
			case T_SNK_NW: case T_SNK_SE: t = dx; dx = -dy; dy = -t; break;
			case T_SNK_NE: case T_SNK_SW: t = dx; dx = dy; dy = t; break;
			case T_SNKT_S: case T_SNKT_W: case T_SNKT_N: case T_SNKT_E:
				//tail tiles -- done
				return;
			default: ASSERTP(false, "Bad tile#.");	return;
		}
	}
}

//*****************************************************************************
bool CRoomWidget::UpdateDrawSquareInfo()
//Update square drawing information arrays for a room.
//
//Returns:
//True if successful, false if not (out of memory error).
{
	const DWORD dwSquareCount = this->pRoom->CalcRoomArea();

	//If the square count changed, then I need to realloc arrays.
	if (dwSquareCount != this->dwLastDrawSquareInfoUpdateCount)
	{
		this->dwLastDrawSquareInfoUpdateCount = dwSquareCount;

		//Delete existing arrays.
		DeleteArrays();

		//Allocate new tile image arrays.
		this->pwOSquareTI = new UINT[dwSquareCount];
		this->pwTSquareTI = new UINT[dwSquareCount];
		this->pwMSquareTI = new UINT[dwSquareCount];
		this->pbEdges = new EDGES[dwSquareCount];
		this->pTileInfo = new TILEINFO[dwSquareCount];
		if (!(this->pwOSquareTI && this->pwTSquareTI && this->pwMSquareTI && this->pbEdges))
		{
			DeleteArrays();
			return false;
		}

		memset(this->pwMSquareTI, 0, dwSquareCount * sizeof(UINT));	//init
		this->bAllDirty = true;
	}

	//Set tile image elements of arrays.
	UINT *pwO = this->pwOSquareTI;
	UINT *pwT = this->pwTSquareTI;
	EDGES *pbE = this->pbEdges;
	TILEINFO *pbMI = this->pTileInfo;
	const char *pucO = this->pRoom->pszOSquares;
	const char *pucT = this->pRoom->pszTSquares;
	const UINT wRows = this->pRoom->wRoomRows, wCols = this->pRoom->wRoomCols;
	UINT wTileImage;
	bool drawEdge;

	//Clear all old tile information.
	memset(this->pTileInfo, 0, dwSquareCount * sizeof(TILEINFO));

	for (UINT wRow = 0; wRow < wRows; ++wRow)
	{
		for (UINT wCol = 0; wCol < wCols; ++wCol)
		{
			//Calculate edges.
			//If existance of an edge changes, tile must be redrawn.
			drawEdge = (wRow > 0 ? CalcEdge(*pucO,	*(pucO - wCols),N) : false);
			if (drawEdge != pbE->drawNorthEdge)
			{
				pbMI->dirty = 1;
				pbE->drawNorthEdge = drawEdge;
			}

			drawEdge = (wCol > 0 ? CalcEdge(*pucO,*(pucO - 1),W) : false);
			if (drawEdge != pbE->drawWestEdge)
			{
				pbMI->dirty = 1;
				pbE->drawWestEdge = drawEdge;
			}
			drawEdge = (wCol < wCols-1 ? CalcEdge(*pucO,*(pucO + 1),E) : false);

			if (drawEdge != pbE->drawEastEdge)
			{
				pbMI->dirty = 1;
				pbE->drawEastEdge = drawEdge;
			}

			drawEdge = (wRow < wRows-1 ? CalcEdge(*pucO,*(pucO + wCols),S) : false);
			if (drawEdge != pbE->drawSouthEdge)
			{
				pbMI->dirty = 1;
				pbE->drawSouthEdge = drawEdge;
			}

			//Calculate o-layer tiles
			//If tile changes, it must be redrawn.
			wTileImage = GetTileImageForTileNo(*pucO);
			if (wTileImage == CALC_NEEDED)
				wTileImage = CalcTileImageForOSquare(this->pRoom, wCol, wRow);
			if (wTileImage != *pwO)
			{
				pbMI->dirty = 1;
				*pwO = wTileImage;
			}

			//Calculate t-layer tiles
			//If tile changes, it must be redrawn.
			wTileImage = GetTileImageForTileNo(*pucT);
			if (wTileImage == CALC_NEEDED)
				wTileImage = CalcTileImageForTSquare(this->pRoom, wCol, wRow);
			if (wTileImage != *pwT)
			{
				pbMI->dirty = 1;
				*pwT = wTileImage;
			}

			//Give each m-layer tile a random animation frame
			pbMI->animFrame = RAND(2);
			pbMI->raised = DrawRaised(this->pRoom->GetOSquare(wCol, wRow));

			//Advance to next square.
			++pbE;
			++pwO;
			++pucO;
			++pwT;
			++pucT;
			++pbMI;
		}
	}

	//I'm expecting pointers to have traversed entire size of their arrays--
	//no more, no less.
	ASSERT(pbE == this->pbEdges + dwSquareCount);
	ASSERT(pwT == this->pwTSquareTI + dwSquareCount);
	ASSERT(pwO == this->pwOSquareTI + dwSquareCount);
	ASSERT(pbMI == this->pTileInfo + dwSquareCount);

	//Reflect these changes.
	Repaint(this->wShowCol, this->wShowRow);

	return true;
}

//Much of RoomWidget.cpp's code was cut-and-pasted from GameScreen.cpp, and contains 
//contributions outside of those contained in erikh2000's commits.  See history 
//of GameScreen.cpp if interested.

// $Log: RoomWidget.cpp,v $
// Revision 1.119  2004/05/20 17:44:51  mrimer
// Fixed bug: raised sprites not being erased completely when lowered.
//
// Revision 1.118  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.117  2003/08/16 01:54:18  mrimer
// Moved EffectList to FrontEndLib.  Added general tool tip support to screens.  Added RoomEffectList to room editor screen/widget.
//
// Revision 1.116  2003/08/07 16:10:17  mrimer
// Fixed bug: mimic killing player not being drawn correctly.
//
// Revision 1.115  2003/07/22 19:00:26  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.114  2003/07/19 21:23:28  mrimer
// Made invisibility range graphic more pronounced.
//
// Revision 1.113  2003/07/19 02:18:38  mrimer
// Implemented toggling swordsman invisibility effect.
//
// Revision 1.112  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.111  2003/07/15 00:31:42  mrimer
// Fixed sword display bug (removed raisedSword field in room stats).
//
// Revision 1.110  2003/07/09 21:12:31  mrimer
// Fixed bug: raised sprites on top row drawing off top of room widget.
//
// Revision 1.109  2003/07/09 11:29:40  schik
// Fixed warnings on VS.NET
//
// Revision 1.108  2003/07/09 05:01:26  schik
// Fixed memory leak
//
// Revision 1.107  2003/07/03 21:44:51  mrimer
// Port warning/bug fixes (committed on behalf of Gerry JJ).
//
// Revision 1.106  2003/07/03 08:09:12  mrimer
// Fixed crashing bug (dangling pRoom pointer when level changes).
//
// Revision 1.105  2003/07/01 20:27:16  mrimer
// Fixed a display bug.
//
// Revision 1.104  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.103  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.102  2003/06/21 10:41:33  mrimer
// Fixed more item placement logic.
//
// Revision 1.101  2003/06/21 06:54:00  mrimer
// Completed door agent designation.  Revised some door traversal code and effects.
//
// Revision 1.100  2003/06/21 04:10:59  schik
// Orbs affecting a door are now colored according to the toggle/open/close action
//
// Revision 1.99  2003/06/20 23:54:03  mrimer
// Fixed a bug (strike orb effect).
//
// Revision 1.98  2003/06/19 22:22:17  mrimer
// Fixed bug: yellow doors of different state shouldn't be adjacent.
// Added overwriting orientable objects of the same object type.
//
// Revision 1.97  2003/06/19 04:09:57  mrimer
// Fixed some placement logic bugs.
//
// Revision 1.96  2003/06/18 03:38:46  mrimer
// Refined editor item placement validity logic.
//
// Revision 1.95  2003/06/17 18:22:33  mrimer
// Removed safety placement checks.
//
// Revision 1.94  2003/06/16 21:54:31  mrimer
// Improved editor look and feel: tar and orb agent effects made transparent.
//
// Revision 1.93  2003/06/16 20:03:22  mrimer
// Fixed bug: unintentional orb agent deletions.
// Added tile to monster menu to delete only monsters.
//
// Revision 1.92  2003/06/14 23:56:04  mrimer
// Fixed bug: monsters disappear when mimic is being placed.
//
// Revision 1.91  2003/06/09 23:53:45  mrimer
// Added ResetRoom().
//
// Revision 1.90  2003/06/09 21:47:21  erikh2000
// Fixed a bug with walking down the stairs.
//
// Revision 1.89  2003/06/09 19:30:27  mrimer
// Fixed some level editor bugs.
//
// Revision 1.88  2003/06/06 18:19:50  mrimer
// Fixed a crash on screen size change.
//
// Revision 1.87  2003/05/29 20:47:45  mrimer
// Fixed bug: long mimic cursor bolt trails not being erased.
//
// Revision 1.86  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.85  2003/05/23 21:43:04  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.84  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.83  2003/05/21 03:08:06  mrimer
// Fixed another warning.
//
// Revision 1.82  2003/05/19 20:40:30  erikh2000
// Fixes for warnings.
//
// Revision 1.81  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.80  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.79  2003/04/23 00:00:44  mrimer
// Fixed some potential display bugs if the widget size ever changes.
//
// Revision 1.78  2003/04/17 21:05:43  mrimer
// Fixed two display bugs.
//
// Revision 1.77  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.76  2003/01/04 23:09:57  mrimer
// Updated swordsman interface.
//
// Revision 1.75  2002/12/22 02:32:13  mrimer
// Added ShowRoomTransition() to pan between rooms.
// Revised swordsman vars.  Simplified DrawSwordsman() params.
//
// Revision 1.74  2002/11/22 22:03:44  mrimer
// Fixed tar placement safety logic.
//
// Revision 1.73  2002/11/22 02:30:36  mrimer
// Added AddOrbAgentToolTip() and AddToolTipEffect().
// Fixed IsSafePlacement logic, and a door highlighting bug.
//
// Revision 1.72  2002/11/18 17:33:54  mrimer
// Converted animation from frame-based to time-based.
//
// Revision 1.71  2002/11/15 03:06:42  mrimer
// Added support for the room editor, including: event handlers, effect methods, and interface vars.
// Added LoadFromRoom().  Made widget not require a CCurrentGame to display a CDbRoom.
// Moved some macros to the .h file.
//
// Revision 1.70  2002/10/21 20:25:35  mrimer
// Fixed swordsman drawing bug.
//
// Revision 1.69  2002/10/14 21:20:34  mrimer
// Fixed bug: top of mimic cursor sword not erased when pointing up.
//
// Revision 1.68  2002/10/14 17:51:19  mrimer
// Fixed bug: mimic sword not refreshing.
//
// Revision 1.67  2002/10/14 17:24:18  mrimer
// Fixed paint bugs when drawing raised monsters.
// Moved dirtyTiles into TILEINFO.
// Renamed pbMSquareInfo to pTileInfo.
// Made DrawRaised() const.
//
// Revision 1.66  2002/10/11 18:30:05  mrimer
// Added AnimateMonsters().
//
// Revision 1.65  2002/10/11 17:38:30  mrimer
// Fixed some repainting errors.
//
// Revision 1.64  2002/10/11 07:47:10  erikh2000
// Fixed problem with checkpoints not being shown in room widget display.
//
// Revision 1.63  2002/10/10 21:43:23  mrimer
// Added BlitLayeredTileImage() to draw o- and t-layer tiles simultaneously.
//
// Revision 1.61  2002/10/07 18:27:37  mrimer
// Optimized room pre-rendering (added dirtyTiles and bAllDirty) in Repaint() to only draw tiles that have changed.
//
// Revision 1.60  2002/10/03 21:49:49  mrimer
// Now allow mimic placement and player death to occur simultaneously.
//
// Revision 1.59  2002/10/03 21:04:52  mrimer
// Added DrawSerpent() and code to show whole serpent when it kills player.
//
// Revision 1.58  2002/10/03 19:07:31  mrimer
// Further revised Paint() and Repaint() to handle freezing the room action and better show the death fade.
// Added DrawMonster().
// Made pRoomSnapshotWidget full screen size to remove tile placement inconsistencies.
// Removed optional offset to rect effects.
//
// Revision 1.57  2002/10/03 02:48:52  erikh2000
// Put invisibility square back to how it was.
//
// Revision 1.56  2002/10/01 22:53:55  erikh2000
// Made the square around invisible swordsman smaller by one to more accurately show which monsters can smell him.
//
// Revision 1.55  2002/10/01 16:26:54  mrimer
// Added DrawMonsters() and DrawRaised().
//
// Revision 1.54  2002/09/30 18:58:30  mrimer
// Made invisibility range effect more subtle.
//
// Revision 1.53  2002/09/30 18:40:59  mrimer
// Added DarkenRect(), BAndWRect(), and BoundsCheckRect().
// Enhanced mimic placement effect to show action has stopped during placement.
// Renamed DrawHighlightRect() to ShadeRect().
//
// Revision 1.52  2002/09/27 21:52:14  mrimer
// Sped up room repaint by initially storing an image of the room to minimize recalculation.
//
// Revision 1.51  2002/09/27 17:45:55  mrimer
// Added DrawHighlightRect().  Added visual cue for range of invisibility effect.
// Enhanced appearance of mimic placement.
//
// Revision 1.50  2002/09/24 22:19:54  mrimer
// Enhanced illegal mimic placement location effect.
//
// Revision 1.49  2002/09/24 21:36:47  mrimer
// Added opacity parameter to DrawTileImage().
// Enhanced swordsman invisibility effect.
// Fixed strange mimic placement drawing bug.
// Made some parameters and vars const.
//
// Revision 1.48  2002/09/14 21:38:47  mrimer
// Made RemoveTLayerEffectsOfType() public.
//
// Revision 1.47  2002/09/14 21:21:42  mrimer
// Moved effect list code into CEffectList.
// Added 'Neather orb hit animation.
// Fixed bug: hiding frame rate cancels other last-layer effects.
//
// Revision 1.46  2002/09/05 18:23:09  erikh2000
// Effects are cleared whenever the room widget is loaded from a new game.
//
// Revision 1.45  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.44  2002/08/30 23:59:42  erikh2000
// Added frame rate display.
//
// Revision 1.43  2002/08/25 19:01:04  erikh2000
// Added code that puts effects into list sorted by their draw sequence.
//
// Revision 1.42  2002/07/22 17:33:28  mrimer
// Added DrawNeather().
//
// Revision 1.41  2002/07/20 23:17:38  erikh2000
// Removed all code related to scaling from class.
//
// Revision 1.40  2002/07/05 10:37:55  erikh2000
// Unrolled a loop in the scaling code for better performance.
//
// Revision 1.39  2002/06/22 05:59:34  erikh2000
// Changed code to use new CMimic sword coord methods.
//
// Revision 1.38  2002/06/21 22:33:18  mrimer
// Optimized scaled down room calculation/drawing.
//
// Revision 1.37  2002/06/21 05:20:26  mrimer
// Revised includes.
//
// Revision 1.36  2002/06/20 00:55:54  erikh2000
// If widget doesn't have a current game set, it will draw a placeholder rect.
//
// Revision 1.35  2002/06/16 22:16:41  erikh2000
// If widget is clipped, an assertian will now fire.
// Widget can now be set to show checkpoints or not show them.
//
// Revision 1.34  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.33  2002/06/15 18:33:57  erikh2000
// Paint() exits early if current game is not set.
//
// Revision 1.32  2002/06/14 01:00:52  erikh2000
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
//
// Revision 1.31  2002/06/11 22:54:25  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.30  2002/06/05 03:13:26  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.29  2002/05/24 23:41:45  erikh2000
// Put outline-drawing inside screen surface lock/unlock.
//
// Revision 1.28  2002/05/24 23:18:57  mrimer
// Refined edge drawing next to walls/pits.
//
// Revision 1.27  2002/05/24 15:26:35  mrimer
// Add outlines to walls and pit-adjacent floor squares.
//
// Revision 1.26  2002/05/21 21:37:11  erikh2000
// Changed a col/row check to use a macro for consistency.
//
// Revision 1.25  2002/05/17 23:06:33  erikh2000
// Changed logic so that square-drawing arrays are not deleted and alloced so often.
//
// Revision 1.24  2002/05/17 01:12:16  erikh2000
// Removed some debug checks on the anim frame array.
//
// Revision 1.23  2002/05/17 00:35:26  erikh2000
// Set animation on monsters to happen twice as frequently.
// Put in temporary assertian checks on the animation frames.
//
// Revision 1.22  2002/05/16 19:16:33  mrimer
// Updated death sequence: improved monster animation.
//
// Revision 1.21  2002/05/16 18:22:04  mrimer
// Added player death animation.
//
// Revision 1.20  2002/05/15 23:43:28  mrimer
// Completed exit level sequence.
//
// Revision 1.19  2002/05/15 14:29:53  mrimer
// Moved animation data out of DRODLIB (CMonster) and into DROD (CRoomWidget).
//
// Revision 1.18  2002/05/15 01:27:34  erikh2000
// Corrected an assert so that it wouldn't fire at the wrong time.
//
// Revision 1.17  2002/05/14 19:06:16  mrimer
// Added monster animation.
//
// Revision 1.16  2002/05/12 03:19:41  erikh2000
// Added method to return screen rect of one room square.
//
// Revision 1.15  2002/05/10 22:40:22  erikh2000
// RoomWidget now draws an *anti-aliased* scaled image.
//
// Revision 1.14  2002/04/29 00:18:26  erikh2000
// When monsters, mimics, or swordsman are standing on top of walls/doors, they will be drawn slightly raised.
//
// Revision 1.13  2002/04/25 10:48:43  erikh2000
// Room widget will now draw a scaled down room.
//
// Revision 1.12  2002/04/25 09:30:14  erikh2000
// Added stub to draw a black rectangle when widget area is less than needed for full-scale display.
//
// Revision 1.11  2002/04/23 03:11:34  erikh2000
// Checkpoints are special-cased to be drawn on top of floor tiles in the opaque-layer drawing loop.
//
// Revision 1.10  2002/04/22 09:43:41  erikh2000
// Mimic cursor is now depicted with a bolt from swordman to cursor.
//
// Revision 1.9  2002/04/22 09:41:17  erikh2000
// Initial check-in.
//
// Revision 1.8  2002/04/22 02:54:17  erikh2000
// RoomWidget now loads a parts bitmap for displaying bolts.
//
// Revision 1.7  2002/04/19 21:43:49  erikh2000
// Removed references to ScreenConstants.h.
//
// Revision 1.6  2002/04/13 19:44:24  erikh2000
// Added new type parameter to CWidget construction call.
//
// Revision 1.5  2002/04/12 22:54:25  erikh2000
// Fixed widget so it would paint its children.
//
// Revision 1.4  2002/04/12 05:18:18  erikh2000
// Wrote code to handle effects.
//
// Revision 1.3  2002/04/11 10:17:11  erikh2000
// Changed bitmap loading for tiles to use CBitmapManager.
//
// Revision 1.2  2002/04/09 10:05:39  erikh2000
// Fixed revision log macro.
//
