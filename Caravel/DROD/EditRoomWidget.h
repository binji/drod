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

#ifndef EDITROOMWIDGET_H
#define EDITROOMWIDGET_H

//This derived class provides editing support in the CRoomWidget.

#include "RoomWidget.h"

//Plotting a long monster segment in the editor.
typedef struct {
	UINT wHeadX, wHeadY;	//head position of long monster
	UINT wTailX, wTailY;	//current tail position of long monster
	UINT wSX, wSY;			//start of segment
	UINT wEX, wEY;			//end of segment
	UINT wDirection;		//direction of segment, from head to tail
	bool bHorizontal;	//whether segment is horiz. or vert.
} MonsterSegment;

//Tile types used only here (for editor).
const UINT T_SWORDSMAN = TOTAL_TILE_COUNT + 0;	//for placing the level entrance
const UINT T_NOMONSTER = TOTAL_TILE_COUNT + 1;	//for erasing monsters only

class CEditRoomWidget : public CRoomWidget
{
public:
	CEditRoomWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH);

	bool				AddDoorEffect(COrbAgentData *pOrbAgent);
	void				AddOrbAgentsEffect(COrbData *pOrb, const bool bEditingOrb = true);
	bool				AddOrbEffect(COrbAgentData *pOrbAgent);
	void				AddOrbAgentToolTip(const UINT wX, const UINT wY,
			const UINT wAgentType);
	void				AddMonsterSegmentEffect(const UINT wMonsterType);
	void				AddPendingPlotEffect(const UINT wObjectNo,
			const UINT* pwTileImageNo, const UINT wXSize=1, const UINT wYSize=1,
			const bool bSinglePlacement=false);
	void				AddToolTipEffect(const UINT wX, const UINT wY,
			const MESSAGE_ID messageID);

	UINT				GetSerpentStraightTile(const UINT wX, const UINT wY,
			const UINT wDirection, const bool bShow) const;
	UINT				GetSerpentTailTile(const UINT wX, const UINT wY,
			const UINT wDirection, const bool bShow) const;
	UINT				GetSerpentTurnTile(const UINT wX, const UINT wY,
			const UINT wDirection, const bool bShow) const;

   bool           IsObjectReplaceable(const UINT wObject, const UINT wTileLayer,
         const UINT wTileNo) const;
	bool				IsSafePlacement(const UINT wSelectedObject,
			const UINT wX, const UINT wY, const UINT wO=NO_ORIENTATION,
         const bool bAllowSelf=false) const;

	bool				LoadFromRoom(CDbRoom *pRoom);
   void           ResetRoom();

	bool				Plotted(const UINT wCol, const UINT wRow) const;
	void				ResetPlot(void);
	void				SetPlot(const UINT wCol, const UINT wRow);

	virtual void	Paint(bool bUpdateRect = true);
	virtual void	Repaint(int wCol=0, int wRow=0,
			int wWidth=CDrodBitmapManager::DISPLAY_COLS, int wHeight=CDrodBitmapManager::DISPLAY_ROWS);

   UINT				wStartX, wStartY;	//a starting tile coord
	UINT				wMidX, wMidY;		//set to upper-left corner during drag
	UINT				wEndX, wEndY;		//an ending tile coord
	bool				bMouseInBounds;	//whether plot is active
	bool				bSinglePlacement;	//whether only one object is to be placed
	bool				bPlacing;			//in placing object state
	UINT				wPX, wPY, wPO;		//level start position
	UINT				wOX, wOY;			//selected orb position
	MonsterSegment monsterSegment;	//long monster being plotted

protected:
   virtual	~CEditRoomWidget();

  	virtual void	HandleAnimate() {if (pRoom) Paint(false);}

private:
	virtual void	DrawMonsters(CMonster *const pMonsterList,
			SDL_Surface *pDestSurface, const bool bActionIsFrozen);

   virtual void	HandleMouseDown(const SDL_MouseButtonEvent &Button);
	virtual void	HandleDrag(const SDL_MouseMotionEvent &Motion);
	virtual void	HandleMouseMotion(const SDL_MouseMotionEvent &Motion);
	virtual void	HandleMouseUp(const SDL_MouseButtonEvent &Button);

	bool *				pPlotted;	//level editor: where an object is placed
};

#endif //#ifndef EDITROOMWIDGET_H

// $Log: EditRoomWidget.h,v $
// Revision 1.4  2003/08/16 01:54:19  mrimer
// Moved EffectList to FrontEndLib.  Added general tool tip support to screens.  Added RoomEffectList to room editor screen/widget.
//
// Revision 1.3  2003/07/19 21:27:54  mrimer
// Added placement constraint on placing swordsman and his sword.
//
// Revision 1.2  2003/07/07 23:29:29  mrimer
// Fixed some mistakes in the object placement permission logic.
//
// Revision 1.1  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
