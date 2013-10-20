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

#include <string>
using std::string;

#define RELEASE_WITH_DEBUG_INFO //Uncomment for assert messages in release builds.
#undef USE_LOGCONTEXT

#ifdef RELEASE_WITH_DEBUG_INFO
#   ifndef _DEBUG
#       define _DEBUG
#   endif
#endif

//Assertian-reporting macros.  Avoid simply calling ASSERT(false) because after changes to source code,
//the file and line# may not be sufficient to track the location in current source of the assertian.  Use
//either ASSERT(expression) or ASSERTP(false, "Description of error.").
#define _ASSERT_VOID_CAST(exp)   static_cast<void>(exp)
#ifdef _DEBUG
# ifdef WIN32
#   define ASSERT(exp)          _ASSERT_VOID_CAST( (exp) ? 0 : AssertErr(__FILE__,__LINE__,#exp) )
#   define ASSERTP(exp, desc)   _ASSERT_VOID_CAST( (exp) ? 0 : AssertErr(__FILE__,__LINE__,(desc)) )
# else
#   define ASSERT(exp)          if (!(exp)) AssertErr(__FILE__,__LINE__,#exp)
#   define ASSERTP(exp, desc)   if (!(exp)) AssertErr(__FILE__,__LINE__,(desc))
# endif
#else
#   define ASSERT(exp)
#   define ASSERTP(exp,desc)
#endif

//Log context is applied to error logging to show a history of what happened before an error.
//When CLogText() is is instanced, error logging will show "BEGIN The task", and when it goes out
//of scope, error logging will show "END The task".  These messages are only shown when an error
//occurs.
#if defined(USE_LOGCONTEXT) && defined(_DEBUG)
    #define LOGCONTEXT(desc) CLogContext _LogContext((desc))

    class CLogContext
    {
    public:
        string strDesc;
        CLogContext(const char *pszDesc);
        ~CLogContext(void);
    };
#else
    #define LOGCONTEXT(desc)
#endif

//Use for dynamic casts so that debug-builds will show failed casts.
#ifdef _DEBUG
#	define DYN_CAST(totype,fromtype,source) \
	DynCastAssert<totype,fromtype>(source, __FILE__, __LINE__)
#else
#	define DYN_CAST(totype,fromtype,source) \
	dynamic_cast<totype>(source)
#endif

//Use to assert on a statement that should execute in retail as well as debug builds.
#ifdef _DEBUG
#	define VERIFY(exp) \
		if (!(exp)) AssertErr(__FILE__, __LINE__,#exp)
#else
#	define VERIFY(x) exp
#endif	

//Log an error to the error log.
#ifdef _DEBUG
#	define LOGERR(x) LogErr(x)
#else
#	define LOGERR(x)
#endif

//Show a message in debug output.
#ifdef _DEBUG
#	define DEBUGPRINT(x)	DebugPrint(x)
#else
#	define DEBUGPRINT(x)
#endif

//Prototypes.
//Debug-mode.
#ifdef _DEBUG
    void AssertErr(const char *pszFile, int nLine, const char *pszDesc);
    void LogErr(const char *pszMessage);
    void DebugPrint(const char *pszMessage);

    template<typename To_p, typename From_p>
    inline To_p DynCastAssert(From_p pSource, const char *pszFile, int nLine)
    {
	    To_p pTarget = dynamic_cast<To_p>(pSource);
	    if (pSource && !pTarget) AssertErr(pszFile, nLine, "Dynamic cast failed.");
	    return pTarget;
    }
#endif

//Prevent accidental assigns and copies by declaring private methods in a class for
//copy constructor and assign operator.
#define PREVENT_DEFAULT_COPY(ClassName) \
  private: \
    ClassName (const ClassName &Src); \
    ClassName &operator= (const ClassName &Src)

#endif //...#ifndef ASSERT_H

// $Log: Assert.h,v $
// Revision 1.16  2003/11/09 05:20:54  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.15  2003/10/08 17:32:24  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.14  2003/10/07 21:09:49  erikh2000
// Added context information to logging.
//
// Revision 1.13  2003/10/06 02:36:41  erikh2000
// Asserts will now log a more detailed error description.
//
// Revision 1.12  2003/08/09 00:37:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/07/31 23:21:34  erikh2000
// Added NULL instruction to release-build ASSERT definition to avoid compiler warnings.
//
// Revision 1.10  2003/07/30 23:56:26  mrimer
// Linux port fix.
//
// Revision 1.9  2003/07/29 16:26:58  erikh2000
// Fixed bug causing crashes in release builds when assertians failed.
//
// Revision 1.8  2003/07/27 21:57:11  erikh2000
// Commented out debug message define.  Will keep this way for rest of beta unless something comes up.
//
// Revision 1.7  2003/07/16 08:07:57  mrimer
// Added DYN_CAST macro to report dynamic_cast failures.
//
// Revision 1.6  2003/06/28 20:08:59  erikh2000
// Revised conditional compilation for debug messages in release builds.
//
// Revision 1.5  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/28 02:34:10  erikh2000
// Added temporay #define _DEBUG for beta builds.
//
// Revision 1.3  2003/06/18 23:55:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/06/15 04:19:14  mrimer
// Added linux compatibility (comitted on behalf of trick).
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
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
