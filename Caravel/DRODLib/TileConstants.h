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
 * Michael Welsh Duggan (md5i)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef TILECONSTANTS_H
#define TILECONSTANTS_H

#include <BackEndLib/Types.h>

//Tile constants--reordering existing constants may break macros.  Add new 
//tile constants to end and make corresponding update to TILE_COUNT and 
//TILE_LAYER.  Section originally contributed by md5i.
#define T_EMPTY         0
#define T_FLOOR         1
#define T_PIT           2
#define T_STAIRS        3
#define T_WALL          4
#define T_WALL_B        5
#define T_DOOR_C        6
#define T_DOOR_M        7
#define T_DOOR_R        8
#define T_DOOR_Y        9
#define T_DOOR_YO       10
#define T_TRAPDOOR      11
#define T_OB_1          12
#define T_ARROW_N       13
#define T_ARROW_NE      14
#define T_ARROW_E       15
#define T_ARROW_SE      16
#define T_ARROW_S       17
#define T_ARROW_SW      18
#define T_ARROW_W       19
#define T_ARROW_NW      20
#define T_POTION_I      21
#define T_POTION_K      22
#define T_SCROLL        23
#define T_ORB           24
#define T_SNK_EW        25
#define T_SNK_NS        26
#define T_SNK_NW        27
#define T_SNK_NE        28
#define T_SNK_SW        29
#define T_SNK_SE        30
#define T_SNKT_S        31
#define T_SNKT_W        32
#define T_SNKT_N        33
#define T_SNKT_E        34
#define T_TAR           35
#define T_CHECKPOINT    36

#define TILE_COUNT			(37) //Number of tile constants from above list.
#define IsValidTileNo(t)   ((t) >= 0 && (t) < TILE_COUNT)

//
//Tile-related macros.
//

#define bIsArrow(t)   ( (t) >= T_ARROW_N && (t) <= T_ARROW_NW )

#define bIsSerpent(t)   ( (t) >= T_SNK_EW && (t) <= T_SNKT_E )


//Add to monster values so they don't overlap o- and t-layer values on a tile.
//NOTE: Make sure list matches that in MonsterFactory.h.
const UINT	M_OFFSET = TILE_COUNT;
#define T_ROACH		(M_ROACH+M_OFFSET)
#define T_QROACH		(M_QROACH+M_OFFSET)
#define T_REGG			(M_REGG+M_OFFSET)
#define T_GOBLIN		(M_GOBLIN+M_OFFSET)
#define T_NEATHER		(M_NEATHER+M_OFFSET)
#define T_WWING		(M_WWING+M_OFFSET)
#define T_EYE			(M_EYE+M_OFFSET)
#define T_SERPENT		(M_SERPENT+M_OFFSET)
#define T_TARMOTHER	(M_TARMOTHER+M_OFFSET)
#define T_TARBABY		(M_TARBABY+M_OFFSET)
#define T_BRAIN		(M_BRAIN+M_OFFSET)
#define T_MIMIC		(M_MIMIC+M_OFFSET)
#define T_SPIDER		(M_SPIDER+M_OFFSET)
#define MONSTER_COUNT		(13)	//Number of monsters in above list.

#define TOTAL_TILE_COUNT		(TILE_COUNT+MONSTER_COUNT) //Number of all tile constants.
#define IsMonsterTileNo(t)		((t) >= TILE_COUNT && (t) < TOTAL_TILE_COUNT)

//Layer associated with each tile--0 is opaque layer, 1 is transparent layer,
//		2 is monster layer.
const UINT TILE_LAYER[TOTAL_TILE_COUNT] = 
{
	1,	//T_EMPTY         0
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
	0,	//T_OB_1          12
	1,	//T_ARROW_N       13
	1,	//T_ARROW_NE      14
	1,	//T_ARROW_E       15
	1,	//T_ARROW_SE      16
	1,	//T_ARROW_S       17
	1,	//T_ARROW_SW      18
	1,	//T_ARROW_W       19
	1,	//T_ARROW_NW      20
	1,	//T_POTION_I      21
	1,	//T_POTION_K      22
	1,	//T_SCROLL        23
	1,	//T_ORB           24
	1,	//T_SNK_EW        25
	1,	//T_SNK_NS        26
	1,	//T_SNK_NW        27
	1,	//T_SNK_NE        28
	1,	//T_SNK_SW        29
	1,	//T_SNK_SE        30
	1,	//T_SNKT_S        31
	1,	//T_SNKT_W        32
	1,	//T_SNKT_N        33
	1,	//T_SNKT_E        34
	1,	//T_TAR           35
	0,	//T_CHECKPOINT    36
	2,	//T_ROACH			+0
	2,	//T_QROACH			+1
	2,	//M_REGG				+2
	2,	//M_GOBLIN			+3
	2,	//M_NEATHER			+4
	2,	//M_WWING			+5
	2,	//M_EYE				+6
	2,	//M_SERPENT			+7
	2,	//M_TARMOTHER		+8
	2,	//M_TARBABY			+9
	2,	//M_BRAIN			+10
	2,	//M_MIMIC			+11
	2,	//M_SPIDER			+12
};

#endif //...#ifndef TILECONSTANTS_H

// $Log: TileConstants.h,v $
// Revision 1.8  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.7  2003/05/22 23:39:01  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.6  2002/11/13 23:17:41  mrimer
// Added tile constants for monsters.
//
// Revision 1.5  2002/04/22 09:40:35  erikh2000
// Added an include to fix compile error.
//
// Revision 1.4  2002/03/04 22:21:28  erikh2000
// Moved T_CHECKPOINT to opaque layer.
//
// Revision 1.3  2002/02/28 00:59:46  erikh2000
// Added T_CHECKPOINT constant.
//
// Revision 1.2  2001/10/20 05:47:26  erikh2000
// Copied md5i's new tile constants from DRODUtil.  Added comments and array for which layer a tile should be plotted.
//
// Revision 1.1.1.1  2001/10/01 22:20:33  erikh2000
// Initial check-in.
//
