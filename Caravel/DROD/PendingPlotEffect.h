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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef PENDINGPLOTEFFECT_H
#define PENDINGPLOTEFFECT_H

#include "DrodEffect.h"

//****************************************************************************************
class CPendingPlotEffect : public CEffect
{
public:
	CPendingPlotEffect(CWidget *pSetWidget, const UINT wObjectNo,
			const UINT* wTileImageNo, const UINT wXSize=1, const UINT wYSize=1);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);

private:
	void PlotStaircase(const UINT wStartX, const UINT wStartY,
			const UINT wEndX, const UINT wEndY, SDL_Surface* pDestSurface);

	const UINT *	pwTileImageNo;
	UINT		wObjectNo, wXSize, wYSize;
	static unsigned char nOpacity;
	static bool bRising;
};

#endif //...#ifndef PENDINGPLOTEFFECT_H

// $Log: PendingPlotEffect.h,v $
// Revision 1.6  2003/08/05 01:39:47  mrimer
// Moved DROD-specific effects from FrontEndLib.
//
// Revision 1.5  2003/07/01 20:30:32  mrimer
// Added optional destination surface parameter to draw methods.
// Made all Draw() methods explicit virtual.  Fixed some method definition bugs.
//
// Revision 1.4  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.3  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.2  2002/11/22 02:16:16  mrimer
// Added PlotStaircase() to perform special handling for staircases.  Made static method vars into class vars.
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
