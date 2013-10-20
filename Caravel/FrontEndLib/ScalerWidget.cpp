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
 * mrimer
 *
 * ***** END LICENSE BLOCK ***** */

#include "ScalerWidget.h"

#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

//
//Public methods.
//

//***************************************************************************************
CScalerWidget::CScalerWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH) //(in) Passed to CWidget.
	: CWidget(WT_Scaler, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
{
	this->pTrueScaleSurface = NULL;
	this->bNewScaleDimensions = false;
	this->ppTrueToDestScaleMap = NULL;

	this->nAntiAliasY = 0;
	this->bAntiAliasInProgress = false;

	this->pTrueScaleContainer = new CFrameWidget(TAG_UNSPECIFIED, 0, 0, 0, 0, wszEmpty);
}

//***************************************************************************************
CScalerWidget::~CScalerWidget(void)
//Destructor.
{
	delete this->pTrueScaleContainer;
}

//***************************************************************************************
int CScalerWidget::GetScaledX(const int nTrueX)
//Given an X coordinate on the true-scale (source) surface, what is the corresponding
//X coordinate on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledXPixelsPerTrue = (double) this->w / (double) TrueRect.w;

	return this->x + static_cast<int>( ((double) nTrueX * dblScaledXPixelsPerTrue) );
}

//***************************************************************************************
int CScalerWidget::GetScaledY(const int nTrueY)
//Given a Y coordinate on the true-scale (source) surface, what is the corresponding
//Y coordinate on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledYPixelsPerTrue = (double) this->h / (double) TrueRect.h;

	return this->y + static_cast<int>( ((double) nTrueY * dblScaledYPixelsPerTrue) );
}

//***************************************************************************************
int CScalerWidget::GetScaledW(const int nTrueW)
//Given a width on the true-scale (source) surface, what is the corresponding
//width on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledXPixelsPerTrue = (double) this->w / (double) TrueRect.w;

	return static_cast<int>( ((double) nTrueW * dblScaledXPixelsPerTrue) );
}

//***************************************************************************************
int CScalerWidget::GetScaledH(const int nTrueH)
//Given a height on the true-scale (source) surface, what is the corresponding
//height on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledYPixelsPerTrue = (double) this->h / (double) TrueRect.h;

	return static_cast<int>( ((double) nTrueH * dblScaledYPixelsPerTrue) );
}

//***************************************************************************************
bool CScalerWidget::Load(void)
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	ASSERT(!this->pTrueScaleSurface);
	ASSERT(!this->ppTrueToDestScaleMap);

	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	if (ContainerRect.w && ContainerRect.h)
	{
		if (!CreateNewTrueScaleSurface() || !CalcScaleInstructions()) 
			return false;
		this->pTrueScaleContainer->SetDestSurface(this->pTrueScaleSurface);
		this->bNewScaleDimensions = false;
	}

	//Load scaled and normal children.
	this->bIsLoaded = this->pTrueScaleContainer->LoadChildren() && LoadChildren();
	return this->bIsLoaded ;
}

//***************************************************************************************
void CScalerWidget::Unload(void)
//Unloads resources for the widget.
{
	ASSERT(this->bIsLoaded);

	//Unload any children widgets.
	UnloadChildren();
	this->pTrueScaleContainer->UnloadChildren();

	if (this->pTrueScaleSurface) 
	{
		SDL_FreeSurface(this->pTrueScaleSurface);
		this->pTrueScaleSurface = NULL;
	}
	
	if (this->ppTrueToDestScaleMap)
	{
		delete [] this->ppTrueToDestScaleMap;
		this->ppTrueToDestScaleMap = NULL;
	}
	
	//In case widget is reloaded, force reinits on everything.
	this->bNewScaleDimensions = true;
	this->bAntiAliasInProgress = false;

	this->bIsLoaded = false;
}

//***************************************************************************************
CWidget * CScalerWidget::AddScaledWidget(
//Adds the widget to a parentless container widget used to render widgets to
//destination.
//
//Params:
	CWidget *pNewWidget, //(in) Widget to add.
	bool bLoad)			 //(in) Load the widget?
//
//Returns:
//The pNewWidget param or NULL if widget load failed.
{
	//Add the widget.
	if (!this->pTrueScaleContainer->AddWidget(pNewWidget, bLoad)) 
		return NULL; //Load failed.

	//Check for increase in child container rect.
	SDL_Rect ContainerRect, ChildrenRect;
	pTrueScaleContainer->GetRect(ContainerRect);
	pTrueScaleContainer->GetRectContainingChildren(ChildrenRect);
	if (!ARE_RECTS_EQUAL(ContainerRect, ChildrenRect))
	{
		//There must be at least one scaled widget at 0,0 or things aren't
		//going to work.  When adding widgets to CScalerWidget, make sure this 
		//is true.
		ASSERT(ChildrenRect.x == 0 && ChildrenRect.y == 0);

		//The container rect should only grow since you can only add scaled 
		//widgets, and can't remove them.
		ASSERT(ChildrenRect.w >= ContainerRect.w);
		ASSERT(ChildrenRect.h >= ContainerRect.h);

		pTrueScaleContainer->Resize(ChildrenRect.w, ChildrenRect.h);
		this->bNewScaleDimensions = true;
		this->bAntiAliasInProgress = false;
	}

	//If widget was added after CScalerWidget loaded, then set dest surface.
	if (this->pTrueScaleSurface)
		pNewWidget->SetDestSurface(this->pTrueScaleSurface);

	return pNewWidget;
}

//***************************************************************************************
void CScalerWidget::Resize(UINT wSetW, UINT wSetH)
//Resizes plus flags for new scaling dimensions.
{
	CWidget::Resize(wSetW, wSetH);
	this->bNewScaleDimensions = true;
	this->bAntiAliasInProgress = false;
}

//***************************************************************************************
void CScalerWidget::PaintClipped(
   const int /*nX*/, const int /*nY*/, const UINT /*wW*/, const UINT /*wH*/, const bool /*bUpdateRect*/)
{
	//Paint() uses direct access to pixels, so it can't be clipped with 
	//SDL_SetClipRect().  Either you need to write PaintClipped() or change
	//the situation which causes this widget to be clipped.
	ASSERTP(false, "Can't paint clipped.");
}

//***************************************************************************************
void CScalerWidget::Paint(
//Draws children to a hidden true-scale surface, then draws a scaled version to the
//widget area of the destination surface.
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

	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	if (ContainerRect.w == 0) 
	{
		//No children to paint yet.
		ASSERT(ContainerRect.x == 0);
		ASSERT(ContainerRect.y == 0);
		ASSERT(ContainerRect.h == 0);
		return;
	}

	//If scaling dimensions have changed, update true-scale surface and the 
	//instructions for scaling from true-scale to widget areas.
	if (this->bNewScaleDimensions)
	{
		if (!this->pTrueScaleSurface || 
				this->pTrueScaleSurface->w != ContainerRect.w ||
				this->pTrueScaleSurface->h != ContainerRect.h)
		{
			if (!CreateNewTrueScaleSurface()) {ASSERTP(false, "True scale surface alloc failed."); return;}
			this->pTrueScaleContainer->SetDestSurface(this->pTrueScaleSurface);
		}
		if (!CalcScaleInstructions()) {ASSERTP(false, "Calc scale instr. failed."); return;}
		this->bNewScaleDimensions = false;
		this->bAntiAliasInProgress = false;
	}

	//Paint scaled children to true-scale surface.
	this->pTrueScaleContainer->PaintChildren(false);

	//Draw quick scale to widget area.
	DrawScaledQuick();
	
	//Set up members to later draw anti-aliased during animate.
	//Try to anti-alias in about one second.
	this->nAntiAliasY = this->y;
	this->bAntiAliasInProgress = true;

	//Paint children (non-scaled).
	PaintChildren();

	//Put it up on the screen.
	if (bUpdateRect) UpdateRect();
}

//
//Protected methods.
//

//***************************************************************************************
void CScalerWidget::HandleAnimate(void)
//Handles animation occurring between events.
{
	ASSERT(IsVisible());

	if (this->bAntiAliasInProgress)
	{	
		//I'm going to spend 25ms working on the anti-aliased image each time
		//HandleAnimate() is called.  HandleAnimate() probably won't be called 
		//more often than 33ms (30 fps) so this gives the event-handler a little
		//time to work on other things.  A higher value might result in choppiness.
		//A lower value would make the anti-aliasing take longer.  If it seems
		//like the value should be different on different screens, then we can
		//add a method to set the maximum time to spend per animate.
		const DWORD MAX_TIME_PER_ANIMATE = 25;

		//Anti-alias the lines.
		UINT wAntiAliasLineCount =
				DrawAntiAliasedLines(this->nAntiAliasY, MAX_TIME_PER_ANIMATE);

		//Update anti-aliasing status.
		this->nAntiAliasY += wAntiAliasLineCount;
		this->bAntiAliasInProgress = (this->nAntiAliasY < static_cast<int>(this->y + this->h));

		//Paint children (non-scaled).
		PaintChildren();

		//Show finished anti-aliasing when done.
		if (!this->bAntiAliasInProgress) UpdateRect();
	}
}

//
//Private methods.
//

//***************************************************************************************
void CScalerWidget::DrawScaledQuick(void)
//Do a quick scaled draw from true-scale surface to widget area on screen surface.
{
	ASSERT(!this->bNewScaleDimensions);
	ASSERT(this->pTrueScaleSurface);
	ASSERT(this->ppTrueToDestScaleMap);

	SDL_Surface *pDestSurface = LockDestSurface();
	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	ASSERT(wBPP == 3);

	//Set pointer to first element of map.  There is one element for each
	//destination pixel that points to a pixel in the true-scale surface to be copied.
	Uint8 * *ppSrc = this->ppTrueToDestScaleMap;
	
	//Calc offset between end of a row and beginning of next.
	DWORD dwRowOffset = (pDestSurface->w - this->w) * wBPP;

	//Calc location of topleft pixel.
	Uint8 *pSeek = (Uint8 *)(pDestSurface->pixels) + 
			( (this->y * pDestSurface->w) + this->x) * wBPP;

	//Calc location of bottomleft pixel plus one row too far.
	Uint8 *pEndOfSeek = pSeek + (this->h * pDestSurface->w * wBPP);

	//Each iteration fills one row of pixels.
	Uint8 *pEndOfRow, *pSrc;
	while (pSeek != pEndOfSeek)
	{
		ASSERT(pSeek < pEndOfSeek);
		pEndOfRow = pSeek + (this->w * wBPP);

		//Each iteration sets 3 bytes (1 pixel) from the scale map.
		while (pSeek != pEndOfRow)
		{
			pSrc = *ppSrc;
			pSeek[0] = pSrc[0];
			pSeek[1] = pSrc[1];
			pSeek[2] = pSrc[2];
			pSeek += wBPP;

			++ppSrc;
		}

		//Advance to beginning of next row.
		pSeek += dwRowOffset;
	}

	UnlockDestSurface();
}

//***************************************************************************************
UINT CScalerWidget::DrawAntiAliasedLines(
//Draw one or more destination lines using the true-scale surface for source.
//Pixels will be anti-aliased by averaging their neighbors.
//
//Params:
	int nStartY,		//(in)	Destination line at which to begin drawing.
	DWORD dwMaxTime)	//(in)	Maximum amount of time, in milliseconds to spend
						//		in routine.  Actually, routine is likely to go
						//		a tiny bit longer.
//
//Returns:
//Number of lines that were drawn.
{
	DWORD dwStopTime = SDL_GetTicks() + dwMaxTime;

	//Number of source pixels to move for each dest pixel.
	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	const double dblIncSrcX = (double) ContainerRect.w / (double) this->w;
	const double dblIncSrcY = (double) ContainerRect.h / (double) this->h;
	
	//Algorithm is incorrect if dest is less than half the size of source.
	//It only uses a maximum of four source pixels to average into one dest pixel.
	ASSERT(dblIncSrcX <= 2 && dblIncSrcY <= 2);
	
	//Lock dest surface for pixel copying.
	SDL_Surface *pDestSurface = LockDestSurface();
	const UINT wBPP = pDestSurface->format->BytesPerPixel;

	//Copy pixels over.
	Uint8 *pSrc, *pSrcTop, *pDest, *pSrcEast, *pSrcSouth, *pSrcSoutheast;
	UINT xSrc, ySrc;
	double dblSouthOffsetPercent, dblEastOffsetPercent, dblSoutheastOffsetPercent;
	double dblSrcPercent, dblSrcX;
	double dblSrcY = dblIncSrcY * (double) (nStartY - this->y);

	//shorthand vars for speed optimization
	const UINT yEnd = this->y + this->h;
	const UINT xEnd = this->x + this->w;
	const UINT yEndMinusOne = yEnd - 1;
	const UINT xEndMinusOne = xEnd - 1;
	const UINT thisx = this->x;
	const UINT wPitch =	this->pTrueScaleSurface->pitch;

	//Loop vars.
	double yPercent;
	double xPercent;
	double OneMinusYPercent;

	//Each iteration draws one anti-aliased line.
   UINT yDest;
	for (yDest = nStartY; yDest < yEndMinusOne; ++yDest)
	{
		dblSrcX = 0;
		ySrc = (UINT) dblSrcY;
		yPercent = dblSrcY - ySrc;	//shorthand
		OneMinusYPercent = 1.0 - yPercent;

		//init
		pDest = (Uint8 *)(pDestSurface->pixels) +
					(yDest * pDestSurface->pitch) + (this->x * wBPP);
		pSrcTop = (Uint8 *)(this->pTrueScaleSurface->pixels) +
					(ySrc * wPitch);

		//Each iteration draws one anti-aliased pixel.
		for (UINT xDest = thisx; xDest < xEndMinusOne; ++xDest)
		{
			xSrc = (UINT) dblSrcX;
			xPercent = dblSrcX - xSrc;	//shorthand

			dblSouthOffsetPercent = yPercent * (1.0 - xPercent);
			dblEastOffsetPercent = xPercent * OneMinusYPercent;
			dblSoutheastOffsetPercent = yPercent * xPercent;
			ASSERT(dblEastOffsetPercent + dblSouthOffsetPercent +
					dblSoutheastOffsetPercent <= 1.0);
			dblSrcPercent = 1.0 - dblSoutheastOffsetPercent - dblEastOffsetPercent -
				dblSouthOffsetPercent;

			//last row and column are skipped to avoid lots of bounds logic here
			pSrc = pSrcTop + (xSrc * wBPP);
			pSrcEast = pSrc + wBPP;
			pSrcSouth = pSrc + wPitch;
			pSrcSoutheast = pSrcSouth + wBPP;

			pDest[0] = static_cast<Uint8>(pSrc[0] * dblSrcPercent +
						pSrcEast[0] * dblEastOffsetPercent +
						pSrcSouth[0] * dblSouthOffsetPercent +
						pSrcSoutheast[0] * dblSoutheastOffsetPercent);

			pDest[1] = static_cast<Uint8>(pSrc[1] * dblSrcPercent +
						pSrcEast[1] * dblEastOffsetPercent +
						pSrcSouth[1] * dblSouthOffsetPercent +
						pSrcSoutheast[1] * dblSoutheastOffsetPercent);

			pDest[2] = static_cast<Uint8>(pSrc[2] * dblSrcPercent +
						pSrcEast[2] * dblEastOffsetPercent +
						pSrcSouth[2] * dblSouthOffsetPercent +
						pSrcSoutheast[2] * dblSoutheastOffsetPercent);

			dblSrcX += dblIncSrcX;
			pDest += wBPP;
		}
		dblSrcY += dblIncSrcY;

		if (SDL_GetTicks() > dwStopTime) break;
	}

	UnlockDestSurface();

	return (yDest - nStartY) + 1;
}

//***************************************************************************************
bool CScalerWidget::CreateNewTrueScaleSurface(void)
//Creates a new surface that can display all of the child widgets in true scale.
//
//Returns:
//True if successful, false if not.
{
	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	ASSERT(ContainerRect.w != 0 && ContainerRect.h!=0);

	//If I already have a true-scale surface, free it.
	if (this->pTrueScaleSurface)
	{
		SDL_FreeSurface(this->pTrueScaleSurface);
		this->pTrueScaleSurface = NULL;
	}

	//Create the new surface.
	this->pTrueScaleSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			ContainerRect.w, ContainerRect.h, 
			24, 0, 0, 0, 0);
	
	//Delete the true-to-dest scale map since it has pointers into the deleted
	//pixel data.
	if (this->ppTrueToDestScaleMap)
	{
		delete [] this->ppTrueToDestScaleMap;
		this->ppTrueToDestScaleMap = NULL;
	}

	return (this->pTrueScaleSurface != NULL);
}

//***************************************************************************************
bool CScalerWidget::CalcScaleInstructions(void)
//Calculate instructions for scaling.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(this->w && this->h);
	ASSERT(this->pTrueScaleSurface);

	//Delete the true-to-dest scale map because the destination size may have changed.
	if (this->ppTrueToDestScaleMap)
	{
		delete [] this->ppTrueToDestScaleMap;
		this->ppTrueToDestScaleMap = NULL;
	}

	//Alloc a new true-to-dest scale map.
	this->ppTrueToDestScaleMap = new Uint8 *[this->w * this->h];

	//Calc number of source pixels to move for each dest pixel.
	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	const double dblIncSrcX = (double) ContainerRect.w / (double) this->w;
	const double dblIncSrcY = (double) ContainerRect.h / (double) this->h;
	
	//Set up some shorthand vars for better speed in loop.
	const UINT yEnd = this->y + this->h;
	const UINT xEnd = this->x + this->w;
	const UINT wPitch =	this->pTrueScaleSurface->pitch;
	const UINT wBPP = this->pTrueScaleSurface->format->BytesPerPixel;
	const int thisx = this->x;
	Uint8 *pSrcPixels = static_cast<Uint8 *>(this->pTrueScaleSurface->pixels);

	//Each iteration fills map for one destination row.
	UINT xSrc, ySrc;
	double dblSrcX, dblSrcY = 0;
	Uint8 *pSrcTop, *pSrc;
	Uint8 * *ppMapElement = this->ppTrueToDestScaleMap;
	UINT xDest, yDest;
	for (yDest = this->y; yDest < yEnd; ++yDest)
	{
		dblSrcX = 0;
		ySrc = static_cast<UINT>(dblSrcY);
		pSrcTop = pSrcPixels +	(ySrc * wPitch);

		//Each iteration fills map for one destination pixel.
		for (xDest = thisx; xDest < xEnd; ++xDest)
		{
			xSrc = static_cast<UINT>(dblSrcX);
			pSrc = pSrcTop + (xSrc * wBPP);
			*(ppMapElement++) = pSrc;

			dblSrcX += dblIncSrcX;
		}

		dblSrcY += dblIncSrcY;
	}

	ASSERT(ppMapElement - this->ppTrueToDestScaleMap == static_cast<int>(this->w * this->h));

	return true;
}

//Some of ScalerWidget.cpp's code was cut-and-pasted from RoomWidget.cpp, and contains
//contributions outside of those contained in erikh2000's commits.  Specifically,
//mrimer made optimizations to DrawScaledImage() which reappeared in
//CalcScaleInstructions() and DrawAntiAliasedLines().  See history of RoomWidget.cpp
//if interested.

// $Log: ScalerWidget.cpp,v $
// Revision 1.6  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.5  2003/07/22 18:41:41  mrimer
// Changed reinterpret_casts to static_casts.
//
// Revision 1.4  2003/07/09 11:56:51  schik
// Made PaintClipped() arguments consistent among derived classes (VS.NET warnings)
//
// Revision 1.3  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.7  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.6  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.5  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.4  2002/08/30 22:43:12  erikh2000
// Replaced visibility check with an assert since CEventHandlerWidget has been changed to not animate invisible widgets.
//
// Revision 1.3  2002/08/30 20:01:59  mrimer
// Fixed HandleAnimate() to not paint when IsVisible() is false.
//
// Revision 1.2  2002/07/22 01:56:03  erikh2000
// Fixed a problem with painting child widgets.
// Added methods to get scaled coordinates from true-scale coordinates.
//
// Revision 1.1  2002/07/20 22:34:12  erikh2000
// Initial check-in.
//
