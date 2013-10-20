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

#ifndef DRODFONTMANAGER_H
#define DRODFONTMANAGER_H

#include <FrontEndLib/FontManager.h>

//Set of all fonts.
enum FONTTYPE
{
	F_Unspecified = FONTLIB::F_Unspecified,
	F_Message = FONTLIB::F_Message,
	F_Small = FONTLIB::F_Small,
	F_FrameCaption = FONTLIB::F_FrameCaption,
	F_ListBoxItem = FONTLIB::F_ListBoxItem,
	F_SelectedListBoxItem = FONTLIB::F_SelectedListBoxItem,
	F_Button = FONTLIB::F_Button,
	F_ButtonWhite = FONTLIB::F_ButtonWhite,
	F_Button_Disabled = FONTLIB::F_Button_Disabled,
	F_FrameRate = FONTLIB::F_FrameRate,
	F_FlashMessage_1 = FONTLIB::F_FlashMessage_1,
	F_FlashMessage_2 = FONTLIB::F_FlashMessage_2,
	F_Header,
	F_Title,
	F_Scroll,
	F_Sign,
	F_LevelName,
	F_LevelInfo,
	F_LevelDescription,
	F_Lyric,
	F_CreditsText,
	F_CreditsHeader,
	F_CreditsSubheader,
	F_BeethroSpeech,
	F_AudienceSpeech,

	F_Last
};

const UINT FONT_COUNT = F_Last;

//****************************************************************************
class CDrodFontManager : public CFontManager
{
public:
	CDrodFontManager();

	virtual UINT		Init();

private:
	virtual bool	LoadFonts();
};

//Define global pointer to the one and only CDrodFontManager object.
#ifndef INCLUDED_FROM_DRODFONTMANAGER_CPP
	extern CDrodFontManager *g_pTheDFM;
#endif

#endif //...#ifndef DRODFONTMANAGER_H

// $Log: DrodFontManager.h,v $
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.1  2003/05/22 23:35:03  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
// Revision 1.24  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.23  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.22  2002/11/15 02:43:44  mrimer
// Added a font for white button text.
//
// Revision 1.21  2002/09/03 21:36:18  erikh2000
// Fixed a bug that was preventing text being drawn at negative coords.
// Made changes to loaded font settings.
// Added a method to get the background color of a loaded font.
//
// Revision 1.20  2002/08/31 00:01:05  erikh2000
// Added font for displaying frame rate.
//
// Revision 1.19  2002/08/23 23:35:44  erikh2000
// Added font settings for credits screen text.
//
// Revision 1.18  2002/07/09 23:11:48  mrimer
// Added GetFontColor() and SetFontColor().
//
// Revision 1.17  2002/07/02 23:54:43  mrimer
// Added font caching routines.
//
// Revision 1.16  2002/06/23 10:49:32  erikh2000
// Changed GetTextRectHeight() to also return width of widest line.
// Added more fonts.
//
// Revision 1.15  2002/06/21 04:55:17  mrimer
// Renamed DrawText to DrawTextXY (to avoid windows conflict).
//
// Revision 1.14  2002/06/14 02:31:46  erikh2000
// Changed color of disabled button text.
//
// Revision 1.13  2002/06/09 06:39:08  erikh2000
// Twiddling.
//
// Revision 1.12  2002/06/07 18:18:49  mrimer
// Removed unneeded "using ..."
//
// Revision 1.11  2002/06/03 22:55:42  mrimer
// Inserted SDL files.
//
// Revision 1.10  2002/05/31 23:45:24  mrimer
// Added Maroon.
//
// Revision 1.9  2002/05/25 04:31:09  mrimer
// Added DrawHotkeyTextToLine().
// Consolidated specified SDL_Colors to .h file.
//
// Revision 1.8  2002/05/20 18:38:46  mrimer
// Added enabled/disabled flag to widget.
//
// Revision 1.7  2002/05/17 22:55:24  erikh2000
// Added support for drawing outlined text.
//
// Revision 1.6  2002/05/16 23:09:50  mrimer
// Added FlashMessageEffect and level cleared message.
//
// Revision 1.5  2002/04/29 00:10:14  erikh2000
// Added fonts for drawing list box text.
// Fonts can now be drawn with anti-aliasing.
// Revised attributes for some of the fonts.
//
// Revision 1.4  2002/04/25 09:33:12  erikh2000
// Added new "header" font and fiddled with fonts a bit.
//
// Revision 1.3  2002/04/20 08:21:41  erikh2000
// Added new fonts and ability to override line skip and word spacing associated with TTF.
//
// Revision 1.2  2002/04/19 21:52:03  erikh2000
// Added GetFontHeight() method.
//
// Revision 1.1  2002/04/16 10:38:20  erikh2000
// Initial check-in.
//
