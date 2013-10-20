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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DbDate.cpp
//Implementation of CDbDate.

#include "DbDate.h"
#include "DbMessageText.h"

//
//CDbDate public methods.
//

//****************************************************************************************
CDbDate::CDbDate(
//Constructor that takes time_t value to specify date/time object will hold.
//
//Params:
	time_t tSet)	//(in) GMT date/time to set this object to.
{
	SetGMT(tSet);
}

//****************************************************************************************
CDbDate::CDbDate(
//Constructor that takes individual values to specify date/time object will hold.
//
//Params:
	UINT wSetYear,		//(in) 4-digit year.			All of these are GMT.
	UINT wSetMonth,		//(in) 1-12							.
	UINT wSetDay,		//(in) 1-31							.
	UINT wSetHour,		//(in) 0-23. 0 is default.			.
	UINT wSetMinute,	//(in) 0-59. 0 is default.			.
	UINT wSetSecond)	//(in) 0-59. 0 is default.			.
{
	struct tm tmSet;

	ASSERT(wSetYear >= 1970 && wSetYear <= 9999);
	ASSERT(wSetMonth >=1 && wSetMonth <= 12);
	ASSERT(wSetDay >= 1 && wSetDay <= 31);
	ASSERT(wSetHour <= 23);
	ASSERT(wSetMinute <= 59);
	ASSERT(wSetSecond <= 59);

	tmSet.tm_year = (int) wSetYear - 1900;
	tmSet.tm_mon = (int) wSetMonth - 1;
	tmSet.tm_mday = (int) wSetDay;
	tmSet.tm_hour = (int) wSetHour;
	tmSet.tm_min = (int) wSetMinute;
	tmSet.tm_sec = (int) wSetSecond;
	tmSet.tm_yday = 0;
	tmSet.tm_wday = 0;
	tmSet.tm_isdst = 0;

	time_t tSet = mktime(&tmSet);
	SetGMT(tSet);
}

//****************************************************************************************
void CDbDate::SetToNow(void)
//Set time to now.
{
	time(&(this->tGMTime));
}

//****************************************************************************************
void CDbDate::SetGMT(
//Sets date/time for this object from GMT values.
//
//Params:
	time_t tSetGMT) //(in) GMT date/time.
{
	this->tGMTime = tSetGMT;
}

//****************************************************************************************
void CDbDate::GetLocalFormattedText(
//Get formatted date/time with conversion to local time zone.
//
//Params:
	DWORD dwFormatFlags,	//(in)		One or more DF_* flag constants.
	WSTRING &wstrText)		//(in/out)	Accepts an empty or non-empty string to which
							//			text will be appended.
const
{
	static const MESSAGE_ID dwMonthMID[12] = {MID_January, MID_February, MID_March, 
			MID_April, MID_May, MID_June, MID_July, MID_August, MID_September, 
			MID_October, MID_November, MID_December};
	WCHAR dummy[20];

	_tzset();
	struct tm *tmGet = localtime(&this->tGMTime);	//convert to local time
	bool bShowDate = false;

	if ((dwFormatFlags & DF_LONG_DATE) == DF_LONG_DATE)
	{
		wstrText += (const WCHAR *)CDbMessageText(dwMonthMID[tmGet->tm_mon]);
		wstrText += L" ";
		wstrText += _itow(tmGet->tm_mday,dummy,10);
		wstrText += L", ";
		wstrText += _itow(1900 + tmGet->tm_year,dummy,10);
		bShowDate = true;
	} else if ((dwFormatFlags & DF_SHORT_DATE) == DF_SHORT_DATE)
	{
		const int nYear = (1900 + tmGet->tm_year) % 100;
		wstrText += _itow(tmGet->tm_mon + 1,dummy,10);
		wstrText += L"/";
		wstrText += _itow(tmGet->tm_mday,dummy,10);
		wstrText += L"/";
		if (nYear < 10)
			wstrText += L"0";
		wstrText += _itow(nYear,dummy,10);
		bShowDate = true;
	}

	if ((dwFormatFlags & DF_SHORT_TIME) == DF_SHORT_TIME)
	{
		WCHAR ampm[3] = L"AM";
		if (bShowDate)
			wstrText += L" ";

		if (tmGet->tm_hour > 12)
    {
			wcscpy(ampm, L"PM");
			tmGet->tm_hour -= 12;
    }
		if (tmGet->tm_hour == 0)  // Adjust if midnight hour.
			tmGet->tm_hour = 12;

		wstrText += _itow(tmGet->tm_hour,dummy,10);
		wstrText += L":";
		if (tmGet->tm_min < 10)
			wstrText += L"0";
		wstrText += _itow(tmGet->tm_min,dummy,10);
		wstrText += ampm;
	}
}

// $Log: DbDate.cpp,v $
// Revision 1.1  2003/02/25 00:01:27  erikh2000
// Initial check-in.
//
// Revision 1.14  2003/02/16 20:29:31  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.13  2002/10/22 05:28:07  mrimer
// Revised includes.
//
// Revision 1.12  2002/10/10 03:23:26  erikh2000
// Fixed error in short-date formatting.
//
// Revision 1.11  2002/10/04 02:21:24  mrimer
// Fixed short month formatting error.
//
// Revision 1.10  2002/06/15 18:25:59  erikh2000
// Made month name load from database on each call to prevent an error.
//
// Revision 1.9  2002/06/13 22:49:00  erikh2000
// Changed GetLocalFormattedText() to return a safer wstring.
// Month names are loaded from the database.
//
// Revision 1.8  2002/06/13 21:39:00  mrimer
// Added call to _tzset().  Removed delete(tmGet).
//
// Revision 1.7  2002/06/12 18:21:04  mrimer
// Implemented GetLocalFormattedText().
//
// Revision 1.6  2002/05/15 01:19:24  erikh2000
// Added GetLocalFormattedText() stub.
//
// Revision 1.5  2002/04/28 23:40:53  erikh2000
// Revised #includes.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2002/02/07 22:31:42  erikh2000
// Added CDbDate::SetToNow().
//
// Revision 1.2  2001/12/08 01:40:20  erikh2000
// Changed default constructor to set date/time to now.
//
// Revision 1.1.1.1  2001/10/01 22:20:08  erikh2000
// Initial check-in.
//
