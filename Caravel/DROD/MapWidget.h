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
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <FrontEndLib/FocusWidget.h>
#include "../DRODLib/CurrentGame.h"

//******************************************************************************
class CMapWidget : public CFocusWidget
{		
	public:
		CMapWidget(DWORD dwSetTagNo, int nSetX, int nSetY, UINT wSetW, 
				UINT wSetH, const CCurrentGame *pSetCurrentGame);

		void				ClearMap();
		void				GetSelectedRoomXY(DWORD &dwRoomX, DWORD &dwRoomY) const;
		bool				IsDeletingRoom() const {return this->bDeletingRoom;}
		bool				LoadFromCurrentGame(const CCurrentGame *pSetCurrentGame);
		bool				LoadFromLevel(CDbLevel *pLevel);
		virtual void	Paint(bool bUpdateRect = true);
		virtual void	PaintClipped(const int nX, const int nY, const UINT wW,
				const UINT wH, const bool bUpdateRect = true);
		bool				PasteRoom();
		virtual void	Resize(const UINT wSetW, const UINT wSetH);
		void				SelectRoom(const DWORD dwRoomX, const DWORD dwRoomY);
		void				SelectRoomAtCoords(const int nX, const int nY);
		void				SetDarkenedRooms(CIDList &RoomIDs);
		void				SetDestSurface(SDL_Surface *pNewSurface);
		void				UpdateFromCurrentGame();
		void				UpdateFromCurrentLevel();

		bool				bVacantRoom;	//when room at selected coords doesn't exist

	protected:
		virtual ~CMapWidget();

		virtual bool   Load();
		virtual void   Unload();
		virtual void   HandleMouseDown(const SDL_MouseButtonEvent &Button);
		virtual void   HandleKeyDown(const SDL_KeyboardEvent &Key);
    
	private:
		void				DrawMapSurfaceFromRoom(const CDbRoom *pRoom);
		inline SURFACECOLOR	GetMapColorFromTile(const UINT wOpaqueTile,
				const UINT wTransparentTile, const bool bRoomConquered,
				const bool bLevelComplete, const bool bDarkened,
                const bool bPendingExit);
		inline Uint8 *		GetRoomStart(const DWORD dwRoomX, const DWORD dwRoomY);
		bool				LoadMapSurface();
		void				InitMapColors();
		bool				IsAdjacentToValidRoom(const DWORD dwRoomX, const DWORD dwRoomY);
		inline void			LockMapSurface();
		void				RemoveBlueDoorsFromMapSurface();
		bool				SelectRoomIfValid(const DWORD dwRoomX, const DWORD dwRoomY);
		inline void			UnlockMapSurface();
		void				UpdateMapSurface();

		DWORD					dwLevelID;
		CIDList					ExploredRooms;
		CIDList					ConqueredRooms;
		CIDList					DarkenedRooms;
		bool					bIsLevelComplete;

		const CCurrentGame *	pCurrentGame;	//to show map of a game in progress
		CDbLevel *				pLevel;			//to show map of entire level
		SDL_Surface *			pMapSurface;

		DWORD					dwSelectedRoomX, dwSelectedRoomY;
		DWORD					dwLeftRoomX, dwTopRoomY;
		DWORD					dwRightRoomX, dwBottomRoomY;

		UINT					wLastSrcX, wLastSrcY;
		UINT					wBorderW, wBorderH;
		bool					bScrollHorizontal, bScrollVertical;

		bool					bEditing;	//whether level editor is being used
		bool					bCopyingRoom;	//room is being copy-and-pasted
													//(otherwise, it just is being cut)
		bool					bDeletingRoom;	//room is get pasted over
		CDbRoom *			pRoom;		//room being cut or copied
};

#endif //#ifndef MAPWIDGET_H

// $Log: MapWidget.h,v $
// Revision 1.28  2003/07/12 20:32:03  mrimer
// Made virtual widget methods explicit virtual.
// Updated widget load/unload methods.
//
// Revision 1.27  2003/06/22 05:58:28  mrimer
// Moved body of UpdateFromCurrentLevel() to .cpp.
//
// Revision 1.26  2003/06/18 21:21:37  schik
// Added visual cue when room is in conquered state.
//
// Revision 1.25  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.24  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.23  2003/05/03 23:31:29  mrimer
// Added ClearMap().
//
// Revision 1.22  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.21  2002/11/22 22:02:42  mrimer
// Implemented cutting, copying, and pasting rooms.
// Added UpdateFromCurrentLevel().
//
// Revision 1.20  2002/11/15 02:38:00  mrimer
// Added support for use in level editor.
//
// Revision 1.19  2002/09/03 21:30:33  erikh2000
// Added ASSERT to fire if widget is scrolled, since widget doesn't have code to support scrolling offsets yet.
//
// Revision 1.18  2002/07/22 01:00:29  erikh2000
// Made destructor declaration virtual just so that the virtual status is more obvious.
//
// Revision 1.17  2002/07/21 01:41:59  erikh2000
// Removed AcceptsTextEntry() override.  (Widget handles keypresses but doesn't convert them to text.)
//
// Revision 1.16  2002/07/19 20:33:27  mrimer
// Added AcceptsTextEntry() and revised HandleKeyDown() to have no return type.
//
// Revision 1.15  2002/07/05 10:31:37  erikh2000
// Changed widget to respond to new event-handling methods.
//
// Revision 1.14  2002/07/03 21:59:57  mrimer
// Revised prototypes (added consts).
//
// Revision 1.13  2002/06/21 04:57:09  mrimer
// Revised includes.
//
// Revision 1.12  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.11  2002/06/14 00:56:22  erikh2000
// Wrote SetDestSurface() override that recalcs cached surface colors on each call.
//
// Revision 1.10  2002/06/07 17:46:42  mrimer
// Fixed mouse button handling.
//
// Revision 1.9  2002/06/05 23:57:55  mrimer
// Added SelectRoomIfValid().
//
// Revision 1.8  2002/06/05 03:09:07  mrimer
// Added focus graphic.  Added widget focusability and keyboard control.
//
// Revision 1.7  2002/05/10 22:38:06  erikh2000
// Added ability to draw some rooms in a darkened state.
//
// Revision 1.6  2002/04/29 00:14:40  erikh2000
// Added methods used for selecting rooms by clicking on the map widget.
//
// Revision 1.5  2002/04/25 09:33:51  erikh2000
// Added #includes to fix compile errors.
//
// Revision 1.4  2002/04/14 00:33:01  erikh2000
// Added PaintClipped() method that fires an assertian.
//
// Revision 1.3  2002/04/09 10:05:39  erikh2000
// Fixed revision log macro.
//
