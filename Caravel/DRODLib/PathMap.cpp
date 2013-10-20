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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "Pathmap.h"
#include <BackEndLib/Ports.h>

#include <utility>		//std::swap

#ifdef _DEBUG
//#define DEBUG_PATHMAP //Comment out to get rid of pathmap debug outputs.

#	ifdef DEBUG_PATHMAP
#		include <stdlib.h>
#		include <stdio.h>
#	endif

#endif

//**********************************************************************************
//Module-scope vars.
const int m_dxDir[]={-1, 0, 1, -1, 0, 1, -1, 0, 1}; //x offsets that correspond to enumerated DIRECTION type.
const int m_dyDir[]={-1, -1, -1, 0, 0, 0, 1, 1, 1}; //y offsets that correspond to enumerated DIRECTION type.

//**********************************************************************************
CPathMap::CPathMap(
//Constructor.  Sets object vars to default values and allocates and initializes the map squares and immediate
//recalc arrays.
//
//Accepts:
	const UINT wCols, const UINT wRows, //Size to initialize map to.
	POINT xyTarget)
	: bConstructorSuccess(false)
	, wCols(wCols)
	, wRows(wRows)
	, lpSquares(NULL)
	, xyTarget(xyTarget)
	, lpxyImmRecalc(NULL)
{
	UINT wSquareI;
	SQUARE *pSquare;

	//Allocate map.
	const UINT wArea=wCols*wRows;
	this->lpSquares=new SQUARE[wArea];
	if (this->lpSquares==NULL) {ASSERTP(false, "Alloc failed."); return;}

	//Initialize all of the map squares.
	for (wSquareI=wArea; wSquareI--; )
	{
		this->lpSquares[wSquareI].eState=recalc;
		this->lpSquares[wSquareI].eDirection=none;
	}

	//Allocate immediate recalc array
	this->lpxyImmRecalc=new POINT[wArea];
	if (!this->lpxyImmRecalc) {ASSERTP(false, "Alloc failed.(2)");}

	//Get squares ready for recalc.
	pSquare = this->lpSquares + GetSquareIndex(this->xyTarget);
	pSquare->eDirection=none;
	pSquare->eState=ok;
	pSquare->wTargetDist=0;
	this->wNumImmRecalcs=1;
	this->lpxyImmRecalc[0]=this->xyTarget;
	this->wImmRecalcI=0;

	this->bConstructorSuccess=true;
}

//**********************************************************************************
CPathMap::~CPathMap(void)
//Destructor.
{
	delete [] this->lpSquares;
	delete [] this->lpxyImmRecalc;
}

//**********************************************************************************
bool CPathMap::CalcPaths(
//Calculates paths to a specified target.  If the target and map squares are unchanged and there is still a
//previous call did not calculate paths for all squares withing range, this work is continued.
//
//Accepts:
	const UINT wMaxDistance)          //Maximum distance from target square to calculate paths for (optional).
//
//Returns:
//true if requested path calculations are completed, false if not.
{
	UINT wImmRecI;
	UINT wNewImmRecalcs;
	UINT x, y;
	SQUARE *pSquare;
	UINT wLowestScore, wScore;
	SQUARE *pScoreSquare;

	if (this->IsCalcDone())
		return true;

	//Get current distance from target of calculated squares by looking at the
	//first square in the imm recalc array.
	UINT wDistance=this->lpSquares[GetSquareIndex(this->lpxyImmRecalc[0])].wTargetDist;
	
	//If a maximum distance has been specified, see if I've already reached it.
	if (wMaxDistance && wDistance>=wMaxDistance)
		goto IncompletePathMap;

	//Main loop--the perimeter of squares to be calculated reaches one square farther out from the
	//target each iteration.  Loop and function will exit (return true) if it runs out of squares to 
	//calculate.  Loop will exit if maximum range criteria has been specified and met.
	while (true)
	{
		//Add new squares to immediate recalculation array by finding eligible squares adjacent to
		//the current imm rec squares.
		wNewImmRecalcs=this->wNumImmRecalcs;
		for (wImmRecI = 0; wImmRecI < wNewImmRecalcs; wImmRecI++)
		{					
			x = this->lpxyImmRecalc[wImmRecI].x;
			y = this->lpxyImmRecalc[wImmRecI].y;
					
			//Check every adjacent square for recalc eligibility.
			for (DIRECTION dir = (DIRECTION)0; dir<DIR_COUNT; dir++)
			{
				if (dir == none)
					continue;

				const UINT nx = x + m_dxDir[dir];
				const UINT ny = y + m_dyDir[dir];
				
				if (nx >= this->wCols || ny >= this->wRows)
					continue;

				pSquare = this->lpSquares + (ny*this->wCols + nx);

				if (pSquare->eState == recalc)
				{
					pSquare->eState = immediate; 
					this->lpxyImmRecalc[this->wNumImmRecalcs].x = nx;
					this->lpxyImmRecalc[this->wNumImmRecalcs].y = ny;
					++this->wNumImmRecalcs;
				}
			}
		}
						
		//See if any squares were added to imm rec.
		if (this->wNumImmRecalcs == wNewImmRecalcs)
		{//Nope--done with calculating paths.
			this->wNumImmRecalcs = 0;
#ifdef DEBUG_PATHMAP
			{
				string strOutput = "---Complete Pathmap---\r\n";
				GetDebugOutput(true,false,false,strOutput);
				strOutput += "\r\n";
				GetDebugOutput(false,true,false,strOutput);
				DEBUGPRINT(strOutput.begin());
			}
#endif // DEBUG_PATHMAP
			return true;
		}

		//Move the new immediate recalculations up the array to the beginning.
		memmove((void *) this->lpxyImmRecalc,
				(void *) &this->lpxyImmRecalc[wNewImmRecalcs],
				(size_t) (sizeof(POINT) * (this->wNumImmRecalcs-wNewImmRecalcs)));
		this->wNumImmRecalcs -= wNewImmRecalcs;
			
		//For every square in the immediate recalculation array:
		for (wImmRecI = 0; wImmRecI < this->wNumImmRecalcs; wImmRecI++)
		{
			x = this->lpxyImmRecalc[wImmRecI].x;
			y = this->lpxyImmRecalc[wImmRecI].y;
			ASSERT(x <= this->wCols && y <= this->wRows);
			pSquare = this->lpSquares + (y * this->wCols + x);

			//Score all adjacent squares for movement towards the target.
			wLowestScore=10000;

			for (DIRECTION dir = (DIRECTION)0; dir<DIR_COUNT; dir++)
			{
				if (dir == none)
					continue;

				const int dx = m_dxDir[dir];
				const int dy = m_dyDir[dir];
				const UINT nx = x + dx;
				const UINT ny = y + dy;
				const bool perp = (!dx != !dy);  // horz or vert movement

				pScoreSquare = this->lpSquares + (ny*this->wCols + nx);

				if (nx >= this->wCols || ny >= this->wRows)
					continue;

				if (pScoreSquare->eState == ok)
				{
					wScore = pScoreSquare->wTargetDist * 2 + (perp ? 0 : 1);
					if (wScore < wLowestScore) 
					{
						wLowestScore = wScore; 
						pSquare->eDirection = dir;
					}
				}
			}

			//Set direction and target distance from adjacent square with lowest score.
			if (wLowestScore == 10000) //Surrounded by obstacles and/or uncalculated squares.
			{
				pSquare->eDirection = none;		//Nowhere to go!
				pSquare->wTargetDist = 10000;   //Ensures no other squares will point to this one.
			} else {
				pSquare->wTargetDist = wDistance + 1;  //Distance counter that increments 1 per loop.
			}
					
			//Square doesn't need recalculation anymore.
			pSquare->eState = ok;
		}

		++wDistance;
			
		//Continue main loop?
		if (wMaxDistance && wDistance>=wMaxDistance)
			break;
	} //...while (true)

IncompletePathMap:
  	//Exit without completing.
#ifdef DEBUG_PATHMAP
	{
		string strOutput = "---Incomplete Pathmap---\r\n";
		GetDebugOutput(true,false,false,strOutput);
		strOutput += "\r\n";
		GetDebugOutput(false,true,false,strOutput);
		DEBUGPRINT(strOutput.begin());
	}
#endif //DEBUG_PATHMAP
  	return false;
}

//**********************************************************************************
void CPathMap::GetRecPaths(
//Gets recommended paths to take from a specified square in order to get to the target.
//
//Accepts:
	const UINT wX, const UINT wY, //Square to request paths from.
//
//Returns by parameter:
	POINT *lpxyPaths,					//Coordinates of adjacent squares given in order of recommendation.  Caller
	//Should pass a pointer to memory allocated as an array of POINT[9].  9 is number
	//of possible directions.
	UINT &wNumPaths)			//Number of recommended paths returned.
const
{
	int dx, dy;
	UINT wSquareI;
	UINT diagonalScore;
	SORTPOINT sortPoints[9];

	//Put squares into an array that are valid squares to move to.
	wNumPaths=0;
	for (dy=-1; dy<2; dy++)
	{
		const UINT yTo = wY+dy;
		if (yTo<this->wRows)
		{
			for (dx=-1; dx<2; dx++)
			{
				const UINT xTo = wX+dx;
				if (xTo<this->wCols)
				{
					wSquareI=GetSquareIndex(xTo,yTo);
					if (!(this->lpSquares[wSquareI].eState==obstacle || 
						  this->lpSquares[wSquareI].eState==recalc) )
					{
						//Add the square to array.
						diagonalScore = (dx && dy) ? 2 : !dx && !dy ? 1 : 0;	//discourage diagonal moves and sitting still
						sortPoints[wNumPaths].xy.x=xTo;
						sortPoints[wNumPaths].xy.y=yTo;
						sortPoints[wNumPaths].wScore =
								this->lpSquares[wSquareI].wTargetDist*3 + diagonalScore;
						++wNumPaths;
					}
				} //...if (xTo<=this->wCols)
			} //...for (dx=-1; dx<2; dx++)
		} //...if (yTo<=this->wRows)
	} //...for (dy=-1; dy<2; dy++)
	
	//Sort the array of sort points by distance from swordsman.
	if (wNumPaths) StableSortPoints(wNumPaths, sortPoints);
	
	//Copy the sort points into the points that will be returned.
	for (wSquareI=wNumPaths; wSquareI--; )
	{
		lpxyPaths[wSquareI]=sortPoints[wSquareI].xy;
	}
}

//**********************************************************************************
void CPathMap::GetSquare(
//Gets a specified map square.  Object functions access square array directly.
//
//Accepts:
	const UINT wX, const UINT wY,
//this->lpSquares
//
//Returns by parameter:
	SQUARE &lpSquare)
const
{	
	lpSquare=this->lpSquares[GetSquareIndex(wX,wY)];
}

//***************************************************************************
void CPathMap::Reset(void)
//Sets all the map squares so that they need recalculation.
//
//Accepts:
//this->wCols
//this->wRows
//
//Changes:
//this->lpSquares
{
	const UINT wLastSquareI=this->wRows*this->wCols;
	
	//Initialize all of the map squares.
	for (UINT wSquareI=0; wSquareI<wLastSquareI; wSquareI++) 
	{//Every square's state is either obstacle or recalc.
		if (this->lpSquares[wSquareI].eState!=obstacle)
			this->lpSquares[wSquareI].eState=recalc;
	}
	
	//Get squares ready for recalc.
	SQUARE *const pSquare = this->lpSquares + GetSquareIndex(this->xyTarget);
	pSquare->eDirection=none;
	pSquare->eState=ok;
	pSquare->wTargetDist=0;
	this->wNumImmRecalcs=1;
	this->lpxyImmRecalc[0]=this->xyTarget;
	this->wImmRecalcI=0;
}

//**********************************************************************************
void CPathMap::SetTarget(
//Sets the target and if it differs from the last sets squares for recalculation
//
//Accepts:
	const POINT xyTarget)
//
//Changes:
//this->xyTarget
{
	if (!(xyTarget.x==this->xyTarget.x && xyTarget.y==this->xyTarget.y))
	{
		this->xyTarget=xyTarget;
		Reset();
	}
}

//*****************************************************************************
void CPathMap::SetSquare(
//Intended for calls outside object.  Sets the state of a specified square to obstacle or not an 
//obstacle.  Other states are not allowed.
//
//Accepts:
	const UINT wX, const UINT wY, 
	const bool bIsObstacle)	
//
//Changes:
//this->lpSquares
{
	SQUARE *const pSquare = this->lpSquares + GetSquareIndex(wX,wY);
		
	switch (pSquare->eState)
	{
	case ok:
	case recalc:
	case immediate:
		if (bIsObstacle)
		{
			pSquare->eState=obstacle;
			Reset();
		}
		break;

	case obstacle:
		if (!bIsObstacle)
		{
			pSquare->eState=recalc;
			Reset();
		}
		break;

	default: ASSERTP(false, "Bad square state.");
	}
}

//*****************************************************************************
//A bunch of snappy one-liners.
inline UINT CPathMap::GetSquareIndex(const POINT xy) const {return xy.y*this->wCols+xy.x;}
inline UINT CPathMap::GetSquareIndex(const UINT x, const UINT y) const {return y*this->wCols+x;}
inline UINT CPathMap::GetRow(const UINT wSquareIndex) const {return wSquareIndex / this->wCols;}
inline UINT CPathMap::GetCol(const UINT wSquareIndex) const {return wSquareIndex % this->wCols;}
inline DIRECTION CPathMap::GetDirFromDxDy(const UINT dx, const UINT dy) {return DIRECTION((dy+1)*3+dx+1);}
int CPathMap::GetDXFromDir(const DIRECTION eDir) {return m_dxDir[eDir];}
int CPathMap::GetDYFromDir(const DIRECTION eDir) {return m_dyDir[eDir];}

//*****************************************************************************
bool CPathMap::IsCalcDone(void) const
{
	return (!this->wNumImmRecalcs);
}

//*****************************************************************************
void CPathMap::StableSortPoints(
//Stable (bubble) sort for SORTPOINT struct.  Just as fast as a quick sort
//for 9 elements.  Faster for fewer elements.
//
//Accepts:
	const UINT nElements,
//
//Changes:
	SORTPOINT *pSortPoints)
{
	UINT nI;
	for (UINT nJ=nElements; --nJ; )
		for (nI=0; nI!=nJ; ++nI)
			if (pSortPoints[nI].wScore > pSortPoints[nI+1].wScore)
				std::swap(pSortPoints[nI],pSortPoints[nI+1]);
}

#if 0
//NOTE: Removed to be replaced with a stable sort that is just as fast.
//*****************************************************************************
void CPathMap::QSortPoints(
//Quick sort routine for SORTPOINT struct.  Called recursively.
//NOTE: This is not a stable sort.
//
//Accepts:
	const int nFirstI,
	const int nLastI,
//
//Changes:
	SORTPOINT *pSortPoints)
const
{
	SORTPOINT SortPointX=pSortPoints[(nFirstI+nLastI)/2], SortPointY;
	int nI=nFirstI, nJ=nLastI;

	while (nI<=nJ)
	{
		while (pSortPoints[nI].wScore<SortPointX.wScore && nI<nLastI)
			++nI;
		while (pSortPoints[nJ].wScore>SortPointX.wScore && nJ>nFirstI)
			--nJ;
		if (nI<=nJ)
		{
			SortPointY=pSortPoints[nI];					
			pSortPoints[nI]=pSortPoints[nJ];
			pSortPoints[nJ]=SortPointY;
			++nI;
			--nJ;
		}
	} //...while (nI<=nJ)

	if (nFirstI<nJ) QSortPoints(nFirstI, nJ, pSortPoints);
	if (nI<nLastI) QSortPoints(nI, nLastI, pSortPoints);
}
#endif

//**************************************************************************************
void CPathMap::GetDebugOutput(
//Gets output-formatted representation of pathmap for debugging purposes.
//
//Params:
	bool bShowDirection,	//(in)		Show direction attribute of squares?
	bool bShowState,		//(in)		Show state attribute of squares?
	bool bShowDistance,		//(in)		Show distance attribute of squares?
	string &strOutput)		//(in/out)	Appends several CRLF separated lines.
const
{
	ASSERT(bShowDirection || bShowState || bShowDistance);

	//typedef enum tagDirection {nw=0, n=1, ne=2, w=3, none=4, e=5, sw=6, s=7, se=8} DIRECTION;
	const char szarrDirection[10] = "789456123";

	//typedef enum tagState {obstacle=0, recalc=1, ok=2, immediate=3} STATE;
	const char szarrState[5] = "#?.!";
	
	//Distance
	//00 to 99, and -- for larger than 99.

	if (!this->lpSquares)
	{
		strOutput += "Pathmap not initialized.\r\n";
		return;
	}

	char szChar[2];
	szChar[1] = '\0';
	char szDistance[3];

	SQUARE *pSquare = this->lpSquares;

	//Each iteration concats output for one row of squares.
	for (UINT wRowI = 0; wRowI < this->wRows; ++wRowI)
	{
		//Each iteration concats output for one square.
		for (UINT wColI = 0; wColI < this->wCols; ++wColI)
		{
			//Append direction for square.
			if (bShowDirection)
			{
				szChar[0] = szarrDirection[pSquare->eDirection];
				strOutput += szChar;
			}

			//Append state for square.
			if (bShowState)
			{
				szChar[0] = szarrState[pSquare->eState];
				strOutput += szChar;
			}
			
			//Append distance for square.
			if (bShowDistance)
			{
				if (pSquare->wTargetDist < 100)
				{
					_itoa(pSquare->wTargetDist, szDistance, 10);
					strOutput += szDistance;
				}
				else
					strOutput += "--";
			}
			++pSquare;
		}

		//Append end of row CR/LF.
		strOutput += "\r\n";
	}
}

// $Log: PathMap.cpp,v $
// Revision 1.20  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.19  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.18  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.17  2003/05/22 23:39:02  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.16  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.15  2002/12/22 01:17:39  mrimer
// Changed path scoring metric to discourage standing still.
// Changed parameter type (from POINT to UINT,UINT).
//
// Revision 1.14  2002/11/23 00:13:29  mrimer
// Fixed some bugs.
//
// Revision 1.13  2002/11/22 01:59:30  mrimer
// Made some parameters const.
//
// Revision 1.12  2002/11/18 18:46:40  mrimer
// Replaced the unstable quicksort with a stable sort that works just as fast here.
// Documented a path-scoring bug and suggested a solution for next release.
// Changed several ints to UINTs.  Made several vars and parameters const.  Simplified conditional statements.
// Changed member naming convention to be consistent.
//
// Revision 1.11  2002/11/15 01:32:15  mrimer
// Moved member initialization in constructor's initialization list.
// Made some vars const.
//
// Revision 1.10  2002/10/04 18:56:45  mrimer
// Added const to some local vars.
//
// Revision 1.9  2002/06/24 20:34:46  mrimer
// Fixed a delete.
//
// Revision 1.8  2002/06/21 04:42:38  mrimer
// Revised includes.
//
// Revision 1.7  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.6  2002/02/25 03:38:38  erikh2000
// Several changes to correct bugs.
// Made read-only methods const.
// Added CPathMap::GetDebugOutput().
// Added debug outputs in CalcPaths().
//
// Revision 1.5  2002/02/23 04:57:11  erikh2000
// Fixed incorrect out of bounds check in CPathMap::CalcPaths().
//
// Revision 1.4  2002/02/07 23:27:04  erikh2000
// Fixed errors where "delete []" should be used instead of "delete".
//
// Revision 1.3  2001/11/16 07:18:14  md5i
// Brain functions added.
//
// Revision 1.2  2001/11/06 00:05:07  erikh2000
// I removed the second dwMaxTime param of CalcPaths() to simplify it a bit.
//
// Revision 1.1.1.1  2001/10/01 22:20:31  erikh2000
// Initial check-in.
//
