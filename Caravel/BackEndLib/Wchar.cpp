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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Matt Schikore (schik)
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "Wchar.h"
#include "Assert.h"
#include "Types.h"
#include "Ports.h"  // for towlower, iswspace

//Some common small strings.  Larger strings are localizable and should be kept in database.
const WCHAR wszAsterisk[] =	{W_t('*'), W_t(0)};
const WCHAR wszColon[] =	{W_t(':'), W_t(0)};
const WCHAR wszComma[] =	{W_t(','), W_t(0)};
const WCHAR wszCRLF[] =		{W_t('\r'), W_t('\n'), W_t(0)};
const WCHAR wszElipsis[] =	{W_t('.'), W_t('.'), W_t('.'), W_t(0)};
const WCHAR wszEmpty[] =	{W_t(0)};
const WCHAR wszExclamation[] = {W_t('!'), W_t(0)};
const WCHAR wszHyphen[] =  { W_t('-'),W_t(0) };
const WCHAR wszParentDir[] =	{W_t('.'), W_t('.'), W_t(0)};
const WCHAR wszPeriod[] =	{W_t('.'), W_t(0)};
const WCHAR wszQuestionMark[] = {W_t('?'), W_t(0)};
const WCHAR wszQuote[] =	{W_t('\"'), W_t(0)};
const WCHAR wszSemicolon[] =	{W_t(';'), W_t(0)};
const WCHAR wszSpace[] =	{W_t(' '), W_t(0)};
const WCHAR wszLeftParen[] ={W_t('('), W_t(0)};
const WCHAR wszRightParen[] ={W_t(')'), W_t(0)};
const WCHAR wszDrodDotDat[] = {W_t('d'), W_t('r'), W_t('o'), W_t('d'), W_t('1'), W_t('_'), W_t('5'), W_t('.'), W_t('d'), W_t('a'), W_t('t'), W_t(0)};
#ifdef WIN32
const WCHAR wszSlash[] ={W_t('\\'), W_t(0)};
#else
const WCHAR wszSlash[] ={W_t('/'), W_t(0)};
#endif

//*****************************************************************************
void AsciiToUnicode(const char *psz, WSTRING &wstr)
{
	const UINT MAXLEN_CONVERT = 1024;
	static WCHAR wczConvertBuffer[MAXLEN_CONVERT + 1];
	ASSERT(strlen(psz) <= MAXLEN_CONVERT);

	const char *pszRead = psz;
	WCHAR *pwczWrite = wczConvertBuffer;
	while (*pszRead != '\0')
	{
      pWCv(pwczWrite++) = static_cast<WCHAR_t>(*(pszRead++));
	}
	pWCv(pwczWrite) = 0;

	wstr = wczConvertBuffer;
}

//*****************************************************************************
void UnicodeToAscii(const WSTRING& wstr, char *psz)
{
	char* pszWrite = psz;
	for (WSTRING::const_iterator i = wstr.begin(); i != wstr.end(); i++)
	{
		*(pszWrite++) = (char)pWCv(i);
	}
	*(pszWrite++) = 0;
}

//*****************************************************************************
void UnicodeToAscii(const WSTRING& wstr, string &str)
{
    str.reserve(wstr.size());
	for (WSTRING::const_iterator i = wstr.begin(); i != wstr.end(); i++)
        str += (char)pWCv(i);
}

//*****************************************************************************
bool charFilenameSafe(const WCHAR wc)
//Returns: whether char can be used in filenames
//
//ASSUME: alphanumeric and space are usable
{
   const WCHAR lwc = towlower(wc);
   return
      (lwc >= 'a' && lwc <= 'z') ||
      (lwc >= '0' && lwc <= '9') ||
      lwc == ' ';
}

//*****************************************************************************
WSTRING filenameFilter(const WSTRING &wstr)
//Returns: input string with all characters, unusable in filenames, removed
{
   WSTRING filteredStr;
	for (WSTRING::const_iterator i = wstr.begin(); i != wstr.end(); i++)
	{
		const WCHAR wc = (WCHAR)*i;
      if (charFilenameSafe(wc))
         filteredStr += wc;
   }
   return filteredStr;
}

//*****************************************************************************
WSTRING filterFirstLettersAndNumbers(const WSTRING &wstr)
//Returns: string with only the first letter from each word, and digits
{
   WSTRING filteredStr;
   bool bAfterWhiteSpace = true;
	for (WSTRING::const_iterator i = wstr.begin(); i != wstr.end(); i++)
	{
		const WCHAR wc = (WCHAR)*i;
      if (iswspace(wc))
         bAfterWhiteSpace = true;
      else if ((bAfterWhiteSpace && iswalpha(wc)) || iswdigit(wc))
      {
         filteredStr += wc;
         bAfterWhiteSpace = false;
      }
   }
   return filteredStr;
}

//*****************************************************************************
WSTRING filterUpperCase(const WSTRING &wstr)
//Returns: string with only upper-case characters
{
   WSTRING filteredStr;
	for (WSTRING::const_iterator i = wstr.begin(); i != wstr.end(); i++)
	{
		const WCHAR wc = (WCHAR)*i;
      if (iswupper(wc))
         filteredStr += wc;
   }
   return filteredStr;
}

//*****************************************************************************
int _Wtoi(const WCHAR* wsz)
{
	static char buffer[33];
	UnicodeToAscii(wsz, buffer);
	return atoi(buffer);
}

//*****************************************************************************
WCHAR* _itoW(int value, WCHAR* wcz, int radix)
{
	char buffer[66];
	int i = 0;
	do {
		int val = value % radix;
		if (val < 10) buffer[i++] = (value % radix) + '0';
		else buffer[i++] = (value % radix) - 10 + 'A';
	} while ((value /= radix) > 0);

	buffer[i] = '\0';

	char c;
	int j;
	for (i = 0, j = strlen(buffer)-1; i<j; i++, j--) {
		c = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = c;
	}
	WSTRING wStr;
	AsciiToUnicode(buffer, wStr);
	WCScpy(wcz, wStr.c_str());
	return wcz;
}

//*****************************************************************************
WCHAR* _ltoW(long value, WCHAR* wcz, int radix)
{
	char buffer[66];
	int i = 0;
	do {
		int val = value % radix;
		if (val < 10) buffer[i++] = static_cast<char>((value % radix) + '0');
		else buffer[i++] = static_cast<char>((value % radix) - 10 + 'A');
	} while ((value /= radix) > 0);

	buffer[i] = '\0';

	char c;
	int j;
	for (i = 0, j = strlen(buffer)-1; i<j; ++i, --j) {
		c = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = c;
	}
	WSTRING wStr;
	AsciiToUnicode(buffer, wStr);
	WCScpy(wcz, wStr.c_str());
	return wcz;
}

//*****************************************************************************
UINT WCSlen(const WCHAR* wsz)
{
	UINT length = 0;
	const WCHAR* wszRead = wsz;
	while (*wszRead++ != 0)
		++length;

	return length;
}

//*****************************************************************************
WCHAR* WCScpy(WCHAR* pwczDest, const WCHAR* pwczSrc)
{
	UINT index = 0;
	while (pwczSrc[index] != 0)
		++index;

	memcpy(pwczDest, pwczSrc, (index+1) * sizeof(WCHAR));
	return pwczDest;
}

//*****************************************************************************
WCHAR* WCSncpy(WCHAR* pwczDest, const WCHAR* pwczSrc, UINT n)
{
	WCHAR *pwczWrite = pwczDest;
	const WCHAR *pwczRead = pwczSrc;

	while ((*pwczRead != 0) && n--)
	{
		*(pwczWrite++) = *(pwczRead++);
	}
	pWCv(pwczWrite) = 0;
	return pwczDest;
}

//*****************************************************************************
WCHAR* WCScat(WCHAR *pwcz1, const WCHAR *pwcz2)
{
	WCHAR *pwczWrite = pwcz1;
	while (*pwczWrite != 0)
		++pwczWrite;

	const WCHAR *pwczRead = pwcz2;
	while (*pwczRead != 0)
		*(pwczWrite++) = *(pwczRead++);
	pWCv(pwczWrite) = 0;

	return pwcz1;
}

//*****************************************************************************
int WCScmp(const WCHAR* pwcz1, const WCHAR* pwcz2)
{
	const UINT len1 = WCSlen(pwcz1);
	const UINT len2 = WCSlen(pwcz2);
	UINT index = 0;

	while (index < len1 && index < len2) {
		if (pwcz1[index] < pwcz2[index]) return -1;
		if (pwcz2[index] < pwcz1[index]) return 1;
		++index;
	}
	// If we get to this point, either they're the same or one string
	// is longer than the other.
	if (len1 == len2) return 0;
	if (len1 < len2) return -1;
   return 1;
}

//*****************************************************************************
int WCSicmp(const WCHAR* pwcz1, const WCHAR* pwcz2)
{
	const UINT len1 = WCSlen(pwcz1);
	const UINT len2 = WCSlen(pwcz2);
	UINT index = 0;

	while (index < len1 && index < len2) {
		if (towlower(pwcz1[index]) < towlower(pwcz2[index])) return -1;
		if (towlower(pwcz2[index]) < towlower(pwcz1[index])) return 1;
		++index;
	}
	// If we get to this point, either they're the same or one string
	// is longer than the other.
	if (len1 == len2) return 0;
	if (len1 < len2) return -1;
   return 1;
}

//*****************************************************************************
int WCSncmp(const WCHAR* pwcz1, const WCHAR* pwcz2, const UINT n)
{
	UINT len1 = 0, len2 = 0;
  while (len1 < n && pwcz1[len1] != 0) ++len1;
  while (len2 < n && pwcz2[len2] != 0) ++len2;
	UINT index = 0;

	while (index < len1 && index < len2 && index < n) {
		if (pwcz1[index] < pwcz2[index]) return -1;
		if (pwcz2[index] < pwcz1[index]) return 1;
		++index;
	}
	// If we get to this point, either n chars have been matched,
	// or they're the same, or one string is longer than the other.
	if (index == n) return 0;
	if (len1 == len2) return 0;
	if (len1 < len2) return -1;
   return 1;
}

WCHAR* WCStok(WCHAR *wszStart, const WCHAR *wszDelim)
{
	static WCHAR *wszNext = NULL;

	if ((NULL == wszStart) && (NULL == (wszStart = wszNext)))
		return NULL;

	UINT dindex;

	// Skip initial delimiters
	for (; pWCv(wszStart); ++wszStart)
	{
		for (dindex = 0;
				wszDelim[dindex] && (*wszStart != wszDelim[dindex]);
				++dindex);

		if (!wszDelim[dindex]) break;  // No more initial delimiters
	}

	// Now hunt for next delimiters
	for (wszNext = wszStart; pWCv(wszNext); ++wszNext)
	{
		for (dindex = 0;
				wszDelim[dindex] && (*wszNext != wszDelim[dindex]);
				++dindex);

		if (!wszDelim[dindex]) continue;

		// Found a delimiter: Terminate token and prepare for next (if any)
		pWCv(wszNext) = 0;
		wszNext = (pWCv(++wszNext) ? wszNext : NULL);
		return wszStart;
	}

	// No delimiters, end of string
	wszNext = NULL;
	return wszStart;
}

//*****************************************************************************
WCHAR* fgetWs(WCHAR* pwcz, int n, FILE* pFile)
{
	ASSERT(pwcz != NULL);

	WCHAR c;
	WCHAR* wszWrite = pwcz;
	int numread;
	do {
		numread = fread( &c, sizeof(WCHAR), 1, pFile );
		if (numread == 0)
		{
			//Reached end of file data.
			pWCv(wszWrite++) = '\0';	//null terminate string
			break;
		}
		*(wszWrite++) = c;
		--n;
	} while (c != 0 && n > 0);

	return pwcz;
}

// $Log: Wchar.cpp,v $
// Revision 1.13  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.12  2003/10/06 02:39:13  erikh2000
// Added overload for Unicode-to-ascii conversion function that take string param.CVS: ----------------------------------------------------------------------
//
// Revision 1.11  2003/07/31 21:30:26  mrimer
// Fixed a bug.
//
// Revision 1.10  2003/07/19 02:25:06  mrimer
// Added to a comment.
//
// Revision 1.9  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.8  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/07/09 11:57:46  schik
// Fixed a VS.NET warning
//
// Revision 1.6  2003/07/03 21:37:22  mrimer
// Added WCStok (committed on behalf of Gerry JJ) and two methods for string modifying.
//
// Revision 1.5  2003/06/16 20:15:22  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.4  2003/06/16 18:44:37  mrimer
// Fixed some bugs.
//
// Revision 1.3  2003/06/12 21:41:09  mrimer
// Added methods to filter file name chars.
//
// Revision 1.2  2003/05/23 21:10:57  mrimer
// Added port to APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.6  2003/05/08 23:23:30  erikh2000
// Fixed error in WCSncmp that occurs when one of the strings is not null-terminated.
//
// Revision 1.5  2003/04/29 11:05:05  mrimer
// Added WSCncmp().  Refined some code.
// Added license header and message log.
//

