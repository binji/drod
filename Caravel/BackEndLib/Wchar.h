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
 * Matt Schikore (schik)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef WCHAR_H
#define WCHAR_H

#include <string>
#include <functional>

#ifdef __linux__

#include "CharTraits.h"

using namespace std;

#else

#define WCv(x) (x)
#define pWCv(x) (*(x))

using namespace std;

typedef unsigned short	WCHAR_t;
typedef WCHAR_t	      WCHAR;    //wc, 16-bit UNICODE character

#endif

typedef basic_string<WCHAR, std::char_traits<WCHAR>, allocator<WCHAR> > WSTRING;

typedef unsigned int		UINT;

// This could actaully be replaced by '{' and '}' around the char constants,
// but it's handy to keep it in case we need to change it later.
#define W_t(x) {x}

//Some common small strings.  Larger strings are localizable and should be kept in database.
extern const WCHAR wszAsterisk[], wszColon[], wszComma[], wszCRLF[],
   wszElipsis[], wszEmpty[], wszExclamation[], wszHyphen[], wszParentDir[],
   wszPeriod[], wszQuestionMark[], wszQuote[], wszSemicolon[], wszSpace[],
   wszLeftParen[], wszRightParen[], wszDrodDotDat[], wszSlash[];


void AsciiToUnicode(const char *psz, WSTRING &wstr);
void UnicodeToAscii(const WSTRING& wstr, char *psz);
void UnicodeToAscii(const WSTRING& wstr, string &str);

bool charFilenameSafe(const WCHAR wc);
WSTRING  filenameFilter(const WSTRING &wstr);
WSTRING  filterFirstLettersAndNumbers(const WSTRING &wstr);
WSTRING  filterUpperCase(const WSTRING &wstr);

int		_Wtoi(const WCHAR* wsz);
WCHAR*	_itoW(int value, WCHAR* wcz, int radix);
WCHAR*	_ltoW(long value, WCHAR* wcz, int radix);

UINT		WCSlen(const WCHAR* wsz);
WCHAR*	WCScpy(WCHAR* wszDest, const WCHAR* wszSrc);
WCHAR*	WCSncpy(WCHAR* wszDest, const WCHAR* wszSrc, UINT n);
WCHAR*	WCScat(WCHAR* pwcz1, const WCHAR* pwcz2);
int		WCScmp(const WCHAR* pwcz1, const WCHAR* pwcz2);
int		WCSicmp(const WCHAR* pwcz1, const WCHAR* pwcz2);
int		WCSncmp(const WCHAR* pwcz1, const WCHAR* pwcz2, const UINT n);
WCHAR*	WCStok(WCHAR *wszStart, const WCHAR *wszDelim);

struct WSTRINGicmp : public binary_function<WSTRING, WSTRING, bool> {
   bool operator()(const WSTRING& x, const WSTRING& y) const { return WCSicmp(x.c_str(), y.c_str()) < 0;}
};

WCHAR*	fgetWs( WCHAR* wcz, int n, FILE* pFile);

#endif

// $Log: Wchar.h,v $
// Revision 1.15  2004/09/22 09:50:25  gjj
// gcc 3.4 namespace problem workaround
//
// Revision 1.14  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.13  2003/10/06 02:39:13  erikh2000
// Added overload for Unicode-to-ascii conversion function that take string param.CVS: ----------------------------------------------------------------------
//
// Revision 1.12  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/07/22 19:01:26  mrimer
// Added wszComma.
//
// Revision 1.10  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.8  2003/07/03 21:37:22  mrimer
// Added WCStok (committed on behalf of Gerry JJ) and two methods for string modifying.
//
// Revision 1.7  2003/07/01 20:31:37  mrimer
// Added more common wchar texts.
//
// Revision 1.6  2003/06/30 18:50:04  schik
// Added new function to be used as case-insensitive predicate for a container.
//
// Revision 1.5  2003/06/18 23:55:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/16 20:15:22  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.3  2003/06/12 21:41:09  mrimer
// Added methods to filter file name chars.
//
// Revision 1.2  2003/05/30 01:14:03  mrimer
// Added another file access type string.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.4  2003/04/29 11:05:05  mrimer
// Added WSCncmp().  Refined some code.
// Added license header and message log.
//
