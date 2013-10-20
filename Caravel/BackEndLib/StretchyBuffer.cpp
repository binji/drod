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

//StretchyBuffer.cpp.
//Implementation of CStretchyBuffer.

#include "StretchyBuffer.h"
#include "Assert.h"
#include "Types.h"

#include <string.h>

//
//Public methods.
//

//****************************************************************************************
CStretchyBuffer::CStretchyBuffer(void)
	: pBuf(NULL)
//Constructor.
{
	Clear();
}

//****************************************************************************************
CStretchyBuffer::CStretchyBuffer(
//Constructor that initializes buffer.
//
//Params:
	const BYTE *pSetBuf,	//(in) Buffer that will be copied to memory alloced by this object.
	const DWORD dwSetBufSize)		//(in) Size of buffer in bytes.
	: pBuf(NULL)
{
	Clear();
	Set(pSetBuf, dwSetBufSize);
}

//****************************************************************************************
void CStretchyBuffer::Clear(void)
//Zeros member vars and frees resources associated with this object.
{
	delete[] this->pBuf;
	this->pBuf = NULL;

	this->dwBufSize = this->dwAllocSize = 0L;
}

//******************************************************************************
void CStretchyBuffer::Encode()
//Encodes/decodes data in the buffer.
//
//Params:
{
	BYTE *pBuf = this->pBuf;
	for (UINT nIndex=Size(); nIndex--; )
	{
		*pBuf = ~*pBuf;	//flip all bits
		++pBuf;
	}
}

//****************************************************************************************
BYTE *CStretchyBuffer::GetCopy(void)
//Gets a copy of this object's buffer that this object will not delete or change.
//
//Returns:
//Pointer to new buffer or NULL.
{
	if (this->dwBufSize == 0) return NULL; //Buffer is empty.

	//Alloc new buffer.
	BYTE *pCopyBuf = new BYTE[this->dwBufSize];
	if (!pCopyBuf) return NULL;

	//Copy bytes into new buffer.
	memcpy(pCopyBuf, this->pBuf, this->dwBufSize);

	return pCopyBuf;
}

//****************************************************************************************
void CStretchyBuffer::Set(
//Sets object's buffer to contain same bytes as specified buffer.
//
//Params:
	const BYTE *pSetBuf, //(in) Buffer to copy.
	DWORD dwSetBufSize)  //(in) Size of buffer.
{
	ASSERT(dwSetBufSize > 0);
	
	//Make sure object buffer is large enough.
	if (dwSetBufSize > this->dwAllocSize)
	{
		if (!Alloc(dwSetBufSize)) {ASSERTP(false, "Allocation failure."); return;}
	}

	//Copy buffer to object buffer.
	memcpy(this->pBuf, pSetBuf, dwSetBufSize);
	this->dwBufSize = dwSetBufSize;
}

void CStretchyBuffer::SetSize(
//back door, when pBuf is written to directly
//
//Params:
	const DWORD dwSetBufSize)	//(in)
{
	ASSERT(this->dwAllocSize >= this->dwBufSize);
	this->dwBufSize = dwSetBufSize;
}

//****************************************************************************************
void CStretchyBuffer::Append(
//Appends a buffer to the object's buffer.
//
//Params:
	const BYTE *pAddBuf, //(in) Buffer to append.
	DWORD dwAddBufSize)	 //(in) Size of buffer.
{
	ASSERT(dwAddBufSize > 0);

	//Make sure object buffer is large enough.
	if (this->dwBufSize + dwAddBufSize > this->dwAllocSize)
	{
		if (!Realloc((this->dwBufSize + dwAddBufSize) * 2)) {ASSERTP(false, "Allocation failure."); return;}
	}

	//Copy buffer to end of object buffer.
	memcpy(this->pBuf + this->dwBufSize, pAddBuf, dwAddBufSize);
	this->dwBufSize += dwAddBufSize;
}

//****************************************************************************************
bool CStretchyBuffer::Alloc(
//Allocs the buffer.  
//
//Only call Alloc() if you wish to copy data into the buffer directly (not using 
//class methods) and the buffer must be preallocated to a specific size.
//
//Params:
	DWORD dwNewAllocSize) //(in) Size to allocate buffer to.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(dwNewAllocSize > 0);

	if (dwNewAllocSize == this->dwAllocSize) return true; //Nothing to do.

	//Delete existing buffer.
	delete[] this->pBuf;
	this->dwAllocSize = this->dwBufSize = 0;

	//Alloc new buffer and set related member vars.
	this->pBuf = new BYTE[dwNewAllocSize];
	if (!this->pBuf) return false;
	this->dwAllocSize = dwNewAllocSize;

	return true;
}

//****************************************************************************************
bool CStretchyBuffer::Realloc(
//Reallocates the buffer to a new size and containing whatever is in the existing buffer.
//
//Only call Alloc() if you wish to copy data into the buffer directly (not using 
//class methods) and the buffer must be preallocated to a specific size.
//
//Params:
	DWORD dwNewAllocSize) //(in) Size to reallocate buffer to.
//
//Returns:
//True if successful, false if not.
{
	BYTE *pNewBuf = NULL;
	DWORD dwCopySize;

	ASSERT(dwNewAllocSize > 0);

	if (dwNewAllocSize == this->dwAllocSize) return true; //Nothing to do.

	//Create new buffer.
	pNewBuf = new BYTE[dwNewAllocSize];
	if (!pNewBuf) 
	{
		Clear(); 
		return false;
	}

	//Copy bytes from old buffer into new buffer.  Data will be truncated if
	//it all can't fit.
	dwCopySize = this->dwBufSize;
	if (dwCopySize > 0)
	{
		if (dwCopySize > dwNewAllocSize) dwCopySize = dwNewAllocSize;
		ASSERT(this->pBuf);
		memcpy(pNewBuf, this->pBuf, dwCopySize);
	}
	
	//Delete old buffer.
	delete[] this->pBuf;

	//Set member vars for new buffer.
	this->pBuf = pNewBuf;
	this->dwAllocSize = dwNewAllocSize;
	this->dwBufSize = dwCopySize;

	return true;
}

// $Log: StretchyBuffer.cpp,v $
// Revision 1.2  2003/10/06 02:39:39  erikh2000
// Added descriptions to assertians.CVS: ----------------------------------------------------------------------
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.9  2003/05/04 00:21:50  mrimer
// Moved CStretchyBuffer encoding/decoding into the class (from CFiles).
//
// Revision 1.8  2003/04/28 22:15:40  mrimer
// Fixed a bug.  Added error checking.  Made multiple Append calls more efficient.
//
// Revision 1.7  2002/12/22 00:42:06  mrimer
// Made parameter const.  Revised Clear().
//
// Revision 1.6  2002/06/21 04:48:48  mrimer
// Refined includes.
//
// Revision 1.5  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.4  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.3  2002/04/11 10:07:33  erikh2000
// Changed a few private methods to public methods.
//
// Revision 1.2  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.1.1.1  2001/10/01 22:20:32  erikh2000
// Initial check-in.
//
