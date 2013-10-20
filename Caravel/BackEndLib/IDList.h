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

//IDList.h
//Declarations for CIDList.
//Class for managing a list of ID values.

#ifndef IDLIST_H
#define IDLIST_H

#include "Types.h"
#include "AttachableObject.h"

struct IDNODE
{
	DWORD dwID;
	CAttachableObject *pvPrivate;
	bool bIsAttached;
	IDNODE *pNext;
};

//Simplifies passing array count param to IsInList().
#define IDCOUNT(dw) (sizeof(dw) / sizeof(DWORD))

class CIDList : public CAttachableObject
{
public:
   CIDList();
	virtual ~CIDList() {Clear();}

	CIDList(const CIDList &Src);
	CIDList &operator= (const CIDList &Src);
	void operator += (const CIDList &Src);
	void operator -= (const CIDList &Src);
	
	void	Add(const DWORD dwID, CAttachableObject *pvPrivate = NULL, const bool bIsAttached=false);
	bool	AreIDsInList(const UINT wIDArrayCount, const DWORD *pdwIDArray) const;
	void	Clear();
	bool	ContainsList(const CIDList &Against) const;
	IDNODE *Get(const DWORD dwIndex) const;
	IDNODE *GetByID(const DWORD dwID) const;
	DWORD	GetID(const DWORD dwIndex) const;
	DWORD	GetSize() const {return this->dwSize;}
	bool	IsIDInList(const DWORD dwID) const;
	void	Remove(const DWORD dwID);

protected:
	IDNODE *pFirstID;
	IDNODE *pLastID;
	DWORD dwSize;
};

#endif //...#ifndef IDLIST_H

// $Log: IDList.h,v $
// Revision 1.2  2003/08/22 02:45:42  mrimer
// Revised CAttachableObject usage.  Tweaked some code.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.15  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.14  2002/10/10 03:14:35  erikh2000
// Added method to remove an ID from list.
//
// Revision 1.13  2002/07/21 00:18:06  erikh2000
// Added contributor credits for mrimer.
//
// Revision 1.12  2002/07/19 20:23:16  mrimer
// Added CAttachableObject references.
//
// Revision 1.11  2002/07/05 17:59:34  mrimer
// Minor fixes (includes, etc.)
//
// Revision 1.10  2002/05/10 22:33:12  erikh2000
// Added -= operator that lets you subtract matching members of one list from another.
//
// Revision 1.9  2002/04/28 23:51:55  erikh2000
// Fixed a memory leak error in assignment operator.
// Added += operator to use for appending one list to another.
//
// Revision 1.8  2002/04/09 01:06:42  erikh2000
// Added CIDList::DoesContainList() method.
//
// Revision 1.7  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.6  2001/12/08 01:43:18  erikh2000
// Added ability to attach private data to CIDList so that it is deleted in ~CIDList.
//
// Revision 1.5  2001/11/05 04:56:39  erikh2000
// Add copy contructor and CIDList assignment operator.
//
// Revision 1.4  2001/10/21 00:32:09  erikh2000
// Renamed IsInList() overloads to IsIDInList() and AreIDsInList().
// Made read-only methods const so that const pointers to CIDList could have their const methods called.
//
// Revision 1.3  2001/10/20 10:15:36  erikh2000
// Added private data member that is stored along with IDs in list.
//
// Revision 1.2  2001/10/07 04:38:34  erikh2000
// General cleanup--finished removing game logic code from DROD.EXE.
// Added ProcessCommand() method to CCurrentGame to be used as interface between UI and game logic.
//
// Revision 1.1.1.1  2001/10/01 22:20:15  erikh2000
// Initial check-in.
//
