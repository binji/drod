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

//********************************************************************************************
//File operations module.
//
//Note: Be careful about infinite loops caused by calling ASSERT, VERIFY, or LogErr in this 
//module.
//********************************************************************************************

#ifdef WIN32 //Many things will not compile w/o WIN32 API.  Fix them if you are porting.
#	include <windows.h> //Should be first include.
#endif

#include "Files.h"
#include "Assert.h"

#include <string.h>
#include <stdio.h>
#include <mk4.h>

#include <string>
using std::string;

char *lpszStartupPath=NULL;

//Initialize static members of class.
char * CFiles::pszAppPath = NULL;
char * CFiles::pszDatPath = NULL;
DWORD CFiles::dwRefCount = 0;

//
//Public methods.
//

//******************************************************************************
CFiles::CFiles(void)
//Constructor.
{
	if (this->dwRefCount == 0) InitClass();
	this->dwRefCount++;
}

//******************************************************************************
CFiles::~CFiles(void)
//Destructor.
{
	this->dwRefCount--;
	if (this->dwRefCount == 0) DeinitClass();
}

//******************************************************************************
void CFiles::TryToFixDataPath()
//Error-- try to find the Data dir myself and create a new DataPath.txt.
{
	char pszDatPathTxt[MAX_PATH + 1];
	
	sprintf(pszDatPathTxt, "%s\\DataPath.txt", pszAppPath);
	
	if (!FindPossibleDatPath(pszAppPath, pszDatPath) ||
			!WriteDataPathTxt(pszDatPathTxt, pszDatPath, true)) {
		//Use app path so at least error logging may work later.
		strcpy(pszDatPath, pszAppPath);
	}
}

//*****************************************************************************
bool CFiles::DoesFileExist(
//Determines if a specified file exists by trying to open it.
//
//Params:
  const char *pszFilepath) //(in)
//
//Returns:
//True if file exist, false if not.
{
	OFSTRUCT ofs;

	return (OpenFile(pszFilepath, &ofs, OF_EXIST) != HFILE_ERROR);	
}

//*****************************************************************************
bool CFiles::HasReadWriteAccess(
//Determines if a specified file has read/write access by trying to open it.
//
//Params:
  const char *pszFilepath) //(in)
//
//Returns:
//True if file exist, false if not.
{
	OFSTRUCT ofs;
	
	HFILE hfile = OpenFile(pszFilepath, &ofs, OF_READWRITE);
	if (hfile != HFILE_ERROR)
	{
		_lclose(hfile);
		return true;
	}
	return false;
}

//******************************************************************************
void CFiles::AppendErrorLog(
//Appends text to the error log.
//
//Params:
	const char *pszText) //(in) Text to write at end of file.
{
	char *pszLogFilepath = new char[MAX_PATH+1];
	
	//Open file for append.
	OFSTRUCT ofs;
	sprintf(pszLogFilepath, "%s\\drod.err", GetDatPath());
	HFILE hfileLog = HFILE_ERROR;
	if (DoesFileExist(pszLogFilepath))
	{
		hfileLog = OpenFile(pszLogFilepath, &ofs, OF_WRITE);
		_llseek(hfileLog, 0, FILE_END);
	}
	else
		hfileLog = OpenFile(pszLogFilepath, &ofs, OF_CREATE | OF_WRITE);
	
	if (hfileLog != HFILE_ERROR)
	{
		//Write text to file.
		_lwrite(hfileLog, pszText, strlen(pszText));
		
		//Close file.
		_lclose(hfileLog);
	}
}

//******************************************************************************
bool CFiles::WriteDRODProfileString(
//Writes a string to DROD.INI.
//
//Params:
  const char *pszSection,	//(in)
  const char *pszKey,		//(in)
  const char *pszValue)		//(in)
//
//Returns:
//True if successful, false if not.
{
	char *pszINIPath = new char[MAX_PATH+1];
  
	sprintf(pszINIPath, "%s\\drod.ini", this->pszDatPath);
  
	bool bSuccess = (WritePrivateProfileString(pszSection, pszKey, pszValue, pszINIPath)!=0);
  
	delete[] pszINIPath;
  
	return bSuccess;
}

//******************************************************************************
bool CFiles::GetDRODProfileString(
//Gets a string from DROD.INI.
//
//Params:
	const char *pszSection, //(in)
	const char *pszKey,		//(in)
	char *pszValue)		//(in/out) Caller-alloced to MAX_PROFILE_STRING+1
//
//Returns:
//True if successfully found entry or false if not.
{
	char *pszINIPath = new char[MAX_PATH+1];

	sprintf(pszINIPath, "%s\\drod.ini", this->pszDatPath);

	GetPrivateProfileString(pszSection, pszKey, "", pszValue, 
		MAX_PROFILE_STRING, pszINIPath);
  
	delete[] pszINIPath;
	return (pszValue[0]!='\0');
}

//******************************************************************************
bool CFiles::ReadFileIntoBuffer(
//Reads a file into a buffer.  Preferrably only be used with small files.
//
//Params:	
	const char *pszFilepath,	//(in)	Full path to file to be read.
	CStretchyBuffer &Buffer)	//(out)	Receives file data.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = false;
	ASSERT(pszFilepath);
	ASSERT(Buffer.Size() == 0);

	OFSTRUCT ofs;
	HFILE hfile = OpenFile(pszFilepath, &ofs, OF_READ);
	if (hfile != HFILE_ERROR)
	{
		DWORD dwFileSize = GetFileSize((HANDLE)(hfile), NULL);
		if (dwFileSize > 0)
		{
			if (Buffer.Alloc(dwFileSize + 1))
			{
				DWORD dwReadCount;
				BYTE *pBuffer = (BYTE*)Buffer;
				if (ReadFile((HANDLE)(hfile), pBuffer, dwFileSize, &dwReadCount, NULL))
				{
					pBuffer[dwReadCount] = '\0';
					Buffer.SetSize(dwReadCount);
					bSuccess = true;
				}
			}
		}	
		CloseHandle((HANDLE)(hfile));
	}

	return bSuccess;
}

//******************************************************************************
bool CFiles::WriteBufferToFile(
//Writes a file from a buffer.  Overwrites an existing file.
//
//Params:	
	const char *pszFilepath,		//(in)	Full path to file to be written.
	const CStretchyBuffer &Buffer)	//(in)	Data to write.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = false;
	ASSERT(pszFilepath);
	ASSERT(Buffer.Size() != 0);

	OFSTRUCT ofs;
	HFILE hfile = OpenFile(pszFilepath, &ofs, OF_CREATE | OF_WRITE);
	if (hfile != HFILE_ERROR)
	{
		BYTE *pBuffer = (BYTE*)Buffer;
		DWORD dwBufferSize = Buffer.Size(), dwWriteCount;

		bSuccess = (WriteFile((HANDLE)(hfile), pBuffer, dwBufferSize, 
				&dwWriteCount, NULL) != 0);			

		CloseHandle((HANDLE)(hfile));
	}

	return bSuccess;
}

//******************************************************************************
void CFiles::EncodeBuffer(
//Encodes/decodes data in a buffer.
//
//Params:
	CStretchyBuffer &buffer)	//(in/out)	Buffer to encode/decode.
{
	BYTE *pBuf = (BYTE*)buffer;
	for (UINT nIndex=buffer.Size(); nIndex--; )
	{
		*pBuf = ~*pBuf;	//flip all bits
		++pBuf;
	}
}

//******************************************************************************
void CFiles::MutateFileName(
//Unprotected and protected datafiles differ by their last letter.
//This routine changes one variant to the other.
//Params:
	char *pszFilepath)		//(in/out)	Full path to file to be read.
							//Mutates file name by changing last letter.
{
	const int nLength = strlen(pszFilepath);
	if (FileIsEncrypted(pszFilepath))
	{
		switch (pszFilepath[nLength-2])
		{
		case 'm':
			pszFilepath[nLength-1] = 'p';
			break;
		case 'a':
			pszFilepath[nLength-1] = 'v';
			break;
		case '3':
			pszFilepath[nLength-1] = 'm';
			break;
		default:	//the general ".dat"
			pszFilepath[nLength-1] = 't';
			break;
		}
	} else {
		pszFilepath[nLength-1] = '_';
	}
}

//******************************************************************************
bool CFiles::GetTrueDatafileName(
//Find out true name of file to load.
//It may be protected/unprotected, so check variant (i.e. last char changed).
//
//Returns: whether file was found
//
//Params:
	char *const pszFilepath)	//(in/out)	Full path to file to be read.
										//Filename is mutated (by changing last letter)
										//if file of original name is not found.
{
	//If file exists, return true.
	OFSTRUCT ofs;
	HFILE hfile = OpenFile(pszFilepath, &ofs, OF_READ);
	if (hfile != HFILE_ERROR)
	{
		CloseHandle((HANDLE)(hfile));
		return true;
	}

	//File not found.  Check variant with last char changed.
	MutateFileName(pszFilepath);
	
	//Check again whether file exists.
	hfile = OpenFile(pszFilepath, &ofs, OF_READ);
	if (hfile != HFILE_ERROR)
	{
		CloseHandle((HANDLE)(hfile));
		return true;
	}

	//Didn't find file.
	return false;
}

//******************************************************************************
bool CFiles::ProtectFile(
//Encodes a file.
//
//Params:
	const char *pszFilepath)	//(in)	Full path to file to be read.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = false;
	const UINT wNumBytesToEncode = 10;
	CStretchyBuffer buffer;

	if (ReadFileIntoBuffer(pszFilepath,buffer))
	{
		EncodeBuffer(buffer);
		
		//Get name of destination file.
		string destFilePath = pszFilepath;
		MutateFileName(destFilePath.begin());

		//Write file to disk.
		OFSTRUCT ofs;
		HFILE hfile = OpenFile(destFilePath.begin(), &ofs, OF_CREATE | OF_WRITE);
		if (hfile != HFILE_ERROR)
		{
			DWORD dwWriteCount;
			BYTE *pBuffer = (BYTE*)buffer;
			if (WriteFile((HANDLE)hfile, pBuffer, buffer.Size(), &dwWriteCount, NULL))
				bSuccess = true;
			CloseHandle((HANDLE)hfile);
		}
	}

	return bSuccess;
}

//
//Private methods.
//

//******************************************************************************
void CFiles::InitClass(void)
//Sets up static members of class.
{
	//Get full path to executable.
	ASSERT(!this->pszAppPath);
	this->pszAppPath = new char[MAX_PATH + 1];
	GetModuleFileName(NULL, this->pszAppPath, MAX_PATH);
	
	//Truncate app path to just the path portion.
	char *pszLastBackslash = this->pszAppPath;
	while (*pszLastBackslash != '\0') ++pszLastBackslash; //Search to end.
	while (pszLastBackslash >= this->pszAppPath && *pszLastBackslash != '\\')
		--pszLastBackslash; //Search to last backslash.
	*pszLastBackslash = '\0'; //Truncates the string.

	//Get dat path from DataPath.txt.
	char *pszDatPathTxt = new char[MAX_PATH + 1];
	ASSERT(!this->pszDatPath);
	this->pszDatPath = new char[MAX_PATH + 1];
	sprintf(pszDatPathTxt, "%s\\DataPath.txt", this->pszAppPath);
	OFSTRUCT ofs;
	HFILE hfile = OpenFile(pszDatPathTxt, &ofs, OF_READ);
	if (hfile == HFILE_ERROR) 
	{
		TryToFixDataPath();
	}
	else
	{
		DWORD dwPathSize = GetFileSize((HANDLE)(hfile), NULL);
		if (dwPathSize == 0 || dwPathSize > MAX_PATH)
		{
			TryToFixDataPath();
		}
		else
		{
			DWORD dwReadCount;
			if (ReadFile((HANDLE)(hfile), this->pszDatPath, dwPathSize, 
					&dwReadCount, NULL))
				this->pszDatPath[dwPathSize] = '\0';
			else
			{
				//Error--use app path so at least error logging may work later.
				strcpy(this->pszDatPath, this->pszAppPath);
			}
		}
	}
	
	delete[] pszDatPathTxt;
	if (hfile != HFILE_ERROR) CloseHandle((HANDLE)(hfile));
}

//******************************************************************************
void CFiles::DeinitClass(void)
//Frees and zeroes static members of class.
{
	ASSERT(this->dwRefCount==0);

	delete[] this->pszAppPath;
	this->pszAppPath=NULL;
	
	delete[] this->pszDatPath;
	this->pszDatPath=NULL;
}

//******************************************************************************
bool CFiles::FindPossibleDatPath(
//Finds a directory that could be a data directory because it contains 
//"drod1_5.dat".
//
//Params:
	const char *pszStartPath,	//(in)	Directory to begin looking at.  Will
								//		go up levels from this dir.  Expecting
								//		no "\" at end.
	char *pszPossibleDatPath)	//(out)	Prealloced to _MAX_PATH + 1.
//
//Returns:
//True if a possible directory was found, false if not.
{
	UINT wStartPathLen = strlen(pszStartPath);

	strcpy(pszPossibleDatPath, pszStartPath);

	char szDRODDatFilepath[_MAX_PATH + 1];
	char *pszEndPath = pszPossibleDatPath + wStartPathLen;

	//Each iteration checks for DROD1_5.DAT in one directory plus up in a "Data"
	//directory.
	while(true)
	{
		sprintf(szDRODDatFilepath, "%s\\drod1_5.dat", pszPossibleDatPath);
		if (DoesFileExist(szDRODDatFilepath)) 
		{
			//Found DROD1_5.dat so dir is possible dat path.
			return true;
		}

		sprintf(szDRODDatFilepath, "%s\\Data\\drod1_5.dat", pszPossibleDatPath);
		if (DoesFileExist(szDRODDatFilepath)) 
		{
			//Found DROD1_5.dat so dir is possible dat path.  Add "data" to end 
			//of possible dat path.
			pszEndPath[0] = '\\';
			pszEndPath[1] = 'D';
			pszEndPath[2] = 'a';
			pszEndPath[3] = 't';
			pszEndPath[4] = 'a';
			pszEndPath[5] = '\0';
			return true; 
		}


		//Change filepath so that it is specifying parent dir.
		while (pszEndPath != pszPossibleDatPath && *pszEndPath != '\\') --pszEndPath;
		*pszEndPath = '\0';
		
		if (pszEndPath == pszPossibleDatPath) 
			return false; //Ran out of parent directories to check.
	}

	//Execution never gets here.
	return false;
}

//******************************************************************************
bool CFiles::WriteDataPathTxt(
//Write a DataPath.txt file.  If one already exists, this will fail.
//
//Params:
	const char *pszFilepath,	//(in)	Complete filepath to write to.
	const char *pszDatPath,		//(in)	Path to write inside file.
	const bool overwrite)		//(in)	Overwrite the bad datapath file.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pszFilepath);
	ASSERT(pszDatPath);

	if (DoesFileExist(pszFilepath) && !overwrite) return false;
	
	OFSTRUCT ofs;
	memset(&ofs, 0, sizeof(ofs));
	HFILE hfile = OpenFile(pszFilepath, &ofs, OF_CREATE | OF_WRITE);
	if (hfile == HFILE_ERROR) return false;

	bool bSuccess = (_lwrite(hfile, pszDatPath, strlen(pszDatPath)) != 0);
	
	_lclose(hfile);

	return bSuccess;
}

// $Log: Files.cpp,v $
// Revision 1.2  2003/05/19 21:12:14  mrimer
// Some code maintenance.
//
// Revision 1.1  2003/02/25 00:01:33  erikh2000
// Initial check-in.
//
// Revision 1.18  2002/11/15 01:34:37  mrimer
// Made some parameters const.
//
// Revision 1.17  2002/09/24 21:14:28  mrimer
// Declared general file operations as static.
//
// Revision 1.16  2002/08/30 00:24:56  erikh2000
// Made small change to get rid of compiler warning.
//
// Revision 1.15  2002/08/29 09:17:50  erikh2000
// Added method to write a buffer to a file.
//
// Revision 1.14  2002/08/23 23:24:20  erikh2000
// Changed some params.
//
// Revision 1.13  2002/07/20 23:01:55  erikh2000
// Revised #includes.
//
// Revision 1.12  2002/07/18 20:15:42  mrimer
// Added GetTrueDatafileName() and MutateFileName().  Revised ProtectFile().
//
// Revision 1.11  2002/07/17 22:37:54  mrimer
// Finished file encryption code.
//
// Revision 1.10  2002/07/02 23:53:31  mrimer
// Added file encryption/unencryption routines.
//
// Revision 1.9  2002/06/21 04:35:09  mrimer
// Revised includes.
//
// Revision 1.8  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.7  2002/04/28 23:51:11  erikh2000
// Added new method to check for read/write access on a file.
//
// Revision 1.6  2002/04/11 10:08:53  erikh2000
// Wrote CFiles::ReadFileIntoBuffer().
//
// Revision 1.5  2002/03/14 03:51:18  erikh2000
// Added TryToFixDataPath() and changed InitClass() to make calls to it.  (Committed on behalf of mrimer.)
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/12/16 02:13:19  erikh2000
// Fixed error in FindPossibleDatPath().
//
// Revision 1.2  2001/10/13 02:37:53  erikh2000
// If DataPath.txt is not present, DRODLib will now search for a possible data path to use and write this path to DataPath.txt.
//
// Revision 1.1.1.1  2001/10/01 22:20:15  erikh2000
// Initial check-in.
//
