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
//Top-level class for accessing DROD data from database.

#ifndef DBXML_H
#define DBXML_H

#include "Db.h"

#include "expat.h"
#pragma comment(lib,"libexpat.lib")


#include <vector>

//*****************************************************************************
class CDbXML : public CDb
{
public:
	//Importing and exporting methods.
	static MESSAGE_ID	ImportXML(const char *pszFilename);
	static bool	ExportXML(const char *pszTableName,
			c4_IntProp &propID, const DWORD dwPrimaryKey,
			const char *pszFilename, const bool bAppend = false);

private:
	static CDbBase * GetNewRecord(const VIEWTYPE vType);

	static VIEWTYPE ParseViewType(const char *str);
	static VIEWPROPTYPE ParseViewpropType(const char *str);
	static PROPTYPE ParsePropType(const char *str);

	static void StartElement(void *userData, const char *name, const char **atts);
	static void InElement(void *userData, const XML_Char *s, int len);
	static void EndElement(void *userData, const char *name);
	static void UpdateLocalIDs();

	static vector <CDbBase*> dbRecordStack;	//stack of records being parsed
	static vector <CDbBase*> dbImportedRecords;	//imported records
	static vector <VIEWTYPE> dbRecordTypes;	//record types
	static vector <VIEWPROPTYPE> vpCurrentType;	//stack of viewprops being parsed
	static MESSAGE_ID ImportStatus;		//whether data were successfully imported
	static vector <bool>  SaveRecord;	//whether record should be saved to the DB
};

#endif //...#ifndef DBMXL_H

// $Log: DbXML.h,v $
// Revision 1.1  2003/02/25 00:01:33  erikh2000
// Initial check-in.
//
// Revision 1.2  2003/01/09 22:46:14  erikh2000
// Changed absolute expat file references to relative.  Add include and lib directories in your VC options.
//
// Revision 1.1  2002/12/22 01:47:46  mrimer
// Initial check-in.
//
