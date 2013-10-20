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

//MonsterMessage.h
//Declarations for CMonsterMessage. 
//Class for holding monster message data.

#ifndef MONSTERMESSAGE_H
#define MONSTERMESSAGE_H

#include "AttachableObject.h"
#include "Types.h"
#include "Wchar.h"

//*****************************************************************************
enum MONSTER_MESSAGE_TYPE
{
	MMT_OK,
	MMT_YESNO
};

//*****************************************************************************
class CMonster;
class CMonsterMessage : public CAttachableObject
{
public:
	CMonsterMessage(const MONSTER_MESSAGE_TYPE eSetType, const DWORD dwSetMessageID, 
			CMonster *pSetSender);
	CMonsterMessage(const CMonsterMessage &Src);
	
	WCHAR * GetMessageText(void);

	CMonster *				pSender;			//Monster that sent the message.
	DWORD					dwMessageID;		//Message sent.
	MONSTER_MESSAGE_TYPE	eType;				//Type of message.
};

#endif //...#ifndef MONSTERMESSAGE_H

//$Log: MonsterMessage.h,v $
//Revision 1.1  2003/02/25 00:01:37  erikh2000
//Initial check-in.
//
//Revision 1.10  2003/02/17 00:48:12  erikh2000
//Remove L" string literals.
//
//Revision 1.9  2003/01/08 00:38:08  mrimer
//Moved method bodies into MonsterMessage.cpp.
//
//Revision 1.8  2002/07/22 00:54:13  erikh2000
//Fixed problem caused using memset to zero object members.
//Replaced default constructor with a member-setting constructor.
//
//Revision 1.7  2002/07/20 23:02:31  erikh2000
//Changed class so that it didn't need to be derived from CDbBase.
//
//Revision 1.6  2002/07/19 20:23:16  mrimer
//Added CAttachableObject references.
//
//Revision 1.5  2002/07/05 17:59:05  mrimer
//Added destructor.
//
//Revision 1.4  2002/04/28 23:40:53  erikh2000
//Revised #includes.
//
//Revision 1.3  2002/03/05 01:54:10  erikh2000
//Added 2002 copyright notice to top of file.
//
//Revision 1.2  2001/12/16 03:09:33  erikh2000
//Fix CVS log macro.
//
