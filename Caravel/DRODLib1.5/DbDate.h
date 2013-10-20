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

//DbDate.h
//Declarations for CDbDate.
//Class for handling dates set and retrieved from database.

#ifndef DBDATE_H
#define DBDATE_H

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

class CDbDate
{
public:
	CDbDate(void) {SetToNow();}
	CDbDate(time_t tSet);
	CDbDate(UINT nSetYear, UINT nSetMonth, UINT nSetDay, UINT nSetHour=0, 
			UINT nSetMinute=0, UINT nSetSecond=0);
	~CDbDate(void) {}

	time_t operator = (time_t tSet) {SetGMT(tSet); return tSet;}
	operator time_t () {return tGMTime;}
	bool	operator == (time_t time) const {return tGMTime == time;}

	void	GetLocalFormattedText(DWORD dwFormatFlags, WSTRING &wstrText) const;

	void	SetGMT(time_t tSet);
	void	SetToNow(void);

private:
	time_t tGMTime;
};

#endif //...#ifndef DBDATE_H

// $Log: DbDate.h,v $
// Revision 1.1  2003/02/25 00:01:27  erikh2000
// Initial check-in.
//
// Revision 1.10  2003/02/17 00:48:11  erikh2000
// Remove L" string literals.
//
// Revision 1.9  2003/02/16 20:29:31  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.8  2002/12/22 01:39:54  mrimer
// Added equality operator.
//
// Revision 1.7  2002/06/13 22:49:00  erikh2000
// Changed GetLocalFormattedText() to return a safer wstring.
// Month names are loaded from the database.
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
