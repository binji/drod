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

//Sound.h
//Declarations for CSound.
//Class for playing waves and music for DROD.

#ifndef SOUND_H
#define SOUND_H

#ifdef WIN32
#pragma warning(disable:4786)
#endif
#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>
#include <BackEndLib/Wchar.h>

#if defined(__native_client__)
#include <SDL_mixer.h>
typedef Mix_Chunk SOUND;
typedef Mix_Music MUSIC;
#define USE_SDL_MIXER 1
#elif !defined(__sgi)
#include <fmod.h>
typedef FSOUND SOUND;
typedef FMUSIC_MODULE MUSIC;
#define USE_FMOD 1
#else
#define NO_SOUND
#endif

#include <list>
#include <string>

using namespace std;

//Music and sound effect IDs.
namespace SOUNDLIB
{
	enum SONGID
	{
		SONGID_NONE = 0
	};

	enum SEID
	{
		SEID_NONE = -1,
		SEID_BUTTON,
		SEID_READ
	};
};

//Class for loading and playing samples for a sound effect.
class CSoundEffect
{
public:
	CSoundEffect(void);
	~CSoundEffect(void);
	
	int						GetChannel(void) const {return this->nChannel;}
	bool					IsLoaded(void) const {return this->bIsLoaded;}
	bool					Load(list<WSTRING> FilepathArray, const int nSetChannel, 
			const bool bSetPlayRandomSample=false);
	void					Play(void);
	void					Unload(void);

private:
#if !defined(__sgi)
	SOUND*			LoadWave(const WCHAR *pwszWaveFilepath) const;

	list<SOUND *>	Samples;
#endif

	int						nChannel;
	bool					bPlayRandomSample;
	bool					bIsLoaded;
	
#if !defined(__sgi)
	list<SOUND *>::iterator iLastSamplePlayed;
#endif

	PREVENT_DEFAULT_COPY(CSoundEffect);
};

//Class used to perform all sound operations.
class CSound
{
public:
	CSound(const bool bNoSound, const UINT SOUND_EFFECT_COUNT,
			const UINT SAMPLE_CHANNEL_COUNT, const UINT MODULE_CHANNEL_COUNT);
	virtual ~CSound(void);

	int				GetCurrentPlayingSong(void) {return this->eCurrentPlayingSongID;}
	UINT			GetLyricNoteCount(void) const;
	int				GetMusicVolume(void) const {return this->nMusicVolume;}
	int				GetSoundVolume(void) const {return this->nSoundVolume;}
	void			PlaySong(const UINT nSongID, const int nLyricInstrumentNo = -1);
	void			PlaySoundEffect(const UINT eSEID);
	void			EnableMusic(const bool bSetMusic);
	void			EnableSoundEffects(const bool bSetSoundEffects);
	bool			IsMusicOn(void) const {return this->bMusicOn;}
	bool			IsSongFinished(void) const;
	bool			IsSoundEffectPlaying(const UINT eSEID) const;
	bool			IsSoundEffectsOn(void) const {return this->bSoundEffectsOn;}
	void			SetSoundEffectsVolume(const int volume);
	void			SetMusicVolume(const int volume);
	bool			StopSong(void);
	bool			WaitForSoundEffectsToStop(const DWORD dwMaxWaitTime = 3000L) const;

	static UINT SOUND_EFFECT_COUNT, SAMPLE_CHANNEL_COUNT, MODULE_CHANNEL_COUNT, CHANNEL_COUNT;

protected:
	void			DeinitSound(void);
	int				GetLastFSOUNDError(string &strErrorDesc) const;
	virtual bool			GetSongFilepath(const UINT nSongID, WSTRING &wstrFilepath)=0;
	virtual bool			GetWaveFilepaths(const UINT eSEID, list<WSTRING> &FilepathList) const=0;
	virtual bool			LoadSoundEffects()=0;
	bool			InitSound(void);
	void			UnloadSoundEffects(void);

	bool      bNoSound;
	bool			bMusicOn;
	bool			bMusicAvailable;
	bool			bSoundEffectsOn;
	bool			bSoundEffectsAvailable;
	UINT			eCurrentPlayingSongID;
	int				nSoundVolume, nMusicVolume;

#if !defined(NO_SOUND)
	MUSIC *	pModule;
#endif
#if defined(USE_SDL_MIXER)
	SDL_RWops* pModuleRWops;
#endif
	CSoundEffect	*SoundEffectArray;
	
	UINT			*ChannelSoundEffects;

	PREVENT_DEFAULT_COPY(CSound);
};

//Define global pointer to the one and only CSound object.
#ifndef INCLUDED_FROM_SOUND_CPP
	extern CSound *g_pTheSound;
#endif

#endif //...#ifndef SOUND_H

// $Log: Sound.h,v $
// Revision 1.4  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.3  2003/05/29 19:59:03  mrimer
// Made LoadSoundEffects() pure virtual.  Called in derived class constructor.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:32  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.20  2003/05/08 23:27:09  erikh2000
// Constructor accepts parameter to disable all sound and avoid FMOD calls entirely.
//
// Revision 1.19  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.18  2002/11/15 02:23:23  mrimer
// Added sounds for cue events EvilEyeWoke and OrbActivatedByMimic.
//
// Revision 1.17  2002/10/25 02:55:14  erikh2000
// Added new method to tell when a song has finished playing.
//
// Revision 1.16  2002/10/23 23:02:33  erikh2000
// Made the sound initialization less assuming and added error logging to it.
//
// Revision 1.15  2002/10/10 01:29:48  erikh2000
// Added methods to check if sound effects or music are turned on.
//
// Revision 1.14  2002/08/30 22:43:44  erikh2000
// Added method that waits for all sound effects to stop playing.
//
// Revision 1.13  2002/08/28 21:40:54  erikh2000
// Added new sounds for breaking crumbling wall, tar stab, door opening, and last brain destroyed.
//
// Revision 1.12  2002/08/28 20:30:16  mrimer
// Added 'Neather sound IDs, and TIRED.
//
// Revision 1.11  2002/08/24 21:46:42  erikh2000
// You can now check if a sound effect is playing or not.
//
// Revision 1.10  2002/08/23 23:48:48  erikh2000
// Added missing "mrimer" to contributor list.
// Renamed several methods and types for consistency and accuracy.
// Added lyric tracking.
// Sound effect indicates a collection of one or more loaded samples.
// More than one sound effect can play on a single channel.
//
// Revision 1.9  2002/07/22 18:38:17  mrimer
// Changed waves and song consts to enumerated types.
//
// Revision 1.8  2002/07/02 23:55:24  mrimer
// Added LoadWave().
//
// Revision 1.7  2002/06/23 10:59:26  erikh2000
// Added methods to get current sound and music volumes.
// Removed some dead code.
//
// Revision 1.6  2002/06/11 17:01:48  mrimer
// Moved '#include <fmod.h>' from Sound.cpp.
//
// Revision 1.5  2002/05/16 19:37:43  mrimer
// Added SetSoundVolume() and SetMusicVolume().
//
// Revision 1.4  2002/04/09 01:50:37  erikh2000
// Added macro to prevent default copy on CSound.
//
// Revision 1.3  2002/03/05 01:52:59  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.2  2001/11/06 08:46:13  erikh2000
// Added extra constant and mapping for trapdoor wave.  (Committed on behalf of jpburford.)
//
// Revision 1.1.1.1  2001/10/01 22:18:07  erikh2000
// Initial check-in.
//
