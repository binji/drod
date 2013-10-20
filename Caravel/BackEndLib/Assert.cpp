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

#include "Assert.h"

#ifdef _DEBUG

//Assertion checking module.
#ifdef WIN32
#	include <windows.h> //Should be first include.
#	pragma warning(disable:4786)
#endif
#include "Types.h"
#include <stdio.h>

#include "Files.h"

#ifndef WIN32
void OutputDebugStringA(const char *pszMessage)
// Outputs a string to stderr (Non-WIN32 only)
{
	fprintf(stderr, "DROD:%s\n", pszMessage);
}
#endif

//**************************************************************************************
#ifdef USE_LOGCONTEXT
CLogContext::CLogContext(const char *pszDesc) : strDesc(pszDesc) {CFiles::PushLogContext(pszDesc);}
CLogContext::~CLogContext(void) {CFiles::PopLogContext(strDesc.c_str());}
#endif

//**************************************************************************************
void LogErr(const char *pszMessage)
//Logs an error to the error log.
{
	//CFiles Files;

#ifdef WIN32
	MessageBeep(0);
#endif
	DebugPrint(pszMessage);
    CFiles Files;
	Files.AppendErrorLog(pszMessage);
}

//**************************************************************************************
void AssertErr(const char *pszFile, int nLine, const char *pszDesc)
//Logs an assertion error to the error log.
{
	char szMessage[500];
	sprintf(szMessage, "Assertion error in line %d of %s: \"%s\"\r\n", nLine, pszFile, pszDesc);
	LogErr(szMessage);
#ifndef RELEASE_WITH_DEBUG_INFO //Don't debugbreak() in a release build, because it will just crash.
#ifdef WIN32
	DebugBreak();
#endif
#endif
}

//**************************************************************************************
void DebugPrint(const char *pszMessage)
//Send message to debug output.
{
	char szChunk[256];
	szChunk[255] = '\0'; //Last char will always be term zero.
	const char *pszSeek = pszMessage;
	char *psWrite = szChunk;
	UINT wWriteChars = 0;

	//Copy every 255 chars to temp buffer and send in chunks.  Each iteration
	//processes one char from message.
	while (*pszSeek != '\0')
	{
		*(psWrite++) = *(pszSeek++);
		if ((++wWriteChars) == 255) //Chunk is full and ready to send.
		{
			OutputDebugStringA(szChunk);

			//Reset write position.
			psWrite = szChunk;
			wWriteChars = 0;
		}
	}

	//Write whatever is left over in the chunk.
	if (wWriteChars)
	{
		*psWrite = '\0';
		OutputDebugStringA(szChunk);
	}
}

#endif //...#ifdef _DEBUG

// $Log: Assert.cpp,v $
// Revision 1.10  2003/11/09 05:20:54  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/10/07 21:09:48  erikh2000
// Added context information to logging.
//
// Revision 1.8  2003/10/06 02:36:41  erikh2000
// Asserts will now log a more detailed error description.
//
// Revision 1.7  2003/07/30 04:51:58  mrimer
// Removed unneeded #includes.
//
// Revision 1.6  2003/06/28 20:08:59  erikh2000
// Revised conditional compilation for debug messages in release builds.
//
// Revision 1.5  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/28 02:34:10  erikh2000
// Added temporay #define _DEBUG for beta builds.
//
// Revision 1.3  2003/06/18 23:55:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/28 23:02:53  erikh2000
// Made changes to CFiles methods to prevent errors.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.11  2003/04/21 21:22:22  mrimer
// Mode CFiles method call static.
//
// Revision 1.10  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.9  2002/07/20 22:59:55  erikh2000
// Revised #includes.
//
// Revision 1.8  2002/06/21 04:30:22  mrimer
// Revised includes.
//
// Revision 1.7  2002/05/10 22:29:57  erikh2000
// Put debug break back into assertians.
//
// Revision 1.6  2002/04/30 05:11:15  erikh2000
// #ifdef'ed all the code so it will compile out in debug builds.
//
// Revision 1.5  2002/04/28 23:33:13  erikh2000
// Encapsulated win32 calls.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2002/02/25 03:36:17  erikh2000
// Added DEBUGPRINT macro and associated routine.
//
// Revision 1.2  2001/10/16 00:01:02  erikh2000
// Fixed problems with stack overflows when many assertians fired at one time.
//
// Revision 1.1.1.1  2001/10/01 22:20:06  erikh2000
// Initial check-in.
//
