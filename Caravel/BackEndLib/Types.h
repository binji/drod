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
 * Portions created by the Initial Developer are Copyright (C) 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//Type declarations.

#ifndef TYPES_H
#define TYPES_H

#include "Wchar.h"

//Make sure I have the basics, like NULL.  I don't know how portable this 
//include is.
#	include <time.h>

#	ifndef _WINDOWS_ //If <windows.h> wasn't included.

typedef unsigned char		BYTE;
#ifndef UCHAR
typedef unsigned char      UCHAR;
#endif
typedef unsigned short		USHORT;
typedef unsigned int		   UINT;
typedef unsigned long		DWORD;
#ifndef ULONG
typedef unsigned long      ULONG;
#endif

typedef struct tagPOINT
{
    long  x;
    long  y;
} POINT;

#	endif	//...#ifndef _WINDOWS_

typedef short			SHORT;

#ifdef WIN32
typedef unsigned __int64 ULONGLONG;
#elif (defined __linux__)
typedef unsigned long long ULONGLONG;
#else
//!!FIX: for other OSs
typedef unsigned long ULONGLONG;
#endif

//good random number picking (rely more on high order bits)
#define RAND(a)	(UINT)(((ULONGLONG)rand() * (a)) / ((ULONGLONG)RAND_MAX+1))
//uniform random value between plus and minus 'a'
#define fRAND_MID(a) (((a) * ((rand() % 20000) / 10000.0)) - (a))

#endif //...#ifndef TYPES_H

// $Log: Types.h,v $
// Revision 1.5  2003/09/16 20:39:53  mrimer
// Added macro fRAND_MID to generate a random floating point number.
//
// Revision 1.4  2003/07/22 19:37:22  mrimer
// Linux port fixes (committted on behalf of Gerry JJ).
//
// Revision 1.3  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/24 02:06:29  mrimer
// Fixes for APPLE portability (committed on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.8  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.7  2003/02/17 00:48:12  erikh2000
// Remove L" string literals.
//
// Revision 1.6  2003/02/16 20:27:53  erikh2000
// Added WSTRING typedef.
//
// Revision 1.5  2002/07/20 23:04:49  erikh2000
// Changed Types.h so that it doesn't #include windows.h.  I want to be able to easily see the places where I am using Windows API declarations, and minimize dependencies.
//
