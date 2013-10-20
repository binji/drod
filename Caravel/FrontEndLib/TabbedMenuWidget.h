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

#ifndef TABBEDMENUWIDGET_H
#define TABBEDMENUWIDGET_H

#include "FocusWidget.h"
#include <list>

//******************************************************************************
class CTabbedMenuWidget : public CFocusWidget
{		
public:
	CTabbedMenuWidget(const DWORD dwSetTagNo, const int nSetX,
			const int nSetY, const UINT wSetW, const UINT wSetH,
			const UINT wTabs);
	virtual ~CTabbedMenuWidget();

	//Adding a widget the standard way defaults to first tag-menu.
	virtual CWidget *		AddWidget(CWidget *pNewWidget, bool bLoad = false)
		{return AddWidgetToTab(pNewWidget, 0, bLoad);}
	virtual CWidget *		AddWidgetToTab(CWidget *pNewWidget, const UINT wTab,
			bool bLoad = false);

	virtual void	Paint(bool bUpdateRect = true);
	void				SetTabTile(const UINT wTab, const UINT wTileNo);
	UINT				GetSelectedTab() const {return wSelectedTab;}

protected:
	virtual void		HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);
	virtual void		HandleMouseDown(const SDL_MouseButtonEvent &Button);

private:
	void				DrawTabs(void);
	void				DrawFocused(void);
	void				SelectTab(const UINT wTab);

	list<CWidget *>*	plChildWidgetsForTab;
	UINT					wNumTabs;
	UINT					wSelectedTab;
	UINT *				pwTabTileNo;
};

#endif //#ifndef TABBEDMENUWIDGET_H

// $Log: TabbedMenuWidget.h,v $
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
