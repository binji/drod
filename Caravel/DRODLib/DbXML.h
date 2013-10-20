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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DbXML.h
//Declarations for CDbXML.
//Interface for exporting/importing database data to/from disk.

#ifndef DBXML_H
#define DBXML_H

#include "Db.h"
#include "../Texts/MIDs.h"

#include "expat.h"
#ifdef WIN32
#  pragma comment(lib,"libexpat.lib")
#endif

#include <vector>

//*****************************************************************************
class CDbXML : public CDb
{
public:
	//Importing and exporting methods.
   static void CleanUp();
	static MESSAGE_ID	ImportXML(const WCHAR *pszFilename);
	static MESSAGE_ID	ImportXML(char *buf, const ULONG size);
	static bool	ExportXML(const char *pszTableName,
			c4_IntProp &propID, const DWORD dwPrimaryKey,
			const WCHAR *pszFilename);

   static bool WasImportSuccessful();

   static UINT GetLevelStartIDs(const DWORD dwHoldID, const DWORD dwMapHoldID,
         CImportInfo &info);

   static CImportInfo info;

private:
   static bool ContinueImport(const MESSAGE_ID status = MID_ImportSuccessful);

	static CDbBase * GetNewRecord(const VIEWTYPE vType);

	static VIEWTYPE ParseViewType(const char *str);
	static VIEWPROPTYPE ParseViewpropType(const char *str);
	static PROPTYPE ParsePropType(const char *str);

	static void StartElement(void *userData, const char *name, const char **atts);
	static void InElement(void *userData, const XML_Char *s, int len);
	static void EndElement(void *userData, const char *name);
	static void UpdateLocalIDs();
   static void UpdateHighlightDemoIDs(CDbPlayer *pPlayer);

   static void CreateLevelStartSaves(CImportInfo &info);
   static void ModifyLevelStartSaves(CImportInfo &info);

   static vector <CDbBase*> dbRecordStack;	//stack of records being parsed
	static vector <CDbBase*> dbImportedRecords;	//imported records
	static vector <VIEWTYPE> dbRecordTypes;	//record types
	static vector <VIEWPROPTYPE> vpCurrentType;	//stack of viewprops being parsed
	static vector <bool>  SaveRecord;	//whether record should be saved to the DB
};

#endif //...#ifndef DBMXL_H

// $Log: DbXML.h,v $
// Revision 1.10  2003/08/01 17:26:05  mrimer
// Fixed bug: can't confirm user import choices.  Added ClearUp().
//
// Revision 1.9  2003/07/29 13:34:01  mrimer
// Added player import fix: UpdateHighlightDemoIDs().
//
// Revision 1.8  2003/07/25 00:05:10  mrimer
// Added a parameter to GetLevelStartIDs() to fix it.
//
// Revision 1.7  2003/07/13 06:45:58  mrimer
// Removed unused parameter from Export().
//
// Revision 1.6  2003/07/09 21:27:42  mrimer
// Revised import routines to be more robust: added GetLevelStartIDs(), CreateLevelStartSaves(), ModifyLevelStartSaves(), and ContinueImport().
// Attemps to repair corrupted hold records.
// Optimized import for multiple pass parsing.
// Renamed "PrimaryKeyMaps Maps" to "CImportInfo info".
//
// Revision 1.5  2003/07/07 23:33:05  mrimer
// Added more import diagnostic messages and logic.
//
// Revision 1.4  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.2  2003/01/09 22:46:14  erikh2000
// Changed absolute expat file references to relative.  Add include and lib directories in your VC options.
//
// Revision 1.1  2002/12/22 01:47:46  mrimer
// Initial check-in.
//
