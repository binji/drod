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

#ifdef WIN32
#	include <windows.h> //Should be first include.
#endif
#include <stdio.h>
#include <string.h>

//Assertion checking and debugging info.
#ifdef _DEBUG
//****************************************************************
void AssertErr(char *pszFile, int nLine, char *pszDesc)
{
	char szMessage[500];
    printf(szMessage, "Assertion error in line %d of %s: \"%s\"\r\n", nLine, pszFile, pszDesc);
	#ifdef WIN32
		DebugBreak();
	#endif
}
#endif

// $Log: Assert.cpp,v $
// Revision 1.7  2004/08/08 21:46:15  mrimer
// Parameter fix (committed on behalf of erikh2000).
//
// Revision 1.6  2002/07/20 23:22:53  erikh2000
// Revised #includes.
//
// Revision 1.5  2002/06/05 03:23:23  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.4  2002/05/15 13:09:25  mrimer
// Added log macro to end of file.
//
