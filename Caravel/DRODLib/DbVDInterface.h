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

#ifndef DBVDINTERFACE_H
#define DBVDINTERFACE_H

#ifdef WIN32 
#	pragma warning(disable:4786) 
#endif 

#include "DbBase.h"
#include "DbMessageText.h"
#include "DbRefs.h"
#include <BackEndLib/IDList.h>

#ifdef WIN32 
#	pragma warning(disable:4786) 
#endif 

//******************************************************************************************
class CDb;

template<typename VDElement>
class CDbVDInterface : public CDbBase
{
protected:
	friend class CDb;
	CDbVDInterface(char* const viewDefStr, const c4_IntProp viewDefPrimaryKeyProp)
		: bIsMembershipLoaded(false)
		, pCurrentRow(NULL)
		, viewDefStr(viewDefStr)
		, viewDefPrimaryKeyProp(viewDefPrimaryKeyProp)
	{}

public:
	~CDbVDInterface() {}

	virtual void		Delete(const DWORD dwVDID);
	virtual string		ExportXML(const DWORD dwVDID, CDbRefs &dbRefs, const bool bRef=false)=0;
	static VDElement *  GetByID(const DWORD dwVDID);
	void                GetIDs(CIDList &IDs);
	VDElement *         GetFirst();
	VDElement *         GetNext();
	virtual VDElement * GetNew();
    DWORD               GetSize(void);

    void ResetMembership() {this->bIsMembershipLoaded = false;}

	virtual bool        Update() {return false;}

protected:
	virtual void		LoadMembership();

	bool		bIsMembershipLoaded;
	CIDList		MembershipIDs;
	IDNODE *	pCurrentRow;

	char *		viewDefStr;
	c4_IntProp	viewDefPrimaryKeyProp;
};

//
// Implementation of template methods
//

//*****************************************************************************
template<typename VDElement>
void CDbVDInterface<VDElement>::Delete(
//Get an element by its Primary Key ID.
//
//Params:
//NOTE: Unused param names commented out to suppress warning
	const DWORD /*dwVDID*/)	//(in)
//
//Returns:
//Deletes row in DB.
{
	//!!todo (here?)
}

//*****************************************************************************
template<typename VDElement>
VDElement* CDbVDInterface<VDElement>::GetByID(
//Get an element by its Primary Key ID.
//
//Params:
	const DWORD dwVDID)	//(in)
//
//Returns:
//Pointer to loaded element which caller must delete, or NULL if no matching element 
//was found.
{
	VDElement *pVDElement = new VDElement();
	if (pVDElement)
	{
		if (!pVDElement->Load(dwVDID))
		{
			delete pVDElement;
			pVDElement=NULL;
		}
	}
	return pVDElement;
}

//*****************************************************************************
template<typename VDElement>
void CDbVDInterface<VDElement>::GetIDs(
//Gets IDs in membership.
//
//Params:
	CIDList &IDs)	//(out) Receives copy of object's membership list.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(this->bIsMembershipLoaded);

	IDs = this->MembershipIDs;
}

//*****************************************************************************
template<typename VDElement> DWORD CDbVDInterface<VDElement>::GetSize(void)
//Returns count of elements in membership.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(this->bIsMembershipLoaded);

	return this->MembershipIDs.GetSize();
}

//*****************************************************************************
template<typename VDElement>
VDElement* CDbVDInterface<VDElement>::GetFirst()
//Gets first element.  A subsequent call to GetNext() will retrieve the second element.
//
//Returns:
//Pointer to loaded Element which caller must delete, or NULL if no matching element
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//Set current row to first.
	this->pCurrentRow = this->MembershipIDs.Get(0);
	
	//If there are no rows, then just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load element.
	VDElement *pVDElement = GetByID(this->pCurrentRow->dwID);
	
	//Advance row to next.
	this->pCurrentRow = this->pCurrentRow->pNext;

	return pVDElement;
}

//*****************************************************************************
template<typename VDElement>
VDElement* CDbVDInterface<VDElement>::GetNext()
//Gets next element.
//
//Returns:
//Pointer to loaded element which caller must delete, or NULL if no matching element
//was found.
{
	if (!this->bIsMembershipLoaded) LoadMembership();
	ASSERT(bIsMembershipLoaded);
	
	//If at end of rows, just return NULL.
	if (!this->pCurrentRow) return NULL;

	//Load element.
	VDElement *pVDElement = GetByID(pCurrentRow->dwID);
	
	//Advance row to next.
	pCurrentRow = pCurrentRow->pNext;

	return pVDElement;
}

//*****************************************************************************
template<typename VDElement>
VDElement* CDbVDInterface<VDElement>::GetNew()
//Get a new object that will be added to database when it is updated.
//
//Returns:
//Pointer to new element
{
	//After element object is updated, membership changes, so reset the flag.
	this->bIsMembershipLoaded = false;

	//Return new object.
	return new VDElement;	
}

//
//CDbVDInterface protected methods.
//

//*****************************************************************************
template<typename VDElement>
void CDbVDInterface<VDElement>::LoadMembership()
//Load the membership list with all viewdef IDs.
{
	ASSERT(CDbBase::IsOpen());
	c4_View View = CDbBase::GetView(this->viewDefStr);
	const DWORD dwCount = View.GetSize();

	//Each iteration gets a new ID and puts in membership list.
	this->MembershipIDs.Clear();
	for (DWORD dwViewDefI = 0; dwViewDefI < dwCount; ++dwViewDefI)
	{
		this->MembershipIDs.Add(this->viewDefPrimaryKeyProp(View[dwViewDefI]));
	}
	this->pCurrentRow = this->MembershipIDs.Get(0);
	this->bIsMembershipLoaded = true;
}


#endif //...#ifndef DBVDINTERFACE_H

// $Log: DbVDInterface.h,v $
// Revision 1.6  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/03 06:20:48  mrimer
// Added ResetMembership().
//
// Revision 1.4  2003/05/28 23:04:02  erikh2000
// Added method to get size of membership.
//
// Revision 1.3  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.2  2003/05/22 23:39:00  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.1  2003/05/20 18:10:43  mrimer
// Initial check-in (new DB base class w/ code refactored from other DB subclasses).
//
