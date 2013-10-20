#include "Wchar.h"
#include "Assert.h"
#include "Types.h"

//******************************************************************************************************
void AsciiToUnicode(const char *psz, WSTRING &wstr)
{
	const UINT MAXLEN_CONVERT = 512;
	static WCHAR wczConvertBuffer[MAXLEN_CONVERT + 1];
	ASSERT(strlen(psz) <= MAXLEN_CONVERT);

	const char *pszRead = psz;
	WCHAR *pwczWrite = wczConvertBuffer;
	while (*pszRead != '\0')
		*(pwczWrite++) = (WCHAR) *(pszRead++);
	*pwczWrite = 0;

	wstr = wczConvertBuffer;
}
