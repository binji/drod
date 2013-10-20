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
 * John Wm. Wicks (j_wicks)
 *
 * ***** END LICENSE BLOCK ***** */

#if !defined(WIN32) && !defined(__linux__)
void ShowHelp(const char* /*pszPageName*/) { }
#elif defined(WIN32)
#	include <windows.h> //Should be first include.

#	include "../DRODLib/Db.h"
#	include "../DRODLib/DbPlayers.h"
#	include <BackEndLib/MessageIDs.h>
#	include <BackEndLib/Files.h>
#	include <stdio.h>

extern HWND m_hwndWin;
HWND m_hBrwsWin=NULL;

//*****************************************************************************
void Error()
//Shows error message if file can't be opened.
{
    MessageBeep(0);
     
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        0, // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );
    OutputDebugStringA((char *)lpMsgBuf);
    CFiles Files;
    Files.AppendErrorLog((char *)lpMsgBuf);
    // Free the buffer.
    LocalFree( lpMsgBuf );
}

//*****************************************************************************
void ShowHelp(
//Uses the system default browser to show a page from the help directory.
//
//Params:
	const char *pszPageName)	//(in) Just the filename, i.e. "contents.html".
{

 	const int MaxPath=260;
 	char *pszHelp = new char[MaxPath+1];
	WSTRING wstrHelp;
 	char *pszCurLang = new char[MaxPath+1];
	WCHAR wszOpen[] = {'o', 'p', 'e', 'n', 0 };
   char pszOpen[] = {'o', 'p', 'e', 'n', 0 };
	WCHAR wszBuffer[16];
	WSTRING wstrPageName;

	AsciiToUnicode(pszPageName, wstrPageName);

	CFiles Files;
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	int nLanguageCode = (pCurrentPlayer == NULL) ? (int) English :
			pCurrentPlayer->Settings.GetVar("Language", (int) English);
	if (pCurrentPlayer) delete pCurrentPlayer;
	wstrHelp = Files.GetResPath();
	wstrHelp += SLASH;
	wstrHelp += 'H';
	wstrHelp += 'e';
	wstrHelp += 'l';
	wstrHelp += 'p';
	wstrHelp += SLASH;
	wstrHelp += _itoW(nLanguageCode, wszBuffer, 10);
	wstrHelp += SLASH;
	wstrHelp += wstrPageName;

 	if (m_hBrwsWin && (!IsWindow(m_hBrwsWin))) m_hBrwsWin = NULL;

   if (CFiles::WindowsCanBrowseUnicode())
   {
	   SHELLEXECUTEINFOW myShExInfo;
      //Open file in browser window.
	   myShExInfo.cbSize = sizeof(SHELLEXECUTEINFO);
      //myShExInfo.fMask = (SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS);
      myShExInfo.hwnd = m_hBrwsWin;
	   myShExInfo.fMask = NULL;
	   myShExInfo.lpParameters = NULL;
	   myShExInfo.lpDirectory = NULL;
	   myShExInfo.nShow = SW_SHOWNORMAL;
	   myShExInfo.hInstApp = NULL;
	   myShExInfo.lpVerb = wszOpen;
	   myShExInfo.lpFile = wstrHelp.c_str();
	   if(!ShellExecuteExW(&myShExInfo))
         Error();
 	   if( (int)myShExInfo.hInstApp > 32 ) m_hBrwsWin = myShExInfo.hwnd;
   }
   else
   {
	   SHELLEXECUTEINFOA myShExInfo;
      //Open file in browser window.
	   myShExInfo.cbSize = sizeof(SHELLEXECUTEINFO);
      //myShExInfo.fMask = (SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS);
      myShExInfo.hwnd = m_hBrwsWin;
	   myShExInfo.fMask = NULL;
	   myShExInfo.lpParameters = NULL;
	   myShExInfo.lpDirectory = NULL;
	   myShExInfo.nShow = SW_SHOWNORMAL;
	   myShExInfo.hInstApp = NULL;
      myShExInfo.lpVerb = pszOpen;
      UnicodeToAscii(wstrHelp, pszHelp);
      myShExInfo.lpFile = pszHelp;
	   if(!ShellExecuteExA(&myShExInfo))
         Error();
    	if( (int)myShExInfo.hInstApp > 32 ) m_hBrwsWin = myShExInfo.hwnd;
   }


	// If we were successful then save the HWND for later use
 	delete [] pszHelp;
 	delete [] pszCurLang;
}

#elif defined(__linux__)
#	include "../DRODLib/Db.h"
#	include "../DRODLib/DbPlayers.h"
#	include <BackEndLib/Files.h>
#	include <stdlib.h>

static const WCHAR wszAddHelpPath[] = {
	{SLASH},{'H'},{'e'},{'l'},{'p'},{SLASH},{0}};

void ShowHelp(
//Uses the system default browser to show a page from the help directory.
//
//Params:
	const char *pszPageName)	//(in) Just the filename, i.e. "contents.html".
{
	char *pszBrowser = getenv("BROWSER");
	if (!pszBrowser)
	{
		fprintf(stderr, "Couln't open help because the BROWSER environment variable isn't set.\n");
		return;
	}

	WCHAR wszBuffer[16];
	CFiles Files;

	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	int nLanguageCode = (pCurrentPlayer == NULL) ? (int) English :
			pCurrentPlayer->Settings.GetVar("Language", (int) English);
	if (pCurrentPlayer) delete pCurrentPlayer;
	WSTRING wstrHelp = Files.GetResPath();
	wstrHelp += wszAddHelpPath;
	wstrHelp += _itoW(nLanguageCode, wszBuffer, 10);
	wstrHelp += wszSlash;
	char pszHelp[wstrHelp.length() + strlen(pszBrowser) + strlen(pszPageName) + 4];
	strcpy(pszHelp, pszBrowser);
	strcat(pszHelp, " ");
	UnicodeToAscii(wstrHelp, pszHelp + strlen(pszHelp));
	strcat(pszHelp, pszPageName);
	strcat(pszHelp, " &");
	system(pszHelp);
}

#endif //...#if !(defined(WIN32) && !defined(__linux__)...#else

//Previous to the revisions described below, all code was contributed by j_wicks.
// $Log: Browser.cpp,v $
// Revision 1.21  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.20  2003/08/11 12:48:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.19  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.18  2003/08/01 20:27:21  schik
// Added code to hopefully make Help work in Win95/98/ME
//
// Revision 1.17  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.16  2003/05/28 23:13:11  erikh2000
// Methods of CFiles are called differently.
//
// Revision 1.15  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.14  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.13  2003/05/08 22:04:41  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.12  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.11  2002/12/22 02:44:38  mrimer
// Fixed <windows.h> #define bug.
//
// Revision 1.10  2002/09/24 21:38:49  mrimer
// Revised calls to CFiles::AppendErrorLog().
//
// Revision 1.9  2002/07/20 23:07:04  erikh2000
// Revised #includes.
//
// Revision 1.8  2002/07/17 21:17:43  mrimer
// Revised #includes.  Refined code a bit.
//
// Revision 1.7  2002/06/09 06:37:21  erikh2000
// Changed code to get current language from DB-stored player settings instead of DROD.ini.
//
// Revision 1.6  2002/04/16 10:39:27  erikh2000
// Put Windows code in #ifdef WIN32 block.  Added revision log macro.
//
