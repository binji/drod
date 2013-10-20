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

#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include "Widget.h"

#include <BackEndLib/Types.h>
#include <BackEndLib/Wchar.h>

//******************************************************************************
class CFrameWidget : public CWidget
{		
public:
	CFrameWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, 
			UINT wSetH, const WCHAR *pwczSetCaption);
   virtual ~CFrameWidget();

	virtual void	Paint(bool bUpdateRect = true);
	virtual void	PaintClipped(const int nX, const int nY, const UINT wW,
         const UINT wH, const bool bUpdateRect = true);
	void				SetCaption(const WCHAR *pwczSetCaption);

private:
	WCHAR * 			pwczCaption;
};

#endif //#ifndef FRAMEWIDGET_H

// $Log: FrameWidget.h,v $
// Revision 1.5  2003/09/16 19:03:11  schik
// Fixed memory leak
//
// Revision 1.4  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.3  2003/07/09 11:56:50  schik
// Made PaintClipped() arguments consistent among derived classes (VS.NET warnings)
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
// Revision 1.6  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.5  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.4  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.3  2002/04/19 21:52:47  erikh2000
// Frames can now display a text caption in their topleft corner.
//
// Revision 1.2  2002/04/14 00:30:46  erikh2000
// Added PaintClipped() method that fires an assertion.
//
// Revision 1.1  2002/04/13 19:28:51  erikh2000
// Initial check-in.
//
