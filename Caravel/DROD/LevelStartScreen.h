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
 * Portions created by the Initial Developer are Copyright (C) 
 * 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef LEVELSTARTSCREEN_H
#define LEVELSTARTSCREEN_H

#include "DrodScreen.h"
#include <FrontEndLib/LabelWidget.h>

#include <BackEndLib/Types.h>

class CLevelStartScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;

	CLevelStartScreen();

	virtual bool   Load();
	virtual void   Paint(bool bUpdateRect=true);
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	void		DrawDivider(const int nY);
	virtual void   OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);
	virtual void   OnMouseDown(const DWORD dwTagNo, const SDL_MouseButtonEvent &Button);

	CLabelWidget *	pHoldNameWidget, *pLevelNameWidget;
	CLabelWidget *	pCreatedWidget;
	CLabelWidget *	pAuthorWidget;
	CLabelWidget *	pDescriptionWidget;
};

#endif //...#ifndef LEVELSTARTSCREEN_H

// $Log: LevelStartScreen.h,v $
// Revision 1.11  2003/08/16 00:39:49  mrimer
// Tweaking method definition.
//
// Revision 1.10  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.9  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.8  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.7  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.6  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.5  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.4  2002/11/22 02:17:31  mrimer
// Added hold name above level name.
//
// Revision 1.3  2002/07/05 10:36:55  erikh2000
// Changed to use new event-handling scheme.
//
// Revision 1.2  2002/06/23 10:54:02  erikh2000
// The level start screen now works completely.
//
// Revision 1.1  2002/06/21 22:31:17  erikh2000
// Initial check-in.
//
