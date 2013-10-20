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

//This is a CDialog that has been augmented to show a list of files.

#ifndef FILEDIALOGWIDGET_H
#define FILEDIALOGWIDGET_H

#include "DialogWidget.h"
#include <BackEndLib/MessageIDs.h>
#include <BackEndLib/Wchar.h>

enum FileExtensionType {
	EXT_PLAYER=0,
	EXT_HOLD=1,
	EXT_DEMO=2,
	EXT_COUNT
};

class CLabelWidget;
class CListBoxWidget;
class CTextBoxWidget;
class CFileDialogWidget : public CDialogWidget
{
public:
	CFileDialogWidget(const DWORD dwSetTagNo, const int nSetX=0,
			const int nSetY=0);

	WSTRING		GetSelectedDirectory() const {return this->dirpath;}
	WSTRING		GetSelectedFileName() const;
	void		PopulateDirList();
	void		PopulateFileList();
	void		SetDirectory(const WCHAR *wDirPath=NULL);
	void		SetExtension(const FileExtensionType extensionType);
	void		SetFilename(const WCHAR *wFilename);
	void		SetPrompt(const MESSAGE_ID messageID);
	void		SelectFile();
	void		SetWriting(const bool bWriting) {this->bWriting = bWriting;}

protected:
   virtual bool   Load();
	virtual void   OnClick(const DWORD dwTagNo);
	virtual void	OnDoubleClick(const DWORD dwTagNo);
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
	virtual void	OnSelectChange(const DWORD dwTagNo);

private:
   static void  CheckDrives();

   void		GoToDirectory();
	void		PopulateExtensionList(const FileExtensionType extensionType);

	CLabelWidget *pPromptLabel;
	CListBoxWidget *pDirListBoxWidget, *pFileListBoxWidget, *pExtensionListBoxWidget;
	CTextBoxWidget *pDirNameTextBox, *pFilenameTextBox;

	WSTRING dirpath;
	bool bWriting;	//whether file selected will be written to disk or not (i.e. read in)
};

#endif

// $Log: FileDialogWidget.h,v $
// Revision 1.10  2003/08/01 20:29:22  schik
// WindowsCanBrowseUnicode() was moved to CFiles
//
// Revision 1.9  2003/07/26 22:58:47  mrimer
// Overrode OnClick() to select the current directory on OK if the file list is empty.
//
// Revision 1.8  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.7  2003/06/30 19:28:55  mrimer
// Fixed bug: drives not showing first time.
//
// Revision 1.6  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/19 06:48:26  schik
// Windows 95 & 98 (& WinMe?) don't have _wfind* functions.  Windows version is detected at runtime and non-unicode versions are used on those platforms.
//
// Revision 1.4  2003/06/13 06:01:10  mrimer
// Changed disk existence check to only occur once, when dialog is activated.
//
// Revision 1.3  2003/06/10 01:20:14  mrimer
// Fixed some bugs.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.7  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.6  2003/04/29 11:12:09  mrimer
// Added better file extension handling.  Minor interface improvements.  Fixed various bugs.
//
// Revision 1.5  2003/04/26 17:13:22  mrimer
// Added GetSelectedDirectory().
//
// Revision 1.4  2003/04/13 04:31:40  mrimer
// Changed file selection to occur on double click.
// Removed checking the A: and B: drives on Windows.
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
