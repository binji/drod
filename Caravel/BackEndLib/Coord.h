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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef COORD_H
#define COORD_H

#include "Types.h"
#include "AttachableObject.h"

//Class for storing one set of coords.
class CCoord : public CAttachableObject
{
public:
	CCoord(const UINT wSetCol, const UINT wSetRow) 
		: CAttachableObject(), wCol(wSetCol), wRow(wSetRow)
	{ }
	UINT wCol, wRow;
};

//Class for storing coords plus direction of movement onto.
class CMoveCoord : public CAttachableObject
{
public:
	CMoveCoord(const UINT wSetCol, const UINT wSetRow, const UINT wSetO) 
		: CAttachableObject(), wCol(wSetCol), wRow(wSetRow), wO(wSetO)
	{ }
	UINT wCol, wRow, wO;
};

#endif //...#ifndef COORD_H

// $Log: Coord.h,v $
// Revision 1.2  2003/08/22 02:45:42  mrimer
// Revised CAttachableObject usage.  Tweaked some code.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
