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

#ifndef FACEWIDGET_H
#define FACEWIDGET_H

#include "DrodSound.h"
#include <FrontEndLib/Widget.h>

const UINT CX_FACE = 67;
const UINT CY_FACE = 98;

//Moods the swordsman can be in that affect his facial expression.  
enum MOOD
{
	Normal = 0,
	Aggressive,
	Nervous,
	Strike,
	Happy,
	Dieing,
	Talking,

	Mood_Count
};

//Face frame enum--corresponds to order within bitmap.
enum FACE_FRAME
{
	FF_Normal = 0,
	FF_Normal_Blinking,
	FF_Striking,
	FF_Aggressive,
	FF_Aggressive_Blinking,
	FF_Dieing_1,
	FF_Nervous,
	FF_Nervous_Blinking,
	FF_Dieing_2,
	FF_Happy,
	FF_Happy_Blinking,
	FF_Dieing_3,
	FF_Talking,
	FF_Talking_Blinking
};

//******************************************************************************
class CFaceWidget : public CWidget
{		
	public:
		CFaceWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, 
				UINT wSetH);

		virtual void		Paint(bool bUpdateRect = true);
		virtual void		PaintClipped(const int nX, const int nY, const UINT wW,
				const UINT wH, const bool bUpdateRect = true);
		void				ResetForPaint() {this->bMoodDrawn = false;}
		void				SetMood(MOOD eSetMood, Uint32 lDelay=0);
		void				SetMoodToSoundEffect(MOOD eSetMood, SEID eUntilSEID);
		void				SetReading(const bool bReading) {bIsReading = bReading;}
 
	protected:
		virtual void   HandleAnimate();
		virtual bool   IsAnimated() const {return true;}
		virtual bool   Load();
		virtual void   Unload();

	private:
		void				MovePupils();
		void				DrawPupils();
		inline void			DrawPupils_DrawOnePupil(
				SDL_Surface *pDestSurface, const int nDestX, const int nDestY, 
				const int nMaskX, const int nMaskY) const;
		bool				bHasMoodDelayPassed() {return SDL_GetTicks()-lStartDelayMood>lDelayMood;}
		void				SetMoodDelay(Uint32 lDelay);

		SDL_Surface *		pFacesSurface;
		MOOD				eMood, ePrevMood;
		Uint32				lDelayMood, lStartDelayMood;			//how long to show face (0 means indefinitely)
		bool				bMoodDrawn, bIsReading, bIsBlinking;	//whether face is reading/blinking
		SEID				eMoodSEID;
		int					nPupilX, nPupilY;
		int					nPupilTargetX, nPupilTargetY;
};

#endif //#ifndef FACEWIDGET_H

// $Log: FaceWidget.h,v $
// Revision 1.13  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.12  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.10  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.9  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.8  2002/08/24 21:45:15  erikh2000
// Moods can now be set to last the duration of a sound effect.
//
// Revision 1.7  2002/08/23 23:33:57  erikh2000
// Changed face widget code to use new graphics.
// Pupils move to random targets instead of just wandering.
// Pupils drawn partially covered when they are at edges of movement areas.
//
// Revision 1.6  2002/07/22 02:48:52  erikh2000
// Made face and room widgets animated.
//
// Revision 1.5  2002/06/15 23:47:57  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.4  2002/04/20 08:20:40  erikh2000
// Added ResetForPaint() method to fix a problem with face not refreshing completely in some cases.
//
// Revision 1.3  2002/04/19 21:49:08  erikh2000
// Moved face widget coords and dimensions constants around.
// Changed a few absolute destination coords to be relative to face widget.
//
// Revision 1.2  2002/04/18 17:39:39  mrimer
// Implemented CFaceWidget::Paint().
//
// Revision 1.1  2002/04/09 10:01:37  erikh2000
// Initial check-in.
//
