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
 * ***** END LICENSE BLOCK ***** */

//This is just a CDialog that has been changed so that it deactivates when a 
//keydown event is received.

#ifndef KEYPRESSDIALOGWIDGET_H
#define KEYPRESSDIALOGWIDGET_H

#include "DialogWidget.h"

#include <SDL.h>

class CKeypressDialogWidget : public CDialogWidget
{
public:
	CKeypressDialogWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH);

	SDLKey			GetKey(void) const {return this->Key;}

private:
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &Key);

	SDLKey			Key;
};

#endif

// $Log: KeypressDialogWidget.h,v $
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.4  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.3  2002/07/21 01:41:59  erikh2000
// Removed AcceptsTextEntry() override.  (Widget handles keypresses but doesn't convert them to text.)
//
// Revision 1.2  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.1  2002/07/05 10:41:00  erikh2000
// Initial check-in.
//
