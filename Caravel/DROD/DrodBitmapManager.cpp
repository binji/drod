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

#define INCLUDED_FROM_DRODBITMAPMANAGER_CPP
#include "DrodBitmapManager.h"
#undef INCLUDED_FROM_DRODBITMAPMANAGER_CPP

#include "TileImageConstants.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>

//Holds the only instance of CDrodBitmapManager for the app.
CDrodBitmapManager *g_pTheDBM = NULL;

UINT CDrodBitmapManager::DISPLAY_COLS = 38;
UINT CDrodBitmapManager::DISPLAY_ROWS = 32;
UINT CDrodBitmapManager::CX_ROOM = 0;
UINT CDrodBitmapManager::CY_ROOM = 0;

//
//Public methods.
//

//**********************************************************************************
CDrodBitmapManager::CDrodBitmapManager()
	: CBitmapManager()
//Constructor.
{
	CBitmapManager::CX_TILE = 14;
	CBitmapManager::CY_TILE = 14;
	CDrodBitmapManager::CX_ROOM = CBitmapManager::CX_TILE * CDrodBitmapManager::DISPLAY_COLS;
	CDrodBitmapManager::CY_ROOM = CBitmapManager::CY_TILE * CDrodBitmapManager::DISPLAY_ROWS;

	this->wTileCount = TI_COUNT;
	this->TileImageTypes = new TILEIMAGETYPE[TI_COUNT];

	for (UINT wI = TI_COUNT; wI--; )
		this->TileImageTypes[wI] = TIT_Unspecified;
}

//**********************************************************************************
UINT CDrodBitmapManager::Init(void)
//Initializes bitmap manager so that it can be used.
//
//Returns:
//MID_Success of a message ID describing failure.
{
	//Create the tiles surface.  It will be large enough to hold TI_COUNT tiles.
	this->pTileImagesSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			CX_TILE * TI_COUNT, CY_TILE, 24, 0, 0, 0, 0);
	if (!this->pTileImagesSurface) return MID_OutOfMemory;

	this->TransparentColor = GetSurfaceColor(this->pTileImagesSurface, 192, 192, 192);

	//Success.
	return MID_Success;
}

//**********************************************************************************
bool CDrodBitmapManager::LoadTileImagesForStyle(
//Loads tile images corresponding to a style.  Tile images will become available in
//future calls to GetTileImage().
//
//Params:
	const UINT wStyleNo)	//(in)	Style to load.
//
//Returns:
//True if successful, false if not.
{	
	ASSERT(wStyleNo > 0 && wStyleNo < 99);

	//Load tiles specific to style.
	WSTRING wstrFilename;
	AsciiToUnicode("Style", wstrFilename);
	WCHAR szNum[3];
	wstrFilename += _itoW(wStyleNo, szNum, 10);
	WSTRING wstr;
	AsciiToUnicode("Tiles", wstr);
	wstrFilename += wstr;
	if (!LoadTileImages(wstrFilename.c_str(), false)) return false;

	//Load general tiles that apply to every style.
	AsciiToUnicode("GeneralTiles", wstr);
	return LoadTileImages(wstr.c_str(), true);
}

//**********************************************************************************
bool CDrodBitmapManager::LoadTileImages(
//Loads a tile image bitmap.  If tile images that it contains are already loaded,
//the tile images will be overwritten in in the tile images surface.
//
//Params:
	const WCHAR *wszName,			//(in)	Name of bitmap without extension.
	bool bReplaceAntiAliasColors)	//(in)	If true then certain reserved colors
									//		will be replaced with anti-aliasing 
									//		colors appropriate for the current
									//		style.
//
//Returns:
//True if successful, false if not.
{
	//Lock now to speed up lock/unlock pairs in called routines.
	LockTileImagesSurface();

	//Figure out anti-aliasing replacement colors.
	SURFACECOLOR Replace75, Replace50;
	if (bReplaceAntiAliasColors)
		GetAntiAliasReplacementColors(Replace75, Replace50);

	//Load the source bitmap containing tile images.
	UINT wCols, wRows;
	SDL_Surface *pSrcSurface = LoadBitmapSurface(wszName);
	if (!pSrcSurface) return false;
	ASSERT(pSrcSurface->w % CX_TILE == 0);
	ASSERT(pSrcSurface->h % CY_TILE == 0);
	wCols = (pSrcSurface->w / CX_TILE);
	wRows = (pSrcSurface->h / CY_TILE);
	
	//Get mapping index from the tile image map file.  Each indice specifies
	//which TI_* constant corresponds to the tile image within the source bitmap.
	list<UINT> MappingIndex;
	SDL_Rect src = {0, 0, CX_TILE, CY_TILE};
	SDL_Rect dest = {0, 0, CX_TILE, CY_TILE};
	if (GetMappingIndexFromTileImageMap(wszName, MappingIndex))
	{
		list<UINT>::const_iterator iIndex = MappingIndex.begin();

		//Copy source tile images into the manager's tile image surface.  Their
		//position is determined from the mapping index.
		UINT wTileImageNo;
		for (UINT wY = 0; wY < wRows; ++wY)
		{
			for (UINT wX = 0; wX < wCols; ++wX)
			{
				wTileImageNo = *iIndex;
				if (wTileImageNo != (UINT)(TI_UNSPECIFIED))
				{
					//Blit tile image to manger's tile image surface.
					ASSERT(wTileImageNo <= TI_COUNT);
					dest.x = wTileImageNo * CX_TILE;
					SDL_BlitSurface(pSrcSurface, &src, this->pTileImagesSurface, &dest);

					//Set type of tile image.
					if (DoesTileImageContainTransparentPixels(wTileImageNo))
						this->TileImageTypes[wTileImageNo] = TIT_Transparent;
					else
						this->TileImageTypes[wTileImageNo] = TIT_Opaque;

					//Replace anti-aliasing colors for transparent tile images.
					if (bReplaceAntiAliasColors &&
							TileImageTypes[wTileImageNo] == TIT_Transparent)
						ReplaceAntiAliasingColors(wTileImageNo, Replace75, Replace50);
				}
				++iIndex;
				if (iIndex == MappingIndex.end()) break;

				src.x += CX_TILE;
			}
			if (iIndex == MappingIndex.end()) break;

			src.x = 0;
			src.y += CY_TILE;
		}		
	}

	UnlockTileImagesSurface();

	SDL_FreeSurface(pSrcSurface);

	return true;
}

//**********************************************************************************
void CDrodBitmapManager::GetAntiAliasReplacementColors(
//Gets the best anti-aliasing colors for transparent blits on currently loaded
//tiles.
//
//Params:
	SURFACECOLOR &Replace75,	//(out)	75% transparency color.
	SURFACECOLOR &Replace50)	//(out)	50% transparency color.
{
	//
	//Find a good average color for a transparent pixel.  In other words, if I had
	//to guess what color would be underneath a transparent pixel of a game sprite,
	//what would be a close guess?
	//

	SURFACECOLOR AverageTransparent;

	//Are the dark and light floor square tiles loaded?
	if (this->TileImageTypes[TI_EMPTY_L] != TIT_Unspecified &&
			this->TileImageTypes[TI_EMPTY_D] != TIT_Unspecified) //Yes.
	{
		//Get the topleft pixel of TI_EMPTY_D tile image.
		LockTileImagesSurface();
		SURFACECOLOR Dark;
		Uint8 *pPixel = (Uint8 *) this->pTileImagesSurface->pixels + 
				(CX_TILE * TI_EMPTY_D * this->pTileImagesSurface->format->BytesPerPixel);
		Dark.byt1 = pPixel[0];
		Dark.byt2 = pPixel[1];
		Dark.byt3 = pPixel[2];

		//Get the topleft pixel of TI_EMPTY_L tile image.
		SURFACECOLOR Light;
		pPixel = (Uint8 *) this->pTileImagesSurface->pixels + 
				(CX_TILE * TI_EMPTY_L * this->pTileImagesSurface->format->BytesPerPixel);
		Light.byt1 = pPixel[0];
		Light.byt2 = pPixel[1];
		Light.byt3 = pPixel[2];

		//Swap R,G,B if one is darker than the other.  This is to simplify averaging
		//calcs below.
		BYTE bytSwap;
		#define SWAPBYTE(a,b) {bytSwap = (a); (a) = (b); (b) = bytSwap;}

		if (Dark.byt1 > Light.byt1) SWAPBYTE(Dark.byt1, Light.byt1);
		if (Dark.byt2 > Light.byt2) SWAPBYTE(Dark.byt2, Light.byt2);
		if (Dark.byt3 > Light.byt3) SWAPBYTE(Dark.byt3, Light.byt3);

		#undef SWAPBYTE
		

		//Get average color between dark and light colors.
		#define GETAVERAGE(a,b) ((b) + ((a) - (b)) / 2)

		AverageTransparent.byt1 = GETAVERAGE(Dark.byt1, Light.byt1);
		AverageTransparent.byt2 = GETAVERAGE(Dark.byt2, Light.byt2);
		AverageTransparent.byt3 = GETAVERAGE(Dark.byt3, Light.byt3);

		#undef GETAVERAGE
	}
	else
	{
		//The two tile images I wanted to use aren't loaded, so just pick 
		//an average transparent color that might not be too bad.
		AverageTransparent.byt1 = AverageTransparent.byt2 = 
				AverageTransparent.byt3 = 207;
	}

	//
	//Calculate the anti-aliasing colors as being a certain percentage
	//between black and the average transparent color.  100% transparent would
	//give RGB values equal to the average transparent color.  0% transparent
	//would be black.
	//

	#define TRANSLUCENTBYTE(byt, percent) static_cast<Uint8>( (byt) * (1 - (percent)) )

	Replace75.byt1 = TRANSLUCENTBYTE(AverageTransparent.byt1, .75);
	Replace75.byt2 = TRANSLUCENTBYTE(AverageTransparent.byt2, .75);
	Replace75.byt3 = TRANSLUCENTBYTE(AverageTransparent.byt3, .75);

	Replace50.byt1 = TRANSLUCENTBYTE(AverageTransparent.byt1, .50);
	Replace50.byt2 = TRANSLUCENTBYTE(AverageTransparent.byt2, .50);
	Replace50.byt3 = TRANSLUCENTBYTE(AverageTransparent.byt3, .50);

	#undef TRANSLUCENTBYTE

	//
	//If either of the two replacement colors are same as the reserved
	//transparent color, then I have to change slightly so that they aren't
	//used for color keying.
	//

	if (Replace75.byt1 == this->TransparentColor.byt1 &&
			Replace75.byt2 == this->TransparentColor.byt2 &&
			Replace75.byt3 == this->TransparentColor.byt3)
		Replace75.byt1 += (Replace75.byt1 == 255) ? -1 : 1;

	if (Replace50.byt1 == this->TransparentColor.byt1 &&
			Replace50.byt2 == this->TransparentColor.byt2 &&
			Replace50.byt3 == this->TransparentColor.byt3)
		Replace50.byt1 += (Replace50.byt1 == 255) ? -1 : 1;
}


// $Log: DrodBitmapManager.cpp,v $
// Revision 1.6  2003/10/20 17:49:37  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.5  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.4  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/05/25 22:48:11  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.1  2003/05/22 23:35:04  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
// Revision 1.27  2003/05/04 00:23:36  mrimer
// Revised CStretchyBuffer API.
//
// Revision 1.26  2003/04/08 13:08:25  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.25  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.24  2003/02/17 03:28:08  erikh2000
// Removed L() macro.
//
// Revision 1.23  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.22  2003/02/16 20:32:17  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.21  2003/01/04 23:13:28  mrimer
// Made surface lock error handling more robust.
//
// Revision 1.20  2002/11/15 02:48:21  mrimer
// Added DoesStyleExist().
//
// Revision 1.19  2002/10/11 15:29:56  mrimer
// Optimized blitting two tiles when top one is opaque.
//
// Revision 1.18  2002/10/11 01:54:51  erikh2000
// Changed assertians to handle case of a t-square containing an opaque tile image.
//
// Revision 1.17  2002/10/10 21:43:23  mrimer
// Added BlitLayeredTileImage() to draw o- and t-layer tiles simultaneously.
//
// Revision 1.16  2002/10/10 17:17:32  mrimer
// Fixed drawing transparent tile bug.
// Revised includes.
//
// Revision 1.15  2002/10/07 20:11:01  mrimer
// Added BlitTileImagePart().
//
// Revision 1.14  2002/10/03 02:48:13  erikh2000
// Cleaned up testing code.
//
// Revision 1.13  2002/10/01 22:52:09  erikh2000
// Added some test routines for blitting.  Will come back and clean it up.
//
// Revision 1.12  2002/09/30 18:38:40  mrimer
// Added BAndWRect(), BAndWTile(), DarkenRect(), and DarkenTile() for room effects.
//
// Revision 1.11  2002/09/27 17:42:45  mrimer
// Added ShadeTile().  Made ShadeRect() more general purpose.
//
// Revision 1.10  2002/09/24 21:19:30  mrimer
// Added ShadeTile().
// Added opacity value to BlitTileImage().
// Made some parameters const.
//
// Revision 1.9  2002/07/18 20:17:42  mrimer
// Added loading of encrypted files with altered filenames.
//
// Revision 1.8  2002/07/02 23:56:11  mrimer
// Added file unencryption code.
//
// Revision 1.7  2002/06/14 00:41:46  erikh2000
// Changed static surfacecolor declarations to const to prevent errors.
//
// Revision 1.6  2002/05/21 21:31:35  erikh2000
// Simplified some color-setting code.
//
// Revision 1.5  2002/04/19 21:43:46  erikh2000
// Removed references to ScreenConstants.h.
//
// Revision 1.4  2002/04/14 00:25:02  erikh2000
// Bitmap manager will now anti-alias tiles based on the current style.
//
// Revision 1.3  2002/04/12 22:51:34  erikh2000
// Changed design of BM to use reference counting for bitmap loads.  New GetBitmapSurface() and ReleaseBitmapSurface() methods replace LoadBitmapSurface().
//
// Revision 1.2  2002/04/11 21:17:56  erikh2000
// Fixed a release build problem with reference to LogErr().
//
// Revision 1.1  2002/04/11 10:20:52  erikh2000
// Initial check-in.
//

