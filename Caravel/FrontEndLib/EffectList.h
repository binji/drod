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

#ifndef EFFECTLIST_H
#define EFFECTLIST_H

#include "Effect.h"

//****************************************************************************************
class CScreen;
class CEffectList
{
public:
	CEffectList(CWidget *pOwnerWidget);
	virtual ~CEffectList() {Clear();}

	void				AddEffect(CEffect *pEffect);
	virtual void	Clear(const bool bRepaint=false);
	bool				ContainsEffectOfType(const UINT eEffectType);
	void				DrawEffects(const bool bFreezeEffects=false,
         SDL_Surface *pDestSurface=NULL);
	virtual void	RemoveEffectsOfType(const UINT eEffectType);

	list<CEffect *> Effects;

protected:
	CWidget *	pOwnerWidget;
   CScreen *   pOwnerScreen;

	//Save time when effects are temporarily stopped
	Uint32			dwTimeEffectsWereFrozen;
};

#endif //...#ifndef EFFECTLIST_H

// $Log: EffectList.h,v $
// Revision 1.1  2003/08/16 00:43:21  mrimer
// Refactored DROD effect list code into a general base class for screen effects.
//
