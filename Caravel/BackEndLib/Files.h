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

//Files.h
//Declarations for CFiles.
//Class for handling file-related tasks.

#ifndef FILES_H
#define FILES_H

#if defined WIN32
#	define HAS_UNICODE
#elif defined __sgi
#	undef HAS_UNICODE
#elif defined __APPLE__
# 	undef HAS_UNICODE
#elif defined __linux__
#	undef HAS_UNICODE
#else
#error Platform unknown!  Does this platform support Unicode?
#endif


#ifdef WIN32
#	define SLASH '\\'
#	pragma warning(disable:4786)
#else
#	define SLASH '/'
#endif

#include "Assert.h"
#include "StretchyBuffer.h"
#include "IniFile.h"
#include <string>

using namespace std;

#define MAX_PROFILE_STRING 80
#ifndef MAX_PATH
#	ifdef PATH_MAX
#		define MAX_PATH PATH_MAX
#	else
#		define MAX_PATH 260
#	endif
#endif

class CFiles
{
public:
    CFiles(const WCHAR *wszSetAppPath = NULL,
            const WCHAR *wszSetGameName = NULL, const WCHAR *wszSetGameVer = NULL);
    ~CFiles();

    static FILE *           Open(const WCHAR *pszFilepath, const char *pszOptions);
    static bool             DoesFileExist(const WCHAR *pszFilepath);
    static bool             HasReadWriteAccess(const WCHAR *pwzFilepath);
    static bool             IsValidPath(const WCHAR *pwzPath);
    static bool             ReadFileIntoBuffer(const WCHAR *pwzFilepath, CStretchyBuffer &Buffer);
    static bool             WriteBufferToFile(const WCHAR *pwzFilepath, const CStretchyBuffer &Buffer);
    static bool             WriteBufferToFile(const char *pszFilepath, const CStretchyBuffer &Buffer);
    static bool             FileCopy(const WCHAR *pszSourceFilepath, const WCHAR *pszDestFilepath);

    //These methods require class construction.
    void                    AppendErrorLog(const char *pszText);
    const WCHAR *           GetAppPath() {ASSERT(wszAppPath); return wszAppPath;}
    const WCHAR *           GetDatPath() {ASSERT(wszDatPath); return wszDatPath;}
    const WCHAR *           GetResPath() {ASSERT(wszResPath); return wszResPath;}
    bool                    GetGameProfileString(const char *pszSection, const char *pszKey,
		    string &strValue);
    void	                TryToFixDataPath();
    bool                    WriteGameProfileString(const char *pszSection, const char *pszKey,
		    const char *pszValue);

#ifdef USE_LOGCONTEXT
    //Used to set context that is described in error log.  Use LOGCONTEXT() macro from
    //instead of calling directly.
    static void             PushLogContext(const char *pszDesc);
    static void             PopLogContext(const char *pszDesc);
#endif

    //File encryption/unencryption.
    static bool             FileIsEncrypted(const WCHAR *wszFilepath) {
	    return wszFilepath[WCSlen(wszFilepath) - 1] == '_';}
    static bool             ProtectFile(const WCHAR *pszFilepath);
    //symmetric operations
    static bool             UnprotectFile(const WCHAR *pszFilepath) {return ProtectFile(pszFilepath);}
    static bool             GetTrueDatafileName(WCHAR *pszFilepath);
    static void             MutateFileName(WCHAR *pszFilepath);
    static bool            WindowsCanBrowseUnicode();
#ifdef __linux__
    static bool             CreatePathIfInvalid (const WCHAR *wszPath);
    static const WCHAR *    GetHomePath() {return wstrHomePath.c_str();}
#endif

private:
    void                    DeinitClass();
#ifdef __linux__
    static bool             CheckDataAndMakeWritable(WCHAR *wszPath);
    static bool             CheckForDataFile(WCHAR *wszPath, const WCHAR *wszFmt,
                                  const WCHAR *wszFile, const bool bTryClean, const bool bData);
    static bool             FindPossibleDatFile(const WCHAR *wszFilename, WCHAR *wszPathBuf);
    static bool             FindPossibleResFile(const WCHAR *wszFilename, WCHAR *wszPathBuf);
#else
    static bool             FindPossibleDatPath(const WCHAR *wszStartPath, WCHAR *wszPossibleDatPath);
    static bool             FindPossibleResPath(const WCHAR *wszStartPath, WCHAR *wszPossibleResPath);
#endif
    void		            InitClass(const WCHAR *pszSetAppPath,
  		    const WCHAR *wszSetGameName, const WCHAR *wszSetGameVer);
    static bool             WriteDataPathTxt(const WCHAR *pszFilepath,
		    const WCHAR *wszDatPath, const WCHAR *wszResPath, const bool overwrite);

#ifdef __linux__
    static WSTRING wstrHomePath;
#endif
    static WCHAR *wszAppPath;
    static WCHAR *wszDatPath;
    static WCHAR *wszResPath;
    static WCHAR *wszGameName, *wszGameVer;
    static unsigned long dwRefCount;
    static CIniFile gameIni;
    static bool bInitIni;

    PREVENT_DEFAULT_COPY(CFiles);
};

#ifdef __linux__
//A small class for getting trees of search paths.  The expr parameter to
//the constructor is a string in a special format containing the paths we want
//with a few control chars thrown in (all control chars except ; are replaced
//by SLASH in the output).  Most control chars adds a bit to the setup.
//Next() returns every bit combination in turn, starting with all bits set
//and decreasing.  The control chars are:
//
//  '/'  (no bit)  Always write
//  ';'  (no bit)  (Invisible) Always write, switch off all control chars,
//                 return position
//  '?'  (+1 bit)  Write if bit is on ("toggle")
//  '<'  (+1 bit)  Write if bit is on and preceding bit is on ("tail")
//  '>'  (+1 bit)  Write if bit is on and preceding bit is off ("swap")
//
//So, passing the string "/one?two>three<four" returns this in the buffer
//(four calls to Next()):
//
//      "/one/two", "/one/three/four", "/one/three", "/one"
//
//(one is static, two is toggled and swapped with three, and four is
// three's tail)

class CPathGen {
   WCHAR       *wszBuffer;
   const WCHAR *wszExpr;
   UINT         wBits, wN;

public:
   CPathGen(WCHAR *buffer, const WCHAR *expr);
   int Next();
};
#endif //#ifdef __linux__

#endif //...#ifndef FILES_H

// $Log: Files.h,v $
// Revision 1.17  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.16  2003/11/09 05:20:54  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.15  2003/10/07 21:09:49  erikh2000
// Added context information to logging.
//
// Revision 1.14  2003/10/06 02:37:08  erikh2000
// Added file-copying method.
// Error log appends now show date/time.
//
// Revision 1.13  2003/08/07 18:36:24  erikh2000
// Removed redundant IsPathValid() function.
// Fixed a bug with an unterminated string constant.
//
// Revision 1.12  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/08/05 00:09:36  erikh2000
// Added IsValidPath() function.
//
// Revision 1.10  2003/08/01 20:26:50  schik
// Moved WindowsCanBrowseUnicode() to CFiles and made it static.
//
// Revision 1.9  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.8  2003/06/16 20:15:22  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.7  2003/06/15 04:19:15  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.6  2003/05/28 23:02:53  erikh2000
// Made changes to CFiles methods to prevent errors.
//
// Revision 1.5  2003/05/26 14:53:58  mrimer
// Made two more methods static.
//
// Revision 1.4  2003/05/26 01:41:10  erikh2000
// INI-related functions now return values in STD strings, to avoid prealloc requirement.
//
// Revision 1.3  2003/05/24 17:23:39  mrimer
// Removed an obsolete variable.
//
// Revision 1.2  2003/05/23 21:10:57  mrimer
// Added port to APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.20  2003/05/19 21:11:19  mrimer
// Some code maintenance.
//
// Revision 1.19  2003/05/18 03:41:28  schik
// Now shows an error when compiling on a platform for which it is unknown whether or not Unicode is supported.
//
// Revision 1.18  2003/05/04 00:21:50  mrimer
// Moved CStretchyBuffer encoding/decoding into the class (from CFiles).
//
// Revision 1.17  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.16  2003/02/24 17:04:21  erikh2000
// Added file separator constant.
//
// Revision 1.15  2002/11/13 23:18:35  mrimer
// Made a parameter const.
//
// Revision 1.14  2002/09/24 21:14:28  mrimer
// Declared general file operations as static.
//
// Revision 1.13  2002/08/29 09:17:50  erikh2000
// Added method to write a buffer to a file.
//
// Revision 1.12  2002/08/23 23:24:20  erikh2000
// Changed some params.
//
// Revision 1.11  2002/07/18 20:15:42  mrimer
// Added GetTrueDatafileName() and MutateFileName().  Revised ProtectFile().
//
// Revision 1.10  2002/07/05 17:59:34  mrimer
// Minor fixes (includes, etc.)
//
// Revision 1.9  2002/07/02 23:53:31  mrimer
// Added file encryption/unencryption routines.
//
// Revision 1.8  2002/04/28 23:51:11  erikh2000
// Added new method to check for read/write access on a file.
//
// Revision 1.7  2002/04/11 10:08:29  erikh2000
// Wrote CFiles::ReadFileIntoBuffer().
//
// Revision 1.6  2002/04/09 01:04:31  erikh2000
// Twiddling.
//
// Revision 1.5  2002/03/14 03:51:40  erikh2000
// Added TryToFixDataPath() and overwrite parameter to WriteDataPathTxt().  (Committed on behalf of mrimer.)
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2001/12/08 01:44:59  erikh2000
// Added PREVENT_DEFAULT_COPY() macro to class declaration.
//
// Revision 1.2  2001/10/13 02:37:53  erikh2000
// If DataPath.txt is not present, DRODLib will now search for a possible data path to use and write this path to DataPath.txt.
//
// Revision 1.1.1.1  2001/10/01 22:20:15  erikh2000
// Initial check-in.
//
