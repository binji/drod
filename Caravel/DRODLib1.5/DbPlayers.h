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
 * Portions created by the Initial Developer are Copyright (C) 
 * 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//DbPlayers.h
//Declarations for CDbPlayers and CDbPlayer.
//Classes for accessing player data from database.

#ifndef DBPLAYERS_H
#define DBPLAYERS_H

#include "DbDate.h"
#include "DbMessageText.h"
#include "DbPackedVars.h"
#include "DbRefs.h"
#include "IDList.h"

//*****************************************************************************
class CDb;
class CDbPlayer;
class CDbPlayers : public CDbBase
{
public:
	void		Delete(const DWORD dwPlayerID, const bool bRetainRef=true);
	string	ExportXML(const DWORD dwPlayerID, CDbRefs &dbRefs, const bool bRef=false);
	void		FilterByLocal(void);
	static DWORD		FindByName(const WCHAR *pwczName);
	static CDbPlayer *	GetByID(const DWORD dwPlayerID);
	CDbPlayer *	GetFirst(void);
	CDbPlayer *	GetNext(void);
	CDbPlayer * GetNew(void);
	virtual bool	Update(void) {return false;}

protected:
	friend CDb;

	CDbPlayers::CDbPlayers(void) 
		: pCurrentRow(NULL)
		, bIsMembershipLoaded(false)
		, bFilterByLocal(false)
	{ }

private:
	void		LoadMembership(void);

	CIDList		MembershipIDs;
	bool		bIsMembershipLoaded;
	IDNODE *	pCurrentRow;
	bool		bFilterByLocal;
};

//*****************************************************************************
class CDbPlayer : public CDbBase
{
protected:
	friend CDbPlayers;
	CDbPlayer(void)
	{
		Clear();
	}

public:
	~CDbPlayer(void)
	{
		//No automatic flushing for texts.
		this->NameText.Cancel();
		this->EMailText.Cancel();
	}

	bool			Load(DWORD dwLoadPlayerID);
	virtual MESSAGE_ID	SetProp(const PROPTYPE pType, char* const str,
			PrimaryKeyMaps &Maps, bool &bSaveRecord);
	virtual bool	Update(void);

	DWORD			dwPlayerID;
	bool			bIsLocal;
	CDbMessageText	NameText;
	CDbMessageText	EMailText;
	CDbPackedVars	Settings;

private:
	DWORD			GetLocalID(void);
	bool			UpdateExisting(void);
	bool			UpdateNew(void);
	void			Clear(void);

	CDbMessageText	OriginalNameText;	//GUID fields
	CDbDate			Created, LastUpdated;
};

#endif //...#ifndef DBPLAYERS_H

// $Log: DbPlayers.h,v $
// Revision 1.1  2003/02/25 00:01:30  erikh2000
// Initial check-in.
//
// Revision 1.6  2002/12/22 01:33:48  mrimer
// Added XML import/export support.  Added Created and LastUpdated fields.
//
// Revision 1.5  2002/11/22 21:58:17  mrimer
// Made Delete() not const.
//
// Revision 1.4  2002/11/14 19:10:21  mrimer
// Added Delete().
// Moved member initialization into constructor initialization list.
// Made some parameters const.
//
// Revision 1.3  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.2  2002/08/30 00:23:43  erikh2000
// New players can be added to database now.
// Wrote method to find a player by name.
//
// Revision 1.1  2002/06/09 05:58:56  erikh2000
// Initial check-in.
//
