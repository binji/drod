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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//MonsterMessage.cpp

#include "MonsterMessage.h"
#include "Db.h"

//*****************************************************************************
CMonsterMessage::CMonsterMessage(MONSTER_MESSAGE_TYPE eSetType,
		MESSAGE_ID eSetMessageID, CMonster *pSetSender)
   : CAttachableObject()
{
	this->eType = eSetType;
	this->eMessageID = eSetMessageID;
	this->pSender = pSetSender;
}

//*****************************************************************************
CMonsterMessage::CMonsterMessage(const CMonsterMessage &Src) 
   : CAttachableObject()
{
	this->eType = Src.eType;
	this->eMessageID = Src.eMessageID;
	this->pSender = Src.pSender;
}

//*****************************************************************************
WCHAR * CMonsterMessage::GetMessageText(void)
{
	ASSERT(this->eMessageID > (MESSAGE_ID)0);
	return g_pTheDB->GetMessageText(this->eMessageID);
}

//$Log: MonsterMessage.cpp,v $
//Revision 1.6  2003/06/19 01:53:45  mrimer
//Linux port fixes (committed on behalf of Gerry JJ).
//
//Revision 1.5  2003/05/13 01:10:25  erikh2000
//Revised includes to reduce dependencies.
//
//Revision 1.4  2003/05/09 02:43:20  mrimer
//Tweaked some ASSERTS to compile as __assume's for Release build.
//
//Revision 1.3  2003/05/08 23:22:57  erikh2000
//Revised to support v1.6 format.
//
//Revision 1.2  2003/05/08 22:01:08  mrimer
//Replaced local CDb instances with a pointer to global instance.
//
//Revision 1.1  2003/01/08 00:38:48  mrimer
//Initial check-in (moved method bodies from .h file).
//
