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

//DbPackedVars.h
//Declarations for CDbPackedVars.
//Class for accessing multiple variables in blob intended for database storage.
//
//USAGE
//
//To add new variables to CDbPackedVars, call one of the SetVar() methods overloaded
//for the type you want to store.  For types not represented, you can choose to add
//a new SetVar() or use the general-purpose SetVar(const void *, DWORD) method.
//
//To retrieve a variable, call one of the GetVar() methods.  Returned pointers are only
//good until CDbPackedVars goes out of scope or Clear() is called.
//
//To pack all the current variables into a single buffer, that can be stored in
//a database field, call GetPackedBuffer().
//
//You can unpack a byte buffer into CDbPackedVars in a few different ways.  Depending on 
//where your calling code is starting from you may wish to use the "const byte *" or
//"c4_BytesRef &" assignment operators.  Unpacking the byte buffer will result in the
//variables that were stored in the buffer to become accessible through the class methods.

#ifndef DBPACKEDVARS_H
#define DBPACKEDVARS_H

#include <BackEndLib/Types.h>
#include <BackEndLib/Wchar.h>

#ifdef __sgi
#include <string.h>
#else
#include <cstring>
#endif

#include <mk4.h>

typedef struct tagUnpackedVar UNPACKEDVAR;
typedef struct tagUnpackedVar
{
	char *pszName;
	void *pValue;
	DWORD dwValueSize;
	UNPACKEDVAR *pNext;
} UNPACKEDVAR;

class CDbPackedVars
{
public:
	CDbPackedVars() {pFirstVar=pLastVar=NULL; Clear();}
	CDbPackedVars(const CDbPackedVars& Src);
	CDbPackedVars(const BYTE *pBuf);
	~CDbPackedVars();

   CDbPackedVars& operator=(const CDbPackedVars &Src) {SetMembers(Src); return *this;}
	const BYTE * operator = (const BYTE *pBuf) {UnpackBuffer(pBuf, 1); return pBuf;}
   CDbPackedVars & operator = (const c4_BytesRef &Buf)
 	{
 		c4_Bytes Bytes = (c4_Bytes) Buf;
		UnpackBuffer(Bytes.Contents(), Bytes.Size());
		return *this;
	}
	c4_BytesRef & operator = (c4_BytesRef &Buf) 
	{
		c4_Bytes Bytes = (c4_Bytes) Buf;
		UnpackBuffer(Bytes.Contents(), Bytes.Size()); 
		return Buf;
	}

	void			Clear();
	bool			DoesVarExist(const char *pszVarName) const
	{
		return (FindVarByName(pszVarName)!=NULL);
	}
	BYTE *			GetPackedBuffer(DWORD &dwBufferSize);
	void *			GetVar(const char *pszVarName, const void *pNotFoundValue = NULL) const;
	const char *	GetVar(const char *pszVarName, const char *pszNotFoundValue = NULL) const;
	const WCHAR *	GetVar(const char *pszVarName, const WCHAR *pwczNotFoundValue = NULL) const;
	int				GetVar(const char *pszVarName, int nNotFoundValue = 0) const;
	UINT			GetVar(const char *pszVarName, UINT wNotFoundValue = 0) const;
	DWORD			GetVar(const char *pszVarName, DWORD dwNotFoundValue = 0) const;
	char			GetVar(const char *pszVarName, char cNotFoundValue = 0) const;
	BYTE			GetVar(const char *pszVarName, BYTE ucNotFoundValue = 0) const;

	DWORD			GetVarValueSize(const char *pszVarName) const;

	void *			SetVar(const char *pszVarName, const void *pValue, DWORD dwValueSize);
	char *			SetVar(const char *pszVarName, const char *pszValue) 
	{
		return (char *) SetVar(pszVarName, (const void *) pszValue, strlen(pszValue)+1);
	}
	WCHAR *			SetVar(const char *pszVarName, const WCHAR *pwczValue) 
	{
		return (WCHAR *) SetVar(pszVarName, (const void *) pwczValue, (WCSlen(pwczValue)+1)*sizeof(WCHAR));
	}
	DWORD *			SetVar(const char *pszVarName, DWORD dwValue) 
	{
		return (DWORD *) SetVar(pszVarName, (const void *) &dwValue, sizeof(dwValue));
	}
	int *			SetVar(const char *pszVarName, int nValue) 
	{
		return (int *) SetVar(pszVarName, (const void *) &nValue, sizeof(nValue));
	}
	UINT *			SetVar(const char *pszVarName, UINT wValue) 
	{
		return (UINT *) SetVar(pszVarName, (const void *) &wValue, sizeof(wValue));
	}
	char *			SetVar(const char *pszVarName, char cValue)
	{
		return (char *) SetVar(pszVarName, (const void *) &cValue, sizeof(cValue));
	}
	BYTE *			SetVar(const char *pszVarName, unsigned char ucValue)
	{
		return (BYTE *) SetVar(pszVarName, (const void *) &ucValue, sizeof(ucValue));
	}

private:
	void SetMembers(const CDbPackedVars &Src);
	UNPACKEDVAR *	FindVarByName(const char *pszVarName) const;
	bool			UnpackBuffer(const BYTE *pBuf, const UINT bufSize);

	UNPACKEDVAR *	pFirstVar;
	UNPACKEDVAR *	pLastVar;
};

#endif //...#ifndef DBEXTRAVARS_H

// $Log: DbPackedVars.h,v $
// Revision 1.19  2003/08/09 16:26:36  mrimer
// Fixed cross-platform bug: zero-size byte buffer might be uninitialized.
//
// Revision 1.18  2003/06/30 16:46:49  mrimer
// Fixed a bug.
//
// Revision 1.17  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.16  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.15  2003/05/22 23:39:00  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.14  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.13  2003/02/17 00:48:12  erikh2000
// Remove L" string literals.
//
// Revision 1.12  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.11  2002/08/29 09:17:29  erikh2000
// Added methods to support string storage.
//
// Revision 1.10  2002/07/20 23:01:08  erikh2000
// Revised #includes.
//
// Revision 1.9  2002/06/21 04:17:22  mrimer
// Revised includes.
//
// Revision 1.8  2002/06/09 06:16:55  erikh2000
// Fixed a couple of errors that had made unpacking vars impossible.
// Added type size checking to GetVar() methods.
//
// Revision 1.7  2002/06/05 03:03:09  mrimer
// Added includes.
//
// Revision 1.6  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.5  2002/04/09 01:03:54  erikh2000
// Changed UCHAR declarations to BYTE for consistency.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/12/01 01:22:43  erikh2000
// Changed GetPackedBuffer() method to return buffer size.
//
// Revision 1.2  2001/11/25 21:02:43  erikh2000
// Added usage docs.
//
// Revision 1.1.1.1  2001/10/01 22:20:10  erikh2000
// Initial check-in.
//
