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
 * JP Burford (jpburford), John Wm. Wicks (j_wicks), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//DrodSound.cpp
//Implementation of CDrodSound class.

#include "DrodSound.h"
#include <BackEndLib/Files.h>

//*****************************************************************************
CDrodSound::CDrodSound(const bool bNoSound)
: CSound(bNoSound, SEID_COUNT, ::SAMPLE_CHANNEL_COUNT, ::MODULE_CHANNEL_COUNT)
{	
	//Load sound effects ahead of time so that playing a sample takes less time.
	if (this->bSoundEffectsAvailable)
		this->bSoundEffectsAvailable = LoadSoundEffects();
}

//*****************************************************************************
bool CDrodSound::GetWaveFilepaths(
//Gets full path to one or more wave files for a sound effect.
//
//Params:
	const UINT eSEID,				//(in)	One of the SEID_* constants.
	list<WSTRING> &FilepathArray)	//(out)	List of full paths to one or more wave files.
//
//Returns:
//True if successful, false if not.
const
{
	FilepathArray.clear();

	//Get INI key name that corresponds to wave ID.
	string strKeyName;
	switch (eSEID)
	{
		case SEID_SPLAT:		strKeyName="Splat"; break;
		case SEID_DIE:			strKeyName="Die"; break;
		case SEID_SWING:		strKeyName="Swing"; break;
		case SEID_WALK:			strKeyName="Walk"; break;
		case SEID_OOF:			strKeyName="Oof"; break;
		case SEID_CLEAR:		strKeyName="Clear"; break;
		case SEID_ORBHIT:		strKeyName="OrbHit"; break;
		case SEID_ORBHITMIMIC:		strKeyName="OrbHitMimic"; break;
		case SEID_READ:			strKeyName="Read"; break;
		case SEID_SCARED:		strKeyName="Scared"; break;
		case SEID_COMPLETE:		strKeyName="Complete"; break;
		case SEID_MIMIC:		strKeyName="Mimic"; break;
		case SEID_POTION:		strKeyName="Potion"; break;
		case SEID_TRAPDOOR:		strKeyName="Trapdoor";break;
		case SEID_CHECKPOINT:	strKeyName="Checkpoint"; break;
		case SEID_TITLEBUTTON:	strKeyName="TitleButton"; break;
		case SEID_BUTTON:		strKeyName="Button"; break;
		case SEID_BREAKWALL:	strKeyName="BreakWall"; break;
		case SEID_DOOROPEN:		strKeyName="DoorOpen"; break;
		case SEID_LASTBRAIN:	strKeyName="LastBrain"; break;
		case SEID_STABTAR:		strKeyName="StabTar"; break;
		case SEID_TIRED:		strKeyName="Tired"; break;
		case SEID_NLAUGHING:	strKeyName="NLaughing"; break;
		case SEID_NFRUSTRATED:	strKeyName="NFrustrated"; break;
		case SEID_NSCARED:		strKeyName="NScared"; break;
		case SEID_EVILEYEWOKE:	strKeyName="EvilEyeWoke"; break;
		default: return false;
	}

	//Retrieve song filename(s) from INI file.
	string strFilenameList;
    CFiles Files;
	if (!Files.GetGameProfileString("Waves", strKeyName.c_str(), strFilenameList))
		return false;

	static const WCHAR wcszSounds[] = { W_t(SLASH), W_t('S'), W_t('o'), W_t('u'), W_t('n'), W_t('d'), W_t('s'), W_t(SLASH), W_t(0) };

	WSTRING wstrFilepath;
	const char *pszStartOfFilename = strFilenameList.c_str();
	WSTRING wstrStartOfFilename;
	for(char *pszSeek = &(*strFilenameList.begin()); *pszSeek != '\0'; ++pszSeek)
	{
		if (*pszSeek == ';') //Found end of filename.
		{
			*pszSeek = '\0'; //Null-terminate the filename.

			//Concatenate data path and filename.
            CFiles Files;
			wstrFilepath = Files.GetResPath();
			wstrFilepath += wcszSounds;
			AsciiToUnicode(pszStartOfFilename, wstrStartOfFilename);
			wstrFilepath += wstrStartOfFilename;
			FilepathArray.push_back(wstrFilepath);

			pszStartOfFilename = pszSeek + 1;
		}
	}

	//Concatenate data path and last filename.
	wstrFilepath = Files.GetResPath();
	wstrFilepath += wcszSounds;
	AsciiToUnicode(pszStartOfFilename, wstrStartOfFilename);
	wstrFilepath += wstrStartOfFilename;
	FilepathArray.push_back(wstrFilepath);

	return true;
}

//***********************************************************************************
bool CDrodSound::GetSongFilepath(
//Gets full path to song file.
//
//Params:
	const UINT eSongID,	//(in)	One of the SONGID_* constants.
	WSTRING &wstrFilepath)	//(out)	Returns full path to song file.
//
//Returns:
//True if successful, false if not.
{
	//Get INI key name that corresponds to song ID.
	string strKeyName;
	switch (eSongID)
	{
		case SONGID_INTRO:			strKeyName = "Intro"; break;
		case SONGID_LEVELCOMPLETE:	strKeyName = "LevelComplete"; break;
		case SONGID_WINLEVEL1:		strKeyName = "WinLevel1"; break;
		case SONGID_WINLEVEL2:		strKeyName = "WinLevel2"; break;
		case SONGID_GAME1:			strKeyName = "Game1"; break;
		case SONGID_GAME2:			strKeyName = "Game2"; break;
		case SONGID_GAME3:			strKeyName = "Game3"; break;
		case SONGID_GAME4:			strKeyName = "Game4"; break;
		case SONGID_GAME5:			strKeyName = "Game5"; break;
		case SONGID_GAME6:			strKeyName = "Game6"; break;
		case SONGID_GAME7:			strKeyName = "Game7"; break;
		case SONGID_GAME8:			strKeyName = "Game8"; break;
		case SONGID_GAME9:			strKeyName = "Game9"; break;
		case SONGID_WINGAME:		strKeyName = "WinGame"; break;
		case SONGID_CREDITS:		strKeyName = "Credits"; break;
		case SONGID_ENDOFTHEGAME:	break; //Not looked up from DROD.ini.
		default: return false;
	}

	//Retrieve song filename from INI file.
    CFiles Files;
	string strFilename;
	if (eSongID == SONGID_ENDOFTHEGAME)
        //Hard-coded instead of from INI because user customizing this song would screw up 
        //end of the game.
		strFilename = "theendofthegame.s3m";
	else
	{
		if (!Files.GetGameProfileString("Songs", strKeyName.c_str(), strFilename))
			return false;
	}

	WSTRING wstrFilename;
	AsciiToUnicode(strFilename.c_str(), wstrFilename);

	static const WCHAR wszMusic[] = { W_t(SLASH), W_t('M'), W_t('u'), W_t('s'), W_t('i'), W_t('c'), W_t(SLASH), W_t(0) };

	//Concatenate data path and filename.
	wstrFilepath = Files.GetResPath();
	wstrFilepath += wszMusic;
	wstrFilepath += wstrFilename;

	return true;
}

//**********************************************************************************
bool CDrodSound::LoadSoundEffects(void)
//Load sound effects into an array.
//
//Returns:
//True if at least one sound effect loaded, false if not.
{
#ifdef __sgi
	return false;
#else
	bool bOneSoundEffectLoaded = false;

	//Sound effect channels come after module channels.
	int nSoundEffectChannel = MODULE_CHANNEL_COUNT;
	list<WSTRING> FilepathArray;

	//For readability--macro that loads one sound effect.
#	define SHARED_CHANNEL_SOUNDEFFECT(s) \
	GetWaveFilepaths((s), FilepathArray); \
	this->SoundEffectArray[(s)].Load(FilepathArray, nSoundEffectChannel, true)

	//Channel n+1--Beethro's voice.	
	SHARED_CHANNEL_SOUNDEFFECT( SEID_OOF );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_SCARED );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_DIE );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_CLEAR );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_TIRED );

	//Channel n+2--'Neather's voice.
	++nSoundEffectChannel;
	SHARED_CHANNEL_SOUNDEFFECT( SEID_NLAUGHING );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_NFRUSTRATED );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_NSCARED );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_EVILEYEWOKE );

	//Channel n+3--Won't play at the same time.
	++nSoundEffectChannel;
	SHARED_CHANNEL_SOUNDEFFECT( SEID_TITLEBUTTON );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_SPLAT );

	//Channel n+4--Won't play at the same time.
	++nSoundEffectChannel;
	SHARED_CHANNEL_SOUNDEFFECT( SEID_BUTTON );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_ORBHIT );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_ORBHITMIMIC );

	//Channel n+5--Won't play at the same time.
	++nSoundEffectChannel;
	SHARED_CHANNEL_SOUNDEFFECT( SEID_MIMIC );
	GetWaveFilepaths(SEID_WALK, FilepathArray);
	this->SoundEffectArray[SEID_WALK].Load(FilepathArray, nSoundEffectChannel, false);

	//Channel n+6--Won't play at the same time.
	++nSoundEffectChannel;
	SHARED_CHANNEL_SOUNDEFFECT( SEID_POTION );
	SHARED_CHANNEL_SOUNDEFFECT( SEID_SWING );

#	undef SHARED_CHANNEL_SOUNDEFFECT

//Macro for readability.
#	define PRIVATE_CHANNEL_SOUNDEFFECT(s) \
	GetWaveFilepaths((s), FilepathArray); \
	this->SoundEffectArray[(s)].Load(FilepathArray, ++nSoundEffectChannel)

	//Each sound effect below gets a separate channel.
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_READ );
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_COMPLETE );
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_TRAPDOOR );
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_CHECKPOINT );
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_BREAKWALL );
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_DOOROPEN );
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_LASTBRAIN );
	PRIVATE_CHANNEL_SOUNDEFFECT( SEID_STABTAR );

#	undef PRIVATE_CHANNEL_SOUNDEFFECT

	//If this assertian fires, the SAMPLE_CHANNEL_COUNT constant has probably
	//not been updated to reflect a change in the number of required channels for
	//samples.
   ASSERT(static_cast<UINT>(nSoundEffectChannel + 1) == CHANNEL_COUNT);

	//Reserve the sample channels so that FMOD doesn't use them for playing
	//songs.
	for (UINT nReserveChannel = MODULE_CHANNEL_COUNT;
			nReserveChannel < CHANNEL_COUNT;	++nReserveChannel)
	{
		if (!FSOUND_SetReserved(nReserveChannel, true)) ASSERTP(false, "FSOUND_SetReserved() failed.");
	}

	//Check sound effects to see what got loaded.
	for (int nSEI = 0; nSEI < SEID_COUNT; ++nSEI)
	{
		if (this->SoundEffectArray[nSEI].IsLoaded())
			bOneSoundEffectLoaded = true;
		else
      {
			//If this fires, check 1. a load macro is present for each
			//SEID_* value. 2. filenames specified in DROD.INI are correct.
         CFiles f;
         f.AppendErrorLog("A sound effect failed to load.  Check whether "
            "the filenames specified in DROD.INI are correct.\r\n");
			ASSERTP(false, "Sound effect failed to load.");
      }
	}
	return bOneSoundEffectLoaded;
#endif
}

// $Log: DrodSound.cpp,v $
// Revision 1.13  2003/10/06 02:45:07  erikh2000
// Added descriptions to assertions.
//
// Revision 1.12  2003/08/09 00:38:53  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.11  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.10  2003/07/10 15:55:00  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/07/05 02:33:51  mrimer
// Put in some diagnostic error messages when sounds/music aren't loaded.
//
// Revision 1.8  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.7  2003/06/26 11:44:15  schik
// VS.NET compatibility fixes
//
// Revision 1.6  2003/05/29 20:00:24  mrimer
// Now LoadSoundEffects() is called in derived class constructor instead of base to fix a bug.
//
// Revision 1.5  2003/05/28 23:13:11  erikh2000
// Methods of CFiles are called differently.
//
// Revision 1.4  2003/05/26 14:56:01  mrimer
// Changed methods calls to static class calls.
//
// Revision 1.3  2003/05/26 01:56:18  erikh2000
// Changed calls that get profile strings.
//
// Revision 1.2  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.1  2003/05/22 23:35:04  mrimer
// Initial check-in (inheriting from the corresponding file in the new FrontEndLib).
//
