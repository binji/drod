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

#ifndef COORDSTACK_H
#define COORDSTACK_H

#include "Types.h"

typedef struct tagCoordNode COORDNODE;
typedef struct tagCoordNode
{
	UINT wX, wY;
	COORDNODE *pNext;
} COORDNODE;

class CCoordStack
{
public:
	CCoordStack(void)
		: pFirst(&Sentry)
		, dwCount(0)
	{
		Sentry.wX = Sentry.wY = -1;
		Sentry.pNext = NULL;
	}
	void Push(const UINT wX, const UINT wY)
	{
		ASSERT(wX != Sentry.wX || wY != Sentry.wY);
		COORDNODE *pNew = new COORDNODE;
		pNew->wX = wX;
		pNew->wY = wY;
		pNew->pNext = this->pFirst;
		this->pFirst = pNew;
		++(this->dwCount);
	}
	bool Pop(UINT &wX, UINT &wY)
	{
		//Check for empty stack.
		if (this->pFirst == &Sentry) return false;

		//Set vars for return.
		wX = this->pFirst->wX;
		wY = this->pFirst->wY;
		
		//Remove the current first member.
		COORDNODE *pDelete = this->pFirst;
		this->pFirst = this->pFirst->pNext;
		delete pDelete;
		
		--(this->dwCount);
		return true;
	}
	bool Top(UINT &wX, UINT &wY)
	//Return values in first element w/o removing it.
	{
		//Check for empty stack.
		if (this->pFirst == &Sentry) return false;

		//Set vars for return.
		wX = this->pFirst->wX;
		wY = this->pFirst->wY;

		return true;
	}
	void Append(const UINT wX, const UINT wY)
	//Add element to end of list.
	{
		ASSERT(wX != Sentry.wX || wY != Sentry.wY);
		COORDNODE *pTrav = this->pFirst;
		COORDNODE *pNew = new COORDNODE;
		pNew->wX = wX;
		pNew->wY = wY;
		pNew->pNext = &Sentry;
		if (pTrav == &Sentry)
			this->pFirst = pNew;
		else
		{
			while (pTrav->pNext != &Sentry)
				pTrav = pTrav->pNext;
			pTrav->pNext = pNew;
		}
		++(this->dwCount);
	}
	bool IsMember(const UINT wX, const UINT wY)
	//Checks whether (wX,wY) is in the list.
	{
		COORDNODE *pTrav = this->pFirst;
		while (pTrav != &Sentry)
		{
			if (pTrav->wX == wX && pTrav->wY == wY)
				return true;
			pTrav = pTrav->pNext;
		}
		return false;
	}
	bool Remove(const UINT wX, const UINT wY)
	//Removes element (wX,wY) from the list.
	{
		if (this->pFirst == &Sentry) return false;
		ASSERT(wX != Sentry.wX || wY != Sentry.wY);

		COORDNODE *pTemp, *pTrav = this->pFirst;
		if (pTrav->wX == wX && pTrav->wY == wY)
		{
			//remove first element
			this->pFirst = pTrav->pNext;
			delete pTrav;

			--(this->dwCount);
			return true;
		}
		while (pTrav->pNext != &Sentry)
		{
			pTemp = pTrav->pNext;
			if (pTemp->wX == wX && pTemp->wY == wY)
			{
				//remove element 'pTemp'
				pTrav->pNext = pTemp->pNext;	//cut 'pTemp' out of list
				delete pTemp;

				--(this->dwCount);
				return true;
			}
			pTrav = pTrav->pNext;
		}
		return false;
	}
	void Clear(void)
	{
		if (this->pFirst == &Sentry) return;

		COORDNODE *pSeek = this->pFirst, *pDelete;
		while (pSeek != &Sentry)
		{
			pDelete = pSeek;
			pSeek = pSeek->pNext;
			delete pDelete;
		}

		this->pFirst = &Sentry;
		this->dwCount = 0L;
	}
	DWORD GetSize(void)
	{
		return this->dwCount;
	}

private:
	COORDNODE	Sentry;
	COORDNODE *	pFirst;
	DWORD		dwCount;

	PREVENT_DEFAULT_COPY(CCoordStack);
};

#endif //...#ifndef COORDSTACK_H

// $Log: CoordStack.h,v $
// Revision 1.1  2003/02/25 00:01:17  erikh2000
// Initial check-in.
//
// Revision 1.8  2002/09/13 20:43:32  mrimer
// Added Top(), Append(), and some assertions.
//
