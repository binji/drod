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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef OPTIONBUTTONWIDGET_H
#define OPTIONBUTTONWIDGET_H

#include "FocusWidget.h"

#include <BackEndLib/Wchar.h>

//Standard height of option buttons.
const UINT CY_STANDARD_OPTIONBUTTON = 21;

//******************************************************************************
class COptionButtonWidget : public CFocusWidget
{		
	public:
		COptionButtonWidget(const DWORD dwSetTagNo, const int nSetX,
				const int nSetY, const UINT wSetW, const UINT wSetH,
				const WCHAR *pwczSetCaption, const bool bSetChecked,
				const bool bWhiteText=false);
      virtual ~COptionButtonWidget();

		bool				IsChecked() const {return this->bIsChecked;}
		virtual void	Paint(bool bUpdateRect = true);
		void				SetCaption(const WCHAR *pwczSetCaption);
		void				SetChecked(const bool bSetChecked) {this->bIsChecked = bSetChecked;}
		void				SetChecked(const int nSetChecked) {this->bIsChecked = (nSetChecked != 0);}
		void				ToggleCheck(const bool bShowEffects=false);

	protected:
		virtual bool   Load();
		virtual void   Unload();

		virtual void   HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);
		virtual void   HandleMouseDown(const SDL_MouseButtonEvent &Button);

	private:
		void				DrawChecked();
		void				DrawUnchecked();
		void				DrawFocused();

		bool				bFocusRegionsSaved;
		bool				bIsChecked;
		bool				bWhiteText;	//show text white instead of black
		SDL_Surface *		pFocusSurface;
		WCHAR *				pwczCaption;
};

#endif //#ifndef OPTIONBUTTONWIDGET_H

// $Log: OptionButtonWidget.h,v $
// Revision 1.4  2003/09/16 19:03:11  schik
// Fixed memory leak
//
// Revision 1.3  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.11  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.10  2002/11/15 02:35:23  mrimer
// Revised display options and constructor.
//
// Revision 1.9  2002/07/21 01:41:59  erikh2000
// Removed AcceptsTextEntry() override.  (Widget handles keypresses but doesn't convert them to text.)
//
// Revision 1.8  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.7  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.6  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.5  2002/06/11 22:43:32  mrimer
// Condensed SetUnchecked() into SetChecked(bool).
//
// Revision 1.4  2002/06/05 03:09:07  mrimer
// Added focus graphic.  Added widget focusability and keyboard control.
//
// Revision 1.3  2002/06/03 22:56:29  mrimer
// Added widget focusability.
//
// Revision 1.2  2002/04/24 08:14:05  erikh2000
// Implemented the class.
//
// Revision 1.1  2002/04/23 03:13:39  erikh2000
// Initial check-in.
//
