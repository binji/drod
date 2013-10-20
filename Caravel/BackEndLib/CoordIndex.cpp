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

//CoordIndex.cpp.
//Implementation of CCoordIndex,

#include "Assert.h"
#include "Types.h"
#include "CoordIndex.h"

//
//CCoordIndex public methods.
//

//*******************************************************************************
CCoordIndex::CCoordIndex(
//Constructor.
//
//Params:
   const UINT wCols, const UINT wRows) //[default = (0,0)]
   : CAttachableObject()
   , dwCoordCount(0L)
   , wCols(wCols), wRows(wRows)
   , pbytIndex(NULL)
{
   if (wCols && wRows)
      Init(wCols, wRows);
}

//*******************************************************************************
CCoordIndex::~CCoordIndex()
//Destructor.
{
	delete[] this->pbytIndex;
}

//*******************************************************************************
bool CCoordIndex::Init(
//Must be called before other methods to alloc the index.
//
//Params:
	const UINT wSetCols,	//(in) Defines size of internal index and range of
	const UINT wSetRows)	//(in) coords that other methods may accept.
//
//Returns:
//True if index was allocated, false if not.
{
	ASSERT(wSetCols > 0);
   ASSERT(wSetRows > 0);

	//500 = unreasonably large and probably an error.
	ASSERT(wSetCols < 500);
	ASSERT(wSetRows < 500);

	//Delete existing index storage.
	if (this->pbytIndex)
	{
		delete[] this->pbytIndex;
		this->pbytIndex = NULL;
		this->dwCoordCount = 0L;
	}

	//Create new index storage.
	this->pbytIndex = new BYTE[wSetCols * wSetRows];
	if (this->pbytIndex)
	{
		memset(this->pbytIndex, 0, wSetCols * wSetRows);
		this->wCols = wSetCols;
		this->wRows = wSetRows;
		return true;
	}

   //To make assertians fire in other methods that have been incorrectly 
	//called.
	this->wCols = this->wRows = 0;
	return false;
}

//*******************************************************************************
void CCoordIndex::Add(
//Adds a set of coords to the index.
//
//Params:
	const UINT wX, const UINT wY)
{
	ASSERT(wX < this->wCols && wY < this->wRows);
	DWORD dwSquareI = wY * this->wCols + wX;
	this->dwCoordCount += (this->pbytIndex[dwSquareI] == 0);
	ASSERT(this->dwCoordCount <= this->wCols * this->wRows);
	this->pbytIndex[dwSquareI] = 1;
}

//*******************************************************************************
void CCoordIndex::Remove(
//Removes a set of coords to the index.
//
//Params:
	const UINT wX, const UINT wY)
{
	ASSERT(wX < this->wCols && wY < this->wRows);
	DWORD dwSquareI = wY * this->wCols + wX;
	this->dwCoordCount -= (this->pbytIndex[dwSquareI] == 1);
	ASSERT(this->dwCoordCount <= this->wCols * this->wRows);
	this->pbytIndex[dwSquareI] = 0;
}

//*******************************************************************************
void CCoordIndex::Clear()
//Removes all coords from the index.
{
	if (this->pbytIndex)
		memset(this->pbytIndex, 0, this->wCols * this->wRows);
	this->dwCoordCount = 0L;
}

//*******************************************************************************
bool CCoordIndex::Exists(
//Are coords currently in the index?
//
//Params:
	const UINT wX, const UINT wY)
//
//Returns:
//True if they are, false if not.
const
{
	ASSERT(wX < this->wCols && wY < this->wRows);
	return (this->pbytIndex[wY * this->wCols + wX] == 1);
}

// $Log: CoordIndex.cpp,v $
// Revision 1.4  2003/08/22 02:45:42  mrimer
// Revised CAttachableObject usage.  Tweaked some code.
//
// Revision 1.3  2003/08/09 00:37:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/07/29 03:50:54  mrimer
// Revised constructor to allow initializing the dimensions.  Made destructor virtual.
//
