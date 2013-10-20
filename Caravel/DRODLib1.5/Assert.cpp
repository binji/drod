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

#ifdef _DEBUG

#include "Assert.h"

//Assertion checking module.
#ifdef WIN32
#	include <windows.h> //Should be first include.
#endif
#include "Types.h"
#include <stdio.h>
#include <string.h>
#include <mk4.h>

#include "Files.h"

//**************************************************************************************
void LogErr(const char *pszMessage)
//Logs an error to the error log.
{
	CFiles Files;

#ifdef WIN32
	MessageBeep(0);
	OutputDebugString(pszMessage);
#endif
	Files.AppendErrorLog(pszMessage);
}

//**************************************************************************************
void AssertErr(const char *pszFile, int nLine)
//Logs an assertian error to the error log.
{
	char szMessage[100];
	sprintf(szMessage, "Assertian error in line %d of %s.\r\n", nLine, pszFile);
	LogErr(szMessage);
	DebugPrint(szMessage);
#ifdef WIN32
	DebugBreak();
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
#ifdef WIN32
			OutputDebugString(szChunk);
#else
	#error Debug output code not provided.
#endif

			//Reset write position.
			psWrite = szChunk;
			wWriteChars = 0;
		}
	}

	//Write whatever is left over in the chunk.
	if (wWriteChars)
	{
		*psWrite = '\0';
		#ifdef WIN32
			OutputDebugString(szChunk);
		#else
			#error Debug output code not provided.
		#endif
	}
}

#endif //...#ifdef _DEBUG

// $Log: Assert.cpp,v $
// Revision 1.1  2003/02/25 00:01:14  erikh2000
// Initial check-in.
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
