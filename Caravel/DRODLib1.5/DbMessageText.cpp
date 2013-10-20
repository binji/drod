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

#include "DbMessageText.h"
#include "Wchar.h"

//
//Public methods.
//

//*****************************************************************************
CDbMessageText::CDbMessageText(
//Constructor.
//
//Params:
	const MESSAGE_ID dwSetMessageID)	//(in)	MessageID that text will be bound
												//	to.  0 (unbound) is default.
{
	this->bIsDirty = this->bIsLoaded = false;
	this->dwMessageID = 0L;
	this->wstrText = wszEmpty;
	if (dwSetMessageID)
		Bind(dwSetMessageID);
	else
		BindNew();
}

//*****************************************************************************
CDbMessageText::~CDbMessageText(void)
//Destructor.
{
	//Write anything that hasn't been written.
	Flush();
}

//*****************************************************************************
void CDbMessageText::Bind(
//Bind text to this message ID.  Subsequent database loads and updates will
//affect a record in MessageTexts determined by this value.
//
//Params:
	const MESSAGE_ID dwSetMessageID)	//(in)	MessageID to bind to.
{
	ASSERT(dwSetMessageID != 0L);

	Flush();
	this->dwMessageID = dwSetMessageID;
	this->bIsLoaded = false;
}

//*****************************************************************************
void CDbMessageText::BindNew(void)
//Bind text to a new message that doesn't exist yet.  
{
	Flush();
	this->dwMessageID = 0L;
	this->bIsLoaded = false;
}

//*****************************************************************************
void CDbMessageText::Clear(void)
//Release binding and set string to empty without making any writes.
{
	this->bIsDirty = this->bIsLoaded = false;
	this->dwMessageID = 0L;
	this->wstrText.resize(0);
}

//*****************************************************************************
void CDbMessageText::Delete(void)
	//Deletes message text permanently from the DB.
{
	DeleteMessage(this->dwMessageID);
}

//*****************************************************************************
MESSAGE_ID CDbMessageText::Flush(void)
//Commits changes to the database.  If text was not bound, it will become bound
//to new message ID.
{
	if (this->bIsDirty)
	{
		if (this->dwMessageID)
			ChangeMessageText(this->dwMessageID, 
					(this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty);
		else
			this->dwMessageID = AddMessageText(
					(this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty);
		this->bIsDirty = false;
	}
	return this->dwMessageID;
}

//*****************************************************************************
const WCHAR * CDbMessageText::operator = (
//Assign a new string.
//
//Params:
	const WCHAR * wczSet) //(in)	New string.
//
//Returns:
//Const pointer to string.
{
	this->wstrText = (wczSet ? wczSet : wszEmpty);
	this->bIsDirty = true;
	this->bIsLoaded = true; //In other words, if we weren't loaded before, it
							//doesn't matter now, because new value will overwrite.

	return (this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty;
}

//*****************************************************************************
const WCHAR * CDbMessageText::operator = (
//Assign a new string.
//
//Params:
	const CDbMessageText &text) //(in)	Make copy of this text.
{
	WCHAR const* pwczText;
	if (!text.bIsLoaded && text.dwMessageID)
	{
		pwczText = GetMessageText(text.dwMessageID);
	} else {
		pwczText = (const WCHAR*)text;
	}

	return *this = pwczText;
}

//*****************************************************************************
const WCHAR * CDbMessageText::operator += (
//Append a string to end.
//
//Params:
	const WCHAR * wczSet) //(in)	String to append to current.
//
//Returns:
//Const pointer to string.
{
	if (!this->bIsLoaded && this->dwMessageID)
	{
		const WCHAR *pwczText = GetMessageText(this->dwMessageID);
		this->wstrText = (pwczText != NULL) ? pwczText : wszEmpty;
		ASSERT(pwczText); //Probably bad binding if fires.
		this->bIsLoaded = true;
	}

	this->wstrText += wczSet;
	this->bIsDirty = true;

	return (this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty;
}

//*****************************************************************************
bool CDbMessageText::operator == (
//Compare two strings.
//
//Params:
	CDbMessageText &text)	//(in)
const
{
	return !wcscmp(this->wstrText.c_str(), text.wstrText.c_str());
}

//*****************************************************************************
CDbMessageText::operator const WCHAR *(void)
//Returns const string, loading from database if needed.
{
	if (!this->bIsLoaded && this->dwMessageID)
	{
		const WCHAR *pwczText = GetMessageText(this->dwMessageID);
		this->wstrText = (pwczText != NULL) ? pwczText : wszEmpty;
		ASSERT(pwczText); //Probably bad binding if fires.
		this->bIsLoaded = true;
	}
	return (this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty;
}

//*****************************************************************************
CDbMessageText::operator const WCHAR *(void) const
//Returns const string.  No loading from DB.
{
	return (this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty;
}

// $Log: DbMessageText.cpp,v $
// Revision 1.1  2003/02/25 00:01:30  erikh2000
// Initial check-in.
//
// Revision 1.6  2003/02/17 00:48:11  erikh2000
// Remove L" string literals.
//
// Revision 1.5  2003/01/08 00:42:05  mrimer
// Fixed bug in assignment operator.
//
// Revision 1.4  2002/12/22 02:08:12  mrimer
// Added some operators.
//
// Revision 1.3  2002/11/13 23:13:49  mrimer
// Added Delete().
// Made some parameters const.
//
// Revision 1.2  2002/06/15 18:28:45  erikh2000
// Added BindNew() method.
// Fixed some state logic errors.
//
// Revision 1.1  2002/06/09 05:58:55  erikh2000
// Initial check-in.
//
