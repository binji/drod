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

#include "EditRoomWidget.h"
#include "PendingPlotEffect.h"
#include "TileImageCalcs.h"

#include <FrontEndLib/ToolTipEffect.h>
#include <FrontEndLib/TransTileEffect.h>
#include "DrodEffect.h"
#include "RoomEffectList.h"

#include "../DRODLib/Db.h"
#include "../DRODLib/MonsterFactory.h"
#include "../Texts/MIDs.h"

const SURFACECOLOR Red = {255, 0, 0};
const SURFACECOLOR BlueGreen = {0, 255, 255};
const SURFACECOLOR PaleYellow = {255, 255, 128};
const SURFACECOLOR Orange = {255, 128, 0};

//
//Public methods.
//

//*****************************************************************************
CEditRoomWidget::CEditRoomWidget(
//Constructor.
//
//Params:
	DWORD dwSetTagNo,						//(in)	Required params for CWidget
	int nSetX, int nSetY,					//		constructor.
	UINT wSetW, UINT wSetH)					//
	: CRoomWidget(dwSetTagNo, nSetX, nSetY, wSetW, wSetH, NULL)
	, wStartX((UINT)-1), wStartY((UINT)-1)
	, wMidX((UINT)-1), wMidY((UINT)-1)
	, wEndX((UINT)-1), wEndY((UINT)-1)
	, bMouseInBounds(false)
	, bSinglePlacement(false)
	, bPlacing(true)
	, wPX((UINT)-1), wPY((UINT)-1), wPO((UINT)-1)
	, wOX((UINT)-1), wOY((UINT)-1)
	, pPlotted(NULL)
{
}

//*****************************************************************************
CEditRoomWidget::~CEditRoomWidget()
{
   delete[] this->pPlotted;
}

//*****************************************************************************
void CEditRoomWidget::AddToolTipEffect(
//Adds a tool tip to room tile.
//
//Params:
	const UINT wX, const UINT wY,	//(in) desired square
	const MESSAGE_ID messageID)
{
	CCoord Coord(wX,wY);
	AddLastLayerEffect(new CToolTipEffect(this, Coord,
			g_pTheDB->GetMessageText(messageID)));
}

//*****************************************************************************
bool CEditRoomWidget::AddOrbEffect(
//Add an effect to display the affect an orb agent has on a door.
//
//Returns: whether the orb agent is on a door
//
//Params:
	COrbAgentData *pOrbAgent)	//(in) Orb agent to display
{
   const UINT wX = pOrbAgent->wX, wY = pOrbAgent->wY;
	ASSERT(this->pRoom->IsValidColRow(wX, wY));

   const UINT wTileNo = this->pRoom->GetTSquare(wX, wY);
   // Only works for orbs
   if (wTileNo != T_ORB) return false;

	switch (pOrbAgent->wAction)
	{
		case OA_NULL:
			//Just highlight the door (mouse is over it).
			AddShadeEffect(wX,wY,PaleYellow);
		break;

		case OA_TOGGLE:
			AddShadeEffect(wX,wY,Orange);
		break;

		case OA_OPEN:
			AddShadeEffect(wX,wY,BlueGreen);
		break;

		case OA_CLOSE:
			AddShadeEffect(wX,wY,Red);
		break;
		
		default:
			ASSERTP(false, "Bad orb agent.");
		break;
	}

   return true;
}


//*****************************************************************************
bool CEditRoomWidget::AddDoorEffect(
//Add an effect to display the affect an orb agent has on a door.
//
//Returns: whether the orb agent is on a door
//
//Params:
	COrbAgentData *pOrbAgent)	//(in) Orb agent to display
{
	const UINT wX = pOrbAgent->wX, wY = pOrbAgent->wY;
	ASSERT(this->pRoom->IsValidColRow(wX, wY));

	//Get door type being affected.
	const UINT wOriginalTileNo = this->pRoom->GetOSquare(wX, wY);

	//Only works for doors.
	if (wOriginalTileNo != T_DOOR_Y && wOriginalTileNo != T_DOOR_YO)
		return false;

	//Contains coords to evaluate.
	CCoordStack drawCoords;
   this->pRoom->GetAllDoorSquares(wX, wY, drawCoords);

	//Each iteration pops one pair of coordinates for plotting,
	//Exits when there are no more coords in stack to plot.
	UINT wDrawX, wDrawY;
	while (drawCoords.Pop(wDrawX, wDrawY))
	{
		ASSERT(this->pRoom->IsValidColRow(wDrawX, wDrawY));
		SetPlot(wDrawX, wDrawY);

		//Plot new tile.
		CCoord coord(wDrawX,wDrawY);
		switch(pOrbAgent->wAction)
		{
			case OA_NULL:
				//Just highlight the door (mouse is over it).
				AddShadeEffect(wDrawX,wDrawY,PaleYellow);
			break;

			case OA_TOGGLE:
				AddLastLayerEffect(new CTransTileEffect(this, coord,
						wOriginalTileNo == T_DOOR_Y ? TI_DOOR_YO : TI_DOOR_Y));
				AddShadeEffect(wDrawX,wDrawY,Orange);
			break;

			case OA_OPEN:
				AddLastLayerEffect(new CTransTileEffect(this, coord, TI_DOOR_YO));
				AddShadeEffect(wDrawX,wDrawY,BlueGreen);
			break;

			case OA_CLOSE:
				AddLastLayerEffect(new CTransTileEffect(this, coord, TI_DOOR_Y));
				AddShadeEffect(wDrawX,wDrawY,Red);
			break;
			
			default:
				ASSERTP(false, "Bad orb agent.");
			break;
		}
	}

	return true;
}

//*****************************************************************************
void CEditRoomWidget::AddOrbAgentsEffect(
//Add an effect to display the affect an orb has on yellow doors in the room.
//
//Params:
	COrbData *pOrb,	//(in) Orb data to display
   const bool bEditingOrb)//(in) whether an orb is being edited (and should be drawn)
                      //(default = true) if false, a door is being edited; i.e.,
                      //effect of all orbs on one door is being displayed,
                      //not one orb on all its doors.
{
	ASSERT(pOrb);

	//Remove last effect to repaint affected areas
	this->pLastLayerEffects->RemoveEffectsOfType(ETRANSTILE);
	this->pLastLayerEffects->RemoveEffectsOfType(ESHADE);

   if (!this->pLastLayerEffects->ContainsEffectOfType(EORBHIT))
	   AddStrikeOrbEffect(*pOrb, bEditingOrb);

	ResetPlot();	//tracks which door squares are highlighted
	//Highlight any door/orb mouse is over.
	COrbAgentData TentativeOrbAgent;
	TentativeOrbAgent.wX = this->wEndX;
	TentativeOrbAgent.wY = this->wEndY;
	TentativeOrbAgent.wAction = OA_NULL;
   const bool bDrawingSomething = (bEditingOrb ?
      AddDoorEffect(&TentativeOrbAgent) : AddOrbEffect(&TentativeOrbAgent));
	if (bDrawingSomething && pOrb->wAgentCount > 0)
		ResetPlot();	//call again after door/orb highlight

	if (pOrb->wAgentCount == 0)
	{
		//Show lightning from orb to mouse cursor -- ready to place an agent.
		this->wOX = pOrb->wX;
		this->wOY = pOrb->wY;
	} else {
		this->wOX = static_cast<UINT>(-1);	//don't show lightning to mouse cursor

	   //For each orb agent...
		COrbAgentData *pSeek = pOrb->parrAgents, *pStop = pSeek + pOrb->wAgentCount;
		while (pSeek != pStop)
		{
         if (bEditingOrb)
			   AddDoorEffect(pSeek);
         else
            AddOrbEffect(pSeek);
			++pSeek;
		}
	}
}

//*****************************************************************************
void CEditRoomWidget::AddOrbAgentToolTip(
//Adds a tool tip near (wX,wY).
//
//Params:
	const UINT wX, const UINT wY, const UINT wAgentType)	//(in)
{
   UINT wNearX, wNearY;
   wNearX = (wX < this->pRoom->wRoomCols - 1 ? wX + 1 : wX);
   wNearY = (wY > 0 ? wY - 1 : wY);

	const int x = this->x + wNearX * CX_TILE;
	const int y = this->y + wNearY * CY_TILE;
	this->pLastLayerEffects->RemoveEffectsOfType(ETOOLTIP);
	switch (wAgentType)
	{
		case OA_NULL: break;
		case OA_TOGGLE: AddToolTipEffect(x, y, MID_OrbAgentToggle);	break;
		case OA_OPEN: AddToolTipEffect(x, y, MID_OrbAgentOpen);	break;
		case OA_CLOSE: AddToolTipEffect(x, y, MID_OrbAgentClose);	break;
	}
}

//*****************************************************************************
void CEditRoomWidget::AddMonsterSegmentEffect(
//Add an effect to display a long monster segment to room.
//
//Params:
	const UINT wMonsterType)			//(in)	Monster to be placed.
{
	UINT wX, wY, wTileNo;

	ASSERT(wMonsterType == M_SERPENT);	//currently, only long monster

	this->pLastLayerEffects->RemoveEffectsOfType(ETRANSTILE);
	this->pLastLayerEffects->RemoveEffectsOfType(ESHADE);

	//Save segment start and end.
	const UINT wSX = this->monsterSegment.wSX;
	const UINT wSY = this->monsterSegment.wSY;
	this->monsterSegment.wEX = this->wEndX;
	this->monsterSegment.wEY = this->wEndY;

	//Only show tiles in a horizontal or vertical direction.
	//Determine which way to display.
	const bool bHorizontal = this->monsterSegment.bHorizontal =
			(abs(static_cast<int>(this->wEndX) - static_cast<int>(wSX))
				>= abs(static_cast<int>(this->wEndY) - static_cast<int>(wSY)));
   const UINT wMinX =
			(bHorizontal ? min(wSX,this->wEndX) : wSX);
	const UINT wMinY =
			(bHorizontal ? wSY : min(wSY,this->wEndY));
	const UINT wMaxX =
			(bHorizontal ? max(wSX,this->wEndX) : wMinX);
	const UINT wMaxY =
			(bHorizontal ? wMinY : max(wSY,this->wEndY));
	const UINT wDirection = this->monsterSegment.wDirection = (bHorizontal ?
			(this->wEndX > wSX ? W : E) :
			(this->wEndY > wSY ? N : S));
	const bool bSegment = (wMaxY != wMinY || wMaxX != wMinX);
	bool bHead;

	//Calculate new pending tail position.
	this->monsterSegment.wTailX = (bHorizontal ? this->wEndX : wMinX);
	this->monsterSegment.wTailY = (bHorizontal ? wMinY : this->wEndY);

	for (wY=wMinY; wY<=wMaxY; wY++)
		for (wX=wMinX; wX<=wMaxX; wX++)
		{
			bHead = false;
			//Calculate tile to display.
			if (wX == wSX && wY == wSY)
			{
				if (wX == this->monsterSegment.wHeadX &&
						wY == this->monsterSegment.wHeadY)
				{
					//Show head.
					switch (wDirection)
					{
						case N: wTileNo = TI_SNK_N;	break;
						case S: wTileNo = TI_SNK_S;	break;
						case E: wTileNo = TI_SNK_E;	break;
						case W: wTileNo = TI_SNK_W;	break;
					}
					bHead = true;
				} else if ((wX == this->monsterSegment.wTailX) &&
						(wY == this->monsterSegment.wTailY))
				{
					//Only one tile -- it's the tail.
					//Since it would paint over where a tail should already be,
					//don't have to do anything here.
					continue;
				} else {
					//Show a twist (a turn where the tail currently is).
					wTileNo = GetSerpentTurnTile(wX,wY,wDirection,true);
				}
			} else if ((wX == this->monsterSegment.wTailX) &&
						(wY == this->monsterSegment.wTailY))
			{
				//Show tail.
				wTileNo = GetSerpentTailTile(wX,wY,wDirection,true);
			} else {
				//Show straight segment.
				//If backtracking, erase.
				wTileNo = GetSerpentStraightTile(wX,wY,wDirection,true);
			}

			if (IsSafePlacement(wMonsterType + M_OFFSET,wX,wY))
			{
				CCoord coord(wX,wY);
				AddLastLayerEffect(new CTransTileEffect(this, coord, wTileNo));
				if (bHead && !bSegment)
					AddShadeEffect(wX,wY,Red);	//can't plot only the head
			}
			else
				//Obstacle there -- can't place.
				AddShadeEffect(wX,wY,Red);
		}
}

//*****************************************************************************
UINT CEditRoomWidget::GetSerpentStraightTile(
//Show/plot tail in the proper direction.
//
//Returns: tile to show/plot
//
//Params:
	const UINT wX, const UINT wY, const UINT wDirection,	//(in)
	const bool bShow)	//(in) Whether this is for display or room plotting
const
{
	const UINT wTTileNo = this->pRoom->GetTSquare(wX,wY);
	UINT wTileNo;
	switch (wDirection)
	{
		case W:
		case E:
			//Horizontal
			switch (wTTileNo)
			{
				case T_SNK_EW: wTileNo = T_EMPTY;	break;	//erasing
				case T_SNK_NW: wTileNo = T_SNK_NE;	break;	//changing a turn
				case T_SNK_NE: wTileNo = T_SNK_NW;	break;
				case T_SNK_SW: wTileNo = T_SNK_SE;	break;
				case T_SNK_SE: wTileNo = T_SNK_SW;	break;
				default: wTileNo = T_SNK_EW;	break;			//continuing straight
			}
			break;
		case N:
		case S:
			//Vertical
			switch (wTTileNo)
			{
				case T_SNK_NS:	wTileNo = T_EMPTY;	break;
				case T_SNK_NW: wTileNo = T_SNK_SW;	break;
				case T_SNK_NE: wTileNo = T_SNK_SE;	break;
				case T_SNK_SW: wTileNo = T_SNK_NW;	break;
				case T_SNK_SE: wTileNo = T_SNK_NE;	break;
				default: wTileNo = T_SNK_NS;	break;
			}
			break;
	}

	if (bShow)
		return (wTileNo == T_EMPTY ? TI_EMPTY_L :
				GetTileImageForTileNo(wTileNo));

	return wTileNo;
}

//*****************************************************************************
UINT CEditRoomWidget::GetSerpentTailTile(
//Show/plot tail in the proper direction.
//
//Returns: tile to show/plot
//
//Params:
	const UINT wX, const UINT wY, const UINT wDirection,	//(in)
	const bool bShow)	//(in) Whether this is for display or room plotting
const
{
	const UINT wTTileNo = this->pRoom->GetTSquare(wX,wY);
	UINT wTileNo;
	switch (wDirection)
	{
		case N:
			switch (wTTileNo)
			{
				case T_SNK_NS: wTileNo = T_SNKT_N;	break;
				case T_SNK_SE: wTileNo = T_SNKT_E;	break;
				case T_SNK_SW: wTileNo = T_SNKT_W;	break;
				default: wTileNo = T_SNKT_S;	break;
			}
			break;							
		case S:
			switch (wTTileNo)
			{
				case T_SNK_NS: wTileNo = T_SNKT_S;	break;
				case T_SNK_NE: wTileNo = T_SNKT_E;	break;
				case T_SNK_NW: wTileNo = T_SNKT_W;	break;
				default: wTileNo = T_SNKT_N;	break;
			}
			break;
		case E: 
			switch (wTTileNo)
			{
				case T_SNK_EW: wTileNo = T_SNKT_E;	break;
				case T_SNK_NW: wTileNo = T_SNKT_N;	break;
				case T_SNK_SW: wTileNo = T_SNKT_S;	break;
				default: wTileNo = T_SNKT_W;	break;
			}
			break;							
		case W: 
			switch (wTTileNo)
			{
				case T_SNK_EW: wTileNo = T_SNKT_W;	break;
				case T_SNK_NE: wTileNo = T_SNKT_N;	break;
				case T_SNK_SE: wTileNo = T_SNKT_S;	break;
				default: wTileNo = T_SNKT_E;	break;
			}
			break;							
		default: ASSERTP(false, "Bad orientation."); return 0;
	}

	if (bShow) return GetTileImageForTileNo(wTileNo);

	return wTileNo;
}

//*****************************************************************************
UINT CEditRoomWidget::GetSerpentTurnTile(
//Show/plot a turn in a serpent in the proper direction.
//(Change a tail piece into a turn.  Erase if backing up.)
//
//Returns: tile to show/plot
//
//Params:
	const UINT wX, const UINT wY, const UINT wDirection,	//(in)
	const bool bShow)	//(in) Whether this is for display or room plotting
const
{
	UINT wTileNo;
	switch (this->pRoom->GetTSquare(wX,wY))
	{
	case T_SNKT_S:
		switch (wDirection)
		{
			case N: wTileNo = T_SNK_NS;	break;	//continue
			case S: wTileNo = T_EMPTY;		break;	//erase
			case E: wTileNo = T_SNK_SE;	break;	//turn
			case W: wTileNo = T_SNK_SW;	break;
		}
		break;
	case T_SNKT_N:
		switch (wDirection)
		{
			case N: wTileNo = T_EMPTY;		break;
			case S: wTileNo = T_SNK_NS;	break;
			case E: wTileNo = T_SNK_NE;	break;
			case W: wTileNo = T_SNK_NW;	break;
		}
		break;
	case T_SNKT_E:
		switch (wDirection)
		{
			case N: wTileNo = T_SNK_NE;	break;
			case S: wTileNo = T_SNK_SE;	break;
			case E: wTileNo = T_EMPTY;		break;
			case W: wTileNo = T_SNK_EW;	break;
		}
		break;
	case T_SNKT_W:
		switch (wDirection)
		{
			case N: wTileNo = T_SNK_NW;	break;
			case S: wTileNo = T_SNK_SW;	break;
			case E: wTileNo = T_SNK_EW;	break;
			case W: wTileNo = T_EMPTY;		break;
		}
		break;
	default: ASSERTP(false, "Bad serpent turn tile.");	return 0;
	}

	if (bShow)
		return (wTileNo == T_EMPTY ? TI_EMPTY_L :
				GetTileImageForTileNo(wTileNo));

	return wTileNo;
}

//*****************************************************************************
void CEditRoomWidget::AddPendingPlotEffect(
//Adds a PendingPlot effect to room.
//Erase previous PendingPlot effects.
//
//Params:
	const UINT wObjectNo,			//(in)	Object to be placed.
	const UINT* pwTileImageNo,		//(in)	Image of object to place.
	const UINT wXSize, const UINT wYSize,	//(in) Dimensions of object being displayed
											//(default: 1x1)
	const bool bSinglePlacement)	//(in)	Show only one object (default=false)
{
	this->pLastLayerEffects->RemoveEffectsOfType(EPENDINGPLOT);
	AddLastLayerEffect(
		new CPendingPlotEffect(this, wObjectNo, pwTileImageNo, wXSize, wYSize));
	this->bSinglePlacement = bSinglePlacement;
}

//*****************************************************************************
bool CEditRoomWidget::IsObjectReplaceable(
//Returns: whether this object can be put down on (replacing) itself
//
//Params:
   const UINT wObject,     //(in) object being placed
   const UINT wTileLayer,  //(in) object layer
   const UINT wTileNo)     //(in) object on tile layer
const
{
   switch (wTileLayer)
   {
   case 0:
      //Floor types can replace themselves,
      //except for doors: to facilitate editing them when clicked on.
      if (wObject == T_DOOR_Y || wObject == T_DOOR_YO)
         return false;
      if (wObject == T_OB_1)  //can't clobber existing 2x2 obstacles
         return false;
      return wObject == wTileNo;
   case 1:
      //Arrows can replace arrows.
      if (bIsArrow(wObject) && bIsArrow(wTileNo))
         return true;
      //Can't replace customizable objects: scrolls and orbs.
      if (wObject == T_SCROLL || wObject == T_ORB)
         return false;
      return wObject == wTileNo;
   case 2: 
      //Same type of monster can replace itself, except long monsters.
      if (wObject == T_SERPENT)
         return false;
      if (wObject >= TILE_COUNT && wObject < TILE_COUNT + MONSTER_COUNT)
         return wObject == wTileNo;
      return false;
   default: ASSERTP(false, "Bad tile layer."); return false;
   }
}

//*****************************************************************************
bool CEditRoomWidget::IsSafePlacement(
//For room editing.
//
//Returns: whether placement of currently selected object is allowed at
//specified room square or not.
//
//Params:
	const UINT wSelectedObject,		//(in)	Object to be placed
	const UINT wX, const UINT wY,		//(in)	Coords where placement is pending.
   const UINT wO,                   //(in)   Orientation of object (default = NO_ORIENTATION)
	const bool bAllowSelf)				//(in)	In general, don't allow plotting
									//over same object (default = false).  A value of
									//true is to check the room state for errors,
									//when placement isn't actually occurring.
const
{
	if (!this->pRoom->IsValidColRow(wX,wY))
		return false;	//out of bounds

	if (this->pPlotted[ARRAYINDEX(wX,wY)])
		return true;	//this part can be replaced

	//Get what's on the other layers to see if they're compatible.
	UINT wTileNo[3];
	const UINT wTileLayer = wSelectedObject == T_SWORDSMAN ? 1 :	//handle placing player start position
			wSelectedObject == T_NOMONSTER ? 2 : TILE_LAYER[wSelectedObject];
	const UINT wSquareIndex = ARRAYINDEX(wX,wY);
	wTileNo[0] = this->pRoom->pszOSquares[wSquareIndex];
	wTileNo[1] = this->pRoom->pszTSquares[wSquareIndex];
	const CMonster *pMonster = this->pRoom->pMonsterSquares[wSquareIndex];
   wTileNo[2] = pMonster ? pMonster->wType : T_NOMONSTER;

   static const UINT wEmptyTile[] = {T_FLOOR, T_EMPTY, T_NOMONSTER};

	//Don't allow objects to clobber existing objects on their layer.
	if (wTileLayer == 2 && wSelectedObject != T_NOMONSTER)
	{
		//Monster can't overwrite a monster of a different type.
		if (!bAllowSelf && pMonster)
         if (pMonster->wType != wSelectedObject - TILE_COUNT)
            return false;
   } else {
      //Universal placement rules:
      //1. "Empty" tiles can be placed on anything to erase them.
      //2. If the tile being placed is different than what's already there,
      //    it can only be placed if what's there is "empty".
      //3. If the tile being placed is the same as the one already there,
      //    it can be placed if all that's changing is the object's orientation.
      if ((wSelectedObject != wEmptyTile[wTileLayer]) &&
		      (!bAllowSelf || (wTileNo[wTileLayer] != wSelectedObject)) &&
            (wTileNo[wTileLayer] != wEmptyTile[wTileLayer]) &&
            !IsObjectReplaceable(wSelectedObject, wTileLayer, wTileNo[wTileLayer]))
		   return false;
	}

   //Don't allow placing anything unsafe under swordsman's starting position.
   if (this->bShowingSwordsman)
   {
      if (this->wPX == wX && this->wPY == wY)
      {
		   if (!(wTileNo[0] == T_FLOOR || wTileNo[0] == T_TRAPDOOR ||
					   wTileNo[0] == T_DOOR_YO || wTileNo[0] == T_DOOR_Y ||
                  wTileNo[0] == T_DOOR_C || wTileNo[0] == T_DOOR_M || 
                  wTileNo[0] == T_DOOR_R))
            return false;
         if (wTileNo[1] == T_POTION_I || wTileNo[1] == T_POTION_K || wTileNo[0] == T_CHECKPOINT ||
                  wTileNo[1] == T_TAR || wTileNo[1] == T_ORB || bIsArrow(wTileNo[1]))
            return false;
         if (pMonster)
            return false;
      }
      //No items affected by sword can be placed under it.
      const UINT wSX = this->wPX + nGetOX(this->wPO), wSY = this->wPY + nGetOY(this->wPO);
      if (wSX == wX && wSY == wY)
      {
         switch (wTileLayer)
         {
            case 0:
               if (wSelectedObject == T_WALL_B)
                  return false;
               break;
            case 1:
               if (wSelectedObject == T_TAR || wSelectedObject == T_ORB)
                  return false;
               break;
            case 2:
               //Sword can be placed on serpents, but no other monster.
               if (wSelectedObject != T_NOMONSTER && wSelectedObject != T_SERPENT)
                  return false;
               break;
         }
      }
   }

	//Checks on other layers.
	switch (wSelectedObject)
	{
		//O-layer stuff
		case T_EMPTY:
		case T_FLOOR:
			//Empty stuff goes with anything
			return true;
		case T_TRAPDOOR:
			//Trapdoor -- anything can be on it.
			return true;
      case T_CHECKPOINT:
         //Checkpoint - can't be under the swordsman
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
         break;
		case T_OB_1:
			//Obstacle -- Matches with nothing.
         if (wTileNo[1] != T_EMPTY || pMonster)
            return false;
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
         break;
      case T_PIT:
         //Pit -- wwings can be on it.
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
			if (!bAllowSelf && wTileNo[0] == T_PIT)
				return false;
			return (wTileNo[1] == T_EMPTY || bIsArrow(wTileNo[1])) &&
               (!pMonster || pMonster->wType == M_WWING);
      case T_STAIRS:
         //Don't allow stair juxtaposition w/ t-layer items (except tar).
         if (wTileNo[1] == T_POTION_I || wTileNo[1] == T_POTION_K ||
               wTileNo[1] == T_SCROLL || bIsArrow(wTileNo[1]))
            return false;
         break;
      case T_DOOR_YO:
         //Open doors can't have orbs on them, but can have tar and serpents.
         if (wTileNo[1] == T_ORB) return false;
         if (wTileNo[1] == T_TAR) return true;
         if (bIsSerpent(wTileNo[1])) return true;
         break;
      case T_WALL:
      case T_WALL_B:
		  if (wTileNo[2] != T_NOMONSTER) return false;
		  if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
		  // FALL-THROUGH
      case T_DOOR_C:
      case T_DOOR_M:
      case T_DOOR_R:
      case T_DOOR_Y:
         //Walls/Doors can't have orbs on them.
         //But can have tar.
         if (wTileNo[1] == T_ORB) return false;
         if (wTileNo[1] == T_TAR) return true;
         break;

		//T-layer stuff
		case T_SWORDSMAN:
      {
         //copied from above
			if (!(wTileNo[0] == T_FLOOR || wTileNo[0] == T_TRAPDOOR ||
					wTileNo[0] == T_DOOR_YO || wTileNo[0] == T_DOOR_Y ||
               wTileNo[0] == T_DOOR_C || wTileNo[0] == T_DOOR_M || 
               wTileNo[0] == T_DOOR_R) || pMonster)
            return false;
         //Sword cannot be placed where it will affect an item.
         if (wO == NO_ORIENTATION)
            return true;
         const UINT wSX = wX + nGetOX(wO), wSY = wY + nGetOY(wO);
         if (!this->pRoom->IsValidColRow(wSX,wSY))
            return true;
	      const UINT wSwordSquareIndex = ARRAYINDEX(wSX,wSY);
	      UINT wSwordTileNo[3];
	      wSwordTileNo[0] = this->pRoom->pszOSquares[wSwordSquareIndex];
	      wSwordTileNo[1] = this->pRoom->pszTSquares[wSwordSquareIndex];
	      const CMonster *pMonster = this->pRoom->pMonsterSquares[wSwordSquareIndex];
         wSwordTileNo[2] = pMonster ? pMonster->wType : T_NOMONSTER;
         return (!(wSwordTileNo[0] == T_WALL_B || wSwordTileNo[1] == T_TAR ||
               wSwordTileNo[1] == T_ORB ||
               (wSwordTileNo[2] != T_NOMONSTER && wSwordTileNo[2] != M_SERPENT)));
      }
		case T_ORB:
			//Only on normal floor.
		   if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
			return (wTileNo[0] == T_FLOOR || wTileNo[0] == T_TRAPDOOR) && !pMonster;
		case T_POTION_I:
		case T_POTION_K:
		case T_SCROLL:
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
			//Not on monsters that use/affect the t-layer.
			if (wTileNo[2] == M_SERPENT || wTileNo[2] == M_TARMOTHER) return false;
			//Can't go on things player never steps on and off of.
			return (wTileNo[0] != T_WALL && wTileNo[0] != T_PIT &&
					wTileNo[0] != T_STAIRS && wTileNo[0] != T_OB_1);
		case T_ARROW_NW: case T_ARROW_N: case T_ARROW_NE: case T_ARROW_W:
		case T_ARROW_E: case T_ARROW_SW: case T_ARROW_S: case T_ARROW_SE:
			//Not on stairs or obstacles or serpents/tar mothers.
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
         return ((wTileNo[0] != T_STAIRS && wTileNo[0] != T_OB_1) &&
					(wTileNo[1] == T_EMPTY || bIsArrow(wTileNo[1])) &&
               !(wTileNo[2] == M_SERPENT || wTileNo[2] == M_TARMOTHER));

		case T_TAR:
			//Can't go on a pit.
			//Only tar mother can be on tar.
         if (pMonster && pMonster->wType != M_TARMOTHER) return false;
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
         return (wTileNo[0] != T_PIT);

		//M-layer stuff
		case T_NOMONSTER:
         //Can always erase monsters.
         return true;
		case T_SERPENT:
			//can't go on anything in t-layer, or monsters
			if (wTileNo[1] != T_EMPTY && (!bIsSerpent(wTileNo[1]) || !bAllowSelf))
            return false;
         if (pMonster && !bAllowSelf)
            return false;
			//no break
		case T_ROACH:
		case T_QROACH:
		case T_GOBLIN:
		case T_EYE:
		case T_TARBABY:
		case T_BRAIN:
		case T_SPIDER:
      case T_NEATHER:
			//Ground movement types
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
			return ((wTileNo[0] == T_FLOOR || wTileNo[0] == T_DOOR_YO ||
					wTileNo[0] == T_TRAPDOOR || wTileNo[0] == T_CHECKPOINT) &&
					!(wTileNo[1] == T_ORB || wTileNo[1] == T_TAR));
		case T_TARMOTHER:
			//Also can (should) be on tar.
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
			return (wTileNo[1] == T_TAR) &&
					(wTileNo[0] == T_FLOOR || wTileNo[0] == T_DOOR_YO ||
					wTileNo[0] == T_TRAPDOOR || wTileNo[0] == T_CHECKPOINT);

		case T_WWING:
			//Air movement types
         if (this->bShowingSwordsman && this->wPX == wX && this->wPY == wY) return false;
			return ((wTileNo[0] == T_FLOOR || wTileNo[0] == T_DOOR_YO ||
					wTileNo[0] == T_TRAPDOOR || wTileNo[0] == T_CHECKPOINT ||
					wTileNo[0] == T_PIT || wTileNo[0] == T_STAIRS) &&
					!(wTileNo[1] == T_ORB || wTileNo[1] == T_TAR));
	}

	if (bIsSerpent(wSelectedObject))
	{
		//Serpent pieces -- can't be plotted over monsters or pieces.
		return (wTileNo[1] == T_EMPTY || wTileNo[0] == T_FLOOR ||
            wTileNo[0] == T_TRAPDOOR || wTileNo[0] == T_DOOR_YO ||
            wTileNo[0] == T_CHECKPOINT);
	}

   //Allow placement if square is empty.
   if (wTileNo[wTileLayer] == wSelectedObject &&
         (bAllowSelf || IsObjectReplaceable(wSelectedObject, wTileLayer, wTileNo[wTileLayer])))
      return true;
   return wTileNo[wTileLayer] == wEmptyTile[wTileLayer];
}

//*****************************************************************************
bool CEditRoomWidget::LoadFromRoom(
//Loads widget from a room.
//
//Params:
	CDbRoom *pRoom)
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pRoom);
	this->pRoom = pRoom;

	//Show level starting position?
	CDbLevel *pLevel = g_pTheDB->Levels.GetByID(pRoom->dwLevelID);
	ASSERT(pLevel);	//Corrupt DB.
	if (pRoom->dwRoomID == pLevel->dwRoomID)
	{
		this->wPX = pLevel->wX;
		this->wPY = pLevel->wY;
		this->wPO = pLevel->wO;
		ShowSwordsman();
	} else {
		this->pCurrentGame = NULL;
      this->wPX = static_cast<UINT>(-1);
		HideSwordsman();
	}
	delete pLevel;

	//Update vars used for comparison of widget to current game.
	this->wStyle = pRoom->wStyle;
	this->dwRoomX = pRoom->dwRoomX;
	this->dwRoomY = pRoom->dwRoomY;

	ClearEffects();

	//Load tile images.
	if (!g_pTheDBM->LoadTileImagesForStyle(this->wStyle)) return false;

   delete[] this->pPlotted;
   this->pPlotted = new bool[pRoom->CalcRoomArea()];

	//Set tile image arrays to new current room.
	ResetForPaint();

	return true;
}

//*****************************************************************************
void CEditRoomWidget::ResetRoom()
//Unload a loaded room.
{
   this->pRoom = NULL;
 	this->pCurrentGame = NULL;
}

//*****************************************************************************
bool CEditRoomWidget::Plotted(const UINT wCol, const UINT wRow) const
{
   ASSERT(this->pPlotted);
	return this->pPlotted[ARRAYINDEX(wCol,wRow)];
}

//*****************************************************************************
void CEditRoomWidget::ResetPlot(void)
//Reset plotted flag on all squares.
{
	memset(this->pPlotted, false, this->pRoom->CalcRoomArea() * sizeof(bool));
}

//*****************************************************************************
void CEditRoomWidget::SetPlot(
//Mark a square as having been plotted.
//
//Params:
	const UINT wCol, const UINT wRow)	//(in)	Square to set plot flag for.
{
	ASSERT(this->pRoom->IsValidColRow(wCol,wRow));

	this->pPlotted[ARRAYINDEX(wCol,wRow)] = true;
}

//*****************************************************************************
void CEditRoomWidget::Paint(
//Plots current room to display.
//Displays editing state.
//
//Params:
	bool bUpdateRect)			//(in)	If true (default) and destination
								//		surface is the screen, the screen
								//		will be immediately updated in
								//		the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

   if (!this->pRoom)
	{
		DrawPlaceholder();
		return;
	}

 	SDL_Surface *pDestSurface = GetDestSurface();

	//1. Blit pre-rendered room.
	SDL_Rect src = {this->x, this->y, this->w, this->h};
	SDL_Rect dest = {this->x, this->y, this->w, this->h};
	ASSERT(this->pRoomSnapshotSurface);
	if (this->bAllDirty)
	{
		//Blit entire room.
		SDL_BlitSurface(this->pRoomSnapshotSurface, &src, pDestSurface, &dest);
	} else {
		src.w = dest.w = CX_TILE;
		src.h = dest.h = CY_TILE;

      AnimateMonsters();

      //Touch up tiles where effects were drawn last frame.
		DirtyEffectTiles(false, false, false);

		//Redraw all tiles that have changed.
		BlitDirtyRoomTiles();
	}

	//2. Draw effects that go on top of room image, under monsters/swordsman.
	this->pTLayerEffects->DrawEffects();

   //3. Repaint player/monsters.
   if (this->bAllDirty)
   {
      //Draw monsters (that aren't killing swordsman).
      DrawMonsters(this->pRoom->pFirstMonster, pDestSurface, false);
	} else {
		//Paint monsters whose tiles have been repainted.
		//Also check whether monster is raised up and tile above is dirty.
		//This signifies the monster's image has been clipped on top
		//and must be repainted.
	   TILEINFO *pbMI;
		CMonster *pMonster = this->pRoom->pFirstMonster;
		while (pMonster)
		{
			pbMI = this->pTileInfo + ARRAYINDEX(pMonster->wX,
					pMonster->wY);
			ASSERT(pMonster->wType != M_MIMIC);
			if (pbMI->dirty || (pbMI->raised && pMonster->wY > 0 &&
						this->pTileInfo[ARRAYINDEX(pMonster->wX,
								pMonster->wY-1)].dirty))
				DrawMonster(pMonster, this->pRoom, pDestSurface, false);
			
			pMonster = pMonster->pNext;
		}
	}

   //4. Draw swordsman.
   if (this->bShowingSwordsman && this->wPX != static_cast<UINT>(-1))
   {
      CSwordsman swordsman;
      swordsman.wX = this->wPX;
      swordsman.wY = this->wPY;
      swordsman.SetOrientation(this->wPO);
      swordsman.bIsVisible = true;
      DrawSwordsman(swordsman, pDestSurface);
   }

	//5. Draw effects that go on top of everything else drawn in the room.
	this->pLastLayerEffects->DrawEffects();

	//Everything should have been (re)painted by now.
	//Undirty all the tiles.
	this->bAllDirty = false;

   //???need?
   TILEINFO *pbMI;
	pbMI = this->pTileInfo;
	TILEINFO *const pbMIStop = pbMI +
			CDrodBitmapManager::DISPLAY_ROWS * CDrodBitmapManager::DISPLAY_COLS;
	while (pbMI != pbMIStop)
		(pbMI++)->dirty = 0;

	//Orb/door editing: show lightning from orb/door to mouse cursor.
	if (this->wOX != static_cast<UINT>(-1))
	{
		static const UINT CX_TILE_HALF = CX_TILE / 2;
		static const UINT CY_TILE_HALF = CY_TILE / 2;
		DrawBoltInRoom(this->x + this->wOX * CX_TILE + CX_TILE_HALF,
				this->y + this->wOY * CY_TILE + CY_TILE_HALF,
				this->x + this->wEndX * CX_TILE + CX_TILE_HALF,
				this->y + this->wEndY * CY_TILE + CY_TILE_HALF);
      const UINT xBuffer = this->wOX < this->wEndX ? 2 : -2;
      const UINT yBuffer = this->wOY < this->wEndY ? 3 : -3;
      DirtyTileRect(this->wOX - xBuffer, this->wOY - yBuffer,
            this->wEndX + xBuffer, this->wEndY + yBuffer);
	}

	//Paint widget children on top of everything.
	PaintChildren();

	//Put it up on the screen.
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CEditRoomWidget::Repaint(
//Repaints the "physical room" (the o- and t-layers) within the given rectangle.
//Uses "smart" room pre-rendering, only painting tiles that have changed since
//last call.
//
//Params:
	int wCol, int wRow,		//(in) top-left tile coords
	int wWidth, int wHeight)
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	const UINT wRowOffset = this->pRoom->wRoomCols - wWidth;
	const UINT wStartPos = wRow * this->pRoom->wRoomCols + wCol;
	const UINT wXEnd = wCol + wWidth;
	const UINT wYEnd = wRow + wHeight;

	SDL_Surface *pDestSurface = this->pRoomSnapshotSurface;	//draw to here

#define DrawRoomTile(wX,wY,wTileImageNo) g_pTheBM->BlitTileImage(\
		wTileImageNo, this->x + wX * CX_TILE,\
		this->y + wY * CY_TILE, pDestSurface, 255)
#define DrawTransparentRoomTile(wX,wY,wTileImageNo,opacity)\
      g_pTheBM->BlitTileImage(wTileImageNo, this->x + wX * CX_TILE,\
		this->y + wY * CY_TILE, pDestSurface, (opacity))
#define DrawLayeredRoomTiles(wX, wY, wBottomTileImageNo, wTopTileImageNo) \
		g_pTheBM->BlitLayeredTileImage(wBottomTileImageNo, wTopTileImageNo,\
		this->x + wX * CX_TILE,\
		this->y + wY * CY_TILE, pDestSurface, 255)

	UINT *pwOTI = this->pwOSquareTI + wStartPos;
	UINT *pwTTI = this->pwTSquareTI + wStartPos;
	const EDGES *pbE = this->pbEdges + wStartPos;
	const TILEINFO *pbMI = this->pTileInfo + wStartPos;
	UINT wX, wY;

	//Note: Don't need to lock this surface.
	ASSERT(!SDL_MUSTLOCK(pDestSurface));
	bool bIsCheckpoint, bIsTransparentTar;
	for (wY = wRow; wY < wYEnd; ++wY)
	{
		for (wX = wCol; wX < wXEnd; ++wX)
		{
			if (this->bAllDirty || pbMI->dirty)
			{
				bIsCheckpoint = this->bShowCheckpoints &&
						this->pRoom->GetOSquare(wX, wY)==T_CHECKPOINT;
				bIsTransparentTar = this->pRoom->GetTSquare(wX, wY)==T_TAR;
				
				//If there are no edges or checkpoint, can optimize tile drawing.
				if (!(pbE->drawEastEdge ||
						pbE->drawNorthEdge || pbE->drawSouthEdge || pbE->drawWestEdge)
					&& !bIsCheckpoint && !bIsTransparentTar)
				{
					if (*pwTTI != TI_TEMPTY)
					{
						//Draw o- and t-layers simultaneously.
						DrawLayeredRoomTiles(wX, wY, *pwOTI, *pwTTI);
					}
					else
					{
						//Only draw opaque layer.
						DrawRoomTile(wX, wY, *pwOTI);
					}
				} else {
					//1. Draw opaque layer.
					DrawRoomTile(wX, wY, *pwOTI);

					//Draw checkpoint on top of floor.
					if (bIsCheckpoint)
					{
						DrawRoomTile(wX, wY, TI_CHECKPOINT);
					}

					//2. Draw outline around squares that need it.
					DrawTileEdges(wX, wY, pbE, pDestSurface);

					//3. Draw transparent layer.
					if (*pwTTI != TI_TEMPTY)
					{
                  if (bIsTransparentTar)
						   DrawTransparentRoomTile(wX, wY, *pwTTI, 127);
                  else 
                     DrawRoomTile(wX, wY, *pwTTI);
					}
				}
			}

			//Advance all pointers to next square.
			++pwOTI;
			++pwTTI;
			++pbMI;
			++pbE;
		}
		pwOTI += wRowOffset;
		pwTTI += wRowOffset;
		pbMI += wRowOffset;
		pbE += wRowOffset;
	}

#undef DrawRoomTile
#undef DrawLayeredRoomTiles
#undef DrawTransparentRoomTile
}

//
// Private methods
//

//*****************************************************************************
void CEditRoomWidget::DrawMonsters(
//Draws monsters.
//
//Params:
	CMonster *const pMonsterList,	//(in)	Monsters to draw.
   SDL_Surface *pDestSurface,
	const bool bActionIsFrozen)	//(in)	Whether action is currently stopped.
{
	CMonster *pMonster = pMonsterList;
	while (pMonster)
	{
		DrawMonster(pMonster, this->pRoom, pDestSurface, bActionIsFrozen);
		pMonster = pMonster->pNext;
	}
}

//******************************************************************************
inline void GetMinMax(
//Calculates the min and max of two vars.
//
//Params:
	const UINT v1, const UINT v2,	//(in)
	UINT &vMin, UINT &vMax)			//(out)
{
	vMin = min(v1,v2);
	vMax = max(v1,v2);
}

//*****************************************************************************
void CEditRoomWidget::HandleMouseDown(
//Handles a mouse down event.
//
//Params:
	const SDL_MouseButtonEvent &Button)	//(in)	Event to handle.
{
	this->wStartX = this->wMidX = this->wEndX = (Button.x - this->x) / CX_TILE;
	this->wStartY = this->wMidY = this->wEndY = (Button.y - this->y) / CY_TILE;
}

//*****************************************************************************
void CEditRoomWidget::HandleDrag(
//Handles a mouse drag event.
//
//Params:
	const SDL_MouseMotionEvent &Motion)
{
	//Don't update coords when outside widget.
	if (Motion.x < this->x || Motion.y < this->y) return;
	if (Motion.x >= this->x + this->w || Motion.y >= this->y + this->h) return;

	//When placing only one object, keep it located at mouse cursor.
	if (this->bSinglePlacement)
	{
		this->wStartX = (Motion.x - this->x) / CX_TILE;
		this->wStartY = (Motion.y - this->y) / CY_TILE;
	}

	//Preserve mouse down position.
	GetMinMax(this->wStartX, (Motion.x - this->x) / CX_TILE,
			this->wMidX, this->wEndX);
	GetMinMax(this->wStartY, (Motion.y - this->y) / CY_TILE,
			this->wMidY, this->wEndY);
}

//*****************************************************************************
void CEditRoomWidget::HandleMouseMotion(
//Handles a mouse motion event.
//
//Params:
	const SDL_MouseMotionEvent &Motion)
{
	if (this->bPlacing) return;

	//Don't update coords when outside widget.
	if (Motion.x < this->x || Motion.y < this->y) return;
	if (Motion.x >= this->x + this->w || Motion.y >= this->y + this->h) return;

	this->wEndX = (Motion.x - this->x) / CX_TILE;
	this->wEndY = (Motion.y - this->y) / CY_TILE;
}

//*****************************************************************************
void CEditRoomWidget::HandleMouseUp(
//Handles a mouse up event.
//
//Params:
	const SDL_MouseButtonEvent &Button)	//(in)	Event to handle.
{
	const UINT x1 = this->wStartX;
	const UINT y1 = this->wStartY;
	const UINT x2 = (Button.x - this->x) / CX_TILE;
	const UINT y2 = (Button.y - this->y) / CY_TILE;
	this->bMouseInBounds = false;

	//Don't update coords when outside widget.
	if (Button.x < this->x || Button.y < this->y) return;
	if (Button.x >= this->x + this->w || Button.y >= this->y + this->h) return;

	this->bMouseInBounds = true;
	GetMinMax(x1,x2,this->wStartX,this->wEndX);
	GetMinMax(y1,y2,this->wStartY,this->wEndY);
}

// $Log: EditRoomWidget.cpp,v $
// Revision 1.30  2005/03/15 21:43:07  mrimer
// Now items can't be put under tar mothers so they don't get clobbered by tar growth.
//
// Revision 1.29  2004/01/02 00:58:31  mrimer
// Fixed bug: wwings can be placed on tar.
//
// Revision 1.28  2003/12/01 19:16:04  mrimer
// Allowed replacing potions without error highlighting appearing.
//
// Revision 1.27  2003/11/16 03:16:41  mrimer
// Fixed bug: arrows can be placed under serpent heads.
//
// Revision 1.26  2003/10/10 16:04:38  mrimer
// Fixed cosmetic bugs: 'Neather marked as placement error; error highlights not redrawn when exiting orb agent placement.
//
// Revision 1.25  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.24  2003/08/16 01:54:18  mrimer
// Moved EffectList to FrontEndLib.  Added general tool tip support to screens.  Added RoomEffectList to room editor screen/widget.
//
// Revision 1.23  2003/08/07 16:09:34  mrimer
// Fixed bug: player's sword on serpent is marked as error.
//
// Revision 1.22  2003/08/05 01:39:25  mrimer
// Moved DROD-specific effects from FrontEndLib.
//
// Revision 1.21  2003/08/04 19:14:10  schik
// Monsters not allowed on tar.
//
// Revision 1.20  2003/08/03 13:40:55  schik
// Fixed a few bugs in IsSafePlacement()
//
// Revision 1.19  2003/08/02 02:01:01  mrimer
// Fixed bad T_DOOR_YO placement logic.
//
// Revision 1.18  2003/07/30 23:55:30  mrimer
// Fixed bug: placing serpent on other serpent's head.
//
// Revision 1.17  2003/07/30 04:23:06  mrimer
// Fixed bugs: Beethro and sword placement constraints not being handled correctly.
//
// Revision 1.16  2003/07/29 03:55:00  mrimer
// Fixed obstacle, stair placement rules.
//
// Revision 1.15  2003/07/29 02:12:07  schik
// Fixes for placing swordsman on monsters
//
// Revision 1.14  2003/07/28 22:03:08  schik
// Dis-allowed puting a wall under the swordsman
//
// Revision 1.13  2003/07/28 21:54:00  schik
// Dis-allowed placing walls under monsters
//
// Revision 1.12  2003/07/19 21:27:54  mrimer
// Added placement constraint on placing swordsman and his sword.
//
// Revision 1.11  2003/07/19 02:21:19  mrimer
// Fixed placement bugs.
//
// Revision 1.10  2003/07/17 01:52:31  mrimer
// Fixed bug: serpents being placed where t-layer objects are.
//
// Revision 1.9  2003/07/16 07:44:03  mrimer
// Fixed bug: can't replace wraithwings with wraithwings.
//
// Revision 1.8  2003/07/09 21:13:36  mrimer
// Fixed placement bugs.
//
// Revision 1.7  2003/07/07 23:29:29  mrimer
// Fixed some mistakes in the object placement permission logic.
//
// Revision 1.6  2003/07/03 21:44:51  mrimer
// Port warning/bug fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/07/01 20:29:01  mrimer
// Fixed bug: swordsman being drawn in every room.
//
// Revision 1.4  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/06/26 17:56:59  mrimer
// Tightened constraints of objects that can go on swordsman start position.
//
// Revision 1.2  2003/06/25 06:22:32  mrimer
// Some fixes for bugs introduced in the code transfer.
//
// Revision 1.1  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
