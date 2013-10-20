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

//CClipboard
//Declarations for CClipboard.
//Class for accessing and modifying the system clipboard

#ifndef CLIPBOARD_H
#define CLIPBOARD_H
#ifdef WIN32
#	pragma warning(disable:4786)
#endif

#include <string>
#include <map>
#include "Wchar.h"


using namespace std;

//***************************************************************************
class CClipboard
{
public:
	CClipboard();
	~CClipboard();

	static bool GetString(string& sClip);
	static bool SetString(const string& sClip);
	static bool GetString(WSTRING& sClip);
	static bool SetString(const WSTRING& sClip);
};

#endif //...#ifndef CLIPBOARD_H

// $Log: Clipboard.h,v $
// Revision 1.1  2003/08/15 15:50:27  schik
// New class for getting and setting clipboard data.  Right now, only strings are implemented, and only on the Windows platform.
//

