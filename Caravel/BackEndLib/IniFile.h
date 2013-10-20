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
 * Matt Schikore (Schik)
 *
 * ***** END LICENSE BLOCK ***** */

//IniFile
//Declarations for CIniFile.
//Class for accessing and modifying .ini files.

#ifndef INIFILE_H
#define INIFILE_H
#ifdef WIN32
#	pragma warning(disable:4786)
#endif

#include <string>
#include <map>

#include "Wchar.h"

using namespace std;

//******************************************************************************************
class CIniSection
{
public:
	CIniSection(const char *pszSetName);
	CIniSection(const CIniSection& copy);
	CIniSection();
	~CIniSection();

	bool GetString(const char *pszKey, const char *pszDefault, string &strBuffer);
	void WriteString(const char *pszKey, const char *pszValue);

	void Save(FILE* pFile);

private:
	string strName;
	map<string, string> entries;
};

//******************************************************************************************
class CIniFile
{
public:

	CIniFile();
	~CIniFile();

	bool Load(const WCHAR *wszSetFilename);
	
	//Use this method to access data in the INI file
	bool		GetString(const char *pszSection, const char *pszKey,
			const char *pszDefault, string &strBuffer);
	
	//Use this method to add/modify data in the INI file
	void		WriteString(const char *pszSection, const char *pszKey,
			const char *pszValue);

private:
	WSTRING wstrFilename;
	bool  bLoaded;
   bool  bDirty;
	map <string, CIniSection> sections;
};

#endif //...#ifndef INIFILE_H

// $Log: IniFile.h,v $
// Revision 1.3  2003/09/11 02:02:25  mrimer
// Linux fixes (committed on behlaf of Gerry JJ).
//
// Revision 1.2  2003/05/26 01:41:10  erikh2000
// INI-related functions now return values in STD strings, to avoid prealloc requirement.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.1  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
//

