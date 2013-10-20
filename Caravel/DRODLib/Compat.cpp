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
 *
 * ***** END LICENSE BLOCK ***** */

#include "Compat.h"

//*******************************************************************************************************
bool CCompat::ExportXML(
//Exports database(s) at a specified location to XML files.  Databases must be stored in current 
//format version.
//
//Params:
//NOTE: Parameter names are commented out to suppress unused parameter warnings.
	const char * /*pszFromPath*/,	//(in)	Databases from which to export.
	const char * /*pszToPath*/,		//(in)	Location to write XML files.
	DWORD /*dwFlags*/,				//(in)	Indicates which objects to export.
	string &/*strResultText*/)		//(out)	Receives text to describe results.
//
//Returns:
//True if successful, false if not.
{
	//!!todo
	return false;
}

//*******************************************************************************************************
bool CCompat::ImportXML(
//Imports data stored in XML files at a specified location.  XML files must be in previous format
//version.
//
//Params:
//NOTE: Parameter names are commented out to suppress unused parameter warnings.
	const char * /*pszFromPath*/,	//(in)	Location from which to read XML files.
	const char * /*pszToPath*/,		//(in)	Databases that will receive import.  Must be current version.
	DWORD /*dwFlags*/,				//(in)	Indicates which objects to import.
	string &/*strResultText*/)		//(out)	Receives text to describe results.
//
//Returns:
//True if successful, false if not.
{
	//!!todo
	return false;
}

//*******************************************************************************************************
bool CCompat::CreateData(
//Create databases.
//
//Params:
//NOTE: Parameter names are commented out to suppress unused parameter warnings.
	const char * /*pszPath*/,		//(in)	Where to create databases.
	string &/*strResultText*/)		//(out)	Receives text to describe results.
//
//Returns:
//True if successful, false if not.
{
	//!!todo
	return false;
}

//*******************************************************************************************************
bool CCompat::DeleteData(
//Delete databases.
//
//Params:
//NOTE: Parameter names are commented out to suppress unused parameter warnings.
	const char * /*pszPath*/,		//(in)	Where to delete databases.
	string &/*strResultText*/)		//(out)	Receives text to describe results.
//
//Returns:
//True if successful, false if not.
{
	//!!todo
	return false;
}

// $Log: Compat.cpp,v $
// Revision 1.3  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/09 02:42:37  mrimer
// Added license block and log comment.
//
