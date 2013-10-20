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

#include "FocusWidget.h"
#include "EventHandlerWidget.h"

//
//Public methods.
//

//******************************************************************************
CFocusWidget::CFocusWidget(
//Constructor.
//
//Params:
	WIDGETTYPE eSetType,				//(in)	Required params for CWidget constructor.
	DWORD dwSetTagNo,						//
	int nSetX, int nSetY,				//
	UINT wSetW, UINT wSetH)			//
	: CWidget(eSetType, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
{
	this->bSelected = false;
}

//******************************************************************************
void CFocusWidget::Select(
//Selects the widget.
//Repaints if 'bPaint' is set.
//
//Params:
	const bool bPaint)	//(in) whether to repaint widget
{
	ASSERT(IsActive());
	this->bSelected = true;
	if (bPaint)
		this->Paint();
}

//******************************************************************************
void CFocusWidget::Unselect(
//Unselects the widget.
//Repaints if visible and 'bPaint' is set.
//
//Params:
	const bool bPaint)	//(in) whether to repaint widget
{
	this->bSelected = false;
	if (bPaint && IsVisible())
		this->Paint();
}

//******************************************************************************
void CFocusWidget::Disable(void)
//CWidget override that updates selection.
{
	CWidget::Disable();
	if (this->bSelected)
	{
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->SelectNextWidget();
	}
}

//******************************************************************************
void CFocusWidget::Hide(
//CWidget override that updates selection.
//
//Params:
	const bool bPaint)	//(in) whether to repaint next widget selected
{
	CWidget::Hide();
	if (this->bSelected)
	{
		CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
		if (pEventHandler) pEventHandler->SelectNextWidget(bPaint);
	}
}

// $Log: FocusWidget.cpp,v $
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.4  2002/07/17 21:21:10  mrimer
// Revised Hide().
//
// Revision 1.3  2002/07/05 10:32:17  erikh2000
// Many changes related to new event-handling scheme.
//
// Revision 1.2  2002/06/21 05:06:36  mrimer
// Moved Select() and Unselect() code from .h.  Added optional paints.
//
// Revision 1.1  2002/05/24 14:12:17  mrimer
// Initial check-in.
//
