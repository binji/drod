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
 * Portions created by the Initial Developer are Copyright (C)
 * 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "CreditsScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodScreenManager.h"
#include "DrodSound.h"
#include <FrontEndLib/ScrollingTextWidget.h>
#include "../DRODLib/Db.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Wchar.h>

float				CCreditsScreen::fScrollRateMultiplier = 1.0f;
UINT				CCreditsScreen::wNormalScrollRate = 33;

//
//Protected methods.
//

//************************************************************************************
CCreditsScreen::CCreditsScreen(void)
	: CDrodScreen(SCR_Credits)
	, pBackgroundSurface(NULL)
	, pScrollingText(NULL)
//Constructor.
{
}

//*****************************************************************************
void CCreditsScreen::Paint(
//Overridable method to paint the screen.  
//
//Params:
	bool bUpdateRect)				//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	//Blit the background graphic.
	SDL_BlitSurface(this->pBackgroundSurface, NULL, GetDestSurface(), NULL);

	PaintChildren();
	
	if (bUpdateRect) UpdateRect();
}

//************************************************************************************
bool CCreditsScreen::Load(void)
//Load resources for screen.
//
//Returns:
//True if successful, false if not.
{
	const UINT CY_CREDITS = 480;
	const int Y_CREDITS = 0;
	const int X_CREDITS = 100;
	const UINT CX_CREDITS = 440;

	ASSERT(!this->bIsLoaded);

	//Load background bitmap.
	this->pBackgroundSurface = g_pTheBM->GetBitmapSurface("Credits");
	if (!this->pBackgroundSurface) return false;

	this->pScrollingText = new CScrollingTextWidget(0L, X_CREDITS, Y_CREDITS, 
			CX_CREDITS, CY_CREDITS);
	this->pScrollingText->SetBackground(this->pBackgroundSurface);
	AddWidget(this->pScrollingText);
	
	//Set up the lyric labels.
	AddLyricLabels();

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//************************************************************************************
void CCreditsScreen::Unload(void)
//Unload resources for screen.
{ 
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pBackgroundSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Credits");
		this->pBackgroundSurface = false;
	}

	this->bIsLoaded = false;
}

//************************************************************************************
bool CCreditsScreen::SetForActivate(void)
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	HideCursor();

	HideLyricLabels();

	g_pTheSound->PlaySong(SONGID_CREDITS);

    //Fix up scrolling text widget state in case screen was activated before.
	this->pScrollingText->ClearText();
    this->pScrollingText->ScrollAbsolute(0,0);
    this->pScrollingText->Show();

//Add some text to the scrolling text widget.
#	define A_TEXT(mid) \
		this->pScrollingText->AddText(g_pTheDB->GetMessageText(mid), F_CreditsText);\
      this->pScrollingText->AddText(wszCRLF, F_CreditsText)

	A_TEXT(MID_CreditsIntro);

    if (!g_pTheSound->IsMusicOn())
    {
        A_TEXT(MID_CreditsIntroNoSong);
    }

    A_TEXT(MID_Credits);

	if (g_pTheSound->IsMusicOn())
	{
		A_TEXT(MID_CreditsSong);
	}
	else
	{
		A_TEXT(MID_CreditsNoSong);
	}

	//Return to title screen when done.
	g_pTheSM->ClearReturnScreens();
	g_pTheSM->InsertReturnScreen(SCR_Title);

	return true;
}

#define SetRate(r) this->pScrollingText->SetScrollRate((UINT)\
	(r * this->wNormalScrollRate))

//*****************************************************************************************
void CCreditsScreen::OnKeyDown(
//
//Params:
	const DWORD dwTagNo, const SDL_KeyboardEvent &Key)
{ 
	HideCursor();

	CScreen::OnKeyDown(dwTagNo,Key); //For alt-F4, F10, etc.
	if (IsDeactivating()) {ShowCursor(); return;}

    //Ignore control keys below if song is playing.
    if (!this->pScrollingText->IsVisible()) return;

	//Check for other keys.
	switch (Key.keysym.sym)
	{
		case SDLK_PAUSE:
			//pause animation
			this->bPaused = true;
			SetRate(0);
			break;
		case SDLK_SPACE:
			//toggle pause animation unless song is playing.
			this->bPaused = !this->bPaused;
			SetRate(this->bPaused ? 0 : this->fScrollRateMultiplier);
			break;
		case SDLK_KP2: case SDLK_DOWN:
			//increase scroll rate
			this->bPaused = false;
			if (this->fScrollRateMultiplier < 4.0f) this->fScrollRateMultiplier += 0.10f;
			SetRate(this->fScrollRateMultiplier);
			break;
		case SDLK_KP8: case SDLK_UP:
			//slow scroll rate
			this->bPaused = false;
			if (this->fScrollRateMultiplier > 0.15f) this->fScrollRateMultiplier -= 0.10f;
			SetRate(this->fScrollRateMultiplier);
			break;
		default:
			//unpause
			this->bPaused = false;
			SetRate(this->fScrollRateMultiplier);
			break;
	}
}

//************************************************************************************
void CCreditsScreen::OnBetweenEvents(void)
//Handle events 
{	
	CScreen::OnBetweenEvents();

	//If the scrolling text widget is visible, then I am in the first stage.
	if (this->pScrollingText->IsVisible())
	{
		//Go on to second stage after the scrolling text is done.
		if (!this->pScrollingText->ContainsText())
		{
			if (g_pTheSound->IsMusicOn())	//Play the end song.
			{
				this->pScrollingText->Hide();

				//"11" -- This instrument plays notes for lyrics.
				g_pTheSound->PlaySong(SONGID_ENDOFTHEGAME, 11);
			}
			else								//No music--just exit.
			{
				GoToScreen(SCR_Return);
				return;
			}
		}
	}	
	else	//Second stage.
	{
		if (g_pTheSound->IsSongFinished())
		{
			GoToScreen(SCR_Return);
			return;
		}

		//Start showing lyrics once the "End of the Game" song begins playing.
		//Lyrics are synced to notes on a specific track.
		static UINT wCurrentNote = 0;
		UINT wNewNote = g_pTheSound->GetLyricNoteCount(); //Will always return 0 until EotG plays.
		if (wCurrentNote < wNewNote)
		{
			for (UINT wShowNote = wCurrentNote + 1; 
					wShowNote <= wNewNote; ++wShowNote)
				ShowLyricLabel(wShowNote - 1);
			wCurrentNote = wNewNote;
		}
	}
}

//*****************************************************************************
bool CCreditsScreen::OnQuit(void)
//Called when SDL_QUIT event is received.
//Returns whether Quit action was confirmed.
{
	//Pause action while the "Really quit?" dialog is activated.
	const bool bWasPaused = this->bPaused;
	if (!bWasPaused)
	{
		this->bPaused = true;
		SetRate(0);
	}

	const bool bQuit = CScreen::OnQuit();

	if (!bWasPaused)
	{
		this->bPaused = false;
		SetRate(this->fScrollRateMultiplier);
	} else {
		//redraw screen parts
		PaintChildren();
		UpdateRect();
	}

	return bQuit;
}

#undef SetRate

//
//Private methods.
//

//************************************************************************************
void CCreditsScreen::HideLyricLabels(void)
//Hides all the lyric labels.
{
	for (vector<CLabelWidget *>::iterator iSeek = this->LyricLabels.begin();
			iSeek != this->LyricLabels.end(); ++iSeek)
		(*iSeek)->Hide();
}

//************************************************************************************
void CCreditsScreen::AddLyricLabels(void)
//Add label widgets to display lyrics.
{
	CLabelWidget *pLabel;
	UINT wTextWidth, wTextHeight;
	const UINT CX_SPACE = 5;
   WSTRING wstr;

//Add word.
#define A_W() \
   wstr = WCStok(NULL, wszSpace); \
	g_pTheFM->GetTextWidthHeight(F_Lyric, wstr.c_str(), wTextWidth, wTextHeight); \
	pLabel = new CLabelWidget(0L, xPos, yPos, wTextWidth, wTextHeight, F_Lyric, wstr.c_str()); \
	pLabel->Hide(); \
	AddWidget(pLabel); \
	this->LyricLabels.push_back(pLabel); \
	xPos += wTextWidth + CX_SPACE;

//Add syllable. (no space after).
#define A_S() \
   wstr = WCStok(NULL, wszSpace); \
	g_pTheFM->GetTextWidthHeight(F_Lyric, wstr.c_str(), wTextWidth, wTextHeight); \
	pLabel = new CLabelWidget(0L, xPos, yPos, wTextWidth, wTextHeight, F_Lyric, wstr.c_str()); \
	pLabel->Hide(); \
	AddWidget(pLabel); \
	this->LyricLabels.push_back(pLabel); \
	xPos += wTextWidth;

//End of line.
#define A_EOL \
	yPos += wTextHeight; \
	xPos = xLineStart;

   //Prepare lyrics.
   WSTRING lyricStr = g_pTheDB->GetMessageText(MID_CreditsSongLyrics);
   WCHAR lyrics[1000];
   WCScpy(lyrics, lyricStr.c_str());
   wstr = WCStok(lyrics, wszSpace);

	//
	//Add the labels using above macros.  Each macro adds the next word/syllable.
	//
	
	int xPos = 150;
	int xLineStart = xPos;
	int yPos = 5;

	A_W() A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_W()
	
	xPos = xLineStart = 320;
	yPos = 120;

	A_W() A_W() A_EOL
	A_W() A_W() A_W() A_S() A_S() A_EOL
	A_W() A_W() A_W() 
	A_S() A_S() A_W() A_W() A_EOL
	A_W() A_W() A_W()
	
	xPos = xLineStart = 100;
	yPos = 240;

	A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_EOL
	A_W() A_W() A_W()
	
	xPos = xLineStart = 340;
	yPos = 280;

	A_W() A_W() A_W() A_S() A_W() A_W() A_EOL
	A_W() A_W() A_W() A_S() A_W() A_W() A_EOL
	A_S() A_W() A_W() A_W() A_W() A_W()
	
	xPos = 270;
	yPos = 440;
	A_W() A_W() A_W() A_W()

#undef A_EOL
#undef A_S
#undef A_W
}

//************************************************************************************
void CCreditsScreen::ShowLyricLabel(UINT wLabelNo)
{
	if (wLabelNo >= this->LyricLabels.size()) return; //Out of bounds.

	CLabelWidget *pLabel = this->LyricLabels[wLabelNo];
	pLabel->Show();
	pLabel->Paint();
}

// $Log: CreditsScreen.cpp,v $
// Revision 1.26  2003/08/12 22:46:33  erikh2000
// Changed credits text.
// Fixed a bug with credits text not appearing on second activation of credits screen.
// Fixed a bug with pausing in middle of end song.
//
// Revision 1.25  2003/07/03 21:44:51  mrimer
// Port warning/bug fixes (committed on behalf of Gerry JJ).
//
// Revision 1.24  2003/07/01 20:24:55  mrimer
// Moved all end of game text into DB.
//
// Revision 1.23  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.22  2003/06/20 00:40:47  mrimer
// Added mention of the level editor.
//
// Revision 1.21  2003/06/15 00:16:06  mrimer
// Reinserted compiled out end script.  Updated strings to work with Unicode fonts.
//
// Revision 1.20  2003/06/06 18:21:30  mrimer
// Revised some calls.
//
// Revision 1.19  2003/05/28 23:11:52  erikh2000
// TA_* constants are used differently.
//
// Revision 1.18  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.17  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.16  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.15  2003/02/17 03:28:08  erikh2000
// Removed L() macro.
//
// Revision 1.14  2003/02/17 00:51:05  erikh2000
// Removed L" string literals.
//
// Revision 1.13  2002/10/25 02:55:49  erikh2000
// End song only plays once now, and then sends player back to title screen.
//
// Revision 1.12  2002/10/25 01:39:45  erikh2000
// Updated credits.
//
// Revision 1.11  2002/10/10 01:29:16  erikh2000
// When music is disabled, some different control paths are taken.
//
// Revision 1.10  2002/10/04 17:53:41  mrimer
// Added pause and speed control to this screen.
// Added mouse cursor display logic.
//
// Revision 1.9  2002/09/03 21:33:32  erikh2000
// Changed calls to CScrollingTextWidget::AddText() to match new params.
// Moved background-painting code out of OnBetweenEvents() that is now contained in CScrollingTextWidget.
//
// Revision 1.8  2002/08/28 20:33:18  mrimer
// Tweaking.
//
// Revision 1.7  2002/08/23 23:28:19  erikh2000
// Added credits text.
// Added "end of the game" song with synched lyric labels.
//
// Revision 1.6  2002/08/01 17:27:13  mrimer
// Revised some screen transition and music setting logic.
//
// Revision 1.5  2002/07/29 17:15:22  mrimer
// Restore music to user setting once win sequence is done.
//
// Revision 1.4  2002/07/26 18:30:45  mrimer
// Credits screen now clears screen queue and returns to Title screen when done.
//
// Revision 1.3  2002/07/23 20:15:16  mrimer
// Various revisions.
//
// Revision 1.2  2002/07/22 21:03:55  mrimer
// Play credits song.  Added some sample text.
//
// Revision 1.1  2002/07/19 20:32:39  mrimer
// Initial check-in.
//
