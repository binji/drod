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

//This is a CDialog that has been augmented to show a list of levels.

#ifndef LEVELSELECTDIALOGWIDGET_H
#define LEVELSELECTDIALOGWIDGET_H

#include <FrontEndLib/DialogWidget.h>
#include <BackEndLib/MessageIDs.h>

class CDbLevel;
class CLabelWidget;
class CListBoxWidget;
class CLevelSelectDialogWidget : public CDialogWidget
{
public:
	CLevelSelectDialogWidget(const DWORD dwSetTagNo, const int nSetX=0,
			const int nSetY=0);

	virtual bool   Load();

   void     EnableCancelButton(const bool bFlag);
	DWORD		GetSelectedItem() const;
	void		PopulateLevelList();
	void		SelectItem(const DWORD dwTagNo);
	void		SetPrompt(const MESSAGE_ID messageID);
	void		SetSourceLevel(CDbLevel *pLevel);

protected:
	virtual void	OnClick(const DWORD dwTagNo);
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);

private:
	CListBoxWidget *	pLevelListBoxWidget;
	CDbLevel *pSourceLevel;
	CLabelWidget *pPromptLabel;
};

#endif

// $Log: LevelSelectDialogWidget.h,v $
// Revision 1.6  2003/07/16 07:41:19  mrimer
// Revised selecting stair destination UI.
//
// Revision 1.5  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.4  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.2  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.1  2002/12/22 02:26:45  mrimer
// Initial check-in.
//
