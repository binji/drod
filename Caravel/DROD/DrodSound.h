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
 * JP Burford (jpburford), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DrodSound.h
//Declarations for CDrodSound.
//Class for playing waves and music for DROD.

#ifndef DRODSOUND_H
#define DRODSOUND_H

#include <FrontEndLib/Sound.h>

//Song IDs.
enum SONGID 
{
	SONGID_NONE = SOUNDLIB::SONGID_NONE,
	SONGID_INTRO,
	SONGID_LEVELCOMPLETE,
	SONGID_WINLEVEL1,
	SONGID_WINLEVEL2,
	SONGID_GAME1,
	SONGID_GAME2,
	SONGID_GAME3,
	SONGID_GAME4,
	SONGID_GAME5,
	SONGID_GAME6,
	SONGID_GAME7,
	SONGID_GAME8,
	SONGID_GAME9,
	SONGID_WINGAME,
	SONGID_CREDITS,
	SONGID_ENDOFTHEGAME
};

//Sound effect IDs.  The sequence and values of the constants does not affect
//anything.  I have them arranged by the channel they will play on.  The 
//actual channel assignment occurs in LoadSoundEffects(), so if you
//rearrange or add to the SEIDs, LoadSoundEffects() should also be updated.  
//
//If a sample is played on a channel and another sample is already playing on that
//channel, the currently playing sample will stop.  Samples are grouped together 
//when: 
//1. There is a thing in the game that couldn't make two sounds at once, i.e. 
//   a person speaking, so it is natural to stop a current sample on the same 
//   channel.
//2. There is no case where the two samples would play at the same time.  
//
//Performance is better when less channels are used, but when in doubt, give a 
//sample its own channel.
enum SEID
{
	SEID_NONE = -1,

	SEID_BUTTON = SOUNDLIB::SEID_BUTTON,	//channel n+4
	SEID_READ = SOUNDLIB::SEID_READ,			//separate channel

	//Channel n+1--Beethro's voice.
	SEID_OOF,
	SEID_SCARED,
	SEID_DIE,
	SEID_CLEAR,
	SEID_TIRED,

	//Channel n+2--'Neather's voice.
	SEID_NLAUGHING,
	SEID_NFRUSTRATED,
	SEID_NSCARED,
	SEID_EVILEYEWOKE,

	//Channel n+3--Won't play at the same time.
	SEID_TITLEBUTTON,
	SEID_SPLAT,

	//Channel n+4--Won't play at the same time.
	SEID_ORBHIT,
	SEID_ORBHITMIMIC,

	//Channel n+5--Won't play at the same time.
	SEID_MIMIC,
	SEID_WALK,

	//Channel n+6--Won't play at the same time.
	SEID_POTION,
	SEID_SWING,

	//Each sound effect below gets a separate channel.
	SEID_COMPLETE,
	SEID_TRAPDOOR,
	SEID_CHECKPOINT,
	SEID_BREAKWALL,
	SEID_DOOROPEN,
	SEID_LASTBRAIN,
	SEID_STABTAR,

	SEID_COUNT
};

//Whenever a SEID is added above, recount the channels and update value below.
//Must exactly match or assertian will fire in LoadSoundEffects().
const UINT SAMPLE_CHANNEL_COUNT = 14;
const UINT MODULE_CHANNEL_COUNT = 16;

class CDrodSound : public CSound
{
public:
	CDrodSound(const bool bNoSound);
	virtual ~CDrodSound() {}

protected:
	virtual bool GetWaveFilepaths(const UINT eSEID, list<WSTRING> &FilepathList) const;
	virtual bool GetSongFilepath(const UINT nSongID, WSTRING &wstrFilepath);
	virtual bool LoadSoundEffects();
};

#endif //...#ifndef DRODSOUND_H

// $Log: DrodSound.h,v $
// Revision 1.3  2003/05/29 20:00:25  mrimer
// Now LoadSoundEffects() is called in derived class constructor instead of base to fix a bug.
//
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.1  2003/05/22 23:35:04  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
