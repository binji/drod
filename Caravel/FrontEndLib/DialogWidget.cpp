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

#include "DialogWidget.h"
#include "BitmapManager.h"
#include "ButtonWidget.h"
#include "ListBoxWidget.h"
#include "Screen.h"
#include "TextBoxWidget.h"
#include <BackEndLib/Types.h>
#include <BackEndLib/Assert.h>

//
//Public methods.
//

//******************************************************************************
CDialogWidget::CDialogWidget(
//Constructor.
//
//Params:
	const DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	const int nSetX, const int nSetY,		//		constructor.
	const UINT wSetW, const UINT wSetH,		//
   const bool bListBoxDoubleClickReturns) //[default = false]
	: CEventHandlerWidget(WT_Dialog, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, dwDeactivateValue(TAG_UNSPECIFIED)
	, dwReqTextField(0L)
   , bListBoxDoubleClickReturns(bListBoxDoubleClickReturns)
{
}

//******************************************************************************
DWORD CDialogWidget::Display(void)
//All-in-one method to show a dialog, paint it, activate it, and hide the 
//dialog again.  Caller should probably repaint parent widget of dialog to 
//erase the dialog.
//
//Returns:
//Tag# indicating reason for exit--either TAG_QUIT or widget tag that was clicked.
{
	Show();

	SetForActivate();
	SelectFirstWidget(false);
	Paint();

	Activate();

   //Re-enable the OK button if it was disabled for mandatory text entry.
   if (this->dwReqTextField)
   {
	   CButtonWidget *pOKButton = DYN_CAST(CButtonWidget*, CWidget *,
			   GetWidget(TAG_OK));
      if (pOKButton)
         pOKButton->Enable();
   }

	Hide();

	return this->dwDeactivateValue;
}

//******************************************************************************
void CDialogWidget::Paint(
//Paint widget area.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	//NOTE: It is not necessary to apply scrolling offsets to dialog coords.
	//Dialogs float on top of their parents in the same position, regardless
	//of scrolling offsets.

	SDL_Surface *pDestSurface = GetDestSurface();

	//Source coords and dimensions.
	static const UINT CX_CORNER = 4;
	static const UINT CY_CORNER = 4;
	static const UINT X_LEFT_BEVEL = 0;
	static const UINT X_RIGHT_BEVEL = 132;
	static const UINT Y_TOP_BEVEL = 0;
	static const UINT Y_BOTTOM_BEVEL = 132;
	static const UINT X_CENTER = X_LEFT_BEVEL + CX_CORNER;
	static const UINT Y_CENTER = Y_TOP_BEVEL + CY_CORNER;
	static const UINT CX_CENTER = 128;
	static const UINT CY_CENTER = 128;
	ASSERT(this->w > CX_CORNER * 2);
	ASSERT(this->h > CY_CORNER * 2);

	//Dest coords and dimensions.
	UINT xLeftBevel = this->x;
	UINT xRightBevel = this->x + this->w - CX_CORNER;
	UINT yTopBevel = this->y;
	UINT yBottomBevel = this->y + this->h - CY_CORNER;
	UINT xCenter = xLeftBevel + CX_CORNER;
	UINT yCenter = yTopBevel + CY_CORNER;

	//Draw top-left corner.
	SDL_Rect src = {X_LEFT_BEVEL, Y_TOP_BEVEL, CX_CORNER, CY_CORNER};
	SDL_Rect dest = {xLeftBevel, yTopBevel, CX_CORNER, CY_CORNER};
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw top-right corner.
	src.x = X_RIGHT_BEVEL;
	dest.x = xRightBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-right corner.
	src.y = Y_BOTTOM_BEVEL;
	dest.y = yBottomBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom-left corner.
	src.x = X_LEFT_BEVEL;
	dest.x = xLeftBevel;
	SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);

	//Draw bottom bevel.
	src.x = X_CENTER;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw top bevel.
	dest.y = yTopBevel;
	src.y = Y_TOP_BEVEL;
	dest.w = src.w = CX_CENTER;
	for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
	{
		if (dest.x + CX_CENTER > xRightBevel)
			dest.w = src.w = xRightBevel - dest.x; //Clip the bevel to remaining width.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw left bevel.
	dest.x = xLeftBevel;
	src.x = X_LEFT_BEVEL;
	src.y = Y_CENTER;
	src.w = dest.w = CX_CORNER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw right bevel.
	dest.x = xRightBevel;
	src.x = X_RIGHT_BEVEL;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		if (dest.y + CY_CENTER > yBottomBevel)
			dest.h = src.h = yBottomBevel - dest.y; //Clip the bevel to remaining height.
		SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
	}

	//Draw center.
	src.x = X_CENTER;
	src.y = Y_CENTER;
	src.h = dest.h = CY_CENTER;
	for (dest.y = yCenter; dest.y < static_cast<int>(yBottomBevel); dest.y += CY_CENTER)
	{
		src.w = dest.w = CX_CENTER;
		for (dest.x = xCenter; dest.x < static_cast<int>(xRightBevel); dest.x += CX_CENTER)
		{
			if (dest.x + CX_CENTER > xRightBevel)
				dest.w = src.w = xRightBevel - dest.x; //Clip the blit to remaining width.
			if (dest.y + CY_CENTER > yBottomBevel)
				dest.h = src.h = yBottomBevel - dest.y; //Clip the blit to remaining height.
			SDL_BlitSurface(this->pPartsSurface, &src, pDestSurface, &dest);
		}
	}

	PaintChildren();

	if (bUpdateRect) UpdateRect();
}

//
//Protected methods.
//

//******************************************************************************
bool CDialogWidget::SetForActivate(void)
//Called before dialog is painted and activated.
//
//Returns:
//True if activation should continue, false if not.
{
	CheckTextBox();

	this->dwDeactivateValue=TAG_UNSPECIFIED;
	return true;
}

//*****************************************************************************
void CDialogWidget::OnClick(
//Handles click event.
//
//Params:
	const DWORD dwTagNo)				//(in)	Widget that received event.
{
	if (dwTagNo)
	{
		CWidget *pWidget = GetWidget(dwTagNo);
		if (pWidget->GetType() == WT_Button)	//only buttons will return from dialog
		{
			this->dwDeactivateValue = dwTagNo;
			Deactivate();
		}
	}
}

//*****************************************************************************
void CDialogWidget::OnDoubleClick(
//Handles double click event.
//
//Params:
	const DWORD dwTagNo)	//(in) Widget event applies to.
{
	if (dwTagNo)
	{
		CWidget *pWidget = GetWidget(dwTagNo);
		if (pWidget->GetType() == WT_ListBox && this->bListBoxDoubleClickReturns)
		{
         CListBoxWidget *pListBox = DYN_CAST(CListBoxWidget*, CWidget*, pWidget);
			if (pListBox->ClickedSelection())
         {
            //Double-clicking list box selection on the dialog activates OK button
			   this->dwDeactivateValue = TAG_OK;
			   Deactivate();
         }
		}
	}
}

//*****************************************************************************
void CDialogWidget::OnSelectChange(
//Handles selection change event.
//Pass event on to parent, if exists.
//
//Params:
	const DWORD dwTagNo)
{
	CEventHandlerWidget *pEventHandler = GetParentEventHandlerWidget();
	if (pEventHandler) pEventHandler->OnSelectChange(dwTagNo);
}

//*****************************************************************************
bool CDialogWidget::OnQuit()
//Handles SDL_QUIT event.
{
   if (this->pParent && this->pParent->GetType() == WT_Screen)
   {
      CScreen *const pScreen = static_cast<CScreen *const>(this->pParent);
      if (!pScreen->OnQuit())
         return false;
   }

   this->dwDeactivateValue = TAG_QUIT;
	Deactivate();
	return true;
}

//*****************************************************************************
void CDialogWidget::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD /*dwTagNo*/,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	switch (Key.keysym.sym)
	{
      case SDLK_RETURN:
         if (Key.keysym.mod & KMOD_ALT &&	!GetHotkeyTag(Key.keysym.sym))
            //going to next case
		case SDLK_F10:
         if (this->pParent && this->pParent->GetType() == WT_Screen)
         {
            CScreen *const pScreen = static_cast<CScreen *const>(this->pParent);
			   pScreen->ToggleScreenSize();
            Paint();
         }
		break;

		case SDLK_F4:	//Activator will need to look for TAG_QUIT and handle it.
			if (Key.keysym.mod & KMOD_ALT)
			{
				OnQuit();
			}
		break;

		case SDLK_ESCAPE:
			this->dwDeactivateValue = TAG_ESCAPE;
			Deactivate();
		return;

      default: break;
	}

	CheckTextBox();
}

//
// Private methods
//

//*****************************************************************************
void CDialogWidget::CheckTextBox()
//When this->dwReqTextField is set to the tag ID of a text box,
//this implies text must be entered.  To enforce this, the OK button is
//enabled only when the text box is non-empty.
{
	CButtonWidget *pOKButton = DYN_CAST(CButtonWidget*, CWidget *,
			GetWidget(TAG_OK));
   if (!pOKButton) return; //OK button is not present or hidden

   if (this->dwReqTextField)
   {
		CTextBoxWidget *pTextBox = DYN_CAST(CTextBoxWidget*, CWidget *,
				GetWidget(this->dwReqTextField));
		ASSERT(pTextBox);

		if (WCSlen(pTextBox->GetText()))
			pOKButton->Enable();
		else
			pOKButton->Disable();
		pOKButton->Paint();
   }
}

// $Log: DialogWidget.cpp,v $
// Revision 1.12  2004/01/02 01:08:47  mrimer
// Fixed bug: double-clicking on list box scroll area closes dialog.
//
// Revision 1.11  2003/07/24 04:01:53  mrimer
// Added optional flag allowing double-clicking a list box on the dialog to exit w/ OK return value.
//
// Revision 1.10  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.9  2003/07/12 22:13:59  mrimer
// Clicking "X" when a dialog box is active now brings up quit dialog instead of just closing the dialog.
//
// Revision 1.8  2003/07/12 20:31:33  mrimer
// Updated widget load/unload methods.
//
// Revision 1.7  2003/07/07 23:41:14  mrimer
// F10 now toggles full screen, when possible.
//
// Revision 1.6  2003/06/26 17:21:12  mrimer
// Fixed a bug (always enabling OK button on activation).  Revised some code.
//
// Revision 1.5  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/19 04:10:58  mrimer
// Fixed a bug (when no OK button is present).
//
// Revision 1.3  2003/06/17 23:12:54  mrimer
// Fixed a bug.
//
// Revision 1.2  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.26  2003/05/03 23:32:22  mrimer
// Added optional enforcement of entering some text on a text input dialog.
//
// Revision 1.25  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.24  2002/12/22 02:24:51  mrimer
// Added OnSelectChange().
//
// Revision 1.23  2002/10/04 18:03:48  mrimer
// Changed OnQuit() to return true.
//
// Revision 1.22  2002/09/03 21:34:40  erikh2000
// Added a comment giving instructions about scrolling.
//
// Revision 1.21  2002/07/17 21:20:19  mrimer
// Tweaking.
//
// Revision 1.20  2002/07/05 10:34:12  erikh2000
// Many changes to work with new event-handling scheme.
//
// Revision 1.19  2002/06/26 17:18:07  mrimer
// Removed UnFocus() calls.
//
// Revision 1.18  2002/06/25 05:43:19  mrimer
// Added call to StopRepeat().
//
// Revision 1.17  2002/06/24 22:21:44  mrimer
// Robust dialog box focus init when it has no focus widgets.
//
// Revision 1.16  2002/06/21 05:02:51  mrimer
// Added InitFocus() call.
//
// Revision 1.15  2002/06/16 06:48:56  erikh2000
// Fixed an access violation occurring when handlers return TAG_ESCAPE or TAG_QUIT.
//
// Revision 1.14  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.13  2002/06/14 18:33:41  mrimer
// Fixed some event handling.
//
// Revision 1.12  2002/06/14 00:47:12  erikh2000
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
//
// Revision 1.11  2002/06/13 21:51:35  mrimer
// Enhanced event handling in Activate().
//
// Revision 1.10  2002/06/11 22:57:45  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.9  2002/06/03 22:51:55  mrimer
// Added CEventHandlerWidget base class.
//
// Revision 1.8  2002/06/03 18:02:02  mrimer
// Fixed Activate().
//
// Revision 1.7  2002/06/01 04:57:47  mrimer
// Finished implementing hotkeys.
//
// Revision 1.6  2002/05/21 18:11:23  mrimer
// Added changing command key mappings in settings screen.
//
// Revision 1.5  2002/04/29 00:07:36  erikh2000
// Renamed "depressed" to "pressed" for consistency.
//
// Revision 1.4  2002/04/19 21:46:47  erikh2000
// Moved coordinate-in-widget-checking code to CWidget::GetWidgetContainingCoords().
//
// Revision 1.3  2002/04/14 00:26:49  erikh2000
// Changed Activate() event loop to not paint repeatedly.
//
// Revision 1.2  2002/04/13 19:43:18  erikh2000
// Class can now take over event loop and handle repaints to itself.
//
// Revision 1.1  2002/04/12 22:49:32  erikh2000
// Initial check-in.
//
