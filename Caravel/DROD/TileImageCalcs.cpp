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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Richard Cookney (timeracer), Michael Welsh Duggan (md5i), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "TileImageCalcs.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/GameConstants.h"
#include "../DRODLib/DbRooms.h"
#include <BackEndLib/Assert.h>

//Definitions.
const UINT MonsterTileImageArray[MONSTER_TYPES][ORIENTATION_COUNT] = {
	//nw							n								ne
	//w								none						e
	//sw							s								se

	//M_ROACH
   {
	TI_ROACH_NW,			TI_ROACH_N,			TI_ROACH_NE,
	TI_ROACH_W,				DONT_USE,				TI_ROACH_E,
	TI_ROACH_SW,			TI_ROACH_S,			TI_ROACH_SE
   },

	//M_QROACH
   {
	TI_QROACH_NW,			TI_QROACH_N,		TI_QROACH_NE,
	TI_QROACH_W,			DONT_USE,				TI_QROACH_E,
	TI_QROACH_SW,			TI_QROACH_S,		TI_QROACH_SE
   },

	//M_REGG (Egg deposited at 8 o'clock, born at midnight.)
   {
	TI_REGG_3,				TI_REGG_4,			DONT_USE,
	TI_REGG_2,				DONT_USE,				DONT_USE,
	TI_REGG_1,					DONT_USE,				DONT_USE
   },

	//M_GOBLIN
   {
	TI_GOBLIN_NW,			TI_GOBLIN_N,		TI_GOBLIN_NE,
	TI_GOBLIN_W,			DONT_USE,				TI_GOBLIN_E,
	TI_GOBLIN_SW,			TI_GOBLIN_S,		TI_GOBLIN_SE
   },

	//M_NEATHER
   {
	TI_NTHR_NW,				TI_NTHR_N,			TI_NTHR_NE,
	TI_NTHR_W,				DONT_USE,				TI_NTHR_E,
	TI_NTHR_SW,				TI_NTHR_S,			TI_NTHR_SE
   },

	//M_WWING
   {
	TI_WW_NW,					TI_WW_N,				TI_WW_NE,
	TI_WW_W,					DONT_USE,				TI_WW_E,
	TI_WW_SW,					TI_WW_S,				TI_WW_SE
   },

	//M_EYE
   {
	TI_EYE_NW,				TI_EYE_N,				TI_EYE_NE,
	TI_EYE_W,					DONT_USE,				TI_EYE_E,
	TI_EYE_SW,				TI_EYE_S,				TI_EYE_SE
   },

	//M_SERPENT
   {
	DONT_USE,					TI_SNK_N,				DONT_USE,
	TI_SNK_W,					DONT_USE,				TI_SNK_E,
	DONT_USE,					TI_SNK_S,				DONT_USE
   },

	//M_TARMOTHER
   {
	DONT_USE,					DONT_USE,				DONT_USE,
	TI_TAREYE_WC,			TI_TAREYE_WO,				DONT_USE,
	TI_TAREYE_EC,			TI_TAREYE_EO,				DONT_USE
   },

	//M_TARBABY
   {
	TI_TARBABY,				TI_TARBABY,			TI_TARBABY,
	TI_TARBABY,				DONT_USE,			TI_TARBABY,
	TI_TARBABY,				TI_TARBABY,			TI_TARBABY
   },

	//M_BRAIN
   {
	DONT_USE,					DONT_USE,				DONT_USE,
	DONT_USE,					TI_BRAIN,				DONT_USE,
	DONT_USE,					DONT_USE,				DONT_USE
   },

	//M_MIMIC
   {
	TI_MIMIC_NW,			TI_MIMIC_N,			TI_MIMIC_NE,
	TI_MIMIC_W,				DONT_USE,				TI_MIMIC_E,
	TI_MIMIC_SW,			TI_MIMIC_S,			TI_MIMIC_SE
   },

	//M_SPIDER
   {
	TI_SPIDER_NW,			TI_SPIDER_N,		TI_SPIDER_NE,
	TI_SPIDER_W,			DONT_USE,				TI_SPIDER_E,
	TI_SPIDER_SW,			TI_SPIDER_S,		TI_SPIDER_SE
   }
};

const UINT AnimatedMonsterTileImageArray[MONSTER_TYPES][ORIENTATION_COUNT] = {
	//nw							n								ne
	//w								none						e
	//sw							s								se

	//M_ROACH
   {
	TI_ROACH_ANW,			TI_ROACH_AN,		TI_ROACH_ANE,
	TI_ROACH_AW,			DONT_USE,				TI_ROACH_AE,
	TI_ROACH_ASW,			TI_ROACH_AS,		TI_ROACH_ASE
   },

	//M_QROACH
   {
	TI_QROACH_ANW,		TI_QROACH_AN,		TI_QROACH_ANE,
	TI_QROACH_AW,			DONT_USE,				TI_QROACH_AE,
	TI_QROACH_ASW,		TI_QROACH_AS,		TI_QROACH_ASE
   },

	//M_REGG (Egg deposited at 8 o'clock, born at midnight.)
   {
	TI_REGG_A3,				TI_REGG_A4,			DONT_USE,
	TI_REGG_A2,				DONT_USE,				DONT_USE,
	TI_REGG_A1,					DONT_USE,				DONT_USE
   },

	//M_GOBLIN
   {
	TI_GOBLIN_ANW,		TI_GOBLIN_AN,		TI_GOBLIN_ANE,
	TI_GOBLIN_AW,			DONT_USE,				TI_GOBLIN_AE,
	TI_GOBLIN_ASW,		TI_GOBLIN_AS,		TI_GOBLIN_ASE
   },

	//M_NEATHER
   {
	TI_NTHR_NW,				TI_NTHR_N,			TI_NTHR_NE,
	TI_NTHR_W,				DONT_USE,				TI_NTHR_E,
	TI_NTHR_SW,				TI_NTHR_S,			TI_NTHR_SE
   },

	//M_WWING
   {
	TI_WW_ANW,				TI_WW_AN,				TI_WW_ANE,
	TI_WW_AW,					DONT_USE,				TI_WW_AE,
	TI_WW_ASW,				TI_WW_AS,				TI_WW_ASE
   },

	//M_EYE
   {
	TI_EYE_ANW,				TI_EYE_AN,			TI_EYE_ANE,
	TI_EYE_AW,					DONT_USE,			TI_EYE_AE,
	TI_EYE_ASW,				TI_EYE_AS,			TI_EYE_ASE
   },

	//M_SERPENT
   {
	DONT_USE,					TI_SNK_AN,			DONT_USE,
	TI_SNK_AW,				DONT_USE,			TI_SNK_AE,
	DONT_USE,					TI_SNK_AS,			DONT_USE
   },

	//M_TARMOTHER
   {
	DONT_USE,					DONT_USE,				DONT_USE,
	TI_TAREYE_WC,			TI_TAREYE_WO,				DONT_USE,
	TI_TAREYE_EC,			TI_TAREYE_EO,				DONT_USE
   },

	//M_TARBABY
   {
	TI_TARBABY_A,			TI_TARBABY_A,		TI_TARBABY_A,
	TI_TARBABY_A,			TI_TARBABY_A,		TI_TARBABY_A,
	TI_TARBABY_A,			TI_TARBABY_A,		TI_TARBABY_A
   },

	//M_BRAIN
   {
	DONT_USE,					DONT_USE,				DONT_USE,
	DONT_USE,					TI_BRAIN_A,			DONT_USE,
	DONT_USE,					DONT_USE,				DONT_USE
   },

	//M_MIMIC
   {
	TI_MIMIC_NW,			TI_MIMIC_N,			TI_MIMIC_NE,
	TI_MIMIC_W,				DONT_USE,				TI_MIMIC_E,
	TI_MIMIC_SW,			TI_MIMIC_S,			TI_MIMIC_SE
   },

	//M_SPIDER
   {
	TI_SPIDER_ANW,		TI_SPIDER_AN,		TI_SPIDER_ANE,
	TI_SPIDER_AW,			DONT_USE,				TI_SPIDER_AE,
	TI_SPIDER_ASW,		TI_SPIDER_AS,		TI_SPIDER_ASE
   }
};

//Prototypes.
UINT	CalcTileImageForCrumblyWall(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForFloor(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForObstacle(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForPit(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForStairs(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForTar(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForWall(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForYellowDoor(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForGreenDoor(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForBlueDoor(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);
UINT	CalcTileImageForRedDoor(const CDbRoom *pRoom, const UINT wCol, const UINT wRow);

//*****************************************************************************
bool CalcEdge(
//Returns:
//True if a black edge should be drawn between these two tiles, else false.
//
//	1. Wall and crumbly wall squares need to be outlined around their edges
//		to show division between wall squares and non-wall squares.
//	2. Pit squares need to be outlined around their edges
//		to show division between pit and non-pit squares.
//		NOTE: draw the outline on the non-pit squares, not in the pit itself
//
//Params:
	const UINT wTileNo, const UINT wAdjTileNo,	//(in)
	const UINT side)	//(in) To prevent some sides from being edged for aesthetic reasons
{
	switch (wTileNo)
	{
		case T_WALL: case T_WALL_B:
			if (wAdjTileNo == T_PIT && (side == E || side == W))
				return false;
			if (wAdjTileNo != T_WALL && wAdjTileNo != T_WALL_B && side != S)
				return true;
			return false;
		case T_PIT:
			if ((wAdjTileNo == T_WALL || wAdjTileNo == T_WALL_B) && side == S)
				return false;
			if (wAdjTileNo != T_PIT && side != N)	//don't edge visible edge of pits (i.e. when they're on the south of non-pits)
				return true;
			return false;
		default:
			return false;
	}
}


//*****************************************************************************
UINT GetTileImageForMonster(
//Gets a tile image to display for a monster.
//
//Params:
	const CMonster *pMonster,	//(in)	Contains data needed to figure out
	const UINT wAnimFrame)		//(in)  Which array to get tile from
								//		tile image.
//
//Returns:
//TI_* constant or CALC_NEEDED if tile needs to be calculated based on
//other context.
{
	ASSERT(IsValidMonsterType(pMonster->wType));
	ASSERT(IsValidOrientation(pMonster->wO));
	ASSERT(MonsterTileImageArray[pMonster->wType][pMonster->wO] != DONT_USE);

	return (wAnimFrame == 0 ?
			MonsterTileImageArray[pMonster->wType][pMonster->wO] :
			AnimatedMonsterTileImageArray[pMonster->wType][pMonster->wO]);
}

//*****************************************************************************
UINT GetTileImageForTileNo(
//Gets a tile image to display a tile.  Oversimplified for now.
//
//Params:
	const UINT wTileNo)	//(in)	A T_* constant.
//
//Returns:
//TI_* constant or CALC_NEEDED if tile needs to be calculated based on
//other context.
{
	const UINT TileImageArray[TILE_COUNT] =
	{
		TI_TEMPTY,		//T_EMPTY
		CALC_NEEDED,	//T_FLOOR
		CALC_NEEDED,	//T_PIT
		CALC_NEEDED,	//T_STAIRS
		CALC_NEEDED,	//T_WALL
		CALC_NEEDED,	//T_WALL_B
		CALC_NEEDED,	//T_DOOR_C
		CALC_NEEDED,	//T_DOOR_M
		CALC_NEEDED,		//T_DOOR_R
		CALC_NEEDED,	//T_DOOR_Y
		TI_DOOR_YO,		//T_DOOR_YO
		TI_TRAPDOOR,	//T_TRAPDOOR
		CALC_NEEDED,	//T_OB_1
		TI_ARROW_N,		//T_ARROW_N
		TI_ARROW_NE,	//T_ARROW_NE
		TI_ARROW_E,		//T_ARROW_E
		TI_ARROW_SE,	//T_ARROW_SE
		TI_ARROW_S,		//T_ARROW_S
		TI_ARROW_SW,	//T_ARROW_SW
		TI_ARROW_W,		//T_ARROW_W
		TI_ARROW_NW,	//T_ARROW_NW
		TI_POTION_I,	//T_POTION_I
		TI_POTION_K,	//T_POTION_K
		TI_SCROLL,		//T_SCROLL
		TI_ORB_D,		//T_ORB
		TI_SNK_EW,		//T_SNK_EW
		TI_SNK_NS,		//T_SNK_NS
		TI_SNK_NW,		//T_SNK_NW
		TI_SNK_NE,		//T_SNK_NE
		TI_SNK_SW,		//T_SNK_SW
		TI_SNK_SE,		//T_SNK_SE
		TI_SNKT_S,		//T_SNKT_S
		TI_SNKT_W,		//T_SNKT_W
		TI_SNKT_N,		//T_SNKT_N
		TI_SNKT_E,		//T_SNKT_E
		CALC_NEEDED,	//T_TAR
		CALC_NEEDED		//T_CHECKPOINT
	};

	ASSERT(IsValidTileNo(wTileNo));
	return TileImageArray[wTileNo];
}

//*****************************************************************************
UINT CalcTileImageForOSquare(
//Calculates a tile image for a square on o-layer.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Indicates square for which to calculate.
//
//Returns:
//TI_* constant.
{
	const UINT wTileNo = pRoom->GetOSquare(wCol, wRow);

	//Caller shouldn't have called this routine unless GetTileImageForTileNo()
	//indicated a tile calculation was needed.
	ASSERT(GetTileImageForTileNo(wTileNo == CALC_NEEDED));

	switch (wTileNo)
	{
		case T_CHECKPOINT:
		case T_FLOOR:	return CalcTileImageForFloor(pRoom, wCol, wRow);
		case T_OB_1:	return CalcTileImageForObstacle(pRoom, wCol, wRow);
		case T_PIT:		return CalcTileImageForPit(pRoom, wCol, wRow);
		case T_STAIRS:	return CalcTileImageForStairs(pRoom, wCol, wRow);
		case T_WALL:	return CalcTileImageForWall(pRoom, wCol, wRow);
		case T_WALL_B:	return CalcTileImageForCrumblyWall(pRoom, wCol, wRow);
		case T_DOOR_Y:	return CalcTileImageForYellowDoor(pRoom, wCol, wRow);
		case T_DOOR_M:	return CalcTileImageForGreenDoor(pRoom, wCol, wRow);
		case T_DOOR_C:	return CalcTileImageForBlueDoor(pRoom, wCol, wRow);
		case T_DOOR_R:	return CalcTileImageForRedDoor(pRoom, wCol, wRow);
	}

	//Shouldn't get here.  Possibly the map in GetTileImageForTileNo() has
	//CALC_NEEDED specified for a tile#, but above switch doesn't handle for
	//this tile#.
	ASSERTP(false, "Unexpected tile#.");
	return TI_TEMPTY;
}

//*****************************************************************************
UINT CalcTileImageForTSquare(
//Calculates a tile image for a square on t-layer.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Indicates square for which to calculate.
//
//Returns:
//TI_* constant.
{
	const UINT wTileNo = pRoom->GetTSquare(wCol, wRow);

	//Caller shouldn't have called this routine unless GetTileImageForTileNo()
	//indicated a tile calculation was needed.
	ASSERT(GetTileImageForTileNo(wTileNo == CALC_NEEDED));

	switch (wTileNo)
	{
		case T_TAR:		return CalcTileImageForTar(pRoom, wCol, wRow);
	}

	//Shouldn't get here.  Possibly the map in GetTileImageForTileNo() has
	//CALC_NEEDED specified for a tile#, but above switch doesn't handle for
	//this tile#.
	ASSERTP(false, "Unexpected tile#.");
	return TI_TEMPTY;
}

//*****************************************************************************
UINT CalcTileImageForYellowDoor(
//Calcs a tile image to display for a yellow door square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Door square.
//
//Returns:
//TI_* constant.
{
	//If north door, set bit 1.
	UINT wCalcCode = wRow > 0 ?
		pRoom->GetOSquare(wCol, wRow - 1) == T_DOOR_Y : false;

	//If south door, set bit 2.
	wCalcCode += 2 * (wRow < pRoom->wRoomRows - 1 ?
			pRoom->GetOSquare(wCol, wRow + 1) == T_DOOR_Y : false);

	//If west door, set bit 3.
	wCalcCode += 4 * (wCol > 0 ?
			pRoom->GetOSquare(wCol - 1, wRow) == T_DOOR_Y : false);

	//If east door, set bit 4.
	wCalcCode += 8 * (wCol < pRoom->wRoomCols - 1 ?
			pRoom->GetOSquare(wCol + 1, wRow) == T_DOOR_Y : false);

	static UINT TileImages[16] = {

		// ?.?	0		?#?		1		?.?		2		?#?		3
		// .X.			.X.				.X.				.X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_Y,		TI_DOOR_YN,		TI_DOOR_YS,		TI_DOOR_YNS,

		// ?.?	4		?#?		5		?.?		6		?#?		7
		// #X.			#X.				#X.				#X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_YW,		TI_DOOR_YNW,	TI_DOOR_YSW,	TI_DOOR_YNSW,

		// ?.?	8		?#?		9		?.?		10		?#?		11
		// .X#			.X#				.X#				.X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_YE,		TI_DOOR_YNE,	TI_DOOR_YSE,	TI_DOOR_YNSE,

		// ?.?	12		?#?		13		? ?		14		?#?		15
		// #X#			#X#				#X#				#X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_YWE,	TI_DOOR_YNWE,	TI_DOOR_YSWE,	TI_DOOR_YNSWE
	};

	ASSERT(wCalcCode < 16);
	return TileImages[wCalcCode];
}

//*****************************************************************************
UINT CalcTileImageForGreenDoor(
//Calcs a tile image to display for a green door square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Door square.
//
//Returns:
//TI_* constant.
{
	//If north door, set bit 1.
	UINT wCalcCode = wRow > 0 ?
		pRoom->GetOSquare(wCol, wRow - 1) == T_DOOR_M : false;

	//If south door, set bit 2.
	wCalcCode += 2 * (wRow < pRoom->wRoomRows - 1 ?
			pRoom->GetOSquare(wCol, wRow + 1) == T_DOOR_M : false);

	//If west door, set bit 3.
	wCalcCode += 4 * (wCol > 0 ?
			pRoom->GetOSquare(wCol - 1, wRow) == T_DOOR_M : false);

	//If east door, set bit 4.
	wCalcCode += 8 * (wCol < pRoom->wRoomCols - 1 ?
			pRoom->GetOSquare(wCol + 1, wRow) == T_DOOR_M : false);

	static UINT TileImages[16] = {

		// ?.?	0		?#?		1		?.?		2		?#?		3
		// .X.			.X.				.X.				.X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_M,		TI_DOOR_MN,		TI_DOOR_MS,		TI_DOOR_MNS,

		// ?.?	4		?#?		5		?.?		6		?#?		7
		// #X.			#X.				#X.				#X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_MW,		TI_DOOR_MNW,	TI_DOOR_MSW,	TI_DOOR_MNSW,

		// ?.?	8		?#?		9		?.?		10		?#?		11
		// .X#			.X#				.X#				.X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_ME,		TI_DOOR_MNE,	TI_DOOR_MSE,	TI_DOOR_MNSE,

		// ?.?	12		?#?		13		? ?		14		?#?		15
		// #X#			#X#				#X#				#X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_MWE,	TI_DOOR_MNWE,	TI_DOOR_MSWE,	TI_DOOR_MNSWE
	};

	ASSERT(wCalcCode < 16);
	return TileImages[wCalcCode];
}

//*****************************************************************************
UINT CalcTileImageForBlueDoor(
//Calcs a tile image to display for a blue door square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Door square.
//
//Returns:
//TI_* constant.
{
	//If north door, set bit 1.
	UINT wCalcCode = wRow > 0 ?
		pRoom->GetOSquare(wCol, wRow - 1) == T_DOOR_C : false;

	//If south door, set bit 2.
	wCalcCode += 2 * (wRow < pRoom->wRoomRows - 1 ?
			pRoom->GetOSquare(wCol, wRow + 1) == T_DOOR_C : false);

	//If west door, set bit 3.
	wCalcCode += 4 * (wCol > 0 ?
			pRoom->GetOSquare(wCol - 1, wRow) == T_DOOR_C : false);

	//If east door, set bit 4.
	wCalcCode += 8 * (wCol < pRoom->wRoomCols - 1 ?
			pRoom->GetOSquare(wCol + 1, wRow) == T_DOOR_C : false);

	static UINT TileImages[16] = {

		// ?.?	0		?#?		1		?.?		2		?#?		3
		// .X.			.X.				.X.				.X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_C,		TI_DOOR_CN,		TI_DOOR_CS,		TI_DOOR_CNS,

		// ?.?	4		?#?		5		?.?		6		?#?		7
		// #X.			#X.				#X.				#X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_CW,		TI_DOOR_CNW,	TI_DOOR_CSW,	TI_DOOR_CNSW,

		// ?.?	8		?#?		9		?.?		10		?#?		11
		// .X#			.X#				.X#				.X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_CE,		TI_DOOR_CNE,	TI_DOOR_CSE,	TI_DOOR_CNSE,

		// ?.?	12		?#?		13		? ?		14		?#?		15
		// #X#			#X#				#X#				#X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_CWE,	TI_DOOR_CNWE,	TI_DOOR_CSWE,	TI_DOOR_CNSWE
	};

	ASSERT(wCalcCode < 16);
	return TileImages[wCalcCode];
}

//*****************************************************************************
UINT CalcTileImageForRedDoor(
//Calcs a tile image to display for a red door square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Door square.
//
//Returns:
//TI_* constant.
{
	//If north door, set bit 1.
	UINT wCalcCode = wRow > 0 ?
		pRoom->GetOSquare(wCol, wRow - 1) == T_DOOR_R : false;

	//If south door, set bit 2.
	wCalcCode += 2 * (wRow < pRoom->wRoomRows - 1 ?
			pRoom->GetOSquare(wCol, wRow + 1) == T_DOOR_R : false);

	//If west door, set bit 3.
	wCalcCode += 4 * (wCol > 0 ?
			pRoom->GetOSquare(wCol - 1, wRow) == T_DOOR_R : false);

	//If east door, set bit 4.
	wCalcCode += 8 * (wCol < pRoom->wRoomCols - 1 ?
			pRoom->GetOSquare(wCol + 1, wRow) == T_DOOR_R : false);

	static UINT TileImages[16] = {

		// ?.?	0		?#?		1		?.?		2		?#?		3
		// .X.			.X.				.X.				.X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_R,		TI_DOOR_RN,		TI_DOOR_RS,		TI_DOOR_RNS,

		// ?.?	4		?#?		5		?.?		6		?#?		7
		// #X.			#X.				#X.				#X.
		// ?.?			?.?				?#?				?#?
		TI_DOOR_RW,		TI_DOOR_RNW,	TI_DOOR_RSW,	TI_DOOR_RNSW,

		// ?.?	8		?#?		9		?.?		10		?#?		11
		// .X#			.X#				.X#				.X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_RE,		TI_DOOR_RNE,	TI_DOOR_RSE,	TI_DOOR_RNSE,

		// ?.?	12		?#?		13		? ?		14		?#?		15
		// #X#			#X#				#X#				#X#
		// ?.?			?.?				?#?				?#?
		TI_DOOR_RWE,	TI_DOOR_RNWE,	TI_DOOR_RSWE,	TI_DOOR_RNSWE
	};

	ASSERT(wCalcCode < 16);
	return TileImages[wCalcCode];
}

//*****************************************************************************
UINT CalcTileImageForFloor(
//Calcs a tile image to display for a floor square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Floor square.
//
//Returns:
//TI_* constant.
{
	const bool bIsDark = ((wCol + wRow) % 2 == 0);

	//Get tiles in N, NW, and W positions that will be used to calc
	//returned tile image.  When squares to evaluate are out-of-bounds, I make
	//a guess of what the out-of-bounds squares might be.
	UINT wNTileNo, wWTileNo, wNWTileNo;
	if (wCol == 0)
	{
		//W and NW are out-of-bounds.

		//If N is also out-of-bounds then I can handle with quick return.
		if (wRow == 0) return bIsDark ? TI_EMPTY_D : TI_EMPTY_L;

		//Pretend the out-of-bounds squares are a continuation of center and north
		//squares.
		wNTileNo = pRoom->GetOSquare(wCol, wRow - 1);
		wWTileNo = pRoom->GetOSquare(wCol, wRow);
		wNWTileNo = wNTileNo;
	}
	else if (wRow == 0)
	{
		//N and NW are out-of-bounds.

		//Pretend the out-of-bounds squares are a continuation of center and north
		//squares.
		wNTileNo = pRoom->GetOSquare(wCol, wRow);
		wWTileNo = pRoom->GetOSquare(wCol - 1, wRow);
		wNWTileNo = wWTileNo;
	}
	else
	{
		//All squares to check are in-bounds.
		wNTileNo = pRoom->GetOSquare(wCol, wRow - 1);
		wWTileNo = pRoom->GetOSquare(wCol - 1, wRow);
		wNWTileNo = pRoom->GetOSquare(wCol - 1, wRow - 1);
	}

#define CastsWallShadow(t) ((t) == T_WALL || (t) == T_WALL_B || (t) == T_DOOR_Y || \
                (t) == T_DOOR_C || (t) == T_DOOR_R || (t) == T_DOOR_M)

	const bool bIsWallN = CastsWallShadow(wNTileNo);
	const bool bIsWallW = CastsWallShadow(wWTileNo);
	const bool bIsWallNW = CastsWallShadow(wNWTileNo);

#undef CastsWallShadow

	const bool bIsObN = (wNTileNo == T_OB_1);
	const bool bIsObW = (wWTileNo == T_OB_1);
	const bool bIsObNW = (wNWTileNo == T_OB_1);

	//Note: Wall shadows and obstacle shadows are not intended to mix.  The room
	//editor should disallow it.  If this rule has been violated, obstacle
	//shadows will be ignored.

	if (bIsWallN || bIsWallW || bIsWallNW)
	{
		//Return tile image with wall shadows.
		const UINT wCalcArrayI =
			(bIsWallNW * 1) +
			(bIsWallW * 2) +
			(bIsWallN * 4) +
			(bIsDark * 8);
		static const UINT CalcArray[] =
		{
			//bIsDark=false (0 - 7)

				// ..				#.					..					#.
				// .				.					#					#
				TI_EMPTY_L,		TI_SHADO_LNW,	TI_SHADO_LSW,	TI_SHADO_LW,

				// .#				##					.#						##
				// .				.					#						#
				TI_SHADO_LNE,	TI_SHADO_LN,	TI_SHADO_LNESW,	TI_SHADO_LNWI,

			//bIsDark=true (8 - 15)

				// ..				#.					..					#.
				// .				.					#					#
				TI_EMPTY_D,		TI_SHADO_DNW,	TI_SHADO_DSW,	TI_SHADO_DW,

				// .#				##					.#						##
				// .				.					#						#
				TI_SHADO_DNE,	TI_SHADO_DN,	TI_SHADO_DNESW,	TI_SHADO_DNWI
		};
		ASSERT(wCalcArrayI < sizeof(CalcArray) / sizeof(UINT));
		return CalcArray[wCalcArrayI];
	}
	else
	{
		//Return tile image with obstacle shadows or empty square.
		//(For the error cases, do the best you can.)
		const UINT wCalcArrayI =
			(bIsObNW * 1) +
			(bIsObW * 2) +
			(bIsObN * 4) +
			(bIsDark * 8);
		static const UINT CalcArray[] =
		{
			//bIsDark=false (0 - 7)

			// ..					#.						..						#.
			// .					.						#						#
			TI_EMPTY_L,			TI_OBSHADO_LNW,	TI_OBSHADO_LSW,	TI_OBSHADO_LW,

			// .#					##						.# (edit error)	## (edit error)
			// .					.						#						#
			TI_OBSHADO_LNE,	TI_OBSHADO_LN,		TI_SHADO_LNESW,	TI_SHADO_LNWI,

			//bIsDark=true (8 - 15)

			// ..					#.					..						#.
			// .					.					#						#
			TI_EMPTY_D,		TI_OBSHADO_DNW,	TI_OBSHADO_DSW,	TI_OBSHADO_DW,

			// .#					##					.# (edit error)	## (edit error)
			// .					.					#						#
			TI_OBSHADO_DNE,	TI_OBSHADO_DN,	TI_SHADO_DNESW,	TI_SHADO_DNWI
		};
		ASSERT(wCalcArrayI < sizeof(CalcArray) / sizeof(UINT));
		return CalcArray[wCalcArrayI];
	}
}

//*****************************************************************************
UINT CalcTileImageForWall(
//Calcs a tile image to display for a wall square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Square for which to calc.
//
//Returns:
//TI_* constant.
{
	//Get south tile.
	const UINT wSTileNo = (wRow < pRoom->wRoomRows - 1) ?
			pRoom->GetOSquare(wCol, wRow + 1) : T_WALL;
	if (wSTileNo == T_WALL || wSTileNo == T_WALL_B)
		return TI_WALL;
	else
		return TI_WALL_S;
}

//*****************************************************************************
UINT CalcTileImageForCrumblyWall(
//Calcs a tile image to display for a crumbly wall square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Square for which to calc.
//
//Returns:
//TI_* constant.
{
	//Get south tile.
	const UINT wSTileNo = (wRow < pRoom->wRoomRows - 1) ?
			pRoom->GetOSquare(wCol, wRow + 1) : T_WALL;
	if (wSTileNo == T_WALL || wSTileNo == T_WALL_B)
		return TI_WALL_B;
	else
		return TI_WALL_BS;
}

//*****************************************************************************
UINT CalcTileImageForStairs(
//Calcs a tile image to display for a stairs square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Square for which to calc.
//
//Returns:
//TI_* constant.
{
	//Find out what position in a staircase this tile is.
	UINT wStairsCol = 0;
	UINT wStairsRow = 0;
	UINT wPrevTile;

	if (wCol == 0)
		wStairsCol = 1;
	else
	{
		wPrevTile = T_STAIRS;
		while (wPrevTile==T_STAIRS)
		{
			++wStairsCol;
			wPrevTile = (wCol-wStairsCol>0) ?
				pRoom->GetOSquare(wCol-wStairsCol, wRow) : T_WALL;
		}
	}

	if (wRow == 0)
		wStairsRow = 1;
	else
	{
		wPrevTile = T_STAIRS;
		while (wPrevTile==T_STAIRS)
		{
			++wStairsRow;
			wPrevTile = (wRow-wStairsRow>0) ?
				pRoom->GetOSquare(wCol, wRow-wStairsRow) : T_WALL;
		}
	}

	//Now we know the position relative to the beginning of the stairs,
	//we can work out which tile to show.
	const UINT wLastShadedTile=(wStairsCol * 3) - 2;

	if (wStairsRow > wLastShadedTile) return TI_STAIRS_5;			//Full shading.
	if (wStairsRow == wLastShadedTile) return TI_STAIRS_4;		// A lot of shading.
	if (wStairsRow + 1 == wLastShadedTile) return TI_STAIRS_3;	// Medium Shading.
	if (wStairsRow + 2 == wLastShadedTile) return TI_STAIRS_1;	// Minimal Shading.
	//Only option remaining at this point is < wLastShadedTile-2.
	return TI_STAIRS_2;														//No shading.
}

//*****************************************************************************
UINT CalcTileImageForTar(
//Calcs a tile image to display for a tar square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Square for which to calc.
//
//Returns:
//TI_* constant.
{
	//Going to use bit setting to work out what is around our current tile.
	//The bits for the tiles look like this (where C is our tile):
	//
	//	  1	  2	  4
	//	128	  C	  8
	//	 64	 32	 16
	//
	//Bits will be set in the bNotTarTiles byte if the square in question does
	//not have tar. Ie. if our tile (C) was against the northern wall but had
	//tar all around it apart from that, the bNotTarTiles would be 7 (1 or 2 or 4).
	//from that, we will then score each point of the compass and use the scores to
	//decide which return value to use. The scores will be out of 3 for each point
	//of the compass the lower the score, the more surrounding squares with tar in
	//them.

	//To make everything nice and easy, we're going to have an array storing the
	//bit values of each tile. These constants point to the elements in that array
	//and the compass points in the scores array. The order of this makes the scoring
	//easier later.
	const int I_NWTILE	= 0;
	const int I_NTILE	= 1;
	const int I_NETILE	= 2;
	const int I_ETILE	= 3;
	const int I_SETILE	= 4;
	const int I_STILE	= 5;
	const int I_SWTILE	= 6;
	const int I_WTILE	= 7;
	const int I_CTILE	= 8;

	//Bits to be set for each missing tile. Note the tile order spirals clockwise
	//from the NW.
	const UINT B_TILEBITS[] =
	{
		1, 2, 4, 8, 16, 32, 64, 128
	};

	//Adjustments needed to wRow and wCol to get tile corresponding to the tile
	//order above.
	const int I_ROWADJUST[] =
	{
		-1, -1, -1, 0, 1, 1, 1, 0
	};
	const int I_COLADJUST[] =
	{
		-1, 0, 1, 1, 1, 0, -1, -1
	};

	int iCompassScores[I_CTILE];//A score for each point of the compass.
	int iTotalScore = 0;		//Total of all compass scores (easier for some decisions).
	UINT bNotTarTiles = 0;	//Byte for bit setting the non tar tiles.
	UINT wWhatTile = 0;			//What's in the surrounding tile.
	UINT wCheckRow = 0;			//Row position of the tile to check for tar.
	UINT wCheckCol = 0;			//Column position of the tile to check for tar.

	//Initialise the scores array.
   int iTilePointer;
	for (iTilePointer=I_CTILE; iTilePointer--; )
		iCompassScores[iTilePointer] = 0;

	//Find out which of the 8 tiles around us don't have tar in them.
	for (iTilePointer=0; iTilePointer < I_CTILE; iTilePointer++)
	{
		//Find out what is in this particular surrounding tile.
		wCheckRow = wRow + I_ROWADJUST[iTilePointer];
		wCheckCol = wCol + I_COLADJUST[iTilePointer];
		wWhatTile = pRoom->IsValidColRow(wCheckCol, wCheckRow) ?
			pRoom->GetTSquare(wCheckCol, wCheckRow) : T_WALL;

		//If the surrounding tile doesn't have tar in it then set the relevent
		//bit in bNotTarTiles and work out the scores for each point of the compass. If a
		//tile surrounding ours is empty, it and the ones next to it in our tile order
		//will get +1 score so basically, if 2 tiles are empty of tar, they will both get
		//a score of 2. If 3 tiles together are empty (i.e. around a corner) the middle
		//tile will score 3 and the other 2 will score 2.
		if (wWhatTile!=T_TAR)
		{
			bNotTarTiles = bNotTarTiles | B_TILEBITS[iTilePointer];
			int iPrevTile = iTilePointer - 1;
			int iNextTile = iTilePointer + 1;

			if (iPrevTile < I_NWTILE) iPrevTile = I_WTILE;
			if (iNextTile==I_CTILE) iNextTile = I_NWTILE;

			iCompassScores[iTilePointer] += 1;
			iCompassScores[iPrevTile] += 1;
			iCompassScores[iNextTile] += 1;
			iTotalScore += 3;
		}
	}

	//Now we can work out what to return.
	if (iTotalScore==0) return TI_TAR_NSEW;		//Surrounded by tar.
	if (iTotalScore>15) return TI_TAR_NWSEI;	//Not much Tar around this tile so just use any.

	//Only 1 empty square.
	if (iTotalScore==3)
	{
		if (bNotTarTiles & B_TILEBITS[I_NTILE]) return TI_TAR_N;
		if (bNotTarTiles & B_TILEBITS[I_ETILE]) return TI_TAR_E;
		if (bNotTarTiles & B_TILEBITS[I_STILE]) return TI_TAR_S;
		if (bNotTarTiles & B_TILEBITS[I_WTILE]) return TI_TAR_W;
		if (bNotTarTiles & B_TILEBITS[I_NWTILE]) return TI_TAR_SEI;
		if (bNotTarTiles & B_TILEBITS[I_NETILE]) return TI_TAR_SWI;
		if (bNotTarTiles & B_TILEBITS[I_SWTILE]) return TI_TAR_NEI;
		if (bNotTarTiles & B_TILEBITS[I_SETILE]) return TI_TAR_NWI;
	}

	//Normal Corners.
	if (iCompassScores[I_NWTILE]==3) return TI_TAR_NW;
	if (iCompassScores[I_NETILE]==3) return TI_TAR_NE;
	if (iCompassScores[I_SETILE]==3) return TI_TAR_SE;
	if (iCompassScores[I_SWTILE]==3) return TI_TAR_SW;

        //Complex Corners.
	if (bNotTarTiles & B_TILEBITS[I_NTILE])
	{
		if (bNotTarTiles & B_TILEBITS[I_WTILE] || bNotTarTiles & B_TILEBITS[I_SWTILE])
			return TI_TAR_NW;
		if (bNotTarTiles & B_TILEBITS[I_ETILE] || bNotTarTiles & B_TILEBITS[I_SETILE])
			return TI_TAR_NE;
	}
	if ((bNotTarTiles & B_TILEBITS[I_NETILE]) && (bNotTarTiles & B_TILEBITS[I_WTILE]))
		return TI_TAR_NW;
	if ((bNotTarTiles & B_TILEBITS[I_NWTILE]) && (bNotTarTiles & B_TILEBITS[I_ETILE]))
		return TI_TAR_NE;

	if (bNotTarTiles & B_TILEBITS[I_STILE])
	{
		if (bNotTarTiles & B_TILEBITS[I_WTILE] || bNotTarTiles & B_TILEBITS[I_NWTILE])
			return TI_TAR_SW;
		if (bNotTarTiles & B_TILEBITS[I_ETILE] || bNotTarTiles & B_TILEBITS[I_NETILE])
			return TI_TAR_SE;
	}
	if ((bNotTarTiles & B_TILEBITS[I_SETILE]) && (bNotTarTiles & B_TILEBITS[I_WTILE]))
		return TI_TAR_SW;
	if ((bNotTarTiles & B_TILEBITS[I_SWTILE]) && (bNotTarTiles & B_TILEBITS[I_ETILE]))
		return TI_TAR_SE;

	//Edges.
	if (iCompassScores[I_NTILE]>1) return TI_TAR_N;
	if (iCompassScores[I_ETILE]>1) return TI_TAR_E;
	if (iCompassScores[I_STILE]>1) return TI_TAR_S;
	if (iCompassScores[I_WTILE]>1) return TI_TAR_W;

	//Inner corners.
	if (iCompassScores[I_NWTILE]==1)
	{
		if (iCompassScores[I_SETILE]==1) return TI_TAR_NWSEI;
		return TI_TAR_SEI;
	}

	if (iCompassScores[I_NETILE]==1)
	{
		if (iCompassScores[I_SWTILE]==1) return TI_TAR_NESWI;
		return TI_TAR_SWI;
	}

	if (iCompassScores[I_SETILE]==1) return TI_TAR_NWI;
	if (iCompassScores[I_SWTILE]==1) return TI_TAR_NEI;

	//We shouldn't get to here but if we do, we have a wierd number of tiles
	//with tar in around this one so return an arbitary tile (hopefully, it
	//will look nice enough.
	return TI_TAR_NESWI;
}

//*****************************************************************************
UINT CalcTileImageForObstacle(
//Calcs a tile image to display for an obstacle.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Square for which to calc.
//
//Returns:
//TI_* constant.
{
	static const UINT corner[2][2] = {{TI_OB_NW,TI_OB_SW},{TI_OB_NE,TI_OB_SE}};
	UINT x = wCol, y = wRow, xPos = 0, yPos = 0;

	//Find parts of obstacle to figure out which corner this square is at.
	while (x > 0) {
		if (pRoom->GetOSquare(x - 1, wRow) == T_OB_1)
		{
			++xPos;
			--x;
		} else break;
	}
	while (y > 0) {
		if (pRoom->GetOSquare(wCol, y - 1) == T_OB_1)
		{
			++yPos;
			--y;
		} else break;
	}

	//Boundary checks (for top and left edges of screen).
	if (wCol == 0 && pRoom->GetOSquare(1, wRow) != T_OB_1)
		if (wRow == 0 && pRoom->GetOSquare(0, 1) != T_OB_1)
			return TI_OB_SE;
		else return corner[1][yPos % 2];
	else
		if (wRow == 0 && pRoom->GetOSquare(wCol, 1) != T_OB_1)
			return corner[xPos % 2][1];
		//else return corner[xPos % 2][yPos % 2];	//handled below

	return corner[xPos % 2][yPos % 2];
}

//*****************************************************************************
UINT CalcTileImageForPit(
//Calcs a tile image to display for a pit square.
//
//Params:
	const CDbRoom *pRoom,	//(in)	Room to use for calcs--not necessarily the
							//		current room.
	const UINT wCol, const UINT wRow)	//(in)	Square for which to calc.
//
//Returns:
//TI_* constant.
{
#define IsPit(x,y)	(pRoom->GetOSquare(x, y) == T_PIT)
#define IsTrapDoor(x,y)	(pRoom->GetOSquare(x, y) == T_TRAPDOOR)

	//Look north until I find a non-pit square.
	UINT wLookRow = wRow;
	while (wLookRow != (UINT)(-1) && IsPit(wCol, wLookRow)) --wLookRow;
	const UINT wRowsFromTop = (wRow - wLookRow);

	//If farther than three rows from top of pit, then tile image does not
	//depend on top of pit.
	if (wLookRow == (UINT)(-1) || wRowsFromTop > 3)
	{
		if (wCol == 0) return TI_SPIKE; //Easy return.

		//Calculate shadow on pit floor.
		const bool bIsNNWPit = (wRow > 1) ?
				IsPit(wCol - 1, wRow - 2) : IsPit(wCol - 1, 0);
		const bool bIsNNNWPit = (wRow > 2) ?
				IsPit(wCol - 1, wRow - 3) : IsPit(wCol - 1, 0);
		const bool bIsNNNWWPit = (wRow > 2 && wCol > 1) ?
				IsPit(wCol - 2, wRow - 3) : IsPit(wCol - 1, 0);

		const UINT wCalcI = (bIsNNNWWPit * 4) + (bIsNNWPit * 2) + (bIsNNNWPit * 1);
		static const UINT CalcArray[] =
		{
			//##P			#PP				##P				#PP
			//?#P			?#P				?PP				?PP
			//??P			??P				??P				??P
			//??X			??X				??X				??X
			TI_SPIKE_D,	TI_SPIKE_DSW,	TI_SPIKE_D,		TI_SPIKE,

			//P#P			PPP				P#P				PPP
			//?#P			?#P				?PP				?PP
			//??P			??P				??P				??P
			//??X			??X				??X				??X
			TI_SPIKE_D,	TI_SPIKE_DSW,	TI_SPIKE_DNE,	TI_SPIKE
		};
		ASSERT(wCalcI < sizeof(CalcArray) / sizeof(UINT));

		return CalcArray[wCalcI];
	}

	//Need to know west pit for either pit wall or spikes calc.
	const bool bIsWPit = (wCol > 0) ?
			IsPit(wCol - 1, wLookRow) : IsPit(0, wLookRow);

	//See if I'm calcing for pit wall or spikes.
	if (wRowsFromTop < 3)
	{
		//Calc for pit wall.

		//Figure out things I need to know for calculation.
		const bool bIsEPit = (wCol < pRoom->wRoomCols - 1) ?
				IsPit(wCol + 1, wLookRow) : true;
		const bool bIsUpperWall = (wRowsFromTop == 1);
		const UINT wCalcI = (bIsUpperWall * 4) + (bIsWPit * 2) + (bIsEPit * 1);
		static const UINT CalcArray[] =
		{
			//bIsUpperWall = false

				TI_PIT_DS, TI_PIT_DSE, TI_PIT_LS, TI_PIT_LSE,


			//bIsUpperWall = true

				TI_PIT_DN, TI_PIT_DNE, TI_PIT_LN, TI_PIT_LNE
		};
		ASSERT(wCalcI < sizeof(CalcArray) / sizeof(UINT));
		return CalcArray[wCalcI];
	}
	else
	{
		//Calc for spike.

		//Figure out things I need to know for calculation.
		const bool bIsWWPit = (wCol > 1) ?
				IsPit(wCol - 2, wLookRow) : IsPit(0, wLookRow);
		bool bIsSWPit;
		if (wCol > 0)
		{
			bIsSWPit = (wLookRow < pRoom->wRoomRows - 1) ?
				IsPit(wCol - 1, wLookRow + 1) :
				IsPit(wCol - 1, pRoom->wRoomRows - 1);
		}
		else
		{
			bIsSWPit = (wLookRow < pRoom->wRoomRows - 1) ?
				IsPit(0, wLookRow + 1) :
				IsPit(0, pRoom->wRoomRows - 1);
		}
		const UINT wCalcI = (bIsSWPit * 4) + (bIsWPit * 2) + (bIsWWPit * 1);
		static const UINT CalcArray[] =
		{
			//###			P##				#P#				PP#
			//?#P			?#P				?#P				?#P
			//??P			??P				??P				??P
			//??X			??X				??X				??X
			TI_SPIKE_D,		TI_SPIKE_D,		TI_SPIKE_DSW,	TI_SPIKE_DSW,

			//###			P##				#P#				PP#
			//?PP			?PP				?PP				?PP
			//??P			??P				??P				??P
			//??X			??X				??X				??X
			TI_SPIKE_D,		TI_SPIKE_DNE,	TI_SPIKE,	TI_SPIKE
		};
		ASSERT(wCalcI < sizeof(CalcArray) / sizeof(UINT));
		return CalcArray[wCalcI];
	}
#undef IsPit
#undef IsTrapDoor
}

// $Log: TileImageCalcs.cpp,v $
// Revision 1.35  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.34  2003/10/06 02:45:08  erikh2000
// Added descriptions to assertions.
//
// Revision 1.33  2003/07/03 21:44:51  mrimer
// Port warning/bug fixes (committed on behalf of Gerry JJ).
//
// Revision 1.32  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.31  2003/05/23 21:43:05  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.30  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.29  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.28  2002/11/22 02:27:58  mrimer
// Fixed a bug.
//
// Revision 1.27  2002/11/15 03:00:50  mrimer
// Moved monster tiles to the .h file for use by the room editor.
//
// Revision 1.26  2002/08/25 19:02:01  erikh2000
// Changed tile calc for pit squares to draw supporting walls underneath like before.
//
// Revision 1.25  2002/07/26 18:26:58  mrimer
// Fixed bug: one wrong tile being drawn in pit spikes.
// Fixed bad pit shadows around trapdoors.
//
// Revision 1.24  2002/07/25 18:58:49  mrimer
// Revised pit drawing logic (below trapdoors).  Tweaking.
//
// Revision 1.23  2002/06/21 05:25:09  mrimer
// Revised includes.
// Added consts to formal parameters.
//
// Revision 1.22  2002/05/24 23:18:57  mrimer
// Refined edge drawing next to walls/pits.
//
// Revision 1.21  2002/05/24 15:26:35  mrimer
// Add outlines to walls and pit-adjacent floor squares.
//
// Revision 1.20  2002/05/15 14:29:53  mrimer
// Moved animation data out of DRODLIB (CMonster) and into DROD (CRoomWidget).
//
// Revision 1.19  2002/05/14 19:06:16  mrimer
// Added monster animation.
//
// Revision 1.18  2002/04/29 00:23:22  erikh2000
// Revised #includes.
//
// Revision 1.17  2002/04/25 22:36:19  erikh2000
// Added calc routines for doors that use new tile images.
//
// Revision 1.16  2002/04/23 17:59:10  mrimer
// Fixed various tile calculation bugs for T_OB_1.
//
// Revision 1.15  2002/04/23 03:09:52  erikh2000
// Squares containing a checkpoint will now be calculated to have floor tile images.
//
// Revision 1.14  2002/04/09 01:49:27  erikh2000
// Changed "FALSE" to "false".
//
// Revision 1.13  2002/03/28 22:56:44  erikh2000
// Fixed problem with crumbly walls not casting shadows.  (Committed on behalf of mrimer.)
//
// Revision 1.12  2002/03/05 01:52:59  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.11  2002/02/28 01:01:22  erikh2000
// Added handling for T_CHECKPOINT.
//
// Revision 1.10  2002/02/24 03:55:44  erikh2000
// Fixed problem with incorrect tile image for northwest arrow.
//
// Revision 1.9  2001/12/16 02:22:48  erikh2000
// Added #include needed for CurrentGame.h to compile.
//
// Revision 1.8  2001/11/17 23:06:25  erikh2000
// Used renamed mimic constants.  (Committed on behalf of j_wicks.)
//
// Revision 1.7  2001/11/14 01:36:20  md5i
// Fix some tile growth calculation problems.
//
// Revision 1.6  2001/11/13 05:38:12  md5i
// Added tar patch by timeracer.  Small TarMother fix by me.
//
