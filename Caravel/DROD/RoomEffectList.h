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

#ifndef ROOMEFFECTLIST_H
#define ROOMEFFECTLIST_H

#include <FrontEndLib/EffectList.h>
#include "RoomWidget.h"

//****************************************************************************************
class CRoomEffectList : public CEffectList
{
public:
	CRoomEffectList(CRoomWidget* pRoom)
		: CEffectList(dynamic_cast<CWidget*>(pRoom)), pOwnerWidget(pRoom) {}

	virtual void	Clear(const bool bRepaint=false);
	void				DirtyTiles() const;
	void				DirtyTilesForEffect(CEffect *pEffect) const;
	void				DirtyTilesInRect(const UINT xStart, const UINT yStart,
			const UINT xEnd, const UINT yEnd) const;
	virtual void	RemoveEffectsOfType(const UINT eEffectType);

protected:
   CRoomWidget *pOwnerWidget;
};

#endif //...#ifndef ROOMEFFECTLIST_H

// $Log: RoomEffectList.h,v $
// Revision 1.1  2003/08/16 01:19:35  mrimer
// Refactored code from EffectList.
//
// Revision 1.8  2003/07/01 20:34:22  mrimer
// Added optional destination surface parameter to draw routine.
//
// Revision 1.7  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.6  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.5  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.4  2002/11/15 02:45:26  mrimer
// Modified Clear().
//
// Revision 1.3  2002/10/10 21:11:39  mrimer
// Added DirtyTiles(), DirtyTilesForEffect(), DirtyTilesInRect() to support optimized room drawing.
//
// Revision 1.2  2002/10/03 19:03:29  mrimer
// Added temporarily freezing events.
//
// Revision 1.1  2002/09/14 21:23:27  mrimer
// Initial check-in.
//
