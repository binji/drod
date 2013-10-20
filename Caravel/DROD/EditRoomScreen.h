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
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef EDITROOMSCREEN_H
#define EDITROOMSCREEN_H

#include "RoomScreen.h"
#include "EditRoomWidget.h"
#include "../DRODLib/DbRooms.h"
#include <set>

//Conditions of the editing process.
enum EditState
{
	ES_PLACING,		//standard state: placing objects
	ES_ORB,			//setting orb agents
   ES_DOOR,       //setting orb agents for one door
	ES_SCROLL,		//entering scroll text
	ES_TESTROOM,	//selecting position to begin playing room
	ES_LONGMONSTER	//laying long monster segments
};

enum PlotType
{
	PLOT_NOTHING,	//nothing was plotted (due to illegal placement)
	PLOT_HEAD,		//only a head was plotted
	PLOT_DONE,		//nothing plotted (and we're done)
	PLOT_SEGMENT,	//a segment was plotted (2+ pieces)
	PLOT_ERROR		//neither a head nor tail was plotted
};

class CDbLevel;
class CMapWidget;
class CRoomWidget;
class CTabbedMenuWidget;
class CEditRoomScreen : public CRoomScreen
{
public:
	static void		FillInRoomEdges(CDbRoom* const pRoom);

	bool		SetRoom(const DWORD dwRoomID);	//loads level and room instances
	DWORD		GetRoomID() const {return (this->pRoom ? this->pRoom->dwRoomID : 0L);}

protected:
	friend class CDrodScreenManager;

	CEditRoomScreen();

	virtual bool   Load();
	virtual bool   SetForActivate();
	virtual void   Unload();

private:
	void		AddPlotEffect(const UINT wObjectNo);
	DWORD		AddRoom(const DWORD dwRoomX, const DWORD dwRoomY);
	void		ApplyPlayerSettings();

	static void		ClearList(list<CDbRoom*> &List);
	const UINT*		DisplaySelection(const UINT wObjectNo) const;
	void		EditObjects();
	bool		EditScrollText(const UINT wX, const UINT wY);
   bool     EraseAndPlot(const UINT wX, const UINT wY, const UINT wObjectToPlot,
         const bool bFixImmediately=true);
   void     EraseObjects();
	COrbAgentData* FindOrbAgentFor(const UINT wX, const UINT wY, COrbData* pOrb);
	COrbAgentData* FindOrbAgentFor(COrbData* pOrb, CCoordStack &doorCoords);
	void		FixUnstableObstacles();
	void		FixUnstableTar();
	void		FixCorruptStaircase(const UINT wX, const UINT wY);
	void		FixCorruptStaircaseEdge(const UINT wMinX, const UINT wMaxX,
			const UINT wMinY, const UINT wMaxY,	const UINT wEvalX, const UINT wEvalY);
	const UINT*		GetTileImageForMonsterType(const UINT wType, const UINT wO,
			const UINT wAnimFrame) const;
	bool		IsObstacleWholeAt(const UINT wX, const UINT wY) const;

	bool		LoadRoomAtCoords(const DWORD dwRoomX, const DWORD dwRoomY);

	virtual void	OnBetweenEvents();
	virtual void	OnClick(const DWORD dwTagNo);
	virtual void	OnKeyDown(const DWORD dwTagNo, const SDL_KeyboardEvent &KeyboardEvent);
	virtual void	OnMouseDown(const DWORD dwTagNo, const SDL_MouseButtonEvent &Button);
	virtual void	OnMouseMotion(const DWORD dwTagNo, const SDL_MouseMotionEvent &Motion);
	virtual void	OnMouseUp(const DWORD dwTagNo, const SDL_MouseButtonEvent &Button);
	virtual void	OnMouseWheel(const SDL_MouseButtonEvent &Button);
	virtual void	OnSelectChange(const DWORD dwTagNo);
	virtual bool	OnQuit();

	virtual void	Paint(bool bUpdateRect=true);

   void     PasteRegion(const UINT wX, const UINT wY);
	void		PlotLastMonsterSegment(const UINT wTailX, const UINT wTailY,
			const UINT wDirection);
	PlotType		PlotMonsterSegment();
	void		PlotObjects();
	bool		PlotObjectAt(const UINT wX, const UINT wY, const UINT wObject,
			const UINT wO);
	void		PlotStaircase();
   void     ReadyToPasteRegion();
   void     Redo();
	void		RemoveBadObTiles(const UINT wCX, const UINT wCY);
	bool		RemoveMonster(CMonster *pMonster, bool bConfirmation=false);
	bool		RemoveObjectAt(const UINT wX, const UINT wY, const UINT wPlottedObject,
			bool &bSpecialTileRemoved, bool &bTarRemoved, bool &bStairsRemoved,
         const bool bSafetyOn=true);
   void     RemoveRoomChange();
	void		RoomChanging();
	bool		SaveRoom();
   bool     SaveRoomToDB();
   void     SetButtons(const bool bPaint=true);
   void     SetDestinationLevel(const UINT wX1, const UINT wY1,
         const UINT wX2, const UINT wY2, const bool bRequireValue=false);
   void     SetOrbAgentsForDoor(const UINT wX, const UINT wY);
	void		SetSignTextToCurrentRoom();
	void		SetState(const EditState eState);
	bool		SetUnspecifiedPlayerSettings(CDbPackedVars &Settings);
	void		ShowErrors();
   void     Undo();
	void		UpdatePlayerDataFromWidgets();

	CDbLevel *			pLevel;
	CDbRoom *			pRoom;

	CEditRoomWidget *	pRoomWidget;
	CTabbedMenuWidget * pTabbedMenu;
	UINT					wSelectedObject;	//object selected for placement
	UINT					wSelectedObjectSave;	//object selected, while
														//something else is being placed
	list<CDbRoom*>		UndoList, RedoList;	//stored room states
	EditState			eState;			//current state of room editing
	CMonster *			pLongMonster;	//long monster being placed
	COrbData *			pOrb;				//orb being modified
	bool					bShowErrors;	//visually display objects with errors
	bool					bAutoSave;		//always save the room when leaving it
	bool					bSafetyOn;		//safe editing
	bool					bRoomDirty;		//whether a save is needed
   int               nUndoSize;     //facilitates dirty checking

	UINT					wTestX, wTestY, wTestO;	//start player here when room testing

	UINT					wO;				//current placement orientation
	bool					bShowingTip;	//tool tip displaying

   //Room region duplication.
   UINT              wCopyX1, wCopyY1, wCopyX2, wCopyY2;
   bool              bAreaJustCopied, bReadyToPaste;

   DWORD             dwSavePlayerID, dwTestPlayerID;   //ID of temporary player for room testing
   set<CMonster*>    monstersNotDeleted;
};

#endif //...#ifndef EDITROOMSCREEN_H

// $Log: EditRoomScreen.h,v $
// Revision 1.31  2004/08/08 21:29:02  mrimer
// Fixed bug: room saves not always occurring when needed.
//
// Revision 1.30  2003/09/22 22:31:27  mrimer
// Fixed bug: pasting obstacles cause strange behavior.
//
// Revision 1.29  2003/08/09 00:38:53  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.28  2003/08/02 00:00:09  mrimer
// Changed SaveRoom() to return bool.
//
// Revision 1.27  2003/07/30 04:26:23  mrimer
// Changed EditScrollText() to return bool (true if text change is OKed).
//
// Revision 1.26  2003/07/29 03:56:01  mrimer
// Optimized error checking of long yellow doors.
//
// Revision 1.25  2003/07/21 22:10:21  mrimer
// Changed SaveRoomTODB() to return a bool.  Require making personal hold copy before modifying any rooms on map.  Fixed display bug: yellow doors on room edge.
//
// Revision 1.24  2003/07/19 21:29:22  mrimer
// Fixed bug: scroll text not being copied; user prompted multiple times to delete a serpent.
//
// Revision 1.23  2003/07/17 01:54:37  mrimer
// Fixed bug: trying to dynamic_cast classes with inaccessable base class: Made inheritance public. (Committed on behalf of Gerry JJ.)
//
// Revision 1.22  2003/07/16 07:41:19  mrimer
// Revised selecting stair destination UI.
//
// Revision 1.21  2003/07/15 00:37:16  mrimer
// Fixed room region copy errors and faulty modified hold copying errors.
//
// Revision 1.20  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.19  2003/07/11 17:29:03  mrimer
// Changed SetDestinationLevel() to receive the stair region as an explicit parameter.
// Implemented pasting the marked room region multiple times.
//
// Revision 1.18  2003/07/06 04:56:59  mrimer
// Added SaveRoomToDB() with more correct handling of room saving logic.
//
// Revision 1.17  2003/07/02 02:19:09  mrimer
// Changed SetRoom()'s return type to bool.
//
// Revision 1.16  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.15  2003/06/27 19:16:11  mrimer
// Enabled checkpoints during play-testing (using temporary player that gets deleted after playtesting).
//
// Revision 1.14  2003/06/26 17:44:17  mrimer
// Added SetDestinationLevel().
//
// Revision 1.13  2003/06/24 20:14:52  mrimer
// Refactored editing code in the CRoomWidget into new derived class CEditRoomWidget.
//
// Revision 1.12  2003/06/21 10:42:37  mrimer
// Added PasteRegion() and auxilliary methods and vars.
//
// Revision 1.11  2003/06/21 06:54:00  mrimer
// Completed door agent designation.  Revised some door traversal code and effects.
//
// Revision 1.10  2003/06/18 03:39:53  mrimer
// Added parameter to SetButtons().
//
// Revision 1.9  2003/06/17 18:21:35  mrimer
// Added Undo/Redo buttons.  Revised item placement/customization/deletion UI.
//
// Revision 1.8  2003/06/16 20:39:22  mrimer
// Added: mouse wheel changes orientation.
//
// Revision 1.7  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.6  2003/05/18 20:03:21  erikh2000
// Replaced "friend x" with "friend class x".
//
// Revision 1.5  2003/05/03 23:35:29  mrimer
// Added removal of a corrupted staircase.
// Now, when a room in a hold is edited, the player becomes the author.  Hold texts are revised.
//
// Revision 1.4  2002/12/22 02:25:43  mrimer
// Moved SetExitLevelID() to CScreen.
//
// Revision 1.3  2002/11/22 22:01:09  mrimer
// Added FixUnstableTar().
//
// Revision 1.2  2002/11/22 02:25:45  mrimer
// Added support for special handling of staircases, obstacles, playtesting, and displaying tool tips.
//
// Revision 1.1  2002/11/15 02:51:13  mrimer
// Initial check-in.
//
