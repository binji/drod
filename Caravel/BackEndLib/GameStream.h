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
 * 1997, 2000, 2001, 2002, 2003 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Matt Schikore (Schik)
 *
 * ***** END LICENSE BLOCK ***** */

//GameStream.h
//Declarations for CGameStream.
//Class for reading from and writing to a file with a WCHAR* filename

#ifndef GAMESTREAM_H
#define GAMESTREAM_H
#ifdef WIN32
#	pragma warning(disable:4786)
#endif

#include <mk4.h>
#include <stdio.h>
#include "Wchar.h"

class CGameStream : public c4_Stream
{
public:
	CGameStream(const WCHAR* wszSetFilename, const char *pszOptions = "rb+");
	virtual ~CGameStream();

	virtual int Read(void* buffer, int size);
	virtual bool Write(const void* buffer, int size);

   virtual bool Save(c4_Storage* pStorage);

private:
	WCHAR* wszFilename;
	FILE*  pFile;
};

#endif // GAMESTREAM_H

// $Log: GameStream.h,v $
// Revision 1.3  2003/07/07 23:21:13  mrimer
// Replaced assertion with returning false on fail.
//
// Revision 1.2  2003/06/16 20:15:22  erikh2000
// Wrapped wfopen/fopen() and changed calls.
//
// Revision 1.1  2003/05/22 21:47:05  mrimer
// Initial check-in (files taken from DRODLib).
//
// Revision 1.2  2003/05/08 23:21:38  erikh2000
// Added optional file open param to constructor.
// Made destructor only close file if one was opened successfully.
//
// Revision 1.1  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
