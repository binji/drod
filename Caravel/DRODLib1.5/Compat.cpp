#include "Compat.h"

//*******************************************************************************************************
bool CCompat::ExportXML(
//Exports database(s) at a specified location to XML files.  Databases must be stored in current 
//format version.
//
//Params:
	const char *pszFromPath,	//(in)	Databases from which to export.
	const char *pszToPath,		//(in)	Location to write XML files.
	DWORD dwFlags)				//(in)	Indicates which objects to export.
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
	const char *pszFromPath,	//(in)	Location from which to read XML files.
	const char *pszToPath,		//(in)	Databases that will receive import.  Must be current version.
	DWORD dwFlags)				//(in)	Indicates which objects to import.
//
//Returns:
//True if successful, false if not.
{
	//!!todo
	return false;
}
