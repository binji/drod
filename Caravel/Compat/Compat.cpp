//Compat.cpp
//
//SUMMARY
//
//This project builds a compatibility DLL which can be used to access a previous version of the DRODLib
//in a project which may have the current version of DRODLib statically linked to it.  Having old DRODLib
//code inside of a DLL allows more than one version of DRODLib to be accessed at one time because namespace
//conflicts are avoided.  This DLL defines a consistent interface to access a subset of DRODLib.
//
//USAGE
//
//To set up a project for a new compatibility DLL, perform the following steps:
//1.	Find a version of the DRODLib project which contains the functionality you want.  You may need
//		to use CVS to retrieve an earlier version.
//2.	Copy this version to a new directory under "Caravel".  Call the directory "DRODLib" suffixed with 
//		the version of the data that the library accesses, i.e. "DRODLib1_6".
//3.	Rename DRODLib.dsp to the suffixed name, i.e. "DRODLib1_6.dsp".
//4.	Edit the .DSP of the DRODLib project with notepad.  Search and replace "DRODLib" to "DRODLib" 
//		suffixed with the version.
//5.	Copy Compat.cpp and Compat.dsp, but not CompatAPI.h, to a new directory under "Caravel".  Call
//      the directory "Compat" suffixed by the version.
//6.	Rename Compat.dsp so that it is suffixed by the version.
//7.	Edit Compat.dsp with notepad.  Search and replace "Compat" to "Compat" suffixed with the version.
//8.	Open the copy of Compat.cpp and edit the '#include "../DRODLib/Compat.h"' line to specify the
//		suffixed directory, i.e. '#include "../DRODLib1_6/Compat.h"'.
//9.	Add your two new projects to the master workspace.
//10.	Change the library dependency in the new Compat project to point to the new DRODLib project.
//11.	When you build the new Compat project, it should make a compatibility DLL.

//Purposefully, the #include below goes down and back up in the directory.  This is so that one
//file will specify the interface for all DLLs.  If your DLL build breaks, either:
//1. Change your mismatched definition in CompatXXX.cpp, or...
//2. Change CompatAPI.h to match your definition, and fix any other CompatXXX.cpp files to also match.
#include "../Compat/CompatAPI.h"

//Change this line in copied project per above instructions.
#include "../DRODLib/Compat.h"

//****************************************************************************************************
_DWORD DECLSPEC GetVersion(void)
//Get version of this compatibility DLL.  This indicates what version of data the DLL can access.
{
	return CCompat::GetVersion();
}

//****************************************************************************************************
bool DECLSPEC ImportXML(
//Imports data stored in XML files at a specified location.  XML files must be in previous format
//version.
//
//Params:
	const char *pszFromPath,	//(in)	Location from which to read XML files.
	const char *pszToPath,		//(in)	Databases that will receive import.  Must be current version.
	_DWORD dwFlags)				//(in)	Indicates which objects to import.
//
//Returns:
//True if successful, false if not.
{
	return CCompat::ImportXML(pszFromPath, pszToPath, dwFlags);
}

//****************************************************************************************************
bool DECLSPEC ExportXML(
//Exports database(s) at a specified location to XML files.  Databases must be stored in current 
//format version.
//
//Params:
	const char *pszFromPath,	//(in)	Databases from which to export.
	const char *pszToPath,		//(in)	Location to write XML files.
	_DWORD dwFlags)				//(in)	Indicates which objects to export.
//
//Returns:
//True if successful, false if not.
{
	return CCompat::ExportXML(pszFromPath, pszToPath, dwFlags);
}
