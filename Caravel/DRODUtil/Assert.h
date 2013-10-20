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

//Assertion checking and debugging info.

#ifndef ASSERT_H
#define ASSERT_H

#define _ASSERT_VOID_CAST(exp)   static_cast<void>(exp)

#ifdef _DEBUG
#   define ASSERT(exp)          _ASSERT_VOID_CAST( (exp) ? 0 : AssertErr(__FILE__,__LINE__,#exp) )
#   define ASSERTP(exp, desc)   _ASSERT_VOID_CAST( (exp) ? 0 : AssertErr(__FILE__,__LINE__,(desc)) )
#else
#   define ASSERT(exp)
#   define ASSERTP(exp,desc)    
#endif

#ifdef _DEBUG
#	define VERIFY(x) \
		if (!(x)) AssertErr(__FILE__, __LINE__,#x)
#else
#  define VERIFY(x) x
#endif

//Prototypes.
#ifdef _DEBUG
void 		AssertErr(char *, int, char *);
#endif

#endif ///..#ifndef ASSERT_H

// $Log: Assert.h,v $
// Revision 1.6  2004/08/08 21:46:15  mrimer
// Parameter fix (committed on behalf of erikh2000).
//
// Revision 1.5  2003/10/06 02:48:36  erikh2000
// Added descriptions to assertians.
//
// Revision 1.4  2003/07/24 18:01:33  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2002/05/15 13:09:25  mrimer
// Added log macro to end of file.
//
