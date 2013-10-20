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
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef TILEIMAGECALCS_H
#define TILEIMAGECALCS_H

#include "TileImageConstants.h"
#include "../DRODLib/MonsterFactory.h"

//Return value for functions that can't return a tile image.
const UINT CALC_NEEDED = (UINT)(-1);

class CDbRoom;
class CMonster;

bool	CalcEdge(const UINT wTile1No, const UINT wTile2No, const UINT side);
UINT	GetTileImageForMonster(const CMonster *pMonster, const UINT wAnimFrame);
UINT	GetTileImageForTileNo(UINT wTileNo);
UINT	CalcTileImageForOSquare(const CDbRoom *pRoom, UINT wCol, UINT wRow);
UINT	CalcTileImageForTSquare(const CDbRoom *pRoom, UINT wCol, UINT wRow);

//Monster tiles.
//Determined by orientation and animation frame.
static const UINT DONT_USE = TI_TEMPTY;

extern const UINT MonsterTileImageArray[MONSTER_TYPES][ORIENTATION_COUNT];
extern const UINT AnimatedMonsterTileImageArray[MONSTER_TYPES][ORIENTATION_COUNT];

#endif //...#ifndef TILEIMAGECALCS_H

// $Log: TileImageCalcs.h,v $
// Revision 1.14  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.13  2003/08/13 21:17:39  mrimer
// Fixed tar mother to spawn one move later the first time, making it every 30th move.
// Extended the roach egg hatching cycle by one turn, from four turns to five.
//
// Revision 1.12  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.10  2002/11/15 02:19:49  mrimer
// Moved MonsterTileImageArray, etc., here from .cpp file.
// Added tar mother right eye tiles.
//
// Revision 1.9  2002/06/21 04:57:09  mrimer
// Revised includes.
//
// Revision 1.8  2002/05/24 23:18:57  mrimer
// Refined edge drawing next to walls/pits.
//
// Revision 1.7  2002/05/24 15:26:35  mrimer
// Add outlines to walls and pit-adjacent floor squares.
//
// Revision 1.6  2002/05/15 14:29:53  mrimer
// Moved animation data out of DRODLIB (CMonster) and into DROD (CRoomWidget).
//
// Revision 1.5  2002/04/29 00:23:22  erikh2000
// Revised #includes.
//
// Revision 1.4  2002/04/09 10:05:40  erikh2000
// Fixed revision log macro.
//
