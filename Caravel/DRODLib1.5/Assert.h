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

//Assert.h
//Assertian checking and debugging info.

#ifndef ASSERT_H
#define ASSERT_H

#ifdef _DEBUG
#	define ASSERT(x) \
		if (x)                         \
			NULL;                   \
		else                          \
			AssertErr(__FILE__, __LINE__)
#else
#	if defined _MSC_VER
#		define ASSERT(x)  __assume(x)
#	else
#		define ASSERT(x)  NULL;
#	endif
#endif

//__assume is not used in VERIFY macro because it would cause statements
//to execute twice.
#ifdef _DEBUG	
#	define VERIFY(x) \
		if (x)			\
			NULL;	\
		else			\
			AssertErr(__FILE__, __LINE__)
#else
#	define VERIFY(x) x
#endif	
			
#ifdef _DEBUG
#	define LOGERR(x) LogErr(x)
#else
#	define LOGERR(x) NULL
#endif

#ifdef _DEBUG
#	define DEBUGPRINT(x)	DebugPrint(x)
#else
#	define DEBUGPRINT(x)	NULL
#endif

//Prototypes.
//Debug-mode.
#ifdef _DEBUG
void AssertErr(const char *pszFile, int nLine);
void LogErr(const char *pszMessage);
void DebugPrint(const char *pszMessage);
#endif

//Prevent accidental assigns and copies by declaring private methods in a class for 
//copy constructor and assign operator.
#define PREVENT_DEFAULT_COPY(ClassName) \
  private: \
    ClassName (const ClassName &Src); \
    ClassName &operator= (const ClassName &Src)

#endif //...#ifndef ASSERT_H

// $Log: Assert.h,v $
// Revision 1.1  2003/02/25 00:01:15  erikh2000
// Initial check-in.
//
// Revision 1.7  2002/10/09 22:24:23  erikh2000
// Fixed a few errors in the debug macros.
//
// Revision 1.6  2002/10/09 00:53:25  erikh2000
// Added __assume statement to release build version of ASSERT() macro.
//
// Revision 1.5  2002/07/05 17:56:09  mrimer
// Removed method bodies in PREVENT_DEFAULT_COPY macro.
//
// Revision 1.4  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.3  2002/02/25 03:36:17  erikh2000
// Added DEBUGPRINT macro and associated routine.
//
// Revision 1.2  2001/12/08 01:38:44  erikh2000
// Wrote PREVENT_DEFAULT_COPY() macro.
//
// Revision 1.1.1.1  2001/10/01 22:20:06  erikh2000
// Initial check-in.
//
