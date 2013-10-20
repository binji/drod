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

#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include "FontManager.h"
#include "Widget.h"

#include <string>

using namespace std;

//******************************************************************************
class CLabelWidget : public CWidget
{		
public:
    typedef enum tagTextAlign
    {
	    TA_Left = 0,
	    TA_CenterGroup = 2	//Multiple lines of text are not individually centered, but the
						    //group of lines as a whole are centered within the label area.
    } TEXTALIGN;

	CLabelWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, 
			UINT wSetH, const UINT eSetFontType, const WCHAR *pwczSetText, 
			bool bResizeToFit=false);

	UINT				GetFontType() const {return this->eFontType;}
	void				GetTextWidthHeight(UINT &wW, UINT &wH) const;
	virtual void	Paint(bool bUpdateRect = true);
	void				SetAlign(TEXTALIGN eSetAlign) {this->eTextAlign = eSetAlign;}
	void				SetText(const WCHAR *pwczSetText, bool bResizeToFit=false);

private:
	WSTRING				wstrText;
	UINT			eFontType;
	TEXTALIGN			eTextAlign;
};

#endif //#ifndef LABELWIDGET_H

// $Log: LabelWidget.h,v $
// Revision 1.3  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.2  2003/05/28 22:59:50  erikh2000
// Moved alignment const declarations inside class declaration.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.8  2003/02/16 20:32:19  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.7  2002/09/03 21:37:34  erikh2000
// Made the label widget scrollable.
// Added a resize-to-fit flag to constructor and SetText() that will make the label vertically fit the text assigned to it.
//
// Revision 1.6  2002/07/11 20:54:49  mrimer
// Added GetFontType().
//
// Revision 1.5  2002/06/23 10:53:23  erikh2000
// Text alignment for labels can now be either left-justified or centered.
// Added method to get dimensions of text within label.
//
// Revision 1.4  2002/06/21 01:27:29  erikh2000
// Changed internal storage of label text from WCHAR array to wstring.
//
// Revision 1.3  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.2  2002/04/16 10:44:30  erikh2000
// Label widget now paints text using the font manager.
//
// Revision 1.1  2002/04/09 10:48:33  erikh2000
// Initial check-in.
//
