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
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include "FocusWidget.h"

#include <BackEndLib/Wchar.h>

//Standard height of buttons.
const UINT CY_STANDARD_BUTTON = 22;

//******************************************************************************
class CButtonWidget : public CFocusWidget
{		
	public:
		CButtonWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, 
				UINT wSetH, const WCHAR *pwczSetCaption);
      virtual ~CButtonWidget();

		virtual void   Paint(bool bUpdateRect = true);
		void           Press() {this->bIsPressed=true;}
		void           SetCaption(const WCHAR *pwczSetCaption);
		void           Unpress() {this->bIsPressed=false;}

	protected:
		virtual void   HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);
		virtual void   HandleMouseDown(const SDL_MouseButtonEvent &Button);
		virtual void   HandleMouseUp(const SDL_MouseButtonEvent &Button);
		virtual void   HandleDrag(const SDL_MouseMotionEvent &Motion);

	private:
		void				DrawButtonText(const UINT wOffset);
		void				DrawFocused(const UINT wOffset);
		void				DrawNormal();
		void				DrawPressed();

		bool				bIsPressed;
		WCHAR *				pwczCaption;
};

#endif //#ifndef BUTTONWIDGET_H

// $Log: ButtonWidget.h,v $
// Revision 1.4  2003/09/16 16:02:55  schik
// Fixed memory leak
//
// Revision 1.3  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.2  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.13  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.12  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.11  2002/07/21 01:41:59  erikh2000
// Removed AcceptsTextEntry() override.  (Widget handles keypresses but doesn't convert them to text.)
//
// Revision 1.10  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.9  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.8  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.7  2002/06/03 22:56:29  mrimer
// Added widget focusability.
//
// Revision 1.6  2002/05/20 18:38:46  mrimer
// Added enabled/disabled flag to widget.
//
// Revision 1.5  2002/04/29 00:06:19  erikh2000
// Renamed "depressed" to "pressed" for consistency.
//
// Revision 1.4  2002/04/24 08:12:35  erikh2000
// Made buttons a little taller to fit a larger font.
//
// Revision 1.3  2002/04/19 22:00:31  erikh2000
// Standard button height constant added.
//
// Revision 1.2  2002/04/16 10:40:19  erikh2000
// Added text caption to button painting.
//
// Revision 1.1  2002/04/13 19:28:51  erikh2000
// Initial check-in.
//
