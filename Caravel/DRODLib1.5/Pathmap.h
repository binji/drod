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
 *
 * ***** END LICENSE BLOCK ***** */

//Class for calculating brain-directed monster movement for a room.

//FUTURE CONSIDERATIONS
//
//Force arrows are currently considered obstacles to brain-directed movement.
//
//We need to leave the inaccurate pathfinding in for compatibility, as Serpents
//are unable to move over force arrows, and some rooms depend on this logic,
//but it is entirely possible to later have a "Roach II", "Queen Roach II",
//"Serpent II", etc. that take advantage of better pathfinding.
//The old monsters would still find their old paths in the old rooms.

#ifndef PATHMAP_H
#define PATHMAP_H

#include "Types.h"
#include "Assert.h"

#include <string>
using std::string;

//Path map square that contains only information needed for determining paths.
enum DIRECTION {nw, n, ne, w, none, e, sw, s, se, DIR_COUNT};
enum STATE {obstacle, recalc, ok, immediate};

inline DIRECTION operator--(DIRECTION &d) { return d=static_cast<DIRECTION>(d-1);}
inline DIRECTION operator--(DIRECTION &d, int) { DIRECTION const t=d; --d; return t; }
inline DIRECTION operator++(DIRECTION &d) { return d=static_cast<DIRECTION>(d+1);}
inline DIRECTION operator++(DIRECTION &d, int) { DIRECTION const t=d; ++d; return t; }

typedef struct tagSquare
{
	STATE eState;
	UINT wTargetDist;
	DIRECTION eDirection;
} SQUARE;

//Used for sorting points in QSortPoint().
typedef struct tagSortPoint
{
	POINT xy;
	UINT wScore;
} SORTPOINT;

class CPathMap
{
	public:
	//Public functions.
	CPathMap(const UINT wCols, const UINT wRows, POINT xyTarget);
	~CPathMap(void);
	bool CalcPaths(const UINT wMaxDistance=0);
	void GetDebugOutput(bool bShowDirection, bool bShowState, bool bShowDistance, 
			string &strOutput) const;
	static int GetDXFromDir(const DIRECTION eDir);
	static int GetDYFromDir(const DIRECTION eDir);
	void GetRecPaths(const UINT wX, const UINT wY, POINT *lpxyPath,
			UINT &wNumPaths) const;
	void GetSquare(const UINT wX, const UINT wY, SQUARE &lpSquare) const;
	bool IsCalcDone(void) const;
	void Reset(void);
	void SetSquare(const UINT wX, const UINT wY, const bool bIsObstacle);	
	void SetTarget(const POINT xyTarget);

	//Public data.
	bool bConstructorSuccess;
	UINT wCols;
	UINT wRows;
	SQUARE *lpSquares;

private:
	//Private functions.
	inline UINT			GetCol(const UINT wSquareIndex) const;
	static inline DIRECTION	GetDirFromDxDy(const UINT dx, const UINT dy);
	inline UINT			GetRow(const UINT wSquareIndex) const;
	inline UINT			GetSquareIndex(const POINT xy) const;
	inline UINT			GetSquareIndex(const UINT x, const UINT y) const;
	static void				StableSortPoints(const UINT nElements,
			SORTPOINT *lpSortPoints);

	//Private data.
	POINT xyTarget;
	UINT wNumImmRecalcs;
	UINT wImmRecalcI;
	POINT *lpxyImmRecalc;	

	PREVENT_DEFAULT_COPY(CPathMap);
};

#endif //...#ifndef PATHMAP_H

// $Log: Pathmap.h,v $
// Revision 1.1  2003/02/25 00:01:38  erikh2000
// Initial check-in.
//
// Revision 1.13  2002/12/22 01:17:40  mrimer
// Changed path scoring metric to discourage standing still.
// Changed parameter type (from POINT to UINT,UINT).
//
// Revision 1.12  2002/11/22 01:49:31  mrimer
// Made some parameters const.
//
// Revision 1.11  2002/11/18 18:47:00  mrimer
// Replaced the unstable quicksort with a stable sort that works just as fast here.
// Documented a path-scoring bug and suggested a solution for next release.
// Changed several ints to UINTs.  Made several vars and parameters const.  Simplified conditional statements.
// Changed member naming convention to be consistent.
//
// Revision 1.10  2002/11/14 19:22:23  mrimer
// Added PREVENT_DEFAULT_COPY.  Made some parameters const.
//
// Revision 1.9  2002/07/17 16:08:15  mrimer
// Added FUTURE CONSIDERATIONS.
//
// Revision 1.8  2002/07/05 17:58:14  mrimer
// Removed #include <windows.h>
//
// Revision 1.7  2002/06/21 04:25:57  mrimer
// Revised includes.
//
// Revision 1.6  2002/06/05 03:03:09  mrimer
// Added includes.
//
// Revision 1.5  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2002/02/25 03:38:15  erikh2000
// Made read-only methods const.
// Added CPathMap::GetDebugOutput().
//
// Revision 1.2  2001/11/06 00:05:07  erikh2000
// I removed the second dwMaxTime param of CalcPaths() to simplify it a bit.
//
// Revision 1.1.1.1  2001/10/01 22:20:31  erikh2000
// Initial check-in.
//
