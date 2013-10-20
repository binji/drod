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

#include "Assert.h"
#include "StretchyBuffer.h"

#define MAX_PROFILE_STRING 80
#ifndef MAX_PATH
	#define MAX_PATH 260
#endif

//File separator.
const char szFILE_SEP[] = "\\";

class CFiles
{
public:
	CFiles(void);
	~CFiles(void);

	static void		AppendErrorLog(const char *pszText);
	static bool		DoesFileExist(const char *pszFilepath);
	static const char *GetAppPath(void) {ASSERT(pszAppPath); return pszAppPath;}
	static const char *GetDatPath(void) {ASSERT(pszDatPath); return pszDatPath;}
	bool		GetDRODProfileString(const char *pszSection, const char *pszKey, 
			char *pszValue);
	static bool	HasReadWriteAccess(const char *pszFilepath);
	static bool	ReadFileIntoBuffer(const char *pszFilepath, CStretchyBuffer &Buffer);
	static void		TryToFixDataPath();
	bool		WriteDRODProfileString(const char *pszSection, const char *pszKey, 
			const char *pszValue);
	static bool	WriteBufferToFile(const char *pszFilepath, const CStretchyBuffer &Buffer);

	//File encryption/unencryption.
	static bool		FileIsEncrypted(const char *pszFilepath) {
			return pszFilepath[strlen(pszFilepath) - 1] == '_';}
	static void		EncodeBuffer(CStretchyBuffer &buffer);
	static bool		ProtectFile(const char *pszFilepath);
	//symmetric operations
	static void		DecodeBuffer(CStretchyBuffer &buffer) {EncodeBuffer(buffer);}
	static bool		UnprotectFile(const char *pszFilepath) {return ProtectFile(pszFilepath);}
	static bool		GetTrueDatafileName(char *const pszFilepath);
	static void		MutateFileName(char *pszFilepath);

private:
	void		DeinitClass(void);
	static bool		FindPossibleDatPath(const char *pszStartPath, 
			char *pszPossibleDatPath);
	void		InitClass(void);
	static bool		WriteDataPathTxt(const char *pszFilepath, const char *pszDatPath, 
			const bool overwrite);

	static char *pszAppPath;
	static char *pszDatPath;
	static unsigned long dwRefCount;

	PREVENT_DEFAULT_COPY(CFiles);
};

#endif //...#ifndef FILES_H

// $Log: Files.h,v $
// Revision 1.2  2003/05/19 21:12:14  mrimer
// Some code maintenance.
//
// Revision 1.1  2003/02/25 00:01:34  erikh2000
// Initial check-in.
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
