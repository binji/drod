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

#ifdef WIN32
#	include <windows.h> //Should be first include.
#endif

#include "Files.h"
#include "Assert.h"
#include "IniFile.h"
#include "Date.h"

#include <string.h>
#include <stdio.h>
#include <mk4.h>
#ifdef WIN32
#	include <io.h>
#endif
#ifdef __POSIX__
#  include <unistd.h>
#  include <dirent.h>
#  include <fcntl.h>
#  include <limits.h>

#ifdef __native_client__
#define SSIZE_MAX LONG_MAX
#endif
#endif

#include <string>
using std::string;

#ifndef WIN32
#include <errno.h>
#endif

#include <sys/stat.h>

//Affects what is logged in AppendErrorLog().
string  m_strUnwrittenContext;
UINT    m_wIndentLevel = 0;

char *lpszStartupPath=NULL;

//Initialize static members of class.
WCHAR * CFiles::wszAppPath = NULL;
WCHAR * CFiles::wszDatPath = NULL;
WCHAR * CFiles::wszResPath = NULL;
WCHAR * CFiles::wszGameName = NULL;
WCHAR * CFiles::wszGameVer = NULL;
DWORD CFiles::dwRefCount = 0;
CIniFile CFiles::gameIni;
bool CFiles::bInitIni = false;
#ifdef __POSIX__
WSTRING CFiles::wstrHomePath;
#endif

//Some constants
static const WCHAR wszDat[] = {W_t('.'),W_t('d'),W_t('a'),W_t('t'),W_t(0)};
static const WCHAR wszData[] = {W_t('D'),W_t('a'),W_t('t'),W_t('a'),W_t(0)};
static const WCHAR wszDataPathDotTxt[] = {W_t('D'),W_t('a'),W_t('t'),W_t('a'),W_t('P'),W_t('a'),W_t('t'),W_t('h'),W_t('.'),W_t('t'),W_t('x'),W_t('t'),W_t(0)};
static const WCHAR wszUniqueResFile[] = {
	W_t('B'),W_t('i'),W_t('t'),W_t('m'),W_t('a'),W_t('p'),W_t('s'),W_t(SLASH),
	W_t('H'),W_t('a'),W_t('l'),W_t('p'),W_t('h'),W_t('H'),W_t('o'),W_t('p'),W_t('e'),W_t('f'),W_t('u'),W_t('l'),
	W_t('.'),W_t('b'),W_t('m'),W_t('_'),W_t(0)};

#ifdef __POSIX__
//Constants used in default path construction/search
static const char pszDatEnvVar[] = "DROD_1_6_DAT_PATH";
static const char pszResEnvVar[] = "DROD_1_6_RES_PATH";
static const WCHAR wszHomeConfDir[] = {W_t('.'),W_t('c'),W_t('a'),W_t('r'),W_t('a'),W_t('v'),W_t('e'),W_t('l'),W_t(0)};
static const WCHAR wszTempPath[] = {W_t(SLASH),W_t('t'),W_t('m'),W_t('p'),W_t(0)};

static const WCHAR wszResSearchPath[] = {
   W_t(SLASH),W_t('u'),W_t('s'),W_t('r'),
   W_t('?'),W_t('l'),W_t('o'),W_t('c'),W_t('a'),W_t('l'),
   W_t('?'),W_t('s'),W_t('h'),W_t('a'),W_t('r'),W_t('e'),
   W_t('?'),W_t('g'),W_t('a'),W_t('m'),W_t('e'),W_t('s'),
   W_t('<'),W_t('s'),W_t('h'),W_t('a'),W_t('r'),W_t('e'),W_t(0)};
static const WCHAR wszDatSearchPath[] = {
   W_t(SLASH),W_t('v'),W_t('a'),W_t('r'),
   W_t('?'),W_t('g'),W_t('a'),W_t('m'),W_t('e'),W_t('s'),
   W_t('>'),W_t('l'),W_t('i'),W_t('b'),
   W_t('<'),W_t('g'),W_t('a'),W_t('m'),W_t('e'),W_t('s'),W_t(0)};

//FIXME: Are these filenames stored somewhere else ?
static const char *szDatFiles[] = { "drod1_6.dat", "player.dat", "text.dat",
      "drod.ini", NULL };
#define MAXDATFILELENGTH 16

#endif //#ifdef __POSIX__

//
//Public methods.
//

//******************************************************************************
CFiles::CFiles(
//Constructor.
//
//Params:
	const WCHAR *wszSetAppPath,		//(in)
  	const WCHAR *wszSetGameName,		//(in)
	const WCHAR *wszSetGameVer)		//(in)
{
	if (this->dwRefCount == 0) {
		ASSERT(wszSetAppPath != NULL);
		ASSERT(wszSetGameName != NULL);
		InitClass(wszSetAppPath, wszSetGameName, wszSetGameVer);
	}
	++this->dwRefCount;
}

//******************************************************************************
CFiles::~CFiles()
//Destructor.
{
	--this->dwRefCount;
	if (this->dwRefCount == 0) DeinitClass();
}

//******************************************************************************
//******************************************************************************
#ifdef __POSIX__
//******************************************************************************
bool CFiles::CreatePathIfInvalid (
//Creates path and all required parent directories (unless they already exist).
//
//Params:
   const WCHAR *wszPath)   //(in)  The path that should be created.
//
//Returns: True if successful.
{
   const UINT l = WCSlen(wszPath);
   char path[l+1];
   UnicodeToAscii(wszPath, path);

   struct stat st;
   bool bOk;
   UINT i = 0;

   do {
      for (++i; i < l && path[i] != SLASH; ++i);
      if (path[i] == SLASH) path[i] = 0;
      bOk = ((access(path, R_OK | X_OK) || stat(path, &st))
            ? !mkdir(path, 0755) : S_ISDIR(st.st_mode));
   } while (bOk && i < l && (path[i] = SLASH));

   return bOk;
}

//******************************************************************************
#define UWP_BUFFER 256
#define UWP_BUFPAD 256
#undef USE_GET_CURRENT_DIR_NAME
static bool UnwrinkleEnvPath (
//Get the environment variable env, transform it into an absolute path without
//any relative bits, and put it in the passed WSTRING.
   const char *env,
   WSTRING &wstr)
//
//Returns:
//True if successful.
{
   char *path = getenv(env), *newpath, prevchar;
	if (!path) return false;

	UINT index = 0, dotlevel = 0;
	bool newelement = true;

	if (*path != SLASH) {
		char *tmp;
#ifdef USE_GET_CURRENT_DIR_NAME
		if (!(newpath = get_current_dir_name()))
			return false;
#else
		if (!(newpath = (char*)malloc(UWP_BUFFER)))
			return false;
		for (index = UWP_BUFFER; getcwd(newpath, index) == NULL; newpath = tmp)
		{
			if ((errno != ERANGE) ||
				(!(tmp = (char*)realloc(newpath, index += UWP_BUFPAD))))
			{
				free(newpath);
				return false;
			}
		}
#endif
		index = strlen(newpath);
		tmp = (char*)realloc(newpath, index + strlen(path) + 2);
		if (!tmp)
		{
			free(newpath);
			return false;
		}
		newpath = tmp;
		newpath[index++] = SLASH;
	}
	else
	{
		newpath = (char*)malloc(strlen(path) + 1);
		if (!newpath)
			return false;
	}

	for (prevchar = 0; (newpath[index] = *path);
		++index, prevchar = *(path++))
	{
		if (newelement)
		{
			if (*path == SLASH)
			{
				if (prevchar == SLASH)
					--index;
				else
				{
					index -= dotlevel;
					while (dotlevel--)
						while (index && newpath[--index] != SLASH);
					dotlevel = 0;
				}
			}
			else if ((*path == '.' && ++dotlevel > 2) || *path != '.')
				newelement = false;
			continue;
		}
		if (*path == SLASH)
		{
			newelement = true;
			dotlevel = 0;
		}
	}
	if (newelement)
	{
		while (dotlevel--)
			while (index && newpath[--index] != SLASH);
		if (!index) ++index;
	}

	if (index > 1 && newpath[index - 1] == SLASH)
		--index;
	newpath[index] = 0;

   AsciiToUnicode(newpath, wstr);
   free(newpath);
   return true;
}

#endif // ifdef __POSIX__

//******************************************************************************
void CFiles::TryToFixDataPath()
//Error-- try to find the Data dir myself and create a new DataPath.txt.
{
	WSTRING wstrDatPathTxt;

#ifdef __POSIX__
	// Beware the gotos ..

   WSTRING wstrDatFile = wszGameName;
	wstrDatFile += wszGameVer;
	wstrDatFile += wszDat;

   const WCHAR *wszHome = wstrHomePath.empty() ? NULL : wstrHomePath.c_str();

   if (!FindPossibleDatFile(wstrDatFile.c_str(), wszDatPath) ||
			!FindPossibleResFile(wszUniqueResFile, wszResPath))
		goto TTFDP_error;

	if (wszHome)
	{
      wstrDatPathTxt = wszHome;
		wstrDatPathTxt += wszSlash;
		wstrDatPathTxt += wszHomeConfDir;
		wstrDatPathTxt += wszSlash;
		wstrDatPathTxt += wszGameName;
      wstrDatPathTxt += wszHyphen;
      wstrDatPathTxt += wszGameVer;
		if (!CreatePathIfInvalid(wstrDatPathTxt.c_str())) goto TTFDP_usedat;
	} else {
TTFDP_usedat:
		wszHome = NULL;
		wstrDatPathTxt = wszDatPath; // gah
	}

	wstrDatPathTxt += wszSlash;
	wstrDatPathTxt += wszDataPathDotTxt;

	if (!WriteDataPathTxt(wstrDatPathTxt.c_str(), wszDatPath, wszResPath, true))
	{
		if (wszHome) goto TTFDP_usedat; // Try again without home

TTFDP_error:
		//Try temporary path first (app path may be read-only)
		if (IsValidPath(wszTempPath))
		{
			WCScpy(wszDatPath, wszTempPath);
			WCScpy(wszResPath, wszTempPath);
		}
		else
#else
	wstrDatPathTxt = wszAppPath;
	wstrDatPathTxt += wszSlash;
   wstrDatPathTxt += wszDataPathDotTxt;

	if (!FindPossibleDatPath(wszAppPath, wszDatPath) ||
			!FindPossibleResPath(wszAppPath, wszResPath) ||
			!WriteDataPathTxt(wstrDatPathTxt.c_str(), wszDatPath, wszResPath, true))
	{
#endif
		{ // <- for else in above ifdef

			//Use app path so at least error logging may work later.
			WCScpy(wszDatPath, wszAppPath);
			WCScpy(wszResPath, wszAppPath);
		}
	}
}

//*****************************************************************************
bool CFiles::FileCopy(const WCHAR *pwzSourceFilepath, const WCHAR *pwzDestFilepath)
{
    ASSERT(pwzSourceFilepath && pWCv(pwzSourceFilepath));
    ASSERT(pwzDestFilepath && pWCv(pwzDestFilepath));
    CStretchyBuffer FileBuf;
    if (!ReadFileIntoBuffer(pwzSourceFilepath, FileBuf)) return false;
#ifdef __POSIX__
    // Get stats from source file, and set the destination file mode to be identical.
    struct stat st;
    WSTRING wstrSource = pwzSourceFilepath;
    char pszSource[wstrSource.length() + 1];
    UnicodeToAscii(wstrSource, pszSource);
    stat(pszSource, &st); // The above read succeeded, so assume this succeeds too.
#ifndef __native_client__
    mode_t oldmask = umask(st.st_mode ^ 0777);
#endif
#endif

    //Write the buffer without terminating wchar zero at end.
    bool bSuccess = false;
    FILE *pFile = Open(pwzDestFilepath, "wb");
    if (NULL != pFile)
    {
#ifdef __POSIX__
      // Set the group of this file to the same as its owner.
      fchown(fileno(pFile), (uid_t)-1, st.st_gid);
#endif
      bSuccess = (FileBuf.Size() - 2 == fwrite( (BYTE*)FileBuf, 1, FileBuf.Size() - 2, pFile ));
      fclose( pFile );
    }
#ifdef __linux__
    // Reset file creation mask
    umask(oldmask);
#endif
    return bSuccess;
}

//*****************************************************************************
bool CFiles::DoesFileExist(
//Determines if a specified file exists by trying to open it.
//
//Params:
  const WCHAR *wszFilepath) //(in)
//
//Returns:
//True if file exist, false if not.
{
	FILE* pFile = Open( wszFilepath, "rb" );
	if (NULL == pFile) {
      if (ENOTDIR == errno || ENOENT == errno || EACCES == errno)
         return false;
		return true;
	} else {
		fclose(pFile);
		return true;
	}
}

//*****************************************************************************
FILE * CFiles::Open(
//Unicode-friendly function to open a file.
    const WCHAR *pwzFilepath,   //(in)    Full path to file.
    const char *pszOptions)     //(in)    fopen()-style options.
//Returns:
//Pointer to file or NULL if an error occurred.  Use fclose() to close file later.
{
    FILE *pFile = NULL;

    //Unicode open.
#ifdef HAS_UNICODE
#   ifdef   WIN32
#	    define WFOPEN _wfopen
#   endif

    WSTRING wstrOptions;
    AsciiToUnicode(pszOptions, wstrOptions);
    pFile = WFOPEN( pwzFilepath, wstrOptions.c_str() );

    //If failed, will try again with ASCII conversion in case wfopen() is stubbed on the O/S.
    if (!pFile)
#endif
    //ASCII open.
    {
        char pszPath[MAX_PATH+1];
        UnicodeToAscii( pwzFilepath, pszPath );
        pFile = fopen( pszPath, pszOptions);
    }

    return pFile;
}

//*****************************************************************************
bool CFiles::HasReadWriteAccess(
//Determines if a specified file has read/write access by trying to open it.
//
//Params:
  const WCHAR *wszFilepath) //(in)
//
//Returns:
//True if file exists, false if not.
{
	FILE* pFile = Open( wszFilepath, "rb+" );
	if (pFile == NULL)
		return false;
	fclose( pFile );
	return true;
}

//******************************************************************************
void CFiles::AppendErrorLog(
//Appends text to the error log.
//
//Params:
	const char *pszText) //(in) Text to write at end of file.
{
    static CDate LastLog;
    static bool bFirstLog = true;

    FILE* pFile;
    WSTRING wstrTemp;
    AsciiToUnicode("drod.err", wstrTemp);

    WSTRING wstrDatPathTxt = GetDatPath();
    wstrDatPathTxt += wszSlash;
    wstrDatPathTxt += wstrTemp;

#ifdef __linux__
    // Make errorlog world-writable
    mode_t oldmask = umask(0);
#endif
    pFile = Open(wstrDatPathTxt.c_str(), "a");
    if (pFile)
    {
        //Write any log context that hasn't been written already.
        if (!m_strUnwrittenContext.empty())
        {
            fwrite(m_strUnwrittenContext.c_str(), 1, m_strUnwrittenContext.size(), pFile);
            m_strUnwrittenContext = "";
        }

        string strIndent;
        for (UINT wIndentNo = 0; wIndentNo < m_wIndentLevel; ++wIndentNo) strIndent += "  ";

        //Write time if this is first log or at least a minute has elapsed since last write.
        CDate Now;
        if ((time_t) Now - (time_t) LastLog > 60 || bFirstLog)
        {
            WSTRING wstrTime;
            string strTime;
            Now.GetLocalFormattedText(DF_SHORT_DATE | DF_SHORT_TIME, wstrTime);

            strTime = strIndent;
            strTime += "*** ";
            if (bFirstLog)
            {
                strTime += "FIRST LOG IN SESSION ";
                bFirstLog = false;
            }
            UnicodeToAscii(wstrTime, strTime);
            strTime += " ***\r\n";
            fwrite(strTime.c_str(), 1, strTime.size(), pFile);
        }

        //Write the actual text passed to method.
        fwrite(strIndent.c_str(), 1, strIndent.size(), pFile);
        fwrite(pszText, 1, strlen(pszText), pFile);
        fclose(pFile);
        LastLog = Now;
    }
#ifdef __linux__
    // Restore old file creation mask.
    umask(oldmask);
#endif
}

//******************************************************************************
bool CFiles::WriteGameProfileString(
//Writes a string to <GameName>.INI.
//
//Params:
  const char *pszSection,	//(in)
  const char *pszKey,		//(in)
  const char *pszValue)		//(in)
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = true;
	gameIni.WriteString(pszSection, pszKey, pszValue);

	return bSuccess;
}

//******************************************************************************
bool CFiles::GetGameProfileString(
//Gets a string from <GameName>.INI.
//
//Params:
	const char *pszSection, //(in)
	const char *pszKey,		//(in)
	string &strValue)		//(out)
//
//Returns:
//True if successfully found entry or false if not.
{
	return gameIni.GetString(pszSection, pszKey, "", strValue);
}

//******************************************************************************
bool CFiles::ReadFileIntoBuffer(
//Reads a file into a buffer.  Preferrably only be used with small files.
//
//Params:
	const WCHAR *wszFilepath,	//(in)	Full path to file to be read.
	CStretchyBuffer &Buffer)	//(out)	Receives file data.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = false;
	ASSERT(wszFilepath);
	ASSERT(Buffer.Size() == 0);

    FILE* pFile = Open( wszFilepath, "rb" );
	BYTE charBuf[16385];
	if (NULL != pFile)
	{
		//Get size of file.
		fseek(pFile, 0, SEEK_END);
		if (!Buffer.Alloc(ftell(pFile)))
		{
			fclose(pFile);
			return false;
		}
		fseek(pFile, 0, SEEK_SET);

		int bytes;
		do {
			bytes = fread( charBuf, 1, 16384, pFile );
			if (bytes > 0) {
				Buffer.Append( charBuf, bytes );
			}
		} while (bytes > 0);
		Buffer.Append((const BYTE*)wszEmpty, sizeof(WCHAR));
		fclose( pFile );
		bSuccess = true;
	}
	else
	{
		char buf[MAX_PATH+1];
		UnicodeToAscii(wszFilepath, buf);
		bSuccess = false;
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

	FILE* pFile = fopen( pszFilepath, "wb" );
	if (NULL != pFile)
	{
		bSuccess = (Buffer.Size() == fwrite( (BYTE*)Buffer, 1, Buffer.Size(), pFile ));
		fclose( pFile );
	}

	return bSuccess;
}

//******************************************************************************
bool CFiles::WriteBufferToFile(
//Writes a file from a buffer.  Overwrites an existing file.
//
//Params:
	const WCHAR *pwszFilepath,		//(in)	Full path to file to be written.
	const CStretchyBuffer &Buffer)	//(in)	Data to write.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = false;
	ASSERT(pwszFilepath);
	ASSERT(Buffer.Size() != 0);

	FILE* pFile = Open( pwszFilepath, "wb" );
	if (NULL != pFile)
	{
		bSuccess = (Buffer.Size() ==
				fwrite( (BYTE*)Buffer, sizeof(BYTE), Buffer.Size(), pFile ));
		fclose( pFile );
	}

	return bSuccess;
}

//******************************************************************************
void CFiles::MutateFileName(
//Unprotected and protected datafiles differ by their last letter.
//This routine changes one variant to the other.
//Params:
	WCHAR *wszFilepath)		//(in/out)	Full path to file to be read.
							//Mutates file name by changing last letter.
{
	const UINT nLength = WCSlen(wszFilepath);
	if (FileIsEncrypted(wszFilepath))
	{
		switch (WCv(wszFilepath[nLength-2]))
		{
		case 'm':
			WCv(wszFilepath[nLength-1]) = 'p';
			break;
		case 'a':
			WCv(wszFilepath[nLength-1]) = 'v';
			break;
		case '3':
			WCv(wszFilepath[nLength-1]) = 'm';
			break;
		default:	//the general ".dat"
			WCv(wszFilepath[nLength-1]) = 't';
			break;
		}
	} else {
		WCv(wszFilepath[nLength-1]) = '_';
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
	WCHAR *wszFilepath)	//(in/out)	Full path to file to be read.
										//Filename is mutated (by changing last letter)
										//if file of original name is not found.
{
	//If file exists, return true.
	FILE* pFile = Open( wszFilepath, "r" );
	if (NULL != pFile)
	{
		fclose(pFile);
		return true;
	}

	//File not found.  Check variant with last char changed.
	MutateFileName(wszFilepath);

	pFile = Open( wszFilepath, "r" );
	if (NULL != pFile)
	{
		fclose(pFile);
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
	const WCHAR *pszFilepath)	//(in)	Full path to file to be read.
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = false;
	CStretchyBuffer buffer;

	if (ReadFileIntoBuffer(pszFilepath,buffer))
	{
		buffer.Encode();

		//Get name of destination file.
		WSTRING destFilePath = pszFilepath;
		MutateFileName(&*destFilePath.begin());

		FILE* pFile = Open( destFilePath.c_str(), "wb" );
		if (NULL != pFile)
		{
			if (buffer.Size() == fwrite( (BYTE*)buffer, 1, buffer.Size(), pFile ))
				bSuccess = true;
			fclose( pFile );
		}
	}

	return bSuccess;
}

//
//Private methods.
//

//******************************************************************************
void CFiles::InitClass(
//Sets up static members of class.
//
//Params:
	const WCHAR *wszSetAppPath,	//(in)
  	const WCHAR *wszSetGameName,		//(in)
	const WCHAR *wszSetGameVer)		//(in)
{
	//Get full path to executable.
	ASSERT(!this->wszAppPath);
	this->wszAppPath = new WCHAR[WCSlen(wszSetAppPath) + 1];
	WCScpy(wszAppPath, wszSetAppPath);

	//Get game name and version.
	ASSERT(!this->wszGameName);
	this->wszGameName = new WCHAR[WCSlen(wszSetGameName) + 1];
	WCScpy(wszGameName, wszSetGameName);
	ASSERT(!this->wszGameVer);
	this->wszGameVer = new WCHAR[WCSlen(wszSetGameVer) + 1];
	WCScpy(wszGameVer, wszSetGameVer);

	//Truncate app path to just the path portion.
	WCHAR *wszLastBackslash = this->wszAppPath;
	while (*wszLastBackslash != '\0') ++wszLastBackslash; //Search to end.
	while (wszLastBackslash >= this->wszAppPath && *wszLastBackslash != SLASH)
		--wszLastBackslash; //Search to last backslash.
	pWCv(wszLastBackslash) = '\0'; //Truncates the string.

#ifdef __POSIX__
   //Get $HOME and create conf dir, so we don't have to later.
   UnwrinkleEnvPath("HOME", wstrHomePath);
   CreatePathIfInvalid((wstrHomePath + wszSlash + wszHomeConfDir + wszSlash
         + wszGameName + wszHyphen + wszGameVer).c_str());
#endif

	//Get dat path from DataPath.txt.
	WSTRING wstrDatPathTxt;
	ASSERT(!this->wszDatPath);
	ASSERT(!this->wszResPath);
	this->wszDatPath = new WCHAR[MAX_PATH + 1];
	this->wszResPath = new WCHAR[MAX_PATH + 1];
#ifdef __POSIX__
	if (!FindPossibleDatFile(wszDataPathDotTxt, this->wszDatPath))
	{
		TryToFixDataPath();
	}
	wstrDatPathTxt = this->wszDatPath;
#else
	wstrDatPathTxt = this->wszAppPath;
#endif
	wstrDatPathTxt += wszSlash;
	wstrDatPathTxt += wszDataPathDotTxt;
	FILE* pFile = Open(wstrDatPathTxt.c_str(), "r");
	if (NULL == pFile) {
		TryToFixDataPath();
	}
	else {
#ifdef HAS_UNICODE
		WCHAR buffer[2 * (MAX_PATH + 1)];
		fgetWs(buffer, 2 * (MAX_PATH + 1), pFile);	// line 1: dat path
		UINT i;
		for (i = 0; i < MAX_PATH && buffer[i] != ';'; ++i) ;
		if (buffer[i] != ';' || !buffer[0] || !buffer[i + 1] || WCSlen(buffer + i + 1) > MAX_PATH)
		{
			TryToFixDataPath();
		}
		else
		{
         UINT j;
			buffer[i] = 0;
         for (j = i + 1; buffer[j] && buffer[j] != ';'; ++j) ;
         buffer[j] = 0;
			WCScpy(this->wszDatPath, buffer);
			WCScpy(this->wszResPath, buffer + i + 1);
		}
#else
		char buffer[2 * (MAX_PATH + 1)];
		fgets(buffer, 2 * (MAX_PATH + 1), pFile);
		UINT i;
		for (i = 0; i < MAX_PATH && buffer[i] != ';'; ++i) ;
		if (buffer[i] != ';' || !buffer[0] || !buffer[i + 1] || strlen(buffer + i + 1) > MAX_PATH)
		{
			TryToFixDataPath();
		}
		else
		{
			WSTRING wstrTemp;
         UINT j;
			buffer[i] = 0;
         for (j = i + 1; buffer[j] && buffer[j] != ';'; ++j) ;
         buffer[j] = 0;
			AsciiToUnicode(buffer, wstrTemp);
			WCScpy(this->wszDatPath, wstrTemp.c_str());
			AsciiToUnicode(buffer + i + 1, wstrTemp);
			WCScpy(this->wszResPath, wstrTemp.c_str());
		}
#endif
		fclose( pFile );
	}

	if (!bInitIni)
	{
		WSTRING wstrIniPath = this->wszDatPath;
		wstrIniPath += wszSlash;
		WSTRING wstrTmp;
		AsciiToUnicode(".ini", wstrTmp);
		wstrIniPath += wszGameName;
		wstrIniPath += wstrTmp;

		bInitIni = true;
		gameIni.Load(wstrIniPath.c_str());
	}

}

//******************************************************************************
void CFiles::DeinitClass()
//Frees and zeroes static members of class.
{
	ASSERT(this->dwRefCount==0);

	delete[] this->wszAppPath;
	this->wszAppPath=NULL;

	delete[] this->wszDatPath;
	this->wszDatPath=NULL;

	delete[] this->wszResPath;
	this->wszResPath=NULL;

	delete[] this->wszGameName;
	this->wszGameName=NULL;

	delete[] this->wszGameVer;
	this->wszGameVer=NULL;
}

//******************************************************************************
#ifdef __POSIX__
//******************************************************************************
bool CFiles::CheckDataAndMakeWritable (
//Check that all the dat-files (and the ini) are present and writable.  If any
//of them are present but not writable, make local writable copies of all files
//in the user's home directory.  If the copy was required and successful,
//wstrPath will be set to the new path of the data.
//
//Params:
   WCHAR *wszPath)   //(in/out)   Path where data is, allocated to MAX_PATH + 1
//
//Returns:
//True if the data is present and writable (regardless if it was copied or not),
//false otherwise.
{
   const WSTRING wstrDest = wstrHomePath + wszSlash + wszHomeConfDir + wszSlash
         + wszGameName + wszHyphen + wszGameVer;
   const unsigned int si = WCSlen(wszPath) + 1, ti = wstrDest.length() + 1;
   char source[si + MAXDATFILELENGTH + 1], target[ti + MAXDATFILELENGTH + 1];

   UnicodeToAscii(wszPath, source);
   UnicodeToAscii(wstrDest, target);
   target[ti - 1] = source[si - 1] = SLASH;

   bool bSamePath = (si == ti && !strncmp(source, target, si));
   int i;

   //First check if all data is writable
   for (i = 0; szDatFiles[i]; ++i) {
      strcpy(source + si, szDatFiles[i]);
      if (access(source, R_OK | W_OK)) {
         if (!bSamePath) break;
         else {
            chmod(source, 0644);  // same path, so we chmod in stead of copy
            if (access(source, R_OK | W_OK)) return false;
         }
      }
   }
   if (!szDatFiles[i]) return true;

   //Some data are read-only (or missing), so we need to make copies (or fail).

   if (wstrHomePath.empty()) return false; //Can't copy if there's no home dir ..

   char buffer[65536 < SSIZE_MAX ? 65536 : SSIZE_MAX]; //read can only handle <= SSIZE_MAX
   for (i = 0; szDatFiles[i]; ++i)
   {
      int sf, tf;
      strcpy(target + ti, szDatFiles[i]);
      strcpy(source + si, szDatFiles[i]);
      if ((sf = open(source, O_RDONLY)) < 0) break;
#ifndef __native_client__
      if ((tf = creat(target, 0644)) < 0) {
         close(sf);
         break;
      }
#endif
      ssize_t l;
      while ((l = read(sf, buffer, 65536 < SSIZE_MAX ? 65536 : SSIZE_MAX))) {
         if (l == (ssize_t)-1) {
            if (errno == EINTR) continue; //read interrupted; try again
            break;
         }
         write (tf, buffer, l);
      }
      close(sf);
      close(tf);
      if (l == (ssize_t)-1) break;
   }

   //If error, unlink copied files
   if (szDatFiles[i]) {
      while (i >= 0) {
         strcpy(target + ti, szDatFiles[i--]);
         unlink(target);
      }
      return false;
   }

   //Success!  Update path, and we're done.
   WCSncpy(wszPath, wstrDest.c_str(), MAX_PATH);
   return true;
}

//******************************************************************************
bool CFiles::CheckForDataFile(
//Check if a file exists in the specified path, and also check the subdirs
//Data (if bTryClean is true), <GameName-GameVer>, and <GameName-GameVer>/Data.
//If the file exists and bData is set, CheckDataAndMakeWritable is called to
//verify that the rest of the data is there and copy the data to a writable
//location if required.
//
//Params:
   WCHAR *wszPath,            // (in/out) Path to file
   const WCHAR *wszFmt,       // (in)     Pathsearch setup string
   const WCHAR *wszFile,      // (in)     Filename (no path)
   const bool bTryClean,      // (in)     Try without adding game name/ver first
   const bool bData)          // (in)     Look for data (req writable or copy)
//
//Returns: true if all data was found and is writable, with wszPath set to the
//path (without filename)
{
   WSTRING wstr = wszFmt;
   wstr += bTryClean ? wszQuestionMark : wszSlash;
   wstr += wszGameName;
   wstr += wszHyphen;
   wstr += wszGameVer;
   wstr += wszQuestionMark;
   wstr += wszData;
   wstr += wszSemicolon; // Break off here
   wstr += wszSlash;
   wstr += wszFile;

   int brkpos;
   CPathGen pg(wszPath, wstr.c_str());
   while ((brkpos = pg.Next())) if (DoesFileExist(wszPath)) {
      WCv(wszPath[brkpos - 1]) = 0;
      if (!bData || CheckDataAndMakeWritable(wszPath))
         return true;
   }
   return false;
}

//******************************************************************************
bool CFiles::FindPossibleDatFile(
//linux/unix specific: Search for a file in some (possibly) writable paths.
//Construct a range of paths from compiled-in defaults (FHS is our friend),
//and see if the specified file exists in one of these.
//
//Params:
   const WCHAR *wszFile,   //(in)   Filename to search for.
   WCHAR *wszPathBuf)      //(out)  Valid path. Preallocate to MAX_PATH + 1
{
   WSTRING wstrTmp;

   // Try home first.
   if (!wstrHomePath.empty())
   {
      wstrTmp = wstrHomePath;
      wstrTmp += wszSlash;
      wstrTmp += wszHomeConfDir;
      if (CheckForDataFile(wszPathBuf, wstrTmp.c_str(), wszFile, false, true)) return true;
   }

   // Then try path in environment variable, app path and the dat search path.
   if ((UnwrinkleEnvPath(pszDatEnvVar, wstrTmp)
         && CheckForDataFile(wszPathBuf, wstrTmp.c_str(), wszFile, true, true))
         || CheckForDataFile(wszPathBuf, wszAppPath, wszFile, true, true)
         || CheckForDataFile(wszPathBuf, wszDatSearchPath, wszFile, false, true)
         || CheckForDataFile(wszPathBuf, wszResSearchPath, wszFile, false, true))
      return true;

   return false;
}

//******************************************************************************
bool CFiles::FindPossibleResFile(
//As above, but for resource (possibly readonly) files. FHS is still our friend.
//
//Params:
   const WCHAR *wszFile,   //(in)   Filename to search for.
   WCHAR *wszPathBuf)      //(out)  Valid path. Preallocate to MAX_PATH + 1
{
   // Try path in environment variable, then app path and the res search path.
   WSTRING wstrTmp;
   if ((UnwrinkleEnvPath(pszResEnvVar, wstrTmp))
         && CheckForDataFile(wszPathBuf, wstrTmp.c_str(), wszFile, true, false)
         || CheckForDataFile(wszPathBuf, wszAppPath, wszFile, true, false)
         || CheckForDataFile(wszPathBuf, wszResSearchPath, wszFile, false, false))
      return true;

   return false;
}

//******************************************************************************
#else // ifdef __POSIX__
//******************************************************************************
bool CFiles::FindPossibleDatPath(
//Finds a directory that could be a data directory because it contains
//"drod1_5.dat".
//
//Params:
	const WCHAR *wszStartPath,	//(in)	Directory to begin looking at.  Will
								//		go up levels from this dir.  Expecting
								//		no "\" at end.
	WCHAR *wszPossibleDatPath)	//(out)	Prealloced to _MAX_PATH + 1.
//
//Returns:
//True if a possible directory was found, false if not.
{
	const UINT wStartPathLen = WCSlen(wszStartPath);

	WCScpy(wszPossibleDatPath, wszStartPath);

	WSTRING wstrGameDatFilepath;
	WCHAR *pwszEndPath = wszPossibleDatPath + wStartPathLen;

	//Each iteration checks for "<GameName><GameVer>.DAT" in one directory plus up in a "data"
	//directory.
	while (true)
	{
		wstrGameDatFilepath = wszPossibleDatPath;   // "x:/somepath"
		wstrGameDatFilepath += wszSlash;            // "x:/somepath/"
		wstrGameDatFilepath += wszGameName;         // "x:/somepath/drod"
		wstrGameDatFilepath += wszGameVer;          // "x:/somepath/drod1_6"
		wstrGameDatFilepath += wszDat;              // "x:/somepath/drod1_6.dat"
		if (DoesFileExist(wstrGameDatFilepath.c_str()))
		{
			//Found DROD1_6.dat so dir is possible dat path.
			return true;
		}

		wstrGameDatFilepath = wszPossibleDatPath;   // "x:/somepath"
		wstrGameDatFilepath += wszSlash;            // "x:/somepath/"
		wstrGameDatFilepath += wszData;             // "x:/somepath/data"
		wstrGameDatFilepath += wszSlash;            // "x:/somepath/data/"
		wstrGameDatFilepath += wszGameName;	        // "x:/somepath/data/drod"
		wstrGameDatFilepath += wszGameVer;          // "x:/somepath/data/drod1_6"
		wstrGameDatFilepath += wszDat;              // "x:/somepath/data/drod1_6.dat"
		if (DoesFileExist(wstrGameDatFilepath.c_str()))
		{
			//Found DROD1_6.dat so dir is possible dat path.  Add "data" to end
			//of possible dat path.
         WCv(pwszEndPath[0]) = SLASH;
			WCv(pwszEndPath[1]) = 'D';
			WCv(pwszEndPath[2]) = 'a';
			WCv(pwszEndPath[3]) = 't';
			WCv(pwszEndPath[4]) = 'a';
			WCv(pwszEndPath[5]) = '\0';
			return true;
		}

		//Change file path so that it is specifying parent dir.
		while (pwszEndPath != wszPossibleDatPath && *pwszEndPath != SLASH)
         --pwszEndPath;
		pWCv(pwszEndPath) = '\0';

		if (pwszEndPath == wszPossibleDatPath)
			return false; //Ran out of parent directories to check.
	}

	//Execution never gets here.
	return false;
}

//******************************************************************************
bool CFiles::FindPossibleResPath(
//Finds a directory that could be a resource directory because it contains
//"Bitmaps", "Fonts", "Help", "Music" and "Sounds".
//
//Params:
	const WCHAR *wszStartPath,	//(in)	Directory to begin looking at.  Will
								//		go up levels from this dir.  Expecting
								//		no slash at end.
	WCHAR *wszPossibleResPath)	//(out)	Prealloced to MAX_PATH + 1.
//
//Returns:
//True if a possible directory was found, false if not.
{
	const UINT wStartPathLen = WCSlen(wszStartPath);

	WCScpy(wszPossibleResPath, wszStartPath);

	WSTRING wstrGameRespath;

	WCHAR *pwszEndPath = wszPossibleResPath + wStartPathLen;

	//Each iteration checks for the resource dirs in the dir and parent dir
	while (true)
	{
		wstrGameRespath = wszPossibleResPath;   // "x:/somepath"
		wstrGameRespath += wszSlash;            // "x:/somepath/"
		wstrGameRespath += wszUniqueResFile;    // "x:/somepath/<res>"
		if (IsValidPath(wstrGameRespath.c_str()))
			return true;
		wstrGameRespath = wszPossibleResPath;   // "x:/somepath"
		wstrGameRespath += wszSlash;            // "x:/somepath/"
		wstrGameRespath += wszData;             // "x:/somepath/Data"
		wstrGameRespath += wszSlash;            // "x:/somepath/Data/"
		wstrGameRespath += wszUniqueResFile;    // "x:/somepath/Data/<res>"
		if (IsValidPath(wstrGameRespath.c_str()))
		{
			// Add 'Data' to the end of the path
			WCv(pwszEndPath[0]) = SLASH;
			WCv(pwszEndPath[1]) = 'D';
			WCv(pwszEndPath[2]) = 'a';
			WCv(pwszEndPath[3]) = 't';
			WCv(pwszEndPath[4]) = 'a';
			WCv(pwszEndPath[5]) = '\0';
			return true;
		}

		//Change file path so that it is specifying parent dir.
		while (pwszEndPath != wszPossibleResPath && *pwszEndPath != SLASH)
			--pwszEndPath;
		pWCv(pwszEndPath) = '\0';

		if (pwszEndPath == wszPossibleResPath)
			return false; //Ran out of parent directories to check.
	}

	//Execution never gets here.
	return false;
}

//******************************************************************************
#endif // ifdef __POSIX__, else

//******************************************************************************
bool CFiles::WriteDataPathTxt(
//Write a DataPath.txt file.  If one already exists, this will fail unless
//overwrite is true.
//
//Params:
	const WCHAR *pwszFilepath,	//(in)	Complete filepath to write to.
	const WCHAR *pwszDatPath,	//(in)	Data path to write inside file.
	const WCHAR *pwszResPath,	//(in)	Resource path to write inside file.
	const bool overwrite)		//(in)	Overwrite the bad datapath file.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pwszFilepath);
	ASSERT(pwszDatPath);
	ASSERT(pwszResPath);

	if (DoesFileExist(pwszFilepath) && !overwrite) return false;

	FILE* pFile = Open(pwszFilepath, "w");
	if (NULL == pFile) return false;

	WSTRING tmp = pwszDatPath;
	tmp += wszSemicolon;
	tmp += pwszResPath;
	tmp += wszSemicolon;
#ifdef HAS_UNICODE
	bool bSuccess = (tmp.length() == fwrite( tmp.c_str(), sizeof(WCHAR), tmp.length(), pFile ));
#else
	char buffer[tmp.length() + 1];
	UnicodeToAscii(tmp, buffer);
	bool bSuccess = (strlen(buffer) == fwrite( buffer, sizeof(char), strlen(buffer), pFile ));
#endif // HAS_UNICODE

	fclose(pFile);

	return bSuccess;
}

//*****************************************************************************
bool CFiles::WindowsCanBrowseUnicode()
// Returns true if the Windows OS has _wfind* functions (i.e. it's NT or better)
{
    //It might be preferable to make a call to wstat(apppath) instead and see if it works.

#ifdef WIN32
    OSVERSIONINFO versionInfo;
    versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	if (!::GetVersionEx (&versionInfo)) {
      // Can't get version - let's assume we can't browse files in unicode
		return false;
	}

	return (
            //At time of writing (8/4/03) this includes: Windows NT 3.51, Windows NT 4.0,
            //Windows 2000, Windows XP, or Windows .NET Server.
            versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ||

            //Any future O/Ss from MS should probably support unicode too.
            versionInfo.dwMajorVersion >= 5);

#else //This isn't even Windows!
    ASSERTP(false, "Bad call to WindowsCanBrowseUnicode().");
    return false;
#endif
}

//*****************************************************************************
bool CFiles::IsValidPath(const WCHAR *pwzPath)
//Returns true if path is valid, false if not.
{
   if (!(pwzPath && pWCv(pwzPath))) return false;
   WSTRING wstrDirPath = pwzPath;

	//Get rid of the trailing slash (stat doesn't like it) UNLESS there is only one slash - i.e. e:\ .
	if (wstrDirPath.size() > 0) {
		int count = 0;
		for (WSTRING::iterator i = wstrDirPath.begin(); i != wstrDirPath.end(); i++) {
			if (*i == wszSlash[0]) count++;
		}
		if (count > 1) {
			if (wstrDirPath[wstrDirPath.size()-1] == wszSlash[0]) {
				wstrDirPath.erase(wstrDirPath.size()-1);
			}
		}
	}

#ifdef WIN32
    //Check for read permission.
    if (CFiles::WindowsCanBrowseUnicode())
        return (_waccess(wstrDirPath.c_str(), 4) == 0);
    else
    {
        char *aDirPath = new char[wstrDirPath.length()+1];
		UnicodeToAscii(wstrDirPath, aDirPath);
        bool bValidPath = (_access(aDirPath, 4) == 0);
        delete [] aDirPath;
        return bValidPath;
    }
#else
	char *aDirPath = new char[wstrDirPath.length()+1];
	struct stat buf;
	UnicodeToAscii(wstrDirPath, aDirPath);
   bool bIsValid = !access(aDirPath, R_OK | X_OK)
         && (!stat(aDirPath, &buf) ? S_ISDIR(buf.st_mode) : false);
	delete[] aDirPath;
   return bIsValid;
#endif
}

#ifdef USE_LOGCONTEXT
//**************************************************************************************
void CFiles::PushLogContext(const char *pszDesc)
{
    const UINT MAXLEN_CONTEXT = 20000;
    const UINT CHOPLEN = 3000;
    for (UINT wIndentNo = 0; wIndentNo < m_wIndentLevel; ++wIndentNo)
        m_strUnwrittenContext += "  ";
    m_strUnwrittenContext += "BEGIN ";
    m_strUnwrittenContext += pszDesc;
    m_strUnwrittenContext += "\r\n";
    ++m_wIndentLevel;

    if (m_strUnwrittenContext.size() > MAXLEN_CONTEXT)
    {
        const char *pszSeek = m_strUnwrittenContext.c_str() + CHOPLEN;
        while (*pszSeek && *pszSeek != '\n') ++pszSeek;
        string strTemp = pszSeek + 1;
        m_strUnwrittenContext = "(Beginning of log context truncated to free memory.)\r\n";
        m_strUnwrittenContext += strTemp;
    }
}

//**************************************************************************************
void CFiles::PopLogContext(const char *pszDesc)
{
    --m_wIndentLevel;
    for (UINT wIndentNo = 0; wIndentNo < m_wIndentLevel; ++wIndentNo)
        m_strUnwrittenContext += "  ";
    m_strUnwrittenContext += "END ";
    m_strUnwrittenContext += pszDesc;
    m_strUnwrittenContext += "\r\n";
}
#endif // #ifdef USE_LOGCONTEXT

//**************************************************************************************
#ifdef __POSIX__
CPathGen::CPathGen (WCHAR *buffer, const WCHAR *expr)
: wszBuffer(buffer), wszExpr(expr)
{
   UINT i, bits = 0;
   for (i = 0; WCv(expr[i]); ++i)
      if (expr[i] == '?' || expr[i] == '<' || expr[i] == '>')
         ++bits;
   this->wBits = bits;
   this->wN = (1 << bits) - 1;
}

//**************************************************************************************
int CPathGen::Next ()
//Fill buffer with the next path to search.
//
//Returns:
//Nonzero if there was more paths, zero otherwise.  If a seperator was used (;), the
//(one-based) position of the last one is returned, otherwise all you get is -1.
{
   if (this->wN == (UINT)-1) {
      WCv(this->wszBuffer[0]) = 0;
      return 0;
   }
   const UINT n = this->wN--;
   UINT m = 1 << (this->wBits - 1), j = 0;
   bool bProcess = true, bWriting = true;
   int ret = -1;
   WCHAR ch;
   for (UINT i = 0; (WCv(ch) = WCv(this->wszExpr[i])); ++i) {
      if (bProcess) switch (WCv(ch)) {
         case ';':
            ret = j + 1;
            bProcess = false;
            bWriting = true;
            continue;
         case '/':
            bWriting = true;
            break;
         case '?':
         case '<':
         case '>':
            if (((bWriting = n & m) && ch != '?')
                  && (((n & (m << 1))?1:0) ^ (ch == '<')?1:0)) // logical xor =)
               return Next();
            WCv(ch) = '/';
            m >>= 1;
            break;
      }
      if (bWriting) this->wszBuffer[j++] = ch;
   }
   WCv(this->wszBuffer[j]) = 0;
   return ret;
}
#endif //#ifdef __POSIX__

// $Log: Files.cpp,v $
// Revision 1.28  2004/08/09 21:06:01  gjj
// Small Linux fix: Also search res path for data since we allow read-only
// data now (the copy-to-home feature).
//
// Revision 1.27  2004/07/18 14:12:15  gjj
// Wops, there's no data.dat in 1.6
//
// Revision 1.26  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.25  2003/11/09 05:20:54  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.24  2003/10/27 20:21:02  mrimer
// Fixed bug corrupting binary files (stripping chars that can be interpreted as EOF from the end of the file).
//
// Revision 1.23  2003/10/09 14:52:37  mrimer
// Added Unix file permission integrity (committed on behalf of Gerry JJ).
//
// Revision 1.22  2003/10/08 17:32:24  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.21  2003/10/07 21:09:49  erikh2000
// Added context information to logging.
//
// Revision 1.20  2003/10/06 02:37:08  erikh2000
// Added file-copying method.
// Error log appends now show date/time.
//
// Revision 1.19  2003/09/16 16:01:13  schik
// return false (instead of assert) if IsValidPath is passed a NULL or empty string
//
// Revision 1.18  2003/08/27 18:35:37  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.17  2003/08/10 02:23:26  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.16  2003/08/09 00:37:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.15  2003/08/07 18:36:24  erikh2000
// Removed redundant IsPathValid() function.
// Fixed a bug with an unterminated string constant.
//
// Revision 1.14  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.13  2003/08/05 00:09:36  erikh2000
// Added IsValidPath() function.
//
// Revision 1.12  2003/08/01 20:26:50  schik
// Moved WindowsCanBrowseUnicode() to CFiles and made it static.
//
// Revision 1.11  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/06/20 20:44:17  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/06/16 20:15:22  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.8  2003/06/15 04:19:14  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.7  2003/06/03 06:19:48  mrimer
// Fixed a bug in reading Unicode text files into buffer.
//
// Revision 1.6  2003/05/30 01:13:36  mrimer
// Fixed another bug causing the error log to be rewritten rather than appended to.
//
// Revision 1.5  2003/05/30 01:00:28  mrimer
// Fixed a bug causing error log not to be written.
//
// Revision 1.4  2003/05/29 19:13:57  mrimer
// Fixed a bug.
//
// Revision 1.3  2003/05/28 23:02:53  erikh2000
// Made changes to CFiles methods to prevent errors.
//
// Revision 1.2  2003/05/26 01:41:10  erikh2000
// INI-related functions now return values in STD strings, to avoid prealloc requirement.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.25  2003/05/19 21:11:20  mrimer
// Some code maintenance.
//
// Revision 1.24  2003/05/18 20:04:30  erikh2000
// Fixed function that finds drod1_5.dat to look for drod(VERSION).dat.
//
// Revision 1.23  2003/05/04 00:21:49  mrimer
// Moved CStretchyBuffer encoding/decoding into the class (from CFiles).
//
// Revision 1.22  2003/04/29 11:06:42  mrimer
// Minor revisions.
//
// Revision 1.21  2003/04/28 22:16:35  mrimer
// Made new version of ReadFileIntoBuffer() more efficient.
//
// Revision 1.20  2003/04/08 13:09:27  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.19  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
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
