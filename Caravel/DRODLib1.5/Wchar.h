#ifndef WCHAR_H
#define WCHAR_H

#include <string>
using namespace std;

typedef unsigned short	WCHAR;    //wc, 16-bit UNICODE character
typedef basic_string<WCHAR, char_traits<WCHAR>, allocator<WCHAR> > WSTRING;

//Some common small strings.  Larger strings are localizable and should be kept in database.
const WCHAR wszAsterisk[] =	{'*', 0};
const WCHAR wszColon[] =	{':', 0};
const WCHAR wszCRLF[] =		{'\r', '\n', 0};
const WCHAR wszElipsis[] =	{'.', '.', '.', 0};
const WCHAR wszEmpty[] =	{0};
const WCHAR wszPeriod[] =	{'.', 0};
const WCHAR wszQuote[] =	{'\"', 0};
const WCHAR wszSpace[] =	{' ', 0};
const WCHAR wszX[] =		{'x', 0};

void AsciiToUnicode(const char *psz, WSTRING &wstr);

#endif