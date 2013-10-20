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
 * Portions created by the Initial Developer are Copyright (C) 2002, 2003
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef DRODSCREEN_H
#define DRODSCREEN_H

#include <FrontEndLib/Screen.h>
#include "DrodScreenManager.h"

#include "LevelSelectDialogWidget.h"

class CDrodScreen : public CScreen
{
public:
	CDrodScreen(const SCREENTYPE eSetType);
	virtual ~CDrodScreen() {}

protected:
   void           EnablePlayerSettings(const DWORD dwPlayerID);
	bool				SelectLevelID(CDbLevel *pLevel, DWORD &dwLevelTagNo,
			const MESSAGE_ID messagePromptID, bool bEnableCancel=true);

	CLevelSelectDialogWidget *	pLevelBox;		//choose from list of levels
};

#endif //...#ifndef DRODSCREEN_H

// $Log: DrodScreen.h,v $
// Revision 1.4  2003/07/16 07:41:19  mrimer
// Revised selecting stair destination UI.
//
// Revision 1.3  2003/07/03 08:03:18  mrimer
// Added EnablePlayerSettings().
//
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.1  2003/05/22 23:35:04  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
