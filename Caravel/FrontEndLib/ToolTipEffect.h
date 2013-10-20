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

#ifndef TOOLTIPEFFECT_H
#define TOOLTIPEFFECT_H

#include "FontManager.h"
#include "Effect.h"
#include <BackEndLib/Coord.h>

#include <string>
#include <SDL.h>

//******************************************************************************
class CToolTipEffect : public CEffect
{
public:
	CToolTipEffect(CWidget *pSetWidget, const CCoord &SetCoord,
			const WCHAR *pwczSetText, const Uint32 dwDisplayDuration=5000,
			const UINT eSetFontType=FONTLIB::F_Small);
   virtual ~CToolTipEffect();

	virtual bool	Draw(SDL_Surface* pDestSurface=NULL);
	virtual long	GetDrawSequence() const {return 1000L;}	//draw last

	UINT				GetFontType() const {return this->eFontType;}
	void				GetTextWidthHeight(UINT &wW, UINT &wH) const;
	void				SetDuration(const Uint32 dwDuration)
			{this->dwDuration = dwDuration;}
	void				SetText(const WCHAR *pwczSetText, bool bResizeToFit=false);

private:
	UINT				x, y, w, h;
	WSTRING				wstrText;
	UINT				eFontType;

	SDL_Surface *	pToolTipSurface;
	Uint32			dwWhenEnabled, dwDuration;
};

#endif //#ifndef TOOLTIPEFFECT_H

// $Log: ToolTipEffect.h,v $
// Revision 1.5  2003/08/16 00:45:01  mrimer
// Optimized tool tip effect.  Fixed a bug.
//
// Revision 1.4  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
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
// Revision 1.3  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.2  2003/02/16 20:32:20  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.1  2002/11/22 02:26:30  mrimer
// Initial check-in.
//
