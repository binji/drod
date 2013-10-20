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
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996, 
 * 1997, 2000, 2001, 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef SCROLLINGTEXTWIDGET_H
#define SCROLLINGTEXTWIDGET_H

#include "FontManager.h"
#include "Widget.h"
#include "LabelWidget.h"

#include <list>
using std::list;

struct TEXTITEM {
	CLabelWidget *pLabel;
	SDL_Color color;
};

//******************************************************************************
class CScrollingTextWidget : public CWidget
{
public:
	CScrollingTextWidget(const DWORD dwSetTagNo,
			const int nSetX, const int nSetY, const UINT wSetW, const UINT wSetH,
			const UINT wPixelsPerSecond=33);

	void				AddText(const WCHAR *pwczAddText, 
			const UINT eFontType, CLabelWidget::TEXTALIGN eTextAlign = CLabelWidget::TA_Left);
	void				ClearText(void);
	bool				ContainsText() const {return this->Children.size() > 0;}
	CLabelWidget *		GetLastShownLabel(void) const;
	bool				IsAllTextShown(void) const;
	virtual void	Paint(bool bUpdateRect = true);	
	void				ScrollText();
	void				SetBackground(SDL_Surface *pSetSurface, const int x = -1, 
			const int y = -1);
	void				SetScrollRate(const UINT wPixelsPerSecond);

	virtual void		HandleAnimate(void) 
	{
		if (ContainsText())
		{
			ScrollText();
			Paint();
		}
	}
	virtual bool		IsAnimated(void) const {return true;}

private:
	SDL_Surface *		pBackgroundSurface;
	int					nBackgroundX, nBackgroundY;
	int					nNextLabelY;

	UINT					wMSPerPixel;
	DWORD					dwLastScroll;
	bool					bPaused;
};

#endif //...#ifndef SCROLLINGTEXTWIDGET_H

// $Log: ScrollingTextWidget.h,v $
// Revision 1.4  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.3  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/28 23:00:42  erikh2000
// Using TA_* constants differently.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.8  2002/10/04 17:54:26  mrimer
// Added pause control.
//
// Revision 1.7  2002/09/05 18:24:06  erikh2000
// Added method to check that all check has been shown.  This typically happens before ContainsText() returns false and there is still text showing in the widget.
//
// Revision 1.6  2002/09/04 21:47:14  mrimer
// Added real time scrolling speed parameter.
//
// Revision 1.5  2002/09/03 21:39:47  erikh2000
// Changed the implementation to use child labels to store and render texts, instead of one SDL surface.
// Added a method used to figure out what the last shown text is.
//
// Revision 1.4  2002/07/23 20:14:02  mrimer
// Added intermediate surface to avoid rerendering text.
// Removed list structure.
// More revisions to get it working.
//
// Revision 1.3  2002/07/22 21:03:00  mrimer
// Added TextBelowWidgetArea().  Fixed some bugs.
//
// Revision 1.2  2002/07/19 20:30:16  mrimer
// Added ContainsText().
//
// Revision 1.1  2002/07/11 20:58:37  mrimer
// Initial check-in.
//
