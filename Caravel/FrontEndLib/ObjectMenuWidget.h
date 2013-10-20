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

#ifndef OBJECTMENUWIDGET_H
#define OBJECTMENUWIDGET_H

#include "FocusWidget.h"
#include <list>

//To display one object on the ObjectMenuWidget.
//For example, an object might be a wall, depicted by a 2x2 wall tile section.
typedef struct
{
	UINT wObjectNo;		//object id
	UINT wXSize, wYSize;	//# tiles to show for display
	UINT *paTiles;			//the literal tiles to show
	int nX, nY;				//relative position in widget
} MENUOBJECT;

typedef list<MENUOBJECT *>::const_iterator OBJECT_ITERATOR;
typedef list<MENUOBJECT *>::reverse_iterator OBJECT_RITERATOR;

#define	NO_SELECTED_OBJECT	((UINT)-1)

//******************************************************************************
class CObjectMenuWidget : public CFocusWidget
{		
public:
	CObjectMenuWidget(const DWORD dwSetTagNo, const int nSetX,
			const int nSetY, const UINT wSetW, const UINT wSetH,
			const UINT wGapX, const UINT wGapY, const UINT wFloorTile);
	virtual ~CObjectMenuWidget();

	void				AddObject(const UINT wObjectNo, const UINT wXSize,
			const UINT wYSize, const UINT* paTiles);
	UINT				GetSelectedObject();

	virtual void	Paint(bool bUpdateRect = true);

protected:
	virtual bool   Load();
	virtual void   Unload();

	virtual void   HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);
	virtual void   HandleMouseDown(const SDL_MouseButtonEvent &Button);

private:
	void				SetSelectedObject(OBJECT_ITERATOR const &pSelectedObject);

	UINT						wGapX, wGapY;
	UINT						wFloorTile;
	list<MENUOBJECT *>	lObjects;	//objects added to the menu
	OBJECT_ITERATOR		iterSelectedObject;

	//To erase previous selection graphic
	SDL_Surface *			pEraseSurface;
	OBJECT_ITERATOR		iterPrevSelectedObject;
	bool						bSelectionDrawn;
};

#endif //#ifndef OBJECTMENUWIDGET_H

// $Log: ObjectMenuWidget.h,v $
// Revision 1.2  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.2  2002/11/22 02:16:59  mrimer
// Made selection border look better.
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
