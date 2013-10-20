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
 * Contributor(s): mrimer
 *
 * ***** END LICENSE BLOCK ***** */

//SUMMARY
//
//CFlashMessageEffect is a class for flashing messages to be drawn on top of widgets.
//Effects are temporary animations drawn by the owner widget.  The screen surface 
//update is performed by the owner widget.
//

#ifndef CFLASHMESSAGEEFFECT_H
#define CFLASHMESSAGEEFFECT_H

#ifdef WIN32
#include <cstdio>
#else
#include <stdio.h>
#endif
#include <string>

#include "Effect.h"
#include "Widget.h"

#include <BackEndLib/Wchar.h>

//******************************************************************************
class CFlashMessageEffect : public CEffect
{
public:
	CFlashMessageEffect(CWidget *pSetWidget, const WCHAR* text, const Uint32 wDuration = 5000);	//5000ms
	
	virtual bool	Draw(SDL_Surface* pDestSurface=NULL);

private:
	SDL_Rect screenRect;
	WSTRING wstrText;
	UINT wAnimFrame;
	Uint32 dwStartTime, dwLastFrame, wDuration;
};

#endif	//...#ifndef CFLASHMESSAGEEFFECT_H

// $Log: FlashMessageEffect.h,v $
// Revision 1.3  2003/07/01 20:20:56  mrimer
// Added optional destination surface parameter to draw routines.  Added explicit virtual keyword to all Draw() methods.
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
// Revision 1.5  2003/02/16 20:32:18  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.4  2002/07/22 00:58:27  erikh2000
// Removed undefined destructor declaration.
//
// Revision 1.3  2002/06/16 22:13:46  erikh2000
// Changed a param to const since it was read-only.
//
// Revision 1.2  2002/05/17 00:47:10  mrimer
// Make copy of specified text string.
//
// Revision 1.1  2002/05/16 23:08:27  mrimer
// Initial checkin.
//
