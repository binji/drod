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
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996, 
 * 1997, 2000, 2001, 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "WinAudienceScreen.h"
#include "DrodBitmapManager.h"

#include <algorithm>
#if defined _MSC_VER
#	define	Min	_MIN
#	define	Max	_MAX
#else
#	define	Min	std::min
#	define	Max	std::max
#endif

//Graphic image locations/dimensions.
const UINT CX_HEAD = 44;
const UINT CY_HEAD = 85;
const UINT X_HEADDEST = 534;
const UINT Y_HEADDEST = 0;
const UINT X_OPENHEADSRC = 648;
const UINT Y_OPENHEADSRC = 212;
const UINT X_CLOSEDHEADSRC = 693;
const UINT Y_CLOSEDHEADSRC = 212;
static SDL_Rect HeadDest = {X_HEADDEST, Y_HEADDEST, CX_HEAD, CY_HEAD};
static SDL_Rect OpenHeadSrc = {X_OPENHEADSRC, Y_OPENHEADSRC, CX_HEAD, CY_HEAD};
static SDL_Rect ClosedHeadSrc = {X_CLOSEDHEADSRC, Y_CLOSEDHEADSRC, CX_HEAD, CY_HEAD};

const UINT CX_ARM = 172;
const UINT CY_ARM = 142;
const UINT X_ARMDEST = 361;
const UINT Y_ARMDEST = 31;
const UINT X_ARM1SRC = 904;
const UINT Y_ARM1SRC = 1;
const UINT X_ARM2SRC = 904;
const UINT Y_ARM2SRC = 144;
const UINT X_ARM3SRC = 904;
const UINT Y_ARM3SRC = 287;
SDL_Rect ArmDest = {X_ARMDEST, Y_ARMDEST, CX_ARM, CY_ARM};
SDL_Rect ArmSrc[4] = {
	{X_ARMDEST, Y_ARMDEST, CX_ARM, CY_ARM},
	{X_ARM1SRC, Y_ARM1SRC, CX_ARM, CY_ARM},
	{X_ARM2SRC, Y_ARM2SRC, CX_ARM, CY_ARM},
	{X_ARM3SRC, Y_ARM3SRC, CX_ARM, CY_ARM}
};

const UINT CX_SPEECHSQUARE = 255;
const UINT CY_SPEECHSQUARE = 210;
const UINT X_SPEECHSQUAREDEST = 392;
const UINT Y_SPEECHSQUAREDEST = 275;
const UINT X_SPEECHSQUARESRC = 648;
const UINT Y_SPEECHSQUARESRC = 1;
SDL_Rect SpeechSquareDest = {X_SPEECHSQUAREDEST, Y_SPEECHSQUAREDEST,
		CX_SPEECHSQUARE, CY_SPEECHSQUARE};
SDL_Rect SpeechSquareSrc = {X_SPEECHSQUARESRC, Y_SPEECHSQUARESRC,
		CX_SPEECHSQUARE, CY_SPEECHSQUARE};

const UINT CX_EYEDEST = 4;
const UINT CY_EYEDEST = 2;
static SDL_Rect LeftEyeDest = {541, 47, CX_EYEDEST, CY_EYEDEST};
static SDL_Rect RightEyeDest = {554, 48, CX_EYEDEST, CY_EYEDEST};

const UINT CX_PUPIL = 2;
const UINT CY_PUPIL = 1;
SURFACECOLOR PupilColor = {90, 76, 77};

//
//Protected methods.
//

//*****************************************************************************
void CWinAudienceScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool /*bUpdateRect*/)	//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	PaintBackground();

	PaintChildren();

	DrawSpeechSquare();

   static bool bDrawnSilent = false;
	if (IsBeethroTalking())
   {
		DrawBeethroTalking();
      bDrawnSilent = false;
   } else {
      if (!bDrawnSilent)
		   DrawBeethroSilent();
      bDrawnSilent = true;
   }
	
	//if (bUpdateRect) UpdateRect();
}

//*****************************************************************************************
void CWinAudienceScreen::DrawBeethroTalking()
//Mouth moves, arm waves, eyes wander
{
	//Draw talking mouth.
	const Uint32 curTime = SDL_GetTicks();
	if (curTime - this->lLastMouthMove > 300)
	{
		if (!this->bPaused)
			this->bMouthOpen = !this->bMouthOpen;
		this->lLastMouthMove = curTime;
		SDL_BlitSurface(this->pWinSurface, (this->bMouthOpen ? &OpenHeadSrc
				: &ClosedHeadSrc), GetDestSurface(), &HeadDest);
		UpdateRect(HeadDest);
	}

	DrawRovingPupils();

	if (curTime - this->lLastArmMove > 1000)
	{
		DrawArm();
		this->lLastArmMove = curTime;
	}
}

//*****************************************************************************************
void CWinAudienceScreen::DrawBeethroSilent()
//Mouth not moving, arm not waving, eyes fixed
{
   //Draw stationary head (mouth) and pupils.
	SDL_BlitSurface(this->pWinSurface, &ClosedHeadSrc, GetDestSurface(), &HeadDest);
	UpdateRect(HeadDest);
	ResetPupils();
	DrawPupils();

	//Draw stationary arm.
	ResetArm();
	DrawArm();
}

//******************************************************************************
void CWinAudienceScreen::DrawRovingPupils()
//Randomly roving eyes.
{
	//Move eyes every 500ms.
	const Uint32 curTime = SDL_GetTicks();
	if (curTime - this->lLastPupilMove > 500 && !this->bPaused)
	{
		this->lLastPupilMove = curTime;

		const int xRand = (((int)rand()%3)-1);
		const int yRand = (((int)rand()%3)-1);
		CurLeftPupilDest.x += xRand;
		CurLeftPupilDest.y += yRand;
		CurRightPupilDest.x += xRand;
		CurRightPupilDest.y += yRand;
		//Bounds checking.  Let eyes go past bounds by a pixel or two.
		{
			//Left eye.
			const Sint16 xLeft=LeftEyeDest.x;
			const Sint16 xRight=xLeft+LeftEyeDest.w;
			CurLeftPupilDest.x = Min(Max(CurLeftPupilDest.x,xLeft),xRight);
			const Sint16 xTop=LeftEyeDest.y;
			const Sint16 xBot=xTop+LeftEyeDest.h-CurLeftPupilDest.h;
			CurLeftPupilDest.y = Min(Max(CurLeftPupilDest.y,xTop),xBot);
		}
		{
			//Right eye.
			const Sint16 xLeft=RightEyeDest.x;
			const Sint16 xRight=xLeft+RightEyeDest.w;
			CurRightPupilDest.x = Min(Max(CurRightPupilDest.x,xLeft),xRight);
			const Sint16 xTop=RightEyeDest.y;
			const Sint16 xBot=xTop+RightEyeDest.h-CurRightPupilDest.h;
			CurRightPupilDest.y = Min(Max(CurRightPupilDest.y,xTop),xBot);
		}
	}

	DrawPupils();
}

//******************************************************************************
void CWinAudienceScreen::DrawPupils()
{
	DrawRect(CurLeftPupilDest,PupilColor);
	UpdateRect(CurLeftPupilDest);
	DrawRect(CurRightPupilDest,PupilColor);
	UpdateRect(CurRightPupilDest);
}

//******************************************************************************
void CWinAudienceScreen::ResetPupils()
//Put in position looking at audience.
{
	CurLeftPupilDest = LeftEyeDest;
	CurLeftPupilDest.y += 1;	//looking at the audience
	CurLeftPupilDest.w = CX_PUPIL;
	CurLeftPupilDest.h = CY_PUPIL;
	CurRightPupilDest = RightEyeDest;
	CurRightPupilDest.y += 1;
	CurRightPupilDest.w = CX_PUPIL;
	CurRightPupilDest.h = CY_PUPIL;
}

//******************************************************************************
void CWinAudienceScreen::DrawArm()
//Draw arm in non-resting position only once before going back to resting
{
	if (IsBeethroTalking() && !this->bPaused)
		this->nArmFrame = (int)rand()%3 + 1;
	SDL_BlitSurface(this->pWinSurface, &ArmSrc[this->nArmFrame],
			GetDestSurface(), &ArmDest);
	UpdateRect(ArmDest);
}

//******************************************************************************
void CWinAudienceScreen::DrawSpeechSquare()
{
	SDL_BlitSurface(this->pWinSurface, &SpeechSquareSrc,
			GetDestSurface(), &SpeechSquareDest);
}

//******************************************************************************
bool CWinAudienceScreen::Load()
//Load resources for the screen.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	ASSERT(!this->pWinSurface);

	this->pWinSurface = g_pTheBM->GetBitmapSurface("AudienceView");
	if (!this->pWinSurface) return false;

	this->pScrollingText = new CScrollingTextWidget(0L, 412, 295, 215, 180, 16);
	this->pScrollingText->SetBackground(this->pWinSurface, 668, 21);
	this->pScrollingText->SetScrollRate(15);
	AddWidget(this->pScrollingText);

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//******************************************************************************
void CWinAudienceScreen::Unload()
//Unload resources for the screen.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pWinSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("AudienceView");
		this->pWinSurface = NULL;
	}

	this->bIsLoaded = false;
}

// $Log: WinAudienceScreen.cpp,v $
// Revision 1.12  2003/08/08 17:34:09  mrimer
// Updated obsolete struct name.
//
// Revision 1.11  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/07/01 20:26:48  mrimer
// Made repainting more efficient to reduce flicker.  Revised some code.
//
// Revision 1.9  2003/06/06 18:12:41  mrimer
// Removed some unneeded assertions.
//
// Revision 1.8  2003/05/22 23:35:36  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.7  2002/10/04 17:57:12  mrimer
// Added pause control to animation.
//
// Revision 1.6  2002/09/05 18:24:50  erikh2000
// Slowed down the scroll text rate.
//
// Revision 1.5  2002/09/04 21:47:56  mrimer
// Moved Animate() code into base win screen class.
//
// Revision 1.4  2002/09/03 23:51:55  mrimer
// Sped up screen animation by removing unnecessary UpdateRect's, etc.
//
// Revision 1.3  2002/09/03 21:43:33  erikh2000
// Got the first part of the script working--Beethro talks, audience talks, text scrolls, the screen switches from audience to room view, a room is shown in the room widget.
//
// Revision 1.2  2002/07/22 17:34:12  mrimer
// Fixed drawing pupils.
//
// Revision 1.1  2002/07/11 20:58:37  mrimer
// Initial check-in.
//
