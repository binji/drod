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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//AttachableObject.h
//Base class for objects attachable to CCueEvents (see CueEvents.h).

#ifndef ATTACHABLEOBJECT_H
#define ATTACHABLEOBJECT_H

unsigned long GetAORefCount();

//*****************************************************************************
class CAttachableObject
{
public:
   CAttachableObject();
	virtual ~CAttachableObject();
};

//*****************************************************************************
template <typename T>
class CAttachableWrapper : public CAttachableObject
{
public:
	CAttachableWrapper(T data) : CAttachableObject(), data(data) { }
	virtual ~CAttachableWrapper() { }
	T data;
	operator T&() {
		return data;
	}
	operator T() const {
		return data;
	}
};

#endif //...#ifndef ATTACHABLEOBJECT_H

// $Log: AttachableObject.h,v $
// Revision 1.2  2003/08/22 02:45:42  mrimer
// Revised CAttachableObject usage.  Tweaked some code.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.3  2002/12/22 01:45:28  mrimer
// Added log comment.
//
