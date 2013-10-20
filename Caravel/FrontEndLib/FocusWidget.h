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

#ifndef FOCUSWIDGET_H
#define FOCUSWIDGET_H

#include "Widget.h"
#include <BackEndLib/Types.h>

//******************************************************************************
class CFocusWidget : public CWidget
{
public:
	CFocusWidget(WIDGETTYPE eSetType, DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH);
	virtual ~CFocusWidget(void) {};

	virtual void	Disable(void);
	virtual void	Hide(const bool bPaint = true);
	virtual bool	IsFocusable(void) const {return true;}
	bool	IsSelected(void) const {return this->bSelected;}
	void	Select(const bool bPaint = true);
	void	Unselect(const bool bPaint = true);
	
private:
	bool	bSelected;
};

#endif //#ifndef FOCUSWIDGET_H

// $Log: FocusWidget.h,v $
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.13  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.12  2002/10/21 20:16:13  mrimer
// Revised includes.  Made IsSelected() explicitly virtual.
//
// Revision 1.11  2002/07/17 21:18:51  mrimer
// Fixed widgets appearing at wrong times with dialog boxes.
//
// Revision 1.10  2002/07/05 10:32:17  erikh2000
// Many changes related to new event-handling scheme.
//
// Revision 1.9  2002/06/21 22:29:57  mrimer
// Made Disable() virtual function.
//
// Revision 1.8  2002/06/21 04:54:23  mrimer
// Added optional painting when selecting/unselecting.
//
// Revision 1.7  2002/06/17 17:59:13  mrimer
// Changed Unselect() to repaint disabled widgets.
//
// Revision 1.6  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.5  2002/06/14 18:32:23  mrimer
// Added comment for HandleKeyEvent().
//
// Revision 1.4  2002/06/13 21:45:28  mrimer
// Added verification that widget is active when selecting/unselecting.
//
// Revision 1.3  2002/06/05 03:06:25  mrimer
// Added mention that inherited destructor is virtual (with no effective change).
//
// Revision 1.2  2002/06/03 22:56:29  mrimer
// Added widget focusability.
//
// Revision 1.1  2002/05/24 14:12:17  mrimer
// Initial check-in.
//
