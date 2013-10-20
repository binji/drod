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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#ifdef WIN32
#	include <windows.h> //Should be first include.
#endif

#ifdef __sgi
#	include <sys/types.h>
#	include <sys/time.h>
#endif

#ifdef __linux__
#	include <sys/time.h>
#endif

#include "Assert.h"
#include "Types.h"

//********************************************************************************
DWORD GetTicks(void)
//NOTE: Might have overflow problems, so don't use for absolute dates down to the millisecond.
{
#ifdef WIN32
	return GetTickCount();
#elif defined(__sgi)
	return 0L;
#elif defined (__APPLE__)
	return clock();
#elif defined (__linux__)
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
#	error System tick count code not provided.
	return 0L;
#endif
}

// $Log: SysTimer.cpp,v $
// Revision 1.3  2003/06/15 04:19:15  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.2  2003/05/23 21:10:57  mrimer
// Added port to APPLE (on behalf of Ross Jones).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.3  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.2  2002/07/20 23:03:00  erikh2000
// Revised #includes.
//
// Revision 1.1  2002/04/28 23:31:33  erikh2000
// Initial check-in.
//
