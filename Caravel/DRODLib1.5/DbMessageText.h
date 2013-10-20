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

//DbMessageText.h
//Declarations for CDbMessageText.
//Class for handling message texts set and retrieved from database.

//USAGE
//
//To provide access to any message text in the database, you can construct a new
//CDbMessageText with the message ID you want.  The text will load on the first
//time it is used, and remain in memory until the object is destroyed.  
//
//  CDbMessageText Text(MID_UnexpectedNeatherDeath);
//	ShowOkayMessage(Text); //Text not loaded until this line.
//
//A one-time construction should work fine too:
//
//  ShowOkayMessage(CDbMessageText(MID_UnexpectedNeatherDeath)); //Text loaded for the call then freed.
//
//Overwriting the value can be done with an assignment call:
//
//  {
//    CDbMessageText NameText(dwNameMID);
//    NameText = L"Charlie";
//  } //Database write occurs here.
//
//The actual database write will occur either when the object is destroyed or a call to
//Flush() is made.  If you don't want a database write when the object is destroyed, make
//a call to Cancel().
//
//If you don't have the message ID at time of construction, you might want to make a call
//to Bind() later.  Like so:
//
//  CDbMessageText NameText;
//  DWORD dwNameMID = GetNameMID();
//  if (dwNameMID)
//  {
//		NameText.Bind(dwNameMID);
//		NameText = L"Charlie";
//  }
//
//If you want to add a new message text, you can assign text to an unbound object and 
//flush it.  Flush() will return a new message ID, and the object will then be bound
//to that new message.  Like so:
//
//  CDbMessageText NameText;
//	NameText = L"Charlie";
//  DWORD dwNameMID = NameText.Flush();
//
//OTHER COMMENTS
//
//Don't expect two or more CDbMessageText objects updating the same message at the
//same time to stay in sync.
//
//The currently selected language affects all operations.  Each message text is uniquely
//identified by a message ID and language code.

#ifndef DBMESSAGETEXT_H
#define DBMESSAGETEXT_H

#include "DbBase.h"
#include "MessageIDs.h"
#include "AttachableObject.h"

#include <string>

using namespace std;

class CDbMessageText : public CDbBase, public CAttachableObject
{
public:
	CDbMessageText(const MESSAGE_ID dwSetMessageID = 0L);
	virtual ~CDbMessageText(void);
	
	//Called implicitly by constructor or explicitly to bind the text to an existing
	//message in the database.
	void	Bind(const MESSAGE_ID dwSetMessageID);

	//Called implicitly by constructor or explicitly to bind the text to a new
	//message in the database.  The new message will not exist until Flush() is called.
	void	BindNew(void);

	//Cancel any writes that would occur when Flush() is called.
	void	Cancel(void) {this->bIsDirty = false;}

	//Release binding and set string to empty without making any writes.
	void	Clear(void);

	//Deletes message text permanently from the DB.
	void	Delete(void);

	//Called implicitly by destructor or explicitly to write any text changes to the
	//database.  If you don't bind to a message, a new one will be created.  The 
	//bound message ID will be returned in either case.
	MESSAGE_ID Flush(void);

	bool	IsEmpty(void) {return this->wstrText.size() == 0;}
	
	virtual bool	Update(void) {Flush();  return true;}

	//Assign a new string.
	const WCHAR * operator = (const WCHAR * wczSet);
	const WCHAR * operator = (const CDbMessageText &text);

	//Append a string to end.
	const WCHAR * operator += (const WCHAR * wczSet);

	//Compare two strings.
	bool operator == (CDbMessageText &text) const;

	//Returns const string, loading from DB if needed.  Don't try to modify the 
	//returned string, because you will be circumventing the state change code.  
	//Use above string modifications operators or add new ones.
	operator const WCHAR *(void);
	operator const WCHAR *(void) const;	//no loading from DB

private:
	DWORD		dwMessageID;
	WSTRING		wstrText;
	bool		bIsLoaded;
	bool		bIsDirty;
};

#endif //...#ifndef DBMESSAGETEXT_H

// $Log: DbMessageText.h,v $
// Revision 1.1  2003/02/25 00:01:30  erikh2000
// Initial check-in.
//
// Revision 1.9  2003/02/16 20:29:32  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.8  2003/01/08 00:42:05  mrimer
// Fixed bug in assignment operator.
//
// Revision 1.7  2002/12/22 01:34:51  mrimer
// Added Update().
// Added IsEmpty(), operator= and operator==.
//
// Revision 1.6  2002/11/13 23:13:49  mrimer
// Added Delete().
// Made some parameters const.
//
// Revision 1.5  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.4  2002/07/25 17:27:32  mrimer
// Added explicit virtual destructor.
//
// Revision 1.3  2002/07/19 20:23:15  mrimer
// Added CAttachableObject references.
//
// Revision 1.2  2002/06/15 18:28:45  erikh2000
// Added BindNew() method.
// Fixed some state logic errors.
//
// Revision 1.1  2002/06/09 05:58:56  erikh2000
// Initial check-in.
//