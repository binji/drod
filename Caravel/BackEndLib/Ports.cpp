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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "Ports.h"

#ifndef WIN32
#ifndef __linux__
// These are only used in DRODLib1.5, which isn't ported to linux.
int _wtoi( const wchar_t* wcStr )
{
	static char szTemp[33];
	wcstombs(szTemp, wcStr, 33);
	return atoi(szTemp);
}

wchar_t* _itow( int value, wchar_t* str, int /*radix*/ )
{
	static char szTemp[33];
	sprintf(szTemp, "%d", value);
	mbstowcs(str, szTemp, 33);
	return str;
}
#endif // #ifndef __linux__

char* _itoa(int value, char* buffer, int radix)
{
   assert(radix > 0);
   //Passing in a NULL string allocates a string, to be deallocated by the caller.
	if (NULL == buffer) buffer = (char*)(malloc(sizeof(char) * 33));

   //Handle negative numbers.
   bool bNegative = false;
   if (value < 0) {bNegative = true; value = -value;}

   UINT i=0;
	do {
		const UINT val = value % radix;
      buffer[i++] = val < 10 ? val + '0' : val - 10 + 'A';
	} while ((value /= radix) > 0);

   if (bNegative) buffer[i++] = '-';
	buffer[i] = '\0';

   //Reverse string.
	char c;
   UINT j=i-1;
	for (i=0; i<j; ++i, --j) {
		c = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = c;
	}
	return buffer;
}

char* _ltoa(long value, char* buffer, int radix)
{
   assert(radix > 0);
   //Passing in a NULL string allocates a string, to be deallocated by the caller.
	if (NULL == buffer) buffer = (char*)(malloc(sizeof(char) * 33));

   //Handle negative numbers.
   bool bNegative = false;
   if (value < 0) {bNegative = true; value = -value;}

   UINT i=0;
	do {
		const UINT val = value % radix;
      buffer[i++] = val < 10 ? val + '0' : val - 10 + 'A';
	} while ((value /= radix) > 0);

   if (bNegative) buffer[i++] = '-';
	buffer[i] = '\0';

   //Reverse string.
	char c;
   UINT j=i-1;
	for (i=0; i<j; ++i, --j) {
		c = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = c;
	}
	return buffer;
}

#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)

void LittleToBig(char *pBuffer, int bytesPer, int count)
{
	char* psz = pBuffer;
	char c;
	for (int n=0; n < count * bytesPer; n += bytesPer)
	{
		for (int b=0; b < bytesPer/2; b++)
		{
			c = psz[n+b];
			psz[n] = psz[n+bytesPer-1-b];
			psz[n+bytesPer-1-b] = c;
		}
	}
}

void LittleToBig(USHORT *pBuffer, int count)
{
	LittleToBig((char*)pBuffer, sizeof(USHORT), count);
}

void LittleToBig(UINT *pBuffer, int count)
{
	LittleToBig((char*)pBuffer, sizeof(UINT), count);
}

void LittleToBig(DWORD *pBuffer, int count)
{
	LittleToBig((char*)pBuffer, sizeof(DWORD), count);
}

void LittleToBig(int *pBuffer, int count)
{
	LittleToBig((char*)pBuffer, sizeof(int), count);
}

/*
void LittleToBig(WCHAR *pBuffer, int count )
{
	LittleToBig((char*)pBuffer, sizeof(WCHAR), count);
}
*/

#endif // big endian

WCHAR towlower(const WCHAR ch)
{
#if defined(__native_client__)
	const WCHAR_t c = ch;
#else
	const WCHAR_t c = ch.value;
#endif
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
	LittleToBig( static_cast<WCHAR *>(&c), 1 );
#endif
	WCHAR r = W_t(tolower( static_cast<char>(c) ));
	return r;
}

bool iswlower(const WCHAR ch)
{
#if defined(__native_client__)
	const WCHAR_t c = ch;
#else
	const WCHAR_t c = ch.value;
#endif
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
	LittleToBig( static_cast<WCHAR *>(&c), 1 );
#endif
	return islower( static_cast<char>(c) );
}

bool iswupper(const WCHAR ch)
{
#if defined(__native_client__)
	const WCHAR_t c = ch;
#else
	const WCHAR_t c = ch.value;
#endif
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
	LittleToBig( static_cast<WCHAR *>(&c), 1 );
#endif
	return isupper( static_cast<char>(c) );
}

bool iswspace(const WCHAR ch)
{
	WSTRING wsz;
	// Any other spaces ?
	AsciiToUnicode(" \t\r\n", wsz);
	return (wsz.find(ch, 0) != WSTRING::npos);
}

bool iswdigit(const WCHAR ch)
{
	WSTRING wsz;
	// Any other digits ?
	AsciiToUnicode("0123456789", wsz);
	return (wsz.find(ch, 0) != WSTRING::npos);
}

bool iswalpha(const WCHAR ch)
{
	return (isalpha(static_cast<char>(WCv(ch))));
}

#endif

// $Log: Ports.cpp,v $
// Revision 1.9  2004/02/23 20:40:38  mrimer
// Linux fix: _itoa and _ltoa didn't handle negative numbers.
//
// Revision 1.8  2003/08/19 20:11:29  mrimer
// Linux maintenance (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/08/09 00:37:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/18 23:55:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/15 04:19:15  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.3  2003/05/24 02:06:29  mrimer
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
