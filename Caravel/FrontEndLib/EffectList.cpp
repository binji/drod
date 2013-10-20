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

#include "EffectList.h"
#include <BackEndLib/Assert.h>
#include "Screen.h"

//*****************************************************************************
CEffectList::CEffectList(CWidget *pOwnerWidget)
   : pOwnerWidget(pOwnerWidget)
   , pOwnerScreen(NULL)
   , dwTimeEffectsWereFrozen(0L)
{
   if (pOwnerWidget->eType == WT_Screen)
      this->pOwnerScreen = DYN_CAST(CScreen*, CWidget*, pOwnerWidget);
}

//*****************************************************************************
void CEffectList::AddEffect(
//Adds an effect to the effect list.
//
//Params:
	CEffect *pEffect)				//(in)	Effect to add.
{
	ASSERT(pEffect);

	//List is sorted by draw sequence--smaller values are at beginning.
	//Add effect in front of the first element with a larger or equal draw
	//sequence.
	for (list<CEffect *>::iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		CEffect *pSeekEffect = *iSeek;
		if (pSeekEffect->GetDrawSequence() >= pEffect->GetDrawSequence())
		{
			this->Effects.insert(iSeek, pEffect);
			return;
		}
	}

	//Effect has the largest draw sequence value so it goes at end of list.
	this->Effects.push_back(pEffect);
}

//*****************************************************************************
void CEffectList::Clear(
//Clears all effects from the effect list.
//
//Params:
	const bool bRepaint)	//(in)	Touch up affected screen areas before deleting
								//(default = false)
{
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
      CEffect *pEffect = *iSeek;
		ASSERT(pEffect);
		if (bRepaint)
      {
			this->pOwnerWidget->Paint();
         if (this->pOwnerScreen)
            this->pOwnerScreen->UpdateRect(pEffect->rAreaOfEffect);
      }
		delete pEffect;
	}
	this->Effects.clear();
}

//*****************************************************************************
bool CEffectList::ContainsEffectOfType(
//Returns true if effect list contains an effect of stated type, else false.
//
//Params:
	const UINT eEffectType)	//(in)
{
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		if (eEffectType == (*iSeek)->GetEffectType())
			return true;
	}

	return false;
}

//*****************************************************************************
void CEffectList::DrawEffects(
//Draws list of effects.
//If freezing effects, save time they are frozen to preserve their frame #.
//When unfreezing, update their start time to now.
//
//Params:
	const bool bFreezeEffects,	//(in) Whether effects are frozen after this draw.
                              //(default = false)
   SDL_Surface *pDestSurface) //(in) where to draw effects (default = NULL)
{
	list<CEffect *>::const_iterator iSeek = this->Effects.begin();
	while (iSeek != this->Effects.end())
	{
      CEffect *pEffect = *iSeek;
		ASSERT(pEffect);
		//Each iteration draws one effect.
		if (!bFreezeEffects && this->dwTimeEffectsWereFrozen)
		{
			//Unfreeze effect where it left off.
			pEffect->dwTimeStarted += SDL_GetTicks() - this->dwTimeEffectsWereFrozen;
			pEffect->dwTimeOfLastMove += SDL_GetTicks() - this->dwTimeEffectsWereFrozen;
		}

		++iSeek;
		if (pEffect->Draw(pDestSurface))
      {
         if (this->pOwnerScreen)
            this->pOwnerScreen->UpdateRect(pEffect->rAreaOfEffect);
      } else {
			//Effect is finished--remove from list.
         if (this->pOwnerScreen)
            this->pOwnerScreen->Paint();  //refresh area of effect
			this->Effects.remove(pEffect);
			delete pEffect;
		}
	}

	this->dwTimeEffectsWereFrozen = (bFreezeEffects ? SDL_GetTicks() : 0L);
}

//*****************************************************************************
void CEffectList::RemoveEffectsOfType(
//Removes all effects of given type from the list.
//
//Params:
	const UINT eEffectType)	//(in)	Type of effect to remove.
{
   bool bRepaint = false;

   //Clear list of given effect type.
	list<CEffect *>::const_iterator iSeek = this->Effects.begin();
	while (iSeek != this->Effects.end())
	{
      CEffect *pEffect = *iSeek;
		ASSERT(pEffect);
		++iSeek;
		if (eEffectType == pEffect->GetEffectType())
		{
			//Remove from list.
			bRepaint = true;
			this->Effects.remove(pEffect);
			delete pEffect;
		}
	}

   if (bRepaint)
   {
      this->pOwnerWidget->Paint();
   }
}

// $Log: EffectList.cpp,v $
// Revision 1.1  2003/08/16 00:43:21  mrimer
// Refactored DROD effect list code into a general base class for screen effects.
//
