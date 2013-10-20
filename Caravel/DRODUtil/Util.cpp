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

//Util.cpp
//Implementation of CUtil.

#include <BackEndLib/GameStream.h>
#include <BackEndLib/Types.h>
#include <BackEndLib/Files.h>
#include "Assert.h"
#define INCLUDED_FOR_UTIL_CPP
#include "Util.h"
#undef INCLUDED_FOR_UTIL_CPP

#include <mk4.h>
#ifdef WIN32
#include <direct.h>
#elif defined __linux__
#include <unistd.h> //chdir
#endif
#include <sys/stat.h>
#include <string>

WCHAR g_szarrVersions[VERSION_COUNT][MAXLEN_VERSION + 1] =
{
	{{'1'},{'.'},{'1'},{'1'},{'c'},{0}},
	{{'1'},{'.'},{'5'},{0}},
	{{'1'},{'.'},{'6'},{0}}
};

//
//CUtil public methods.
//

//*****************************************************************************
CUtil::CUtil(
//Constructor.
//
//Params:
	VERSION eSetVersion,
	const WCHAR* pszSetPath)
{
	ASSERT(pszSetPath && WCSlen(pszSetPath));

	this->eVersion = eSetVersion;
	this->strPath = pszSetPath;
}

//*****************************************************************************
bool CUtil::DoesFileExist(
//Does a file exist?
//
//Params:
	const WCHAR* pszFilepath)	//(in)	Either full path to file or just the 
								//		filename.  In the latter case, the 
								//		object path is used.
//
//Returns:
//True if it does, false if it doesn't.
const
{
	//Check for path separators in file.
	WSTRING strFullPath;
	const WCHAR *pszSeek = pszFilepath;
	while (*pszSeek != '\0' && *pszSeek != SLASH) ++pszSeek;
	if (*pszSeek == SLASH)	//Found a path separator, so treat as a full path.
		strFullPath = pszFilepath;
	else					//Looks like just a filename.
	{
		//Concat full path from object path and filename.
		strFullPath = this->strPath.c_str();
		strFullPath += wszSlash;
		strFullPath += pszFilepath;
	}
	
	//Open the file to verify its existence.  The call below will not leave
	//the file open.
	return CFiles::DoesFileExist(strFullPath.c_str());
}

//*****************************************************************************
bool CUtil::IsPathValid(
//Is a path valid.
//
//Params:
	const WCHAR* pszPath)		//(in)	Path should not end in separator.
//
//Returns:
//True if it is, false if it isn't.
{
	bool bIsValid;
#ifdef WIN32
	struct _stat buf;
	bIsValid = !_wstat(pszPath, &buf) ? (buf.st_mode & _S_IFDIR) != 0 : false;
#else
   WSTRING wstrPath = pszPath;
	char *szPath = new char[wstrPath.length()+1];
	struct stat buf;
	UnicodeToAscii(wstrPath, szPath);
	bIsValid = !stat(szPath, &buf) ? S_ISDIR(buf.st_mode) : false;
	delete[] szPath;
#endif
	return bIsValid;
}

//*****************************************************************************
bool CUtil::CopyViewDef(
//Copy a viewdef's data from one storage file to another.  The dest viewdef must
//have same structure as source.
//
//Params:
  const WCHAR *pwzSrcFilepath,  //(in)  Path to storage file containing 
                                //      viewdef.
  const WCHAR *pwzDestFilepath, //(in)  Destination storage file where copy 
                                //      will be made.
  const char *pszViewDefName)   //(in)  Name of viewdef in source storage 
                                //      file.
{
    bool bSuccess = false;
    bool bDestPendingChanges = false;

    ASSERT(pwzSrcFilepath && pwzDestFilepath && pszViewDefName);

    //Open storage files.
    c4_Storage *pSrcStorage = new c4_Storage;
    c4_Storage *pDestStorage = new c4_Storage;
    CGameStream *pSrcStream = new CGameStream(pwzSrcFilepath);
    CGameStream *pDestStream = new CGameStream(pwzDestFilepath);
    if (!pSrcStorage || !pDestStorage || !pSrcStream || !pDestStream) goto Cleanup;
    if (!pSrcStorage->LoadFrom(*pSrcStream)) goto Cleanup;
    if (!pDestStorage->LoadFrom(*pDestStream)) goto Cleanup;

    {
        //Copy rows over.
        c4_View SrcView = pSrcStorage->View(pszViewDefName);
        c4_View DestView = pDestStorage->View(pszViewDefName);
        int nRowCount = SrcView.GetSize();
        bDestPendingChanges = true;
        for (int nRow = 0; nRow < nRowCount; ++nRow)
        {
            c4_RowRef row = SrcView.GetAt(nRow);
            DestView.Add(row);
        }
    }

    //Commit changes to destination storage.
    pDestStorage->Commit();
    bDestPendingChanges = false;

    //Success.
    bSuccess = true;

Cleanup:
    if (!bSuccess && bDestPendingChanges) pDestStorage->Rollback();
    if (bSuccess) pDestStream->Save(pDestStorage);
    delete pDestStorage;
    delete pSrcStorage;
    delete pDestStream;
    delete pSrcStream;
    return bSuccess;
}

//
//CUtil private methods.
//

//*****************************************************************************
void CUtil::PrintNotImplemented(void) const
//Print a standard message.
{
	printf("The requested operation has not been implemented.\r\n");
}

// $Log: Util.cpp,v $
// Revision 1.12  2003/10/20 17:51:43  erikh2000
// Twiddling.
//
// Revision 1.11  2003/07/25 01:03:18  mrimer
// Relative path fixes (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/07/24 18:01:33  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/05/25 22:54:59  erikh2000
// Fixed some includes.
//
// Revision 1.8  2003/05/22 23:42:15  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.7  2003/05/08 23:32:51  erikh2000
// Added v1.6 support.
//
// Revision 1.6  2003/04/28 18:07:44  erikh2000
// Minor corrections.
//
// Revision 1.5  2003/04/21 22:00:53  mrimer
// Added Unicode support.
//
// Revision 1.4  2002/05/15 13:09:25  mrimer
// Added log macro to end of file.
//
