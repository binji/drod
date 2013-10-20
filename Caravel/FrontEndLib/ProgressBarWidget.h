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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * Caravel Software. All Rights Reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef PROGRESSBARWIDGET_H
#define PROGRESSBARWIDGET_H

#include "Widget.h"
#include <BackEndLib/Types.h>
#include <BackEndLib/Assert.h>

const UINT CY_STANDARD_PROGRESSBAR = 30;

//*********************************************************************************************************
class CProgressBarWidget : public CWidget
{		
	public:
		CProgressBarWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH, BYTE bytSetValue);

		BYTE				GetValue() const {return this->bytValue;}
		virtual void	Paint(bool bUpdateRect = true);
      virtual void   PaintClipped(const int /*nX*/, const int /*nY*/, const UINT /*wW*/,
               const UINT /*wH*/, const bool /*bUpdateRect*/ = true) {ASSERTP(false, "Can't paint clipped.");}
		void				SetValue(const BYTE bytSetValue);

	protected:
      virtual void   HandleAnimate();
      virtual bool   IsAnimated() const {return true;}
		
	private:
      BYTE				bytValue, bytLastDrawnValue;
};

#endif //#ifndef PROGRESSBARWIDGET_H

// $Log: ProgressBarWidget.h,v $
// Revision 1.5  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.4  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.3  2003/07/09 11:56:50  schik
// Made PaintClipped() arguments consistent among derived classes (VS.NET warnings)
//
// Revision 1.2  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
