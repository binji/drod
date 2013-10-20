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
 *
 * ***** END LICENSE BLOCK ***** */

//StretchyBuffer.h
//Declarations for CStretchyBuffer.
//Class for accessing a dynamically sized buffer.

#ifndef STRETCHYBUFFER_H
#define STRETCHYBUFFER_H

#include "Types.h"
#include "Assert.h"
#include "Wchar.h"
#include <cstring>

class CStretchyBuffer
{
public:
	CStretchyBuffer(void);
	CStretchyBuffer(const BYTE *pSetBuf, const DWORD dwSetBufSize);
	~CStretchyBuffer(void) {Clear();}
	
	operator BYTE * () const {return pBuf;}
	operator const BYTE * () const {return pBuf;}
	
	CStretchyBuffer & operator += (DWORD dwAdd) {Append((const BYTE *) &dwAdd, sizeof(dwAdd)); return *this;}
	CStretchyBuffer & operator += (UINT wAdd) {Append((const BYTE *) &wAdd, sizeof(wAdd)); return *this;}
	CStretchyBuffer & operator += (int nAdd) {Append((const BYTE *) &nAdd, sizeof(nAdd)); return *this;}
	CStretchyBuffer & operator += (const char *pszAdd) {Append((const BYTE *) pszAdd, strlen(pszAdd)+1); return *this;}
	CStretchyBuffer & operator += (const WCHAR *pwczAdd) {Append((const BYTE *) pwczAdd, (wcslen(pwczAdd) + 1) * sizeof(WCHAR)); return *this;}
	CStretchyBuffer & operator += (BYTE bytAdd) {Append((const BYTE *) &bytAdd, sizeof(bytAdd)); return *this;}

	bool	Alloc(DWORD dwNewAllocSize);
	void	Append(const BYTE *pAddBuf, DWORD dwAddBufSize);
	void	Clear(void);
	BYTE *	GetCopy(void);
	bool	Realloc(DWORD dwNewAllocSize);
	void	Set(const BYTE *pSetBuf, DWORD dwSetBufSize);
	void	SetSize(const DWORD dwSetBufSize) {dwBufSize = dwSetBufSize;}	//back door, when pBuf is written to directly
	DWORD	Size(void) const {return dwBufSize;}

private:
	DWORD dwBufSize;
	DWORD dwAllocSize;
	BYTE *pBuf;

	PREVENT_DEFAULT_COPY(CStretchyBuffer);
};

#endif //...#ifndef STRETCHYBUFFER_H

// $Log: StretchyBuffer.h,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.11  2003/02/17 00:48:12  erikh2000
// Remove L" string literals.
//
// Revision 1.10  2002/12/22 00:42:07  mrimer
// Made parameter const.  Revised Clear().
//
// Revision 1.9  2002/08/29 09:18:18  erikh2000
// Made some methods const.
//
// Revision 1.8  2002/07/17 22:37:26  mrimer
// Added SetSize().
//
// Revision 1.7  2002/06/05 23:58:34  mrimer
// Added an #include.
//
// Revision 1.6  2002/05/21 19:06:07  erikh2000
// Added a WCHAR append operator.
//
// Revision 1.5  2002/04/11 10:07:33  erikh2000
// Changed a few private methods to public methods.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/12/08 01:43:51  erikh2000
// Added BYTE overload for assignment operator.
// Added PREVENT_DEFAULT_COPY() macro to class declaration.
//
// Revision 1.2  2001/10/09 14:33:34  md5i
// operator += methods now return lvalues
//
// Revision 1.1.1.1  2001/10/01 22:20:32  erikh2000
// Initial check-in.
//
