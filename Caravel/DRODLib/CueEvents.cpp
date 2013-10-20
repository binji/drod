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

#include "CueEvents.h"
#include "MonsterMessage.h"
#include <BackEndLib/AttachableObject.h>

//
//Public methods.
//

//***************************************************************************************
void CCueEvents::Clear(void)
//Frees resources and resets members.
{
	//Delete private data and nodes to track it.
	this->pNextPrivateData = NULL;
	for (UINT wCID = 0; wCID < CUEEVENT_COUNT; ++wCID)
	{
		CID_PRIVDATA_NODE *pSeek = this->parrCIDPrivateData[wCID], *pDelete;
		while (pSeek)
		{
			pDelete = pSeek;
			pSeek = pSeek->pNext;
			ASSERT(pDelete->pvPrivateData);
			if (pDelete->bIsAttached)
				delete pDelete->pvPrivateData;
			delete pDelete;
		}
	}

	//Zero all the members.
	Zero();
}

//***************************************************************************************
UINT CCueEvents::GetOccurrenceCount(
//Get number of times a specified cue event has occurred.
//
//Params:
	CUEEVENT_ID eCID)	//(in)	Cue event for which to count occurrences.
//
//Returns:
//The count.  Will be 0 if cue event has not occurred.
const
{
	UINT wCount = 0;
	CID_PRIVDATA_NODE *pNode = this->parrCIDPrivateData[eCID];
	while (pNode)
	{
		++wCount;
		pNode = pNode->pNext;
	}

	return wCount;
}

//***************************************************************************************
void CCueEvents::Add(
//Sets a cue event to true and associates a private data pointer with that cue event.
//
//Params:
	CUEEVENT_ID eCID,		//(in)	Cue event ID that will be set to true.
	CAttachableObject *pvPrivateData,	//(in)	Private data to associate with cue event.  If NULL
							//		(default) then no private data will be associated.
							//		Never pass an array allocated with "new []"--encapsulate
							//		in a class or struct before passing.
	bool bIsAttached)		//(in)	If set to true then the private data will be deleted
							//		by CCueEvents when it destructs.  If false (default) 
							//		then data should get deleted elsewhere, but not by
							//		caller.  See comments about private data validity 
							//		guarantee in CueEvents.h.
{
	ASSERT(IS_VALID_CID(eCID));
	ASSERT(!(pvPrivateData == NULL && bIsAttached));

	//Set CID flag.
	bool bWasSet = this->barrIsCIDSet[eCID];
	if (!bWasSet)
	{
		this->barrIsCIDSet[eCID] = true;
		++(this->wEventCount);
	}

	//Add private data.
	if (pvPrivateData)
	{
		CID_PRIVDATA_NODE *pNew = new CID_PRIVDATA_NODE;
		pNew->bIsAttached = bIsAttached;
		pNew->pvPrivateData = pvPrivateData;

		//Set next node pointer of new node to point to current first node.  First node may
		//not be present, but that will correctly set next node pointer to NULL.
		pNew->pNext = this->parrCIDPrivateData[eCID];

		//Set first node to new node.  This inserts new node at beginning of list.
		this->parrCIDPrivateData[eCID] = pNew;
	}
}

//***************************************************************************************
CAttachableObject* CCueEvents::GetFirstPrivateData(
//Gets first private data pointer associated with a cue event, and sets object state so 
//that a call to GetNextPrivateData() will return the second private data, if available.
//
//Params:
	CUEEVENT_ID eCID)	//(in)	Which event for which to retrieve private data.
//
//Returns:
//First private data pointer or NULL if there is no private data associated with cue event.
{
	CID_PRIVDATA_NODE *pFirst = this->parrCIDPrivateData[eCID];
	if (pFirst)
	{
		ASSERT(this->barrIsCIDSet[eCID]); //If I found private data then the event should be set.
		ASSERT(pFirst->pvPrivateData);

		this->pNextPrivateData = pFirst->pNext;
		return pFirst->pvPrivateData;
	}
	else
	{
		this->pNextPrivateData = NULL;
		return NULL;
	}
}

//***************************************************************************************
CAttachableObject* CCueEvents::GetNextPrivateData(void)
//Gets next private data pointer after call made to GetFirstPrivateData().  May be called
//multiple times to retrieve a succession of private data pointers.
//
//Returns:
//Private data pointer or NULL if there are no more.
{
	CID_PRIVDATA_NODE *pCurrent = this->pNextPrivateData;
	if (!pCurrent) return NULL; //No more private data left.

	//Advance get-next pointer to next private data node and return 
	//this private data pointer.
	this->pNextPrivateData = pCurrent->pNext;
	ASSERT(pCurrent->pvPrivateData);
	return pCurrent->pvPrivateData;
}

//***************************************************************************************
bool CCueEvents::HasAnyOccurred(
//Checks to see if at least one cue event from a list has been set.
//
//Params:
	UINT wCIDArrayCount,			//# of elements in array.
	const CUEEVENT_ID *peCIDArray)	//IDs to check for.
//
//Returns:
//True if any of the IDs were found, false if not.
const
{
	//Each iteration checks for presence of one CID from 
	for (UINT wCIDI = 0; wCIDI < wCIDArrayCount; ++wCIDI)
	{
		ASSERT(IS_VALID_CID(peCIDArray[wCIDI]));
		if (this->barrIsCIDSet[peCIDArray[wCIDI]]) 
			//Found one from the list--any remaining IDs are ignored.
			return true;
	}

	//Didn't find any.
	return false;
}

//***************************************************************************************
bool CCueEvents::HasOccurredWith(
//Has an event occurred with a matching private data pointer?
//
//Params:
	CUEEVENT_ID eCID,					//(in)	Check for this CID.
	const CAttachableObject *pvPrivateData)	//(in)	Check for this private data.
//
//Returns:
//True if a match was found, false if not.
const
{
	ASSERT(IS_VALID_CID(eCID));
	ASSERT(pvPrivateData);

	CID_PRIVDATA_NODE *pSeek = this->parrCIDPrivateData[eCID];
	if (!pSeek) return false; //The event didn't occur at all.

	//Each iteration checks on private data for a matching pointer.
	ASSERT(this->barrIsCIDSet[eCID]);
	while (pSeek)
	{
		if (pSeek->pvPrivateData == pvPrivateData) return true; //Found it.
		pSeek = pSeek->pNext;
	}

	//Didn't find it.
	return false;
}

//
//Private methods.
//

//***************************************************************************************
void CCueEvents::Zero(void)
//Zero all the members.
{
	ASSERT(sizeof(this->barrIsCIDSet) == 
			sizeof(this->barrIsCIDSet[0]) * CUEEVENT_COUNT);
	ASSERT(sizeof(this->parrCIDPrivateData) == 
			sizeof(this->parrCIDPrivateData[0]) * CUEEVENT_COUNT);

	this->pNextPrivateData = NULL;
	memset(this->barrIsCIDSet, 0, sizeof(this->barrIsCIDSet));
	memset(this->parrCIDPrivateData, 0, sizeof(this->parrCIDPrivateData));
	this->wEventCount = 0;
}

//$Log: CueEvents.cpp,v $
//Revision 1.14  2003/05/23 21:30:36  mrimer
//Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
//Revision 1.13  2003/05/22 23:39:01  mrimer
//Modified to use new BackEndLib.
//
//Revision 1.12  2003/04/06 03:57:00  schik
//Ported to SGI.
//All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
//Revision 1.11  2002/10/16 01:27:43  erikh2000
//Removed CID_StepOffScroll.
//Put CIDs in an enum and changed params for CCueEvents to match.
//
//Revision 1.10  2002/07/22 00:51:57  erikh2000
//Removed dangerous object-zeroing memset call.
//
//Revision 1.9  2002/07/21 00:18:06  erikh2000
//Added contributor credits for mrimer.
//
//Revision 1.8  2002/07/19 20:24:35  mrimer
//Modified CCueEvents to work with CAttachableObject.
//
//Revision 1.7  2002/07/17 02:11:35  erikh2000
//Added new method to get number of occurrences of an event.
//
//Revision 1.6  2002/07/05 18:01:34  mrimer
//Made fix for deleting CMonsterMessage objects in Clear().
//
//Revision 1.5  2002/06/21 04:30:22  mrimer
//Revised includes.
//
//Revision 1.4  2002/04/28 23:40:53  erikh2000
//Revised #includes.
//
//Revision 1.3  2002/03/05 01:54:10  erikh2000
//Added 2002 copyright notice to top of file.
//
//Revision 1.2  2001/11/25 03:26:38  erikh2000
//Fix CVS log keyword.
//

