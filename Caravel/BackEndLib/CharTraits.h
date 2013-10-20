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

#ifndef CHARTRAITS_H
#define CHARTRAITS_H

#include <ext/pod_char_traits.h>  // From libstdc++ CVS -- put in ../ext

using namespace std;
using namespace __gnu_cxx;

// Some useful extras
template<typename T, typename I, typename U>
static inline bool operator==(character<T,I> lhs, U rhs)
	{ return lhs.value == rhs; }

template<typename T, typename I>
static inline bool operator!=(character<T,I> lhs, character<T,I> rhs)
	{ return lhs.value != rhs.value; }

template<typename T, typename I, typename U>
static inline bool operator!=(character<T,I> lhs, U rhs)
	{ return lhs.value != rhs; }

template<typename T, typename I, typename U>
static inline bool operator>=(character<T,I> lhs, U rhs)
	{ return lhs.value >= rhs; }

template<typename T, typename I, typename U>
static inline bool operator<=(character<T,I> lhs, U rhs)
	{ return lhs.value <= rhs; }

template<typename T, typename I, typename U>
static inline bool operator>(character<T,I> lhs, U rhs)
	{ return lhs.value > rhs; }

template<typename T, typename I, typename U>
static inline bool operator<(character<T,I> lhs, U rhs)
	{ return lhs.value < rhs; }

template<typename T, typename I>
static inline bool operator&&(character<T,I> lhs, bool rhs)
	{ return lhs.value && rhs; }

template<typename T, typename I>
static inline bool operator!(character<T,I> lhs)
	{ return !lhs.value; }

typedef unsigned short          WCHAR_t;
typedef character<WCHAR_t, int> WCHAR; //wc, 16-bit UNICODE character

#define WCv(x) (x).value
#define pWCv(x) (x)->value

#endif

// $Log: CharTraits.h,v $
// Revision 1.6  2004/07/18 13:31:25  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.5  2003/08/09 00:37:46  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/07/27 21:56:24  erikh2000
// Updated to use non-local include path to pod_char_traits.h.
//
// Revision 1.3  2003/07/24 19:49:47  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.2  2003/07/22 19:37:22  mrimer
// Linux port fixes (committted on behalf of Gerry JJ).
//
// Revision 1.1  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
