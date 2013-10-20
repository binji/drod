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
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996, 
 * 1997, 2000, 2001, 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "EditRoomScreen.h"
#include "EditSelectScreen.h"
#include "DrodBitmapManager.h"
#include "DrodEffect.h"
#include "DrodSound.h"
#include "Browser.h"
#include "GameScreen.h"
#include "LevelSelectDialogWidget.h"
#include "MapWidget.h"
#include "RoomWidget.h"
#include "TileImageCalcs.h"
#include <FrontEndLib/ListBoxWidget.h>
#include <FrontEndLib/OptionButtonWidget.h>
#include <FrontEndLib/ObjectMenuWidget.h>
#include <FrontEndLib/TabbedMenuWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ButtonWidget.h>

#include "../DRODLib/TileConstants.h"
#include "../DRODLib/Serpent.h"
#include "../DRODLib/Db.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/IDList.h>

#include <set>

const DWORD TAG_MENU = 1010;
const DWORD TAG_OMENU = 1011;
const DWORD TAG_TMENU = 1012;
const DWORD TAG_MMENU = 1013;

const DWORD TAG_UNDO = 1020;
const DWORD TAG_REDO = 1021;

const DWORD TAG_SHOWERRORS = 1030;

//O-layer objects.
const UINT OMenuDisplayTiles[][4] =
{
	{TI_EMPTY_L,TI_EMPTY_D,TI_EMPTY_D,TI_EMPTY_L},		//T_FLOOR
	{TI_TRAPDOOR,TI_TRAPDOOR,TI_TRAPDOOR,TI_TRAPDOOR},	//T_TRAPDOOR
	{TI_WALL,TI_WALL,TI_WALL_S,TI_WALL_S},					//T_WALL
	{TI_WALL_B,TI_WALL_B,TI_WALL_BS,TI_WALL_BS},			//T_WALL_B
	{TI_PIT_LS,TI_PIT_DS,TI_SPIKE,TI_SPIKE_D},			//T_PIT
	{TI_STAIRS_3,TI_STAIRS_2,TI_STAIRS_4,TI_STAIRS_2},	//T_STAIRS
	{TI_DOOR_YSE,TI_DOOR_YSW,TI_DOOR_YNE,TI_DOOR_YNW},	//T_DOOR_Y
	{TI_DOOR_YO,TI_DOOR_YO,TI_DOOR_YO,TI_DOOR_YO},		//T_DOOR_YO
	{TI_DOOR_CSE,TI_DOOR_CSW,TI_DOOR_CNE,TI_DOOR_CNW},	//T_DOOR_C
	{TI_DOOR_MSE,TI_DOOR_MSW,TI_DOOR_MNE,TI_DOOR_MNW},	//T_DOOR_M
	{TI_DOOR_RSE,TI_DOOR_RSW,TI_DOOR_RNE,TI_DOOR_RNW},	//T_DOOR_R
	{TI_OB_NW,TI_OB_NE,TI_OB_SW,TI_OB_SE},					//T_OB_1
	{TI_CHECKPOINT_L}												//T_CHECKPOINT
};

//T-layer objects.
const UINT TMenuDisplayTiles[][4] =
{
	{TI_ORB_D},
	{TI_ARROW_NW},
	{TI_POTION_I},
	{TI_POTION_K},
	{TI_TAR_NW,TI_TAR_NE,TI_TAR_SW,TI_TAR_SE},
	{TI_SCROLL}
};

//M-layer objects.
const UINT MMenuDisplayTiles[][4] =
{
	{TI_SMAN_YSE, TI_EMPTY_D, TI_EMPTY_D, TI_SWORD_YSE},
	{TI_ROACH_SE},
	{TI_QROACH_SE},
	{TI_EYE_SE},
	{TI_WW_SE},
	{TI_TARBABY},
	{TI_SNKT_W,TI_SNK_EW,TI_SNK_E},
	{TI_TAREYE_WO,TI_TAREYE_EO},
	{TI_SPIDER_SE},
	{TI_GOBLIN_SE},
	{TI_BRAIN},
};

//Specifies whether the given object is only allowed to be placed one at a
//time.
const bool SinglePlacement[TOTAL_TILE_COUNT+2] = 
{
	0,	//T_EMPTY         0
	0,	//T_FLOOR         1
	0,	//T_PIT           2
	0,	//T_STAIRS        3
	0,	//T_WALL          4
	0,	//T_WALL_B        5
	0,	//T_DOOR_C        6
	0,	//T_DOOR_M        7
	0,	//T_DOOR_R        8
	0,	//T_DOOR_Y        9
	0,	//T_DOOR_YO       10
	0,	//T_TRAPDOOR      11
	1,	//T_OB_1          12
	0,	//T_ARROW_N       13
	0,	//T_ARROW_NE      14
	0,	//T_ARROW_E       15
	0,	//T_ARROW_SE      16
	0,	//T_ARROW_S       17
	0,	//T_ARROW_SW      18
	0,	//T_ARROW_W       19
	0,	//T_ARROW_NW      20
	0,	//T_POTION_I      21
	0,	//T_POTION_K      22
	1,	//T_SCROLL        23
	0,	//T_ORB           24
	0,	//T_SNK_EW        25
	0,	//T_SNK_NS        26
	0,	//T_SNK_NW        27
	0,	//T_SNK_NE        28    
	0,	//T_SNK_SW        29
	0,	//T_SNK_SE        30
	1,	//T_SNKT_S        31
	1,	//T_SNKT_W        32
	1,	//T_SNKT_N        33
	1,	//T_SNKT_E        34
	0,	//T_TAR           35
	1,	//T_CHECKPOINT    36
	0,	//T_ROACH			+0
	0,	//T_QROACH			+1
	0,	//M_REGG				+2
	0,	//M_GOBLIN			+3
	1,	//M_NEATHER			+4
	0,	//M_WWING			+5
	0,	//M_EYE				+6
	1,	//M_SERPENT			+7
	1,	//M_TARMOTHER		+8
	0,	//M_TARBABY			+9
	0,	//M_BRAIN			+10
	1,	//M_MIMIC			+11
	0,	//M_SPIDER			+12
	1,	//T_SWORDSMAN
	0	//T_NOMONSTER
};

//
//Public methods.
//

//*****************************************************************************
bool CEditRoomScreen::SetRoom(
//Instantiates level and room members by room ID.
//These instances are deleted during call to Unload() upon destruction.
//
//Returns: whether room was successfully loaded
//
//Params:
	const DWORD dwRoomID)	//(in) ID of room to load.
{
	delete this->pRoom;
	this->pRoom = g_pTheDB->Rooms.GetByID(dwRoomID);
   if (!this->pRoom) return false;

   this->pRoomWidget->LoadFromRoom(this->pRoom);

	delete this->pLevel;
	this->pLevel = this->pRoom->GetLevel();
	this->pMapWidget->LoadFromLevel(this->pLevel);
	this->pMapWidget->SelectRoom(this->pRoom->dwRoomX, this->pRoom->dwRoomY);

	//Reset.
	ClearList(this->RedoList);
	ClearList(this->UndoList);
   SetButtons(false);

	SetSignTextToCurrentRoom();

   return true;
}

//
//Protected methods.
//

//*****************************************************************************
CEditRoomScreen::CEditRoomScreen() : CRoomScreen(SCR_EditRoom)
	, pLevel(NULL)
	, pRoom(NULL)
	, pTabbedMenu(NULL)

	, wSelectedObjectSave(static_cast<UINT>(-1))

	, eState(ES_PLACING)
	, pLongMonster(NULL)
	, pOrb(NULL)
	, bShowErrors(true)
	, bAutoSave(true)
	, bRoomDirty(false)
   , nUndoSize(0)

   , wTestX(static_cast<UINT>(-1)), wTestY(static_cast<UINT>(-1))
   , wTestO(static_cast<UINT>(-1))
   , wO(SE)

   , wCopyX1(static_cast<UINT>(-1)), wCopyY1(static_cast<UINT>(-1))
   , wCopyX2(static_cast<UINT>(-1)), wCopyY2(static_cast<UINT>(-1))
   , bAreaJustCopied(false), bReadyToPaste(false)

   , dwSavePlayerID(0), dwTestPlayerID(0)
{
}

//*****************************************************************************
bool CEditRoomScreen::Load()
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	static const int X_ROOM = 102;
	static const int Y_ROOM = 25;
	static const UINT CX_SPACE = 8;
	static const UINT CY_SPACE = 8;
	static const int X_OBJECTMENU = 4;
	static const int Y_OBJECTMENU = 3;
	static const UINT CX_OBJECTMENU = 93;
	static const UINT CY_OBJECTMENU = 300;
	static const int X_INNERMENU = X_OBJECTMENU + 4;
	static const int Y_INNERMENU = 30;
	static const UINT CX_INNERMENU = CX_OBJECTMENU - 8;
	static const int X_UNDO = CX_SPACE;
	static const int Y_UNDO = Y_OBJECTMENU + CY_OBJECTMENU + CY_SPACE;
   static const UINT CX_UNDO = 39;
	static const int X_REDO = X_UNDO + CX_UNDO + CX_SPACE;
	static const int Y_REDO = Y_UNDO;
   static const UINT CX_REDO = CX_UNDO;
	static const int X_SHOWERRORS = CX_SPACE;
	static const int Y_SHOWERRORS = Y_UNDO + CY_STANDARD_BUTTON;

	ASSERT(!this->bIsLoaded);

	if (!CRoomScreen::Load(NULL))
		return false;
	this->pMapWidget->Enable();

	//Add widgets.
	this->pRoomWidget = new CEditRoomWidget(TAG_ROOM, X_ROOM, Y_ROOM,
			CDrodBitmapManager::CX_ROOM, CDrodBitmapManager::CY_ROOM);
	AddWidget(this->pRoomWidget);

	//Object menu.
	this->pTabbedMenu = new CTabbedMenuWidget(TAG_MENU, X_OBJECTMENU,
			Y_OBJECTMENU, CX_OBJECTMENU, CY_OBJECTMENU, 3);
	this->pTabbedMenu->SetTabTile(0, TI_WALL);
	this->pTabbedMenu->SetTabTile(1, TI_ORB_D);
	this->pTabbedMenu->SetTabTile(2, TI_ROACH_SE);
	AddWidget(this->pTabbedMenu);

	//Add object menus to tabbed menu
	CObjectMenuWidget *pObjectMenu;
	pObjectMenu = new CObjectMenuWidget(TAG_OMENU, X_INNERMENU, Y_INNERMENU,
			CX_INNERMENU, 275, 7, 7, TI_EMPTY_L);
	pObjectMenu->AddObject(T_WALL, 2, 2, OMenuDisplayTiles[2]);
	pObjectMenu->AddObject(T_FLOOR, 2, 2, OMenuDisplayTiles[0]);
	pObjectMenu->AddObject(T_WALL_B, 2, 2, OMenuDisplayTiles[3]);
	pObjectMenu->AddObject(T_TRAPDOOR, 2, 2, OMenuDisplayTiles[1]);
	pObjectMenu->AddObject(T_PIT, 2, 2, OMenuDisplayTiles[4]);
	pObjectMenu->AddObject(T_STAIRS, 2, 2, OMenuDisplayTiles[5]);
	pObjectMenu->AddObject(T_DOOR_Y, 2, 2, OMenuDisplayTiles[6]);
	pObjectMenu->AddObject(T_DOOR_YO, 2, 2, OMenuDisplayTiles[7]);
	pObjectMenu->AddObject(T_DOOR_C, 2, 2, OMenuDisplayTiles[8]);
	pObjectMenu->AddObject(T_DOOR_M, 2, 2, OMenuDisplayTiles[9]);
	pObjectMenu->AddObject(T_DOOR_R, 2, 2, OMenuDisplayTiles[10]);
	pObjectMenu->AddObject(T_OB_1, 2, 2, OMenuDisplayTiles[11]);
	pObjectMenu->AddObject(T_CHECKPOINT, 1, 1, OMenuDisplayTiles[12]);
	this->pTabbedMenu->AddWidgetToTab(pObjectMenu,0);

	pObjectMenu = new CObjectMenuWidget(TAG_TMENU, X_INNERMENU, Y_INNERMENU,
			CX_INNERMENU, 275, 7, 7, TI_EMPTY_L);
	pObjectMenu->AddObject(T_ORB, 1, 1, TMenuDisplayTiles[0]);
	pObjectMenu->AddObject(T_ARROW_NW, 1, 1, TMenuDisplayTiles[1]);
	pObjectMenu->AddObject(T_SCROLL, 1, 1, TMenuDisplayTiles[5]);
	pObjectMenu->AddObject(T_EMPTY, 1, 1, OMenuDisplayTiles[0]);
	pObjectMenu->AddObject(T_TAR, 2, 2, TMenuDisplayTiles[4]);
	pObjectMenu->AddObject(T_POTION_I, 1, 1, TMenuDisplayTiles[2]);
	pObjectMenu->AddObject(T_POTION_K, 1, 1, TMenuDisplayTiles[3]);
	this->pTabbedMenu->AddWidgetToTab(pObjectMenu,1);

	pObjectMenu = new CObjectMenuWidget(TAG_MMENU, X_INNERMENU, Y_INNERMENU,
			CX_INNERMENU, 275, 7, 7, TI_EMPTY_L);
	pObjectMenu->AddObject(T_ROACH, 1, 1, MMenuDisplayTiles[1]);
	pObjectMenu->AddObject(T_QROACH, 1, 1, MMenuDisplayTiles[2]);
	pObjectMenu->AddObject(T_EYE, 1, 1, MMenuDisplayTiles[3]);
	pObjectMenu->AddObject(T_NOMONSTER, 1, 1, OMenuDisplayTiles[0]);
	pObjectMenu->AddObject(T_TARBABY, 1, 1, MMenuDisplayTiles[5]);
	pObjectMenu->AddObject(T_WWING, 1, 1, MMenuDisplayTiles[4]);
	pObjectMenu->AddObject(T_SPIDER, 1, 1, MMenuDisplayTiles[8]);
	pObjectMenu->AddObject(T_GOBLIN, 1, 1, MMenuDisplayTiles[9]);
	pObjectMenu->AddObject(T_TARMOTHER, 2, 1, MMenuDisplayTiles[7]);
	pObjectMenu->AddObject(T_SERPENT, 3, 1, MMenuDisplayTiles[6]);
	pObjectMenu->AddObject(T_SWORDSMAN, 2, 2, MMenuDisplayTiles[0]);
	pObjectMenu->AddObject(T_BRAIN, 1, 1, MMenuDisplayTiles[10]);
	this->pTabbedMenu->AddWidgetToTab(pObjectMenu,2);

	this->wSelectedObject = T_WALL;	//first object is active

   //Undo/redo buttons.
   CButtonWidget *pButton = new CButtonWidget(TAG_UNDO, X_UNDO, Y_UNDO,
         CX_UNDO, CY_STANDARD_BUTTON, g_pTheDB->GetMessageText(MID_Undo));
   AddWidget(pButton);
   pButton = new CButtonWidget(TAG_REDO, X_REDO, Y_REDO,
         CX_REDO, CY_STANDARD_BUTTON, g_pTheDB->GetMessageText(MID_Redo));
   AddWidget(pButton);

	//Option settings.
	AddWidget(new COptionButtonWidget(TAG_SHOWERRORS, X_SHOWERRORS, Y_SHOWERRORS,
			CX_OBJECTMENU, CY_STANDARD_OPTIONBUTTON,
			g_pTheDB->GetMessageText(MID_ShowErrors), this->bShowErrors, true));

	//Level list dialog box.
	this->pLevelBox = new CLevelSelectDialogWidget(0L);
	AddWidget(this->pLevelBox);
	this->pLevelBox->Center();
	this->pLevelBox->Hide();

	//Load children widgets.
	this->bIsLoaded = LoadChildren();

	return this->bIsLoaded;
}

//*****************************************************************************
void CEditRoomScreen::Unload()
//Unload resources for screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	CRoomScreen::Unload();

	delete this->pLevel;
	this->pLevel = NULL;

	delete this->pRoom;
	this->pRoom = NULL;

	ClearList(this->UndoList);
	ClearList(this->RedoList);

	this->bIsLoaded = false;
}

//*****************************************************************************
bool CEditRoomScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
   //Set room style.
	ASSERT(this->pRoom);
   this->pRoom->Reload();	//make sure everything's fresh
	this->bRoomDirty = false;
	g_pTheDBM->LoadTileImagesForStyle(this->pRoom->wStyle);
	CRoomScreen::SetMusicStyle(this->pRoom->wStyle);

   //If returning from play-testing, delete the temp player.
   if (this->dwTestPlayerID)
   {
      ASSERT(this->dwSavePlayerID);
      g_pTheDB->Players.Delete(this->dwTestPlayerID, false);
      g_pTheDB->SetPlayerID(this->dwSavePlayerID);
      this->dwSavePlayerID = this->dwTestPlayerID = 0;
   }
   this->nUndoSize = this->UndoList.size();

	//Init the keysym-to-command map and load player editor settings.
	ApplyPlayerSettings();

	SelectFirstWidget(false);

	return true;
}

//
//Private methods.
//

//*****************************************************************************
void CEditRoomScreen::AddPlotEffect(
//Returns: tile #'s to display specified object
//
//Params:
	const UINT wObjectNo)
{
	//Determine size of object to show.
	UINT wCX, wCY;
	switch (wObjectNo)
	{
		case T_OB_1:
			wCX = wCY = 2;
			break;
		case T_STAIRS:
			wCX = wCY = 3;	//at least 3x3
			break;
		case T_TARMOTHER:
			wCX = 2;  wCY = 1;
			break;
		default:
			wCX = wCY = 1;
			break;
	}

	this->pRoomWidget->AddPendingPlotEffect(wObjectNo,
			DisplaySelection(wObjectNo), wCX, wCY, SinglePlacement[wObjectNo]);
}

//*****************************************************************************
void CEditRoomScreen::ClearList(
//Clears a list of room pointers, deleting all room objects pointed to.
//
//Params:
	list<CDbRoom*> &List)
{
	for (list<CDbRoom*>::iterator iter=List.begin();
			iter!=List.end(); ++iter)
		delete *iter;
	List.clear();
}

//*****************************************************************************
const UINT* CEditRoomScreen::GetTileImageForMonsterType(
//Gets a (pointer to a) tile image to display for a monster.
//
//Returns:
//(pointer to a) TI_* constant.
//
//Params:
	const UINT wType,				//(in)	Contains data needed to figure out
	const UINT wO,					//(in)	which array to get tile from
	const UINT wAnimFrame)		//(in)	tile image.
const
{
	ASSERT(IsValidMonsterType(wType));
	ASSERT(IsValidOrientation(wO));
	ASSERT(MonsterTileImageArray[wType][wO] != DONT_USE);

	return (wAnimFrame == 0 ?
			MonsterTileImageArray[wType]+wO :
			AnimatedMonsterTileImageArray[wType]+wO);
}


//*****************************************************************************
const UINT* CEditRoomScreen::DisplaySelection(
//Returns: tile #'s to display specified object
//
//Params:
	const UINT wObjectNo)
const
{
	//Swordsman image.
	static const UINT SMAN_TI[] = {TI_SMAN_YNW, TI_SMAN_YN, TI_SMAN_YNE, 
			TI_SMAN_YW, TI_TEMPTY, TI_SMAN_YE,
			TI_SMAN_YSW, TI_SMAN_YS, TI_SMAN_YSE};

	//Force arrow images.
	static const UINT ARROW_TI[] = {TI_ARROW_NW, TI_ARROW_N, TI_ARROW_NE,
		TI_ARROW_W, TI_TEMPTY, TI_ARROW_E, TI_ARROW_SW, TI_ARROW_S, TI_ARROW_SE};

	//Images not included in menu tiles.
	static const UINT Tiles[][4] = {
		{TI_DOOR_Y},
		{TI_DOOR_C},
		{TI_DOOR_M},
		{TI_DOOR_R},
		{TI_TAR_NSEW}
	};

	switch (wObjectNo)
	{
		//O-layer
		case T_FLOOR: return OMenuDisplayTiles[0];
		case T_TRAPDOOR: return OMenuDisplayTiles[1];
		case T_WALL: return OMenuDisplayTiles[2];
		case T_WALL_B: return OMenuDisplayTiles[3];
		case T_PIT: return OMenuDisplayTiles[4]+2;
		case T_STAIRS: return OMenuDisplayTiles[5];
		case T_DOOR_Y: return Tiles[0];
		case T_DOOR_YO: return OMenuDisplayTiles[7];
		case T_DOOR_C: return Tiles[1];
		case T_DOOR_M: return Tiles[2];
		case T_DOOR_R: return Tiles[3];
		case T_OB_1: return OMenuDisplayTiles[11];
		case T_CHECKPOINT: return OMenuDisplayTiles[12];

		//T-layer
		case T_EMPTY: return OMenuDisplayTiles[0];
		case T_ORB: return TMenuDisplayTiles[0];
		case T_ARROW_NW: return ARROW_TI + this->wO;
		case T_POTION_I: return TMenuDisplayTiles[2];
		case T_POTION_K: return TMenuDisplayTiles[3];
		case T_TAR: return Tiles[4];
		case T_SCROLL: return TMenuDisplayTiles[5];

		//M-layer
		case T_SWORDSMAN: return SMAN_TI + this->wO;
		case T_NOMONSTER: return OMenuDisplayTiles[0];
		case T_ROACH: return GetTileImageForMonsterType(M_ROACH, this->wO, 0);
		case T_QROACH: return GetTileImageForMonsterType(M_QROACH, this->wO, 0);
		case T_EYE: return GetTileImageForMonsterType(M_EYE, this->wO, 0);
		case T_WWING: return GetTileImageForMonsterType(M_WWING, this->wO, 0);
		case T_TARBABY: return GetTileImageForMonsterType(M_TARBABY, this->wO, 0);
		case T_SERPENT: return GetTileImageForMonsterType(M_SERPENT, E, 0);
		case T_TARMOTHER: return MMenuDisplayTiles[7];
		case T_SPIDER: return GetTileImageForMonsterType(M_SPIDER, this->wO, 0);
		case T_GOBLIN: return GetTileImageForMonsterType(M_GOBLIN, this->wO, 0);
		case T_BRAIN: return GetTileImageForMonsterType(M_BRAIN, NO_ORIENTATION, 0);

		default: ASSERTP(false, "Bad object#."); return false;
	}
}

//*****************************************************************************
DWORD CEditRoomScreen::AddRoom(
//Inserts a new room into the current level at given position in the DB.
//(Code was mostly cut-and-pasted from CEditSelectScreen::AddRoom().)
//
//Returns: new room's ID#
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in) Coords of chosen room.
{
	if (!this->pLevel)
		return 0L;
	if (this->pLevel->FindRoomIDAtCoords(dwRoomX,dwRoomY))
		return 0L;	//room already exists here

   //Get new room.
   CDbRoom *pRoom = this->pLevel->Rooms.GetNew();

   //Set members that correspond to database fields.
   //Note: pRoom->dwLevelID was already set to match its containing level
	//in the call to CDbRooms::GetNew().
	pRoom->dwRoomX = dwRoomX;
	pRoom->dwRoomY = dwRoomY;
	pRoom->wRoomCols = CDrodBitmapManager::DISPLAY_COLS;
	pRoom->wRoomRows = CDrodBitmapManager::DISPLAY_ROWS;
	pRoom->wStyle = this->pRoom->wStyle;	//maintain style
	pRoom->bIsRequired = true;

	//Make room empty.
	const DWORD dwSquareCount = pRoom->wRoomCols * pRoom->wRoomRows;
	pRoom->pszOSquares = new char[dwSquareCount + 1];
	if (!pRoom->pszOSquares)
	{
		delete pRoom;
		return 0L;
	}
	pRoom->pszTSquares = new char[dwSquareCount + 1];
	if (!pRoom->pszTSquares)
	{
		delete pRoom;
		return 0L;
	}
	memset(pRoom->pszOSquares, T_FLOOR, dwSquareCount * sizeof(char));
	memset(pRoom->pszTSquares, T_EMPTY, dwSquareCount * sizeof(char));
	pRoom->pszOSquares[dwSquareCount] = 0;
	pRoom->pszTSquares[dwSquareCount] = 0;

	//Add proper edges to the room.
	FillInRoomEdges(pRoom);

	//Save the new room.
	if (!pRoom->Update())
	{
		ShowOkMessage(MID_RoomNotSaved);
		delete pRoom;
		return 0L;
	}

	const DWORD dwRoomID = pRoom->dwRoomID;
	delete pRoom;

	//Update map.
	this->pMapWidget->LoadFromLevel(this->pLevel);
	LoadRoomAtCoords(dwRoomX,dwRoomY);

	return dwRoomID;
}

//*****************************************************************************
void CEditRoomScreen::ApplyPlayerSettings()
//Apply player settings to the game screen.
{
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	if (!pCurrentPlayer) {ASSERTP(false, "Couldn't retrieve player."); return;} //Corrupt db.

	//Set the keysym-to-command map from player settings.
	InitKeysymToCommandMap(pCurrentPlayer->Settings);

	//Set default room editing options, if needed.
	if (SetUnspecifiedPlayerSettings(pCurrentPlayer->Settings))
		pCurrentPlayer->Update();

	//Set room editing options.
	this->bAutoSave = 0!=pCurrentPlayer->Settings.GetVar("AutoSave", true);

	COptionButtonWidget *pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_SHOWERRORS));
	this->bShowErrors = 0!=pCurrentPlayer->Settings.GetVar("ShowErrors", true);
	pOptionButton->SetChecked(this->bShowErrors);

	delete pCurrentPlayer;
}

//*****************************************************************************
bool CEditRoomScreen::LoadRoomAtCoords(
//Loads a room on this level and sets the current room to it.
//If a failure occurs, the current room will stay loaded.
//
//Params:
	const DWORD dwRoomX, const DWORD dwRoomY)	//(in)	Coords to specify room to load.
//
//Returns:
//True if successful, false if not.
{
   //Reload room, even if it has the same coordinates as the current room,
   //as it might have just been updated in the DB.

	CDbRoom *pNewRoom = this->pLevel->GetRoomAtCoords(dwRoomX, dwRoomY);
	if (!pNewRoom)
      return false;

	//Finish/discard any operation that was in place.
	SetState(ES_PLACING);
	this->pRoomWidget->ClearEffects(false);
	ClearList(this->UndoList);
	ClearList(this->RedoList);
   SetButtons();

	SaveRoom();
   //If a copy of another player's hold was just made,
   //then we need to load the room from it instead of this one.
   if (this->pLevel->dwLevelID != pNewRoom->dwLevelID)
   {
      delete pNewRoom;
      pNewRoom = this->pLevel->GetRoomAtCoords(dwRoomX,
            (this->pLevel->dwLevelID * 100) + (dwRoomY % 100));
      if (!pNewRoom)
         return false;
   }

	delete this->pRoom;
	this->pRoom = pNewRoom;
   this->bRoomDirty = false;

	this->pRoomWidget->LoadFromRoom(pNewRoom);
	this->pMapWidget->SelectRoom(this->pRoom->dwRoomX, this->pRoom->dwRoomY);
	CRoomScreen::SetMusicStyle(this->pRoom->wStyle);
	SetSignTextToCurrentRoom();
	Paint();

   //Reset room area marked for copying.
   this->wCopyX1 = this->wCopyY1 = this->wCopyX2 = this->wCopyY2 = (UINT)-1;
   this->bAreaJustCopied = this->bReadyToPaste = false;

	return true;
}

//*****************************************************************************
void CEditRoomScreen::OnBetweenEvents()
//Called periodically when no events are being processed.
{
   CScreen::OnBetweenEvents();
   ShowCursor();

	switch (this->eState)
	{
      case ES_DOOR:
         ASSERT(this->pOrb);
         this->pRoomWidget->AddOrbAgentsEffect(this->pOrb, false);
         RequestToolTip(MID_DoorAgentTip);
      break;

      case ES_ORB:
			ASSERT(this->pOrb);
			this->pRoomWidget->AddOrbAgentsEffect(this->pOrb);
         RequestToolTip(MID_OrbAgentTip);
		break;

		case ES_LONGMONSTER:
         RequestToolTip(MID_LongMonsterTip);
		break;

		case ES_TESTROOM:
         RequestToolTip(MID_TestRoomTip);
		break;

      default: break;
	}

   //The EditRoomWidget doesn't call UpdateRect() on calls to Animate().
   //This is to remove flickering effects caused by redrawing the room,
   //temporarily erasing screen effects (e.g. tooltips) that get drawn over the room widget.
   this->pRoomWidget->UpdateRect();
}

//*****************************************************************************
void CEditRoomScreen::OnMouseDown(
//Called when widget receives a mouse down event.
//
//Params:
	const DWORD dwTagNo, const SDL_MouseButtonEvent &Button)
{
	CScreen::OnMouseDown(dwTagNo, Button);

	switch (dwTagNo)
	{
		case TAG_ROOM:
			switch (this->eState)
			{
			case ES_PLACING:
				if (RightMouseButton())
               AddPlotEffect(T_FLOOR); //Deleting object
            else if (!this->bReadyToPaste)
					AddPlotEffect(this->wSelectedObject);  //Plotting object
				break;
			case ES_TESTROOM:
				AddPlotEffect(T_SWORDSMAN);
				break;
         default: break;
			}
		break;
      default: break;
	}
}

//*****************************************************************************
void CEditRoomScreen::OnMouseMotion(
//Called when widget receives a mouse move event.
//
//Params:
	const DWORD dwTagNo, const SDL_MouseMotionEvent &Motion)
{
	CScreen::OnMouseMotion(dwTagNo, Motion);

	switch (dwTagNo)
	{
	case TAG_ROOM:
		switch (this->eState)
		{
			case ES_LONGMONSTER:
				//Plot pending long monster segment.
				if (this->wSelectedObject != T_SERPENT)
				{
					//Remove part of monster placed so far.
					if (this->pLongMonster)
						RemoveMonster(this->pLongMonster,true);
					SetState(ES_PLACING);
					Paint();
					break;
				}
				this->pRoomWidget->AddMonsterSegmentEffect(this->wSelectedObject -
						M_OFFSET);
			break;
			case ES_PLACING:
			{
            if (this->bAreaJustCopied || this->bReadyToPaste) break;
				CWidget *pFocusWidget = MouseDraggingInWidget();
				if (pFocusWidget)
				{
					//Mouse drag.
					if (pFocusWidget->GetTagNo() == dwTagNo)
					{
						if (RightMouseButton())
                     AddPlotEffect(T_FLOOR); //Deleting objects.
                  else
							AddPlotEffect(this->wSelectedObject);  //Plotting objects.
					}
				}
			}
			break;
			case ES_TESTROOM:
			{
				CWidget *pFocusWidget = MouseDraggingInWidget();
				if (pFocusWidget)
					//Mouse drag.
					if (pFocusWidget->GetTagNo() == dwTagNo)
						AddPlotEffect(T_SWORDSMAN);
			}
			break;
         default: break;
		}
	break;
   default: break;
	}
}

//*****************************************************************************
void CEditRoomScreen::OnMouseUp(
//Called when widget receives a mouse up event.
//
//Params:
	const DWORD /*dwTagNo*/, const SDL_MouseButtonEvent &/*Button*/)
{
	//Any plotting is now finished.
	this->pRoomWidget->RemoveLastLayerEffectsOfType(EPENDINGPLOT);
	this->pRoomWidget->RemoveLastLayerEffectsOfType(ETRANSTILE);
}

//*****************************************************************************
void CEditRoomScreen::OnMouseWheel(
//Called when a mouse wheel event is received.
//
//Params:
   const SDL_MouseButtonEvent &Button)
{
   //Mouse wheel changes orientation.
   if (Button.button == SDL_BUTTON_WHEELDOWN)
   {
		this->wO = nNextCO(this->wO);
		g_pTheSound->PlaySoundEffect(SEID_SWING);
   } else if (Button.button == SDL_BUTTON_WHEELUP)
   {
		this->wO = nNextCCO(this->wO);
		g_pTheSound->PlaySoundEffect(SEID_SWING);
   }

   //Update room widget if a plot is occurring.
	CWidget *pFocusWidget = MouseDraggingInWidget();
	if (pFocusWidget)
		if (pFocusWidget->GetTagNo() == TAG_ROOM)
		{
			//Mouse drag is occurring.
			//Update objects being plotted.
			AddPlotEffect(eState == ES_TESTROOM ? T_SWORDSMAN :
					this->wSelectedObject);
		}
}

//*****************************************************************************
void CEditRoomScreen::OnClick(
//Called when screen receives a click event.
//
//Params:
	const DWORD dwTagNo) //(in)	Widget that event applied to.
{
	switch (dwTagNo)
	{
		case TAG_ESCAPE:
			if (this->eState != ES_PLACING)
         {
				SetState(ES_PLACING);
            Paint(); //redraw room highlights
         } else {
			   SaveRoom();
			   UpdatePlayerDataFromWidgets();

			   //Remove any playtesting sessions.
			   {
				   CGameScreen *pGameScreen = DYN_CAST(CGameScreen *, CScreen*,
						   g_pTheSM->GetLoadedScreen(SCR_Game));
				   pGameScreen->UnloadGame();
			   }

			   GoToScreen(SCR_Return);
         }
		break;

		case TAG_UNDO:
         Undo();
      break;

		case TAG_REDO:
         Redo();
      break;

      case TAG_ROOM:
			switch (this->eState)
			{
				case ES_LONGMONSTER:
				{
					//Place a long monster segment if there are no obstacles in the way.
					PlotType plot = PlotMonsterSegment();
					switch (plot)
					{
						case PLOT_NOTHING:
							//An illegal placement was attempted -- try again.
							break;
						case PLOT_HEAD:
						{
							//Only a head was plotted -- remove it.
							CCueEvents Ignored;
							this->pRoom->KillMonsterAtSquare(
									this->pRoomWidget->monsterSegment.wHeadX,
									this->pRoomWidget->monsterSegment.wHeadY,
									Ignored);

							//Remove saved state before serpent was begun, since
							//nothing actually changed.
                     RemoveRoomChange();

							SetState(ES_PLACING);
							Paint();	//erase shaded square and redraw errors
						}
							break;
						case PLOT_DONE:
							//Done placing monster.
							SetState(ES_PLACING);
							Paint();	//redraw errors
							break;
						case PLOT_SEGMENT:
							//Another segment was placed -- continue.
							break;
						case PLOT_ERROR:
							ASSERTP(false, "Plot error.");
							break;
					}
				}
				break;

            case ES_DOOR:
            {
					const UINT wX = this->pRoomWidget->wEndX,
							wY = this->pRoomWidget->wEndY;
               const UINT wDoorX = this->pOrb->wX,
                    wDoorY = this->pOrb->wY;
               const UINT wOSquare = this->pRoom->GetOSquare(wX, wY);
               const UINT wTSquare = this->pRoom->GetTSquare(wX, wY);
               if (wOSquare == T_DOOR_Y || wOSquare == T_DOOR_YO)
					{
						//Start editing the orb agents associated with this door.
                  //this->pOrb will keep track of which orb affect door.
                  SetOrbAgentsForDoor(wX, wY);
               }
               else if (wTSquare == T_ORB)
               {
                  //Change the door's association with the orb clicked on.
						RoomChanging();
                  COrbAgentData *pAgent = NULL;
						COrbData *pEditOrb = this->pRoom->GetOrbAtCoords(wX,wY);
						if (!pEditOrb)
						{
							//Add an orb record for this orb.
							pEditOrb = this->pRoom->AddOrbToSquare(wX,wY);
                     ASSERT(pEditOrb);
                  } else {
                     // See whether this orb has any agents this door.
                     pAgent = FindOrbAgentFor(wDoorX, wDoorY, pEditOrb);
                  }
						if (!pAgent)
						{
                     //No -- give the orb an agent to this door.
							pEditOrb->AddAgent(wDoorX,wDoorY,OA_TOGGLE);
                     this->pOrb->AddAgent(wX, wY, OA_TOGGLE);
							this->pRoomWidget->AddOrbAgentToolTip(wX, wY, OA_TOGGLE);
						} else {
                     //Yes -- change the orb's effect on this door.
                     COrbAgentData* pDoorAgent = this->pOrb->GetAgentAt(wX, wY);
                     ASSERT(pDoorAgent);
							++pAgent->wAction;
							if (pAgent->wAction > static_cast<UINT>(OA_CLOSE))
							{
								pEditOrb->DeleteAgent(pAgent);
                        this->pOrb->DeleteAgent(pDoorAgent);
								this->pRoomWidget->AddOrbAgentToolTip(wX, wY, OA_NULL);
                     } else {
								++pDoorAgent->wAction;
								this->pRoomWidget->AddOrbAgentToolTip(wX, wY, pAgent->wAction);
                     }
						}
             }
               else {
						//If neither is here, leave orb editing state.
						this->pRoomWidget->ResetPlot();
						this->pRoomWidget->wOX = static_cast<UINT>(-1);
                  delete this->pOrb;
						SetState(ES_PLACING);
						this->pRoomWidget->ClearEffects();
						Paint();	//refresh room
               }
            }
            break;

				case ES_ORB:
				{
					ASSERT(this->pOrb);	//some orb should always be selected
					UINT wX = this->pRoomWidget->wEndX,
							wY = this->pRoomWidget->wEndY;
					if (this->pRoom->GetTSquare(wX,wY) == T_ORB)
					{
						//Start editing this orb.
						COrbData *pNewOrb = this->pRoom->GetOrbAtCoords(wX,wY);
						//!!Add to list of orbs being edited in some cases.
						if (!pNewOrb)
						{
							//Add an orb record to the room.
							this->pOrb = this->pRoom->AddOrbToSquare(wX,wY);
						} else {
							this->pOrb = pNewOrb;
						}
						this->pRoomWidget->RemoveLastLayerEffectsOfType(ETOOLTIP);
						this->pRoomWidget->AddOrbAgentsEffect(this->pOrb);
					} else {
						const UINT wOTileNo = this->pRoom->GetOSquare(wX,wY);
						//If a door exists here, add, modify or delete its orb agent.
						if (wOTileNo == T_DOOR_Y || wOTileNo == T_DOOR_YO)
						{
							RoomChanging();
                     //Is this door already affected by the orb?
							COrbAgentData *pAgent = FindOrbAgentFor(wX,wY,this->pOrb);
							if (!pAgent)
							{
                        //No -- add new agent for this door.
								this->pOrb->AddAgent(wX,wY,OA_TOGGLE);
								this->pRoomWidget->AddOrbAgentToolTip(wX, wY,
										OA_TOGGLE);
							} else {
                        //Yes -- change the agent's effect on the door.
								++pAgent->wAction;
								if (pAgent->wAction > static_cast<UINT>(OA_CLOSE))
								{
									this->pOrb->DeleteAgent(pAgent);
									this->pRoomWidget->AddOrbAgentToolTip(wX, wY,
											OA_NULL);
								}
								else
									this->pRoomWidget->AddOrbAgentToolTip(wX, wY,
											pAgent->wAction);
							}

							this->pRoomWidget->AddOrbAgentsEffect(this->pOrb);
						} else {
							//If neither is here, leave orb editing state.
							this->pRoomWidget->ResetPlot();
							this->pRoomWidget->wOX = static_cast<UINT>(-1);
							SetState(ES_PLACING);
							this->pRoomWidget->ClearEffects(true);
							Paint();	//refresh room
						}
					}
				}
				break;

				case ES_PLACING:
					if (RightMouseButton())
               {
                  EraseObjects();
               } else {
                  //If Ctrl-C was hit to copy a room region while the mouse
                  //was down, then let it come up w/o doing anything.
                  if (this->bAreaJustCopied)
                  {
                     this->bAreaJustCopied = false;
                     break;
                  }

                  if (!this->bReadyToPaste)
                  {
					      PlotObjects();
                  } else {
                     //Duplicate marked room region.
     					   g_pTheSound->PlaySoundEffect(SEID_MIMIC);

                     ASSERT(this->wCopyX1 != (UINT)-1);
					      const UINT wX = this->pRoomWidget->wEndX,
							      wY = this->pRoomWidget->wEndY;
                     PasteRegion(wX,wY);

                     //Must hit Ctrl-V again to re-paste.
                     this->bReadyToPaste = false;
                  }
               }
				break;

				case ES_TESTROOM:
				{
					const UINT wX = this->pRoomWidget->wEndX,
							wY = this->pRoomWidget->wEndY;
               if (!this->pRoomWidget->IsSafePlacement(T_SWORDSMAN, wX, wY,
                     this->wO)) return;

					if (!SaveRoom())	//update latest changes
               {
                  //Reload the room in its unmodified state.
                  this->bRoomDirty = false;  //Don't ask again to save room.
                  LoadRoomAtCoords(this->pRoom->dwRoomX, this->pRoom->dwRoomY);
                  //Confirm starting place is still valid.
                  if (!this->pRoomWidget->IsSafePlacement(T_SWORDSMAN, wX, wY,
                        this->wO)) return;
               }

					CGameScreen *pGameScreen = DYN_CAST(CGameScreen *, CScreen *,
							g_pTheSM->GetLoadedScreen(SCR_Game));
					SetState(ES_PLACING);

               CDbPlayer *pCurPlayer = g_pTheDB->GetCurrentPlayer();
               ASSERT(pCurPlayer);

               //Create temporary player for play-testing.
               CDbPlayer *pPlayer = g_pTheDB->Players.GetNew();
               pPlayer->bIsLocal = false;
		         pPlayer->NameText = wszEmpty;
		         pPlayer->EMailText = wszEmpty;
               pPlayer->Settings = pCurPlayer->Settings;
               pPlayer->Update();
               this->dwTestPlayerID = pPlayer->dwPlayerID;
               this->dwSavePlayerID = g_pTheDB->GetPlayerID();
               g_pTheDB->SetPlayerID(this->dwTestPlayerID);
               delete pPlayer;
               delete pCurPlayer;

					if (pGameScreen->TestRoom(this->pRoom->dwRoomID, wX, wY, this->wO))
						GoToScreen(SCR_Game);
               else
               {
                  //Delete temp player if starting play-testing failed.
                  g_pTheDB->SetPlayerID(this->dwSavePlayerID);
                  g_pTheDB->Players.Delete(this->dwTestPlayerID, false);
                  this->dwSavePlayerID = this->dwTestPlayerID = 0;
               }
				}
				break;
            default: break;
			}
		break;

		//Editor options.
		case TAG_SHOWERRORS:
      {
         COptionButtonWidget *pButton = static_cast<COptionButtonWidget *>(
					GetWidget(TAG_SHOWERRORS));
			this->bShowErrors = pButton->IsChecked();
			//Update room error display.
			if (!this->bShowErrors)
				this->pRoomWidget->RemoveLastLayerEffectsOfType(ESHADE);
			Paint();	
      }
		break;

		default:
			//Hotspots
			if (this->wLastMouseX <= 48 && 455 <= this->wLastMouseY)
			{
				//Esc -- menu
				SaveRoom();
				UpdatePlayerDataFromWidgets();
				GoToScreen(SCR_Return);
			}
			else if (58 <= this->wLastMouseX && this->wLastMouseX <= 97 &&
					455 <= this->wLastMouseY)
			{
				//F1 -- help
				SetFullScreen(false);
				ShowHelp("editroom.html");
			}
		break;
	}
}

//*****************************************************************************
void CEditRoomScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD dwTagNo,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Key event.
{
	if (this->eState == ES_PLACING)
		CScreen::OnKeyDown(dwTagNo, Key);

	//Check for other keys.
	switch (Key.keysym.sym)
	{
		case SDLK_ESCAPE:
			if (this->eState != ES_PLACING)
         {
				SetState(ES_PLACING);
            Paint(); //redraw room highlights
         } else {
				SaveRoom();
				UpdatePlayerDataFromWidgets();

				//Remove any playtesting sessions.
				{
					CGameScreen *pGameScreen = DYN_CAST(CGameScreen *, CScreen *,
							g_pTheSM->GetLoadedScreen(SCR_Game));
					pGameScreen->UnloadGame();
				}
			}
		break;

		case SDLK_F1:
			SetFullScreen(false);
			ShowHelp("editroom.html");
		break;

		case SDLK_F4:
			if (Key.keysym.mod & KMOD_ALT)
			{
				//Save on exit.
				UpdatePlayerDataFromWidgets();
				SaveRoom();
			}
		break;

		case SDLK_F5:
			SetState(ES_TESTROOM);
		break;

		case SDLK_c:
			if (Key.keysym.mod & KMOD_CTRL)
			{
            //Only allow paste when mouse button is down in room widget
            //when Ctrl-C is pressed.
            StopKeyRepeating();  //don't repeat this operation
            CWidget *pWidget = MouseDraggingInWidget();
            if (pWidget && pWidget->GetTagNo() == TAG_ROOM &&
                  !RightMouseButton() && this->eState == ES_PLACING)
            {
               ASSERT(this->pRoomWidget->wEndX != static_cast<UINT>(-1));
               g_pTheSound->PlaySoundEffect(SEID_POTION);

               //Store coords of last selected room region.
               this->wCopyX1 = this->pRoomWidget->wMidX;
               this->wCopyY1 = this->pRoomWidget->wMidY;
               this->wCopyX2 = this->pRoomWidget->wEndX;
               this->wCopyY2 = this->pRoomWidget->wEndY;
               this->bAreaJustCopied = true;
               ReadyToPasteRegion();

	            //Stop any plotting effects.
	            this->pRoomWidget->RemoveLastLayerEffectsOfType(EPENDINGPLOT);
	            this->pRoomWidget->RemoveLastLayerEffectsOfType(ETRANSTILE);
            }
         }
      break;

      case SDLK_v:
			if (Key.keysym.mod & KMOD_CTRL)
			{
            StopKeyRepeating();  //don't repeat this operation
				CWidget *pWidget = GetSelectedWidget();
				switch (pWidget->GetTagNo())
				{
					case TAG_MAP:
					{
						//Paste a room.
                  if (!SaveRoomToDB()) return;
						CMapWidget *pMap = DYN_CAST(CMapWidget*, CWidget*, pWidget);
						if (pMap->IsDeletingRoom())
							if (ShowYesNoMessage(MID_DeleteRoomPrompt) != TAG_YES)
								break;
						this->bRoomDirty = false;	//don't save old room
						const bool bUpdate = pMap->PasteRoom();
						if (bUpdate)
						{
							DWORD dwRoomX, dwRoomY;
							this->pMapWidget->GetSelectedRoomXY(dwRoomX, dwRoomY);
							LoadRoomAtCoords(dwRoomX, dwRoomY);
                     return;
						}
					}
					break;

               default: break;
            }

            //If a room wasn't just pasted, Ctrl-V will indicate that the next
            //click will paste the room region previously selected for copying.
            if (this->wCopyX1 != (UINT)-1)
            {
               ReadyToPasteRegion();
            }
			}
		break;

		case SDLK_z:
			if (Key.keysym.mod & KMOD_CTRL)
            Undo();
		break;
		case SDLK_y:
			if (Key.keysym.mod & KMOD_CTRL)
            Redo();
		break;
      default: break;
	}

	switch (dwTagNo)
	{
		//Editor options.  (Copied from OnClick().)
		case TAG_SHOWERRORS:
      {
         COptionButtonWidget *pButton = static_cast<COptionButtonWidget *>(
					GetWidget(TAG_SHOWERRORS));
			this->bShowErrors = pButton->IsChecked();
			//Update room error display.
         if (!this->bShowErrors && !this->bReadyToPaste)
				this->pRoomWidget->RemoveLastLayerEffectsOfType(ESHADE);
			Paint();
      }
		break;
	}

	//Check for command keys.
	const UINT wOldO = this->wO;
	const int nCommand = this->KeysymToCommandMap[Key.keysym.sym];
	switch (nCommand)
	{
		//Rotate orientation.
		case CMD_C:
			this->wO = nNextCO(this->wO);
			g_pTheSound->PlaySoundEffect(SEID_SWING);
			break;
		case CMD_CC:
			this->wO = nNextCCO(this->wO);
			g_pTheSound->PlaySoundEffect(SEID_SWING);
			break;
	}

	if (wOldO != this->wO)
	{
		//Update room widget if a plot is occurring.
      if (!this->bReadyToPaste)
      {
		   CWidget *pFocusWidget = MouseDraggingInWidget();
		   if (pFocusWidget)
			   if (pFocusWidget->GetTagNo() == TAG_ROOM)
			   {
				   //Mouse drag is occurring.
				   if (!RightMouseButton())
					   //Update objects being plotted.
					   AddPlotEffect(eState == ES_TESTROOM ? T_SWORDSMAN :
							   this->wSelectedObject);
			   }
      }
	}
}

//*****************************************************************************
void CEditRoomScreen::OnSelectChange(
//Handles a selection change.
//
//Params:
	const DWORD dwTagNo) //(in) Widget event applies to.
{
	switch (dwTagNo)
	{
		case TAG_MAP:
		{
			//Selected room on map has changed.
			DWORD dwRoomX, dwRoomY;
			this->pMapWidget->Paint();
			this->pMapWidget->GetSelectedRoomXY(dwRoomX, dwRoomY);
			if (this->pMapWidget->bVacantRoom)
			{
				if (ShowYesNoMessage(MID_AddRoomPrompt) == TAG_YES &&
                  SaveRoomToDB())
				{
					SaveRoom();
					AddRoom(dwRoomX,dwRoomY);
            } else {
               //Put the map selection back on the current room.
               this->pMapWidget->SelectRoom(this->pRoom->dwRoomX,
                     this->pRoom->dwRoomY);
               this->pMapWidget->Paint();
            }
			}
			else
				LoadRoomAtCoords(dwRoomX, dwRoomY);
		}
		break;

		case TAG_MENU:
		{
			//When a new tab is selected, update the selected object from
			//the visible object menu.
			const UINT wTab = DYN_CAST(CTabbedMenuWidget *, CWidget *,
					GetWidget(dwTagNo))->GetSelectedTab();
			CObjectMenuWidget *pMenu;
			switch (wTab)
			{
				case 0: pMenu = DYN_CAST(CObjectMenuWidget *, CWidget *,
						GetWidget(TAG_OMENU));	break;
				case 1: pMenu = DYN_CAST(CObjectMenuWidget *, CWidget *,
						GetWidget(TAG_TMENU));	break;
				case 2: pMenu = DYN_CAST(CObjectMenuWidget *, CWidget *,
						GetWidget(TAG_MMENU));	break;
				default: ASSERTP(false, "Bad tab on edit menu.");	break;
			}
			const UINT wObject = pMenu->GetSelectedObject();
			if (wObject != NO_SELECTED_OBJECT)
				this->wSelectedObject = wObject;
		}
		break;

		case TAG_OMENU:
		case TAG_TMENU:
		case TAG_MMENU:
		{
			//New placement object selection.
			const UINT wObject = DYN_CAST(CObjectMenuWidget *, CWidget *,
					GetWidget(dwTagNo))->GetSelectedObject();
			if (wObject != NO_SELECTED_OBJECT)
				this->wSelectedObject = wObject;
		}
		break;
	}
}

//*****************************************************************************
bool CEditRoomScreen::OnQuit()
//Called when screen receives a click event.
{
	const bool bQuit = CScreen::OnQuit();
	if (bQuit)
	{
		UpdatePlayerDataFromWidgets();
		SaveRoom();
	}
	return bQuit;
}

//*****************************************************************************
void CEditRoomScreen::EditObjects()
{
	const UINT wX = this->pRoomWidget->wEndX, wY = this->pRoomWidget->wEndY;
	const UINT wOTileNo = this->pRoom->GetOSquare(wX,wY);
	const UINT wTTileNo = this->pRoom->GetTSquare(wX,wY);
	CMonster *pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);

	//Is a serpent here?
	if (pMonster)
   {
		if (pMonster->wType == M_SERPENT)
		{
			//Resume editing this serpent.
			g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);
			CSerpent *pSerpent = DYN_CAST(CSerpent*, CMonster *, pMonster);

			//Get tail coordinates.
			UINT wTailX, wTailY;
			{
				//!!There's gotta be a better way to do this.
				CCueEvents Ignored;
				CCurrentGame *pTempGame = g_pTheDB->GetNewTestGame(
						this->pRoom->dwRoomID, Ignored, 0, 0, 0);
				delete pTempGame->pRoom;
				pTempGame->pRoom = this->pRoom;
				pSerpent->SetCurrentGame(pTempGame);
				pSerpent->GetTail(wTailX,wTailY);
				pSerpent->ResetCurrentGame();
				pTempGame->pRoom = NULL;
				delete pTempGame;
			}

			UINT wDirection, wTailDirection;
			this->pRoomWidget->monsterSegment.wHeadX = pSerpent->wX;
			this->pRoomWidget->monsterSegment.wHeadY = pSerpent->wY;
			this->pRoomWidget->monsterSegment.wTailX = wTailX;
			this->pRoomWidget->monsterSegment.wTailY = wTailY;
			switch (this->pRoom->GetTSquare(wTailX,wTailY))
			{
				case T_SNKT_E:	wDirection = E;	wTailDirection = W;	break;
				case T_SNKT_W:	wDirection = W;	wTailDirection = E;	break;
				case T_SNKT_N:	wDirection = N;	wTailDirection = S;	break;
				case T_SNKT_S:	wDirection = S;	wTailDirection = N;	break;
			}
			this->pRoomWidget->monsterSegment.wDirection = wDirection;
			PlotLastMonsterSegment(wTailX,wTailY,wTailDirection);
			//don't need to set bHorizontal, wEX and wEY
			this->pLongMonster = pMonster;
			RoomChanging();
			SetState(ES_LONGMONSTER);
			this->wSelectedObjectSave = this->wSelectedObject;
			this->wSelectedObject = pMonster->wType + M_OFFSET;
			return;
		}
   }

	switch (wOTileNo)
	{
		case T_STAIRS:
      {
         SetDestinationLevel(wX,wY,wX,wY);
      }
		break;

      case T_DOOR_Y:
      case T_DOOR_YO:
      {
			//Edit door's "orb" agents.
			g_pTheSound->PlaySoundEffect(SEID_ORBHIT);
			SetState(ES_DOOR);
         SetOrbAgentsForDoor(wX,wY);
      }
      break;
	}

	switch (wTTileNo)
	{
		case T_SCROLL:
         EditScrollText(wX,wY);
		break;

		case T_ORB:
			//Edit orb's door agents.
			g_pTheSound->PlaySoundEffect(SEID_ORBHIT);
			SetState(ES_ORB);
			this->pOrb = this->pRoom->GetOrbAtCoords(wX,wY);
			if (!this->pOrb)
			{
				//Add an orb record to the room.
				this->pOrb = this->pRoom->AddOrbToSquare(wX,wY);
			}
			this->pRoomWidget->AddOrbAgentsEffect(this->pOrb);
		break;
	}
}

//*****************************************************************************
bool CEditRoomScreen::EditScrollText(
//Edit text on scroll.
//Returns: whether text on scroll was OKed or Cancelled
//
//Params:
   const UINT wX, const UINT wY) //(in)
{
	g_pTheSound->PlaySoundEffect(SEID_READ);
	const WCHAR *wStr = this->pRoom->GetScrollTextAtSquare(wX,wY);
	WSTRING wstrDescription;
   if (wStr)
      wstrDescription = wStr;
	this->pScrollLabel->SetText(wStr);
	SetState(ES_SCROLL);
	ShowScroll();
	const DWORD dwTagNo = ShowTextInputMessage(MID_EnterScrollText,
			wstrDescription, true);
   const bool bOK = dwTagNo == TAG_OK;
	if (bOK)
	{
		RoomChanging();
		this->pRoom->SetScrollTextAtSquare(wX,wY,&*wstrDescription.begin());
	}
	SetState(ES_PLACING);
	Paint();	//repaint menu

   return bOK;
}

//*****************************************************************************
void CEditRoomScreen::FillInRoomEdges(
//Adds a border to the room matching that of the adjacent rooms.
//
//Params:
	CDbRoom* const pRoom)	//(in/out) Room to modify
{
   ASSERT(pRoom);

	//Load adjacent rooms.
	CDbRoom *pAdjRoom[4];
	{
		pAdjRoom[0] = g_pTheDB->Rooms.GetByCoords(pRoom->dwLevelID,
				pRoom->dwRoomX,pRoom->dwRoomY-1);   //above
		pAdjRoom[1] = g_pTheDB->Rooms.GetByCoords(pRoom->dwLevelID,
				pRoom->dwRoomX+1,pRoom->dwRoomY);   //to right
		pAdjRoom[2] = g_pTheDB->Rooms.GetByCoords(pRoom->dwLevelID,
				pRoom->dwRoomX,pRoom->dwRoomY+1);   //below
		pAdjRoom[3] = g_pTheDB->Rooms.GetByCoords(pRoom->dwLevelID,
				pRoom->dwRoomX-1,pRoom->dwRoomY);   //to left
      ASSERT(!pAdjRoom[0] || pRoom->wRoomCols == pAdjRoom[0]->wRoomCols);
      ASSERT(!pAdjRoom[1] || pRoom->wRoomRows == pAdjRoom[1]->wRoomRows);
      ASSERT(!pAdjRoom[2] || pRoom->wRoomCols == pAdjRoom[2]->wRoomCols);
      ASSERT(!pAdjRoom[3] || pRoom->wRoomRows == pAdjRoom[3]->wRoomRows);
	}

#define GetSquare(pAdjRoom,wX,wY) (pAdjRoom ? pAdjRoom->GetOSquare(wX,wY) :\
		T_WALL)
#define SetSquare(wTileNo,wX,wY) if (wTileNo == T_WALL || wTileNo == T_PIT)\
		pRoom->Plot(wX,wY,wTileNo);

	//Examine adjacent rooms' bordering edge.
	UINT wX, wY, wAdjTile;
	for (wX=pRoom->wRoomCols; wX--; )
	{
		//top
		wAdjTile = GetSquare(pAdjRoom[0], wX, pAdjRoom[0]->wRoomRows-1);
		SetSquare(wAdjTile, wX, 0);
		//bottom
		wAdjTile = GetSquare(pAdjRoom[2], wX, 0);
		SetSquare(wAdjTile, wX, pRoom->wRoomRows-1);
	}
	for (wY=pRoom->wRoomRows; wY--; )
	{
		//left
		wAdjTile = GetSquare(pAdjRoom[3], pAdjRoom[3]->wRoomCols-1, wY);
      if (pRoom->GetOSquare(0, wY) != T_PIT) //don't overwrite modified corners
		   SetSquare(wAdjTile, 0, wY);
		//right
		wAdjTile = GetSquare(pAdjRoom[1], 0, wY);
      if (pRoom->GetOSquare(pRoom->wRoomCols-1, wY) != T_PIT)
		   SetSquare(wAdjTile, pRoom->wRoomCols-1, wY);
	}

	delete pAdjRoom[0];
	delete pAdjRoom[1];
	delete pAdjRoom[2];
	delete pAdjRoom[3];

#undef GetSquare
#undef SetSquare
}

//*****************************************************************************
COrbAgentData* CEditRoomScreen::FindOrbAgentFor(
//Finds which agent of orb affects the door connected to given square.
//
//Returns: pointer to agent if found, else NULL
//
//Params:
	const UINT wX, const UINT wY,	//(in) Coord to start searching from
	COrbData* pOrb)					//(in) Orb to search for
{
	ASSERT(this->pRoom->IsValidColRow(wX, wY));

   //Gather set of all squares this door is on.
	CCoordStack doorCoords;
   this->pRoom->GetAllDoorSquares(wX, wY, doorCoords);

   return FindOrbAgentFor(pOrb, doorCoords);
}

//*****************************************************************************
COrbAgentData* CEditRoomScreen::FindOrbAgentFor(
//Finds which agent of orb affects the door on the specified squares.
//
//Returns: pointer to agent if found, else NULL
//
//Params:
   COrbData* pOrb,               //(in)
   CCoordStack &doorCoords)      //(in/out) contents destroyed on exit
{
	this->pRoomWidget->ResetPlot();

	//Each iteration pops one pair of coordinates for plotting.
	UINT wDoorX, wDoorY, wIndex;
	while (doorCoords.Pop(wDoorX, wDoorY))
	{
		this->pRoomWidget->SetPlot(wDoorX, wDoorY);

      //Check whether there's an agent designated here.
		for (wIndex=pOrb->wAgentCount; wIndex--; )
		{
			if (pOrb->parrAgents[wIndex].wX == wDoorX &&
					pOrb->parrAgents[wIndex].wY == wDoorY)
				return pOrb->parrAgents + wIndex;	//Found it
		}
	}

	return NULL;	//didn't find one
}

//*****************************************************************************
void CEditRoomScreen::SetDestinationLevel(
//Show dialog to set destination level for stairs at (wX,wY).
//
//Params:
   UINT wX1, UINT wY1,	//(in) rectangle stairs are on
   UINT wX2, UINT wY2,
   const bool bRequireValue)        //(in) whether Cancel button is disabled (default = false)
{
	g_pTheSound->PlaySoundEffect(SEID_WALK);
	DWORD dwDestLevelID;
   const bool bFound = this->pRoom->GetExitLevelIDAt(wX1,wY1,dwDestLevelID);
   if (!bFound)
   {
      //This can happen using older (pre-release) room data where the staircase
      //didn't have the exit region explicitly defined -- Find the stair region.
      do {
         if (wY1 == 0) break;
         if (this->pRoom->GetOSquare(wX1,wY1-1) != T_STAIRS) break;
         --wY1;
      } while (!this->pRoom->GetExitLevelIDAt(wX1,wY1,dwDestLevelID));
      do {
         if (wY2 == this->pRoom->wRoomRows - 1) break;
         if (this->pRoom->GetOSquare(wX1,wY2+1) != T_STAIRS) break;
         //Stop if a defined stairway is found directly below this one.
         DWORD dwTemp;
         if (this->pRoom->GetExitLevelIDAt(wX1,wY2+1,dwTemp)) break;
         ++wY2;
      } while (true);
      while (wX1 > 0 && this->pRoom->GetOSquare(wX1-1,wY1) == T_STAIRS)
         --wX1;
      while (wX2 < this->pRoom->wRoomCols - 1 &&
            this->pRoom->GetOSquare(wX2+1,wY1) == T_STAIRS)
         ++wX2;
   }
   bool bValueSet = false;
   do {
	   if (SelectLevelID(this->pLevel, dwDestLevelID, MID_ExitLevelPrompt, !bRequireValue))
      {
		   RoomChanging();
		   this->pRoom->SetExit(dwDestLevelID, wX1, wY1, wX2, wY2);
         bValueSet = true;
      }
   } while (bRequireValue && !bValueSet);
}

//*****************************************************************************
void CEditRoomScreen::SetOrbAgentsForDoor(
//Finds all orbs acting on this door and
//displays visual representation in the room widget.
//OUT: this->pOrb contains list of all orbs acting on this door
//
//Params:
   const UINT wX, const UINT wY)	//(in) square door is on
{
   delete this->pOrb;
   this->pOrb = new COrbData;
   this->pOrb->wX = wX;
   this->pOrb->wY = wY;

   CCoordStack doorCoords;
   this->pRoom->GetAllDoorSquares(wX, wY, doorCoords);

   for (UINT orb=0; orb < this->pRoom->wOrbCount; ++orb) {
      COrbData* pData = &(this->pRoom->parrOrbs[orb]);
      for (UINT agent=0; agent < pData->wAgentCount; ++agent) {
         COrbAgentData* pAgentData = pData->parrAgents + agent;
         if (doorCoords.IsMember(pAgentData->wX, pAgentData->wY))
         {
            COrbAgentData* pNewAgent = new COrbAgentData(pData->wX, pData->wY, pAgentData->wAction);
            this->pOrb->AddAgent(pNewAgent);
            this->pRoomWidget->AddOrbEffect(pAgentData);
         }
      }
   }
   if (this->pOrb->wAgentCount > 0)
   {
		this->pRoomWidget->RemoveLastLayerEffectsOfType(ETOOLTIP);
      this->pRoomWidget->AddOrbAgentsEffect(this->pOrb, false);
      this->pRoomWidget->ResetPlot();
  }
}

//*****************************************************************************
bool CEditRoomScreen::IsObstacleWholeAt(
//Determines whether obstacle placed at this square would not be fragmented,
//according to the rule that a minimum of a 2x2 square of obstacle can exist.
//(Code combined from CalcTileImageForObstacle() and CDbRoom::IsTarStableAt().)
//
//Params:
	const UINT wX, const UINT wY)	//(in) square being considered
//
//Returns:
//Whether obstacle is whole here.
const
{
	bool obstacles[3][3] = {{false}};	//center position is square being considered
	UINT x = wX, y = wY, xPos = 0, yPos = 0;

	//Find parts of obstacle to figure out which corner this square is at.
	while (x > 0) {
		if (this->pRoom->GetOSquare(x - 1, wY) == T_OB_1)
		{
			++xPos;
			--x;
		} else break;
	}
	while (y > 0) {
		if (this->pRoom->GetOSquare(wX, y - 1) == T_OB_1)
		{
			++yPos;
			--y;
		} else break;
	}

	//Mark where adjacent obstacles are.
	//Obstacles on the edge of the screen are okay.
	for (x=wX-1; x!=wX+2; x++)
		for (y=wY-1; y!=wY+2; y++)
			obstacles[x-wX+1][y-wY+1] = (this->pRoom->IsValidColRow(x,y) ?
					(this->pRoom->GetOSquare(x,y) == T_OB_1) : true);

	//Boundary checks (for top and left edges of screen).
	if (wX == 0 && pRoom->GetOSquare(1, wY) != T_OB_1)
		if (wY == 0 && pRoom->GetOSquare(0, 1) != T_OB_1)
			return true;	//ok (top-left corner)
		else return (yPos % 2 == 0 ? obstacles[1][2] : obstacles[1][0]);
	else
		if (wY == 0 && pRoom->GetOSquare(wX, 1) != T_OB_1)
			return (xPos % 2 == 0 ? obstacles[2][1] : obstacles[0][1]);

	//Is this obstacle piece correctly adjacent to 3 other pieces?
	if (xPos % 2 == 0)
		if (yPos % 2 == 0)	//upper-left corner
			return (obstacles[2][2] && obstacles[2][1] && obstacles[1][2]);
		else	//lower-left corner
			return (obstacles[2][0] && obstacles[2][1] && obstacles[1][0]);
	else
		if (yPos % 2 == 0)	//upper-right corner
			return (obstacles[0][2] && obstacles[0][1] && obstacles[1][2]);
		else	//lower-right corner
			return (obstacles[0][0] && obstacles[0][1] && obstacles[1][0]);
}

//*****************************************************************************
void CEditRoomScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool bUpdateRect)				//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	PaintBackground();
	PaintSign();
	this->pRoomWidget->ResetForPaint();
   if (this->eState != ES_LONGMONSTER) //don't reset if a serpent's being placed
	   this->pRoomWidget->ResetPlot();
	PaintChildren();

	if (this->bShowErrors &&
         this->eState != ES_LONGMONSTER) //don't reset error highlights if a serpent's being placed
		ShowErrors();

	//Draw scroll on top when visible.
	if (this->eState == ES_SCROLL)
		PaintScroll();

	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CEditRoomScreen::ReadyToPasteRegion()
//Highlight the area about to be copied.
{
   static const SURFACECOLOR PaleYellow = {255, 255, 128};

   this->bReadyToPaste = true;
   this->pRoomWidget->RemoveLastLayerEffectsOfType(ESHADE);

   for (UINT wY = this->wCopyY1; wY <= this->wCopyY2; ++wY)
      for (UINT wX = this->wCopyX1; wX <= this->wCopyX2; ++wX)
         this->pRoomWidget->AddShadeEffect(wX,wY,PaleYellow);
   ShowErrors();
}

//*****************************************************************************
void CEditRoomScreen::PasteRegion(
//Paste contents of marked room region to clicked location.
//
//NOTE: Does shallow copy -- just of non-complex room items,
//i.e., customized items are left undefined, and serpents are not copied.
//
//Params:
   const UINT wX, const UINT wY) //(in) top-left corner of destination region
{
   if (this->wCopyX1 == (UINT)-1) return; //no region marked
   this->pRoomWidget->RemoveLastLayerEffectsOfType(ESHADE);	   
   if (this->wCopyX1 == wX && this->wCopyY1 == wY) { //same position
      this->pRoomWidget->ResetForPaint();
      return;  
   }

	RoomChanging();

   //Determine order to copy tiles in so that everything gets moved
   //correctly even if some of the selected area is being overwritten.
   const int xInc = wX <= this->wCopyX1 ? 1 : -1;
   const int yInc = wY <= this->wCopyY1 ? 1 : -1;
   const UINT wStartX = xInc > 0 ? this->wCopyX1 : this->wCopyX2;
   const UINT wEndX = xInc < 0 ? this->wCopyX1 : this->wCopyX2;
   const UINT wStartY = yInc > 0 ? this->wCopyY1 : this->wCopyY2;
   const UINT wEndY = yInc < 0 ? this->wCopyY1 : this->wCopyY2;

   //Copy tiles, one at a time.
   UINT x, y, xIndex, yIndex, wTileNo, wOldTile;
   CMonster *pMonster, *pNewMonster;
   COrbData *pOldOrb, *pNewOrb;
   WCHAR *oldScrollText = NULL;
   CCueEvents Ignored;
   CCoordStack removedStairs;
   bool bTarModified = false, bObstaclesModified = false;
   for (y=wStartY; true; y += yInc)
   {
      if (y >= this->pRoom->wRoomRows) continue;
      yIndex = wY + (y - this->wCopyY1);
      if (yIndex >= this->pRoom->wRoomRows) continue;
      for (x=wStartX; true; x += xInc)
      {
         if (x >= this->pRoom->wRoomCols) continue;
         xIndex = wX + (x - this->wCopyX1);
         if (xIndex >= this->pRoom->wRoomCols) continue;

         //Copy o-layer.
         wTileNo = this->pRoom->GetOSquare(x,y);
         wOldTile = this->pRoom->GetOSquare(xIndex, yIndex);
         switch (wOldTile)
         {
            case T_OB_1:
               bObstaclesModified = true;
               break;
            case T_STAIRS:
               removedStairs.Push(xIndex, yIndex);
               break;
            default: break;
         }
         EraseAndPlot(xIndex, yIndex, wTileNo, false);
         switch (wTileNo)
         {
            case T_OB_1:
               bObstaclesModified = true;
               break;
            default: break;
         }

         //Copy t-layer.
         wTileNo = this->pRoom->GetTSquare(x,y);
         wOldTile = this->pRoom->GetTSquare(xIndex, yIndex);
         if (!bIsSerpent(wTileNo))
         {
            switch (wOldTile)
            {
               case T_TAR:
                  bTarModified = true;
                  break;
               default: break;
            }
            EraseAndPlot(xIndex, yIndex, wTileNo, false);
            switch (wTileNo)
            {
               case T_ORB:
                  //Copy orb behavior.
                  pNewOrb = this->pRoom->AddOrbToSquare(xIndex, yIndex);
                  pOldOrb = this->pRoom->GetOrbAtCoords(x,y);  //do after the above line
                  if (pOldOrb)
                  {
		               for (UINT wAgentI=pOldOrb->wAgentCount; wAgentI--; )
		                  pNewOrb->AddAgent(pOldOrb->parrAgents + wAgentI);
                  }
                  break;
               case T_SCROLL:
               {
                  //Copy scroll behavior.
                  oldScrollText = (WCHAR*)this->pRoom->GetScrollTextAtSquare(x,y);
                  if (oldScrollText)
                     this->pRoom->SetScrollTextAtSquare(xIndex, yIndex, oldScrollText);
                  break;
               }
               case T_TAR:
                  bTarModified = true;
                  break;
               default: break;
            }
         }

         //Copy m-layer.
		   this->pRoom->KillMonsterAtSquare(xIndex, yIndex, Ignored);

         pMonster = this->pRoom->GetMonsterAtSquare(x,y);
         if (pMonster && pMonster->wType != M_SERPENT)
         {
			   pNewMonster = this->pRoom->AddNewMonster(
					   pMonster->wType, xIndex, yIndex);
            pNewMonster->wO = pMonster->wO;
         }
         if (x == wEndX) break;
      }
      if (y == wEndY) break;
   }
   this->monstersNotDeleted.clear();

   //Fix up messed-up things around the destination area.
   if (bObstaclesModified)
      FixUnstableObstacles();
   while (removedStairs.Pop(x,y))
	   FixCorruptStaircase(x,y);
   if (bTarModified)
      FixUnstableTar();

   if (this->bShowErrors)
		ShowErrors();
	this->pRoomWidget->ResetForPaint();
}

//*****************************************************************************
void CEditRoomScreen::PlotLastMonsterSegment(
//Follow tail until serpent turns.
//
//Params:
	const UINT wTailX, const UINT wTailY, const UINT wDirection)	//(in)
{
	UINT wX = wTailX, wY = wTailY, wTileNo;
	int wOX, wOY;
	switch (this->pRoomWidget->GetSerpentTailTile(wX,wY,wDirection,false))
	{
		case T_SNKT_N:
			wOX = 0;	wOY = 1;
			break;
		case T_SNKT_S:
			wOX = 0;	wOY = -1;
			break;
		case T_SNKT_E:
			wOX = -1;	wOY = 0;
			break;
		case T_SNKT_W:
			wOX = 1;	wOY = 0;
			break;
		default: ASSERTP(false, "Bad serpent tile.");	break;
	}
	this->pRoomWidget->SetPlot(wX,wY);	//tail
	do
	{
		wX += wOX;
		wY += wOY;
		this->pRoomWidget->SetPlot(wX,wY);	//possibly a twist
		wTileNo = this->pRoom->GetTSquare(wX,wY);
	} while (wTileNo == T_SNK_EW || wTileNo == T_SNK_NS);

	this->pRoomWidget->monsterSegment.wSX = wTailX;
	this->pRoomWidget->monsterSegment.wSY = wTailY;
}

//*****************************************************************************
PlotType CEditRoomScreen::PlotMonsterSegment()
//Plot pieces along a monster segment (look in the room widget for location).
//
//Returns: whether plot was successful.
{
	UINT wX, wY, wTileNo;

	//Only plot tiles in a horizontal or vertical direction.
	//Determine which way to do it.
	CEditRoomWidget *pRW = this->pRoomWidget;
	const bool bHorizontal = pRW->monsterSegment.bHorizontal;
	const UINT wMinX = (bHorizontal ? min(pRW->monsterSegment.wSX,
			pRW->monsterSegment.wEX) : pRW->monsterSegment.wSX);
	const UINT wMinY = (bHorizontal ? pRW->monsterSegment.wSY :
			min(pRW->monsterSegment.wSY,pRW->monsterSegment.wEY));
	const UINT wMaxX = (bHorizontal ? max(pRW->monsterSegment.wSX,
			pRW->monsterSegment.wEX) : wMinX);
	const UINT wMaxY = (bHorizontal ? wMinY : max(pRW->monsterSegment.wSY,
			pRW->monsterSegment.wEY));
	const UINT wDirection = pRW->monsterSegment.wDirection;

	//Make sure plotting is legal on all spots.
	for (wY=wMinY; wY<=wMaxY; wY++)
		for (wX=wMinX; wX<=wMaxX; wX++)
			if (!pRW->IsSafePlacement(this->wSelectedObject,wX,wY))
				return PLOT_NOTHING;

	bool bHeadPlotted = false, bTailPlotted = false, bSegmentPlotted = false;
	//Plot head first.
	//Necessary in order to have a pointer to the monster
	//when placing the segments.
	for (wY=wMinY; wY<=wMaxY; wY++)
		for (wX=wMinX; wX<=wMaxX; wX++)
			if (wX == pRW->monsterSegment.wHeadX &&
					wY == pRW->monsterSegment.wHeadY)
			{
				ASSERT(!bHeadPlotted);
				bHeadPlotted = true;

				CMonster *pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);
				if (pMonster)
				{
					//Head already here.  Update head's direction.
					ASSERT(pMonster->wType == M_SERPENT);
					pMonster->wO = wDirection;
				} else {
					PlotObjectAt(wX, wY, this->wSelectedObject, wDirection);
				}
			}

	UINT wNextStartX, wNextStartY, wTailTileNo;
	bool bTail;	//whether monster tail is being plotted
	for (wY=wMinY; wY<=wMaxY; wY++)
		for (wX=wMinX; wX<=wMaxX; wX++)
		{
			bTail = false;
			//Calculate tile to plot.
			if (wX == pRW->monsterSegment.wHeadX &&
					wY == pRW->monsterSegment.wHeadY)
			{
				//Plot head.  Handled above.
				continue;
			}
			if (wX == pRW->monsterSegment.wSX && wY == pRW->monsterSegment.wSY)
			{
				if ((wX == pRW->monsterSegment.wTailX) &&
						(wY == pRW->monsterSegment.wTailY))
				{
					//Only plotting one tile, and it's the tail:
					//This indicates we're done plotting, so don't do anything.
					return PLOT_DONE;
				} else {
					//Plot a twist (change the tail into a turn).
					//If backtracking, erase the piece.
					wTileNo = this->pRoomWidget->GetSerpentTurnTile(wX,wY,
						wDirection,false);
					bSegmentPlotted = true;
				}
			} else if (wX == pRW->monsterSegment.wTailX &&
					wY == pRW->monsterSegment.wTailY)
			{
				wTileNo = this->pRoomWidget->GetSerpentTailTile(wX,wY,wDirection,
						false);
				bTail = bTailPlotted = true;

				//Plot next segment from this point.
				wNextStartX = wX;
				wNextStartY = wY;
            wTailTileNo = wTileNo;
			} else {
				//Plot straight segment.  If backtracking, erase.
				wTileNo = this->pRoomWidget->GetSerpentStraightTile(wX,wY,
						wDirection,false);
				bSegmentPlotted = true;

				//If backtracking over head, replace it (handled above).
				if (wX == pRW->monsterSegment.wHeadX &&
						wY == pRW->monsterSegment.wHeadY)
					continue;	//don't need to plot anything
			}

			PlotObjectAt(wX, wY, wTileNo,	wDirection);
		}

	this->pRoomWidget->ResetForPaint();

	//Mark every square on the last current monster segment so it can be
	//replaced next plot (and possibly be erased by backing up).
	this->pRoomWidget->ResetPlot();
	if (bTailPlotted)
	{
      UINT wLastSegmentDirection;
		switch (wTailTileNo)
		{
         case T_SNKT_N: wLastSegmentDirection = S; break;
         case T_SNKT_S: wLastSegmentDirection = N; break;
         case T_SNKT_E: wLastSegmentDirection = W; break;
         case T_SNKT_W: wLastSegmentDirection = E; break;
         default: ASSERTP(false, "Bad serpent tail tile."); break;
      }
		PlotLastMonsterSegment(wNextStartX,wNextStartY,wLastSegmentDirection);
	} else {
		//Only a head was placed -- stop now.
		if (bHeadPlotted) return PLOT_HEAD;
	}

	if (bHeadPlotted)
	{
		return (bTailPlotted || bSegmentPlotted ? PLOT_SEGMENT : PLOT_HEAD);
	} else {
		return (bSegmentPlotted ? PLOT_SEGMENT : PLOT_ERROR);
	}
}

//*****************************************************************************
void CEditRoomScreen::PlotObjects()
//Plot objects in a rectangular area.
{
	UINT wX, wY;

	if (!this->pRoomWidget->bMouseInBounds) return;

   //Place objects larger than one square.
	if (SinglePlacement[this->wSelectedObject])
	{
		switch (this->wSelectedObject)
		{
			//O-layer
			case T_OB_1:
				++this->pRoomWidget->wEndX;	//2x2 block
				++this->pRoomWidget->wEndY;
			break;

         //Monsters
			case T_TARMOTHER:
				++this->pRoomWidget->wEndX;	//pair of eyes
			break;

         case T_SERPENT:
				//Can a serpent be placed here?
				if (!this->pRoomWidget->IsSafePlacement(this->wSelectedObject,
						this->pRoomWidget->wStartX, this->pRoomWidget->wStartY))
            {
               //Edit whatever's already here.
  					EditObjects();
					return;
            }

				//Start placing monster segments.
				//The head (and starting position) goes here.
				this->pRoomWidget->monsterSegment.wHeadX =
						this->pRoomWidget->monsterSegment.wSX = 
						this->pRoomWidget->wStartX;
				this->pRoomWidget->monsterSegment.wHeadY =
						this->pRoomWidget->monsterSegment.wSY = 
						this->pRoomWidget->wStartY;

				this->pRoomWidget->ResetPlot();	//no monster pieces placed yet

				RoomChanging();
				SetState(ES_LONGMONSTER);
			return;

			//Placing level start position.
			case T_SWORDSMAN:
				if (!this->pRoomWidget->IsSafePlacement(this->wSelectedObject,
						this->pRoomWidget->wEndX, this->pRoomWidget->wEndY, this->wO))
            {
               //Edit whatever's already here.
  					EditObjects();
					return;
            }

            //If not placing the level start in the level's entrance room,
				//prompt to change the exit room.
				if (this->pRoom->dwRoomID != this->pLevel->dwRoomID)
				{
					g_pTheSound->PlaySoundEffect(SEID_OOF);
					if (ShowYesNoMessage(MID_MoveLevelEntrance) != TAG_YES)
						return;
               if (!SaveRoomToDB())
                  return;
               //Remove saved games for old entrance room, since they're now inapplicable.
               g_pTheDB->SavedGames.DeleteForRoom(this->pLevel->dwRoomID);
					//Move entrance.
					this->pLevel->dwRoomID = this->pRoom->dwRoomID;
					//Update starting room coords.
					DWORD x, y;
					this->pLevel->bGotStartingRoomCoords = false;
					this->pLevel->GetStartingRoomCoords(x,y);
					this->pLevel->Update();
					this->pRoomWidget->ShowSwordsman();
					SetSignTextToCurrentRoom();
					PaintSign();
					UpdateRect();
				}
				else
					this->bRoomDirty = true;	//no room undo

				g_pTheSound->PlaySoundEffect(SEID_CLEAR);

				//Set player starting position.
				this->pRoomWidget->wPX = this->pLevel->wX = this->pRoomWidget->wEndX;
				this->pRoomWidget->wPY = this->pLevel->wY = this->pRoomWidget->wEndY;
				this->pRoomWidget->wPO = this->pLevel->wO = this->wO;
				this->pRoomWidget->DirtyRoom();
			return;
		}

      //Bounds checking.
      if (!this->pRoom->IsValidColRow(this->pRoomWidget->wStartX,this->pRoomWidget->wStartY) ||
            !this->pRoom->IsValidColRow(this->pRoomWidget->wEndX,this->pRoomWidget->wEndY))
         return;

		//If something is already on these squares, edit it.
		for (wY=this->pRoomWidget->wStartY; wY<=this->pRoomWidget->wEndY; ++wY)
			for (wX=this->pRoomWidget->wStartX; wX<=this->pRoomWidget->wEndX; ++wX)
				if (!this->pRoomWidget->IsSafePlacement(this->wSelectedObject, wX, wY))
            {
					EditObjects();
					return;
            }
	}

	if (this->wSelectedObject == T_STAIRS)
	{
		//Staircase is plotted specially.
		PlotStaircase();
	} else {
		RoomChanging();

		bool bSomethingPlotted = false;
		this->pRoomWidget->ResetPlot();
		//Place selected object at each square.
		for (wY=this->pRoomWidget->wStartY; wY<=this->pRoomWidget->wEndY; ++wY)
			for (wX=this->pRoomWidget->wStartX; wX<=this->pRoomWidget->wEndX; ++wX)
				if (PlotObjectAt(wX,wY,this->wSelectedObject,this->wO))
					bSomethingPlotted = true;

		//Play sound effect.  Customize object.
		if (bSomethingPlotted)
		{
			switch (this->wSelectedObject)
			{
				case T_EMPTY:
				case T_FLOOR:
				case T_PIT:
					g_pTheSound->PlaySoundEffect(SEID_BREAKWALL);	break;
				case T_WALL:
				case T_WALL_B:
				case T_OB_1:
				case T_ARROW_NW:
				case T_TRAPDOOR:
					g_pTheSound->PlaySoundEffect(SEID_TRAPDOOR);	break;
				case T_DOOR_C:
				case T_DOOR_M:
				case T_DOOR_R:
				case T_DOOR_Y:
				case T_DOOR_YO:
					g_pTheSound->PlaySoundEffect(SEID_DOOROPEN);	break;
				case T_POTION_I:
				case T_POTION_K:
					g_pTheSound->PlaySoundEffect(SEID_POTION);	break;
				case T_SCROLL:
            {
					g_pTheSound->PlaySoundEffect(SEID_READ);
               const bool bResult = EditScrollText(this->pRoomWidget->wStartX,
                     this->pRoomWidget->wStartY);
               if (!bResult)
                  Undo();  //don't place scroll if text was cancelled
              	break;
            }
				case T_ORB:
					g_pTheSound->PlaySoundEffect(SEID_ORBHIT);	break;
				case T_CHECKPOINT:
					g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);	break;
				case T_ROACH:
				case T_QROACH:
				case T_GOBLIN:
				case T_WWING:
				case T_SPIDER:
				case T_BRAIN:
            case T_NOMONSTER:
					g_pTheSound->PlaySoundEffect(SEID_SPLAT);	break;
				case T_EYE:
					g_pTheSound->PlaySoundEffect(SEID_EVILEYEWOKE);	break;
				case T_TAR:
					//Remove illegal tar formations.
					FixUnstableTar();
				case T_TARMOTHER:
				case T_TARBABY:
					g_pTheSound->PlaySoundEffect(SEID_STABTAR);	break;
			}
			if (this->bShowErrors)
				ShowErrors();
			this->pRoomWidget->ResetForPaint();
      } else {
         RemoveRoomChange();
         if (T_EMPTY != this->wSelectedObject)
            EditObjects();
      }
	}
   this->monstersNotDeleted.clear();
}

//*****************************************************************************
bool CEditRoomScreen::PlotObjectAt(
//Plot object at specified coord, if it's safe to do it.
//Object is either a monster-type, or a room object in the o- or t-layer.
//
//Returns: whether the object was plotted
//
//Params:
	const UINT wX, const UINT wY, const UINT wObject, const UINT wO)	//(in)
{
	static const UINT ARROW_T[] = {T_ARROW_NW, T_ARROW_N, T_ARROW_NE,
		T_ARROW_W, T_EMPTY, T_ARROW_E, T_ARROW_SW, T_ARROW_S, T_ARROW_SE};

	bool bPlotted = false;
	UINT wObjectToPlot;
	switch (wObject)
	{
	case T_ARROW_NW:
		//Select the correct arrow, based on current orientation.
		wObjectToPlot = ARROW_T[this->wO];
		break;
	default:
		wObjectToPlot = wObject;
		break;
	}

	if (this->pRoomWidget->IsSafePlacement(wObjectToPlot, wX, wY))
	{
		CMonster *pMonster, *pTempMonster;
		if (IsMonsterTileNo(wObjectToPlot))
		{
			//Plotting a monster will remove any monster there.
			CCueEvents Ignored;
			pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);
			if (pMonster != this->pLongMonster)
				this->pRoom->KillMonsterAtSquare(wX,wY,Ignored);

			pMonster = this->pRoom->AddNewMonster(
					wObjectToPlot - M_OFFSET, wX, wY);
			ASSERT(pMonster);
			//Set correct monster orientation, etc.
			switch (pMonster->wType)
			{
			case M_BRAIN:
				pMonster->wO = NO_ORIENTATION;
				break;
			case M_TARMOTHER:
				pMonster->wO = NO_ORIENTATION;
				//Change orientation to show the east eye.
				if (wX > 0)
				{
					pTempMonster = this->pRoom->GetMonsterAtSquare(wX-1,wY);
					if (pTempMonster)
					{
						if (pTempMonster->wType == M_TARMOTHER &&
								pTempMonster->wO != S)
							pMonster->wO = S;
					}
				}

				//Put tar under tar mother.
				this->pRoom->Plot(wX,wY,T_TAR);
				break;
			case M_SERPENT:
				pMonster->wO = wO;
				this->pLongMonster = pMonster;	//placing this monster
				break;
			default:
				pMonster->wO = wO;
				break;
			}
			bPlotted = true;
      } else {
         if (EraseAndPlot(wX,wY,wObjectToPlot, true))
            bPlotted = true;
		}
	}
	return bPlotted;
}

//*****************************************************************************
bool CEditRoomScreen::EraseAndPlot(
//Remove object being written over and perform side-effects of its removal.
//Then place object being plotted.
//
//Returns: whether erasure was allowed and plot was made
//
//Params:
   const UINT wX, const UINT wY, //(in)
   const UINT wObjectToPlot,     //(in)
   const bool bFixImmediately)   //(in) default = true
{
	bool bObRemoved=false;	//obstacle tile was removed
	bool bTarRemoved=false;
	bool bStairsRemoved=false;
	if (RemoveObjectAt(wX,wY,wObjectToPlot,
         bObRemoved,bTarRemoved,bStairsRemoved,false))
	{
      if (wObjectToPlot != T_NOMONSTER)
			this->pRoom->Plot(wX,wY,wObjectToPlot,this->pLongMonster);
      if (bFixImmediately)
      {
		   if (bObRemoved)
			   RemoveBadObTiles(wX,wY);
		   if (bTarRemoved)
			   FixUnstableTar();
		   if (bStairsRemoved)
			   FixCorruptStaircase(wX,wY);
      }

      //Keep yellow doors consistent (all touching pieces either open or closed).
      if (wObjectToPlot == T_DOOR_Y || wObjectToPlot == T_DOOR_YO)
      {
         CCoordStack coords;
         this->pRoom->GetAllDoorSquares(wX, wY, coords);
         UINT wCurX, wCurY;

         // Make sure each orb affects a door no more than once
			for (UINT wIndex=0; wIndex<this->pRoom->wOrbCount; ++wIndex)
         {
            bool bFirst = true;
            for (UINT wAgent=0; wAgent<this->pRoom->parrOrbs[wIndex].wAgentCount; ++wAgent)
            {
               if (coords.IsMember(this->pRoom->parrOrbs[wIndex].parrAgents[wAgent].wX,
                        this->pRoom->parrOrbs[wIndex].parrAgents[wAgent].wY))
               {
                  if (bFirst) {
                     bFirst = false;
                  }
                  else
                  {
                     // already have an agent for this orb/door - remove this one.
                     this->pRoom->parrOrbs[wIndex].DeleteAgent(&this->pRoom->parrOrbs[wIndex].parrAgents[wAgent]);
                     // we're removing this one, so we need to back up wAgent by one.
                     wAgent--;
                  }
               }
            }
         }

         while (coords.Pop(wCurX, wCurY))
         {
            const UINT wTileNo = this->pRoom->GetOSquare(wCurX, wCurY);
            ASSERT(wTileNo == T_DOOR_Y || wTileNo == T_DOOR_YO);
            if (wTileNo != wObjectToPlot)
               this->pRoom->Plot(wCurX, wCurY, wObjectToPlot);
         }
      }

      return true;
	}
   return false;
}

//*****************************************************************************
void CEditRoomScreen::PlotStaircase()
//Plot staircase with a wall around the left, bottom and right sides.
//(And nothing on top.)
{
	const UINT wStartX = this->pRoomWidget->wStartX;
	const UINT wStartY = this->pRoomWidget->wStartY;
	UINT wEndX = this->pRoomWidget->wEndX;
	UINT wEndY = this->pRoomWidget->wEndY;
	UINT wX, wY, wObjectNo;

	//3x3 minimum
	if (wEndX - wStartX < 2)
		wEndX = wStartX + 2;
	if (wEndY - wStartY < 2)
		wEndY = wStartY + 2;

	//If all spots aren't safe, don't plot anything
   this->pRoomWidget->ResetPlot();
	for (wY=wStartY; wY<=wEndY; ++wY)
		for (wX=wStartX; wX<=wEndX; ++wX)
		{
			//Determine what part of staircase is being plotted at this square.
			if (wX == wStartX || wX == wEndX || wY == wEndY)
				wObjectNo = T_WALL;
			else if (wY == wStartY)
				continue;	//nothing will be plotted here
			else
				wObjectNo = T_STAIRS;
			if (!this->pRoomWidget->IsSafePlacement(wObjectNo, wX, wY))
         {
            EditObjects();
				return;
         }
		}

	g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);

	RoomChanging();

	//Plot staircase.
	for (wY=wStartY; wY<=wEndY; wY++)
		for (wX=wStartX; wX<=wEndX; wX++)
		{
			//Determine what part of staircase is being plotted at this square.
			if (wX == wStartX || wX == wEndX || wY == wEndY)
				wObjectNo = T_WALL;
			else if (wY == wStartY)
				continue;	//nothing will be plotted here
			else
				wObjectNo = T_STAIRS;

			PlotObjectAt(wX,wY,wObjectNo,this->wO);
		}

   SetDestinationLevel(wStartX + 1, wStartY + 1, wEndX - 1, wEndY - 1, true);

	if (this->bShowErrors)
		ShowErrors();
	this->pRoomWidget->ResetForPaint();
}

//*****************************************************************************
void CEditRoomScreen::EraseObjects()
//Erase objects in the selected rectangular area.
//Only the layer of the currently selected object is affected.
{
	UINT wX, wY;

	if (!this->pRoomWidget->bMouseInBounds) return;

	RoomChanging();

   //Save selected object while erasing.
   const UINT wMenuSelectedObject = this->wSelectedObject;
   const UINT wLayer = this->wSelectedObject == T_SWORDSMAN ? 2 :
         TILE_LAYER[this->wSelectedObject];

   switch (wLayer)
   {
      case 0:
         this->wSelectedObject = T_FLOOR;
         break;
      case 1:
         this->wSelectedObject = T_EMPTY;
         break;
      case 2:
         this->wSelectedObject = T_NOMONSTER;
         break;
   }

	bool bSomethingPlotted = false;
	this->pRoomWidget->ResetPlot();
	//Place selected object at each square.
	for (wY=this->pRoomWidget->wStartY; wY<=this->pRoomWidget->wEndY; ++wY)
		for (wX=this->pRoomWidget->wStartX; wX<=this->pRoomWidget->wEndX; ++wX)
			if (PlotObjectAt(wX,wY,this->wSelectedObject,this->wO))
				bSomethingPlotted = true;
   this->monstersNotDeleted.clear();

	//Play sound effect.
	if (bSomethingPlotted)
	{
		switch (this->wSelectedObject)
		{
			case T_EMPTY:
			case T_FLOOR:
				g_pTheSound->PlaySoundEffect(SEID_BREAKWALL);
            break;
         case T_NOMONSTER:
				g_pTheSound->PlaySoundEffect(SEID_SPLAT);
            break;
		}
		if (this->bShowErrors)
			ShowErrors();
		this->pRoomWidget->ResetForPaint();
   }

   this->wSelectedObject = wMenuSelectedObject;
}

//*****************************************************************************
void CEditRoomScreen::RemoveBadObTiles(
//Remove obstacle tiles when one is removed.
//
//Params:
	const UINT wCX, const UINT wCY)	//(in) Where obstacle was removed
{
	UINT x = wCX, y = wCY, xPos = 0, yPos = 0;

	//Find parts of obstacle to figure out which corner this square is at.
	while (x > 0) {
		if (this->pRoom->GetOSquare(x - 1, wCY) == T_OB_1)
		{
			++xPos;
			--x;
		} else break;
	}
	while (y > 0) {
		if (this->pRoom->GetOSquare(wCX, y - 1) == T_OB_1)
		{
			++yPos;
			--y;
		} else break;
	}
	xPos = xPos % 2;
	yPos = yPos % 2;

	//Remove remaining obstacle pieces.
	UINT wX, wY;
	for (wY=wCY-yPos; wY<=wCY+(1-yPos); wY++)
		for (wX=wCX-xPos; wX<=wCX+(1-xPos); wX++)
			if (this->pRoom->IsValidColRow(wX,wY))
				if (this->pRoom->GetOSquare(wX,wY) == T_OB_1)
					this->pRoom->Plot(wX,wY,T_FLOOR);
}

//*****************************************************************************
void CEditRoomScreen::FixUnstableObstacles()
//Remove non-whole obstacle pieces.
{
	bool bStable;
	UINT wX, wY;
	do {
		bStable = true;
		for (wY=0; wY<this->pRoom->wRoomRows; ++wY)
			for (wX=0; wX<this->pRoom->wRoomCols; ++wX)
			{
				if (this->pRoom->GetOSquare(wX,wY) == T_OB_1)
					if (!IsObstacleWholeAt(wX,wY))
					{
						PlotObjectAt(wX,wY,T_FLOOR,0);
						bStable = false;
					}
			}
	} while (!bStable);
}

//*****************************************************************************
void CEditRoomScreen::FixUnstableTar()
//Remove unstable tar.
{
	bool bStable;
	UINT wX, wY;
	do {
		bStable = true;
		for (wY=0; wY<this->pRoom->wRoomRows; wY++)
			for (wX=0; wX<this->pRoom->wRoomCols; wX++)
			{
				if (this->pRoom->GetTSquare(wX,wY) == T_TAR)
					if (!this->pRoom->IsTarStableAt(wX,wY))
					{
						PlotObjectAt(wX,wY,T_EMPTY,0);
						bStable = false;
					}
			}
	} while (!bStable);
}

//*****************************************************************************
void CEditRoomScreen::FixCorruptStaircase(
//Remove obstacle tiles when one is removed.
//
//Params:
	const UINT wX, const UINT wY)	//(in) Where stair tile was removed
{
	CCoordStack EvalCoords;	//Contains coords to evaluate.
	UINT wMinX = wX, wMaxX = wX, wMinY = wY, wMaxY = wY;	//staircase borders
   const UINT wStairsIndex = this->pRoom->GetExitIndexAt(wX,wY);
   this->pRoom->DeleteExitAtSquare(wX, wY);

	//Push on all squares adjacent to where stair tile was removed.
	if (wX > 0)
		EvalCoords.Push(wX - 1, wY);
	if (wX < this->pRoom->wRoomCols - 1)
		EvalCoords.Push(wX + 1, wY);
	if (wY > 0)
		EvalCoords.Push(wX, wY - 1);
	if (wY < this->pRoom->wRoomRows - 1)
		EvalCoords.Push(wX, wY + 1);

	//Each iteration pops one pair of coordinates for removing stair tiles,
	//and will add coords of adjacent stair tiles for removal.
	//Exits when there are no more coords in stack to evaluate.
	UINT wEvalX, wEvalY;
	while (EvalCoords.Pop(wEvalX, wEvalY))
	{
		ASSERT(this->pRoom->IsValidColRow(wEvalX, wEvalY));

		//Is a stair tile here?
		const UINT wTileNo = this->pRoom->GetOSquare(wEvalX, wEvalY);
		switch (wTileNo)
		{
		case T_STAIRS:
         {
         //Be careful not to delete distinct adjoining staircases.
         const UINT wTemp = this->pRoom->GetExitIndexAt(wEvalX,wEvalY);
         if (wTemp != wStairsIndex && wTemp != static_cast<UINT>(-1))
            break;
         this->pRoom->DeleteExitAtSquare(wEvalX, wEvalY);

			this->pRoom->Plot(wEvalX, wEvalY, T_FLOOR);	//remove stairs

			//Add adjacent (4-neighbor) coords to eval stack.
			if (wEvalX > 0)
				EvalCoords.Push(wEvalX - 1, wEvalY);
			if (wEvalX < this->pRoom->wRoomCols - 1)
				EvalCoords.Push(wEvalX + 1, wEvalY);
			if (wEvalY > 0)
				EvalCoords.Push(wEvalX, wEvalY - 1);
			if (wEvalY < this->pRoom->wRoomRows - 1)
				EvalCoords.Push(wEvalX, wEvalY + 1);

			//Keep track of bounds of staircase to check wall corner tiles.
			if (wEvalX < wMinX)
				wMinX = wEvalX;
			if (wEvalX > wMaxX)
				wMaxX = wEvalX;
			if (wEvalY < wMinY)
				wMinY = wEvalY;
			if (wEvalY > wMaxY)
				wMaxY = wEvalY;
         }
			break;

		case T_WALL:
			//Remove the wall surrounding the stairs if it doesn't
			//have another wall tile adjacent to it.
			FixCorruptStaircaseEdge(wMinX, wMaxX, wMinY, wMaxY, wEvalX, wEvalY);
			break;
		}
	}

	//Handle corners.
	if (wMinX > 0)
	{
		if (wMinY > 0)
			FixCorruptStaircaseEdge(wMinX, wMaxX, wMinY, wMaxY, wMinX-1, wMinY-1);
		if (wMaxY < this->pRoom->wRoomRows - 1)
			FixCorruptStaircaseEdge(wMinX, wMaxX, wMinY, wMaxY, wMinX-1, wMaxY+1);
	}
	if (wMaxX < this->pRoom->wRoomCols - 1)
	{
		if (wMinY > 0)
			FixCorruptStaircaseEdge(wMinX, wMaxX, wMinY, wMaxY, wMaxX+1, wMinY-1);
		if (wMaxY < this->pRoom->wRoomRows - 1)
			FixCorruptStaircaseEdge(wMinX, wMaxX, wMinY, wMaxY, wMaxX+1, wMaxY+1);
	}                                         
}

//*****************************************************************************
void CEditRoomScreen::FixCorruptStaircaseEdge(
//Remove lone wall tiles around a staircase being removed.
//
//Params:
	const UINT wMinX, const UINT wMaxX,				//(in) bounds of stairs
	const UINT wMinY, const UINT wMaxY,				//(in) bounds of stairs
	const UINT wEvalX, const UINT wEvalY)	//(in) Where to check wall tile
{
	ASSERT(this->pRoom->IsValidColRow(wEvalX, wEvalY));

	//This check is needed just for the corner cases.
	if (this->pRoom->GetOSquare(wEvalX, wEvalY) != T_WALL) return;

	if (wEvalX < wMinX && wEvalX > 0)	//left
		if (this->pRoom->GetOSquare(wEvalX-1, wEvalY) == T_WALL)
			return;
	if (wEvalX > wMaxX && wEvalX < this->pRoom->wRoomCols - 1)	//right
		if (this->pRoom->GetOSquare(wEvalX+1, wEvalY) == T_WALL)
			return;
	if (wEvalY < wMinY && wEvalY > 0)	//top (have to handle it)
		if (this->pRoom->GetOSquare(wEvalX, wEvalY-1) == T_WALL)
			return;
	if (wEvalY > wMaxY && wEvalY < this->pRoom->wRoomRows - 1)	//bottom
		if (this->pRoom->GetOSquare(wEvalX, wEvalY+1) == T_WALL)
			return;

	this->pRoom->Plot(wEvalX, wEvalY, T_FLOOR);	//remove wall
}

//*****************************************************************************
bool CEditRoomScreen::RemoveMonster(
//Remove monster.  Remove long monsters on confirmation.
//
//Returns: whether monster was deleted.
//
//Params:
	CMonster *pMonster,
   bool bConfirmation)  //whether user must confirm deletion [default = false]
{
	ASSERT(pMonster);

	if (pMonster->wType == M_SERPENT)
	{
		if (!bConfirmation && (this->monstersNotDeleted.find(pMonster) ==
            this->monstersNotDeleted.end()))
      {
			bConfirmation = (ShowYesNoMessage(MID_EraseLongMonster) == TAG_YES);
         //Don't prompt to delete this monster again
         //(if multiple squares have been queued for removal).
         if (!bConfirmation)
            this->monstersNotDeleted.insert(pMonster);
      }
	}

	if (bConfirmation || pMonster->wType != M_SERPENT)
	{
		CCueEvents Ignored;
		this->pRoom->KillMonsterAtSquare(pMonster->wX,pMonster->wY,Ignored);
		this->pRoomWidget->UpdateFromPlots();
		return true;
	}

	return false;
}

//*****************************************************************************
bool CEditRoomScreen::RemoveObjectAt(
//Removes object at square in layer of replacing object.
//If object is a special one, prompt before removal.
//If removal is not allowed, don't remove it.
//
//Returns: whether object was removed
//
//Params:
	const UINT wX, const UINT wY,	//(in)	Coord
   const UINT wPlottedObject,   //(in)   Object being plotted
	bool &bObRemoved,			//(out)	Obstacle tile was removed
	bool &bTarRemoved,		//(out)	Tar was removed
	bool &bStairsRemoved,	//(out)  Stairs were removed
   const bool bSafetyOn)   //(in) require confirmation on deleting customized
                           // items? (default = true) 
{
	bool bConfirmation = !bSafetyOn;
	const UINT wOTileNo = this->pRoom->GetOSquare(wX,wY),
			wTTileNo = this->pRoom->GetTSquare(wX,wY),
         wLayer = wPlottedObject == T_NOMONSTER ? 2 :
               TILE_LAYER[wPlottedObject];
	CMonster *pMonster;

   switch (wLayer)
   {
   case 0:
	   //O-layer
	   switch (wOTileNo)
	   {
		   case T_OB_1:
			   if (wPlottedObject != T_OB_1)
				   bObRemoved = true;
		   break;

		   case T_STAIRS:
			   if (wPlottedObject != T_STAIRS)
				   bStairsRemoved = true;
		   break;

		   case T_DOOR_Y:
		   case T_DOOR_YO:
		   {
			   UINT wAdjX = static_cast<UINT>(-1), wAdjY = static_cast<UINT>(-1);
			   //If there's an adjacent door piece, move agents for this spot there.
			   //Otherwise, remove all agents attached to this door.
			   for (UINT wIndex=0; wIndex<this->pRoom->wOrbCount; ++wIndex)
			   {
				   COrbAgentData *pAgent = FindOrbAgentFor(wX,wY,
						   this->pRoom->parrOrbs + wIndex);
				   if (!pAgent) continue;
					if (wAdjX == static_cast<UINT>(-1))
					{
						//Check whether there's any door piece adjacent (8-neighbor)
						//to move this agent to.
                  //ATTN: Must check in this defined order to not lose door
                  //associations when there's a region deletion.
						UINT wTX, wTY, wTileNo;
                  static const UINT NEIGHBORS = 8;
                  static const int dx[NEIGHBORS] = {-1, -1,  0,  1, -1, 1, 0, 1};
                  static const int dy[NEIGHBORS] = {-1,  0, -1, -1,  1, 0, 1, 1};
						for (UINT wI=0; wI<NEIGHBORS; ++wI)
                  {
							wTX = wX + dx[wI];
							wTY = wY + dy[wI];
							if (this->pRoom->IsValidColRow(wTX,wTY))
							{
								wTileNo = this->pRoom->GetOSquare(wTX,wTY);
								if (wTileNo == T_DOOR_Y || wTileNo == T_DOOR_YO)
								{
									//Found an adjacent door -- move agents there.
									wAdjX = wTX;
									wAdjY = wTY;
                           break;
								}
							}
						}
					}
					if (wAdjX == static_cast<UINT>(-1))
					{
						if (!bConfirmation)
							bConfirmation = (ShowYesNoMessage(MID_EraseOrbAgent) == TAG_YES);
						if (bConfirmation)
							this->pRoom->parrOrbs[wIndex].DeleteAgent(pAgent);
						else return false;
					} else {
						pAgent->wX = wAdjX;
						pAgent->wY = wAdjY;
					}
			   }
		   }
		   break;
	   }
   break;

   case 1:
	   //T-layer
	   bConfirmation = !bSafetyOn;
	   switch (wTTileNo)
	   {
		   case T_TAR:
			   if (wPlottedObject != T_TAR)
				   bTarRemoved = true;
		   break;

		   case T_SCROLL:
         {
			   const WCHAR *pText = this->pRoom->GetScrollTextAtSquare(wX,wY);
            ASSERT(pText); //should always have scroll text where a scroll is
            if (!pText) break;
            if (WCSlen(pText) == 0)
               bConfirmation = true;
			   if (!bConfirmation)
				   bConfirmation = (ShowYesNoMessage(MID_EraseScroll) == TAG_YES);
			   if (!bConfirmation)
				   return false;
			   this->pRoom->DeleteScrollTextAtSquare(wX,wY);
         }
		   break;
		   case T_ORB:
		   {
			   COrbData *pOrb = this->pRoom->GetOrbAtCoords(wX,wY);
			   if (pOrb)
			   {
				   if (pOrb->wAgentCount == 0)
					   bConfirmation = true;	//can erase an orb w/o agents
				   else
					   if (!bConfirmation)
						   bConfirmation = (ShowYesNoMessage(MID_EraseOrb) == TAG_YES);
				   if (bConfirmation)
					   this->pRoom->DeleteOrbAtSquare(wX,wY);
				   else
					   return false;
			   }
		   }
		   break;
		   default:
		   {
			   //Serpent segments.
			   if (bIsSerpent(wTTileNo))
			   {
				   pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);
				   if (pMonster != this->pLongMonster)
			         if (!RemoveMonster(pMonster))
                     return false;
			   }
		   }
		   break;
	   }
   break;

   case 2:
  	   switch (wPlottedObject)
      {
         case T_NOMONSTER:
		      //Plotting an empty square will remove the monster there.
		      pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);
		      if (pMonster)
		      {
			      if (!RemoveMonster(pMonster))
				      return false;
		      }
         break;
	   }
   break;
   }  //layer

	return true;
}

//*****************************************************************************
void CEditRoomScreen::RemoveRoomChange()
//Removes last undo command from the stack.
{
	delete this->UndoList.back();
	this->UndoList.pop_back();
   SetButtons();
}

//*****************************************************************************
void CEditRoomScreen::RoomChanging()
//Marks the dirty bit
//Saves current room state in the undo list and clears the redo list.
{
	this->bRoomDirty = true;

   if ((int)this->UndoList.size() < this->nUndoSize)
      this->nUndoSize = -1;   //now impossible to revert to clean state

	//Save room state.
	this->UndoList.push_back(new CDbRoom(*(this->pRoom)));

	//Clear redo list.
	ClearList(this->RedoList);

   //Update undo/redo button states.
   SetButtons();
}

//*****************************************************************************
void CEditRoomScreen::SetButtons(const bool bPaint)   //(in) default = true
//Set state of undo/redo buttons.
{
   CButtonWidget *pButton = static_cast<CButtonWidget *>(GetWidget(TAG_UNDO));
   if (this->UndoList.empty())
      pButton->Disable();
   else
      pButton->Enable();
   if (bPaint)
      pButton->Paint();
   pButton = static_cast<CButtonWidget *>(GetWidget(TAG_REDO));
   if (this->RedoList.empty())
      pButton->Disable();
   else
      pButton->Enable();
   if (bPaint)
      pButton->Paint();
}

//*****************************************************************************
void CEditRoomScreen::Redo()
//Redo last room change.
{
	if (this->RedoList.empty())
		return;
	this->UndoList.push_back(this->pRoom);
	this->pRoom = this->RedoList.back();
	this->RedoList.pop_back();
	SetState(ES_PLACING);
   SetButtons();

   this->bRoomDirty = this->UndoList.size() != this->nUndoSize;

   //Refresh room
	this->pRoomWidget->LoadFromRoom(this->pRoom);
	if (this->bShowErrors)
		ShowErrors();
}

//*****************************************************************************
void CEditRoomScreen::Undo()
//Undo last room change.
{
	if (this->UndoList.empty())
		return;
	this->RedoList.push_back(this->pRoom);
	this->pRoom = this->UndoList.back();
	this->UndoList.pop_back();
	SetState(ES_PLACING);
   SetButtons();

   this->bRoomDirty = this->UndoList.size() != this->nUndoSize;

   //Refresh room
	this->pRoomWidget->LoadFromRoom(this->pRoom);
	if (this->bShowErrors)
		ShowErrors();
}

//*****************************************************************************
bool CEditRoomScreen::SaveRoom()
//Updates the current room and level data.
//
//Returns: whether room is current (i.e. not dirty)
{
	ASSERT(this->pRoom);
	ASSERT(this->pLevel);

	if (!this->bRoomDirty)
      return true;	//save not needed

	if (this->bAutoSave)
      SaveRoomToDB();
	else
      if (ShowYesNoMessage(MID_SaveRoomPrompt) == TAG_YES)
         SaveRoomToDB();

   return !this->bRoomDirty;
}

//*****************************************************************************
bool CEditRoomScreen::SaveRoomToDB()
//Updates the current room and level data, if hold is owned by editing player.
//
//Returns: whether player is the author of the hold being edited
{
   CDbHold *pHold = g_pTheDB->Holds.GetByID(this->pLevel->dwHoldID);
   ASSERT(pHold);
   //Does the player own this hold?
   const DWORD dwCurrentPlayerID = g_pTheDB->GetPlayerID();
   ASSERT(dwCurrentPlayerID);
	if (pHold->dwPlayerID != dwCurrentPlayerID)
	{
	   //Editing a room in a hold you don't own will make a modified copy of
      //the hold owned by you.
      if (ShowYesNoMessage(MID_MakeModifiedHoldCopy) != TAG_YES)
      {
         //User doesn't want to make a modified copy of the hold -- don't save anything.
         delete pHold;
         return false;
      }

      SetCursor(CUR_Wait);

      //Copy hold.
		CDbHold *pNewHold = pHold->MakeCopy();
      pNewHold->ChangeAuthor(dwCurrentPlayerID);

      //Get new version of current level and room.
      const DWORD dwNewLevelID = CDbHolds::GetLevelIDAtIndex(this->pLevel->dwLevelIndex, pNewHold->dwHoldID);
      ASSERT(dwNewLevelID);
      CDbLevel *pNewLevel = g_pTheDB->Levels.GetByID(dwNewLevelID);
      ASSERT(pNewLevel);
	   pNewLevel->wX = this->pLevel->wX;   //start position might have changed (but nothing else)
	   pNewLevel->wY = this->pLevel->wY;
	   pNewLevel->wO = this->pLevel->wO;
      pNewLevel->Update();

      const DWORD dwNewRoomY = (pNewLevel->dwLevelID * 100) + (this->pRoom->dwRoomY % 100);
     	CDbRoom *pNewRoom = pNewLevel->GetRoomAtCoords(this->pRoom->dwRoomX, dwNewRoomY);
      ASSERT(pNewRoom);
		const DWORD dwRoomID = pNewRoom->dwRoomID;
		const DWORD dwRoomX = this->pRoom->dwRoomX;
		pNewRoom->MakeCopy(*this->pRoom);   //save modified room (copy scroll texts, and don't overwrite IDs)
		pNewRoom->dwLevelID = pNewLevel->dwLevelID;
		pNewRoom->dwRoomID = dwRoomID;
		pNewRoom->dwRoomX = dwRoomX;
		pNewRoom->dwRoomY = dwNewRoomY;
      pNewRoom->Update();

      SetRoom(pNewRoom->dwRoomID);

      //Update widgets in main Editor screen.
		CEditSelectScreen *pEditSelectScreen = DYN_CAST(CEditSelectScreen *, CScreen *,
				g_pTheSM->GetLoadedScreen(SCR_EditSelect));
      ASSERT(pEditSelectScreen);
      pEditSelectScreen->SetToCopiedHold(pNewHold, pNewLevel);

      delete pNewRoom;
      delete pNewLevel;
      delete pNewHold;
   } else {
      SetCursor(CUR_Wait);

      //Update hold's timestamp (even if nothing was changed).
	   pHold->Update();

      this->pRoom->Update();
	   this->pLevel->Update();

      //Delete all demos/saved games in room, since it's been modified and they might have broken.
      g_pTheDB->SavedGames.DeleteForRoom(this->pRoom->dwRoomID);
   }
   delete pHold;

   g_pTheDB->Commit();

   this->bRoomDirty = false;

   this->pMapWidget->UpdateFromCurrentLevel();

   SetCursor();

   return true;
}

//*****************************************************************************
void CEditRoomScreen::SetSignTextToCurrentRoom()
//Set sign text to description of current room and repaint it.
{
	static const WCHAR wszSignSep[] = { W_t(':'), W_t(' '), W_t(0) };
	WSTRING wstrSignText = (const WCHAR *)this->pLevel->NameText;
	wstrSignText += wszSignSep;
	this->pRoom->GetLevelPositionDescription(wstrSignText);
	switch (this->eState)
	{
	case ES_SCROLL:
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_EnteringScrollStatus);
		break;
	case ES_LONGMONSTER:
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_PlacingLongMonsterStatus);
		break;
	case ES_ORB:
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_DefiningOrbStatus);
		break;
	case ES_DOOR:
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_DefiningDoorStatus);
		break;
	case ES_TESTROOM:
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_TestRoomLocation);
		break;
	default:
		break;
	}
	SetSignText(wstrSignText.c_str());
}

//*****************************************************************************
void CEditRoomScreen::SetState(
//Set editing state and update the screen accordingly.
//
//Params:
	const EditState eState)		//(in)	New state.
{
   if (eState == this->eState) return; //no change

	if (this->eState == ES_SCROLL)
		HideScroll();

   if (eState == ES_PLACING || eState == ES_TESTROOM)
	{
		this->pOrb = NULL;
		this->pLongMonster = NULL;
		this->pRoomWidget->bPlacing = true;
		this->pRoomWidget->ResetPlot();
		this->pRoomWidget->wOX = static_cast<UINT>(-1);
		this->pRoomWidget->ClearEffects();
	} else {
		this->pRoomWidget->bPlacing = false;
	}

	//Restore any saved selected object on state change.
	if (this->wSelectedObjectSave != static_cast<UINT>(-1))
	{
		this->wSelectedObject = this->wSelectedObjectSave;
		this->wSelectedObjectSave = static_cast<UINT>(-1);
	}

	this->eState = eState;

   this->pEffects->RemoveEffectsOfType(ETOOLTIP);
	SetSignTextToCurrentRoom();
	PaintSign();
}

//************************************************************************************
bool CEditRoomScreen::SetUnspecifiedPlayerSettings(
//Any player setting that is unspecified will get a default value.
//This is where the default settings for the editor are defined.
//
//Params:
	CDbPackedVars	&Settings)	//(in/out)	When received, param may contain any number
								//			of vars that match or don't match expected
								//			vars.  Returned with all expected vars set.
{
	bool bUpdate = false;
	//Set-if-missing macros.
#	define SETMISSING(name, value) if (!Settings.DoesVarExist(name))\
		{Settings.SetVar(name, value);	bUpdate = true;}

	SETMISSING("ShowErrors", true);

#	undef SETMISSING

	return bUpdate;
}

//*****************************************************************************
void CEditRoomScreen::ShowErrors()
//Highlight room squares with errors.
{
	static const SURFACECOLOR Red = {255, 0, 0};
	static const SURFACECOLOR Blue = {0, 0, 255};
	static const SURFACECOLOR LightRed = {255, 128, 128};
	static const SURFACECOLOR LightGreen = {128, 255, 128};
	static const SURFACECOLOR Orange = {255, 128, 0};

	UINT wTileNo[3], wAdjTile[3];
	bool bMatchingEdge;
	bool bRedDoors = false, bTrapdoors = false;	//first must imply second
	bool bGreenDoors = false, bMonsters = false;	//" "
	CMonster *pMonster;
	UINT wX, wY;

#define AddShadeToTile(Color) this->pRoomWidget->AddShadeEffect(wX,wY,Color)
#define AddShadeToTileXY(x,y,Color) this->pRoomWidget->AddShadeEffect(x,y,Color)
#define ClosedEdge(wTileNo) (wTileNo[0] == T_WALL || wTileNo[0] == T_OB_1 || wTileNo[0] == T_PIT || \
      wTileNo[1] == T_ORB || wTileNo[1] == T_TAR || bIsSerpent(wTileNo[1]))

   if (!this->bReadyToPaste) this->pRoomWidget->RemoveLastLayerEffectsOfType(ESHADE);

	//Load adjacent rooms.
	CDbRoom *pAdjRoom[4];
	{
		pAdjRoom[0] = g_pTheDB->Rooms.GetByCoords(this->pRoom->dwLevelID,
				this->pRoom->dwRoomX,this->pRoom->dwRoomY-1);
		pAdjRoom[1] = g_pTheDB->Rooms.GetByCoords(this->pRoom->dwLevelID,
				this->pRoom->dwRoomX+1,this->pRoom->dwRoomY);
		pAdjRoom[2] = g_pTheDB->Rooms.GetByCoords(this->pRoom->dwLevelID,
				this->pRoom->dwRoomX,this->pRoom->dwRoomY+1);
		pAdjRoom[3] = g_pTheDB->Rooms.GetByCoords(this->pRoom->dwLevelID,
				this->pRoom->dwRoomX-1,this->pRoom->dwRoomY);
	}

   //Door tile compiling is expensive, so keep track of which door tiles
   //have been checked in order to check each door only once.
   CCoordIndex yellowDoorTiles(this->pRoom->wRoomCols, this->pRoom->wRoomRows);

	UINT wSquareIndex = 0;
	for (wY=0; wY<this->pRoom->wRoomRows; wY++)
		for (wX=0; wX<this->pRoom->wRoomCols; wX++)
		{
			//Verify one square.
			wTileNo[0] = this->pRoom->pszOSquares[wSquareIndex];
			wTileNo[1] = this->pRoom->pszTSquares[wSquareIndex];
			pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);
			if (pMonster)
			{
				wTileNo[2] = pMonster->wType + M_OFFSET;
				bMonsters = true;
			} else {
				wTileNo[2] = 0;
			}

			//Check general tile compatibility.
			if (!this->pRoomWidget->IsSafePlacement(wTileNo[0], wX, wY, NO_ORIENTATION, true))
				AddShadeToTile(Red);
			else if (!this->pRoomWidget->IsSafePlacement(wTileNo[1], wX, wY, NO_ORIENTATION, true))
				AddShadeToTile(Red);
			else if (pMonster)
				if (!this->pRoomWidget->IsSafePlacement(wTileNo[2], wX, wY, NO_ORIENTATION, true))
					AddShadeToTile(Red);

			//Check for rules on specific objects.
			switch (wTileNo[0])
			{
			case T_OB_1:
				//Obstacles should cover a 2x2 area.
				if (!IsObstacleWholeAt(wX,wY))
					AddShadeToTile(Red);
				break;
			case T_DOOR_M:
				bGreenDoors = true;
				break;
			case T_DOOR_R:
				bRedDoors = true;
				break;
			case T_TRAPDOOR:
				bTrapdoors = true;
				break;
			case T_DOOR_Y:
			case T_DOOR_YO:
				{
					//Each door should have an orb that affects it.  Mark doors w/o one.

               //This tile is part of a door that was already checked -- skip it.
               if (yellowDoorTiles.Exists(wX,wY)) break;

               //Gather set of all squares this door is on.
	            CCoordStack doorCoords, coords;
               this->pRoom->GetAllDoorSquares(wX, wY, doorCoords);

               doorCoords.AddTo(yellowDoorTiles);

               //Determine whether an orb agent is connected to this door tile.
               bool bFound = false;
               UINT wIndex;
					for (wIndex=0; wIndex<this->pRoom->wOrbCount; ++wIndex)
               {
                  coords = doorCoords;
						if (FindOrbAgentFor(this->pRoom->parrOrbs + wIndex, coords))
                  {
							bFound = true;
                     break;
                  }
               }
					if (!bFound)
               {
                  //Shade entire door.
                  UINT x, y;
                  while (doorCoords.Pop(x, y))
						   AddShadeToTileXY(x, y, Orange);
               }
					this->pRoomWidget->ResetPlot();
				}
				break;
			}

			switch (wTileNo[1])
			{
			case T_ORB:
				//Orb probably should affect some doors.  (Mark ones that don't.)
				{
					COrbData *pOrb = this->pRoom->GetOrbAtCoords(wX,wY);
					if (!pOrb)
						AddShadeToTile(Orange);
					else if (pOrb->wAgentCount == 0)
						AddShadeToTile(Orange);
				}
				break;

			case T_TAR:
				//Tar should conform to certain constraints.
				if (!this->pRoom->IsTarStableAt(wX,wY))
					AddShadeToTile(LightRed);
				break;
			}

			switch (wTileNo[2])
			{
				case T_TARMOTHER:
				{
					//Eyes should probably be paired.
					const UINT wAdjX = (pMonster->wO == S ? wX - 1 : wX + 1);
					if (!this->pRoom->IsValidColRow(wAdjX,wY))
						AddShadeToTile(Red);
					else
               {
						CMonster *pAdjMonster = this->pRoom->GetMonsterAtSquare(
								wAdjX,wY);
						if (!pAdjMonster)
							AddShadeToTile(Red);
						else if (pAdjMonster->wType != M_TARMOTHER)
							AddShadeToTile(Red);
						//else if (pAdjMonster->wO == pMonster->wO)	//same eye
						//	AddShadeToTile(Green);
					}
				}
				break;
			}

			//Check room edges.
			//Mark tile if it's inconsistent with the adjacent room's edge.
			if (wX == 0)
			{
				wAdjTile[0] = (pAdjRoom[3] ? pAdjRoom[3]->GetOSquare(
						pAdjRoom[3]->wRoomCols-1,wY) : T_WALL);
            wAdjTile[1] = (pAdjRoom[3] ? pAdjRoom[3]->GetTSquare(
               pAdjRoom[3]->wRoomCols-1,wY) : T_ORB);
				bMatchingEdge = ClosedEdge(wTileNo) ==	ClosedEdge(wAdjTile);
				if (!bMatchingEdge)
					AddShadeToTile(Blue);
			}
			if (wX == this->pRoom->wRoomCols-1)
			{
				wAdjTile[0] = (pAdjRoom[1] ? pAdjRoom[1]->GetOSquare(0,wY) : T_WALL);
            wAdjTile[1] = (pAdjRoom[1] ? pAdjRoom[1]->GetTSquare(0,wY) : T_ORB);
				bMatchingEdge = ClosedEdge(wTileNo) ==	ClosedEdge(wAdjTile);
				if (!bMatchingEdge)
					AddShadeToTile(Blue);
			}
			if (wY == 0)
			{
				wAdjTile[0] = (pAdjRoom[0] ? pAdjRoom[0]->GetOSquare(wX,
						pAdjRoom[0]->wRoomRows-1) : T_WALL);
            wAdjTile[1] = (pAdjRoom[0] ? pAdjRoom[0]->GetTSquare(wX,
						pAdjRoom[0]->wRoomRows-1) : T_ORB);
				bMatchingEdge = ClosedEdge(wTileNo) ==	ClosedEdge(wAdjTile);
				if (!bMatchingEdge)
					AddShadeToTile(Blue);
			}
			if (wY == this->pRoom->wRoomRows-1)
			{
				wAdjTile[0] = (pAdjRoom[2] ? pAdjRoom[2]->GetOSquare(wX,0) : T_WALL);
				wAdjTile[1] = (pAdjRoom[2] ? pAdjRoom[2]->GetTSquare(wX,0) : T_ORB);
				bMatchingEdge = ClosedEdge(wTileNo) ==	ClosedEdge(wAdjTile);
				if (!bMatchingEdge)
					AddShadeToTile(Blue);
			}

			++wSquareIndex;
		}

		//Check for inconsistencies in general room state.
		if ((bRedDoors && !bTrapdoors) || (bGreenDoors && !bMonsters))
		{
			wSquareIndex = 0;
			for (wY=0; wY<this->pRoom->wRoomRows; wY++)
				for (wX=0; wX<this->pRoom->wRoomCols; wX++)
				{
					//Verify one square.
					wTileNo[0] = this->pRoom->pszOSquares[wSquareIndex];
					switch (wTileNo[0])
					{
					case T_DOOR_M:
						if (bGreenDoors && !bMonsters)
							AddShadeToTile(LightGreen);	//green door w/o monsters
						break;
					case T_DOOR_R:
						if (bRedDoors && !bTrapdoors)
							AddShadeToTile(LightRed);	//red door w/o trapdoors
						break;
					}

					++wSquareIndex;
				}
		}

		delete pAdjRoom[0];
		delete pAdjRoom[1];
		delete pAdjRoom[2];
		delete pAdjRoom[3];

#undef AddShadeToTile
#undef AddShadeToTileXY
#undef ClosedEdge
}

//************************************************************************************
void CEditRoomScreen::UpdatePlayerDataFromWidgets()
//Synchronizes current player data with widgets on screen.
{
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	if (!pCurrentPlayer) {ASSERTP(false, "Could not retrieve player."); return;} //Corrupt db.

	COptionButtonWidget *pOptionButton = static_cast<COptionButtonWidget *>(
			GetWidget(TAG_SHOWERRORS));
	pCurrentPlayer->Settings.SetVar("ShowErrors", pOptionButton->IsChecked());

	pCurrentPlayer->Update();

	delete pCurrentPlayer;
}

// $Log: EditRoomScreen.cpp,v $
// Revision 1.92  2005/04/13 16:06:46  gjj
// Some small fixes
//
// Revision 1.91  2005/03/15 21:42:22  mrimer
// Fixed bug: dirty flag not reset when room is reloaded.
//
// Revision 1.90  2004/08/10 02:04:58  mrimer
// Fixed scroll text copy bugs when a room is copied.
//
// Revision 1.89  2004/08/08 21:29:02  mrimer
// Fixed bug: room saves not always occurring when needed.
//
// Revision 1.88  2004/06/08 14:20:12  mrimer
// Fixed bug: orb agents can be lost when doors have >1 piece deleted on right/bottom sides.
//
// Revision 1.87  2004/05/20 17:43:37  mrimer
// Fixed bug: repeating region cut/copy-and-pastes when key is held down.
//
// Revision 1.86  2004/01/13 01:26:16  mrimer
// Fixed possible editor bug.
//
// Revision 1.85  2004/01/03 00:05:24  mrimer
// Fixed bug: new room has inconsistent filled in corner squares.
//
// Revision 1.84  2003/12/01 19:17:19  mrimer
// Allowed multiple potion placement.
//
// Revision 1.83  2003/12/01 19:15:07  mrimer
// Fixed bug: not confirming serpent deletion makes later deletion impossible.
//
// Revision 1.82  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.81  2003/10/10 16:04:37  mrimer
// Fixed cosmetic bugs: 'Neather marked as placement error; error highlights not redrawn when exiting orb agent placement.
//
// Revision 1.80  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.79  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.78  2003/09/22 22:31:26  mrimer
// Fixed bug: pasting obstacles cause strange behavior.
//
// Revision 1.77  2003/09/22 16:56:22  mrimer
// Fixed bug: Playtest starts on invalid objects on no-save.
//
// Revision 1.76  2003/09/08 21:53:10  mrimer
// Fixed bug: repainting screen invalidates placing serpent segments.
//
// Revision 1.75  2003/08/20 22:20:06  mrimer
// Now also delete saved games in previous entrance room when level entrance is moved.
//
// Revision 1.74  2003/08/16 01:54:18  mrimer
// Moved EffectList to FrontEndLib.  Added general tool tip support to screens.  Added RoomEffectList to room editor screen/widget.
//
// Revision 1.73  2003/08/09 00:38:53  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.72  2003/08/06 01:08:23  mrimer
// Fixed bug: level exit IDs not being updated on copy.
//
// Revision 1.71  2003/08/04 16:28:00  mrimer
// Fixed bug: undo widgets not updating when room changes are discarded on playtesting.
//
// Revision 1.70  2003/08/04 15:38:24  mrimer
// Removed unneeded parameter.
//
// Revision 1.69  2003/08/02 03:32:49  schik
// Bug fixes for copy rectangle not being drawn correctly
//
// Revision 1.68  2003/08/02 01:28:52  mrimer
// Removed DB ref checks.
//
// Revision 1.67  2003/08/01 23:59:56  mrimer
// Fixed bugs relating to making a hold copy from the room editor.
//
// Revision 1.66  2003/08/01 21:43:17  mrimer
// Fixed bug: wrong room coords cause crash when room in another's hold is edited.
//
// Revision 1.65  2003/07/30 04:25:02  mrimer
// Remove scroll if text entry is cancelled on scroll placement.  Fixed Beethro placement logic errors.
//
// Revision 1.64  2003/07/29 03:56:00  mrimer
// Optimized error checking of long yellow doors.
//
// Revision 1.63  2003/07/28 20:44:27  schik
// You can now delete objects with a right click if the swordsman is the selected object
//
// Revision 1.62  2003/07/24 01:33:21  mrimer
// Now remove exit level records when a staircase is deleted.  Don't allow distinct adjoining staircases to be deleted together.  Fixed problems with implicit exit regions.
//
// Revision 1.61  2003/07/21 22:10:20  mrimer
// Changed SaveRoomTODB() to return a bool.  Require making personal hold copy before modifying any rooms on map.  Fixed display bug: yellow doors on room edge.
//
// Revision 1.60  2003/07/19 22:51:14  erikh2000
// Twiddling.
//
// Revision 1.59  2003/07/19 21:29:21  mrimer
// Fixed bug: scroll text not being copied; user prompted multiple times to delete a serpent.
//
// Revision 1.58  2003/07/19 02:22:06  mrimer
// Now orbs and tar are also considered as closed room edges.
//
// Revision 1.57  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.56  2003/07/16 07:45:16  mrimer
// Fixed bugs: pasted room not appearing; staircase has multiple destinations.  Revised selecting stair destination UI.
//
// Revision 1.55  2003/07/15 00:37:16  mrimer
// Fixed room region copy errors and faulty modified hold copying errors.
//
// Revision 1.54  2003/07/11 18:55:25  mrimer
// Fixed bug: room not saving after undoing/redoing.  Fixed bug: wrong initial selected item.
//
// Revision 1.53  2003/07/11 18:31:52  mrimer
// Rearranged the menu items.
//
// Revision 1.52  2003/07/11 17:29:45  mrimer
// Changed SetDestinationLevel() to receive the stair region as an explicit parameter.
// Implemented pasting the marked room region multiple times.  Highlight this region when about to paste.
//
// Revision 1.51  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.50  2003/07/10 04:20:17  mrimer
// Fixed a bug in PasteRegion().
//
// Revision 1.49  2003/07/09 21:14:34  mrimer
// Fixed a couple bugs.  Changed reinterpret_casts to static/dynamic_casts.
//
// Revision 1.48  2003/07/07 23:30:44  mrimer
// Now a copy of another's hold is made for your player when modified.
//
// Revision 1.47  2003/07/06 04:56:59  mrimer
// Added SaveRoomToDB() with more correct handling of room saving logic.
//
// Revision 1.46  2003/07/02 01:54:49  mrimer
// Made SetRoom() error handling more robust.  Changed return type to bool.
//
// Revision 1.45  2003/07/01 23:45:25  mrimer
// Fixed bug: effects remaining when exiting editor states.  Allowed placing multiple brains simultaneously.
//
// Revision 1.44  2003/07/01 13:36:17  schik
// If two or more doors affected by the same orb are joined (by adding new door tiles), all but one orb agent will be removed.
//
// Revision 1.43  2003/06/30 19:35:22  mrimer
// Fixed some DB access bugs.
//
// Revision 1.42  2003/06/29 04:37:58  schik
// Current player settings are copied into the temp player when testing a room.
//
// Revision 1.41  2003/06/28 19:25:47  mrimer
// Fixed bug: clicking undo while editing door/orb creates lightning storm.
//
// Revision 1.40  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.39  2003/06/27 19:16:11  mrimer
// Enabled checkpoints during play-testing (using temporary player that gets deleted after playtesting).
//
// Revision 1.38  2003/06/26 17:58:45  mrimer
// Added SetDestinationLevel().  Prompt for stairway destination on placement.
//
// Revision 1.37  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.36  2003/06/24 01:58:46  schik
// Turned safety on when deleting a tile, so stairs with holes, etc. can't be created.
//
// Revision 1.35  2003/06/22 13:02:14  schik
// Fixed bug:  If you select T_EMPTY and click on a serpent, and respond "No" to the dialog, it tried to edit the serpent.
//
// Revision 1.34  2003/06/21 10:44:16  mrimer
// Added code to copy and paste screen regions.  Removed saving some noop-commands in undo list.
//
// Revision 1.33  2003/06/21 06:54:00  mrimer
// Completed door agent designation.  Revised some door traversal code and effects.
//
// Revision 1.32  2003/06/21 04:10:59  schik
// Orbs affecting a door are now colored according to the toggle/open/close action
//
// Revision 1.31  2003/06/20 23:56:55  mrimer
// Fixed bug: serpent placement not being erased on ESC.
// Moved tar to middle layer menu.
//
// Revision 1.30  2003/06/20 00:48:31  mrimer
// Fixed bug: can't edit staircase destination.
//
// Revision 1.29  2003/06/19 22:22:17  mrimer
// Fixed bug: yellow doors of different state shouldn't be adjacent.
// Added overwriting orientable objects of the same object type.
//
// Revision 1.28  2003/06/19 20:12:24  schik
// Clicking on an existing door will show its orb agents.  Agents can then be edited by clicking on orbs.
//
// Revision 1.27  2003/06/19 04:09:57  mrimer
// Fixed some placement logic bugs.
//
// Revision 1.26  2003/06/18 04:11:18  mrimer
// After deleting tar, unstable tar no longer turns into tar babies.
//
// Revision 1.25  2003/06/18 03:40:33  mrimer
// Refined SetButtons() painting.  Fixed some object placement bugs.
//
// Revision 1.24  2003/06/17 23:13:57  mrimer
// Code maintenance -- ShowTextInputMessage() now requires text entry by default.
//
// Revision 1.23  2003/06/17 18:21:35  mrimer
// Added Undo/Redo buttons.  Revised item placement/customization/deletion UI.
//
// Revision 1.22  2003/06/16 21:53:30  mrimer
// Fixed an orb agent bug.
//
// Revision 1.21  2003/06/16 20:39:22  mrimer
// Added: mouse wheel changes orientation.
//
// Revision 1.20  2003/06/16 20:03:22  mrimer
// Fixed bug: unintentional orb agent deletions.
// Added tile to monster menu to delete only monsters.
//
// Revision 1.19  2003/06/16 18:50:19  mrimer
// Removed incorrect room highlighting when performing right-button drag.
//
// Revision 1.18  2003/06/09 23:54:26  mrimer
// Now dirty bit gets reset after saving.
//
// Revision 1.17  2003/06/09 19:30:25  mrimer
// Fixed some level editor bugs.
//
// Revision 1.16  2003/05/25 22:48:11  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.15  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.14  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.13  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.12  2003/05/08 22:04:41  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.11  2003/05/03 23:35:29  mrimer
// Added removal of a corrupted staircase.
// Now, when a room in a hold is edited, the player becomes the author.  Hold texts are revised.
//
// Revision 1.10  2003/04/28 14:24:16  mrimer
// Fixed bug with IsRequired room flag.
//
// Revision 1.9  2003/04/08 13:08:26  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.8  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.7  2003/02/16 20:32:18  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.6  2003/01/08 00:58:40  mrimer
// Added unloading a playtesting game session when the screen is left.
//
// Revision 1.5  2002/12/22 02:43:56  mrimer
// Modified handling of level list dialog box to optionally include levels from other holds.
//
// Revision 1.4  2002/11/23 00:35:50  mrimer
// Fixed some bugs.
//
// Revision 1.3  2002/11/22 22:07:36  mrimer
// Added support for cutting, copying, and pasting rooms from the map.
// Revised tar placement rules.  Added FixUnstableTar().
// Fixed some bugs.
//
// Revision 1.2  2002/11/22 02:39:27  mrimer
// Added support for playtesting, displaying tool tips, special handling of obstacles and staircases, and revised placed serpents.
// Added hotspots at text in lower-left corner.  Removed the "Autosave" checkbox.
// Added more sound distinction for placing objects.
// Fixed some bugs.
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
