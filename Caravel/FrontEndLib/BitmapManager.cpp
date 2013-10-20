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

#ifdef WIN32 //Many things will not compile w/o WIN32 API.  Fix them if you are porting.
#	include <windows.h> //Should be first include.
#	pragma warning(disable:4786)
#endif

#define INCLUDED_FROM_BITMAPMANAGER_CPP
#include "BitmapManager.h"
#undef INCLUDED_FROM_BITMAPMANAGER_CPP

#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>
#include <BackEndLib/Ports.h>

#include <SDL.h>

//Holds the only instance of CBitmapManager for the app.
CBitmapManager *g_pTheBM = NULL;

int CBitmapManager::CX_TILE = 0;
int CBitmapManager::CY_TILE = 0;

//
//Public methods.
//

//**********************************************************************************
CBitmapManager::CBitmapManager()
	: pTileImagesSurface(NULL)
	, TileImageTypes(NULL)
	, bIsColorKeySet(false)
	, wTileCount(0)
//Constructor.
{
}

//**********************************************************************************
CBitmapManager::~CBitmapManager()
//Destructor.
{
	//If this assertian fires, there has been a call to GetBitmapSurface() without
	//a matching ReleaseBitmapSurface().
	ASSERT(this->LoadedBitmaps.size()==0);

	if (this->pTileImagesSurface) 
		SDL_FreeSurface(this->pTileImagesSurface);

	delete[] this->TileImageTypes;
	this->wTileCount = 0;
}

//*******************************************************************************
inline void CBitmapManager::BlitTileImage_Trans(
	Uint8 *pSrc, Uint8 *pDest, const DWORD dwSrcPitch, const DWORD dwDestPitch)
{
	const DWORD dwSrcRowOffset = dwSrcPitch - (CX_TILE * 3);
	const DWORD dwDestRowOffset = dwDestPitch - (CX_TILE * 3);
	
#	define DP	if (*pSrc == 192) {pSrc += 3; pDest += 3;} \
				else { *(pDest++) = *(pSrc++); *(pDest++) = *(pSrc++); \
				*(pDest++) = *(pSrc++); }

#	define DR  DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP; \
				pSrc += dwSrcRowOffset; pDest += dwDestRowOffset

#	define DR_LAST	DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP

	//Copy pixels.
	for (int nRowI = CY_TILE-1; nRowI--; )
	{
		DR;
	}
	DR_LAST;

#	undef DR_LAST
#	undef DR
#	undef DP
}

//**********************************************************************************
void CBitmapManager::BlitLayeredTileImage(
//Optimized routine to blit two tile images to a surface, one on top of the other.
//
//Params:
	const UINT wBottomTileImageNo,	//(in)	Bottom tile image to blit.
	const UINT wTopTileImageNo,		//(in)	Top tile image to blit.
	const UINT x, const UINT y,		//(in)	Coords on surface to blit.
	SDL_Surface *pDestSurface,			//(in)	Surface to blit to.
	const Uint8 nOpacity)	//(in)	Tile opacity (0 = transparent, 255 = opaque).
{
	ASSERT(!SDL_MUSTLOCK(pDestSurface));
	ASSERT(this->TileImageTypes[wBottomTileImageNo] == TIT_Opaque);
	ASSERT(nOpacity==255);
	if (this->TileImageTypes[wTopTileImageNo] != TIT_Transparent)
	{
		BlitTileImage(wTopTileImageNo,x,y,pDestSurface);
		return;
	}

	Uint8 *pBottomSrc = (Uint8 *)this->pTileImagesSurface->pixels +
			(wBottomTileImageNo * CX_TILE * 3);
	Uint8 *pTopSrc = (Uint8 *)this->pTileImagesSurface->pixels +
			(wTopTileImageNo * CX_TILE * 3);
	Uint8 *pDest = (Uint8 *)pDestSurface->pixels +
			(y * pDestSurface->pitch) + (x * 3);

	const DWORD dwSrcRowOffset = this->pTileImagesSurface->pitch - (CX_TILE * 3);
	const DWORD dwDestRowOffset = pDestSurface->pitch - (CX_TILE * 3);

	//For each destination pixel in tile
	//	if corresponding source t-layer pixel is transparent 
	//		copy source o-layer pixel to destination pixel 
	//	else 
	//		copy source t-layer pixel to destination pixel 
#	define DP	if (*pTopSrc == 192) {*(pDest++) = *(pBottomSrc++);\
				*(pDest++) = *(pBottomSrc++); *(pDest++) = *(pBottomSrc++);\
				pTopSrc += 3;}\
				else { *(pDest++) = *(pTopSrc++); *(pDest++) = *(pTopSrc++);\
				*(pDest++) = *(pTopSrc++); pBottomSrc += 3;}

#	define DR  DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP; \
				pBottomSrc += dwSrcRowOffset; pTopSrc += dwSrcRowOffset;\
				pDest += dwDestRowOffset

#	define DR_LAST	DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP;DP

	for (int nRowI = CY_TILE-1; nRowI--; )
	{
		DR;
	}
	DR_LAST;

#	undef DR_LAST
#	undef DR
#	undef DP
}

//**********************************************************************************
void CBitmapManager::BlitTileImage(
//Blits tile image to a surface.
//
//Params:
	const UINT wTileImageNo,			//(in)	Tile image to blit.
	const UINT x, const UINT y,		//(in)	Coords on surface to blit.
	SDL_Surface *pDestSurface,			//(in)	Surface to blit to.
	const Uint8 nOpacity,	//(in)	Tile opacity (0 = transparent, 255 = opaque).
   const bool bClippingNeeded)   //(in) if true, don't attempt to draw directly
                                 //to pixel memory (default = false)
{
	ASSERT(wTileImageNo < this->wTileCount);
	ASSERT(this->TileImageTypes[wTileImageNo] != TIT_Unspecified); //Unloaded tile image.

	//Handle transparent blitting using direct pixel access.
	if (this->TileImageTypes[wTileImageNo] == TIT_Transparent && nOpacity==255 &&
      !bClippingNeeded)
	{
      ASSERT(static_cast<int>(x) < pDestSurface->w && static_cast<int>(y) < pDestSurface->h);
		if (SDL_MUSTLOCK(pDestSurface))
		{
			if ( SDL_LockSurface(pDestSurface) <= 0 )
			{
				//This can happen during a debug session.
				return;	//Don't try to draw anything if I can't lock the surface.
			}
		}
		
		Uint8 *pSrc = (Uint8 *)this->pTileImagesSurface->pixels +
				(wTileImageNo * CX_TILE * 3);
		Uint8 *pDest = (Uint8 *)pDestSurface->pixels +
				(y * pDestSurface->pitch) + (x * 3);

		BlitTileImage_Trans(pSrc, pDest, this->pTileImagesSurface->pitch, 
				pDestSurface->pitch);
		
		if (SDL_MUSTLOCK(pDestSurface)) SDL_UnlockSurface(pDestSurface);

		return;
	}

	static const Uint32 TranspColor = SDL_MapRGB(this->pTileImagesSurface->format, 
			192, 192, 192);

	//Set color key as appropriate.
	if (this->TileImageTypes[wTileImageNo] == TIT_Transparent && !this->bIsColorKeySet)
	{
		SDL_SetColorKey(this->pTileImagesSurface, SDL_SRCCOLORKEY, TranspColor);
		this->bIsColorKeySet = true;
	}
	else if (this->TileImageTypes[wTileImageNo] == TIT_Opaque && this->bIsColorKeySet)
	{
		SDL_SetColorKey(this->pTileImagesSurface, 0, 0);
		this->bIsColorKeySet = false;
	}

	//Set coords.
	static SDL_Rect src = {0,0,CX_TILE,CY_TILE};
	static SDL_Rect dest = {0,0,CX_TILE,CY_TILE};
	dest.x = x;
	dest.y = y;
	src.x = (wTileImageNo * CX_TILE);

	//Set transparency level.  Blit.
	if (nOpacity<255)
	{
		SDL_SetAlpha(this->pTileImagesSurface,SDL_SRCALPHA,nOpacity);
		SDL_BlitSurface(this->pTileImagesSurface, &src, pDestSurface, &dest);
		SDL_SetAlpha(this->pTileImagesSurface,0,0); //Remove transparency level.
	} else
		SDL_BlitSurface(this->pTileImagesSurface, &src, pDestSurface, &dest);
}

//**********************************************************************************
void CBitmapManager::BlitTileImagePart(
//Blits part of a tile image to a surface.
//This is useful when only a portion of the tile is non-transparent pixels,
//and the pixels outside of a specified region are transparent and can be ignored.
//
//The code is mostly cut-and-pasted from BlitTileImage() above.
//
//Params:
	const UINT wTileImageNo,			//(in)	Tile image to blit.
	const UINT x, const UINT y,		//(in)	Coords on surface to blit.
	const UINT cxSource, const UINT cySource,	//(in) Amount of tile to blit.
	SDL_Surface *pDestSurface,			//(in)	Surface to blit to.
	const Uint8 nOpacity)	//(in)	Tile opacity (0 = transparent, 255 = opaque).
{
	static const Uint32 TranspColor = SDL_MapRGB(this->pTileImagesSurface->format, 
			192, 192, 192);

	//Set color key as appropriate.
	if (this->TileImageTypes[wTileImageNo] == TIT_Transparent && !this->bIsColorKeySet)
	{
		SDL_SetColorKey(this->pTileImagesSurface, SDL_SRCCOLORKEY, TranspColor);
		this->bIsColorKeySet = true;
	}
	else if (this->TileImageTypes[wTileImageNo] == TIT_Opaque && this->bIsColorKeySet)
	{
		SDL_SetColorKey(this->pTileImagesSurface, 0, 0);
		this->bIsColorKeySet = false;
	}

	//Set coords and size.
	static SDL_Rect src = {0,0,0,0};
	static SDL_Rect dest = {0,0,0,0};
	src.w = dest.w = cxSource;
	src.h = dest.h = cySource;
	dest.x = x;
	dest.y = y;
	src.x = (wTileImageNo * CX_TILE);

	//Set transparency level.  Blit.
	if (nOpacity<255)
	{
		SDL_SetAlpha(this->pTileImagesSurface,SDL_SRCALPHA,nOpacity);
		SDL_BlitSurface(this->pTileImagesSurface, &src, pDestSurface, &dest);
		SDL_SetAlpha(this->pTileImagesSurface,0,0); //Remove transparency level.
	} else
		SDL_BlitSurface(this->pTileImagesSurface, &src, pDestSurface, &dest);
}

//**********************************************************************************
void CBitmapManager::BAndWRect(
//Sets pixels to the black-and-white equivalent of their value.
//
//Params:
	const UINT x, const UINT y,	//(in)	Pixel coords.
	const UINT w, const UINT h,	//
	SDL_Surface *pDestSurface)		//(in)	Surface to blit to.
{
	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	const UINT wPixelByteNo = y * pDestSurface->pitch + (x * wBPP);
	const DWORD dwRowOffset = pDestSurface->pitch - (w * wBPP);

	if (SDL_MUSTLOCK(pDestSurface))
	{
		if ( SDL_LockSurface(pDestSurface) <= 0 )
		{
			ASSERTP(false, "Lock surface failed.");
			return;
		}
	}

	Uint8 *pSeek = (Uint8 *)pDestSurface->pixels + wPixelByteNo;
	Uint8 *const pStop = pSeek + (h * pDestSurface->pitch);

	Uint8 nValue;
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		Uint8 *const pEndOfRow = pSeek + (w * wBPP);

		//Each iteration modifies one pixel.
		while (pSeek != pEndOfRow)
		{
			//Set pixel to the black-and-white equivalent of its value.
			nValue = (Uint8)(((UINT)pSeek[0] + pSeek[1] + pSeek[2]) / 3);
			*(pSeek++) = nValue;
			*(pSeek++) = nValue;
			*(pSeek++) = nValue;
		}
		pSeek += dwRowOffset;
	}

	if (SDL_MUSTLOCK(pDestSurface)) SDL_UnlockSurface(pDestSurface);
}

//**********************************************************************************
void CBitmapManager::DarkenRect(
//Set pixel intensities to % of original.
//
//Params:
	const UINT x, const UINT y,	//(in)	Pixel coords.
	const UINT w, const UINT h,	//
	const float fLightPercent,			//(in)	% of brightness to retain
	SDL_Surface *pDestSurface)		//(in)	Surface to blit to.
{
	ASSERT(fLightPercent >= 0.0 && fLightPercent <= 1.0);

	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	const UINT wPixelByteNo = y * pDestSurface->pitch + (x * wBPP);
	const DWORD dwRowOffset = pDestSurface->pitch - (w * wBPP);

	if (SDL_MUSTLOCK(pDestSurface))
	{
		if ( SDL_LockSurface(pDestSurface) <= 0 )
		{
			ASSERTP(false, "Lock surface failed.(2)");
			return;
		}
	}

	Uint8 *pSeek = (Uint8 *)pDestSurface->pixels + wPixelByteNo;
	Uint8 *const pStop = pSeek + (h * pDestSurface->pitch);

	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		Uint8 *const pEndOfRow = pSeek + (w * wBPP);

		//Each iteration modifies one pixel.
		while (pSeek != pEndOfRow)
		{
			//Set pixel intensity to % of original.
			*(pSeek++) = (unsigned char)(fLightPercent * (*pSeek));
		}
		pSeek += dwRowOffset;
	}

	if (SDL_MUSTLOCK(pDestSurface)) SDL_UnlockSurface(pDestSurface);
}

//**********************************************************************************
WCHAR* CBitmapManager::DoesStyleExist(
//If a style # exists, returns a copy of the relative name of the file.
//
//Params:
	const UINT wStyleNo)	//(in)	Style to load.
//
//Returns:
//True if successful, false if not.
{	
   static const WCHAR wszSTYLE[] = { W_t('S'), W_t('t'), W_t('y'), W_t('l'), W_t('e'), W_t(0) };
	static const WCHAR wszTILES[] = { W_t('T'), W_t('i'), W_t('l'), W_t('e'), W_t('s'), W_t(0) };

   WCHAR wszNum[3];
	FILE* pFile;

	WSTRING wstrFilepath, wstrFilename = wszSTYLE;
	wstrFilename += _itoW(wStyleNo, wszNum, 10);
	wstrFilename += wszTILES;
	GetBitmapFilepath(wstrFilename.c_str(), wstrFilepath);
	pFile = CFiles::Open(wstrFilepath.c_str(), "r");
	if (NULL != pFile)
	{
		fclose(pFile);
		WSTRING wName = wszSTYLE;
		wName += _itoW(wStyleNo, wszNum, 10);
		wName += wszTILES;

		WCHAR *wStr = new WCHAR[wName.length()+1];
		wName.copy(wStr,wName.length());
		WCv(wStr[wName.length()]) = 0;
		return wStr;
	}

	return NULL;
}

//**********************************************************************************
void CBitmapManager::ShadeRect(
//Adds a color shade to tile at (x,y) on surface.
//
//Params:
	const UINT x, const UINT y,	//(in)	Pixel coords.
	const UINT w, const UINT h,	//
	const SURFACECOLOR &Color,		//(in)	Color to shade tile with.
	SDL_Surface *pDestSurface)		//(in)	Surface to blit to.
{
	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	const UINT wPixelByteNo = y * pDestSurface->pitch + (x * wBPP);
	const DWORD dwRowOffset = pDestSurface->pitch - (w * wBPP);

	if (SDL_MUSTLOCK(pDestSurface))
	{
		if ( SDL_LockSurface(pDestSurface) <= 0 )
		{
			ASSERTP(false, "Lock surface failed.(3)");
			return;
		}
	}

	Uint8 *pSeek = (Uint8 *)pDestSurface->pixels + wPixelByteNo;
	Uint8 *const pStop = pSeek + (h * pDestSurface->pitch);

	UINT nHue;
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		Uint8 *const pEndOfRow = pSeek + (w * wBPP);

		//Each iteration modifies one pixel.
		while (pSeek != pEndOfRow)
		{
			//Weighted average of current and mixing color (1:1).
			nHue = pSeek[0];
			nHue += Color.byt3;	//big endian order
			pSeek[0] = nHue/2;
			nHue = pSeek[1];
			nHue += Color.byt2;
			pSeek[1] = nHue/2;
			nHue = pSeek[2];
			nHue += Color.byt1;
			pSeek[2] = nHue/2;

			pSeek += wBPP;
		}
		pSeek += dwRowOffset;
	}

	if (SDL_MUSTLOCK(pDestSurface)) SDL_UnlockSurface(pDestSurface);
}

//**********************************************************************************
void CBitmapManager::LockTileImagesSurface(void)
{
	if (SDL_MUSTLOCK(this->pTileImagesSurface)) 
	{
		if ( SDL_LockSurface(this->pTileImagesSurface) <= 0 ) ASSERTP(false, "Lock surface failed.(4)");
	}
}

//**********************************************************************************
void CBitmapManager::UnlockTileImagesSurface(void)
{
	if (SDL_MUSTLOCK(this->pTileImagesSurface)) 
		SDL_UnlockSurface(this->pTileImagesSurface);
}

//**********************************************************************************
SDL_Surface * CBitmapManager::GetBitmapSurface(
//Gets a bitmap surface.  If bitmap is already loaded, increments a reference count.
//If bitmap isn't already loaded, loads the bitmap.
//
//Note: For every successful call to GetBitmapSurface(), a matching call to
//ReleaseBitmapSurface() must be made, or resources will not be freed and an 
//assertian will fire in the destructor.
//
//Params:
	const char *pszName) //(in)	Name of the bitmap, not including extension.
//
//Returns:
//Loaded surface if successful, or NULL if not.
{
	ASSERT(strlen(pszName) <= MAXLEN_BITMAPNAME);

	WSTRING wstr;
	AsciiToUnicode(pszName, wstr);
	LOADEDBITMAP *pBitmap = FindLoadedBitmap(wstr.c_str());
	if (pBitmap)	//Found it--increment ref count.
	{
		ASSERT(pBitmap->dwRefCount);
		ASSERT(pBitmap->pSurface);

		++pBitmap->dwRefCount;
	}
	else			//Didn't find it--load from file.
	{
		pBitmap = new LOADEDBITMAP;
		pBitmap->pSurface = LoadBitmapSurface(wstr.c_str());
		if (!pBitmap->pSurface)
		{
			//Couldn't load surface from file.
			delete pBitmap;
			return NULL;
		}
		WCScpy(pBitmap->wszName, wstr.c_str());
		pBitmap->dwRefCount = 1;
		this->LoadedBitmaps.push_back(pBitmap);
	}

	//Success.
	return pBitmap->pSurface;
}

//**********************************************************************************
void CBitmapManager::ReleaseBitmapSurface(
//Releases one reference count of a bitmap, and unloads the bitmap if nobody is
//using it anymore (ref count==0).
//
//Params:
	const char *pszName) //(in)	Name of the bitmap, not including extension.
{
	ASSERT(strlen(pszName) <= MAXLEN_BITMAPNAME);

	WSTRING wstr;
	AsciiToUnicode(pszName, wstr);
	LOADEDBITMAP *pBitmap = FindLoadedBitmap(wstr.c_str());

	ASSERT(pBitmap);
	ASSERT(pBitmap->dwRefCount > 0);

	//Decrement ref count.
	if (--(pBitmap->dwRefCount)==0)
	{
		//Unload the bitmap.
		SDL_FreeSurface(pBitmap->pSurface);
		this->LoadedBitmaps.remove(pBitmap);
		delete pBitmap;
	}
}

//
//Private methods.
//

//**********************************************************************************
bool CBitmapManager::DoesTileImageContainTransparentPixels(
//Scans pixels of a tile image for reserved transparent color.
//
//Params:
	UINT wTileImageNo)	//(in)	Tile image to scan.
//
//Returns:
//True if a transparent pixel is found, false if not.
{
	bool bFoundTransparent = true;
	const UINT wBPP = this->pTileImagesSurface->format->BytesPerPixel;

	//Faster to copy these to local vars.
	const BYTE bytT1 = this->TransparentColor.byt1;
	const BYTE bytT2 = this->TransparentColor.byt2;
	const BYTE bytT3 = this->TransparentColor.byt3;

	LockTileImagesSurface();

	//Shortcut: Just look at pixels in the four corners so that opaque tile image 
	//scans don't take as long.  Currently all transparent tile images have a 
	//transparent pixel in one of the corners.  Change routine if and when that changes.

	//Top-left corner.
	Uint8 *pSeek = (Uint8 *) this->pTileImagesSurface->pixels + 
			(CX_TILE * wTileImageNo * wBPP);
	if (pSeek[0] != bytT1 || pSeek[1] != bytT2 || pSeek[2] != bytT3)
	{
		//Top-right corner.
		pSeek += ((CX_TILE - 1) * wBPP);
		if (pSeek[0] != bytT1 || pSeek[1] != bytT2 || pSeek[2] != bytT3)
		{
			//Bottom-left corner.
			pSeek = (Uint8 *) this->pTileImagesSurface->pixels + 
				((CY_TILE - 1) * this->pTileImagesSurface->pitch) +
				(CX_TILE * wTileImageNo * wBPP);
			if (pSeek[0] != bytT1 || pSeek[1] != bytT2 || pSeek[2] != bytT3)
			{
				//Bottom-right corner.
				pSeek += ((CX_TILE - 1) * wBPP);
				bFoundTransparent = (pSeek[0] == bytT1 && pSeek[1] == bytT2 && 
						pSeek[2] == bytT3);
			}
		}
	}

	UnlockTileImagesSurface();
	return bFoundTransparent;
}

//**********************************************************************************
void CBitmapManager::ReplaceAntiAliasingColors(
//For every pixel matching a reserved anti-aliasing color replace it with
//a new pixel color designed to anti-alias for the current style.
//
//Params:
	const UINT wTileImageNo,			//(in)	Tile image to replace on.
	const SURFACECOLOR &Replace75,	//(in)	75% replacement color.
	const SURFACECOLOR &Replace50)	//(in)	50% replacement color.
{
	const UINT wBPP = this->pTileImagesSurface->format->BytesPerPixel;
	const SURFACECOLOR AntiAlias75 = 
			GetSurfaceColor(this->pTileImagesSurface, 0, 0, 128);
	const SURFACECOLOR AntiAlias50 = 
			GetSurfaceColor(this->pTileImagesSurface, 143, 143, 255);

	LockTileImagesSurface();

	Uint8 *pSeek = (Uint8 *) this->pTileImagesSurface->pixels +
			(CX_TILE * wTileImageNo * wBPP);
	Uint8 *const pStop = pSeek + (CY_TILE * this->pTileImagesSurface->pitch);
	const DWORD dwRowOffset = this->pTileImagesSurface->pitch - (CX_TILE * wBPP);
	
	//Each iteration search-and-replaces one row.
	while (pSeek != pStop)
	{
		ASSERT(pSeek < pStop);
		Uint8 *const pEndOfRow = pSeek + (CX_TILE * wBPP);

		//Each iteration search-and-replaces one pixel.
		while (pSeek != pEndOfRow)
		{
			if (pSeek[0] == AntiAlias75.byt1 &&
					pSeek[1] == AntiAlias75.byt2 &&
					pSeek[2] == AntiAlias75.byt3)
			{	//Found 75% pixel--replace with new color.
				pSeek[0] = Replace75.byt1;
				pSeek[1] = Replace75.byt2;
				pSeek[2] = Replace75.byt3;
			}
			else if (pSeek[0] == AntiAlias50.byt1 &&
					pSeek[1] == AntiAlias50.byt2 &&
					pSeek[2] == AntiAlias50.byt3)
			{	//Found 50% pixel--replace with new color.
				pSeek[0] = Replace50.byt1;
				pSeek[1] = Replace50.byt2;
				pSeek[2] = Replace50.byt3;
			}
			pSeek += wBPP;
		}
		pSeek += dwRowOffset;
	}

	UnlockTileImagesSurface();
}

//**********************************************************************************
LOADEDBITMAP * CBitmapManager::FindLoadedBitmap(
//Find a loaded bitmap by name.
//
//Params:
	const WCHAR *wszName) //(in)	Name of the bitmap, not including extension.
//
//Returns:
//Pointer to loaded bitmap structure or NULL if not found.
const
{
	for(list<LOADEDBITMAP *>::const_iterator iSeek = this->LoadedBitmaps.begin();
		iSeek != this->LoadedBitmaps.end(); ++iSeek)
	{
		if (WCScmp(wszName, (*iSeek)->wszName)==0) return *iSeek; //Found it.
	}

	//No match.
	return NULL;
}

//**********************************************************************************
SDL_Surface * CBitmapManager::LoadBitmapSurface(
//Loads a bitmap from the appropriate location into a new surface.
//
//Params:
	const WCHAR *wszName)	//(in)	Name of the bitmap, not including extension.
//
//Returns:
//Loaded surface if successful, or NULL if not.
{
	ASSERT(wszName);
	ASSERT(WCSlen(wszName) > 1);

	WSTRING wstrFilepath;
	GetBitmapFilepath(wszName, wstrFilepath);
	CFiles::GetTrueDatafileName(&*wstrFilepath.begin());
	SDL_Surface *pSurface;
	CStretchyBuffer buffer;
	CFiles::ReadFileIntoBuffer(&*wstrFilepath.begin(),buffer);
	if (CFiles::FileIsEncrypted(wstrFilepath.c_str()))
	{
		//Unprotect encrypted bitmap file.
		buffer.Decode();
	} 
	pSurface = SDL_LoadBMP_RW(SDL_RWFromMem((BYTE*)buffer,buffer.Size()), 0);
	if (!pSurface)
	{
		char szErrMsg[1024];
		char szFilename[MAX_PATH+1];
		UnicodeToAscii(wstrFilepath.c_str(), szFilename);
		sprintf(szErrMsg, "SDL_LoadBmp(\"%s\", ...) failed: %s",
				szFilename, SDL_GetError());
		LOGERR(szErrMsg);
		return NULL;
	}

	return pSurface;
}

//**********************************************************************************
void CBitmapManager::GetBitmapPath(
//Gets path where bitmaps are found.
//
//Params:
   WSTRING &wstrPath)	//(out)	The path.
const
{
   ASSERT(wstrPath.size() == 0);

   CFiles Files;
   wstrPath += Files.GetResPath();
   wstrPath += wszSlash;
   WSTRING wstrTmp;
   AsciiToUnicode("Bitmaps", wstrTmp);
   wstrPath += wstrTmp;
}

//**********************************************************************************
void CBitmapManager::GetBitmapFilepath(
//Gets full path to a bitmap.
//
//Params:
	const WCHAR *wszName,	//(in)	Name of bitmap without extension.
	WSTRING &wstrFilepath)	//(out)	The filepath.
const
{
   static const WCHAR wszBMP[] = { W_t('.'), W_t('b'), W_t('m'), W_t('p'), W_t(0) };

	ASSERT(wszName);
	ASSERT(WCSlen(wszName) > 0);
	ASSERT(wstrFilepath.size() == 0);

	GetBitmapPath(wstrFilepath);
	wstrFilepath += wszSlash;
	wstrFilepath += wszName;
	wstrFilepath += wszBMP;
}

//**********************************************************************************
void CBitmapManager::GetTileImageMapFilepath(
//Gets full path to a tile image map that corresponds to a bitmap.
//
//Params:
	const WCHAR *wszName,	//(in)	Name of bitmap without extension.
	WSTRING &wstrFilepath)	//(out)	The filepath to the tile image map.
const
{
   static const WCHAR wszTIM[] = { W_t('.'), W_t('t'), W_t('i'), W_t('m'), W_t(0) };

	ASSERT(wszName);
	ASSERT(WCSlen(wszName) > 0);
	ASSERT(wstrFilepath.size() == 0);

	GetBitmapPath(wstrFilepath);
	wstrFilepath += wszSlash;
	wstrFilepath += wszName;
	wstrFilepath += wszTIM;
}

//**********************************************************************************
bool CBitmapManager::GetMappingIndexFromTileImageMap(
//Gets a list of TI_* constants that correspond to tile images in a bitmap.  A
//.tim file with a filename matching the tile image bitmap is loaded and parsed into
//the list.
//
//Params:
	const WCHAR *wszName,		//(in)	Name of the bitmap with no file extension.
	list<UINT> &MappingIndex)	//(out)	List of TI_* constants.  First one is for
								//		topleft square in the bitmap, and then
								//		progresses by column and row.
//
//Returns:
//True if successful, false if not.
const
{
	//Load the .tim file into a buffer.
	WSTRING wstrFilepath;
	CStretchyBuffer Buffer;
	GetTileImageMapFilepath(wszName, wstrFilepath);
	if (!CFiles::ReadFileIntoBuffer(wstrFilepath.c_str(), Buffer))
		return false;

	//Parse the buffer for indices.  Here are the rules:
	//
	//Each TOKEN is delimited by any combination of ',', SPACE, CR, or LF.
	//
	//TOKEN is INCLUDE, INCLUDERANGE, or EXCLUDE
	//INCLUDE is one or more digits.
	//INCLUDERANGE is one or more digits, a hyphen, and one or more digits.
	//EXCLUDE is '!' and one or more digits.
	
	UINT wBeginTINo, wEndTINo, wExcludeCount;
	
	//Seek past delimeter chars.
	const char *pszSeek = (const char *)(BYTE *)Buffer;
	pszSeek = GetMappingIndexFromTileImageMap_SeekPastDelimiters(pszSeek);
	if (*pszSeek == '\0') return false; //No tokens in file, so the format is wrong.
	do
	{
		//Parse token.
		pszSeek = GetMappingIndexFromTileImageMap_ParseToken(pszSeek, 
				wBeginTINo, wEndTINo, wExcludeCount);
		if (!pszSeek) return false; //No token, so the format is wrong.

		//Was token of EXCLUDE type?
		if (wExcludeCount)
		{
			//Yes--add that number of TI_UNSPECIFIED indices to the index.
			for (UINT wExcludeI = 0; wExcludeI < wExcludeCount; ++wExcludeI)
				MappingIndex.push_back((UINT)-1);	//TI_UNSPECIFIED
		}
		else
		{
			//No--add range of indices to the index.  There may be only one 
			//indice in range.
			for (UINT wIncludeTINo = wBeginTINo; wIncludeTINo <= wEndTINo;
					++wIncludeTINo)
				MappingIndex.push_back(wIncludeTINo);
		}

		//Seek past delimeter chars.
		pszSeek = GetMappingIndexFromTileImageMap_SeekPastDelimiters(pszSeek);
	} while (*pszSeek != '\0');

	return true;
}

//**********************************************************************************
const char *CBitmapManager::GetMappingIndexFromTileImageMap_ParseToken(
//Parse a token from current position in buffer.  Should only be called from
//GetMappingIndexFromTileImageMap().
//
//Params:
	const char *pszSeek,		//(in)		Location to parse from.
	UINT &wBeginTINo,			//(out)		First TI# indicated by INCLUDE or
								//			INCLUDERANGE token.
	UINT &wEndTINo,				//(out)		Second TI# indicated by INCLUDERANGE
								//			token.  For INCLUDE token this will
								//			be same as wBeginTINo.
	UINT &wExcludeCount)		//(out)		Number of tile images to exclude from
								//			map.  Only set for EXCLUDE tokens.
//
//Returns:
//Pointer to char following token or NULL if token couldn't be parsed.
const
{
	const char *pszRet = pszSeek;
	const UINT MAX_DIGITS = 5;
	char szNumber[MAX_DIGITS + 1];
	UINT wCopyI;

	//Skip past any "junk chars".
	while (*pszRet != '\0' && *pszRet != '!' &&
			!(*pszRet >= '0' && *pszRet <= '9'))
		++pszRet;

	//Can't parse if at EOS.
	if (*pszRet == '\0') return NULL;

	//Is it an EXCLUDE token?
	if (*pszRet == '!')	//Yes.
	{
		++pszRet;

		//Copy numeric chars into buffer.
		for (wCopyI = 0; wCopyI < MAX_DIGITS; ++wCopyI)
		{
			if (!(*pszRet >= '0' && *pszRet <= '9')) break; //End of number found.
			szNumber[wCopyI] = *(pszRet++);
		}
		if (wCopyI == MAX_DIGITS) return NULL; //Number is too large.
		szNumber[wCopyI] = '\0';

		//Set return params.
		wExcludeCount = atoi(szNumber);
		if (wExcludeCount == 0) return NULL; //Exclude count of 0 is invalid.
		wBeginTINo = wEndTINo = (UINT)-1;	//TI_UNSPECIFIED;
	}
	else	//No, it's not an EXCLUDE token.
	{
		wExcludeCount = 0;

		//Copy numeric chars into buffer.
		for (wCopyI = 0; wCopyI < MAX_DIGITS; ++wCopyI)
		{
			if (!(*pszRet >= '0' && *pszRet <= '9')) break; //End of number found.
			szNumber[wCopyI] = *(pszRet++);
		}
		if (wCopyI == MAX_DIGITS) return NULL; //Number is too large.
		szNumber[wCopyI] = '\0';

		//Set begin TI# param from buffer.
		wBeginTINo = atoi(szNumber);

		//Is the next char a hyphen?
		if (*pszRet == '-')
		{
			//Yes, so this is an INCLUDERANGE token.
			++pszRet;

			//Copy numeric chars into buffer.
			for (wCopyI = 0; wCopyI < MAX_DIGITS; ++wCopyI)
			{
				if (!(*pszRet >= '0' && *pszRet <= '9')) break; //End of number found.
				szNumber[wCopyI] = *(pszRet++);
			}
			if (wCopyI == MAX_DIGITS) return NULL; //Number is too large.
			szNumber[wCopyI] = '\0';
			if (wCopyI == 0) return NULL; //Nothing usuable on right side of hyphen.

			//Set end TI# param from buffer.
			wEndTINo = atoi(szNumber);
		}
		else
			//No, so this is an INCLUDE token.  Set end TI# from begin TI#.
			wEndTINo = wBeginTINo;
	}

	//Skip past "junk chars" that are not delimeter chars.
	while (*pszRet != '\0' && *pszRet != '\r' && *pszRet != '\n' && *pszRet != ' ' 
			&& *pszRet != ',')
		++pszRet;

	return pszRet;
}

//**********************************************************************************
const char *CBitmapManager::GetMappingIndexFromTileImageMap_SeekPastDelimiters(
//Seek past any delimiter chars from current position in buffer.  Should only be 
//called from GetMappingIndexFromTileImageMap().
//
//Params:
	const char *pszSeek)		//(in)		Location to seek from.
//
//Returns:
//Pointer to char following delimeters.
const
{
	const char *pszRet = pszSeek;
	while (*pszRet == '\r' || *pszRet == '\n' || *pszRet == ' ' || *pszRet == ',')
		++pszRet;

	return pszRet;
}

// $Log: BitmapManager.cpp,v $
// Revision 1.12  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.11  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.8  2003/07/09 21:02:24  mrimer
// Fixed a crash (drawing to pixel memory outside of the surface).
//
// Revision 1.7  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/06/17 18:17:06  mrimer
// Fixed a bug in DarkenRect().
//
// Revision 1.5  2003/06/16 18:58:53  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.4  2003/05/28 23:01:17  erikh2000
// Calling GetDatPath() differently.
//
// Revision 1.3  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.2  2003/05/23 21:41:23  mrimer
// Added portability for APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
