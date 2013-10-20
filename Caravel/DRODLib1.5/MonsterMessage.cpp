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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//MonsterMessage.cpp

#include "MonsterMessage.h"
#include "Db.h"

//*****************************************************************************
CMonsterMessage::CMonsterMessage(MONSTER_MESSAGE_TYPE eSetType,
		DWORD dwSetMessageID, CMonster *pSetSender)
{
	this->eType = eSetType;
	this->dwMessageID = dwSetMessageID;
	this->pSender = pSetSender;
}

//*****************************************************************************
CMonsterMessage::CMonsterMessage(const CMonsterMessage &Src) 
{
	this->eType = Src.eType;
	this->dwMessageID = Src.dwMessageID;
	this->pSender = Src.pSender;
}
	
//*****************************************************************************
WCHAR * CMonsterMessage::GetMessageText(void)
{
	ASSERT(this->dwMessageID);
	CDb db;
	return db.GetMessageText(this->dwMessageID);
}

//$Log: MonsterMessage.cpp,v $
//Revision 1.1  2003/02/25 00:01:36  erikh2000
//Initial check-in.
//
//Revision 1.1  2003/01/08 00:38:48  mrimer
//Initial check-in (moved method bodies from .h file).
//
