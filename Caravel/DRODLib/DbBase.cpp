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

//Define props in this object
#define INCLUDED_FROM_DBBASE_CPP

#include "DbBase.h"
#include "DBProps.h"
#include "GameConstants.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

#include <fstream>

//Module-scope vars.
static LANGUAGE_CODE    m_eLanguageCode = English;
static DWORD			m_dwRefCount = 0L;
static c4_Storage *		m_pHoldStorage = NULL;
static c4_Storage *		m_pPlayerStorage = NULL;
static c4_Storage *		m_pTextStorage = NULL;

//Used for checking the reference count at application exit.
DWORD GetDbRefCount() {return m_dwRefCount;}

//
//Public methods.
//

//*****************************************************************************
CDbBase::CDbBase()
	: pwczLastMessageText(NULL)
//Constructor.
{
	++m_dwRefCount;
}

//*****************************************************************************
CDbBase::~CDbBase()
//Destructor.
{
   ASSERT(m_dwRefCount != 0);
	--m_dwRefCount;
	if (m_dwRefCount == 0L)
	{
		Close();
		ASSERT(!IsOpen());
	}

   //Release current char buffer.
   delete[] this->pwczLastMessageText;
   this->pwczLastMessageText = NULL;

}

//*****************************************************************************
MESSAGE_ID CDbBase::SetProperty(
//NOTE: Unused parameter names commented out to suppress warnings
   const PROPTYPE /*pType*/,
    char* const /*str*/,
    CImportInfo &/*Maps*/,
    bool &/*bSaveRecord*/)
{
    return MID_NoText;
}

//*****************************************************************************
MESSAGE_ID CDbBase::SetProperty(
//NOTE: Unused parameter names commented out to suppress warnings
   const VIEWPROPTYPE /*vpType*/,
   const PROPTYPE /*pType*/,
   char* const /*str*/,
   CImportInfo &/*Maps*/)
{
    return MID_NoText;
}

//*****************************************************************************
bool CDbBase::IsStorageFileValid(const WCHAR *pwzFilepath)
{
    //Verify file exists.
    if (!CFiles::DoesFileExist(pwzFilepath))
    {
        ASSERTP(false, "Storage file doesn't exist.");
        return false;
    }

    //Open storage.
    char filename[FILENAME_MAX];
    UnicodeToAscii(pwzFilepath, filename);
    c4_Storage DatFile(filename, false);

    //Get the view structure.
    const char *pszViewStruct = DatFile.Description();

    //Get list of view names from structure.
    vector<string> ViewNames;
    string strViewName;
    const UINT MAXLEN_VIEWSTRUCT = 10000; //An arbitrary too-large number.
    const char *pszSeek = pszViewStruct, *pszStop = pszViewStruct + MAXLEN_VIEWSTRUCT;
    UINT wBracketLevel = 0;
    while (*pszSeek && pszSeek != pszStop)
    {
        if (*pszSeek == '[')
        {
            ++wBracketLevel;
            if (wBracketLevel == 1)
            {
                ViewNames.push_back(strViewName);
                strViewName = "";
            }
        }
        else if (*pszSeek == ']')
                --wBracketLevel;
        else if (!(*pszSeek >= 'a' && *pszSeek <= 'z') && !(*pszSeek >= 'A' && *pszSeek <= 'Z') &&
                !(*pszSeek >= '0' && *pszSeek <= '9') && *pszSeek != ':' && *pszSeek != ',' &&
                *pszSeek != '_')
        {
            ASSERTP(false, "Bad char in view structure.");
            return false;
        }
        else
        {
            if (wBracketLevel == 0 && *pszSeek != ',')
                strViewName += *pszSeek;
        }
        ++pszSeek;
    }
    if (ViewNames.size()==0) {
       ASSERTP(false, "Corrupted view structure."); return false;
    }

    //Check each view.
    for (vector<string>::const_iterator iView = ViewNames.begin(); iView != ViewNames.end(); ++iView)
    {
        c4_View CheckView = DatFile.View(iView->c_str());
        UINT wPropCount = CheckView.NumProperties();
        if (!wPropCount) {
           ASSERTP(false, "View with no properties."); return false;
        }
    }

    //Storage file is probably okay.
    return true;
}

//*****************************************************************************
c4_ViewRef CDbBase::GetView(
//Get view from one of the three databases.
	const char *pszViewName) //(in) Name of viewdef.
//
//Returns:
//The view.
{
	if (strcmp(pszViewName, ViewTypeStr(V_Players))==0)
		return m_pPlayerStorage->View(ViewTypeStr(V_Players));
	else if (strcmp(pszViewName, "MessageTexts")==0)
		return m_pTextStorage->View("MessageTexts");
	else
		return m_pHoldStorage->View(pszViewName);
}

//*****************************************************************************
void CDbBase::Commit()
//Commits changes in all the databases.
{
	ASSERT(IsOpen());
	m_pHoldStorage->Commit();
	m_pPlayerStorage->Commit();
	m_pTextStorage->Commit();
}

//*****************************************************************************
void CDbBase::Rollback()
//Rolls back changes in all the databases.
{
    ASSERT(IsOpen());
    m_pHoldStorage->Rollback();
    m_pPlayerStorage->Rollback();
    m_pTextStorage->Rollback();
}

//*****************************************************************************
LANGUAGE_CODE CDbBase::GetLanguage() const
{
	return m_eLanguageCode;
}

//*****************************************************************************
void CDbBase::SetLanguage(LANGUAGE_CODE eSetLanguageCode)
{
	m_eLanguageCode = eSetLanguageCode;
}

//*****************************************************************************
bool CDbBase::IsOpen()
{
	return (m_pHoldStorage != NULL && m_pPlayerStorage != NULL && m_pTextStorage != NULL);
}


//*****************************************************************************
MESSAGE_ID CDbBase::Open(
//Opens database files.
//
//Params:
	const WCHAR *pwszDatFilepath)	//(in)	Path to database files.  If NULL (default),
							//		then the application's data path will be used.
//
//Returns:
//MID_Success or another message ID for failure.
{
    MESSAGE_ID dwRet = MID_Success;
    static const WCHAR pwszDrod[] = { W_t('d'), W_t('r'), W_t('o'), W_t('d'), W_t(0) };
    static const WCHAR pwszDotDat[] = { W_t('.'), W_t('d'), W_t('a'), W_t('t'), W_t(0) };
    static const WCHAR pwszPlayer[] = { W_t('p'), W_t('l'), W_t('a'), W_t('y'), W_t('e'), W_t('r'), W_t('.'), W_t('d'), W_t('a'), W_t('t'), W_t(0) };
    static const WCHAR pwszText[] = { W_t('t'), W_t('e'), W_t('x'), W_t('t'), W_t('.'), W_t('d'), W_t('a'), W_t('t'), W_t(0) };

    try
	{
        //Close databases if already open.
        Close();
        ASSERT(!m_pHoldStorage);
        ASSERT(!m_pPlayerStorage);

        //Concatenate paths to hold and player databases.
        CFiles Files;
        WSTRING wstrHoldDatPath, wstrPlayerDatPath, wstrTextDatPath;
        wstrHoldDatPath = (pwszDatFilepath) ? pwszDatFilepath : Files.GetDatPath();
        wstrHoldDatPath += wszSlash;
        wstrHoldDatPath += pwszDrod;
        wstrHoldDatPath += wszDROD_VER;
        wstrHoldDatPath += pwszDotDat;
        wstrPlayerDatPath = Files.GetDatPath();
        wstrPlayerDatPath += wszSlash;
        wstrPlayerDatPath += pwszPlayer;
        wstrTextDatPath = Files.GetDatPath();
        wstrTextDatPath += wszSlash;
        wstrTextDatPath += pwszText;

        //Make sure the files exist.
        if (    !CFiles::DoesFileExist(wstrHoldDatPath.c_str()) ||
                !CFiles::DoesFileExist(wstrPlayerDatPath.c_str()) ||
				!CFiles::DoesFileExist(wstrTextDatPath.c_str()) )
            return MID_DatMissing;

		//Verify read and write access.
        if (    !CFiles::HasReadWriteAccess(wstrHoldDatPath.c_str()) ||
                !CFiles::HasReadWriteAccess(wstrTextDatPath.c_str()) ||
                !CFiles::HasReadWriteAccess(wstrPlayerDatPath.c_str()) )
            return MID_DatNoAccess;

        //Are databases valid?
        if (    IsStorageFileValid(wstrHoldDatPath.c_str()) &&
                IsStorageFileValid(wstrPlayerDatPath.c_str()) &&
                IsStorageFileValid(wstrTextDatPath.c_str())) //Yes.
        {
            BackupStorageFile(wstrHoldDatPath.c_str());
            BackupStorageFile(wstrTextDatPath.c_str());
            BackupStorageFile(wstrPlayerDatPath.c_str());
        }
        else //No, databases are not valid.
        {
            if (    !RestoreStorageFile(wstrHoldDatPath.c_str()) ||
                    !RestoreStorageFile(wstrTextDatPath.c_str()) ||
                    !RestoreStorageFile(wstrPlayerDatPath.c_str()) )
                return MID_DatCorrupted_NoBackup;
            else
                dwRet = MID_DatCorrupted_Restored;
        }

        //Open the databases.
        //!!For now, we'll ignore the WCHAR filenames and open the DB the fast and memory efficient way.
        char filename[FILENAME_MAX];
        UnicodeToAscii(wstrHoldDatPath.c_str(), filename);
        m_pHoldStorage = new c4_Storage(filename, true);
        UnicodeToAscii(wstrPlayerDatPath.c_str(), filename);
        m_pPlayerStorage = new c4_Storage(filename, true);
        UnicodeToAscii(wstrTextDatPath.c_str(), filename);
        m_pTextStorage = new c4_Storage(filename, true);
        if (!m_pHoldStorage || !m_pPlayerStorage || !m_pTextStorage)
            throw MID_CouldNotOpenDB;
	}
	catch (MESSAGE_ID dwSetRetMessageID)
	{
        dwRet = dwSetRetMessageID;
        delete m_pTextStorage; m_pTextStorage = NULL;
        delete m_pPlayerStorage; m_pPlayerStorage = NULL;
        delete m_pHoldStorage; m_pHoldStorage = NULL;
	}
	return dwRet;
}

//*****************************************************************************
void CDbBase::Close(const bool bCommit)   //Commit before closing (default).
//Closes database files.
{
	//Close hold database.
	if (m_pHoldStorage)
   {
      if (bCommit)
         m_pHoldStorage->Commit();
		delete m_pHoldStorage; m_pHoldStorage = NULL;
   }

	//Close player database.
	if (m_pPlayerStorage)
   {
      if (bCommit)
         m_pPlayerStorage->Commit();
		delete m_pPlayerStorage; m_pPlayerStorage = NULL;
   }

	//Close text database.
	if (m_pTextStorage)
   {
      if (bCommit)
         m_pTextStorage->Commit();
		delete m_pTextStorage; m_pTextStorage = NULL;
	}
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
		DWORD dwMessageTextLen = (MessageTextBytes.Size() - 1) / 2;
		if (pdwLen) *pdwLen=dwMessageTextLen;

        //Which buffer to use?  For speed try to use smaller buffer to avoid allocs.
        const DWORD MAXLEN_SMALL_BUF = 500, MAXLEN_LARGE_BUF = 10000; //10k is unexpectedly large.
        static WCHAR wzSmallBuf[MAXLEN_SMALL_BUF + 1];
        WCHAR *pwzLargeBuf = NULL, *pwzUseBuf;
        if (dwMessageTextLen > MAXLEN_SMALL_BUF)
        {
            if (dwMessageTextLen < MAXLEN_LARGE_BUF)
            {
                pwzUseBuf = pwzLargeBuf = new WCHAR[dwMessageTextLen + 1];
                if (!pwzUseBuf)
                {
                    ASSERTP(false, "Low memory condition.");
                    return NULL;
                }
            }
            else //This is probably corrupted data, but I will try to show the first part of it.
            {
                dwMessageTextLen = *pdwLen = MAXLEN_SMALL_BUF;
                pwzUseBuf = wzSmallBuf;
            }
        }
        else
            pwzUseBuf = wzSmallBuf;

		memcpy( (void*)pwzUseBuf, (const void*)MessageTextBytes.Contents(), dwMessageTextLen * sizeof(WCHAR));
      WCv(pwzUseBuf[dwMessageTextLen]) = 0;

#ifdef __sgi
		char *pStr = (char*)&wszTmp[0];
		for (int n=0; n < dwMessageTextLen; n++)
		{
			char c = pStr[n*2];
			pStr[n*2] = pStr[n*2+1];
			pStr[n*2+1] = c;
		}
#endif
		WCHAR *pwzRet = SetLastMessageText(pwzUseBuf, dwMessageTextLen);
        delete[] pwzLargeBuf;
        return pwzRet;
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
		WCv(pwczRet[0]) = '\0';
	}
	else
	{
		//Copy retrieved message to new buffer and return.
		c4_Bytes MessageTextBytes = p_MessageText(MessageTextsView[dwFoundRowI]);
		DWORD dwMessageTextLen = (MessageTextBytes.Size() - 1) / 2;
		if (pdwLen) *pdwLen=dwMessageTextLen;
		pwczRet = new WCHAR[dwMessageTextLen+1];
		if (!pwczRet) return NULL;
		WCScpy(pwczRet, (const WCHAR *) (MessageTextBytes.Contents()));
	}
	return pwczRet;
}

//*****************************************************************************
MESSAGE_ID CDbBase::AddMessageText(
//Writes a message to database.
//
//Params:
	const WCHAR *pwczMessage)		//(in) Message to write in Unicode.
//
//Returns:
//MessageID of new MessageTexts record.
{
	ASSERT(IsOpen());

	const UINT dwMessageLen = WCSlen(pwczMessage);

	//Add message record.
	c4_View MessageTextsView = m_pTextStorage->View("MessageTexts");
	c4_Bytes MessageBytes(pwczMessage, (dwMessageLen + 1)*sizeof(WCHAR));
	const MESSAGE_ID eMessageID = static_cast<MESSAGE_ID>( GetIncrementedID(p_MessageID) );
	MessageTextsView.Add(
			p_MessageTextID[ GetIncrementedID(p_MessageTextID) ] +
			p_MessageID[static_cast<DWORD>(eMessageID)] +
			p_LanguageCode[m_eLanguageCode] +
			p_MessageText[MessageBytes]);

	return eMessageID;
}

//*****************************************************************************
void CDbBase::ChangeMessageText(
//Changes text of a message in database.
//
//Params:
	const MESSAGE_ID eMessageID,  //(in)	MessageID of message to change.  Current
										            //		language will be used to look up text.
	const WCHAR *pwczMessage)     //(in)	Message to write in Unicode.
{
	ASSERT(eMessageID > (MESSAGE_ID)0);
	ASSERT(IsOpen());

	//Find the message text record.
	c4_View MessageTextsView = m_pTextStorage->View("MessageTexts");
	const DWORD dwRowI = FindMessageText(eMessageID, MessageTextsView);
	if (dwRowI==ROW_NO_MATCH) {
      ASSERTP(false, "dwMessageID probably incorrect."); return;
   }

	//Because writes are expensive, check for an existing value that already
	//matches new value.
	{
		c4_Bytes OriginalMessageBytes = p_MessageText(MessageTextsView[dwRowI]);
		if (WCScmp(pwczMessage, (const WCHAR *) (OriginalMessageBytes.Contents()))==0)
			return; //New message is same as current, so nothing to do.
	}

	//Update message record.
	const UINT dwMessageLen = WCSlen(pwczMessage);
	c4_Bytes MessageBytes(pwczMessage, (dwMessageLen + 1)*sizeof(WCHAR));
	p_MessageText( MessageTextsView[ dwRowI ] ) = MessageBytes;
}

//*****************************************************************************
void CDbBase::DeleteMessage(
//Deletes all message texts (different languages) for one message ID.
//
//Params:
	const MESSAGE_ID eMessageID)	//(in)	MessageID of message text(s) to delete.
{
	c4_View MessageTextsView = m_pTextStorage->View("MessageTexts");

	const DWORD dwRowCount = MessageTextsView.GetSize();
	for (DWORD dwRowI = dwRowCount - 1L; dwRowI != (DWORD)(-1); --dwRowI)
	{
		if (eMessageID == (DWORD) (p_MessageID(MessageTextsView[dwRowI])))
			MessageTextsView.RemoveAt(dwRowI);
	}
}

//*****************************************************************************
DWORD CDbBase::GetIncrementedID(
//Gets the next incremented ID value for a property and increments the value
//in the appropriate database.
//
//Params:
	const c4_IntProp &propID)		//(in) One of the properties stored in IncrementedIDs table.
//
//Returns:
//The next ID which should not be in use.
{
	ASSERT(IsOpen());

	//Which database contains viewdef for this key field?
	c4_Storage *pStorage;
    int nPropID = propID.GetId();
	if (nPropID == p_PlayerID.GetId())
		pStorage = m_pPlayerStorage;
	else if (nPropID == p_MessageID.GetId() || nPropID == p_MessageTextID.GetId())
		pStorage = m_pTextStorage;
	else
		pStorage = m_pHoldStorage;

	c4_View IncrementedIDsView = pStorage->View("IncrementedIDs");
	DWORD dwID = propID(IncrementedIDsView[0]);

	//Set ID value in table to this ID that will now be considered in use.
	propID(IncrementedIDsView[0]) = ++dwID;

	return dwID;
}

//*****************************************************************************
void CDbBase::SetIncrementedID(
//Explicitly sets a next incremented ID value.  To avoid errors, use GetIncrementedID()
//unless you have special knowledge about how the incremented ID values should be set.
//
//Params:
	const c4_IntProp &propID,		//(in)  One of the properties stored in IncrementedIDs table.
    DWORD dwSetID)                  //(in)  Value to give property.
{
	ASSERT(IsOpen());
    ASSERT(dwSetID);

	//Which database contains viewdef for this key field?
	c4_Storage *pStorage;
    int nPropID = propID.GetId();
	if (nPropID == p_PlayerID.GetId())
		pStorage = m_pPlayerStorage;
	else if (nPropID == p_MessageID.GetId() || nPropID == p_MessageTextID.GetId())
		pStorage = m_pTextStorage;
	else
		pStorage = m_pHoldStorage;

    //Set ID value in table.
	c4_View IncrementedIDsView = pStorage->View("IncrementedIDs");
	propID(IncrementedIDsView[0]) = dwSetID;
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
	ASSERT(dwNewMessageTextLen == WCSlen(pwczNewMessageText));

	//Release current char buffer.
	delete[] this->pwczLastMessageText;
	this->pwczLastMessageText = NULL;

	//Allocate new char buffer.
	this->pwczLastMessageText = new WCHAR[dwNewMessageTextLen + 1];
	if (!this->pwczLastMessageText) return NULL;

	//Copy new text to char buffer.
	WCScpy(this->pwczLastMessageText, pwczNewMessageText);

	//Return char buffer pointer.
	return this->pwczLastMessageText;
}

//*****************************************************************************
bool CDbBase::BackupStorageFile(const WCHAR *wszDatFilepath)
{
    WSTRING wstrBackupFilepath = wszDatFilepath;
    WCv(wstrBackupFilepath[wstrBackupFilepath.size() - 1]) = '_';
    return ( CFiles::FileCopy(wszDatFilepath, wstrBackupFilepath.c_str()) &&
             CFiles::DoesFileExist(wstrBackupFilepath.c_str()) );
}

//*****************************************************************************
bool CDbBase::RestoreStorageFile(const WCHAR *wszDatFilepath)
{
    WSTRING wstrBackupFilepath = wszDatFilepath;
    WCv(wstrBackupFilepath[wstrBackupFilepath.size() - 1]) = '_';
    if (!CFiles::DoesFileExist(wstrBackupFilepath.c_str())) return false;  //No backup made.
    return CFiles::FileCopy(wstrBackupFilepath.c_str(), wszDatFilepath);
}

// $Log: DbBase.cpp,v $
// Revision 1.53  2005/03/15 21:51:11  mrimer
// Fixed memory leaks.
//
// Revision 1.52  2004/07/18 13:40:30  gjj
// Typo
//
// Revision 1.51  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.50  2003/12/01 19:11:01  mrimer
// Tweaking.
//
// Revision 1.49  2003/11/09 05:25:26  mrimer
// Made export release code more robust.
//
// Revision 1.48  2003/10/20 17:49:03  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.47  2003/10/08 17:31:15  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.46  2003/10/07 21:10:34  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.45  2003/10/06 16:05:40  erikh2000
// Added SetLanguage().
//
// Revision 1.44  2003/10/06 02:40:33  erikh2000
// When database is opened it is now checked for structure problems and restored from backup if needed.
//
// Revision 1.43  2003/09/16 19:02:13  schik
// Fixed memory leak
//
// Revision 1.42  2003/09/12 19:21:03  mrimer
// Returned Close() to the way it was before DB corruptions started occurring.
//
// Revision 1.41  2003/09/03 21:35:02  erikh2000
// Removed FlushWrites() since it was redundant with Commit().
//
// Revision 1.40  2003/08/19 20:09:34  mrimer
// Linux maintenance (committed on behalf of Gerry JJ).
//
// Revision 1.39  2003/08/12 22:52:56  erikh2000
// Fixed a buffer overrun error.
//
// Revision 1.38  2003/07/25 00:02:42  mrimer
// Returned Rollback() to the way it was before using GameStreams to access the .dat files.
// Added an assertion.
//
// Revision 1.37  2003/07/24 19:44:18  mrimer
// Fixed some incorrect DB access calls.
//
// Revision 1.36  2003/07/19 02:39:30  mrimer
// Made DB view access more robust and maintainable.
//
// Revision 1.35  2003/07/12 22:19:08  mrimer
// Made a temporary fix to open the db faster and use up 30+ MB less memory.  It doesn't use the Unicode file names.
//
// Revision 1.34  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.33  2003/07/09 21:37:44  mrimer
// Fixed a typo causing DROD to crash.
//
// Revision 1.32  2003/07/09 21:19:02  mrimer
// Moved PrimaryKeyMaps to CImportInfo (in new file).
// Made a temporary fix for Rollback().
//
// Revision 1.31  2003/07/06 04:54:50  mrimer
// Tweaking.
//
// Revision 1.30  2003/06/19 01:53:45  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.29  2003/05/28 23:04:54  erikh2000
// Added method to explicitly set incremented primary keys.
// CFiles::GetDatPath() is called differently.
//
// Revision 1.28  2003/05/25 22:46:25  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.27  2003/05/23 21:30:36  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.26  2003/05/22 23:39:01  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.25  2003/05/19 21:11:20  mrimer
// Some code maintenance.
//
// Revision 1.24  2003/05/13 01:10:24  erikh2000
// Revised includes to reduce dependencies.
//
// Revision 1.23  2003/05/09 02:43:20  mrimer
// Tweaked some ASSERTS to compile as __assume's for Release build.
//
// Revision 1.22  2003/05/08 23:19:13  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.21  2003/04/29 11:08:04  mrimer
// Changed some var types.
//
// Revision 1.20  2003/04/06 03:57:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
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
