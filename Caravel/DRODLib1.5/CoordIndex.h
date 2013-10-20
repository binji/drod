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

#ifndef COORDINDEX_H
#define COORDINDEX_H

//CoordIndex.h
//Declarations for CCoordIndex.
//Class for indexing X,Y coordinates.  Should be used for quick membership
//checks.  Be careful about using to track coords over a large area because
//the index can take a lot of memory.  For a 38 x 32 room, 1216+ bytes 
//will be used.

#include "Types.h"
#include "AttachableObject.h"

class CCoordIndex : public CAttachableObject
{
public:
	CCoordIndex(void);
	~CCoordIndex(void);

	void			Add(UINT wX, UINT wY);
	void			Clear(void);
	bool			Exists(UINT wX, UINT wY) const;
	const BYTE *	GetIndex(void) const {return this->pbytIndex;}
	DWORD			GetSize(void) const {return this->dwCoordCount;}
	bool			Init(UINT wSetCols, UINT wSetRows);
	void			Remove(UINT wX, UINT wY);

private:
	DWORD	dwCoordCount;
	UINT	wCols, wRows;
	BYTE *	pbytIndex;

	PREVENT_DEFAULT_COPY(CCoordIndex);
};

#endif //...#ifndef COORDINDEX_H