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

#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include "FocusWidget.h"

const UINT CY_STANDARD_SLIDER = 16;

//******************************************************************************
class CSliderWidget : public CFocusWidget
{		
	public:
		CSliderWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, 
				UINT wSetH, BYTE bytSetValue);

		BYTE				GetValue() const {return this->bytValue;}
		virtual void	Paint(bool bUpdateRect = true);
		void				SetValue(const BYTE bytSetValue);

	protected:
		virtual bool   Load();
		virtual void   Unload();

		virtual void   HandleMouseDown(const SDL_MouseButtonEvent &Button);
		virtual void   HandleDrag(const SDL_MouseMotionEvent &Motion);
		virtual void   HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);

	private:
		void				DrawFocused();
		void				SetToX(const int nX);

		BYTE				bytValue, bytPrevValue;
		bool				bWasSliderDrawn;
		bool				bFocusRegionsSaved;
		SDL_Surface *		pEraseSurface;
		SDL_Surface *		pFocusSurface[2];
};

#endif //#ifndef SLIDERWIDGET_H

// $Log: SliderWidget.h,v $
// Revision 1.2  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.6  2002/07/21 01:41:59  erikh2000
// Removed AcceptsTextEntry() override.  (Widget handles keypresses but doesn't convert them to text.)
//
// Revision 1.5  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.4  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.3  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.2  2002/06/05 03:09:07  mrimer
// Added focus graphic.  Added widget focusability and keyboard control.
//
// Revision 1.1  2002/04/24 08:11:14  erikh2000
// Initial check-in.
//
