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

#ifndef GAMECONSTANTS_H
#define GAMECONSTANTS_H

#include <BackEndLib/Types.h>
#include <BackEndLib/Wchar.h>

//Current game version.  Update as needed.
const DWORD dwCurrentDRODVersion = 160;	//1.6.0
const char szDROD[] = "drod";
const WCHAR wszDROD[] = { W_t('d'), W_t('r'), W_t('o'), W_t('d'), W_t(0) };
const char szDROD_VER[] = "1_6";
const WCHAR wszDROD_VER[] = { W_t('1'), W_t('_'), W_t('6'), W_t(0) };

//Swordsman commands.
#define CMD_UNSPECIFIED 0
#define CMD_N         1
#define CMD_NE		 2
#define CMD_W	     3
#define CMD_E         5
#define CMD_SW      6
#define CMD_S          7
#define CMD_SE       8
#define CMD_C        9
#define CMD_CC      10
#define CMD_NW      11
#define CMD_WAIT   12
//CMD_ESC 13--removed, but keep space for now.
#define CMD_RESTART   14
#define CMD_YES       15
#define CMD_NO        16
#define COMMAND_COUNT 17

//Sword orientation.
const UINT NW = 0;
const UINT N = 1;
const UINT NE = 2;
const UINT W = 3;
const UINT E = 5;
const UINT SW = 6;
const UINT S = 7;
const UINT SE = 8;
const UINT NO_ORIENTATION = 4;
const UINT ORIENTATION_COUNT = 9;
#define IsValidOrientation(o) ((o)>=0 && (o)<ORIENTATION_COUNT)

//Used to determine if a point is inside a rect.
#define IsInRect(x,y,left,top,right,bottom)	 ( (x)>=(left) && (x)<=(right) && (y)>=(top) && (y)<=(bottom) )
 
//Gets the next orientation# for a sword moving clockwise.
#define nNextCO(o)       ( (((o)==0 || (o)==1) * ((o)+1)) + (((o)==2 || (o)==5) * ((o)+3)) + (((o)==7 || (o)==8) * ((o)-1)) + (((o)==3 || (o)==6) * ((o)-3)) )

//Gets the next orientation# for a sword moving counter-clockwise.
#define nNextCCO(o)     ( (((o)==0 || (o)==3) * ((o)+3)) + (((o)==5 || (o)==8) * ((o)-3)) + (((o)==6 || (o)==7) * ((o)+1)) + (((o)==1 || (o)==2) * ((o)-1)) )

//Get the orientation# from the relative sword coordinates.  Where ox and oy range from -1 to 1.
#define nGetO(ox,oy)                             ( ((oy + 1) * 3) + (ox + 1) )

//Get relative horizontal sword position from orientation#.
#define nGetOX(o)                                 ( ((o) % 3) - 1 )

//Get relative vertical sword position from orientation#.
#define nGetOY(o)                                 ( ((o) / 3) - 1 )

//Returns sign of a number.
#define sgn(x)                ( (((int)(x)<0)*-1) + ((int)(x)>0) ) 

#ifndef __max
#define __max(x,y) ((x)>(y)?(x):(y))
#endif

//Returns distance between two points.
#define nDist(x1, y1, x2, y2)                   (__max(abs((int)((x1)-(x2))) , abs((int)((y1)-(y2)))) )

#endif //...#ifndef GAMECONSTANTS_H

// $Log: GameConstants.h,v $
// Revision 1.17  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.16  2003/05/24 02:07:21  mrimer
// Fixes for APPLE portability (committed on behalf of Ross Jones).
//
// Revision 1.15  2003/05/23 21:30:35  mrimer
// Altered the #includes for files from BackEndLib (from "file.h" to <file.h>).
//
// Revision 1.14  2003/05/22 23:39:00  mrimer
// Modified to use new BackEndLib.
//
// Revision 1.13  2003/05/08 23:24:11  erikh2000
// Changed version string to 1.6.
//
// Revision 1.12  2003/04/06 03:57:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.11  2003/02/24 17:03:57  erikh2000
// Added version string.
//
// Revision 1.10  2002/12/22 01:25:30  mrimer
// Changed DROD version to 1.6.
//
// Revision 1.9  2002/11/13 23:19:14  mrimer
// Added CurrentDRODVersion.
//
// Revision 1.8  2002/07/10 03:57:08  erikh2000
// Removed some unused constants and macros.
// Fixed a compilation error involving a namespace conflict with orientation constants.
//
// Revision 1.7  2002/04/12 05:07:42  erikh2000
// Added CMD_UNSPECIFIED and removed CMD_ESC.
//
// Revision 1.6  2002/03/05 01:54:10  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.5  2001/12/16 02:13:36  erikh2000
// Added CMD_YES and CMD_NO.
//
// Revision 1.4  2001/10/21 00:33:41  erikh2000
// Twiddling.
//
// Revision 1.3  2001/10/20 05:43:23  erikh2000
// Added ORIENTATION_COUNT constant.
//
// Revision 1.2  2001/10/16 00:02:31  erikh2000
// Wrote code to display current room and handle basic swordsman movement.
//
// Revision 1.1.1.1  2001/10/01 22:20:15  erikh2000
// Initial check-in.
//
