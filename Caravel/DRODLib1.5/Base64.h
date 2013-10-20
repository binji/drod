
//*********************************************************************
//* C_Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*********************************************************************

#ifndef Base64_H
#define Base64_H

#include "Types.h"
#include "Wchar.h"

namespace Base64
{
	string encode(const string &data);
	string encode(const WSTRING &data);
	string encode(const unsigned char* data, const unsigned long dwDataSize);
	
	void decode(const string &data, string &returnvalue);
	void decode(const string &data, WSTRING &returnvalue);
	string decode(const string &data);
	unsigned long decode(const string &data, unsigned char* &returnvalue);
};

#endif

// $Log: Base64.h,v $
// Revision 1.1  2003/02/25 00:01:16  erikh2000
// Initial check-in.
//
// Revision 1.3  2003/02/17 00:48:11  erikh2000
// Remove L" string literals.
//
// Revision 1.2  2003/02/16 20:29:31  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.1  2002/12/22 01:46:10  mrimer
// Initial check-in.
//
