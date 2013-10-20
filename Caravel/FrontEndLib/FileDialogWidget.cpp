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
 * Portions created by the Initial Developer are Copyright (C) 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */
#ifdef WIN32
#	include <windows.h> //Should be first include.
#endif

#include "FileDialogWidget.h"
#include "BitmapManager.h"
#include "ButtonWidget.h"
#include "FrameWidget.h"
#include "LabelWidget.h"
#include "ListBoxWidget.h"
#include "OptionButtonWidget.h"
#include "TextBoxWidget.h"

#include "IncludeLib.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

#include <set>
#include <fstream>

#ifdef WIN32
#include <io.h>
#include <direct.h>	//for _chdrive
#else
#include <stdio.h>
#include <dirent.h>
#endif
#include <sys/stat.h>

#ifdef __linux__
#include <unistd.h> //for chdir
#endif

//File extension filters.
const WCHAR fileExtension[EXT_COUNT][7] = {
	{W_t('p'),W_t('l'),W_t('a'),W_t('y'),W_t('e'),W_t('r'),W_t(0)},
   {W_t('h'),W_t('o'),W_t('l'),W_t('d'),W_t(0)},
   {W_t('d'),W_t('e'),W_t('m'),W_t('o'),W_t(0)}
};
const WCHAR fileExtensionDesc[EXT_COUNT][30] = {
   {W_t('D'),W_t('R'),W_t('O'),W_t('D'),W_t(' '),W_t('p'),W_t('l'),W_t('a'),W_t('y'),W_t('e'),W_t('r'),W_t(' '),W_t('f'),W_t('i'),W_t('l'),W_t('e'),W_t('s'),W_t(' '),W_t('('),W_t('.'),W_t('p'),W_t('l'),W_t('a'),W_t('y'),W_t('e'),W_t('r'),W_t(')'),W_t(0)},
	{W_t('D'),W_t('R'),W_t('O'),W_t('D'),W_t(' '),W_t('h'),W_t('o'),W_t('l'),W_t('d'),W_t(' '),W_t('f'),W_t('i'),W_t('l'),W_t('e'),W_t('s'),W_t(' '),W_t('('),W_t('.'),W_t('h'),W_t('o'),W_t('l'),W_t('d'),W_t(')'),W_t(0)},
	{W_t('D'),W_t('R'),W_t('O'),W_t('D'),W_t(' '),W_t('d'),W_t('e'),W_t('m'),W_t('o'),W_t(' '),W_t('f'),W_t('i'),W_t('l'),W_t('e'),W_t('s'),W_t(' '),W_t('('),W_t('.'),W_t('d'),W_t('e'),W_t('m'),W_t('o'),W_t(')'),W_t(0)}
};

const DWORD TAG_DIR_LBOX = 1080;
const DWORD TAG_FILE_LBOX = 1081;
const DWORD TAG_EXTENSION_LBOX = 1082;
const DWORD TAG_DIRNAME_BOX = 1083;
const DWORD TAG_FILENAME_BOX = 1084;
const DWORD TAG_CANCEL = 1090;

#ifdef WIN32
	const UINT numDrives = 26;	//A - Z
	bool bDrives[numDrives] = {false};
   bool bDrivesChecked = false;
#endif

//*****************************************************************************
CFileDialogWidget::CFileDialogWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	const int nSetX, const int nSetY)			//		constructor.
	: CDialogWidget(dwSetTagNo, nSetX, nSetY, 300, 435)
	, pPromptLabel(NULL)
	, pDirListBoxWidget(NULL)
	, pFileListBoxWidget(NULL)
	, pExtensionListBoxWidget(NULL)
	, pDirNameTextBox(NULL), pFilenameTextBox(NULL)
	, bWriting(false)
{
   CheckDrives();
}

//*****************************************************************************
void CFileDialogWidget::CheckDrives()
//Checks which drives are available.
{
#ifdef WIN32
   if (!bDrivesChecked)
   {
      for (UINT drive=0; drive<numDrives; ++drive)	
      {
         char buffer[4] = { drive+'a', ':', '\\', 0};
         UINT uType = GetDriveTypeA(buffer); //Use instead of GetDriveType() for pre-NT compatibility.
         bDrives[drive] = ((uType != DRIVE_UNKNOWN) && (uType != DRIVE_NO_ROOT_DIR));
      }
      bDrivesChecked = true;
   }
#endif
}

//*****************************************************************************
bool CFileDialogWidget::Load()
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	const int X_LEFTEDGE = 15;
	const UINT CY_TEXTBOX = 23;

   this->pPromptLabel = new CLabelWidget(0L, X_LEFTEDGE + 20, 10, 250, 20, FONTLIB::F_Small,
			wszEmpty);
	AddWidget(this->pPromptLabel);
	AddWidget(new CFrameWidget(0L, X_LEFTEDGE, 7, 270, 25, NULL));

	//Current directory.
	AddWidget(new CLabelWidget(0L, X_LEFTEDGE, 32, 40, 40, FONTLIB::F_Small,
			g_pTheDB->GetMessageText(MID_CurrentDirectory)));
	this->pDirNameTextBox = new CTextBoxWidget(TAG_DIRNAME_BOX, 58, 35, 227,
			CY_TEXTBOX, 255, false);
	AddWidget(this->pDirNameTextBox);

	this->pDirListBoxWidget = new CListBoxWidget(TAG_DIR_LBOX,
			X_LEFTEDGE, 67, 270, 114);
	AddWidget(this->pDirListBoxWidget);

	//Current file.
	this->pFileListBoxWidget = new CListBoxWidget(TAG_FILE_LBOX,
			X_LEFTEDGE, 188, 270, 156);
	AddWidget(this->pFileListBoxWidget);

	AddWidget(new CLabelWidget(0L, X_LEFTEDGE, 348, 50, 20, FONTLIB::F_Small,
			g_pTheDB->GetMessageText(MID_FileName)));
	this->pFilenameTextBox = new CTextBoxWidget(TAG_FILENAME_BOX,
			65, 348, 220, CY_TEXTBOX, 255, false);
   this->pFilenameTextBox->SetFilenameSafe();
	AddWidget(this->pFilenameTextBox);
   SetEnterText(TAG_FILENAME_BOX);

	//Extension filter.
	AddWidget(new CLabelWidget(0L, X_LEFTEDGE, 373, 50, 20, FONTLIB::F_Small,
			g_pTheDB->GetMessageText(MID_FileType)));
	this->pExtensionListBoxWidget = new CListBoxWidget(TAG_EXTENSION_LBOX,
			X_LEFTEDGE, 393, 200, 30);
	AddWidget(this->pExtensionListBoxWidget);

	//Buttons.
	CButtonWidget *pOKButton = new CButtonWidget(
			TAG_OK, 226, 377, 60, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	AddWidget(pOKButton);
	CButtonWidget *pCancelButton = new CButtonWidget(
			TAG_CANCEL, 226, 402, 60, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Cancel));
	AddWidget(pCancelButton);

   return CWidget::Load();
}

//*****************************************************************************
void CFileDialogWidget::OnClick(
//Handles click event.
//
//Params:
	const DWORD dwTagNo)				//(in)	Widget that received event.
{
   switch (dwTagNo)
   {
      case TAG_OK:
         if (pFileListBoxWidget->GetItemCount() == 0 && !this->bWriting)
         {
            //No files to select for reading.  Select highlighted directory instead.
            GoToDirectory();
            return;
         }
      break;
   }

   CDialogWidget::OnClick(dwTagNo);
}

//*****************************************************************************
void CFileDialogWidget::OnDoubleClick(
//Called when widget receives a mouse click.
//
//Params:
	const DWORD dwTagNo)	//(in) Widget event applies to.
{
	CDialogWidget::OnDoubleClick(dwTagNo);

	switch (dwTagNo)
	{
		case TAG_DIR_LBOX:
			if (this->pDirListBoxWidget->ClickedSelection())
				GoToDirectory();
		break;

		case TAG_FILE_LBOX:
			if (this->pFileListBoxWidget->ClickedSelection())
			{
            //Selects the file.
				this->dwDeactivateValue = TAG_OK;
			   Deactivate();
			}
		break;

  		default: break;
	}
}

//*****************************************************************************
void CFileDialogWidget::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD dwTagNo,			//(in)	Widget event applies to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	CDialogWidget::OnKeyDown(dwTagNo, Key);

	switch (dwTagNo)
	{
		case TAG_DIRNAME_BOX:
			switch (Key.keysym.sym)
			{
				case SDLK_RETURN:
				case SDLK_KP_ENTER:
					SetDirectory(this->pDirNameTextBox->GetText());
				break;

            default: break;
			}
		break;
		case TAG_DIR_LBOX:
		{
			switch (Key.keysym.sym)
			{
				case SDLK_RETURN:
				case SDLK_KP_ENTER:
					GoToDirectory();
				break;

            default: break;
			}
		}
		break;

		case TAG_EXTENSION_LBOX:
			if (this->pExtensionListBoxWidget->GetItemCount() > 1)
			{
				//There might have been a change in extension.  Update file list.
				PopulateFileList();
				if (!this->bWriting) SelectFile();
				Paint();
			}
		break;

      default: break;
	}
}

//*****************************************************************************
void CFileDialogWidget::OnSelectChange(
//Handles a selection change.
//
//Params:
	const DWORD dwTagNo) //(in) Widget event applies to.
{
	switch (dwTagNo)
	{
		case TAG_FILE_LBOX:
         SelectFile();
         Paint();
      break;

		case TAG_EXTENSION_LBOX:
			PopulateFileList();
			if (!this->bWriting) SelectFile();
			Paint();
		break;
	}
}

//*****************************************************************************
void CFileDialogWidget::PopulateDirList()
{
	this->pDirListBoxWidget->Clear();

	UINT wCount = 0;
   set<WSTRING, WSTRINGicmp> wstrDirs;
#ifdef WIN32
   // If not on Windows NT or better, we need to use non-unicode paths here...
   if (!CFiles::WindowsCanBrowseUnicode()) {
	   //Show subdirs.
      char buffer[MAX_PATH+1];
      WSTRING wbuffer;
	   struct _finddata_t filedata;
	   long hFile;

	   //Find first sub-dir in current directory.
      UnicodeToAscii(this->dirpath, buffer);
	   string sFileFilter = buffer;

	   sFileFilter += "\\*";
      if ((hFile = _findfirst(sFileFilter.c_str(), &filedata )) == -1L) {
          return;	//No files in current directory.
      }
	   else
	   {
		   //Don't display the current directory (i.e., ".").
         if (filedata.attrib & _A_SUBDIR && strcmp(".", filedata.name)) {
            AsciiToUnicode(filedata.name, wbuffer);
            wstrDirs.insert(wbuffer);
         }

		   //Find the rest of the sub-dirs.
		   while (_findnext( hFile, &filedata ) == 0)
		   {
            if (filedata.attrib & _A_SUBDIR) {
               AsciiToUnicode(filedata.name, wbuffer);
               wstrDirs.insert(wbuffer);
           }
		   }

		   //Done.
		   _findclose( hFile );
	   }
      char path[MAX_PATH];
      UnicodeToAscii(this->dirpath.c_str(), path);
      UINT len = strlen(path);
      path[len] = SLASH;
      path[++len] = 0;
      for (set<WSTRING, WSTRINGicmp>::iterator wstr = wstrDirs.begin(); wstr != wstrDirs.end(); ++wstr)
      {
         UnicodeToAscii(wstr->c_str(), path + len);
         if (!_access(path, 4))
            this->pDirListBoxWidget->AddItem(++wCount, wstr->c_str());
      }

	   //List all available drives.
	   for (UINT drive=0; drive<numDrives; ++drive)
		   if (bDrives[drive])
		   {
			   static const WCHAR wszLeft[] = {'[',' ',0}, wszRight[] = {':',' ',']',0};
			   WCHAR wszDrive[] = {'A',0};
			   wszDrive[0] = (WCHAR)(drive + 'A');
			   WSTRING wstrDrive = wszLeft;
			   wstrDrive += wszDrive;
			   wstrDrive += wszRight;
			   this->pDirListBoxWidget->AddItem(++wCount, wstrDrive.c_str());
		   }
   } else {
      // We have access to Unicode file browsing

	   struct _wfinddata_t filedata;
	   long hFile;

	   //Find first sub-dir in current directory.
	   WSTRING wFileFilter = this->dirpath;
	   wFileFilter += wszSlash;
	   wFileFilter += wszAsterisk;
	   if ((hFile = _wfindfirst((WCHAR*)wFileFilter.c_str(), &filedata )) == -1L)
          return;	//No files in current directory.

      //Don't display the current directory (i.e., ".").
		if (filedata.attrib & _A_SUBDIR && WCScmp(wszPeriod, filedata.name))
         wstrDirs.insert(filedata.name);

		//Find the rest of the sub-dirs.
		while (_wfindnext( hFile, &filedata ) == 0)
		{
			if (filedata.attrib & _A_SUBDIR)
            wstrDirs.insert(filedata.name);
		}

		//Done.
		_findclose( hFile );
      for (set<WSTRING, WSTRINGicmp>::iterator wstr = wstrDirs.begin(); wstr != wstrDirs.end(); ++wstr)
      {
         WSTRING fullFilename = this->dirpath;
         fullFilename += SLASH;
         fullFilename += wstr->c_str();
         if (!_waccess(fullFilename.c_str(), 4))
            this->pDirListBoxWidget->AddItem(++wCount, wstr->c_str());
      }

	   //List all available drives.
	   for (UINT drive=0; drive<numDrives; ++drive)
		   if (bDrives[drive])
		   {
			   static const WCHAR wszLeft[] = {'[',' ',0}, wszRight[] = {':',' ',']',0};
			   WCHAR wszDrive[] = {'A',0};
			   wszDrive[0] = (WCHAR)(drive + 'A');
			   WSTRING wstrDrive = wszLeft;
			   wstrDrive += wszDrive;
			   wstrDrive += wszRight;
			   this->pDirListBoxWidget->AddItem(++wCount, wstrDrive.c_str());
		   }
   }
#else
#	ifdef HAS_UNICODE
#		error How does a non-Win32 machine get a Unicode list of directories in a directory?
#	else
   char buffer[MAX_PATH];
   DIR *pDir;
   struct dirent *pDirent;
   UnicodeToAscii(this->dirpath, buffer);
   pDir = opendir(buffer);
   UINT len = strlen(buffer);
   buffer[len] = SLASH;
   buffer[++len] = 0;
   if (pDir)
   {
      for (; (pDirent = readdir(pDir)) != NULL; buffer[len] = 0)
      {
         strcat(buffer, pDirent->d_name);
         struct stat st;
         if ((pDirent->d_name[0] != '.' || pDirent->d_name[1] != 0)
            && !stat(buffer, &st) && S_ISDIR(st.st_mode)
            && !access(buffer, R_OK | X_OK))
         {
            // The directory entry is a directory.
            WSTRING wbuffer;
            AsciiToUnicode(pDirent->d_name, wbuffer);
            wstrDirs.insert(wbuffer);
         }
      }
   }

   for (set<WSTRING, WSTRINGicmp>::iterator wstr = wstrDirs.begin(); wstr != wstrDirs.end(); wstr++) {
 		this->pDirListBoxWidget->AddItem(++wCount, wstr->c_str());
   }
#	endif

#endif
}

//*****************************************************************************
void CFileDialogWidget::PopulateFileList()
//Populate list box with all files in current directory.
{
	this->pFileListBoxWidget->Clear();

	UINT wCount = 0;
   set<WSTRING, WSTRINGicmp> wstrFiles;
#ifdef WIN32
   if (!CFiles::WindowsCanBrowseUnicode()) {
 	   //Get the files in the current directory - ASCII version
      char buffer[MAX_PATH+1];
      WSTRING wbuffer;
	   struct _finddata_t filedata;
	   long hFile;

	   //Find first sub-dir in current directory.
      UnicodeToAscii(this->dirpath, buffer);
	   string sFileFilter = buffer;

	   sFileFilter += "\\*.";
      UnicodeToAscii(fileExtension[this->pExtensionListBoxWidget->GetSelectedItem()], buffer);
	   sFileFilter += buffer;
      if ((hFile = _findfirst(sFileFilter.c_str(), &filedata)) == -1L) {
          return;	//No files in current directory.
      }
	   else
	   {
		   if (!(filedata.attrib & _A_SUBDIR || filedata.attrib & _A_SYSTEM ||
            filedata.attrib & _A_HIDDEN)) {
            AsciiToUnicode(filedata.name, wbuffer);
            wstrFiles.insert(wbuffer);
         }

		   //Find the rest of the files.
		   while (_findnext(hFile, &filedata) == 0)
		   {
			   if (!(filedata.attrib & _A_SUBDIR || filedata.attrib & _A_SYSTEM ||
               filedata.attrib & _A_HIDDEN)) {
               AsciiToUnicode(filedata.name, wbuffer);
               wstrFiles.insert(wbuffer);
            }
		   }

		   //Done.
		   _findclose( hFile );
	   }
  }
   else {
	   //Get the files in the current directory - Unicode version
	   struct _wfinddata_t filedata;
	   long hFile;

	   //Find first file in current directory.
	   WSTRING wFileFilter = this->dirpath;
	   wFileFilter += wszSlash;
	   wFileFilter += wszAsterisk;
	   wFileFilter += wszPeriod;
	   wFileFilter += fileExtension[this->pExtensionListBoxWidget->GetSelectedItem()];
	   if ((hFile = _wfindfirst((WCHAR*)wFileFilter.c_str(), &filedata)) == -1L)
          return;	//No files in current directory.
	   else
	   {
		   if (!(filedata.attrib & _A_SUBDIR || filedata.attrib & _A_SYSTEM ||
				   filedata.attrib & _A_HIDDEN))
            wstrFiles.insert(filedata.name);

		   //Find the rest of the files.
		   while (_wfindnext(hFile, &filedata) == 0)
		   {
			   if (!(filedata.attrib & _A_SUBDIR || filedata.attrib & _A_SYSTEM ||
					   filedata.attrib & _A_HIDDEN))
               wstrFiles.insert(filedata.name);
		   }

		   //Done.
		   _findclose( hFile );
	   }
   }
#else
#	ifdef HAS_UNICODE
#		error How does a non-Win32 machine get a Unicode list of files in a directory?
#	else
   char buffer[MAX_PATH];
   DIR *pDir;
   struct dirent *pDirent;
   UnicodeToAscii(this->dirpath, buffer);
   pDir = opendir(buffer);
   UINT len = strlen(buffer);
   buffer[len] = SLASH;
   buffer[++len] = 0;
   if (pDir)
   {
      WSTRING wstrExt = fileExtension[this->pExtensionListBoxWidget->GetSelectedItem()];
      char szExt[wstrExt.length() + 1];
      UnicodeToAscii(wstrExt, szExt);

      for (; (pDirent = readdir(pDir)) != NULL; buffer[len] = 0)
      {
         char *szCurExt = pDirent->d_name + strlen(pDirent->d_name) - strlen(szExt);
         if (szCurExt < pDirent->d_name || (strcmp(szExt, szCurExt)))
            continue;

         strcat(buffer, pDirent->d_name);
         struct stat st;
         if (!stat(buffer, &st) && !S_ISDIR(st.st_mode))
         {
            // The entry is a file.
            WSTRING wbuffer;
            AsciiToUnicode(pDirent->d_name, wbuffer);
            wstrFiles.insert(wbuffer);
         }
      }
   }
#	endif
#endif

   // No Drives to list here, so we just add all files at the end.
   for (set<WSTRING, WSTRINGicmp>::iterator wstr = wstrFiles.begin(); wstr != wstrFiles.end(); wstr++) {
 		this->pFileListBoxWidget->AddItem(++wCount, wstr->c_str());
   }
}

//*****************************************************************************
void CFileDialogWidget::PopulateExtensionList(
//Setup list of file extension filters.
//
//Params:
	const FileExtensionType extensionType)	//(in) Which extension to show
{
	this->pExtensionListBoxWidget->Clear();

	this->pExtensionListBoxWidget->AddItem(extensionType,
			fileExtensionDesc[extensionType]);
}

//*****************************************************************************
void CFileDialogWidget::GoToDirectory()
//Go to selected directory.
{
	const WCHAR *wDirname = this->pDirListBoxWidget->GetSelectedItemText();
	if (wDirname[0] == 0) return;	//nothing is selected
	if (!WCScmp(wszParentDir,wDirname))
	{
		//Go up a directory.
		const int nSlashLoc=this->dirpath.rfind(wszSlash);
		if (nSlashLoc<0)
			this->dirpath.resize(0);
#ifndef WIN32
      else if (nSlashLoc == 0)
         this->dirpath.resize(1); // go to root dir
#endif
		else
			this->dirpath.resize(nSlashLoc);
#ifdef WIN32
	} else if (wDirname[0] == (WCHAR)'[') {
		//Switch drives.
      wstring newDrive = wDirname + 2;
		newDrive.resize(newDrive.size()-2);
      SetDirectory(newDrive.c_str());
      return;
#endif
	} else {
		//Go down a directory.
		if (this->dirpath.c_str() &&
				(this->dirpath.c_str()[this->dirpath.length()-1] != wszSlash[0]))
			this->dirpath += wszSlash;
		this->dirpath += wDirname;
	}

	SetDirectory();
}

//*****************************************************************************
void CFileDialogWidget::SetDirectory(
//Go to selected directory.
//
//Params:
	const WCHAR *wDirPath)	//(in) default = NULL (means to use this->dirpath)
{
   //Change to new directory?
   if (wDirPath && CFiles::IsValidPath(wDirPath)) //Yes.
      this->dirpath = wDirPath;
   else if (!CFiles::IsValidPath(this->dirpath.c_str()))
   {
      // Default path is invalid! Set to dat path.
      CFiles files;
      this->dirpath = files.GetDatPath();
   }

   //Update affected widgets.
   this->pDirNameTextBox->SetText(this->dirpath.c_str());  
   PopulateDirList();
   PopulateFileList();
   if (!this->bWriting)
      SelectFile();

   //Repaint the dialog to show changes.
   Paint();
}

//*****************************************************************************
WSTRING CFileDialogWidget::GetSelectedFileName() const
//Returns: a wstring containing the full file name
{
	WSTRING filename = this->dirpath;
	filename += wszSlash;
	filename += this->pFilenameTextBox->GetText();

	//Add extension if it's missing.
	const UINT index = this->pExtensionListBoxWidget->GetSelectedItem();
   WSTRING ext = wszPeriod;
   ext += fileExtension[index];
	if (WCScmp(filename.c_str() + WCSlen(filename.c_str()) - ext.size(), ext.c_str()))
	{
		filename += wszPeriod;
		filename += fileExtension[index];
	}

	return filename;
}

//*****************************************************************************
void CFileDialogWidget::SelectFile()
//Selects the file selected in the list box.
{
	const WCHAR *wFilename = this->pFileListBoxWidget->GetSelectedItemText();
	if (WCSlen(wFilename))
		this->pFilenameTextBox->SetText(wFilename);
   CheckTextBox();
}

//*****************************************************************************
void CFileDialogWidget::SetExtension(
//Sets the extension appearing in the extension list box.
//
//Params:
	const FileExtensionType extensionType)	//(in)
{
	PopulateExtensionList(extensionType);
}

//*****************************************************************************
void CFileDialogWidget::SetFilename(
//Sets the string appearing in the filename text box.
//
//Params:
	const WCHAR *wFilename)
{
	this->pFilenameTextBox->SetText(wFilename);
}

//*****************************************************************************
void CFileDialogWidget::SetPrompt(
//Sets the file selection prompt.
//
//Params:
	const MESSAGE_ID messageID)	//(in)
{
	this->pPromptLabel->SetText(g_pTheDB->GetMessageText(messageID));
}


// $Log: FileDialogWidget.cpp,v $
// Revision 1.31  2003/08/27 18:36:17  mrimer
// Made directory selection more robust (committed on behalf of Gerry JJ).
//
// Revision 1.30  2003/08/13 22:02:15  mrimer
// Fixed bug: drive selected before checking validity.
//
// Revision 1.29  2003/08/09 22:48:31  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.28  2003/08/05 15:50:22  mrimer
// Fixed bug: OK button inactive when file is selected in FileDialogWidget.
//
// Revision 1.27  2003/08/05 00:13:41  erikh2000
// Fixed bugs appearing in windows 98 for enumerated drives and checking validity of directory.
//
// Revision 1.26  2003/08/03 02:18:29  schik
// Added some temporary debugging
//
// Revision 1.25  2003/08/02 20:18:10  mrimer
// Fixed bugs: can't save file to empty directory; OK button should be disabled when filename textbox is empty.
//
// Revision 1.24  2003/08/01 20:29:22  schik
// WindowsCanBrowseUnicode() was moved to CFiles
//
// Revision 1.23  2003/08/01 17:40:28  schik
// Fixed SetDirectory() for Win95/98/ME machines
//
// Revision 1.22  2003/07/28 18:20:20  schik
// Fixed so trailing slash can be entered in directory name
//
// Revision 1.21  2003/07/26 22:58:47  mrimer
// Overrode OnClick() to select the current directory on OK if the file list is empty.
//
// Revision 1.20  2003/07/25 01:03:18  mrimer
// Relative path fixes (committed on behalf of Gerry JJ).
//
// Revision 1.19  2003/07/19 21:16:23  mrimer
// Fixed bug: incorrectly detecting extension.
//
// Revision 1.18  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.17  2003/07/12 03:04:07  mrimer
// Files can now be selected by double-clicking on them.
//
// Revision 1.16  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.15  2003/07/09 21:03:10  mrimer
// Expanded the dialog so more names will fit in the list boxes.
//
// Revision 1.14  2003/07/09 04:39:03  schik
// (Hopefully) fixed SetDirectory() on Win9x/ME machines.
//
// Revision 1.13  2003/06/30 19:28:55  mrimer
// Fixed bug: drives not showing first time.
//
// Revision 1.12  2003/06/30 18:51:57  schik
// Files and directories are now sorted.
//
// Revision 1.11  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/06/19 15:18:52  schik
// Changed the way drives are detected.  For me at least, it now shows I have an A: drive, but without accessing the drive or showing an Abort, Retry, Fail?  dialog.
//
// Revision 1.9  2003/06/19 06:48:26  schik
// Windows 95 & 98 (& WinMe?) don't have _wfind* functions.  Windows version is detected at runtime and non-unicode versions are used on those platforms.
//
// Revision 1.8  2003/06/16 18:46:38  mrimer
// Fixed some bad UI.
//
// Revision 1.7  2003/06/13 06:14:05  mrimer
// Now filename is updated when a file is clicked on in the file list.
//
// Revision 1.6  2003/06/13 06:01:10  mrimer
// Changed disk existence check to only occur once, when dialog is activated.
//
// Revision 1.5  2003/06/10 01:20:13  mrimer
// Fixed some bugs.
//
// Revision 1.4  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.3  2003/05/24 03:03:40  mrimer
// Modified some remaining DROD-specific code.
//
// Revision 1.2  2003/05/23 21:41:23  mrimer
// Added portability for APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.9  2003/05/08 22:04:41  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.8  2003/04/29 11:12:09  mrimer
// Added better file extension handling.  Minor interface improvements.  Fixed various bugs.
//
// Revision 1.7  2003/04/26 17:15:58  mrimer
// Fixed some bugs with file selection.
//
// Revision 1.6  2003/04/13 04:31:40  mrimer
// Changed file selection to occur on double click.
// Removed checking the A: and B: drives on Windows.
//
// Revision 1.5  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.4  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.3  2003/02/16 20:32:18  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.2  2003/01/08 00:54:18  mrimer
// Fixed some bugs.  Added drive letter selection on WIN32.
//
// Revision 1.1  2002/12/22 02:26:45  mrimer
// Initial check-in.
//
