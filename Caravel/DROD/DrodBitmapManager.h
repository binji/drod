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
 * Portions created by the Initial Developer are Copyright (C) 2002, 2003
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef DRODBITMAPMANAGER_H
#define DRODBITMAPMANAGER_H

#include <FrontEndLib/BitmapManager.h>

#include "TileImageConstants.h"

//****************************************************************************
class CDrodBitmapManager : public CBitmapManager
{
public:
	CDrodBitmapManager();
	virtual ~CDrodBitmapManager() {}
	
	virtual UINT	Init();
	bool			LoadTileImagesForStyle(const UINT wStyleNo);

	static UINT DISPLAY_COLS, DISPLAY_ROWS, CX_ROOM, CY_ROOM;

private:
	void			GetAntiAliasReplacementColors(SURFACECOLOR &Replace75, 
			SURFACECOLOR &Replace50);
	virtual bool			LoadTileImages(const WCHAR *pszName, bool bReplaceAntiAliasColors);
};

//Define global pointer to the one and only CDrodBitmapManager object.
#ifndef INCLUDED_FROM_DRODBITMAPMANAGER_CPP
	extern CDrodBitmapManager *g_pTheDBM;
#endif

#endif //...#ifndef DRODBITMAPMANAGER_H

// $Log: DrodBitmapManager.h,v $
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.1  2003/05/22 23:35:03  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
// Revision 1.15  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.14  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.13  2002/11/15 02:48:21  mrimer
// Added DoesStyleExist().
//
// Revision 1.12  2002/10/10 21:43:23  mrimer
// Added BlitLayeredTileImage() to draw o- and t-layer tiles simultaneously.
//
// Revision 1.11  2002/10/10 17:14:16  mrimer
// Added bIsColorKeySet.
//
// Revision 1.10  2002/10/07 20:11:02  mrimer
// Added BlitTileImagePart().
//
// Revision 1.9  2002/09/30 18:38:40  mrimer
// Added BAndWRect(), BAndWTile(), DarkenRect(), and DarkenTile() for room effects.
//
// Revision 1.8  2002/09/27 17:42:45  mrimer
// Added ShadeTile().  Made ShadeRect() more general purpose.
//
// Revision 1.7  2002/09/24 21:19:30  mrimer
// Added ShadeTile().
// Added opacity value to BlitTileImage().
// Made some parameters const.
//
// Revision 1.6  2002/06/03 22:55:42  mrimer
// Inserted SDL files.
//
// Revision 1.5  2002/04/19 21:59:44  erikh2000
// Tile dimension constants moved here from ScreenConstants.h.
//
// Revision 1.4  2002/04/14 00:25:02  erikh2000
// Bitmap manager will now anti-alias tiles based on the current style.
//
// Revision 1.3  2002/04/12 22:51:34  erikh2000
// Changed design of BM to use reference counting for bitmap loads.  New GetBitmapSurface() and ReleaseBitmapSurface() methods replace LoadBitmapSurface().
//
// Revision 1.2  2002/04/12 05:22:31  erikh2000
// Added an extra #include to get past compile err.
//
// Revision 1.1  2002/04/11 10:20:52  erikh2000
// Initial check-in.
//
