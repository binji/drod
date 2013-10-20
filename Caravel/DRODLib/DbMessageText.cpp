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
#include <BackEndLib/Wchar.h>

//
//Public methods.
//

//*****************************************************************************
CDbMessageText::CDbMessageText(
//Constructor.
//
//Params:
	const MESSAGE_ID eSetMessageID) //(in)	MessageID that text will be bound
                                  //	    to.  UNBOUND_MESSAGE (0) is default.
{
	this->bIsDirty = this->bIsLoaded = false;
	this->eMessageID = UNBOUND_MESSAGE;
	this->wstrText = wszEmpty;
	if (eSetMessageID)
		Bind(eSetMessageID);
	else
		BindNew();
}

//*****************************************************************************
CDbMessageText::~CDbMessageText()
//Destructor.
{
}

//*****************************************************************************
void CDbMessageText::Bind(
//Bind text to this message ID.  Subsequent database loads and updates will
//affect a record in MessageTexts determined by this value.
//
//Params:
	const MESSAGE_ID eSetMessageID)	//(in)	MessageID to bind to.
{
	ASSERT(eSetMessageID != UNBOUND_MESSAGE);

	Flush();
	this->eMessageID = eSetMessageID;
	this->bIsLoaded = false;
}

//*****************************************************************************
void CDbMessageText::BindNew()
//Bind text to a new message that doesn't exist yet.  
{
	Flush();
	this->eMessageID = UNBOUND_MESSAGE;
	this->bIsLoaded = false;
}

//*****************************************************************************
void CDbMessageText::Clear()
//Release binding and set string to empty without making any writes.
{
	this->bIsDirty = this->bIsLoaded = false;
	this->eMessageID = UNBOUND_MESSAGE;
	this->wstrText.resize(0);
}

//*****************************************************************************
void CDbMessageText::Delete()
	//Deletes message text permanently from the DB.
{
	DeleteMessage(this->eMessageID);
}

//*****************************************************************************
MESSAGE_ID CDbMessageText::Flush()
//Commits changes to the database.  If text was not bound, it will become bound
//to new message ID.
{
    ASSERT(CDbBase::IsOpen());
	if (this->bIsDirty)
	{
		if (this->eMessageID != UNBOUND_MESSAGE)
			ChangeMessageText(this->eMessageID, 
					(this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty);
		else
			this->eMessageID = AddMessageText(
					(this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty);
		this->bIsDirty = false;
	}
	return this->eMessageID;
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
	WCHAR const* pwczText = NULL;
   if (text.bIsLoaded)
	   pwczText = (const WCHAR*)text;
   else if (text.eMessageID != UNBOUND_MESSAGE)
	   pwczText = GetMessageText(text.eMessageID);

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
	if (!this->bIsLoaded && this->eMessageID != UNBOUND_MESSAGE)
	{
		const WCHAR *pwczText = GetMessageText(this->eMessageID);
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
//NOTE: This method cannot be const!
{
	return (WCScmp((const WCHAR *)*this, (const WCHAR *)text) == 0);
}

//*****************************************************************************
CDbMessageText::operator const WCHAR *()
//Returns const string, loading from database if needed.
{
	if (!this->bIsLoaded && this->eMessageID != UNBOUND_MESSAGE)
	{
		const WCHAR *pwczText = GetMessageText(this->eMessageID);
		this->wstrText = (pwczText != NULL) ? pwczText : wszEmpty;
		ASSERT(pwczText); //Probably bad binding if fires.
		this->bIsLoaded = true;
	}
	return (this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty;
}

//*****************************************************************************
CDbMessageText::operator const WCHAR *() const
//Returns const string.  No loading from DB.
{
   ASSERT(this->bIsLoaded);
	return (this->wstrText.size()) ? this->wstrText.c_str() : wszEmpty;
}

// $Log: DbMessageText.cpp,v $
// Revision 1.15  2004/08/10 01:56:57  mrimer
// Fixed scroll message bugs.
//
// Revision 1.14  2003/10/06 02:41:20  erikh2000
// Added descriptions to assertions.
//
// Revision 1.13  2003/07/22 18:23:51  mrimer
// Really fixed it this time.
//
// Revision 1.12  2003/07/21 23:13:56  mrimer
// Fixed the last change.
//
// Revision 1.11  2003/07/21 22:04:44  mrimer
// Fixed bug in assignment operator.
//
// Revision 1.10  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/06/12 21:42:28  mrimer
// Fixed a bug in operator==().
//
// Revision 1.8  2003/05/08 23:20:13  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.7  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
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
