//Compat.h
//
//SUMMARY
//
//Class for accessing DRODLib features when the library is built inside of a compatibility.dll.

#ifndef COMPAT_H
#define COMPAT_H

#include "Types.h"
#include "GameConstants.h"

//Import/export flags.
#define IX_PLAYERS	(1)
#define IX_TEXTS	(2)
#define IX_HOLDS	(4)
#define IX_ALL		(IX_PLAYERS + IX_TEXTS + IX_HOLDS)

class CCompat
{
public:
	static DWORD	GetVersion(void) {return dwCurrentDRODVersion;}
	static bool		ImportXML(const char *pszFromPath, const char *pszToPath, DWORD dwFlags);
	static bool		ExportXML(const char *pszFromPath, const char *pszToPath, DWORD dwFlags);
};

#endif //...#ifndef COMPAT_H
