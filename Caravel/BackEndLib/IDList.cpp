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

//IDList.cpp
//Implementation of CIDList.

#include "Types.h"
#include "Assert.h"
#include "IDList.h"

//************************************************************************************
CIDList::CIDList()
   : CAttachableObject()
   , pFirstID(NULL), pLastID(NULL)
   , dwSize(0)
{}

//************************************************************************************
CIDList::CIDList(const CIDList &Src)
   : CAttachableObject()
   , pFirstID(NULL), pLastID(NULL)
   , dwSize(0)
//Copy constructor.  
//
//Note: Any attached private data will be unattached in copy.  In other words, the 
//responsibility for deleting attached private data will not transfer from source 
//CIDList to this CIDList.  Caller should probably not reference private data in this 
//CIDList if source CIDList goes out of scope.
{
	IDNODE *pSeek = Src.pFirstID;
	while (pSeek != NULL)
	{
		Add(pSeek->dwID, pSeek->pvPrivate);
		pSeek = pSeek->pNext;
	}
}

//************************************************************************************
CIDList &CIDList::operator= (const CIDList &Src)
//Assignment operator.  Will clear this list and copy from source list.
//
//Note: Any attached private data will be unattached in copy.  In other words, the 
//responsibility for deleting attached private data will not transfer from source 
//CIDList to this CIDList.  Caller should probably not reference private data in this 
//CIDList if source CIDList goes out of scope.
{
	Clear();

	IDNODE *pSeek = Src.pFirstID;
	while (pSeek != NULL)
	{
		Add(pSeek->dwID, pSeek->pvPrivate);
		pSeek = pSeek->pNext;
	}
	return *this;
}

//************************************************************************************
void CIDList::operator += (const CIDList &Src)
//Append source list to this one.
//
//Note: Any attached private data will be unattached in copy.  In other words, the 
//responsibility for deleting attached private data will not transfer from source 
//CIDList to this CIDList.  Caller should probably not reference private data in this 
//CIDList if source CIDList goes out of scope.
{
	IDNODE *pSeek = Src.pFirstID;
	while (pSeek != NULL)
	{
		Add(pSeek->dwID, pSeek->pvPrivate);
		pSeek = pSeek->pNext;
	}
}

//************************************************************************************
void CIDList::operator -= (const CIDList &Src)
//Remove any IDs in this list that match IDs in source list.
{
	IDNODE *pSeek = this->pFirstID, *pDelete, *pPrev = NULL;
	while (pSeek != NULL)
	{
		if (Src.IsIDInList(pSeek->dwID)) 
		{
			//Remove this node.
			pDelete = pSeek;
			pSeek = pSeek->pNext;
			delete pDelete;

			if (pPrev) 
				pPrev->pNext = pSeek;
			else
				this->pFirstID = pSeek;
			this->dwSize--;
		}
		else	
		{
			//Keep this node.
			pPrev = pSeek;
			pSeek = pSeek->pNext;
		}
	}
}

//************************************************************************************
void CIDList::Add(
//Add an ID to the list.
//
//Params:
	const DWORD dwID,		//(in)	ID to add.
	CAttachableObject *pvPrivate,//(in)	Private data to associate with ID.  If NULL
							//		(default) then no private data will be associated.
							//		Never pass an array allocated with "new []"--encapsulate
							//		in a class or struct before passing.
	const bool bIsAttached)	//(in)	If set to true then the private data will be deleted
							//		by CIDList when it destructs.  If false (default) 
							//		then data should get deleted elsewhere, but not by
							//		caller.
{
	IDNODE *pNew = NULL;
	
	if (IsIDInList(dwID)) return; //Don't add a duplicate.

	//Create new ID node.
	pNew = new IDNODE;
	pNew->dwID = dwID;
	pNew->pvPrivate = pvPrivate;
	pNew->bIsAttached = bIsAttached;
	pNew->pNext = NULL;

	//Add new ID node to list.
	if (this->pLastID)
	{
		this->pLastID->pNext = pNew;
		this->pLastID = pNew;
	}
	else
	{
		this->pFirstID = this->pLastID = pNew;
	}
	this->dwSize++;
}

//************************************************************************************
void CIDList::Remove(
//Removes an ID from list, deleting associated private data if attached.
//
//Params:
	const DWORD dwID)
{
	//Find the node.
	IDNODE *pSeek = this->pFirstID, *pBeforeSeek = NULL;
	while (pSeek)
	{
		if (pSeek->dwID == dwID) break;
		pBeforeSeek = pSeek;
		pSeek = pSeek->pNext;
	}
	if (!pSeek) {ASSERTP(false, "ID to remove not found."); return;} //No match.

	//Remove the node from the list.
	if (pBeforeSeek)
		pBeforeSeek->pNext = pSeek->pNext;
	else
	{
		ASSERT(this->pFirstID == pSeek);
		this->pFirstID = pSeek->pNext;
	}
	if (pSeek == this->pLastID)
		this->pLastID = pBeforeSeek;
	this->dwSize--;

	//Delete the node and maybe its private data.
	if (pSeek->bIsAttached)
		delete pSeek->pvPrivate;
	delete pSeek;
}

//************************************************************************************
void CIDList::Clear()
//Frees resources associated with object and zeroes members.
{
	IDNODE *pDelete, *pSeek=this->pFirstID;

	while (pSeek)
	{
		pDelete = pSeek;
		pSeek = pSeek->pNext;
		if (pDelete->bIsAttached) delete pDelete->pvPrivate;
		delete pDelete;
	}
	this->pFirstID = this->pLastID = NULL;

	this->dwSize = 0L;
}

//************************************************************************************
bool CIDList::IsIDInList(DWORD dwID) const
//Checks for an ID in the list.
{
	IDNODE *pSeek = this->pFirstID;

	while (pSeek)
	{
		if (pSeek->dwID == dwID) return true; //Found it.
		pSeek = pSeek->pNext;
	}
	return false; //Didn't find it.
}

//************************************************************************************
bool CIDList::AreIDsInList(
//Checks for an occurrence of more than one ID in a list.
//
//Params:
	UINT wIDArrayCount,			//# of elements in array.
	const DWORD *pdwIDArray)	//IDs to check for.
//
//Returns:
//True if any of the IDs were found, false if not.
const
{
	IDNODE *pSeek = this->pFirstID;

	UINT wCompareI = 0;
	while (pSeek)
	{
		for (wCompareI = 0; wCompareI < wIDArrayCount; ++wCompareI)
		{
			if (pSeek->dwID == pdwIDArray[wCompareI]) return true; //Found it.
		}
		pSeek = pSeek->pNext;
	}
	return false;
}

//************************************************************************************
bool CIDList::ContainsList(
//Does this list contain all IDs that a second list has.
//
//Params:
	const CIDList &Against)		//(in)	Second list to check against.
//
//True if it does, false if not.
const
{
	IDNODE *pAgainst = Against.pFirstID;
	while (pAgainst)
	{
		if (!IsIDInList(pAgainst->dwID))
			return false;
		pAgainst = pAgainst->pNext;
	}
	return true;
}

//************************************************************************************
DWORD CIDList::GetID(DWORD dwIndex) const
//Gets an ID from list by index.
{
	IDNODE *pSeek = this->pFirstID;

	ASSERT(dwIndex < dwSize);

	DWORD dwCurrentIndex = 0;
	while (pSeek)
	{
		if (dwCurrentIndex == dwIndex) return pSeek->dwID; //Found it.
		pSeek = pSeek->pNext;
		dwCurrentIndex++;
	}
	return 0; //Didn't find it.
}

//************************************************************************************
IDNODE *CIDList::Get(DWORD dwIndex) const
//Gets an ID node from list by index.
{
	IDNODE *pSeek = this->pFirstID;

	if (dwIndex >= dwSize) return NULL;

	DWORD dwCurrentIndex = 0;
	while (pSeek)
	{
		if (dwCurrentIndex == dwIndex) return pSeek; //Found it.
		pSeek = pSeek->pNext;
		dwCurrentIndex++;
	}
	return NULL; //Didn't find it.
}

//************************************************************************************
IDNODE *CIDList::GetByID(DWORD dwID) const
//Gets an ID node from list by it's ID.
{
	IDNODE *pSeek = this->pFirstID;
	while (pSeek)
	{
		if (dwID == pSeek->dwID) return pSeek; //Found it.
		pSeek = pSeek->pNext;
	}
	return NULL; //Didn't find it.
}

// $Log: IDList.cpp,v $
// Revision 1.5  2003/10/09 14:52:56  mrimer
// Tweaking (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/10/06 02:39:39  erikh2000
// Added descriptions to assertians.CVS: ----------------------------------------------------------------------
//
// Revision 1.3  2003/08/22 02:45:42  mrimer
// Revised CAttachableObject usage.  Tweaked some code.
//
// Revision 1.2  2003/06/15 04:19:15  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.15  2002/10/10 03:14:35  erikh2000
// Added method to remove an ID from list.
//
// Revision 1.14  2002/07/21 00:18:06  erikh2000
// Added contributor credits for mrimer.
//
// Revision 1.13  2002/07/19 20:24:35  mrimer
// Modified CCueEvents to work with CAttachableObject.
//
// Revision 1.12  2002/05/10 22:33:12  erikh2000
// Added -= operator that lets you subtract matching members of one list from another.
//
// Revision 1.11  2002/04/28 23:51:55  erikh2000
// Fixed a memory leak error in assignment operator.
// Added += operator to use for appending one list to another.
//
// Revision 1.10  2002/04/09 01:06:41  erikh2000
// Added CIDList::DoesContainList() method.
//
// Revision 1.9  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.8  2002/02/15 02:45:03  erikh2000
// Fixed error in CIDList::Clear() causing access violation.
//
// Revision 1.7  2001/12/16 02:13:59  erikh2000
// Change CIDList::Get() to allow out of range index.
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
