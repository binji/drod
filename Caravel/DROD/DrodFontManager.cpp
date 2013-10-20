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

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#define INCLUDED_FROM_DRODFONTMANAGER_CPP
#include "DrodFontManager.h"
#undef INCLUDED_FROM_DRODFONTMANAGER_CPP

#include <FrontEndLib/ListBoxWidget.h>
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Files.h>

//Holds the only instance of CDrodFontManager for the app.
CDrodFontManager *g_pTheDFM = NULL;

//*****************************************************************************
CDrodFontManager::CDrodFontManager()
	: CFontManager()
{
	this->LoadedFonts = new LOADEDFONT[FONT_COUNT];

	for (UINT wFontI = 0; wFontI < FONT_COUNT; ++wFontI)
		memset(&(this->LoadedFonts[wFontI]), 0, sizeof(this->LoadedFonts[wFontI]));
}

//*****************************************************************************
UINT CDrodFontManager::Init()
//Initializes the font manager.
//
//Returns:
//MID_Success or MID_* describing failure.
{
   static const WCHAR pwzFonts[] = { W_t('F'), W_t('o'), W_t('n'), W_t('t'), W_t('s'), W_t(0) };
	static const WCHAR pwzTomnr[] = { W_t('t'), W_t('o'), W_t('m'), W_t('n'), W_t('r'), W_t('.'), W_t('t'), W_t('t'), W_t('f'), W_t(0) };

   if ( TTF_Init() < 0 ) return MID_TTFInitFailed;
	atexit(TTF_Quit);

	//Create surface used for color-mapping.
	{
        CFiles Files;
		ASSERT(this->pColorMapSurface == NULL);
		WSTRING wstrFontFilepath = Files.GetResPath();
		wstrFontFilepath += wszSlash;
		wstrFontFilepath += pwzFonts;
		wstrFontFilepath += wszSlash;
		wstrFontFilepath += pwzTomnr;
		CStretchyBuffer fontBuffer;
		CFiles::ReadFileIntoBuffer(wstrFontFilepath.c_str(), fontBuffer);
		TTF_Font *pFont = TTF_OpenFontRW(SDL_RWFromMem((BYTE*)fontBuffer, fontBuffer.Size()), 1, 15);
		if (!pFont)
		{
			return MID_TTFInitFailed;
		}
		this->pColorMapSurface = TTF_RenderUNICODE_Solid(pFont, reinterpret_cast<const Uint16*>(wszSpace), Black);
		TTF_CloseFont(pFont);
		if (!this->pColorMapSurface)
		{
			return MID_TTFInitFailed;
		}
	}

	//Load all the fonts used by application.
	if (!LoadFonts())
	{
		return MID_TTFInitFailed;
	}

	return MID_Success;
}

//
//Private methods.
//

//*********************************************************************************
bool CDrodFontManager::LoadFonts(void)
{
   static const WCHAR pwzFonts[] = {W_t('F'),W_t('o'),W_t('n'),W_t('t'),W_t('s'),W_t(0)};
	static const WCHAR pwzTomnr[] = {W_t('t'),W_t('o'),W_t('m'),W_t('n'),W_t('r'),W_t('.'),W_t('t'),W_t('t'),W_t('f'),W_t(0)};
	static const WCHAR pwzEpilog[] = {W_t('e'),W_t('p'),W_t('i'),W_t('l'),W_t('o'),W_t('g'),W_t('.'),W_t('t'),W_t('t'),W_t('f'),W_t(0)};

   CFiles Files;
	WSTRING wstrFontFilepath = Files.GetResPath();
	wstrFontFilepath += wszSlash;
	wstrFontFilepath += pwzFonts;
	wstrFontFilepath += wszSlash;
	WSTRING wstrFont2Filepath = wstrFontFilepath;
	wstrFontFilepath += pwzTomnr;
	wstrFont2Filepath += pwzEpilog;

	//Note: The entire loaded font array has been zeroed, so it is not necessary 
	//to set every member of LOADEDFONT.

	//Load message font.
	UINT wSpaceWidth;
	TTF_Font *pFont = GetFont(wstrFontFilepath, 15);
	if (!pFont) return false;
	this->LoadedFonts[F_Message].pTTFFont = pFont;
	this->LoadedFonts[F_Message].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_Message, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_Message].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_Message].BackColor = LightBrown;
	this->LoadedFonts[F_Message].bAntiAlias = true;
		
	//Load small font.
	pFont = GetFont(wstrFontFilepath, 12);
	if (!pFont) return false;
	this->LoadedFonts[F_Small].pTTFFont = pFont;
	this->LoadedFonts[F_Small].wLineSkipHeight = 13;
	this->LoadedFonts[F_Small].wSpaceWidth = 3;
	this->LoadedFonts[F_Small].BackColor = LightBrown;
	this->LoadedFonts[F_Small].bAntiAlias = true;

	//Load header font.
	pFont = GetFont(wstrFontFilepath, 14, TTF_STYLE_BOLD);
	if (!pFont) return false;
	this->LoadedFonts[F_Header].pTTFFont = pFont;
	this->LoadedFonts[F_Header].wLineSkipHeight = 15;
	this->LoadedFonts[F_Header].wSpaceWidth = 4;
	this->LoadedFonts[F_Header].BackColor = LightBrown;
	this->LoadedFonts[F_Header].bAntiAlias = true;

	//Load button font.
	pFont = GetFont(wstrFontFilepath, 13);
	if (!pFont) return false;
	this->LoadedFonts[F_Button].pTTFFont = pFont;
	this->LoadedFonts[F_Button].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_Button, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_Button].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_Button].BackColor = LightBrown;
	this->LoadedFonts[F_Button].bAntiAlias = true;

	//Load button font (white text).
	pFont = GetFont(wstrFontFilepath, 13);
	if (!pFont) return false;
	this->LoadedFonts[F_ButtonWhite].pTTFFont = pFont;
	this->LoadedFonts[F_ButtonWhite].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_ButtonWhite, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_ButtonWhite].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_ButtonWhite].ForeColor = White;

	//Load disabled button font.
	pFont = GetFont(wstrFontFilepath, 13);
	if (!pFont) return false;
	this->LoadedFonts[F_Button_Disabled].pTTFFont = pFont;
	this->LoadedFonts[F_Button_Disabled].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_Button_Disabled, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_Button_Disabled].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_Button_Disabled].ForeColor = DarkBrown;
	this->LoadedFonts[F_Button_Disabled].BackColor = LightBrown;
	this->LoadedFonts[F_Button_Disabled].bAntiAlias = true;

	//Load title font.
	pFont = GetFont(wstrFont2Filepath, 24);
	if (!pFont) return false;
	this->LoadedFonts[F_Title].pTTFFont = pFont;
	this->LoadedFonts[F_Title].wLineSkipHeight = TTF_FontLineSkip(pFont);
	this->LoadedFonts[F_Title].BackColor = MediumBrown;
	this->LoadedFonts[F_Title].bAntiAlias = true;
	GetWordWidth(F_Title, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_Title].wSpaceWidth = wSpaceWidth;

	//Load frame caption font.
	pFont = GetFont(wstrFontFilepath, 13);
	if (!pFont) return false;
	this->LoadedFonts[F_FrameCaption].pTTFFont = pFont;
	this->LoadedFonts[F_FrameCaption].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_FrameCaption, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_FrameCaption].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_FrameCaption].BackColor = MediumBrown;
	this->LoadedFonts[F_FrameCaption].bAntiAlias = true;

	//Load scroll font.
	pFont = GetFont(wstrFontFilepath, 11);
	if (!pFont) return false;
	this->LoadedFonts[F_Scroll].pTTFFont = pFont;
	this->LoadedFonts[F_Scroll].wLineSkipHeight = 12;
	this->LoadedFonts[F_Scroll].wSpaceWidth = 3;
	this->LoadedFonts[F_Scroll].BackColor = BlueishWhite;
	this->LoadedFonts[F_Scroll].bAntiAlias = true;

	//Load sign font.
	pFont = GetFont(wstrFontFilepath, 16);
	if (!pFont) return false;
	this->LoadedFonts[F_Sign].pTTFFont = pFont;
	this->LoadedFonts[F_Sign].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_Sign, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_Sign].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_Sign].BackColor = BlueishWhite;
	this->LoadedFonts[F_Sign].bAntiAlias = true;

	//Load list box item font.
	pFont = GetFont(wstrFontFilepath, 12);
	if (!pFont) return false;
	this->LoadedFonts[F_ListBoxItem].pTTFFont = pFont;
	this->LoadedFonts[F_ListBoxItem].wLineSkipHeight = CY_LBOX_ITEM;
	this->LoadedFonts[F_ListBoxItem].wSpaceWidth = 4;
	this->LoadedFonts[F_ListBoxItem].BackColor = PinkishWhite;
	this->LoadedFonts[F_ListBoxItem].bAntiAlias = true;


	//Load selected list box item font.
	pFont = GetFont(wstrFontFilepath, 12);
	if (!pFont) return false;
	this->LoadedFonts[F_SelectedListBoxItem].pTTFFont = pFont;
	this->LoadedFonts[F_SelectedListBoxItem].wLineSkipHeight = CY_LBOX_ITEM;
	this->LoadedFonts[F_SelectedListBoxItem].wSpaceWidth = 4;
	this->LoadedFonts[F_SelectedListBoxItem].BackColor = MediumBrown;
	this->LoadedFonts[F_SelectedListBoxItem].bAntiAlias = true;

	//Load flashing message 1 font.
	pFont = GetFont(wstrFont2Filepath, 40);
	if (!pFont) return false;
	this->LoadedFonts[F_FlashMessage_1].pTTFFont = pFont;
	this->LoadedFonts[F_FlashMessage_1].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_FlashMessage_1, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_FlashMessage_1].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_FlashMessage_1].ForeColor = Yellow;
	this->LoadedFonts[F_FlashMessage_1].bOutline = true;
	this->LoadedFonts[F_FlashMessage_1].OutlineColor = Black;
	this->LoadedFonts[F_FlashMessage_1].wOutlineWidth = 2;

	//Load flashing message 2 font.
	pFont = GetFont(wstrFont2Filepath, 38);
	if (!pFont) return false;
	this->LoadedFonts[F_FlashMessage_2].pTTFFont = pFont;
	this->LoadedFonts[F_FlashMessage_2].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_FlashMessage_2, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_FlashMessage_2].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_FlashMessage_2].ForeColor = Yellow;
	this->LoadedFonts[F_FlashMessage_2].bOutline = true;
	this->LoadedFonts[F_FlashMessage_2].OutlineColor = Black;
	this->LoadedFonts[F_FlashMessage_2].wOutlineWidth = 2;

	//Load level name font.
	pFont = GetFont(wstrFontFilepath, 32);
	if (!pFont) return false;
	this->LoadedFonts[F_LevelName].pTTFFont = pFont;
	this->LoadedFonts[F_LevelName].wLineSkipHeight = GetFontHeight(F_LevelName);
	GetWordWidth(F_LevelName, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_LevelName].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_LevelName].ForeColor = Yellow;
	this->LoadedFonts[F_LevelName].BackColor = Black;
	this->LoadedFonts[F_LevelName].bAntiAlias = true;

	//Load level info font.
	pFont = GetFont(wstrFontFilepath, 16);
	if (!pFont) return false;
	this->LoadedFonts[F_LevelInfo].pTTFFont = pFont;
	this->LoadedFonts[F_LevelInfo].wLineSkipHeight = TTF_FontLineSkip(pFont);
	GetWordWidth(F_LevelInfo, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_LevelInfo].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_LevelInfo].ForeColor = Gold;
	this->LoadedFonts[F_LevelInfo].BackColor = Black;
	this->LoadedFonts[F_LevelInfo].bAntiAlias = true;

	//Load level description font.
	pFont = GetFont(wstrFontFilepath, 18);
	if (!pFont) return false;
	this->LoadedFonts[F_LevelDescription].pTTFFont = pFont;
	this->LoadedFonts[F_LevelDescription].wLineSkipHeight = 24;
	this->LoadedFonts[F_LevelDescription].wSpaceWidth = 5;
	this->LoadedFonts[F_LevelDescription].ForeColor = White;
	this->LoadedFonts[F_LevelDescription].BackColor = Black;
	this->LoadedFonts[F_LevelDescription].bAntiAlias = true;

	//Load lyric font.
	pFont = GetFont(wstrFontFilepath, 18);
	if (!pFont) return false;
	this->LoadedFonts[F_Lyric].pTTFFont = pFont;
	this->LoadedFonts[F_Lyric].wLineSkipHeight = 24;
	this->LoadedFonts[F_Lyric].wSpaceWidth = 5;
	this->LoadedFonts[F_Lyric].ForeColor = Yellow;
	this->LoadedFonts[F_Lyric].BackColor = Black;
	this->LoadedFonts[F_Lyric].bAntiAlias = true;

	//Load credits header font.
	pFont = GetFont(wstrFont2Filepath, 34);
	if (!pFont) return false;
	this->LoadedFonts[F_CreditsHeader].pTTFFont = pFont;
	this->LoadedFonts[F_CreditsHeader].wLineSkipHeight = GetFontHeight(F_CreditsHeader);
	GetWordWidth(F_CreditsHeader, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_CreditsHeader].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_CreditsHeader].ForeColor = Yellow;
	this->LoadedFonts[F_CreditsHeader].BackColor = Black;
	this->LoadedFonts[F_CreditsHeader].bAntiAlias = true;

	//Load credits subheader font.
	pFont = GetFont(wstrFont2Filepath, 30);
	if (!pFont) return false;
	this->LoadedFonts[F_CreditsSubheader].pTTFFont = pFont;
	this->LoadedFonts[F_CreditsSubheader].wLineSkipHeight = GetFontHeight(F_CreditsSubheader);
	GetWordWidth(F_CreditsSubheader, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_CreditsSubheader].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_CreditsSubheader].ForeColor = Gold;
	this->LoadedFonts[F_CreditsSubheader].BackColor = Black;
	this->LoadedFonts[F_CreditsSubheader].bAntiAlias = true;

	//Load credits text font.
	pFont = GetFont(wstrFont2Filepath, 26);
	if (!pFont) return false;
	this->LoadedFonts[F_CreditsText].pTTFFont = pFont;
	this->LoadedFonts[F_CreditsText].wLineSkipHeight = GetFontHeight(F_CreditsText);
	GetWordWidth(F_CreditsText, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_CreditsText].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_CreditsText].ForeColor = White;
	this->LoadedFonts[F_CreditsText].BackColor = Black;
	this->LoadedFonts[F_CreditsText].bAntiAlias = true;

	//Load Beethro speech font.
	pFont = GetFont(wstrFont2Filepath, 18);
	if (!pFont) return false;
	this->LoadedFonts[F_BeethroSpeech].pTTFFont = pFont;
	this->LoadedFonts[F_BeethroSpeech].wLineSkipHeight = GetFontHeight(F_BeethroSpeech);
	GetWordWidth(F_BeethroSpeech, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_BeethroSpeech].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_BeethroSpeech].ForeColor = Black;
	this->LoadedFonts[F_BeethroSpeech].BackColor = White;
	this->LoadedFonts[F_BeethroSpeech].bAntiAlias = true;

	//Load audience speech font.
	pFont = GetFont(wstrFont2Filepath, 18);
	if (!pFont) return false;
	this->LoadedFonts[F_AudienceSpeech].pTTFFont = pFont;
	this->LoadedFonts[F_AudienceSpeech].wLineSkipHeight = GetFontHeight(F_BeethroSpeech);
	GetWordWidth(F_AudienceSpeech, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_AudienceSpeech].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_AudienceSpeech].ForeColor = Blue;
	this->LoadedFonts[F_AudienceSpeech].BackColor = White;
	this->LoadedFonts[F_AudienceSpeech].bAntiAlias = true;

	//Load frame rate font.
	pFont = GetFont(wstrFont2Filepath, 26);
	if (!pFont) return false;
	this->LoadedFonts[F_FrameRate].pTTFFont = pFont;
	this->LoadedFonts[F_FrameRate].wLineSkipHeight = GetFontHeight(F_FrameRate);
	GetWordWidth(F_FrameRate, wszSpace, wSpaceWidth);
	this->LoadedFonts[F_FrameRate].wSpaceWidth = wSpaceWidth;
	this->LoadedFonts[F_FrameRate].ForeColor = Yellow;
	this->LoadedFonts[F_FrameRate].BackColor = Black;
	this->LoadedFonts[F_FrameRate].bAntiAlias = false;

	//Make sure all fonts were loaded.
#ifdef _DEBUG
	for (UINT wFontI = 0; wFontI < F_Last; ++wFontI)
	{
		ASSERT(this->LoadedFonts[wFontI].pTTFFont);
	}
#endif
	
	return true;
}

// $Log: DrodFontManager.cpp,v $
// Revision 1.15  2005/04/13 16:06:46  gjj
// Some small fixes
//
// Revision 1.14  2003/10/20 17:49:38  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.13  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.12  2003/09/16 19:00:45  schik
// Fixed memory leak
//
// Revision 1.11  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/07/24 19:45:56  mrimer
// Fixed incorrectly changed cast (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/07/22 18:59:58  mrimer
// Changed reinterpret_casts to static_casts.
//
// Revision 1.8  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/06/13 06:00:21  mrimer
// Changed RenderText() to RenderWord().
// Modified all routines that call it to use correct font spacing width.
// Optimized some text size calculation and rendering routines.
//
// Revision 1.5  2003/05/30 04:05:25  mrimer
// Tweaking.
//
// Revision 1.4  2003/05/28 23:13:11  erikh2000
// Methods of CFiles are called differently.
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
// Revision 1.30  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.29  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.28  2002/11/15 02:43:44  mrimer
// Added a font for white button text.
//
// Revision 1.27  2002/09/03 21:36:18  erikh2000
// Fixed a bug that was preventing text being drawn at negative coords.
// Made changes to loaded font settings.
// Added a method to get the background color of a loaded font.
//
// Revision 1.26  2002/08/31 00:01:04  erikh2000
// Added font for displaying frame rate.
//
// Revision 1.25  2002/08/23 23:35:44  erikh2000
// Added font settings for credits screen text.
//
// Revision 1.24  2002/07/05 10:35:30  erikh2000
// Removed some junk.
//
// Revision 1.23  2002/07/03 22:03:02  mrimer
// Increased description and created by font sizes.
//
// Revision 1.22  2002/07/02 23:54:43  mrimer
// Added font caching routines.
//
// Revision 1.21  2002/06/23 10:49:32  erikh2000
// Changed GetTextRectHeight() to also return width of widest line.
// Added more fonts.
//
// Revision 1.20  2002/06/21 05:07:21  mrimer
// Revised includes.
// Renamed DrawText to DrawTextXY.
//
// Revision 1.19  2002/06/14 02:31:46  erikh2000
// Changed color of disabled button text.
//
// Revision 1.18  2002/06/11 22:56:21  mrimer
// Fixed bug in hotkey highlighting.
//
// Revision 1.17  2002/06/07 18:21:38  mrimer
// Made frame caption font anti-aliased.
//
// Revision 1.16  2002/06/05 03:14:26  mrimer
// Fixed character spacing on buttons.
//
// Revision 1.15  2002/05/31 23:46:16  mrimer
// Added hotkey support.
//
// Revision 1.14  2002/05/25 04:31:09  mrimer
// Added DrawHotkeyTextToLine().
// Consolidated specified SDL_Colors to .h file.
//
// Revision 1.13  2002/05/21 21:34:36  erikh2000
// Added an ASSERT to make sure outline width is set.
//
// Revision 1.12  2002/05/20 18:37:41  mrimer
// Added disabled state to CButtonWidget.
//
// Revision 1.11  2002/05/17 22:55:24  erikh2000
// Added support for drawing outlined text.
//
// Revision 1.10  2002/05/17 09:25:00  erikh2000
// Changed title font to epilog.
//
// Revision 1.9  2002/05/17 00:33:53  erikh2000
// Put in some extra failure checks to avoid access violations during text draws.
// Changed back color of flash message fonts to black.  This shows the text better over light backgrounds.
//
// Revision 1.8  2002/05/16 23:09:50  mrimer
// Added FlashMessageEffect and level cleared message.
//
// Revision 1.7  2002/04/29 00:10:14  erikh2000
// Added fonts for drawing list box text.
// Fonts can now be drawn with anti-aliasing.
// Revised attributes for some of the fonts.
//
// Revision 1.6  2002/04/25 09:33:12  erikh2000
// Added new "header" font and fiddled with fonts a bit.
//
// Revision 1.5  2002/04/24 08:13:07  erikh2000
// Changed sizes and styles of some of the fonts.
//
// Revision 1.4  2002/04/20 08:21:41  erikh2000
// Added new fonts and ability to override line skip and word spacing associated with TTF.
//
// Revision 1.3  2002/04/19 21:51:50  erikh2000
// Added GetFontHeight() method.
// Fixed an error in the handling of drawing of one long word inside of a rectangle.
//
// Revision 1.2  2002/04/17 20:44:32  erikh2000
// Load Tom's New Roman font instead of the previous font.ttf.
//
// Revision 1.1  2002/04/16 10:38:20  erikh2000
// Initial check-in.
//
