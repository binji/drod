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

#define INCLUDED_FROM_FONTMANAGER_CPP
#include "FontManager.h"
#undef INCLUDED_FROM_FONTMANAGER_CPP
#include "Screen.h"
#include "ListBoxWidget.h"
#include "Colors.h"
#include "Outline.h"

#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

//Holds the only instance of CFontManager for the app.
CFontManager *g_pTheFM = NULL;

const UINT MAXLEN_WORD = 1024;

TTF_Font* DROD_OpenFontIndex( const WCHAR *file, int ptsize, long index );

//
//Public methods.
//
//*********************************************************************************
CFontManager::CFontManager()
	: vFontCache(0)	//init empty font cache
	, LoadedFonts(NULL)
   , pColorMapSurface(NULL)
//Constructor.
{
}

//*********************************************************************************
CFontManager::~CFontManager()
//Destructor.
{ 
	for (UINT wFontI=this->vFontCache.size(); wFontI--; )
   {
		TTF_CloseFont(this->vFontCache[wFontI].pFont);
      delete this->vFontCache[wFontI].pBuffer;
   }

	if (this->pColorMapSurface) SDL_FreeSurface(this->pColorMapSurface);

	delete[] this->LoadedFonts;
}

//*********************************************************************************
void CFontManager::DrawTextXY(
//Draw a line of text to a surface.
//
//Params:
	const UINT eFontType,		//(in)	Indicates font and associated settings.
	const WCHAR *pwczText,	   //(in)	Text to draw.
	SDL_Surface *pSurface,	   //(in)	Dest surface.
	const int nX, const int nY,//(in)	Dest coords.
	const UINT wWidth, const UINT wHeight)//(in)	optional size constraints.
const
{
	if (pwczText[0] == L'\0') return; //Nothing to do.

   const LOADEDFONT *pFont = &(this->LoadedFonts[eFontType]);
	ASSERT(pFont->pTTFFont);

	//Adjust drawing position for any spaces preceding the first word.
	const WCHAR *pwczSeek = pwczText;
	UINT wSpaceCount, wCRLFCount;
	pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);
   ASSERT(!wCRLFCount); //shouldn't have any CRs in a single line of text
   if (wCRLFCount) return; //Stop at CR, since only one line of text is being drawn.
   UINT xDraw = wSpaceCount * pFont->wSpaceWidth;

   //Each iteration draws one word to surface.
	SDL_Surface *pText = NULL;
	WCHAR wczWord[MAXLEN_WORD + 1];
	UINT wWordLen;
	while (*pwczSeek != '\0' && (!wWidth || xDraw < wWidth))
   {
		//Copy the next word into buffer.
		pwczSeek = DrawText_CopyNextWord(pwczSeek, wczWord, wWordLen);
		pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);

		//Render the word.
		pText = RenderWord(eFontType, wczWord);
		if (!pText) {ASSERTP(false, "Failed to render word."); return;}

      //Blit word to dest surface (clip if needed).
      const UINT width = wWidth ? wWidth - xDraw : pText->w;
      const UINT height = wHeight ? wHeight : pText->h;
		SDL_Rect src = {0, 0, width, height};
		SDL_Rect dest = {nX + xDraw, nY, width, height};
		SDL_BlitSurface(pText, &src, pSurface, &dest);

 	   xDraw += pText->w;
		SDL_FreeSurface(pText);

		if (wCRLFCount)
         return;  //Stop at CR, since only one line of text is being drawn.

      //Adjust drawing position for spaces found after word.
      xDraw += pFont->wSpaceWidth * wSpaceCount;
   }
}

//*********************************************************************************
WSTRING CFontManager::CalcPartialWord(
//Determine how much of a rendered word will fit within a certain width.
//
//Params:
	const UINT eFontType,		//(in)	Indicates font and associated settings.
   WCHAR *wczWord,            //(in)   Word to render.
   const UINT wWordLen,       //(in)   # chars in word.
   const int nMaxWidth,       //(in)   Allotted width.
   UINT &wCharsNotDrawn)      //(out)  The number of chars that still need to be drawn.
const
{
   WSTRING wstr;
   UINT wCounter = 0;
   UINT wWidth = 0;
   // Find out how many chars will fit in the allotted space
   if (nMaxWidth > 0)
   {
      while (wWidth < static_cast<UINT>(nMaxWidth) && wCounter < wWordLen)
      {
         wstr += wczWord[wCounter++];
         GetWordWidth(eFontType, wstr.c_str(), wWidth);
      }
      if (wCounter == wWordLen && wWidth < static_cast<UINT>(nMaxWidth))
      {
         //Everything fit.
         wCharsNotDrawn = 0;
         return wstr;
      }
   }
   if (wstr.size() == 0)
   {
      //No chars fit.
      wCharsNotDrawn = wWordLen;
      return wstr;
   }
   // Get rid of the last three characters, then add a dash
   if (wstr.size() > 3) { 
      wCounter -= 3;
      wstr.erase(wstr.end()-3, wstr.end());
      static const WCHAR dash = W_t('-');
      wstr += dash;
   }

   wCharsNotDrawn = wWordLen - wCounter;

   return wstr;
}

//*********************************************************************************
void CFontManager::DrawPartialWord(
//Blit as much of a word as will fit within a rectangle on a surface.
//
//Pre-Cond: there is sufficient height in the rectangle to render the word.
//
//Params:
	const UINT eFontType,		//(in)	Indicates font and associated settings.
   WCHAR *wczWord,            //(in)   Word to render.
   const UINT wWordLen,       //(in)   # chars in word.
   const int xDraw, const int yDraw,//(in)   Coord to start drawing word at.
   const UINT wXLimit,        //(in)	Dest rectangle x-pixel limit.
	SDL_Surface *pSurface,	   //(in)	Dest surface.
   UINT &wCharsNotDrawn)      //(out)  The number of chars that still need to be drawn.
const
{
   WSTRING wstr = CalcPartialWord(eFontType, wczWord, wWordLen,
         (int)wXLimit-xDraw, wCharsNotDrawn);
   if (wstr.size() == 0) return; //Nothing to render.

	//Render the word.
	SDL_Surface *pText = RenderWord(eFontType, wstr.c_str());
	if (!pText) {ASSERTP(false, "Failed to render word.(2)"); return;}

   //Blit word to dest surface (clip if needed).
   const UINT width = wXLimit ? wXLimit : pText->w;
   const UINT height = pText->h;
	SDL_Rect src = {0, 0, width, height};
	SDL_Rect dest = {xDraw, yDraw, width, height};
	SDL_BlitSurface(pText, &src, pSurface, &dest);

	SDL_FreeSurface(pText);
}

//*********************************************************************************
void CFontManager::DrawTextToRect(
//Draw text within a rectangle on a surface.
//
//Params:
	const UINT eFontType,		//(in)	Indicates font and associated settings.
	const WCHAR *pwczText,	//(in)	Text to draw.
	int nX, int nY,			//(in)	Dest coords.
	UINT wW, UINT wH,		//(in)	Dest width and height to draw within.
	SDL_Surface *pSurface)	//(in)	Dest surface.
const
{
	const LOADEDFONT *pFont = &(this->LoadedFonts[eFontType]);
	ASSERT(pFont->pTTFFont);
	if (pFont->wLineSkipHeight > wH) return; //No room to display any text.

	if (WCSlen(pwczText) == 0) return; //Nothing to do.
	
	//Adjust drawing position for any spaces and CRLFs preceding the first word.
	int xDraw = nX, yDraw = nY;
	const WCHAR *pwczSeek = pwczText;
	UINT wSpaceCount, wCRLFCount;
	pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);
	if (wCRLFCount)
		yDraw += (wCRLFCount * pFont->wLineSkipHeight);
	else
		xDraw += (wSpaceCount * pFont->wSpaceWidth);

	//Each iteration draws one word to surface.
	SDL_Surface *pText = NULL;
	WCHAR wczWord[MAXLEN_WORD + 1];
	UINT wWordLen;
	while (yDraw + (int)pFont->wLineSkipHeight <= (int)(nY + wH) && *pwczSeek != '\0')
	{
		//Copy the next word into buffer.
		pwczSeek = DrawText_CopyNextWord(pwczSeek, wczWord, wWordLen);

		//Render the text.
		if (pText) SDL_FreeSurface(pText);
		pText = RenderWord(eFontType, wczWord);
		if (!pText) {ASSERTP(false, "Failed to render word.(3)"); return;}

		//Does rendered text fit horizontally in rect after drawing point?
		if (static_cast<UINT>(xDraw + pText->w) <= nX + wW)
      {
         //Rendered text fits.
			//Blit word to dest surface.
			SDL_Rect src = {0, 0, pText->w, pText->h};
			SDL_Rect dest = {xDraw, yDraw, pText->w, pText->h};
			SDL_BlitSurface(pText, &src, pSurface, &dest);

			xDraw += pText->w;
      } else {
         //Would the text fit horizontally at the beginning of a row?
			if (static_cast<UINT>(pText->w) > wW) //No.
         {
				//Moving down to a new row won't help draw this text.  So
				//draw it char-by-char until one char doesn't fit.
            UINT wNumCharsLeft;
            DrawPartialWord(eFontType, wczWord, wWordLen, xDraw, yDraw, nX + wW,
               pSurface, wNumCharsLeft);
            pwczSeek -= wNumCharsLeft;

				//Jump down to next row.
				xDraw = nX;
				yDraw += pFont->wLineSkipHeight;
            continue;   //don't need to check for whitespace
			} //...Text won't fit on a row by itself.
			else
			{
				//Blit word to dest surface on next row,
            //if another row will fit in rectangle.
	         if (yDraw + (int)pFont->wLineSkipHeight * 2 <= (int)(nY + wH))
            {
				   xDraw = nX;
				   yDraw += pFont->wLineSkipHeight;
				   SDL_Rect src = {0, 0, pText->w, pText->h};
				   SDL_Rect dest = {xDraw, yDraw, pText->w, pText->h};
				   SDL_BlitSurface(pText, &src, pSurface, &dest);

				   xDraw += pText->w;
            } else {
               //Otherwise, just draw as much as will fit on this (last) row.
               UINT wNumCharsLeft;
               DrawPartialWord(eFontType, wczWord, wWordLen, xDraw, yDraw,
                     nX + wW, pSurface, wNumCharsLeft);

               xDraw = nX;
				   yDraw += pFont->wLineSkipHeight;
            }
			}
		}

		//Adjust drawing position for spaces and CRLFs found after word.
		pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);
		if (wCRLFCount)
		{
			xDraw = nX;
			yDraw += (pFont->wLineSkipHeight * wCRLFCount);
		}
		else
			xDraw += pFont->wSpaceWidth * wSpaceCount;
	} //...while yDraw is not past the rect.

	if (pText) SDL_FreeSurface(pText);
}

//*********************************************************************************
void CFontManager::DrawHotkeyTextToLine(
//Draw text with letter(s) highlighted, representing hotkeys.
//
//Params:
	const UINT eFontType,			//(in)	Indicates font and associated settings.
	const WCHAR *pwczText,	//(in)	Text to draw.
	int nX, int nY,					//(in)	Dest coords.
	UINT wW,								//(in)	Dest width to draw within.
	SDL_Surface *pSurface)	//(in)	Dest surface.
const
{
	LOADEDFONT *pFont = (LOADEDFONT*)&(this->LoadedFonts[eFontType]);
	const SDL_Color foreColor = pFont->ForeColor;
	const SDL_Color hotkeyColor = Maroon;
	ASSERT(pFont->pTTFFont);
	SDL_Surface *pText = NULL;
	const UINT wStrLen = WCSlen(pwczText);
	int xDraw = nX, yDraw = nY;
	bool bHotkey = false;	//whether next character is a hotkey and should be highlighted

	WCHAR *const wczChars = new WCHAR[WCSlen(pwczText) + 1];
	UINT wNumChars = 0;	//consecutive chars w/o hotkey

	//Render all chars between hotkeys at once.
	//Render hotkeys separately.
	for (UINT wCharI = 0; wCharI < wStrLen; ++wCharI)
	{
		if (pwczText[wCharI] == '&')
		{
			bHotkey = true;
			WCv(wczChars[wNumChars]) = '\0';
		} else {
			wczChars[wNumChars++] = pwczText[wCharI];

			//Set hotkey char to highlighted color.
			if (bHotkey)
			{
				if (foreColor.r == hotkeyColor.r && foreColor.g == hotkeyColor.g && foreColor.b == hotkeyColor.b)
					pFont->ForeColor = White;
				else pFont->ForeColor = hotkeyColor;
				WCv(wczChars[wNumChars]) = '\0';
			} else
				if (wCharI < wStrLen-1)
					continue;	//don't draw anything yet
				else
					WCv(wczChars[wNumChars]) = '\0';	//at end of text -- draw everything left
		}

		if (wNumChars == 0)
			continue;	//nothing to render

		//Render chars.
		pText = RenderWord(eFontType, wczChars);
		if (!pText) {ASSERTP(false,"Failed to render word.(4)"); return;}

		//Is char past right bound?
		ASSERT(static_cast<UINT>(xDraw + pText->w) <= nX + wW);

		//Blit char to dest surface.
		SDL_Rect src = {0, 0, pText->w, pText->h};
		SDL_Rect dest = {xDraw, yDraw, pText->w, pText->h};
		SDL_BlitSurface(pText, &src, pSurface, &dest);
		xDraw += pText->w;

		SDL_FreeSurface(pText);

		if (bHotkey && pwczText[wCharI-1] == '&')
		{
			bHotkey = false;
			pFont->ForeColor = foreColor;	//back to original color
		}

		wNumChars = 0;	//ready to collect chars for next string snippet
	}

	delete[] wczChars;
}

//*********************************************************************************
UINT CFontManager::GetTextRectHeight(
//Get height needed to draw text within a width.  The width of the longest line
//within the rect is also calculated.
//
//Params:
	const UINT eFontType,		//(in)	Indicates font and associated settings.
	const WCHAR *pwczText,	//(in)	Text to draw.
	UINT wW,				//(in)	Dest width to draw within.
	UINT &wLongestLineW,	//(out)	Width of longest line.
   UINT &wH)            //(out)  Height needed to draw text within a width
//
//Returns:
//The width of the last line.
const
{
	const LOADEDFONT *pFont = &(this->LoadedFonts[eFontType]);
	ASSERT(pFont->pTTFFont);

	//Adjust drawing position for any spaces and CRLFs preceding the first word.
	UINT xDraw = 0, yDraw = 0;
	const WCHAR *pwczSeek = pwczText;
	UINT wSpaceCount, wCRLFCount;
	pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);
	if (wCRLFCount)
		yDraw += (wCRLFCount * pFont->wLineSkipHeight);
	else
		xDraw += (wSpaceCount * pFont->wSpaceWidth);

	//Each iteration draws one word to surface.
	wLongestLineW = 0;
	SDL_Surface *pText = NULL;
	WCHAR wczWord[MAXLEN_WORD + 1];
	UINT wWordLen;
	while (*pwczSeek != '\0')
	{
		//Copy the next word into buffer.
		pwczSeek = DrawText_CopyNextWord(pwczSeek, wczWord, wWordLen);

		//Render the text.
		if (pText) SDL_FreeSurface(pText);
		pText = RenderWord(eFontType, wczWord, true);
		if (!pText) {ASSERTP(false, "Failed to render word.(5)"); return 0L;}

		//Does rendered text fit horizontally in rect after drawing point?
		if (xDraw + pText->w > wW) //No.
		{
			//Would the text fit horizontally at the beginning of a row?
			if (static_cast<UINT>(pText->w) > wW) //No.
         {
				//Moving down to a new row won't help draw this text.  So
				//draw it char-by-char until one char doesn't fit.
            UINT wCharI;
				for (wCharI = 0; wCharI < wWordLen; ++wCharI)
				{
					static WCHAR wczChar[2] = { W_t(0), W_t(0) };
					wczChar[0] = wczWord[wCharI];

					//Render the char.
					if (pText) SDL_FreeSurface(pText);
					pText = RenderWord(eFontType, wczChar, true);
					if (!pText) {ASSERTP(false, "Failed to render word.(6)"); return 0L;}

					//Is char past right bound?
					if (xDraw + pText->w > wW) //Yes.
						break;
					else
						xDraw += pText->w;
				}
            //Render the rest on the next line.
            pwczSeek -= wWordLen - wCharI + 1;

				//Jump down to next row.
				if (xDraw > wLongestLineW) wLongestLineW = xDraw;
				xDraw = 0;
				yDraw += pFont->wLineSkipHeight;
            continue;   //don't need to check for whitespace
			} //...Text won't fit on a row by itself.
			else
			{
				//Move down to next row.
	         if (xDraw > wLongestLineW) wLongestLineW = xDraw;
				xDraw = pText->w;
				yDraw += pFont->wLineSkipHeight;
			}
		} //...Rendered text does not fit horizontally within rect.
		else
			//Rendered text fits.
			xDraw += pText->w;

		//Adjust drawing position for spaces and CRLFs found after word.
		pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);
		if (wCRLFCount)
		{
			if (xDraw > wLongestLineW) wLongestLineW = xDraw;
			xDraw = 0;
			yDraw += (pFont->wLineSkipHeight * wCRLFCount);
		}
		else
			xDraw += pFont->wSpaceWidth * wSpaceCount;
	} //...while yDraw is not past the rect.

	if (pText) SDL_FreeSurface(pText);

	if (xDraw > wLongestLineW) wLongestLineW = xDraw;
	if (wLongestLineW > wW) wLongestLineW = wW; //Sometimes it's a few pixels over.

	wH = yDraw + pFont->wLineSkipHeight;

   return xDraw;
}

//*****************************************************************************
void CFontManager::GetTextWidthHeight(
//Get width and height of a line of rendered text.
//
//Params:
	const UINT eFontType,		//(in)	Indicates font and associated settings.
	const WCHAR *pwczText,	//(in)	Text to draw.
	UINT &wW, UINT &wH)		//(out)	Width and height of the text.
const
{
	//Empty text has no dimensions.
	if (WCSlen(pwczText)==0) {wW = 0; wH = 0; return;}

	const LOADEDFONT *pFont = &(this->LoadedFonts[eFontType]);
	ASSERT(pFont->pTTFFont);

	wH = pFont->wLineSkipHeight;

   //Adjust drawing position for any spaces preceding the first word.
	const WCHAR *pwczSeek = pwczText;
	UINT wSpaceCount, wCRLFCount;
	pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);
   ASSERT(!wCRLFCount); //shouldn't have any CRs in a single line of text
   if (wCRLFCount) return; //Stop at CR, since only one line of text is being drawn.
	wW = wSpaceCount * pFont->wSpaceWidth;

   //Each iteration draws one word to surface.
	SDL_Surface *pText = NULL;
	WCHAR wczWord[MAXLEN_WORD + 1];
	UINT wWordLen;
	while (*pwczSeek != '\0')
   {
		//Copy the next word into buffer.
		pwczSeek = DrawText_CopyNextWord(pwczSeek, wczWord, wWordLen);
		pwczSeek = DrawText_SkipOverNonWord(pwczSeek, wSpaceCount, wCRLFCount);

		//Render the word.
		pText = RenderWord(eFontType, wczWord, true);
		if (!pText) {ASSERTP(false,"Failed to render word.(7)"); return;}

 	   wW += pText->w;
		SDL_FreeSurface(pText);

		if (wCRLFCount)
         return;  //Stop at CR, since only one line of text is being drawn.

      //Adjust drawing position for spaces found after word.
      wW += pFont->wSpaceWidth * wSpaceCount;
   }
}

//*****************************************************************************
void CFontManager::GetWordWidth(
//Get width of a word of rendered text.
//NOTE: Call this to initialize font spacing width.
//
//Params:
	const UINT eFontType,		//(in)	Indicates font and associated settings.
	const WCHAR *wczWord,	//(in)	Text to draw.
	UINT &wW)		//(out)	Width of the text.
const
{
  	//Render the word.
	SDL_Surface *pText = RenderWord(eFontType, wczWord, true);
	if (!pText) {ASSERTP(false,"Failed to render word.(8)."); return;}

   wW = pText->w;
	SDL_FreeSurface(pText);
}

//*****************************************************************************
UINT CFontManager::GetFontHeight(
//Gets height of a font, not including line skip space.
//
//Params:
	const UINT eFontType)		//(in)	Indicates font and associated settings.
//
//Returns:
//The height.
const
{
	return TTF_FontHeight(this->LoadedFonts[eFontType].pTTFFont);
}

//*****************************************************************************
SDL_Surface * CFontManager::RenderWord(
//Renders text to a surface.  Uses rendering options associated with a font type.
//
//Params:
	const UINT eFontType,		//(in)	Font to use.
	const WCHAR *pwczText,	//(in)	Text to render.
   const bool bRenderFast) //(in)   Render fast, overriding anti-aliasing if
                           //    needed (default = false)
//
//Returns:
//Surface with rendered text or NULL if an error occurred.
const
{
	SDL_Surface *pText = NULL;

	ASSERT(WCSlen(pwczText));

	const LOADEDFONT *pFont = &(this->LoadedFonts[eFontType]);
	ASSERT(pFont->pTTFFont);

	//Draw the text.
	if (pFont->bAntiAlias && !bRenderFast)
	{
		pText = TTF_RenderUNICODE_Shaded(pFont->pTTFFont, reinterpret_cast<const Uint16*>(pwczText),
				pFont->ForeColor, pFont->BackColor);
		if (!pText) return NULL;

		//Set background color used for anti-aliasing to be transparent.
		Uint32 TransparentColor = SDL_MapRGB(this->pColorMapSurface->format, pFont->BackColor.r, 
					pFont->BackColor.g, pFont->BackColor.b);
		SDL_SetColorKey(pText, SDL_SRCCOLORKEY, TransparentColor);
	}
	else
	{
		//Note that next call will set a color key for non-text pixels in surface.  
		//This color key may be used by AddOutline().
		pText = TTF_RenderUNICODE_Solid(pFont->pTTFFont, reinterpret_cast<const Uint16*>(pwczText), pFont->ForeColor);
		if (!pText) return NULL;
	}

	//Outline the text if specified for font.
	if (pFont->bOutline)
	{
		ASSERT(pFont->wOutlineWidth > 0);
		AddOutline(pText, pFont->OutlineColor, pFont->wOutlineWidth);
	}

	return pText;
}

//*********************************************************************************
TTF_Font* CFontManager::GetFont(
//Returns: a pointer either to a previously cached font or a newly loaded
//font from a call to TTF_OpenFont().
//
//Params:
	WSTRING const &wstrFilename, const UINT pointsize, const int style)
{
	FontCacheMember FCM;
	TTF_Font *pFont;

	//Search for font in font cache.
	for (UINT nIndex=this->vFontCache.size(); nIndex--; )
	{
		FCM = this->vFontCache[nIndex];
		if (FCM.wPointsize == pointsize && FCM.nStyle == style
				&& FCM.wsFilename == wstrFilename)
			return FCM.pFont;	//found identical font -- return it
	}

	//Font not found in cache -- load it.
	CStretchyBuffer* pFontBuffer = new CStretchyBuffer;
	CFiles::ReadFileIntoBuffer(wstrFilename.c_str(), *pFontBuffer);
	pFont = TTF_OpenFontRW(SDL_RWFromMem((BYTE*)*pFontBuffer, pFontBuffer->Size()), 1, pointsize);

	if (style != TTF_STYLE_NORMAL)
		TTF_SetFontStyle(pFont, style);

	//Add font and relevant characteristics to cache.
	FCM.wsFilename = wstrFilename;
	FCM.wPointsize = pointsize;
	FCM.nStyle = style;
	FCM.pFont = pFont;
   FCM.pBuffer = pFontBuffer;
	this->vFontCache.push_back(FCM);
	return pFont;
}

//*********************************************************************************
const WCHAR *CFontManager::DrawText_CopyNextWord(
//Copy one word into word buffer.
//
//Params:
	const WCHAR *pwczStart,	//(in)	Place to begin copying word from.
	WCHAR *pwczWord,		//(out)	Word.
	UINT &wWordLen)			//(out)	Length of word.
//
//Returns:
//Read position after word.
const
{
	const WCHAR *pwczSeek = pwczStart;

	//Copy word into buffer.
	wWordLen = 0;
	WCHAR *pwczWrite = pwczWord;
	while (*pwczSeek != '\0' && *pwczSeek != ' ' && *pwczSeek != '\r' && *pwczSeek != '-')
	{
		++wWordLen;
		if (wWordLen > MAXLEN_WORD)  //Stop copying after max length.
			*pwczSeek++;
		else
			*(pwczWrite++) = *(pwczSeek++);
	}
   if (*pwczSeek == '-') {
		*(pwczWrite++) = *(pwczSeek++);
   }
	ASSERT(pwczWrite != pwczWord); //Bad call.
	pWCv(pwczWrite) = '\0';

	return pwczSeek;
}

//*********************************************************************************
const WCHAR *CFontManager::DrawText_SkipOverNonWord(
//Skip over non-word chars.
//
//Params:
	const WCHAR *pwczStart,	//(in)	Place to begin reading from.
	UINT &wSpaceCount,	//(out)	Number of space chars.
	UINT &wCRLFCount)	//(out)	Number of CRLF pairs.
//
//Returns:
//Read position after non-word chars.
const
{
	const WCHAR *pwczSeek = pwczStart;

	//Seek past non-word chars and count spaces and CRLFs.
	wSpaceCount = 0;
	wCRLFCount = 0;
	while (*pwczSeek == L' ' || *pwczSeek == '\r' || *pwczSeek == '\n')
	{
		if (*pwczSeek == L' ') ++wSpaceCount;
		if (*pwczSeek == L'\r') ++wCRLFCount;
		++pwczSeek;
	}

	return pwczSeek;
}

// $Log: FontManager.cpp,v $
// Revision 1.22  2005/03/15 21:54:33  mrimer
// Fixed memory leak.
//
// Revision 1.21  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.20  2003/09/16 19:02:39  schik
// Fixed memory leak
//
// Revision 1.19  2003/08/09 00:37:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.18  2003/08/01 17:22:03  mrimer
// Fixed bug: width of longest rendered line of text not set.
//
// Revision 1.17  2003/08/01 04:18:50  mrimer
// Fixed bug: char(s) missing when word wraps to next line, followed by other than one space.
// Added CalcPartialWord().
//
// Revision 1.16  2003/08/01 01:29:51  mrimer
// Fixed bug: text drawing outside of rectangle.
//
// Revision 1.15  2003/07/24 19:41:57  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.14  2003/07/22 18:41:41  mrimer
// Changed reinterpret_casts to static_casts.
//
// Revision 1.13  2003/07/19 02:16:42  mrimer
// Fixed some return values and bug: '-'s not being rendered.  Added GetFontLineHeight().  Changed API of GetTextRectHeight().
//
// Revision 1.12  2003/07/14 16:37:02  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/07/02 01:07:22  mrimer
// Fixed bug: access violation in text wrapping detection loop.  Fixed bug: RenderTextXY() drawing past indicated width limit.
//
// Revision 1.9  2003/06/26 13:15:08  schik
// Fixed DrawTextToRect() to wrap words that are too long to fit on one line (and don't have hyphens)
//
// Revision 1.8  2003/06/26 03:14:36  schik
// Scroll text will now be wrapped on dashes as well as spaces and newlines.
//
// Revision 1.7  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/06/13 06:00:22  mrimer
// Changed RenderText() to RenderWord().
// Modified all routines that call it to use correct font spacing width.
// Optimized some text size calculation and rendering routines.
//
// Revision 1.5  2003/06/09 19:22:16  mrimer
// Added optional max width/height params to FM::DrawTextXY().
//
// Revision 1.4  2003/06/03 06:23:19  mrimer
// Added DrawPartialWord().  Fixed rendering bugs when text goes past drawing rectangle.
//
// Revision 1.3  2003/05/30 04:06:22  mrimer
// Cleaned up some code.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//

