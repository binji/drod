#ifndef COMPATAPI_H
#define COMPATAPI_H

//Define any types needed here.  Prefix with _* to avoid namespace collision.  This project doesn't
//get to make many assumptions about what will be included.
typedef unsigned long _DWORD;

//Add for other O/Ss as needed.
#ifndef DECLSPEC
#	ifdef WIN32
#		define DECLSPEC	__declspec(dllexport)
#	else
#		define DECLSPEC
#	endif
#endif

//The API--changes can have farflung consequences.
_DWORD DECLSPEC	GetVersion(void);
bool DECLSPEC	ExportXML(const char *pszFromPath, const char *pszToPath, _DWORD dwFlags);
bool DECLSPEC	ImportXML(const char *pszFromPath, const char *pszToPath, _DWORD dwFlags);

#endif //...#ifndef COMPATAPI_H
