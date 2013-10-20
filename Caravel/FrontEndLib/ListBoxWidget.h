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

#ifndef LISTBOXWIDGET_H
#define LISTBOXWIDGET_H

#include "FocusWidget.h"

#include <BackEndLib/Wchar.h>

using namespace std;

//Height of one list box item.
const UINT CY_LBOX_ITEM = 14;

enum LBOX_CLICK_RESULT
{
	LBCR_Nothing = 0,
	LBCR_UpButton,
	LBCR_DownButton,
	LBCR_PosButton,
	LBCR_PageUp,
	LBCR_PageDown,
	LBCR_NewSelection
};

struct LBOX_ITEM
{
	DWORD dwKey;
	WCHAR *pwczText;
};

//******************************************************************************
class CListBoxWidget : public CFocusWidget
{		
	public:
		CListBoxWidget(const DWORD dwSetTagNo, const int nSetX, const int nSetY,
				const UINT wSetW, const UINT wSetH, const bool bSortAlphabetically=false);
      virtual ~CListBoxWidget() {Clear();}

		void				AddItem(const DWORD dwSetKey, const WCHAR *pwczSetText);
		void				Clear();
		DWORD				GetSelectedItem() const;
		const WCHAR *		GetSelectedItemText() const;
		UINT				GetSelectedLineNumber() {return this->wSelectedLineNo;}
		UINT				GetItemCount() const {return this->Items.size();}
		virtual void	Move(const int nSetX, const int nSetY);
		virtual void	Paint(bool bUpdateRect = true);
		void				RemoveItem(const DWORD dwKey);
		void				SelectLine(const UINT wLineNo);
		void				SelectItem(const DWORD dwKey);
		void				SetSelectedItemText(const WCHAR *pwczSetText);
		bool				ClickedSelection() const;

	protected:
		virtual void   HandleDrag(const SDL_MouseMotionEvent &MouseMotionEvent);
		virtual void   HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);
		virtual void   HandleMouseDown(const SDL_MouseButtonEvent &MouseButtonEvent);
		virtual void   HandleMouseDownRepeat(const SDL_MouseButtonEvent &MouseButtonEvent);
		virtual void   HandleMouseUp(const SDL_MouseButtonEvent &Button);
		virtual void   HandleMouseWheel(const SDL_MouseButtonEvent &Button);

	private:
		void				CalcAreas();
		LBOX_CLICK_RESULT	ClickAtCoords(const int nX, const int nY);
		void				DragPosButton(const int nY);
		void				ScrollDownOneLine();
		void				ScrollDownOnePage();
		void				ScrollUpOneLine();
		void				ScrollUpOnePage();

		list<LBOX_ITEM *>	Items;
		UINT				wTopLineNo;
		UINT				wSelectedLineNo;
		UINT				wDisplayLineCount;
		SDL_Rect			PosRect;
		SDL_Rect			UpRect;
		SDL_Rect			DownRect;
		SDL_Rect			ItemsRect;
		SDL_Rect			ScrollRect;

		LBOX_CLICK_RESULT	eLastClickResult;
		bool				bIsUpButtonPressed;
		bool				bIsDownButtonPressed;
		int					nPosClickY;
		UINT				wPosClickTopLineNo;
		bool				bClickedSelection;

      bool           bSortAlphabetically;
};

#endif //#ifndef LISTBOXWIDGET_H

// $Log: ListBoxWidget.h,v $
// Revision 1.6  2004/01/02 01:09:59  mrimer
// Added optional alphabetical sorting of list items.
//
// Revision 1.5  2003/08/22 04:19:29  mrimer
// Moved call to Clear() from Unload() to the (new) destructor.
//
// Revision 1.4  2003/08/05 17:02:06  mrimer
// Added SetSelectedItemText().
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
// Revision 1.21  2003/05/13 01:11:04  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.20  2003/04/29 11:09:53  mrimer
// Added GetSelectedLineNumber().
//
// Revision 1.19  2003/04/17 21:04:02  mrimer
// Added HandleMouseWheel().
//
// Revision 1.18  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.17  2003/01/08 00:53:43  mrimer
// Added ClickedSelection().
//
// Revision 1.16  2002/12/22 02:22:12  mrimer
// Added GetSelectedItemText().
//
// Revision 1.15  2002/11/15 02:40:20  mrimer
// Made parameters const.
//
// Revision 1.14  2002/07/21 01:41:59  erikh2000
// Removed AcceptsTextEntry() override.  (Widget handles keypresses but doesn't convert them to text.)
//
// Revision 1.13  2002/07/20 23:11:54  erikh2000
// Took out some declarations I accidentally left in.
//
// Revision 1.12  2002/07/20 23:10:47  erikh2000
// Revised the keydown handler to support other keys and paint more quickly.
//
// Revision 1.11  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.10  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.9  2002/06/16 06:24:52  erikh2000
// Added method for removing list box items.
//
// Revision 1.8  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.7  2002/06/09 06:42:28  erikh2000
// Changed a param to const because its use is read-only.
//
// Revision 1.6  2002/06/07 17:46:42  mrimer
// Fixed mouse button handling.
//
// Revision 1.5  2002/06/05 03:09:07  mrimer
// Added focus graphic.  Added widget focusability and keyboard control.
//
// Revision 1.4  2002/05/15 01:26:46  erikh2000
// Added new methods to select a line and get number if items in list box.
//
// Revision 1.3  2002/05/10 22:36:41  erikh2000
// Removed widget area calculation code from paint code and put it in a separate routine.
//
// Revision 1.2  2002/04/29 00:13:14  erikh2000
// Implemented the class.
//
// Revision 1.1  2002/04/25 09:28:28  erikh2000
// Initial check-in.
//
