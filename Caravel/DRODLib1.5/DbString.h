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

#ifndef DBMESSAGETEXT_H
#define DBMESSAGETEXT_H

using namespace std;

#include "Types.h"
#include "DbBase.h"

class CDbMessageText : public CDbBase
{
public:
	CDbMessageText(DWORD dwSetMessageID = 0L) 
	{
		Bind(dwSetMessageID);
	}
	
	~CDbMessageText(void)
	{
		Flush();
	}
	
	void		Bind(DWORD dwSetMessageID) 
	{
		this->dwMessageID = dwSetMessageID;
		this->bIsLoaded = false;
	}
	
	const WCHAR * operator = (const WCHAR * wczSet) 
	{
		wstrText = wczSet;
		this->bIsDirty = true;
	}
	
	operator const WCHAR *() 
	{
		if (!this->bIsLoaded)
		{
			ASSERT(this->dwMessageID);
			this->wstrText = GetMessageText(this->dwMessageText);
			this->bIsLoaded = true;
		}
		return this->wstrText.begin();
	}
	
	void Flush(void)
	{
		if (this->bIsDirty)
		{
			if (this->dwMessageID)
				ChangeMessageText(this->dwMessageID, this->wstrText.begin());
			else
				this->dwMessageID = AddMessageText(this->wstrText.begin());
		}
	}

private:
	DWORD		dwMessageID;
	WSTRING		wstrText;
	bool		bIsLoaded;
	bool		bIsDirty;
};

#endif //...#ifndef DBMESSAGETEXT_H