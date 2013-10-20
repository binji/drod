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

#ifndef BITMAPMANAGER_H
#define BITMAPMANAGER_H

#include "Colors.h"

#include <BackEndLib/MessageIDs.h>
#include <BackEndLib/Types.h>
#include <BackEndLib/Wchar.h>

#include <list>
#include <string>
using namespace std;

const UINT MAXLEN_BITMAPNAME = 256;
typedef struct tagLoadedBitmap
{
	WCHAR			wszName[MAXLEN_BITMAPNAME + 1];
	DWORD			dwRefCount;
	SDL_Surface *	pSurface;
} LOADEDBITMAP;

enum TILEIMAGETYPE
{
	TIT_Unspecified = -1,
	TIT_Opaque,
	TIT_Transparent
};

//****************************************************************************
class CBitmapManager
{
public:
	CBitmapManager();
	virtual ~CBitmapManager();
	
	void			BAndWRect(const UINT x, const UINT y, const UINT w, const UINT h,
			SDL_Surface *pDestSurface);
	void			BAndWTile(const UINT x, const UINT y, SDL_Surface *pDestSurface)
		{BAndWRect(x,y,CX_TILE,CY_TILE,pDestSurface);}
	void			BlitLayeredTileImage(const UINT wBottomTileImageNo,
			const UINT wTopTileImageNo, const UINT x, const UINT y,
			SDL_Surface *pDestSurface, const Uint8 nOpacity);
	void			BlitTileImage(const UINT wTileImageNo,
			const UINT x, const UINT y, SDL_Surface *pDestSurface,
			const Uint8 nOpacity=255, const bool bClippingNeeded=false);
	void			BlitTileImagePart(const UINT wTileImageNo, const UINT x,
			const UINT y, const UINT cxSource, const UINT cySource,
			SDL_Surface *pDestSurface, const Uint8 nOpacity=255);
	void			DarkenRect(const UINT x, const UINT y, const UINT w, const UINT h,
			const float fLightPercent, SDL_Surface *pDestSurface);
	void			DarkenTile(const UINT x, const UINT y,
			const float fLightPercent, SDL_Surface *pDestSurface)
		{DarkenRect(x,y,CX_TILE,CY_TILE,fLightPercent,pDestSurface);}
	WCHAR *		DoesStyleExist(const UINT dStyleNo);
	SDL_Surface *	GetBitmapSurface(const char *wszName);
	virtual UINT	Init() {return 0;}
	void			LockTileImagesSurface(void);
	void			ReleaseBitmapSurface(const char *pszName);
	void			ShadeRect(const UINT x, const UINT y, const UINT w, const UINT h,
			const SURFACECOLOR &Color, SDL_Surface *pDestSurface);
	void			ShadeTile(const UINT x, const UINT y,
			const SURFACECOLOR &Color, SDL_Surface *pDestSurface)
		{ShadeRect(x,y,CX_TILE,CY_TILE,Color,pDestSurface);}
	void			UnlockTileImagesSurface(void);

	static int	CX_TILE, CY_TILE;


protected:
	bool			DoesTileImageContainTransparentPixels(UINT wTileImageNo);
	LOADEDBITMAP *	FindLoadedBitmap(const WCHAR *pszName) const;
	void			GetBitmapPath(WSTRING &wstrPath) const;
	void			GetBitmapFilepath(const WCHAR *pszName, 
			WSTRING &wstrFilepath) const;
	bool			GetMappingIndexFromTileImageMap(const WCHAR *pszName, 
			list<UINT> &MappingIndex) const;
	const char *	GetMappingIndexFromTileImageMap_ParseToken(
			const char *pszSeek, UINT &wBeginTINo, UINT &wEndTINo, 
			UINT &wExcludeCount) const;
	const char *	GetMappingIndexFromTileImageMap_SeekPastDelimiters(
			const char *pszSeek) const;
	void			GetTileImageMapFilepath(const WCHAR *pszName, 
			WSTRING &wstrFilepath) const;
	SDL_Surface *	LoadBitmapSurface(const WCHAR *wszName);
	virtual bool	LoadTileImages(const WCHAR *pszName, bool bReplaceAntiAliasColors)=0;
	void			ReplaceAntiAliasingColors(const UINT wTileImageNo, 
			const SURFACECOLOR &Replace75, const SURFACECOLOR &Replace50);

	SDL_Surface *			pTileImagesSurface;
	TILEIMAGETYPE *		TileImageTypes;
	list<LOADEDBITMAP *>	LoadedBitmaps;
	SURFACECOLOR			TransparentColor;
	bool						bIsColorKeySet;

	UINT						wTileCount;

private:
	void			BlitTileImage_Trans(Uint8 *pSrc, Uint8 *pDest,
			const DWORD dwSrcPitch, const DWORD dwDestPitch);
};

//Define global pointer to the one and only CBitmapManager object.
#ifndef INCLUDED_FROM_BITMAPMANAGER_CPP
	extern CBitmapManager *g_pTheBM;
#endif

#endif //...#ifndef BITMAPMANAGER_H

// $Log: BitmapManager.h,v $
// Revision 1.3  2003/07/09 21:02:24  mrimer
// Fixed a crash (drawing to pixel memory outside of the surface).
//
// Revision 1.2  2003/05/25 22:44:35  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
