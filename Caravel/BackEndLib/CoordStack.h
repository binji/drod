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
#include "CoordIndex.h"

#include <vector>
#include <algorithm>

struct COORDNODE
{
	UINT wX, wY;
   bool operator==(const COORDNODE& rhs) const
   {
      return this->wX == rhs.wX && this->wY == rhs.wY;
   }
};

class CCoordStack
{
public:
	CCoordStack(const UINT reserve = 10)
	{
      this->data.reserve(reserve);
	}
   CCoordStack(const CCoordStack& rhs)
   {
      this->data = rhs.data;
   }
   CCoordStack& operator=(const CCoordStack& rhs)
   {
      this->data = rhs.data;
      return *this;
   }
	void Push(const UINT wX, const UINT wY)
	{
      COORDNODE newNode;
		newNode.wX = wX;
		newNode.wY = wY;
      this->data.push_back(newNode);
	}
	bool Pop(UINT &wX, UINT &wY)
	{
		//Check for empty stack.
		if (this->data.size() == 0) return false;

      vector<COORDNODE>::iterator last = this->data.end();
      --last;

		//Set vars for return.
		wX = last->wX;
		wY = last->wY;

      this->data.erase(last);
		return true;
	}
	bool Top(UINT &wX, UINT &wY) const
	//Return values in first element w/o removing it.
	{
		//Check for empty stack.
		if (this->data.size() == 0) return false;

      vector<COORDNODE>::const_reverse_iterator last = this->data.rbegin();

		//Set vars for return.
		wX = last->wX;
		wY = last->wY;

		return true;
	}
	bool GetAt(const UINT wIndex, UINT &wX, UINT &wY) const
	//Return values in first element w/o removing it.
	{
		//Check for empty stack.
		if (this->data.size() <= wIndex) return false;

		//Set vars for return.
		wX = this->data[wIndex].wX;
		wY = this->data[wIndex].wY;

		return true;
	}
	void Append(const UINT wX, const UINT wY)
	//Add element to end of list.
	{
      COORDNODE newNode;
		newNode.wX = wX;
		newNode.wY = wY;
      this->data.insert(this->data.begin(), newNode);
	}
	bool IsMember(const UINT wX, const UINT wY) const
	//Checks whether (wX,wY) is in the list.
	{
      COORDNODE findNode;
		findNode.wX = wX;
		findNode.wY = wY;
      return find(this->data.begin(), this->data.end(), findNode) != this->data.end();
	}
	bool Remove(const UINT wX, const UINT wY)
	//Removes element (wX,wY) from the list.
	{
      COORDNODE findNode;
		findNode.wX = wX;
		findNode.wY = wY;
      vector<COORDNODE>::iterator found;
      found = find(this->data.begin(), this->data.end(), findNode);
      if (found != this->data.end())
      {
         this->data.erase(found);
         return true;
      }
      return false;
	}
	void Clear()
	{
      this->data.clear();
	}
	DWORD GetSize() const
	{
      return this->data.size();
	}

   void AddTo(CCoordIndex &coordIndex)
   //Sets all the coords in this object in coordIndex.
   {
      for (vector<COORDNODE>::iterator trav = this->data.begin();
            trav != this->data.end(); ++trav)
      {
         //ASSUME: (wX,wY) are within the bounds of coordIndex
         coordIndex.Add(trav->wX, trav->wY);
      }
   }

private:
   vector<COORDNODE> data;

};

#endif //...#ifndef COORDSTACK_H

// $Log: CoordStack.h,v $
// Revision 1.5  2003/08/15 19:27:31  mrimer
// Added GetAt().  Revised some code.
//
// Revision 1.4  2003/07/29 03:49:56  mrimer
// Added AddTo().
//
// Revision 1.3  2003/07/29 02:59:44  schik
// Fixed the return value for Remove
//
// Revision 1.2  2003/07/29 02:58:04  schik
// Revamped CCoordStack to use vector<> instead of a linked list
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.9  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.8  2002/09/13 20:43:32  mrimer
// Added Top(), Append(), and some assertions.
//
