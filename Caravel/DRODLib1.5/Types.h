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
 * Portions created by the Initial Developer are Copyright (C) 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//Type declarations.

#ifndef TYPES_H
#define TYPES_H

//Make sure I have the basics, like NULL.  I don't know how portable this 
//include is.
#	include <time.h>

#	ifndef _WINDOWS_ //If <windows.h> wasn't included.

typedef unsigned long		DWORD;
typedef unsigned int		UINT;
typedef unsigned char		BYTE;
typedef unsigned short		USHORT;

typedef struct tagPOINT
{
    long  x;
    long  y;
} POINT;

#	endif	//...#ifndef _WINDOWS_

typedef short			SHORT;

#endif //...#ifndef TYPES_H

// $Log: Types.h,v $
// Revision 1.1  2003/02/25 00:01:41  erikh2000
// Initial check-in.
//
// Revision 1.7  2003/02/17 00:48:12  erikh2000
// Remove L" string literals.
//
// Revision 1.6  2003/02/16 20:27:53  erikh2000
// Added WSTRING typedef.
//
// Revision 1.5  2002/07/20 23:04:49  erikh2000
// Changed Types.h so that it doesn't #include windows.h.  I want to be able to easily see the places where I am using Windows API declarations, and minimize dependencies.
//
