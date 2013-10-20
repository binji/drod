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

//DbPackedVars.cpp
//Implementation of CDbPackedVars.

#include "DbPackedVars.h"
#include <BackEndLib/StretchyBuffer.h>
#include <BackEndLib/Ports.h>

//
//Public methods.
//

//*******************************************************************************************
CDbPackedVars::CDbPackedVars(
//Constructor.
//
//Params:
	const BYTE *pBuf)	//(in) Buffer containing packed variables.
	: pFirstVar(NULL), pLastVar(NULL)
{
	UnpackBuffer(pBuf, 1);
}

//*******************************************************************************************
CDbPackedVars::~CDbPackedVars(void)
//Destructor.
{
	Clear();
}

//*******************************************************************************************
CDbPackedVars::CDbPackedVars(const CDbPackedVars& Src)
//Copy constructor.
	: pFirstVar(NULL), pLastVar(NULL)
{
   SetMembers(Src);
}

//*******************************************************************************************
void CDbPackedVars::Clear(void)
//Zeroes member vars and frees resources associated with this object.
{
	//Delete all members of packed var list.
	if (this->pFirstVar)
	{
		UNPACKEDVAR *pSeek = this->pFirstVar, *pDelete;
		while (pSeek)
		{
			pDelete = pSeek;
			pSeek = pSeek->pNext;
			delete[] pDelete->pszName;
			delete[] (char*)pDelete->pValue;
			delete pDelete;
		}
		this->pFirstVar = this->pLastVar = NULL;
	}
	ASSERT(this->pLastVar == NULL);
}

//*******************************************************************************************
void CDbPackedVars::SetMembers(const CDbPackedVars &Src)
{
	Clear();

	//Each iteration copies one unpacked var.
	UNPACKEDVAR *pSrcVar = Src.pFirstVar;
	while (pSrcVar)
	{
		SetVar(pSrcVar->pszName, pSrcVar->pValue, pSrcVar->dwValueSize);
		pSrcVar = pSrcVar->pNext;
	}
}

//*******************************************************************************************
void * CDbPackedVars::GetVar(
//Gets value of a variable.
//
//Params:
	const char *pszVarName,		//(in)	Name of var to get value for.
	const void *pNotFoundValue)	//(in)	Pointer to return if variable is not found.  Default
								//		is NULL.
//
//Returns:
//Pointer to variable value.
const
{
	//Find var with matching name.
	UNPACKEDVAR *pFoundVar = FindVarByName(pszVarName);
	if (pFoundVar)
		return pFoundVar->pValue;
	else
		return (void *) pNotFoundValue;
}
int CDbPackedVars::GetVar(const char *pszVarName, int nNotFoundValue) const
//Overload for returning an int value.
{
	int *pnRet = (int *)GetVar(pszVarName, (void *)NULL);
	if (pnRet)
	{
#ifdef __sgi
//		LittleToBig(pnRet);
#endif
		ASSERT(GetVarValueSize(pszVarName)==sizeof(int));
		return *pnRet;
	}
	else
		return nNotFoundValue;
}
UINT CDbPackedVars::GetVar(const char *pszVarName, UINT wNotFoundValue) const
//Overload for returning a UINT value.
{
	UINT *pwRet = (UINT *) GetVar(pszVarName, (void *)NULL);
	if (pwRet)
	{
#ifdef __sgi
//		LittleToBig(pwRet);
#endif
		ASSERT(GetVarValueSize(pszVarName)==sizeof(UINT));
		return *pwRet;
	}
	else
		return wNotFoundValue;
}
DWORD CDbPackedVars::GetVar(const char *pszVarName, DWORD dwNotFoundValue) const
//Overload for returning a DWORD value.
{
	DWORD *pdwRet = (DWORD *) GetVar(pszVarName, (void *)NULL);
	if (pdwRet)
	{
#ifdef __sgi
//		LittleToBig(pdwRet);
#endif
		ASSERT(GetVarValueSize(pszVarName)==sizeof(DWORD));
		return *pdwRet;
	}
	else
		return dwNotFoundValue;
}
char CDbPackedVars::GetVar(const char *pszVarName, char cNotFoundValue) const
//Overload for returning a char value.
{
	char *pcRet = (char *) GetVar(pszVarName, (void *)NULL);
	if (pcRet)
	{
		ASSERT(GetVarValueSize(pszVarName)==sizeof(char));
		return *pcRet;
	}
	else
		return cNotFoundValue;
}
const char * CDbPackedVars::GetVar(const char *pszVarName, const char *pszNotFoundValue) const
//Overload for returning a char * value.
{
	char *pszRet = (char *) GetVar(pszVarName, (void *)NULL);
	if (pszRet)
	{
		ASSERT(GetVarValueSize(pszVarName)==strlen(pszRet)+1);
		return pszRet;
	}
	else
		return pszNotFoundValue;
}
const WCHAR * CDbPackedVars::GetVar(const char *pszVarName, const WCHAR *pwczNotFoundValue) const
//Overload for returning a char * value.
{
	WCHAR *pwczRet = (WCHAR *) GetVar(pszVarName, (void *)NULL);
	if (pwczRet)
	{
#ifdef __sgi
//		LittleToBig(pwczRet, WCSlen(pwczRet));
#endif
		ASSERT(GetVarValueSize(pszVarName)==(WCSlen(pwczRet)+1)*sizeof(WCHAR));
		return pwczRet;
	}
	else
		return pwczNotFoundValue;
}
BYTE CDbPackedVars::GetVar(const char *pszVarName, BYTE ucNotFoundValue) const
//Overload for returning a BYTE value.
{
	char *pucRet = (char *) GetVar(pszVarName, (void *)NULL);
	if (pucRet)
	{
		ASSERT(GetVarValueSize(pszVarName)==sizeof(BYTE));
		return *pucRet;
	}
	else
		return ucNotFoundValue;
}

//*******************************************************************************************
void * CDbPackedVars::SetVar(
//Sets value of either an existing or new var.  If no var with specified name exists then a
//new one is created.
//
//Params:
	const char *pszVarName,		//(in) Name of var to set value for.
	const void *pValue,			//(in) Buffer containing value.
	DWORD dwValueSize)			//(in) Size of value in buffer.
//
//Returns:
//Pointer to memory where value was stored or NULL if not successful.
{
	bool bSuccess = true;

	//Try to get an existing unpacked var with matching name,
	UINT wVarNameSize;
	UNPACKEDVAR *pVar = FindVarByName(pszVarName);
	if (!pVar) //No existing var of same name.
	{
		//Create new var and add to list.
		pVar = new UNPACKEDVAR;
		pVar->pNext = NULL;
		pVar->pszName = NULL;
		pVar->pValue = NULL;
		if (this->pLastVar)
		{
			this->pLastVar->pNext = pVar;
			this->pLastVar = pVar;
		}
		else
		{
			this->pFirstVar = this->pLastVar = pVar;
		}

		//Copy var name to new var.
		wVarNameSize = strlen(pszVarName) + 1;
		ASSERT(wVarNameSize < 256); //256 = reasonable limit for name size.
		pVar->pszName = new char[wVarNameSize];
		if (!pVar->pszName) {bSuccess=false; goto Cleanup;}
		memcpy(pVar->pszName, pszVarName, wVarNameSize);
	}

	//Set value of var.
	ASSERT(dwValueSize < 10000); //10000 = reasonable limit for value size.
	delete[] (char*)pVar->pValue;
	pVar->pValue = new BYTE[dwValueSize];
	if (!pVar->pValue) {bSuccess=false; goto Cleanup;}
	memcpy(pVar->pValue, pValue, dwValueSize);
	pVar->dwValueSize = dwValueSize;

Cleanup:
	if (!bSuccess) 
	{
		Clear();
		return NULL;
	}
	return pVar->pValue;
}

//*******************************************************************************************
BYTE * CDbPackedVars::GetPackedBuffer(
//Packs all of the vars stored in this object into one buffer allocated here.
//
//Params:
	DWORD &dwBufferSize)	//(out) Size in bytes of packed buffer.
//
//Returns:
//Pointer to new buffer which caller must delete.
{
	CStretchyBuffer PackedBuf;

	//Each iteration packs one var into buffer.
	UNPACKEDVAR *pReadVar = this->pFirstVar;
	while (pReadVar)
	{
		PackedBuf += (UINT) strlen(pReadVar->pszName) + 1;
		PackedBuf += pReadVar->pszName;
		PackedBuf += pReadVar->dwValueSize;
		PackedBuf.Append((const BYTE *) pReadVar->pValue, pReadVar->dwValueSize);

		pReadVar = pReadVar->pNext;
	}

	//Append end code to buffer.
	PackedBuf += (UINT) 0;
	dwBufferSize = PackedBuf.Size();
	return PackedBuf.GetCopy();
}

//*******************************************************************************************
DWORD CDbPackedVars::GetVarValueSize(
//Get size of a var.
//
//Params:
	const char *pszVarName) //(in)	Var to get size from.
//
//Returns:
//The size or 0L if no match.
const
{
	UNPACKEDVAR *pVar = FindVarByName(pszVarName);
	return (pVar) ? pVar->dwValueSize : 0L;
}

//
//Private methods.
//

//*******************************************************************************************
bool CDbPackedVars::UnpackBuffer(
//Unpacks variables in buffer into member vars that can be easily accessed.
//
//Params:
	const BYTE *pBuf,	//(in) Buffer to unpack.
   const UINT bufSize)  //(in) Size of buffer (value of 0 indicates nothing to unpack)
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess=true;
	UINT wVarNameSize = 0;
	UNPACKEDVAR *pNewVar = NULL;

	//Packed variable buffer format is:
	//{VarNameSize1 UINT}{VarName1 SZ}{VarValueSize1 DWORD}{VarValue1}
	//{VarNameSize2 UINT}...
	//{EndCode UINT}

	//Remove any vars that have been previously unpacked.
	Clear();

   //Check for empty buffer.
	const BYTE *pRead = pBuf;
   if (bufSize == 0) goto Cleanup;  //Success--nothing to unpack.
	if (!pRead) goto Cleanup; //Success--nothing to unpack.

	//Each iteration unpacks one variable.
	memcpy(&wVarNameSize, pRead, sizeof(UINT));
	pRead += sizeof(UINT);
#ifdef __sgi
	LittleToBig(&wVarNameSize);
#endif
	while (wVarNameSize != 0)
	{
		ASSERT(wVarNameSize < 256); //256 = reasonable limit to var name size.
		
		//Create new packed var.
		pNewVar = new UNPACKEDVAR;

		//Add new unpacked var to list.
		pNewVar->pNext = NULL;
		pNewVar->pszName = NULL;
		pNewVar->pValue = NULL;
		if (this->pLastVar)
		{
			this->pLastVar->pNext = pNewVar;
			this->pLastVar = pNewVar;
		}
		else
		{
			this->pFirstVar = this->pLastVar = pNewVar;
		}
		
		//Copy var name to new unpacked var.
		pNewVar->pszName = new char[wVarNameSize];
		if (!pNewVar->pszName) {bSuccess=false; goto Cleanup;}
		memcpy(pNewVar->pszName, pRead, wVarNameSize);
		ASSERT(pNewVar->pszName[wVarNameSize - 1] == '\0'); //Var name s/b null-terminated.
		pRead += wVarNameSize;

		//Get size of value.
		memcpy(&pNewVar->dwValueSize, pRead, sizeof(DWORD));
#ifdef __sgi
		LittleToBig(&pNewVar->dwValueSize);
#endif
		ASSERT(pNewVar->dwValueSize < 10000); //10000 = reasonable limit to value size.
		pRead += sizeof(DWORD);

		//Copy value to new unpacked var.
		pNewVar->pValue = new char[pNewVar->dwValueSize];
		if (!pNewVar->pValue) {bSuccess=false; goto Cleanup;}
		memcpy(pNewVar->pValue, pRead, pNewVar->dwValueSize);
		pRead += pNewVar->dwValueSize;

		//Get size of next variable name or end code.
		wVarNameSize = *((UINT *) pRead);
		memcpy(&wVarNameSize, pRead, sizeof(UINT));
#ifdef __sgi
		LittleToBig(&wVarNameSize);
#endif
		pRead += sizeof(UINT);
	} //...unpack next var in buffer.

Cleanup:
	if (!bSuccess) Clear();
	return bSuccess;
}

//*******************************************************************************************
UNPACKEDVAR *CDbPackedVars::FindVarByName(
//Finds an unpacked variable in list that matches specified name.
//
//Params:
	const char *pszVarName) //(in) Name of var to find.
//
//Returns:
//Pointer to unpacked var if a match is found, otherwise NULL.
const
{
	UNPACKEDVAR *pSeek = this->pFirstVar;

	while (pSeek)
	{
		if (strcmp(pSeek->pszName, pszVarName)==0) return pSeek; //Found it.
		pSeek = pSeek->pNext;
	}
	return NULL; //No match.
}

// $Log: DbPackedVars.cpp,v $
// Revision 1.17  2003/08/09 16:26:36  mrimer
// Fixed cross-platform bug: zero-size byte buffer might be uninitialized.
//
// Revision 1.16  2003/06/30 16:46:49  mrimer
// Fixed a bug.
//
// Revision 1.15  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.14  2003/05/23 21:30:37  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.13  2003/05/22 23:39:02  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.12  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.11  2002/11/15 02:07:17  mrimer
// Fixed constructors.
//
// Revision 1.10  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.9  2002/08/29 09:17:29  erikh2000
// Added methods to support string storage.
//
// Revision 1.8  2002/07/20 23:00:34  erikh2000
// Changed UCHAR to BYTE for consistency.
//
// Revision 1.7  2002/06/21 04:52:17  mrimer
// Revised includes.
//
// Revision 1.6  2002/06/09 06:16:55  erikh2000
// Fixed a couple of errors that had made unpacking vars impossible.
// Added type size checking to GetVar() methods.
//
// Revision 1.5  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/12/01 01:22:43  erikh2000
// Changed GetPackedBuffer() method to return buffer size.
//
// Revision 1.2  2001/10/30 04:13:47  md5i
// NULL might not be void*.
//
// Revision 1.1.1.1  2001/10/01 22:20:10  erikh2000
// Initial check-in.
//
