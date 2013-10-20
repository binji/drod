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

//Compat.h
//
//SUMMARY
//
//Class for accessing DRODLib features when the library is built inside of a compatibility.dll.

#ifndef COMPAT_H
#define COMPAT_H

#include <BackEndLib/Types.h>
#include "GameConstants.h"

#include <string>

using namespace std;

class CCompat
{
public:
	static bool		CreateData(const char *pszPath, string &strResultText);
	static bool		DeleteData(const char *pszPath, string &strResultText);
	static bool		ExportXML(const char *pszFromPath, const char *pszToPath, DWORD dwFlags,
			string &strResultText);
	static DWORD	GetVersion(void) {return dwCurrentDRODVersion;}
	static bool		ImportXML(const char *pszFromPath, const char *pszToPath, DWORD dwFlags, 
			string &strResultText);
};

#endif //...#ifndef COMPAT_H

// $Log: Compat.h,v $
// Revision 1.4  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.3  2003/05/22 23:39:00  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.2  2003/05/09 02:42:37  mrimer
// Added license block and log comment.
//
