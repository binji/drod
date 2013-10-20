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

// Apple and Linux keep isspace() in ctype.h.
#if defined __APPLE__ || defined __linux__
#  include <ctype.h>
#endif

#include "Assert.h"
#include "IniFile.h"
#include "Files.h"

//******************************************************************************
CIniSection::CIniSection(
//Constructor.
//
//Params:
  const char *pszSetName)	//(in)
{
	ASSERT(pszSetName != NULL);
	strName = pszSetName;
}

//******************************************************************************
CIniSection::CIniSection(
//Copy constructor.
//
//Params:
const CIniSection& copy )  //(in)
{
	strName = copy.strName;
	entries = copy.entries;
}

//******************************************************************************
CIniSection::CIniSection()
//Default constructor.
{
}

//******************************************************************************
bool CIniSection::GetString(
//Gets a string from this section
//
//Params:
  const char *pszKey,		//(in)
  const char *pszDefault,	//(in)
  string &strBuffer)		//(out)
//
//Returns:
//True if the string exists, false if not
{
	map<string,string>::iterator iter = entries.find(pszKey);
	if (iter == entries.end())
	{
      strBuffer = pszDefault;
      return false;
	}
   strBuffer = iter->second.c_str();
	return true;
}

//******************************************************************************
void CIniSection::WriteString(
//Writes a string to this section
//
//Params:
  const char *pszKey,	//(in)
  const char *pszValue)	//(in)
{
	entries[(string)pszKey] = (string)pszValue;
}

//******************************************************************************
void CIniSection::Save(
//Saves the section to the specified (already opened) file
//
//Params:
  FILE* pFile)			//(in)
{
	fprintf( pFile, "[%s]\n", strName.c_str() );

	for (map<string,string>::iterator i = entries.begin(); i != entries.end(); ++i)
	{
		fprintf( pFile, "%s=%s\n", i->first.c_str(), i->second.c_str());
	}
	fprintf(pFile, "\n");
}

//******************************************************************************
CIniSection::~CIniSection()
//Destructor.
{
}


//******************************************************************************
CIniFile::CIniFile()
//Constructor.
   : bLoaded(false)
   , bDirty(false)
{
}


//******************************************************************************
CIniFile::~CIniFile()
//Destructor.
{
	if (this->bLoaded && this->bDirty)
	{
		FILE* pFile = CFiles::Open(wstrFilename.c_str(), "w");
		if (pFile != NULL)
		{
			for (map<string, CIniSection>::iterator i = sections.begin();
					i != sections.end(); ++i)
			{
				i->second.Save(pFile);
			}
			fclose(pFile);
		}
	}
}

//******************************************************************************
bool CIniFile::Load(
//Loads the INI file 
//
//Params:
  const WCHAR *wszSetFilename) //(in)
{
	char buffer[MAX_PROFILE_STRING*2+2];
	wstrFilename = wszSetFilename;

   this->bLoaded = true;
	this->bDirty = false;

	FILE* pFile = CFiles::Open(wstrFilename.c_str(), "r");
	if (NULL == pFile) return false;

	string curSection;
	while (true)
   {
		fgets(buffer, (MAX_PROFILE_STRING+1)*2+1, pFile);
		if (feof(pFile)) break;
		if (buffer[0] == '[')
		{
			string sectionName(&buffer[1]);
			unsigned int end = sectionName.find_first_of(']');
			if (end != string::npos)
			{
				sectionName.erase(end);
				curSection = sectionName;
			}
			else
			{
				// Can't find the closing ] in a section name
				return false;
			}
		}
		else
		{
			string line(buffer);
			bool bIsWhitespace = true;
			for (string::iterator c = line.begin(); c != line.end(); ++c)
			{
				if (!isspace(*c))
				{
					bIsWhitespace = false;
					break;
				}
			}
			if (bIsWhitespace) continue;

			// It's a key/value pair, not a section name
			unsigned int equalsPos = line.find_first_of('=');
			if (equalsPos == string::npos)
			{
				// No = in the line
				return false;
			}
			else
			{
				string key = line.substr(0, equalsPos);
				string value = line.substr(equalsPos+1);
				string::iterator last = value.end();
				last--;
				value.erase(last);

				// Now add the pair to the correct section.
				// See if the section already exists.
				map<string, CIniSection>::iterator sec;
				sec = sections.find(curSection);
				if (sec != sections.end())
				{
					// Section exists
					sec->second.WriteString(key.c_str(), value.c_str());
				}
				else
				{
					// Section doesn't exist - make a new one
					CIniSection newSection(curSection.c_str());
					newSection.WriteString(key.c_str(), value.c_str());
					sections.insert(pair<string,CIniSection>(curSection, newSection));
//						[curSection] = newSection;
				}
			}
		}
	}
	return true;
}

//******************************************************************************
bool CIniFile::GetString(
//Gets the value of the specified section/key
//
//Params:
  const char *pszSection, 	//(in)
  const char *pszKey, 		//(in)
  const char *pszDefault, 	//(in)
  string &strBuffer) 		//(out)
//
//Returns:
//True if the section/key exists, false if not
{
	map<string, CIniSection>::iterator sec = sections.find(pszSection);
	if (sec == sections.end())
	{
      // section does not exist
      strBuffer = pszDefault;
      return false;
	}
	return sec->second.GetString(pszKey, pszDefault, strBuffer);
}

//******************************************************************************
void CIniFile::WriteString(
//Writes a key/value to a section
//
//Params:
  const char *pszSection,	//(in)
  const char *pszKey,		//(in)
  const char *pszValue)		//(in)
{
	ASSERT(this->bLoaded);
	this->bDirty = true;

	map<string, CIniSection>::iterator sec = sections.find(pszSection);
	if (sec == sections.end())
	{
		// section does not exist yet, so create it.
		CIniSection newSec(pszSection);
		newSec.WriteString(pszKey, pszValue);
//		sections.insert(pair<string,CIniSection>(pszSection, newSec>));
		sections[pszSection] = newSec;
	}
	else
	{
		sec->second.WriteString(pszKey, pszValue);
	}
}

// $Log: IniFile.cpp,v $
// Revision 1.6  2003/09/11 02:02:25  mrimer
// Linux fixes (committed on behlaf of Gerry JJ).
//
// Revision 1.5  2003/06/16 20:15:22  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.4  2003/06/15 04:19:15  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.3  2003/05/26 01:41:10  erikh2000
// INI-related functions now return values in STD strings, to avoid prealloc requirement.
//
// Revision 1.2  2003/05/23 21:10:57  mrimer
// Added port to APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.3  2003/05/20 18:12:27  mrimer
// Added log comment.
//
