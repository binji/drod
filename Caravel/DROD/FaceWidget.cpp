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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "FaceWidget.h"
#include "DrodBitmapManager.h"
#include "DrodSound.h"

#include "../DRODLib/GameConstants.h"
#include <BackEndLib/Assert.h>

//Face coords and dimensions.
const UINT CY_FACE_PAD = 1;
const UINT CX_FACE_PAD = 1;
#define yFace(ff) ( CY_FACE_PAD + ((ff) / 3) * (CY_FACE + CY_FACE_PAD) )
#define xFace(ff) ( CX_FACE_PAD + ((ff) % 3) * (CX_FACE + CX_FACE_PAD) )

//Locations of eye masks in the faces bitmap.  An eye mask defines the area a 
//pupil can move within and the parts of that area in which the pupil's pixel
//will be visible.
static SDL_Rect m_EyeMaskRectArray[Mood_Count] =
{
	{138, 413, 41, 10},	//Normal
	{138, 424, 41, 9},	//Aggressive
	{138, 434, 41, 12},	//Nervous
	{138, 470, 41, 9},	//Strike
	{138, 447, 41, 11},	//Happy
	{138, 480, 42, 14},	//Dieing
	{138, 459, 41, 10}	//Talking
};

//Offset from the top-left of the face widget to point where the top-left of the eye
//mask corresponds.
static const POINT m_EyeMaskOffsetArray[Mood_Count] =
{
	{20, 22},	//Normal
	{20, 22},	//Aggressive
	{20, 20},	//Nervous
	{20, 22},	//Strike
	{20, 21},	//Happy
	{20, 18},	//Dieing
	{20, 22}	//Talking
};

//Offset from the top-left of the eye mask to the center of the left pupil.
//Right pupil offset can be found from this by adding CX_BETWEEN_PUPILS.
static const POINT m_LeftPupilOffsetArray[Mood_Count] =
{
	{8, 5},	//Normal
	{8, 4},	//Aggressive
	{8, 6},	//Nervous
	{8, 4},	//Strike
	{8, 5},	//Happy
	{8, 7},	//Dieing
	{8, 5}	//Talking
};

//Pupil-related constants.
static const UINT CX_BETWEEN_PUPILS = 24;
static const int X_PUPIL = 138;
static const int Y_PUPIL = 398;
static const UINT CX_PUPIL = 5;
static const UINT CY_PUPIL = 4;
static const UINT CX_PUPIL_HALF = (CX_PUPIL / 2);
static const UINT CY_PUPIL_HALF = (CY_PUPIL / 2);

//
//Public methods.
//

//******************************************************************************
CFaceWidget::CFaceWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget 
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH)					//
	: CWidget(WT_Face, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
{
	this->pFacesSurface = NULL;
	this->eMood = this->ePrevMood = Normal;

	this->lDelayMood = lStartDelayMood = 0;
	this->bMoodDrawn = this->bIsReading = this->bIsBlinking = false;
	this->nPupilX = this->nPupilY = this->nPupilTargetX = this->nPupilTargetY = 0;
	this->eMoodSEID = SEID_NONE;
}

//****************************************************************************
//Sets the mood state of the face.
void CFaceWidget::SetMood(
//
//Params:
	MOOD eSetMood,					//(in) Mood to show
  Uint32 lDelay)          //(in) Amount of time in msecs to pass before mood reverts to previous.
{
	if (this->ePrevMood != eSetMood && !lDelayMood)	//keep track of previous mood for temporary changes in expression
		this->ePrevMood = this->eMood;	//only keep track of previous mood if it isn't a temporary one

	if (this->eMood != eSetMood) {
		//Mood has changed.
		this->eMood = eSetMood;
		this->bMoodDrawn = false;	//haven't drawn new mood yet
	}

	SetMoodDelay(lDelay);
}

//*****************************************************************************
void CFaceWidget::SetMoodToSoundEffect(
//Sets mood state of the face.  Mood will last until a sound effect finishes playing.
//
//Params:
	MOOD eSetMood,		//(in)	Mood to show.
	SEID eUntilSEID)	//(in)	Sound effect to show mood during.
{
	ASSERT(eUntilSEID > SEID_NONE && eUntilSEID < SEID_COUNT);

	this->eMoodSEID =  eUntilSEID;
	if (this->eMood != eSetMood) 
	{
		//Mood has changed.
		this->eMood = eSetMood;
		this->bMoodDrawn = false;	//haven't drawn new mood yet
	}
}

//*****************************************************************************
void CFaceWidget::SetMoodDelay(
//Changes mood for a specified amount of time.
//
//Accepts:
  Uint32 lDelay)          //(in) Amount of time in msecs to pass before mood reverts to previous.
{
  lDelayMood=lDelay;
  lStartDelayMood=SDL_GetTicks();
}

//****************************************************************************
void CFaceWidget::DrawPupils(void)
//Draws the pupils.
{
	SDL_Surface *pDestSurface = LockDestSurface();

	//Draw left pupil.
	const UINT CX_LEFT_PUPIL_OFFSET = 
		m_LeftPupilOffsetArray[this->eMood].x - CX_PUPIL_HALF + this->nPupilX;
	const UINT CY_PUPIL_OFFSET =
		m_LeftPupilOffsetArray[this->eMood].y - CY_PUPIL_HALF + this->nPupilY;
	
	DrawPupils_DrawOnePupil(pDestSurface,

		//Destination coords at which to draw left pupil.
		this->x + m_EyeMaskOffsetArray[this->eMood].x + CX_LEFT_PUPIL_OFFSET,
		this->y + m_EyeMaskOffsetArray[this->eMood].y + CY_PUPIL_OFFSET,

		//Coords to area of eye mask that corresponds to dest coords.
		m_EyeMaskRectArray[this->eMood].x + CX_LEFT_PUPIL_OFFSET,
		m_EyeMaskRectArray[this->eMood].y + CY_PUPIL_OFFSET
	);

	//Draw right pupil.
	const UINT CX_RIGHT_PUPIL_OFFSET = CX_LEFT_PUPIL_OFFSET + CX_BETWEEN_PUPILS;
	
	DrawPupils_DrawOnePupil(pDestSurface,
		
		//Destination coords at which to draw right pupil.
		this->x + m_EyeMaskOffsetArray[this->eMood].x + CX_RIGHT_PUPIL_OFFSET,
		this->y + m_EyeMaskOffsetArray[this->eMood].y + CY_PUPIL_OFFSET,

		//Coords to area of eye mask that corresponds to dest coords.
		m_EyeMaskRectArray[this->eMood].x + CX_RIGHT_PUPIL_OFFSET,
		m_EyeMaskRectArray[this->eMood].y + CY_PUPIL_OFFSET
	);

	UnlockDestSurface();
}

//****************************************************************************
void CFaceWidget::DrawPupils_DrawOnePupil(
//Draws one pupil to surface.  Should only be called by DrawPupils().
//
//Params:
	SDL_Surface *pDestSurface,			//(in)	Already-locked surface.
	const int nDestX, const int nDestY,	//(in)	Dest coords.
	const int nMaskX, const int nMaskY)	//(in)	Mask coords.
const
{
	//I am going to copy pixels from a source surface to a dest surface.
	//The source surface contains the pupil image.  A mask surface contains
	//pixels that are either black (0,0,0) or white (255,255,255).  If a
	//mask pixel is black, then I don't copy that pixel from source to dest.
	//Masked areas are basically the corners of the eye.

	//Dest coords should draw pupil entirely inside of the face widget area.
	ASSERT(nDestX >= this->x && nDestX + CX_PUPIL < this->x + this->w);
	ASSERT(nDestY >= this->y && nDestY + CY_PUPIL < this->y + this->h);

	//Mask coords should be inside of faces surface.
	ASSERT(nMaskX >= 0 && nMaskX + CX_PUPIL < static_cast<UINT>(this->pFacesSurface->w));
	ASSERT(nMaskY >= 0 && nMaskY + CY_PUPIL < static_cast<UINT>(this->pFacesSurface->h));

	//For speed, this routine is hardcoded to a 5 x 4 pupil and 3 BPP surfaces.
	ASSERT(CX_PUPIL == 5 && CY_PUPIL == 4);
	const UINT BPP = pDestSurface->format->BytesPerPixel;
	ASSERT(BPP == 3);

	//Set pixel pointers to starting locations.
	Uint8 *pMaskPixel = static_cast<Uint8 *>(this->pFacesSurface->pixels) +
			nMaskY * this->pFacesSurface->pitch +
			nMaskX * BPP;
	Uint8 *pSrcPixel = static_cast<Uint8 *>(this->pFacesSurface->pixels) +
			Y_PUPIL * this->pFacesSurface->pitch +
			X_PUPIL * BPP;
	Uint8 *pDestPixel = static_cast<Uint8 *>(pDestSurface->pixels) +
			nDestY * pDestSurface->pitch +
			nDestX * BPP;

	//These are used to advance pointers when they have reached end of rows.
	const UINT FACES_ROW_OFFSET = this->pFacesSurface->pitch - 
			((CX_PUPIL - 1) * BPP);
	const UINT DEST_ROW_OFFSET = pDestSurface->pitch - 
			((CX_PUPIL - 1) * BPP);

	//Macro to copy one pixel if not masked and advance pointers to 
	//next pixel in row.
#	define COPYPIXEL \
		if (*pMaskPixel) \
		{ \
			pDestPixel[0] = pSrcPixel[0]; \
			pDestPixel[1] = pSrcPixel[1]; \
			pDestPixel[2] = pSrcPixel[2]; \
		} \
		pSrcPixel += BPP; \
		pMaskPixel += BPP; \
		pDestPixel += BPP;

	//Macro to copy last pixel in row if not masked and advance pointers to 
	//first pixel in next row.
#	define COPYLASTPIXEL \
		if (*pMaskPixel) \
		{ \
			pDestPixel[0] = pSrcPixel[0]; \
			pDestPixel[1] = pSrcPixel[1]; \
			pDestPixel[2] = pSrcPixel[2]; \
		} \
		pSrcPixel += FACES_ROW_OFFSET; \
		pMaskPixel += FACES_ROW_OFFSET; \
		pDestPixel += DEST_ROW_OFFSET;

	//All the pixel copying happens here.  For performance, I'm not using loops.
	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYLASTPIXEL
	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYLASTPIXEL
	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYLASTPIXEL
	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYPIXEL	COPYLASTPIXEL

#	undef COPYPIXEL
#	undef COPYLASTPIXEL
}

//****************************************************************************
void CFaceWidget::MovePupils(void)
//Moves pupils as appropriate for the current state of the face.
{
	const int xRightBound = CX_PUPIL - 1;
	const int xLeftBound = -xRightBound;
	const int yBottomBound = 
			(m_EyeMaskRectArray[this->eMood].h - CY_PUPIL - CY_PUPIL) / 2;
	const int yTopBound = -yBottomBound + 
		//Nervous mood has big eye corners that pupils can disappear into.
		((this->eMood == Nervous) * 2) +
		//Happy mood--same thing, but corners are a little smaller.
		((this->eMood == Happy) * 1);

	ASSERT(xRightBound >= xLeftBound);
	ASSERT(yBottomBound >= yTopBound);

	if (this->bIsReading)
	{
		//Scroll eyes across as if reading.
		if (--(this->nPupilX) < xLeftBound) this->nPupilX = xRightBound;
		this->nPupilY = yBottomBound;
	}
	else
	{
		switch (this->eMood)
		{	
			case Dieing:
			case Strike:
				//Keep pupils fixed in center.
				this->nPupilX = this->nPupilY = 0;
			break;

			default:
			{
				//Figure out the relaxation level.  Higher value means
				//that the pupils are less likely to move around.
				int nRelaxationLevel;
				if (this->eMood == Happy)
					nRelaxationLevel = 6;
				else if (this->eMood == Aggressive || this->eMood == Nervous)
					nRelaxationLevel = 0;
				else
					nRelaxationLevel = 4;

				//Bounds check the target.
				if (this->nPupilTargetX < xLeftBound) 
					this->nPupilTargetX = xLeftBound;
				else if (this->nPupilTargetX > xRightBound) 
					this->nPupilTargetX = xRightBound;
				if (this->nPupilTargetY < yTopBound) 
					this->nPupilTargetY = yTopBound;
				else if (this->nPupilTargetY > yBottomBound) 
					this->nPupilTargetY = yBottomBound;

				//Are pupils at their target?
				if (this->nPupilX == this->nPupilTargetX &&
						this->nPupilY == this->nPupilTargetY)	//Yes.
				{	
					//After a random delay based on the relaxation level...
					if (!nRelaxationLevel || (int) rand() % nRelaxationLevel == 0) 
					{
						//...pick a new target.
						this->nPupilTargetX = (((int)
								rand() % (xRightBound - xLeftBound + 1)) + xLeftBound);
						this->nPupilTargetY = (((int)
								rand() % (yBottomBound - yTopBound + 1)) + yTopBound);
					}
				}
				else											//No.
				{	//Move pupils towards target.
					int xDist = this->nPupilTargetX - this->nPupilX;
					//Pupils decelerate as they get closer.
					int nXSpeed = 1 + (abs(xDist) > 2) + (abs(xDist) > 4); 
					this->nPupilX += sgn(xDist) * nXSpeed;
					this->nPupilY += sgn(this->nPupilTargetY - this->nPupilY);
				}
			}
			break;
		}
	}
	
	//Bounds check the pupils.
	if (this->nPupilX < xLeftBound) 
		this->nPupilX = xLeftBound;
	else if (this->nPupilX > xRightBound) 
		this->nPupilX = xRightBound;
	if (this->nPupilY < yTopBound) 
		this->nPupilY = yTopBound;
	else if (this->nPupilY > yBottomBound) 
		this->nPupilY = yBottomBound;
}

//*****************************************************************************
void CFaceWidget::PaintClipped(
//
//Params:
	const int /*nX*/, const int /*nY*/, const UINT /*wW*/, const UINT /*wH*/,
	const bool /*bUpdateRect*/)
{
	//Paint() uses direct access to pixels, so it can't be clipped with
	//SDL_SetClipRect().  Either you need to write PaintClipped() or change
	//the situation which causes this widget to be clipped.
	ASSERTP(false, "Face widget should not be clipped.");
}

//******************************************************************************
void CFaceWidget::Paint(
//Paint the face.  Take into account the mood and the time that's passed
//since the last paint to animate the face.
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

	static int iDyingAnimFrame = 0;

	//Make things easy and insist on standard width and height.
	ASSERT(this->w == CX_FACE && this->h == CY_FACE);

	SDL_Surface *pDestSurface = GetDestSurface();
	SDL_Rect Dest = {this->x, this->y, this->w, this->h};

	bool bWasBlinking = this->bIsBlinking;
	if (this->bIsBlinking) 
		this->bIsBlinking = false;
	else
	{
		if (this->eMood == Strike || this->eMood == Dieing)
			this->bIsBlinking = false; //No blinking in these moods.
		else
			this->bIsBlinking = ((rand() % 20) == 0);
	}
	bool bDrawPupils = !this->bIsBlinking;

	//Figure out what face frame to use.
	FACE_FRAME eFrame;
	switch (this->eMood)
	{
		case Normal:
			eFrame = this->bIsBlinking ? FF_Normal_Blinking : FF_Normal;
		break;
		
		case Aggressive:
			eFrame = this->bIsBlinking ? FF_Aggressive_Blinking : FF_Aggressive; 
		break;

		case Nervous: 
			eFrame = this->bIsBlinking ? FF_Nervous_Blinking : FF_Nervous; 
		break;
		
		case Happy: 
			eFrame = this->bIsBlinking ? FF_Happy_Blinking : FF_Happy; 
		break;

		case Talking:
			eFrame = this->bIsBlinking ? FF_Talking_Blinking : FF_Talking; 
		break;
		
		case Strike: 
			eFrame = FF_Striking; 
		break;			
		
		case Dieing:
		{
			switch ((iDyingAnimFrame++) % 4)
			{
				case 0:			eFrame = FF_Dieing_1; break;
				case 1: case 3:	eFrame = FF_Dieing_2; break;
				case 2:			eFrame = FF_Dieing_3; break;
			}
		}
		break;

		default:
		ASSERTP(false, "Bad mood for face.");
	}

	//Blit entire face frame if needed.
	if (!this->bMoodDrawn || this->bIsBlinking != bWasBlinking || 
			this->eMood == Dieing)
	{
		SDL_Rect Src = {xFace(eFrame), yFace(eFrame), this->w, this->h};
		SDL_BlitSurface(this->pFacesSurface, &Src, pDestSurface, &Dest);

		this->bMoodDrawn = true;
	}
	else if (bDrawPupils)
	{	
		//The pupils need to be drawn but I did not blit the face frame.
		//So I will first blit the eye mask area to the dest surface on which
		//the pupils will be drawn below.  This is how the current pupils
		//are "erased".
		SDL_Rect EyeMaskSrc = {xFace(eFrame) + m_EyeMaskOffsetArray[this->eMood].x,
			yFace(eFrame) + m_EyeMaskOffsetArray[this->eMood].y, 
			m_EyeMaskRectArray[this->eMood].w, m_EyeMaskRectArray[this->eMood].h};
		SDL_Rect EyeMaskDest = {this->x + m_EyeMaskOffsetArray[this->eMood].x,
			this->y + m_EyeMaskOffsetArray[this->eMood].y, 
			EyeMaskSrc.w, EyeMaskSrc.h};
		SDL_BlitSurface(this->pFacesSurface, &EyeMaskSrc, pDestSurface, &EyeMaskDest);
	}
	
	//Draw the pupils if needed.
	if (bDrawPupils)
	{
		MovePupils();
		DrawPupils();
	}

	if (bUpdateRect) UpdateRect();
}

//
//Protected methods.
//

//******************************************************************************
void CFaceWidget::HandleAnimate(void)
//Handle animation of the widget.
{
	Uint32 dwNow = SDL_GetTicks();

	//After timing of a temporary face is done (i.e., happy for a moment after 
	//a monster kill) go back to previous mood
	if (	(this->eMood != this->ePrevMood) && 
		
			//Check for specified time elapsed.
			(this->lDelayMood > 0 && 
			dwNow - this->lStartDelayMood > this->lDelayMood) ||
		
			//Check for sound effect finished.
			(this->eMoodSEID != SEID_NONE && 
			!g_pTheSound->IsSoundEffectPlaying(this->eMoodSEID))
		)
	{
		//Set members so that face will be redrawn below in new mood.
		this->eMood = this->ePrevMood;
		this->eMoodSEID = SEID_NONE;
		this->lDelayMood = 0;
		this->bMoodDrawn = false;
	}

	//Animation frame rate is slower (probably) than screen animation rate.
	{
		static Uint32 dwLastAnimate = 0L;
		if (dwNow - dwLastAnimate > 200L || !this->bMoodDrawn) 
		{
			Paint(true);
			dwLastAnimate = dwNow;
		}
	}
}

//******************************************************************************
bool CFaceWidget::Load(void)
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	ASSERT(!this->pFacesSurface);

	this->pFacesSurface = g_pTheBM->GetBitmapSurface("Faces");
	if (!this->pFacesSurface) return false;

	this->bIsLoaded = LoadChildren();
	return this->bIsLoaded;
}

//******************************************************************************
void CFaceWidget::Unload(void)
//Unload resources for the widget.
{
	ASSERT(this->bIsLoaded);

	UnloadChildren();

	if (this->pFacesSurface)
	{
		g_pTheBM->ReleaseBitmapSurface("Faces");
		this->pFacesSurface = NULL;
	}

	this->bIsLoaded = false;
}

// $Log: FaceWidget.cpp,v $
// Revision 1.22  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.21  2003/07/22 18:59:58  mrimer
// Changed reinterpret_casts to static_casts.
//
// Revision 1.20  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.19  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.18  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.17  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.16  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.15  2002/08/24 21:45:15  erikh2000
// Moods can now be set to last the duration of a sound effect.
//
// Revision 1.14  2002/08/23 23:33:57  erikh2000
// Changed face widget code to use new graphics.
// Pupils move to random targets instead of just wandering.
// Pupils drawn partially covered when they are at edges of movement areas.
//
// Revision 1.13  2002/06/25 05:52:29  mrimer
// Revised #includes.
//
// Revision 1.12  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.11  2002/06/14 00:48:24  erikh2000
// Renamed "pScreenSurface" var to more accurate name--"pDestSurface".
//
// Revision 1.10  2002/06/11 22:59:00  mrimer
// Changed "screens" to "destination surfaces".
//
// Revision 1.9  2002/04/22 16:06:33  mrimer
// Fixed timing problem in reverting back to previous mood.
//
// Revision 1.8  2002/04/20 08:20:40  erikh2000
// Added ResetForPaint() method to fix a problem with face not refreshing completely in some cases.
//
// Revision 1.7  2002/04/19 21:49:08  erikh2000
// Moved face widget coords and dimensions constants around.
// Changed a few absolute destination coords to be relative to face widget.
//
// Revision 1.6  2002/04/18 17:39:39  mrimer
// Implemented CFaceWidget::Paint().
//
// Revision 1.5  2002/04/14 00:28:20  erikh2000
// Fixed DrawFilledRect() call.
//
// Revision 1.4  2002/04/13 19:45:06  erikh2000
// Added new type parameter to CWidget constructor call.
//
// Revision 1.3  2002/04/12 22:53:13  erikh2000
// Changed bitmap loading to use new reference-counting methods of CBitmapManager.
//
// Revision 1.2  2002/04/11 10:19:44  erikh2000
// Changed bitmap loading to use CBitmapManager.
//
// Revision 1.1  2002/04/09 10:01:37  erikh2000
// Initial check-in.
//
