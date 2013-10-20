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
 *
 * ***** END LICENSE BLOCK ***** */

//DbDemos.h
//Declarations for CDbDemos and CDbDemo.
//Classes for accessing demo data from database.

#ifndef DBDEMOS_H
#define DBDEMOS_H

#include "CueEvents.h"
#include "DbMessageText.h"
#include "DbRefs.h"
#include "IDList.h"

//Demo stat constants.
const DWORD DS_FinalChecksum		= 0;	//(DWORD *) Checksum.
const DWORD DS_WasRoomConquered		= 1;	//(bool *) Was room conquered?
const DWORD DS_DidPlayerDie			= 2;	//(bool *) Did player die?
const DWORD DS_ProcessedTurnCount	= 3;	//(UINT *) Number of turns processed.
const DWORD DS_DidPlayerLeaveRoom	= 4;	//(bool *) Did player leave room?
const DWORD DS_MonsterCount			= 5;	//(UINT *) Number of monsters in room.
const DWORD DS_MonsterKills			= 6;	//(UINT *) Number of monsters killed.

//Defines section of turns that make up a "scene".
class CDemoScene
{
public:
	CDemoScene(const UINT wSetBeginTurnNo = (UINT)-1,
			const UINT wSetEndTurnNo = (UINT)-1)
			: wBeginTurnNo(wSetBeginTurnNo)
			, wEndTurnNo(wSetEndTurnNo)
	{}
	UINT wBeginTurnNo;
	UINT wEndTurnNo;
};

//*****************************************************************************
class CCurrentGame;
class CDbDemos;
class CDbDemo : public CDbBase
{
protected:
	friend CCurrentGame;
	friend CDbDemos;

	CDbDemo(void) {Clear();}

public:	
	void			Clear(void);
	CCurrentGame *	GetCurrentGame(void) const;
	void			GetNarrationText(WSTRING &wstrText) const;
	const WCHAR *	GetAuthorText(void) const;
	void			GetMostInterestingScene(const DWORD dwMaxTime, CDemoScene &Scene, 
			const UINT wEvalBeginTurnNo = (UINT)(-1), 
			const UINT wEvalEndTurnNo = (UINT)(-1));
	void			GetSceneFromBeginTurnNo(const DWORD dwMaxTime, CDemoScene &Scene,
			const UINT wEvalBeginTurnNo = (UINT)(-1));
	bool			Load(const DWORD dwDemoID);
	virtual MESSAGE_ID	SetProp(const PROPTYPE pType, char* const str,
			PrimaryKeyMaps &Maps, bool &bSaveRecord);
	bool			Test(CIDList &DemoStats) const;
	virtual bool	Update(void);

	DWORD			dwDemoID;
	DWORD			dwSavedGameID;
	CDbMessageText	DescriptionText;
	UINT			wBeginTurnNo;
	UINT			wEndTurnNo;
	UINT			wShowSequenceNo;
	DWORD			dwNextDemoID;
	DWORD			dwChecksum;
	bool			bIsHidden;

private:
	void 	GetNarrationText_English(const CIDList &DemoStats, WSTRING &wstrText) const;
	int		ScoreGameCommand(const CCurrentGame *pGame, const CCueEvents &CueEvents,
			const bool bPrevVisited) const;
	bool	UpdateNew(void);
	bool	UpdateExisting(void);
};

//*****************************************************************************
class CDb;
class CDbRoom;
class CDbDemos : public CDbBase
{
protected:
	friend CDb;
	friend CDbRoom;
	friend CCurrentGame;

	CDbDemos::CDbDemos(void) 
		: pCurrentRow(NULL)
		, bIsMembershipLoaded(false), bFilterByShow(false)
		, dwFilterByRoomID(0L), dwFilterByPlayerID(0L)
	{}

public:
	void		Delete(const DWORD dwDemoID);
	string		ExportXML(const DWORD dwDemoID, CDbRefs &dbRefs);
	void		FilterByPlayer(const DWORD dwSetFilterByPlayerID);
	void		FilterByRoom(const DWORD dwSetFilterByRoomID);
	void		FilterByShow(void);
	DWORD		FindByLatest(void);
	CDbDemo *	GetByID(const DWORD dwDemoID) const;
	CDbDemo *	GetFirst(void);
	void		GetIDs(CIDList &IDs);
	CDbDemo *   GetNew(void);
	CDbDemo *   GetNext(void);
	UINT		GetNextSequenceNo(void) const;

	void		RemoveShowSequenceNo(const UINT wSequenceNo);
	bool		ShowSequenceNoOccupied(const UINT wSequenceNo);
	virtual bool	Update(void) {return false;}

private:
	void		LoadMembership(void);
	void		LoadMembership_All(void);
	void		LoadMembership_ByPlayer(const DWORD dwByPlayerID);
	void		LoadMembership_ByRoom(const DWORD dwByRoomID);
	void		LoadMembership_ByShow(void);

	DWORD		dwFilterByPlayerID, dwFilterByRoomID;
	bool		bFilterByShow;
	bool		bIsMembershipLoaded;
	CIDList		MembershipIDs;
	IDNODE *	pCurrentRow;
};

#endif //...#ifndef DBDEMOS_H

// $Log: DbDemos.h,v $
// Revision 1.1  2003/02/25 00:01:28  erikh2000
// Initial check-in.
//
// Revision 1.27  2003/02/16 20:29:32  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.26  2002/12/22 01:39:38  mrimer
// Added XML import/export support.
// Removed BETA conditional compiles.
//
// Revision 1.25  2002/11/22 02:05:12  mrimer
// Added GetNew().
//
// Revision 1.24  2002/11/14 19:04:23  mrimer
// Added methods to set and reset a demo's show sequence number.
//
// Revision 1.23  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.22  2002/10/17 16:48:02  mrimer
// Added DS_MonsterKills.
//
// Revision 1.21  2002/10/01 22:37:04  erikh2000
// Wrote method to get a current game from a demo.
// Wrote method to get a scene of specified maximum length.
//
// Revision 1.20  2002/08/30 00:22:44  erikh2000
// Finished demo import code.
//
// Revision 1.19  2002/08/29 17:28:12  mrimer
// Added tentative implementation of CDbDemo::LoadFromFile() and CDbDemos::GetFromFile().
//
// Revision 1.18  2002/08/29 09:19:11  erikh2000
// Added method to save a demo to a file.
//
// Revision 1.17  2002/07/17 20:07:13  erikh2000
// Added method to find more interesting range of a demo.
// Revised #includes.
//
// Revision 1.16  2002/07/05 17:59:34  mrimer
// Minor fixes (includes, etc.)
//
// Revision 1.15  2002/06/21 03:29:33  erikh2000
// Renamed GetAllocNarrationText_English() for consistency.
//
// Revision 1.14  2002/06/20 04:05:58  erikh2000
// CDbDemos can now filter to only include demos in the show sequence.
//
// Revision 1.13  2002/06/09 06:13:27  erikh2000
// Added CDbDemo::GetAuthorText().
// Added IsHidden field and hidden field filtering.
//
// Revision 1.12  2002/05/21 19:05:30  erikh2000
// Added narrative description generator for demos.
//
// Revision 1.11  2002/05/15 01:20:08  erikh2000
// Added CDbDemos method that finds latest recorded demo.
//
// Revision 1.10  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.9  2002/02/26 11:46:34  erikh2000
// Added CDbDemo::UpdateNew() and CDbDemo::UpdateExisting().
//
// Revision 1.8  2002/02/23 04:58:08  erikh2000
// Added DS_MonsterCount demo stat.
//
// Revision 1.7  2002/02/15 02:45:38  erikh2000
// Added DS_DidPlayerLeaveRoom constant.
//
// Revision 1.6  2001/12/16 02:08:11  erikh2000
// Added filter by room functionality to CDbDemos.
//
// Revision 1.5  2001/12/08 01:40:51  erikh2000
// Added demo stat constants.
// Added GetFirst() and GetNext() methods.
//
// Revision 1.4  2001/11/23 01:21:26  erikh2000
// Oops, forgot to add an #include needed for compile.
//
// Revision 1.3  2001/11/23 01:08:10  erikh2000
// Added CDbDemo::Test() stub.
//
// Revision 1.2  2001/11/20 00:53:39  erikh2000
// Added field members to CDbDemo.
// Added CDbDemo::Update() stub.
// Made constructor protected.
//
// Revision 1.1.1.1  2001/10/01 22:20:08  erikh2000
// Initial check-in.
//
