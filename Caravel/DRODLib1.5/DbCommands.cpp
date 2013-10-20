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

#include "DbCommands.h"
#include "Assert.h"
#include "StretchyBuffer.h"
#include "SysTimer.h"

//
//CDbCommands public methods.
//

//******************************************************************************
void CDbCommands::Clear(void)
//Frees resources and resets members.
{
	//If object is frozen, then modifying list not allowed.
	if (this->bIsFrozen)
	{
		ASSERT(false);
		return;
	}

	COMMANDNODE *pSeek = this->Sentry.pNext, *pDelete;
	while (pSeek)
	{
		pDelete = pSeek;
		pSeek = pSeek->pNext;
		delete pDelete;
	}
	this->Sentry.pNext = NULL;
	this->pLast = &(this->Sentry);
	this->pCurrent = NULL;

	this->dwSize = 0L;
}

//******************************************************************************
void CDbCommands::Truncate(
//Truncate the command list to contain only the first X commands.
//
//Params:
	DWORD dwKeepCount)	//# of commands to keep.
{
	ASSERT(dwKeepCount < this->dwSize);

	//If object is frozen, then modifying list not allowed.
	if (this->bIsFrozen)
	{
		ASSERT(false);
		return;
	}

	//Skip over the commands I want to keep.
	COMMANDNODE *pSeek = this->Sentry.pNext, *pDelete;
	DWORD dwCommandCount = 1;
	while (dwCommandCount < dwKeepCount)
	{
		++dwCommandCount;
		pSeek = pSeek->pNext;
	}

	//Set last pointer to this command.
	this->pLast = pSeek;
	pSeek = pSeek->pNext;
	this->pLast->pNext = NULL;
	this->pCurrent = NULL;

	//Delete all the remaining commands.
	while (pSeek)
	{
		pDelete = pSeek;
		pSeek = pSeek->pNext;
		delete pDelete;
	}

	//List successfully truncated.
	this->dwSize = dwKeepCount;
}

//******************************************************************************
void CDbCommands::Add(
//Adds a new command to list.
//
//Params:
	int nCommand,					//(in)	One of the CMD_* constants.
	BYTE byt10msElapsedSinceLast)	//(in)	Time elapsed since last command in 10ms
									//		increments.  If 0 (default), the time 
									//		between this call and last call to Add()
									//		will be used.
{
	static DWORD dwTimeOfLastCall = 0L;

	//If object is frozen, then modifying list not allowed.
	if (this->bIsFrozen)
	{
		ASSERT(false);
		return;
	}
		
	//Get time elapsed since last, if not specified.
	BYTE bytElapsed;
	if (byt10msElapsedSinceLast == 0)
	{
		if (dwTimeOfLastCall == 0L)
			bytElapsed = 0; //First call to Add, so elapsed time does not apply.
		else
		{
			//Get # of 10-millisecond intervals between calls.
			DWORD dw10MsElapsedSinceLast = (GetTicks() - dwTimeOfLastCall) / 10;
			
			//Limit to a byte value.
			bytElapsed = (dw10MsElapsedSinceLast > 255) ? 
					255 : static_cast<BYTE>(dw10MsElapsedSinceLast);
		}
	}
	else
		bytElapsed = byt10msElapsedSinceLast;
	
	//Create new command and set members.
	COMMANDNODE *pNew = new COMMANDNODE;
	pNew->bytCommand = static_cast<BYTE>(nCommand);
	ASSERT(static_cast<int>(pNew->bytCommand)==nCommand); //Check for loss of original value.
	pNew->byt10msElapsedSinceLast = bytElapsed;
	pNew->pNext = NULL;

	//Add new command to end of list.
	this->pLast->pNext = pNew;
	this->pLast = pNew;
	++(this->dwSize);

	//Update time of last call to time of this call.
	dwTimeOfLastCall = GetTicks();	
}

//******************************************************************************
BYTE *CDbCommands::GetPackedBuffer(
//Gets a packed buffer containing all the demo commands.
//
//Params:
	DWORD &dwBufferSize)	//(out)	Size in bytes of the buffer.
//
//Returns:
//Pointer to packed buffer or NULL if no commands.
const
{
	CStretchyBuffer PackedBuf;

	//Each iteration packs one command into buffer.
	COMMANDNODE *pReadVar = this->Sentry.pNext;
	while (pReadVar)
	{
		PackedBuf += (BYTE) pReadVar->bytCommand;
		PackedBuf += (BYTE) pReadVar->byt10msElapsedSinceLast;
		
		pReadVar = pReadVar->pNext;
	}

	//Append end code to buffer.
	PackedBuf += (BYTE) 0;
	dwBufferSize = PackedBuf.Size();
	return PackedBuf.GetCopy();
}

//******************************************************************************
COMMANDNODE *CDbCommands::GetFirst(void)
//Returns:
//First command or NULL if there are none.
{
	this->pCurrent = this->Sentry.pNext;
	return this->pCurrent;
}

//******************************************************************************
COMMANDNODE *CDbCommands::GetNext(void)
//Returns:
//Next command or NULL if there are no more.
{
	if (this->pCurrent) this->pCurrent = this->pCurrent->pNext;
	return this->pCurrent;
}

//******************************************************************************
COMMANDNODE *CDbCommands::Get(
//Gets a command by zero-based index.  A subsequent call to GetNext() would
//then return the next command.
//
//Params:
	DWORD dwIndex)	//(in)	Index of command to get.
//
//Returns:
//Command.
{
	ASSERT(dwIndex < this->dwSize);
	DWORD dwCount = 0;
	
	this->pCurrent = this->Sentry.pNext;
	while (this->pCurrent)
	{
		if (dwCount == dwIndex) return this->pCurrent;
		++dwCount;
		this->pCurrent = this->pCurrent->pNext;
	}

	//Bad call with OOB index param.
	ASSERT(false);
	return NULL;
}

//******************************************************************************
COMMANDNODE *CDbCommands::GetConst(
//Gets a command by zero-based index, but does not modify the object.
//A subsequent call to GetNext() is not affected by this call.
//
//Params:
	DWORD dwIndex)	//(in)	Index of command to get.
//
//Returns:
//Command.
const
{
	ASSERT(dwIndex < this->dwSize);
	DWORD dwCount = 0;
	
	COMMANDNODE *pSeek = this->Sentry.pNext;
	while (pSeek)
	{
		if (dwCount == dwIndex) return pSeek;
		++dwCount;
		pSeek = pSeek->pNext;
	}

	//Bad call with OOB index param.
	ASSERT(false);
	return NULL;
}

//
//Private methods.
//

//******************************************************************************
void CDbCommands::UnpackBuffer(
//Unpacks command list from a buffer previously packed by GetPackedBuffer().
//
//Params:
	const BYTE *pBuf)	//(in)	Packed buffer to unpack into this object.
{
	const BYTE *pSeek = pBuf;
	while (pSeek[0] != 0)
	{
		Add(pSeek[0], pSeek[1]);
		pSeek += 2;
	}
}

// $Log: DbCommands.cpp,v $
// Revision 1.1  2003/02/25 00:01:26  erikh2000
// Initial check-in.
//
// Revision 1.12  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.11  2002/07/17 15:46:18  mrimer
// Added log macro to end of file.
//
