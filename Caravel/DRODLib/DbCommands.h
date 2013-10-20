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

//DbCommands.h
//Declarations for CDbCommands.
//Class for storing game commands.

#ifndef DBCOMMANDS_H
#define DBCOMMANDS_H

#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>

#include <mk4.h>

typedef struct tagCommandNode COMMANDNODE;
typedef struct tagCommandNode
{
	BYTE			bytCommand;
	BYTE			byt10msElapsedSinceLast;
	COMMANDNODE *	pNext;
} COMMANDNODE;

//******************************************************************************
class CDbCommands
{
public:
	CDbCommands()
	{
		this->bIsFrozen = false;
		this->Sentry.pNext = NULL;
		this->pLast = &(this->Sentry);
		Clear();
	}

	const BYTE * operator = (const BYTE *pBuf) {UnpackBuffer(pBuf); return pBuf;}
   CDbCommands & operator = (const c4_BytesRef &Buf)
 	{
 		c4_Bytes Bytes = (c4_Bytes) Buf;
		UnpackBuffer(Bytes.Contents());
		return *this;
	}
	c4_BytesRef & operator = (c4_BytesRef &Buf)
	{
		c4_Bytes Bytes = (c4_Bytes) Buf;
		UnpackBuffer(Bytes.Contents());
		return Buf;
	}

	void			Add(int nCommand, BYTE byt10msElapsedSinceLast = 0);
	void			Clear(void);
	void			Freeze(void) {ASSERT(!this->bIsFrozen); this->bIsFrozen=true;}
	COMMANDNODE *	Get(DWORD dwIndex);
	COMMANDNODE *	GetConst(DWORD dwIndex) const;
	COMMANDNODE *	GetFirst(void);
	COMMANDNODE *	GetNext(void);
	BYTE *			GetPackedBuffer(DWORD &dwBufferSize) const;
	DWORD			GetSize(void) const {return this->dwSize;}
	bool			IsFrozen(void) const {return this->bIsFrozen;}
	void			Truncate(DWORD dwKeepCount);
	void			Unfreeze(void) {ASSERT(this->bIsFrozen); this->bIsFrozen=false;};

private:
	void			UnpackBuffer(const BYTE *pBuf);

	COMMANDNODE 	Sentry;
	COMMANDNODE *	pLast;
	COMMANDNODE *	pCurrent;
	DWORD			dwSize;
	bool			bIsFrozen;

	PREVENT_DEFAULT_COPY(CDbCommands);
};

#endif //...#ifndef DBCOMMANDS_H

// $Log: DbCommands.h,v $
// Revision 1.12  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.10  2003/05/22 23:39:00  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.9  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.8  2002/07/17 15:46:18  mrimer
// Added log macro to end of file.
//

