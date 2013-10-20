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
 * 1997, 2000, 2001, 2002, 2003 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//DbBase.cpp
//Implementation of CDbBase.

#include "DbBase.h"
#include "Files.h"
#include "DbProps.h"
#include "Wchar.h"
#include "GameConstants.h"

//Module-scope vars.
static LANGUAGE_CODE	m_eLanguageCode = English;
static DWORD			m_dwRefCount = 0L;
static c4_Storage *		m_pHoldStorage = NULL;
static c4_Storage *		m_pPlayerStorage = NULL;
static c4_Storage *		m_pTextStorage = NULL;

//Used for checking the reference count at application exit.
DWORD GetDbRefCount(void) {return m_dwRefCount;}

//
//Public methods.
//

//*****************************************************************************
CDbBase::CDbBase(void)
	: pwczLastMessageText(NULL)
//Constructor.
{
	m_dwRefCount++;
}

//*****************************************************************************
CDbBase::~CDbBase(void)
//Destructor.
{
	--m_dwRefCount;
	if (m_dwRefCount == 0L) 
	{
		Close();
		ASSERT(!IsOpen());
	}
}

//*****************************************************************************
c4_ViewRef CDbBase::GetView(
//Get view from one of the three databases.
	const char *pszViewName) //(in) Name of viewdef.
//
//Returns:
//The view.
{
	if (strcmp(pszViewName, "Players")==0)
		return m_pPlayerStorage->View("Players");
	else if (strcmp(pszViewName, "MessageTexts")==0)
		return m_pTextStorage->View("MessageTexts");
	else
		return m_pHoldStorage->View(pszViewName);
}

//*****************************************************************************
void CDbBase::Commit(void)
//Commits changes in all the databases.
{
	ASSERT(IsOpen());
	m_pHoldStorage->Commit();
	m_pPlayerStorage->Commit();
	m_pTextStorage->Commit();
}

//*****************************************************************************
void CDbBase::Rollback(void)
//Rolls back changes in all the databases.
{
	ASSERT(IsOpen());
	m_pHoldStorage->Rollback();
	m_pPlayerStorage->Rollback();
	m_pTextStorage->Rollback();
}

//*****************************************************************************
LANGUAGE_CODE CDbBase::GetLanguage(void) const 
{
	return m_eLanguageCode;
}

//*****************************************************************************
bool CDbBase::IsOpen(void)
{
	return (m_pHoldStorage != NULL && m_pPlayerStorage != NULL && m_pTextStorage != NULL);
}


//*****************************************************************************
MESSAGE_ID CDbBase::Open(
//Opens database files.
//
//Note: There is some 1.6 code in here that has been revised to work with 1.5.
//Sorry for the silliness. -Erik
//
//Params:
	const char *pszPath)	//(in)	Path to database files.  If NULL (default),
							//		then the application's data path will be used.
//
//Returns:
//MID_Success or another message ID for failure.
{
	MESSAGE_ID dwRet = MID_Success;

	try
	{
		//Close databases if already open.
		if (IsOpen()) Close();
		ASSERT(!m_pHoldStorage);
		ASSERT(!m_pPlayerStorage);

		//Concatenate paths to hold and player databases.
		string strHoldDatPath, strPlayerDatPath, strTextDatPath;
        strHoldDatPath = (pszPath) ? pszPath : CFiles::GetDatPath();
		strHoldDatPath += szFILE_SEP;
		strHoldDatPath += "drod";
		strHoldDatPath += szDROD_VER;
		strHoldDatPath += ".dat";
		strPlayerDatPath = CFiles::GetDatPath();
		strPlayerDatPath += szFILE_SEP;
		strPlayerDatPath += "player.dat";
		strTextDatPath = CFiles::GetDatPath();
		strTextDatPath += szFILE_SEP;
		strTextDatPath += "text.dat";
		
		//Make sure the files exist.
		if ( !CFiles::DoesFileExist(strHoldDatPath.c_str())) 
			return MID_DatMissing;

		//Verify read and write access.
		if ( !CFiles::HasReadWriteAccess(strHoldDatPath.c_str())) 
			return MID_DatNoAccess;

		//Open the databases.
		m_pHoldStorage = new c4_Storage(strHoldDatPath.c_str(), true);
		if (!m_pHoldStorage) throw(MID_CouldNotOpenDB);
		m_pPlayerStorage = m_pHoldStorage;
		if (!m_pPlayerStorage) throw(MID_CouldNotOpenDB);
		m_pTextStorage = m_pHoldStorage;
		if (!m_pTextStorage) throw(MID_CouldNotOpenDB);
	}
	catch (MESSAGE_ID dwSetRetMessageID)
	{
		dwRet = dwSetRetMessageID;
	}
	return dwRet;
}

//*****************************************************************************
void CDbBase::Close(void)
//Closes database file.
{
	//Close hold database.
	if (m_pHoldStorage)
	{
		m_pHoldStorage->Commit();
		delete m_pHoldStorage;
		m_pHoldStorage = NULL;
	}

	m_pPlayerStorage = NULL;
	m_pTextStorage = NULL;
}

//*****************************************************************************
c4_View CDbBase::CreateHDbViewDef(const char *pszViewDef)
{
	ASSERT(m_pHoldStorage);
	ASSERT(pszViewDef);
	return m_pHoldStorage->GetAs(pszViewDef);
}

//*****************************************************************************
c4_View CDbBase::CreatePDbViewDef(const char *pszViewDef)
{
	ASSERT(m_pPlayerStorage);
	ASSERT(pszViewDef);
	return m_pPlayerStorage->GetAs(pszViewDef);
}

//*****************************************************************************
c4_View CDbBase::CreateTDbViewDef(const char *pszViewDef)
{
	ASSERT(m_pTextStorage);
	ASSERT(pszViewDef);
	return m_pTextStorage->GetAs(pszViewDef);
}

//*****************************************************************************
WCHAR * CDbBase::GetMessageText(
//Get message text from database.  If the current language is set to something other than
//English an attempt to retrieve that text will be made before returning English text.
//
//Params:
	const MESSAGE_ID	dwMessageID,	//(in) Message ID identifying message to retrieve.
	DWORD *		pdwLen)			//(in) If not NULL (default), will be set to length of
								//message.
//
//Returns:
//Pointer to class buffer with message stored in it.  Buffer is cleared after each call
//to GetMessageText.
{
	ASSERT(IsOpen());

	//Find message text.
	c4_View MessageTextsView = m_pTextStorage->View("MessageTexts");
	const DWORD dwFoundRowI = FindMessageText(dwMessageID, MessageTextsView);

	if (dwFoundRowI == ROW_NO_MATCH)
	{
		//Set last message to an empty string and return
		if (pdwLen) *pdwLen=0;
		return SetLastMessageText(wszEmpty, 0);
	}
	else
	{
		//Set last message to retrieved message and return.
		c4_Bytes MessageTextBytes = p_MessageText(MessageTextsView[dwFoundRowI]);
		const DWORD dwMessageTextLen = (MessageTextBytes.Size() - 1) / 2;
		if (pdwLen) *pdwLen=dwMessageTextLen;
		return SetLastMessageText((const WCHAR *) (MessageTextBytes.Contents()), dwMessageTextLen);
	}
}

//*****************************************************************************
WCHAR * CDbBase::GetAllocMessageText(
//Gets message text from database and copies to a return buffer allocated in this routine.  
//If the current language is set to something other than English an attempt to retrieve 
//that text will be made before returning English text.
//
//Params:
	MESSAGE_ID	dwMessageID,	//(in) Message ID identifying message to retrieve.
	DWORD *		pdwLen)			//(in) If not NULL (default), will be set to length of
								//message.
//
//Returns:
//Pointer to new buffer with message stored in it that caller must delete.
const
{
	ASSERT(IsOpen());

	//Find message text.
	c4_View MessageTextsView = m_pTextStorage->View("MessageTexts");
	DWORD dwFoundRowI = FindMessageText(dwMessageID, MessageTextsView);

	WCHAR *pwczRet = NULL;
	if (dwFoundRowI == ROW_NO_MATCH)
	{
		//Set new buffer to empty string and return
		if (pdwLen) *pdwLen=0;
		pwczRet = new WCHAR[1];
		pwczRet[0] = '\0';
	}
	else
	{
		//Copy retrieved message to new buffer and return.
		c4_Bytes MessageTextBytes = p_MessageText(MessageTextsView[dwFoundRowI]);
		DWORD dwMessageTextLen = (MessageTextBytes.Size() - 1) / 2;
		if (pdwLen) *pdwLen=dwMessageTextLen;
		pwczRet = new WCHAR[dwMessageTextLen+1];
		if (!pwczRet) return NULL;
		wcscpy(pwczRet, (const WCHAR *) (MessageTextBytes.Contents()));
	}
	return pwczRet;
}

//*****************************************************************************
DWORD CDbBase::AddMessageText(
//Writes a message to database.
//
//Params:
	const WCHAR *pwczMessage)		//(in) Message to write in Unicode.
//
//Returns:
//MessageID of new MessageTexts record.
{
	ASSERT(IsOpen());
	
	DWORD dwMessageLen = wcslen(pwczMessage);

	//Add message record.
	c4_View MessageTextsView = m_pTextStorage->GetAs(MESSAGETEXTS_VIEWDEF);
	c4_Bytes MessageBytes(pwczMessage, (dwMessageLen + 1)*sizeof(WCHAR));
	const DWORD dwMessageID = GetIncrementedID(p_MessageID);
	MessageTextsView.Add( 
			p_MessageTextID[ GetIncrementedID(p_MessageTextID) ] +
			p_MessageID[dwMessageID] + 
			p_LanguageCode[m_eLanguageCode] +
			p_MessageText[MessageBytes]);

	return dwMessageID;
}

//*****************************************************************************
void CDbBase::ChangeMessageText(
//Changes text of a message in database.
//
//Params:
	const DWORD dwMessageID,		//(in)	MessageID of message to change.  Current
											//		language will be used to look up text.
	const WCHAR *pwczMessage)		//(in)	Message to write in Unicode.
{
	ASSERT(dwMessageID);
	ASSERT(IsOpen());
	
	//Find the message text record.
	c4_View MessageTextsView = m_pTextStorage->GetAs(MESSAGETEXTS_VIEWDEF);
	DWORD dwRowI = FindMessageText(dwMessageID, MessageTextsView);
	if (dwRowI==ROW_NO_MATCH) {ASSERT(false); return;} //dwMessageID probably incorrect.
	
	//Because writes are expensive, check for an existing value that already
	//matches new value.
	{
		c4_Bytes OriginalMessageBytes = p_MessageText(MessageTextsView[dwRowI]);
		if (wcscmp(pwczMessage, (const WCHAR *) (OriginalMessageBytes.Contents()))==0)
			return; //New message is same as current, so nothing to do.
	}

	//Update message record.
	const DWORD dwMessageLen = wcslen(pwczMessage);
	c4_Bytes MessageBytes(pwczMessage, (dwMessageLen + 1)*sizeof(WCHAR));
	p_MessageText( MessageTextsView[ dwRowI ] ) = MessageBytes;
}

//*****************************************************************************
void CDbBase::DeleteMessage(
//Deletes all message texts (different languages) for one message ID.
//
//Params:
	const DWORD dwMessageID)	//(in)	MessageID of message text(s) to delete.
{
	c4_View MessageTextsView = m_pTextStorage->View("MessageTexts");

	const DWORD dwRowCount = MessageTextsView.GetSize();
	for (DWORD dwRowI = dwRowCount - 1L; dwRowI != (DWORD)(-1); --dwRowI)
	{
		if (dwMessageID == (DWORD) (p_MessageID(MessageTextsView[dwRowI])))
			MessageTextsView.RemoveAt(dwRowI);
	}
}

//*****************************************************************************
DWORD CDbBase::GetIncrementedID(
//Gets the next incremented ID value for a property and increments the value
//in the hold database.
//
//Params:
	const c4_IntProp &propID)		//(in) One of the properties stored in IncrementedIds table.
//
//Returns:
//The next ID which should not be in use.
{
	ASSERT(IsOpen());

	//Which database contains viewdef for this key field?
	c4_Storage *pStorage;
	if (propID == p_PlayerID)
		pStorage = m_pPlayerStorage;
	else if (propID == p_MessageID || propID == p_MessageTextID)
		pStorage = m_pTextStorage;
	else
		pStorage = m_pHoldStorage;

	c4_View IncrementedIDsView = pStorage->View("IncrementedIds");
	DWORD dwID = propID(IncrementedIDsView[0]);
	
	//Set ID value in table to this ID that will now be considered in use.
	propID(IncrementedIDsView[0]) = ++dwID;

	return dwID;	
}	

//
//Protected methods.
//

//*****************************************************************************
DWORD CDbBase::LookupRowByPrimaryKey(
//Looks up a row in a view by its primary key ID property.  Assumes primary key
//property is in sequential order.
//
//Params:
	const DWORD dwID,		//(in) Primary key value to match.
	c4_IntProp &IDProp,	//(in) Property that contains primary key.
	c4_View &View)		//(in) View to look for row within.
//
//Returns:
//Row index or ROW_NO_MATCH.
{	
	//Binary search for ID.
	DWORD dwRowI, dwThisID;
	const DWORD dwRowCount = View.GetSize();
	ASSERT(dwRowCount < ROW_NO_MATCH);
	if (dwRowCount == 0) return ROW_NO_MATCH; //Now rows to search.
	DWORD dwFirstRowI = 0;
	DWORD dwLastRowI = dwRowCount - 1;
	while (dwFirstRowI <= dwLastRowI) //Each iteration is one test at a new row position.
	{
		dwRowI = dwFirstRowI + (dwLastRowI - dwFirstRowI + 1) / 2;
		dwThisID = (DWORD) (IDProp(View[dwRowI]));
		if (dwThisID == dwID) return dwRowI;
		if (dwThisID < dwID)
		{
			dwFirstRowI = dwRowI + 1;
		}
		else
		{
			dwLastRowI = dwRowI - 1;
			if (dwLastRowI == (DWORD) -1) return ROW_NO_MATCH; //Prevent looping err.
		}
	}

	return ROW_NO_MATCH;
}

//
//Private methods.
//

//*****************************************************************************
DWORD CDbBase::FindMessageText(
//Finds a message text row that matches a message ID and current language.
//
//Params:
	const DWORD		dwMessageID,	//(in) Message ID to look for.
	c4_View &	MessageTextsView)	//(in) Open MessageTexts view to use.
//
//Returns:
//Index of row in MessageTextsView to use.  ROW_NO_MATCH if no matching 
//message ID was found.
const
{
	const DWORD dwRowCount = MessageTextsView.GetSize();
	DWORD dwRowI, dwEnglishRowI = ROW_NO_MATCH, dwFoundRowI = ROW_NO_MATCH;
	LANGUAGE_CODE eSeekLanguageCode;

	for (dwRowI = 0; dwRowI < dwRowCount; dwRowI++)
	{
		if (dwMessageID == (DWORD) (p_MessageID(MessageTextsView[dwRowI])))
		{
			eSeekLanguageCode = 
					(LANGUAGE_CODE) (int) p_LanguageCode(MessageTextsView[dwRowI]);
			if (eSeekLanguageCode == m_eLanguageCode)
			{		//Found message/language match.
				return dwRowI;
			}
			else	//Found right message, but wrong language.
			{
				if (eSeekLanguageCode == English) dwEnglishRowI = dwRowI;
				dwFoundRowI = dwRowI;
			}
		}
	}
	//No message/language match.  If found an message/English match then return that.
	if (dwEnglishRowI != ROW_NO_MATCH) return dwEnglishRowI;

	//Otherwise return a match found in any language.
	return dwFoundRowI;
}

//*****************************************************************************
WCHAR *CDbBase::SetLastMessageText(
//Sets class char buffer to contain specified text.
//
//Params:
	const WCHAR *pwczNewMessageText,	//(in) New text to copy to char buffer.
	const DWORD dwNewMessageTextLen)			//(in) Chars in message text.
//
//Returns:
//Point to char buffer.
{
	ASSERT(dwNewMessageTextLen == wcslen(pwczNewMessageText));

	//Release current char buffer.
	delete[] this->pwczLastMessageText;
	this->pwczLastMessageText = NULL;

	//Allocate new char buffer.
	this->pwczLastMessageText = new WCHAR[dwNewMessageTextLen + 1];
	if (!this->pwczLastMessageText) return NULL;

	//Copy new text to char buffer.
	wcscpy(this->pwczLastMessageText, pwczNewMessageText);

	//Return char buffer pointer.
	return this->pwczLastMessageText;
}

// $Log: DbBase.cpp,v $
// Revision 1.2  2003/05/19 21:12:14  mrimer
// Some code maintenance.
//
// Revision 1.1  2003/02/25 00:01:24  erikh2000
// Initial check-in.
//
// Revision 1.19  2003/02/24 19:01:43  erikh2000
// Added methods for creating viewdefs.
//
// Revision 1.18  2003/02/24 17:00:17  erikh2000
// Wrote code to handle data in three databases instead of one.  Code is temporarily commented to only use one database for now.
//
// Revision 1.17  2003/02/17 00:48:11  erikh2000
// Remove L" string literals.
//
// Revision 1.16  2002/12/22 02:08:41  mrimer
// Made some vars const.
//
// Revision 1.15  2002/11/13 23:12:24  mrimer
// Moved GetPlayerID() and SetPlayerID() to CDb.
// Made DeleteMessage() static.
// Made some parameters const.
//
// Revision 1.14  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.13  2002/10/17 16:47:37  mrimer
// Added FlushWrites().  Made class methods static.
//
// Revision 1.12  2002/06/16 06:20:53  erikh2000
// Added method to delete a message.
//
// Revision 1.11  2002/06/15 18:24:37  erikh2000
// Added new Commit() method.
// Moved some static object members to module-scope variables for clarity.
// Changed code so that storage is not committed after each database write.
//
// Revision 1.10  2002/06/09 06:09:57  erikh2000
// Fixed some errors that would cause the first MessageText row to not be retrieved.
// Added ChangeMessageText() method.
//
// Revision 1.9  2002/06/05 03:04:14  mrimer
// Fixed errors where "delete []" should be used instead of "delete".
// Removed superfluous NULL pointer checks.
//
// Revision 1.8  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.7  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.6  2002/02/12 04:18:41  erikh2000
// Fixed a release build warning on a VERIFY macro.
//
// Revision 1.5  2001/12/16 02:18:21  erikh2000
// Fixed an access violation error.
//
// Revision 1.4  2001/11/20 00:52:48  erikh2000
// Added CDbBase::GetIncrementedID() and CDbBase::AddMessageText().
//
// Revision 1.3  2001/10/26 23:01:37  erikh2000
// Used protected constructors to discourage direct instantiation.
// Added const declarations for read-only methods so that const pointers to objects would be useful.
//
// Revision 1.2  2001/10/25 06:55:46  erikh2000
// Fixed #40948 "Fix bug in CDbBase::LookupRowByPrimaryKey()".
//
// Revision 1.1.1.1  2001/10/01 22:20:07  erikh2000
// Initial check-in.
//
