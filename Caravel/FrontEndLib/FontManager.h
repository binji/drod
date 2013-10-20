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
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <BackEndLib/MessageIDs.h>
#include <BackEndLib/Types.h>
#include <BackEndLib/Wchar.h>

#include <SDL_ttf.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

typedef struct tagLoadedFont
{
	TTF_Font *	pTTFFont;		//Font object loaded by SDL_TTF.  Must be freed with
								//TTF_CloseFont().

	SDL_Color	ForeColor;		//Color of text.

	bool		bAntiAlias;		//If bAntiAlias is set, then BackColor will be used to
	SDL_Color	BackColor;		//to generate appropriate edge colors.

	bool		bOutline;		//If bOutline is set, then OutlineColor and 
	SDL_Color	OutlineColor;	//OutlineWidth will be used to draw outline around
	UINT		wOutlineWidth;	//text.

	UINT		wLineSkipHeight;//These are used in DrawTextToRect() to override
	UINT		wSpaceWidth;	//the default spacing provided with the font when
								//drawing.
} LOADEDFONT;

//Relevant characteristics for each loaded font.
typedef struct fontCacheMember {
	WSTRING wsFilename;
	UINT wPointsize;
	int nStyle;	//font style (e.g. normal, bold, italic, underline)
	TTF_Font* pFont;
   class CStretchyBuffer* pBuffer; // this can only be deleted when the font is done being used, so keep it here
}	FontCacheMember;

//pre-defined colors
const SDL_Color Black = {0, 0, 0, 0};
const SDL_Color DarkBrown = {138, 126, 103, 0};
const SDL_Color MediumBrown = {190, 181, 165, 0};
const SDL_Color LightGray = {180, 180, 180, 0};
const SDL_Color LightBrown = {205, 196, 179, 0};
const SDL_Color BlueishWhite = {229, 228, 245, 0};
const SDL_Color PinkishWhite = {232, 215, 219, 0};
const SDL_Color DarkYellow = {64, 64, 0, 0};
const SDL_Color MidYellow = {128, 128, 0, 0};
const SDL_Color Yellow = {255, 255, 0, 0};
const SDL_Color White = {255, 255, 255, 0};
const SDL_Color Maroon = {192, 0, 0, 0};
const SDL_Color Gold = {227, 171, 4, 0};
const SDL_Color Blue = {0, 0, 255, 0};

//Fonts needed, but can't place them in a final enumeration yet (i.e.,
//there will probably be more fonts used in the app).
//Give these values to the corresponding names in the final enumeration.
namespace FONTLIB
{
	enum FONTTYPE {
		F_Unspecified = -1,
		F_Message = 0,
		F_Small,
		F_FrameCaption,
		F_ListBoxItem,
		F_SelectedListBoxItem,
		F_Button,
		F_ButtonWhite,
		F_Button_Disabled,
		F_FrameRate,
		F_FlashMessage_1,
		F_FlashMessage_2
	};
};

//****************************************************************************
class CFontManager
{
public:
	CFontManager(void);
	virtual ~CFontManager(void);
	
   WSTRING     CalcPartialWord(const UINT eFontType, WCHAR *wczWord,
         const UINT wWordLen, const int nMaxWidth, UINT &wCharsNotDrawn) const;
	void			DrawTextXY(const UINT eFontType, const WCHAR *pwczText,
         SDL_Surface *pSurface, const int nX, const int nY,
         const UINT wWidth=0, const UINT wHeight=0) const;
	void			DrawTextToRect(const UINT eFontType, const WCHAR *pwczText, 
			int nX, int nY, UINT wW, UINT wH, SDL_Surface *pSurface) const;
	void			DrawHotkeyTextToLine(const UINT eFontType,
			const WCHAR *pwczText, int nX, int nY, UINT wW, SDL_Surface *pSurface) const;
	SDL_Color		GetFontBackColor(const UINT eFontType) const {return this->LoadedFonts[eFontType].BackColor;}
	SDL_Color		GetFontColor(const UINT eFontType) const {return this->LoadedFonts[eFontType].ForeColor;}
	UINT			GetFontHeight(const UINT eFontType) const;
   UINT        GetFontLineHeight(const UINT eFontType) const {return this->LoadedFonts[eFontType].wLineSkipHeight;}
	UINT			GetTextRectHeight(const UINT eFontType, const WCHAR *pwczText, 
			UINT wW, UINT &wLongestLineW, UINT &wH) const;
	void			GetTextWidthHeight(const UINT eFontType, const WCHAR *pwczText,
			UINT &wW, UINT &wH) const;
	void			GetWordWidth(const UINT eFontType, const WCHAR *pwczWord,
			UINT &wW) const;
	virtual UINT		Init()=0;
	void			SetFontColor(const UINT eFontType, SDL_Color color) {
			this->LoadedFonts[eFontType].ForeColor = color;}

protected:
	const WCHAR *	DrawText_CopyNextWord(const WCHAR *pwczStart,	
			WCHAR *wczWord, UINT &wWordLen) const;
	const WCHAR *	DrawText_SkipOverNonWord(const WCHAR *pwczStart, 
			UINT &wTrailSpaceCount,	UINT &wTrailCRLFCount) const;
   void        DrawPartialWord(const UINT eFontType, WCHAR *wczWord,
         const UINT wWordLen, const int xDraw, const int yDraw,
         const UINT wXLimit, SDL_Surface *pSurface,
         UINT& wCharsNotDrawn) const;
   TTF_Font* GetFont(WSTRING const &filename, const UINT pointsize,
			const int style=TTF_STYLE_NORMAL);
	SDL_Surface *	RenderWord(const UINT eFontType, const WCHAR *pwczText,
         const bool bRenderFast=false) const;

	virtual bool	LoadFonts()=0;
	vector<FontCacheMember> vFontCache;
	LOADEDFONT		*LoadedFonts;

	SDL_Surface *	pColorMapSurface;
};

//Define global pointer to the one and only CFontManager object.
#ifndef INCLUDED_FROM_FONTMANAGER_CPP
	extern CFontManager *g_pTheFM;
#endif

#endif //...#ifndef FONTMANAGER_H

// $Log: FontManager.h,v $
// Revision 1.11  2003/09/16 20:15:01  mrimer
// Added log comment.
//
