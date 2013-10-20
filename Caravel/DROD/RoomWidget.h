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

#ifndef ROOMWIDGET_H
#define ROOMWIDGET_H
#ifdef WIN32
#	pragma warning(disable:4786)
#endif

#include "DrodBitmapManager.h"
#include <FrontEndLib/Widget.h>
#include "../DRODLib/Swordsman.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/DbRooms.h"
#include <BackEndLib/Coord.h>
#include <BackEndLib/CoordStack.h>
#include <BackEndLib/Types.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//Converts 2-D room coords into 1-D for array access
#define ARRAYINDEX(x,y) (((y) * (this->pRoom->wRoomCols)) + (x))

//Whether to draw edge on tile.
struct EDGES {
	EDGES() :
		drawNorthEdge(false),
		drawWestEdge(false),
		drawSouthEdge(false),
		drawEastEdge(false)
	{}

	bool drawNorthEdge, drawWestEdge, drawSouthEdge, drawEastEdge;
};

//Info for repainting tiles.
typedef struct {
	BYTE dirty : 1;	//tile is dirty and needs to be repainted
	BYTE animFrame : 1;	//animation frame # of monster here
	BYTE raised : 1;	//o-layer (and object on it) is raised up
	BYTE sword : 1;	//there is a sword here
} TILEINFO;

//******************************************************************************
class CCurrentGame;
class CEffect;
class CMimic;
class CNeather;
class CRoomEffectList;
class CRoomWidget : public CWidget
{
friend class CRoomEffectList;	//to access dirtyTiles

public:
	CRoomWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, 
			UINT wSetH, const CCurrentGame *pSetCurrentGame);

	void				AddLastLayerEffect(CEffect *pEffect);
	void				AddShadeEffect(const UINT wX, const UINT wY,
			const SURFACECOLOR &Color);
	void				AddStrikeOrbEffect(const COrbData &SetOrbData, bool bDrawOrb = true);
	void				AddTLayerEffect(CEffect *pEffect);
	bool				AreCheckpointsVisible() const {return this->bShowCheckpoints;}
	void				ClearEffects(const bool bKeepFrameRate = true);
	void				DirtyRoom() {this->bAllDirty = true;}
	void				GetSquareRect(UINT wCol, UINT wRow, SDL_Rect &SquareRect) const;
	UINT				SwitchAnimationFrame(const UINT wCol, const UINT wRow);
	UINT*				GetMonsterTile(const UINT wCol, const UINT wRow);
	void				HideCheckpoints() {this->bShowCheckpoints = false;}
	void				HideFrameRate();
	void				HideSwordsman() {this->bShowingSwordsman = false;}
	bool				LoadFromCurrentGame(const CCurrentGame *pSetCurrentGame);
	virtual void	Paint(bool bUpdateRect = true);
	virtual void	PaintClipped(const int nX, const int nY, const UINT wW,
         const UINT wH, const bool bUpdateRect = true);
	void				RemoveLastLayerEffectsOfType(const UINT eEffectType);
	void				RemoveTLayerEffectsOfType(const UINT eEffectType);
	virtual void	Repaint(int wCol=0, int wRow=0,
			int wWidth=CDrodBitmapManager::DISPLAY_COLS, int wHeight=CDrodBitmapManager::DISPLAY_ROWS);
	void				ResetForPaint();
   void           ResetRoom() {this->pRoom = NULL;}
	void				ShowCheckpoints() {this->bShowCheckpoints = true;}
	void				ShowRoomTransition(const UINT wExitOrientation);
	void				ShowSwordsman() {this->bShowingSwordsman = true;}
	void				ShowFrameRate();
	void				ToggleFrameRate();
	void				UpdateFromCurrentGame();
	void				UpdateFromPlots();

	const CCurrentGame * GetCurrentGame() {return pCurrentGame;}

	SDL_Surface *	pRoomSnapshotSurface;	//image of the pre-rendered room

protected:
	virtual	~CRoomWidget();

	virtual void	HandleAnimate() {if (pRoom) Paint(true);}
	virtual bool	IsAnimated() const {return true;}
	virtual bool   Load();
	virtual void   Unload();

	void				AnimateMonsters();
	void				BoundsCheckRect(int &wCol, int &wRow,
			int &wWidth, int &wHeight) const;
	void				BAndWRect(SDL_Surface *pDestSurface,
			int wCol=0, int wRow=0,
			int wWidth=CDrodBitmapManager::DISPLAY_COLS,
			int wHeight=CDrodBitmapManager::DISPLAY_ROWS);
	void				BlitDirtyRoomTiles();
	void				DarkenRect(SDL_Surface *pDestSurface,
			const float fLightPercent,	int wCol=0,	int wRow=0,
			int wWidth=CDrodBitmapManager::DISPLAY_COLS,
			int wHeight=CDrodBitmapManager::DISPLAY_ROWS);
	void				ShadeRect(SDL_Surface *pDestSurface,
			const SURFACECOLOR Color, int wCol=0, int wRow=0,
			int wWidth=CDrodBitmapManager::DISPLAY_COLS,
			int wHeight=CDrodBitmapManager::DISPLAY_ROWS);
	void				DeleteArrays();
	void				DirtyEffectTiles(const bool bDrawMimicCursor,
			const bool bInvisible, const bool bMoveMade);
	void				DirtySpriteTiles();
	void				DirtyTileRect(const int x1, const int y1,
			const int x2, const int y2);
	void				DrawBoltInRoom(const int xS, const int yS, const int xC,
			const int yC);
	void				DrawInvisibilityRange(SDL_Surface *pDestSurface,
			const bool bDirtyTilesOnly=false);
	void				DrawMimic(const CMimic *pMimic, const bool bDrawRaised,
			SDL_Surface *pDestSurface, const Uint8 nOpacity=255);
	void				DrawMimicCursor(const UINT wCol, const UINT wRow,
			SDL_Surface *pDestSurface, const bool bDirtyTilesOnly=false);
	void				DrawMonster(CMonster *const pMonster,
			CDbRoom *const pRoom, SDL_Surface *pDestSurface,
			const bool bActionIsFrozen);
	virtual void	DrawMonsters(CMonster *const pMonsterList,
			SDL_Surface *pDestSurface, const bool bActionIsFrozen);
	void				DrawNeather(CNeather *pNeather, const bool bDrawRaised,
			SDL_Surface *pDestSurface);
	void				DrawSerpent(CMonster *pMonster, SDL_Surface *pDestSurface);
	bool				DrawRaised(const UINT wTileNo) const {
		return (wTileNo == T_DOOR_Y || wTileNo == T_DOOR_M ||
				wTileNo == T_DOOR_R || wTileNo == T_DOOR_C ||
				wTileNo == T_WALL ||	wTileNo == T_WALL_B);}
	void				DrawSwordsman(const CSwordsman &swordsman,
			SDL_Surface *pDestSurface);
	void		DrawTileEdges(const UINT wX, const UINT wY,
			const EDGES *pbE, SDL_Surface *pDestSurface);
	void				DrawTileImage(const UINT wCol, const UINT wRow,
			const UINT wTileImageNo, const bool bDrawRaised,
			SDL_Surface *pDestSurface, const Uint8 nOpacity=255);
	bool				UpdateDrawSquareInfo();

	DWORD					dwRoomX, dwRoomY;
	UINT					wStyle;
	UINT					wShowCol, wShowRow;

	const CCurrentGame *	pCurrentGame;	//to show room of a game in progress
	CDbRoom *				pRoom;			//to show room in initial state
	UINT *					pwOSquareTI;	//o-layer tiles
	UINT *					pwTSquareTI;	//t-layer
	UINT *					pwMSquareTI;	//m-layer
	EDGES *					pbEdges;

	CRoomEffectList *		pLastLayerEffects;
	CRoomEffectList *		pTLayerEffects;

	SDL_Surface *			pBoltPartsSurface;

	bool					bShowingSwordsman;	//whether swordsman is visible onscreen
	bool					bShowCheckpoints;
	bool					bShowFrameRate;
	bool					bAddNEffect;	//for 'Neather striking orb

	DWORD					dwLastDrawSquareInfoUpdateCount;
	Uint32				dwLastAnimationFrame;

	//vars for optimization in rendering the room
	bool					bAllDirty;	//all room tiles must be redrawn
	bool					bWasPlacingMimic;	//was placing mimic last frame
   bool              bWasInvisible;    //swordsman was invisible last frame
	UINT					wLastTurn;	//turn # at last frame
	TILEINFO *			pTileInfo;	//info about blits in each room tile
	UINT					wLastOrientation;	//direction swords were pointing last turn
	UINT					wLastX, wLastY;	//position of player last turn
	bool					bLastRaised;		//was player raised last turn

	CCoordStack 		lastSwordCoords;

   int					CX_TILE, CY_TILE;
};

#endif //#ifndef ROOMWIDGET_H

// $Log: RoomWidget.h,v $
// Revision 1.73  2003/08/16 01:54:19  mrimer
// Moved EffectList to FrontEndLib.  Added general tool tip support to screens.  Added RoomEffectList to room editor screen/widget.
//
// Revision 1.72  2003/07/19 02:18:39  mrimer
// Implemented toggling swordsman invisibility effect.
//
// Revision 1.71  2003/07/15 00:31:42  mrimer
// Fixed sword display bug (removed raisedSword field in room stats).
//
// Revision 1.70  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.69  2003/07/09 11:29:40  schik
// Fixed warnings on VS.NET
//
// Revision 1.68  2003/07/09 05:02:31  schik
// Fixed unitialized vars
//
// Revision 1.67  2003/07/03 21:43:36  mrimer
// Added ResetRoom() and changed HandleAnimate() to only call Paint() when room is not NULL.
//
// Revision 1.66  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.65  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.64  2003/06/21 06:54:01  mrimer
// Completed door agent designation.  Revised some door traversal code and effects.
//
// Revision 1.63  2003/06/21 04:10:59  schik
// Orbs affecting a door are now colored according to the toggle/open/close action
//
// Revision 1.62  2003/06/19 22:22:17  mrimer
// Fixed bug: yellow doors of different state shouldn't be adjacent.
// Added overwriting orientable objects of the same object type.
//
// Revision 1.61  2003/06/17 18:22:33  mrimer
// Removed safety placement checks.
//
// Revision 1.60  2003/06/16 20:27:04  mrimer
// Added T_NOMONSTER, used only in the room editor.
//
// Revision 1.59  2003/06/14 23:56:04  mrimer
// Fixed bug: monsters disappear when mimic is being placed.
//
// Revision 1.58  2003/06/09 23:53:46  mrimer
// Added ResetRoom().
//
// Revision 1.57  2003/06/09 19:30:29  mrimer
// Fixed some level editor bugs.
//
// Revision 1.56  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.55  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.54  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.53  2003/04/23 00:00:44  mrimer
// Fixed some potential display bugs if the widget size ever changes.
//
// Revision 1.52  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.51  2003/01/08 00:53:17  mrimer
// Changed includes.
//
// Revision 1.50  2002/12/22 02:20:59  mrimer
// Added panning transition effect between rooms.  Revised DrawSwordsman params.
//
// Revision 1.49  2002/11/22 02:14:34  mrimer
// Added AddOrbAgentToolTip() and AddToolTipEffect().
//
// Revision 1.48  2002/11/18 18:34:05  mrimer
// Added support for time-based, not frame-based, animation.
//
// Revision 1.47  2002/11/15 02:31:42  mrimer
// Added room editor support, including: event handlers, effect routines, a struct for placing long monsters, and interface member vars.
// Moved macros from .cpp file.
//
// Revision 1.46  2002/10/23 20:58:26  erikh2000
// Added an access method to retrieve visibility of checkpoints.
//
// Revision 1.45  2002/10/14 17:24:18  mrimer
// Fixed paint bugs when drawing raised monsters.
// Moved dirtyTiles into TILEINFO.
// Renamed pbMSquareInfo to pTileInfo.
// Made DrawRaised() const.
//
// Revision 1.44  2002/10/11 18:30:31  mrimer
// Added AnimateMonsters().
//
// Revision 1.43  2002/10/10 21:06:52  mrimer
// Optimized room drawing code.
// Added DirtyRoom(), SwitchAnimationFrame(), DeleteArrays(), DirtyEffectTiles(), DirtySpriteTiles(), DirtyTileRect(), DrawInvisibilityRange(), DrawTileEdges().
// Added some vars for drawing optimization.
// Changed pwAnimFrames to pbMSquareInfo and pwMSquareTI.
// Removed an unneeded overloaded Repaint().
//
// Revision 1.42  2002/10/07 18:27:37  mrimer
// Optimized room pre-rendering (added dirtyTiles and bAllDirty) in Repaint() to only draw tiles that have changed.
//
// Revision 1.41  2002/10/03 21:07:07  mrimer
// Added DrawSerpent().
//
// Revision 1.40  2002/10/03 19:07:31  mrimer
// Further revised Paint() and Repaint() to handle freezing the room action and better show the death fade.
// Added DrawMonster().
// Made pRoomSnapshotWidget full screen size to remove tile placement inconsistencies.
// Removed optional offset to rect effects.
//
// Revision 1.39  2002/10/02 21:38:56  mrimer
// Made pRoomSnapshotSurface public (for new death fade effect).
//
// Revision 1.38  2002/10/01 16:26:55  mrimer
// Added DrawMonsters() and DrawRaised().
//
// Revision 1.37  2002/09/30 18:40:59  mrimer
// Added DarkenRect(), BAndWRect(), and BoundsCheckRect().
// Enhanced mimic placement effect to show action has stopped during placement.
// Renamed DrawHighlightRect() to ShadeRect().
//
// Revision 1.36  2002/09/27 21:52:14  mrimer
// Sped up room repaint by initially storing an image of the room to minimize recalculation.
//
// Revision 1.35  2002/09/27 17:45:55  mrimer
// Added DrawHighlightRect().  Added visual cue for range of invisibility effect.
// Enhanced appearance of mimic placement.
//
// Revision 1.34  2002/09/24 21:27:23  mrimer
// Added opacity value to DrawTileImage().
// Made some parameters const.
//
// Revision 1.33  2002/09/14 21:38:49  mrimer
// Made RemoveTLayerEffectsOfType() public.
//
// Revision 1.32  2002/09/14 21:21:42  mrimer
// Moved effect list code into CEffectList.
// Added 'Neather orb hit animation.
// Fixed bug: hiding frame rate cancels other last-layer effects.
//
// Revision 1.31  2002/08/31 00:16:37  erikh2000
// Removed visibility check from HandleAnimate() because CEventHandlerWidget now performs the same check.
//
// Revision 1.30  2002/08/30 23:59:42  erikh2000
// Added frame rate display.
//
// Revision 1.29  2002/08/30 19:59:58  mrimer
// Made HandleAnimate() check for widget being visible before painting.
//
// Revision 1.28  2002/08/25 19:01:04  erikh2000
// Added code that puts effects into list sorted by their draw sequence.
//
// Revision 1.27  2002/07/22 17:33:31  mrimer
// Added DrawNeather().
//
// Revision 1.26  2002/07/22 02:48:52  erikh2000
// Made face and room widgets animated.
//
// Revision 1.25  2002/07/22 01:00:29  erikh2000
// Made destructor declaration virtual just so that the virtual status is more obvious.
//
// Revision 1.24  2002/07/20 23:17:38  erikh2000
// Removed all code related to scaling from class.
//
// Revision 1.23  2002/06/21 04:57:09  mrimer
// Revised includes.
//
// Revision 1.22  2002/06/16 22:16:41  erikh2000
// If widget is clipped, an assertian will now fire.
// Widget can now be set to show checkpoints or not show them.
//
// Revision 1.21  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.20  2002/05/24 23:18:57  mrimer
// Refined edge drawing next to walls/pits.
//
// Revision 1.19  2002/05/24 15:26:34  mrimer
// Add outlines to walls and pit-adjacent floor squares.
//
// Revision 1.18  2002/05/17 23:06:33  erikh2000
// Changed logic so that square-drawing arrays are not deleted and alloced so often.
//
// Revision 1.17  2002/05/17 00:35:27  erikh2000
// Set animation on monsters to happen twice as frequently.
// Put in temporary assertian checks on the animation frames.
//
// Revision 1.16  2002/05/16 18:22:04  mrimer
// Added player death animation.
//
// Revision 1.15  2002/05/15 23:43:28  mrimer
// Completed exit level sequence.
//
// Revision 1.14  2002/05/15 14:29:53  mrimer
// Moved animation data out of DRODLIB (CMonster) and into DROD (CRoomWidget).
//
// Revision 1.13  2002/05/12 03:19:41  erikh2000
// Added method to return screen rect of one room square.
//
// Revision 1.12  2002/04/29 00:18:26  erikh2000
// When monsters, mimics, or swordsman are standing on top of walls/doors, they will be drawn slightly raised.
//
// Revision 1.11  2002/04/25 10:48:44  erikh2000
// Room widget will now draw a scaled down room.
//
// Revision 1.10  2002/04/25 09:30:14  erikh2000
// Added stub to draw a black rectangle when widget area is less than needed for full-scale display.
//
// Revision 1.9  2002/04/22 21:49:51  mrimer
// Added query function GetCurrentGame() to access pCurrentGame.
//
// Revision 1.8  2002/04/22 09:43:41  erikh2000
// Mimic cursor is now depicted with a bolt from swordman to cursor.
//
// Revision 1.7  2002/04/22 09:41:17  erikh2000
// Initial check-in.
//
// Revision 1.6  2002/04/22 02:54:17  erikh2000
// RoomWidget now loads a parts bitmap for displaying bolts.
//
// Revision 1.5  2002/04/19 22:01:00  erikh2000
// Some constants from ScreenConstants.h moved here.
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
