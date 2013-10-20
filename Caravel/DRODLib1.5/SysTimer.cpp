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

#include "Assert.h"
#include "Types.h"

//********************************************************************************
DWORD GetTicks(void)
{
#ifdef WIN32
	return GetTickCount();
#else
#	error System tick count code not provided.
	return 0L;
#endif
}

// $Log: SysTimer.cpp,v $
// Revision 1.1  2003/02/25 00:01:40  erikh2000
// Initial check-in.
//
// Revision 1.2  2002/07/20 23:03:00  erikh2000
// Revised #includes.
//
// Revision 1.1  2002/04/28 23:31:33  erikh2000
// Initial check-in.
//
