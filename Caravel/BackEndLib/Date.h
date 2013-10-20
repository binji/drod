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
 *
 * ***** END LICENSE BLOCK ***** */

//Date.h
//Declarations for CDate.
//Class for handling dates with timezone-independant storage.

#ifndef DATE_H
#define DATE_H

#include "Types.h"
#include "Wchar.h"

#include <string>

#include <time.h>

using namespace std;

//Date-formatting flags.
const DWORD DF_LONG_DATE	= 1L;	//Use words to describe date.  If specified, DF_SHORT_DATE
									//is ignored.
const DWORD DF_SHORT_DATE	= 2L;	//Use numbers to describe date.
const DWORD DF_SHORT_TIME	= 4L;	//Show hours and minutes.

const UINT MONTH_COUNT = 12;

class CDate
{
private:
    static WSTRING  wstrMonthName[MONTH_COUNT];
    static bool     bIsClassInitialized;

public:
	CDate() {SetToNow();}
	CDate(const time_t tSet);
	CDate(const UINT nSetYear, const UINT nSetMonth, const UINT nSetDay,
         const  UINT nSetHour=0, const UINT nSetMinute=0, const UINT nSetSecond=0);
	~CDate() {}

    static void InitClass(const WCHAR *wszSetMonthNames[MONTH_COUNT]);

	time_t operator = (const time_t tSet) {SetGMT(tSet); return tSet;}
	operator time_t () const {return this->tGMTime;}
	bool	operator == (const time_t time) const {return this->tGMTime == time;}

	void	GetLocalFormattedText(const DWORD dwFormatFlags, WSTRING &wstrText) const;

	void	SetGMT(const time_t tSet);
	void	SetToNow();

private:
	time_t tGMTime;
};

#endif //...#ifndef DATE_H

//This class used to be called CDbDate and has a longer revision history with contributions from
//erikh2000, mrimer, schik, and trick.  See dbdate.cpp and dbdate.h in the attic 
//of drodlib for the full history.

// $Log: Date.h,v $
// Revision 1.1  2003/10/28 20:18:55  gjj
// Yet another rename try
//
// Revision 1.1  2003/10/20 17:47:42  erikh2000
// I guess the file got removed from the repository.  Adding it again.  Yikes.
//
// Revision 1.1  2003/07/16 21:18:30  mrimer
// Moved files from DRODLib.
//
// Revision 1.1  2003/10/05 20:15:37  erikh2000
// This is dbdate.cpp/h moved from drodlib, with some modifications.
//
