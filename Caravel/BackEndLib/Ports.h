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

//Type declarations for common Windows types.  I'm just putting in the ones
//that are being used.

#ifndef PORTS_H
#define PORTS_H

#include "Types.h"

// Find the byte-order of the host
#define GAME_BYTEORDER_LITTLE (1)
#define GAME_BYTEORDER_BIG    (2)
// Autodetect
#if (defined(BYTE_ORDER) && defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN))
#	if (BYTE_ORDER == LITTLE_ENDIAN)
#		define GAME_BYTEORDER GAME_BYTEORDER_LITTLE
#	elif (BYTE_ORDER == BIG_ENDIAN)
#		define GAME_BYTEORDER GAME_BYTEORDER_BIG
#	endif
#elif (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN))
#	if (__BYTE_ORDER == __LITTLE_ENDIAN)
#		define GAME_BYTEORDER GAME_BYTEORDER_LITTLE
#	elif (__BYTE_ORDER == __BIG_ENDIAN)
#		define GAME_BYTEORDER GAME_BYTEORDER_BIG
#	endif
#endif
// If that failed, try to recognize the system
#ifndef GAME_BYTEORDER
#	if (defined(WIN32) || defined(__i386__))
#		define GAME_BYTEORDER GAME_BYTEORDER_LITTLE
#	elif (defined(__APPLE__) || defined(__sgi))
#		define GAME_BYTEORDER GAME_BYTEORDER_BIG
#	else
#		error Unknown byte order. Please add your system above.
#	endif
#endif

#ifndef WIN32

#ifdef __APPLE__
#  include <ctype.h>
#endif

#ifdef __linux__
#  include <ctype.h>
// strcasecmp is a BSD 4.4 function, and some systems might actually have
// stricmp. Should check for both and use a custom version if none exist
#  define stricmp strcasecmp
#endif

#ifndef __linux__
// Ugh
int _wtoi( const wchar_t* wcStr );
wchar_t* _itow( int value, wchar_t* str, int radix );
#endif
char* _ltoa( long value, char* buffer, int radix );
char* _itoa( int value, char* pBuffer, int radix );
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
void LittleToBig(char *pBuffer, int bytesPer, int count);
void LittleToBig(USHORT *pBuffer, int count = 1);
void LittleToBig(UINT *pBuffer, int count = 1);
void LittleToBig(DWORD *pBuffer, int count = 1);
void LittleToBig(int *pBuffer, int count = 1);
//void LittleToBig(WCHAR *pBuffer, int count = 1);
#endif // big endian

WCHAR towlower(const WCHAR ch);
bool iswlower(const WCHAR ch);
bool iswupper(const WCHAR ch);
bool iswspace(const WCHAR ch);
bool iswdigit(const WCHAR ch);
bool iswalpha(const WCHAR ch);

#endif //...#ifndef WIN32

#endif //...#ifndef PORTS_H

// $Log: Ports.h,v $
// Revision 1.9  2003/08/09 00:37:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.8  2003/07/22 19:37:22  mrimer
// Linux port fixes (committted on behalf of Gerry JJ).
//
// Revision 1.7  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/18 23:55:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/15 04:19:15  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.3  2003/05/24 02:06:28  mrimer
// Fixes for APPLE portability (committed on behalf of Ross Jones).
//
// Revision 1.2  2003/05/23 21:10:57  mrimer
// Added port to APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.1  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
//
