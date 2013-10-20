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
 * Matt Schikore (Schik)
 *
 * ***** END LICENSE BLOCK ***** */

#ifdef WIN32
#	include <windows.h> //Should be first include.
#	pragma warning(disable:4786)
#endif

#include "Assert.h"
#include "Clipboard.h"

//******************************************************************************
CClipboard::CClipboard()
//Constructor.
{
}

//******************************************************************************
bool CClipboard::SetString(
//Copies the given string into the system clipboard
//Returns: true on success, false otherwise.
//
//Params:
const string& sClip )  //(in)
{
#ifdef WIN32
	HGLOBAL global;
	LPSTR data;

	if (!OpenClipboard( NULL )) return false;
	EmptyClipboard();

	global = GlobalAlloc( GMEM_ZEROINIT, sClip.size()+1 );
	
	if (global == NULL) {
		CloseClipboard();
		return false;
	}

	data = (LPSTR)GlobalLock(global);

	strcpy( data, sClip.c_str() );

	GlobalUnlock( global );
	SetClipboardData( CF_TEXT, global );
	CloseClipboard();
//	GlobalFree( global );

#elif defined(__linux__)
#warning TODO: Add Clipboard write code for Linux.
	return false;
#elif defined(__native_client__)
	return false;
#else
#error How do you set system clipboard data on this system?
#endif
   return true;
}

//******************************************************************************
bool CClipboard::GetString(
//Copies the system clipboard string into sClip
//Returns: true on success, false otherwise.
//
//Params:
string& sClip )  //(out)
{
#ifdef WIN32
   HGLOBAL global;
   LPSTR data;
   unsigned long size;

   if (!OpenClipboard(NULL)) return false;
   global = GetClipboardData(CF_TEXT);
   if (global == NULL) {
      CloseClipboard();
      return false;
   }
   data = (LPSTR)GlobalLock(global);
   size = GlobalSize(global);
   sClip = data;
   GlobalUnlock(global);
   CloseClipboard();
//	GlobalFree( global );

#elif defined(__linux__)
#warning TODO: Add Clipboard read code for Linux.
	return false;
#elif defined(__native_client__)
	return false;
#else
#error How do you get system clipboard data on this system?
#endif
	return true;
}

//******************************************************************************
bool CClipboard::GetString(
//Copies the system clipboard string into sClip
//Returns: true on success, false otherwise.
//
//Params:
	WSTRING& sClip) //(out)
{
   string sStr;
   bool bStatus = GetString(sStr);
   if (bStatus)
      AsciiToUnicode(sStr.c_str(), sClip);
   return bStatus;
}


//******************************************************************************
bool CClipboard::SetString(
//Copies the given string into the system clipboard
//Returns: true on success, false otherwise.
//
//Params:
	const WSTRING& sClip)  //(in)
{
	char* pszBuffer = new char[sClip.size()+1];
	UnicodeToAscii(sClip, pszBuffer);
	const bool bRes = SetString(pszBuffer);
	delete[] pszBuffer;
	return bRes;
}

// $Log: Clipboard.cpp,v $
// Revision 1.3  2005/03/15 21:39:15  mrimer
// Fixed memory leak.
//
// Revision 1.2  2003/09/11 02:02:25  mrimer
// Linux fixes (committed on behlaf of Gerry JJ).
//
