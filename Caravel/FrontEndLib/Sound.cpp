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

//Sound.cpp
//Implementation of CSound class.

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#define INCLUDED_FROM_SOUND_CPP
#include "Sound.h"
#undef INCLUDED_FROM_SOUND_CPP

#include <BackEndLib/Files.h>

#include <SDL.h>

//Global instance of the one-and-only sound object.
CSound *g_pTheSound = NULL;

//Tracks the lyric notes that have played in a song.  This is module-scoped
//because callback needs to access it.
UINT m_wLyricNoteCount = 0;

UINT CSound::SOUND_EFFECT_COUNT = 0;
UINT CSound::SAMPLE_CHANNEL_COUNT = 0;
UINT CSound::MODULE_CHANNEL_COUNT = 0;
UINT CSound::CHANNEL_COUNT = SAMPLE_CHANNEL_COUNT + MODULE_CHANNEL_COUNT;

#if defined(USE_FMOD)
//******************************************************************************
void F_CALLBACKAPI OnLyricNotePlayed(FMUSIC_MODULE* /*pModule*/, unsigned char /*InstrumentNo*/)
//Called by FMOD whenever a note plays for the lyric instrument.
//
//From FMOD docs:
//It is important to note that this callback will be called from directly WITHIN
//the mixer / music update thread, therefore it is imperative that whatever you
//do from this callback be extremely efficient. If the routine takes too long
//then breakups in the sound will occur, or it will basically stop mixing until
//you return from the function.
{
	++m_wLyricNoteCount;
}
#endif

//
//CSoundEffect public methods.
//

//***********************************************************************************
CSoundEffect::CSoundEffect(void)
//Constructor.
{
#if !defined(NO_SOUND)
	this->nChannel = -1;
	this->bPlayRandomSample = this->bIsLoaded = false;
	this->iLastSamplePlayed = this->Samples.end();
#endif
}

//***********************************************************************************
CSoundEffect::~CSoundEffect(void)
//Destructor.
{
	if (this->bIsLoaded) Unload();
}

//***********************************************************************************
bool CSoundEffect::Load(
//Load a sound effect so that it is ready to play.
//
//Params:
	list<WSTRING> FilepathArray,				//(in)	One or more full paths to wave
											//		files that will be loaded as
											//		samples to play for this sound
											//		effect.
	const int nSetChannel,					//(in)	Channel on which samples will
											//		play.
	const bool bSetPlayRandomSample)		//(in)	If true, samples will play
											//		randomly.  If false, (default)
											//		samples will play in sequence.
											//		Does not affect a sound effect
											//		with only one sample.
//
//Returns:
//True if all samples were loaded, false if not.  If no samples load, the Play()
//method will do nothing when called.
{
#if defined(NO_SOUND)
	return true;
#else
	bool bCompleteSuccess = true;
	ASSERT(!this->bIsLoaded);
	ASSERT(static_cast<UINT>(nSetChannel) >= CSound::MODULE_CHANNEL_COUNT);
	ASSERT(static_cast<UINT>(nSetChannel) < CSound::CHANNEL_COUNT);

	this->nChannel = nSetChannel;
	this->bPlayRandomSample = bSetPlayRandomSample;

	//Load each sample from a wave file.
	for (list<WSTRING>::const_iterator iFilepath = FilepathArray.begin();
		iFilepath != FilepathArray.end(); ++iFilepath)
	{
		char buffer[MAX_PATH+1];
		UnicodeToAscii(*iFilepath, buffer);
		SOUND *pSample = LoadWave(iFilepath->c_str());
		if (pSample)
		{
#if defined(USE_FMOD)
			FSOUND_Sample_SetMode(pSample, FSOUND_LOOP_OFF);
#endif
			this->Samples.push_back(pSample);
		}
		else
			bCompleteSuccess = false;
	}

	//If at least one sample loaded, then I will call this sound effect "loaded".
	this->bIsLoaded = (this->Samples.size() > 0);

	return bCompleteSuccess;
#endif
}

//***********************************************************************************
void CSoundEffect::Play(void)
//Plays a sample for the sound effect.
{
#if !defined(NO_SOUND)
	//Do nothing if sound effect is not loaded.
	if (!this->bIsLoaded) return;

	//Stop any other samples playing on this channel.
#if defined(USE_SDL_MIXER)
	if (Mix_HaltChannel(this->nChannel)) ASSERTP(false, "Failed to stop sound.");
#elif defined(USE_FMOD)
	if (!FSOUND_StopSound(this->nChannel)) ASSERTP(false, "Failed to stop sound.");
#endif

	//Figure out which sample to play next.
	list<SOUND *>::iterator iPlaySample;
	if (this->Samples.size() == 1)
		iPlaySample = this->iLastSamplePlayed = this->Samples.begin();
	else if (this->bPlayRandomSample)
	{
		int nRandSample = (int)rand() % this->Samples.size();
		iPlaySample = this->Samples.begin();
		for (int nSampleCount=0; nSampleCount < nRandSample; ++nSampleCount)
		{
			ASSERT(iPlaySample != this->Samples.end());
			++iPlaySample;
		}
		this->iLastSamplePlayed = iPlaySample;
	}
	else
	{
		if (this->iLastSamplePlayed == this->Samples.end())
			this->iLastSamplePlayed = this->Samples.begin();
		else
		{
			if (++(this->iLastSamplePlayed) == this->Samples.end())
				this->iLastSamplePlayed = this->Samples.begin();
		}
		iPlaySample = this->iLastSamplePlayed;
	}

	//Play the sample.
#if defined(USE_SDL_MIXER)
	if (!Mix_PlayChannel(this->nChannel, *iPlaySample, 0)) ASSERTP(false, "Failed to play sound.");
#elif defined(USE_FMOD)
	if (!FSOUND_PlaySound(this->nChannel, *iPlaySample)) ASSERTP(false, "Failed to play sound.");
#endif
#endif
}

//***********************************************************************************
void CSoundEffect::Unload(void)
{
#if !defined(NO_SOUND)
	ASSERT(this->bIsLoaded);

	//Free the samples.
	for (list<SOUND *>::iterator iSeek = this->Samples.begin();
			iSeek != this->Samples.end(); ++iSeek)
#if defined(USE_SDL_MIXER)
		Mix_FreeChunk(*iSeek);
#elif defined(USE_FMOD)
		FSOUND_Sample_Free(*iSeek);
#endif
	this->Samples.clear();

	this->bIsLoaded = false;
#endif
}

//
//CSoundEffect private methods.
//

#if !defined(NO_SOUND)
//**********************************************************************************
SOUND* CSoundEffect::LoadWave(
//Load wave file, unencoding if necessary.
//
//Returns:
//Loaded wave file.
//
//Params:
	const WCHAR *pwszWaveFilepath)	//(in) Path+name of wave file to load
const
{
	WSTRING wstrWaveFilepath = pwszWaveFilepath;
	CFiles::GetTrueDatafileName(&*wstrWaveFilepath.begin());
	CStretchyBuffer buffer;
	CFiles::ReadFileIntoBuffer(pwszWaveFilepath,buffer);
	if (CFiles::FileIsEncrypted(pwszWaveFilepath))
	{
		//Unencode encrpyted wave file.
		buffer.Decode();
	}
#if defined(USE_SDL_MIXER)
	SDL_RWops* rwops = SDL_RWFromMem((char*)(BYTE*)buffer, buffer.Size());
        int freesrc = 1;
        return Mix_LoadWAV_RW(rwops, freesrc);
#elif defined(USE_FMOD)
	return FSOUND_Sample_Load(FSOUND_FREE, (char*)(BYTE*)buffer,
			FSOUND_2D | FSOUND_LOADMEMORY, 0, buffer.Size());
#endif
}
#endif

//
//CSound Public methods.
//

//**********************************************************************************
CSound::CSound(
//Constructor.  If sound initialization fails, this object will be disabled for
//the rest of its life, and calls to methods will generally not do anything.
//
//Params:
   const bool bNoSound,
   const UINT SOUND_EFFECT_COUNT,
   const UINT SAMPLE_CHANNEL_COUNT,
   const UINT MODULE_CHANNEL_COUNT)
   : bNoSound(bNoSound)
   , bMusicOn(!bNoSound), bMusicAvailable(false)
   , bSoundEffectsOn(!bNoSound), bSoundEffectsAvailable(false)
   , eCurrentPlayingSongID(SOUNDLIB::SONGID_NONE)
   , nSoundVolume(128), nMusicVolume(128)
#if !defined(NO_SOUND)
   , pModule(NULL)
#endif
#if defined(USE_SDL_MIXER)
   , pModuleRWops(NULL)
#endif
{
#if !defined(NO_SOUND)
	if (bNoSound) return;

	CSound::SOUND_EFFECT_COUNT = SOUND_EFFECT_COUNT;
	CSound::SAMPLE_CHANNEL_COUNT = SAMPLE_CHANNEL_COUNT;
	CSound::MODULE_CHANNEL_COUNT = MODULE_CHANNEL_COUNT;
	CSound::CHANNEL_COUNT = SAMPLE_CHANNEL_COUNT + MODULE_CHANNEL_COUNT;

  	this->SoundEffectArray = new CSoundEffect[SOUND_EFFECT_COUNT];

	this->ChannelSoundEffects = new UINT[CHANNEL_COUNT];
	for (UINT nChannel=CHANNEL_COUNT; nChannel--; )
		this->ChannelSoundEffects[nChannel] = static_cast<UINT>(SOUNDLIB::SEID_NONE);

	//The call to InitSound() will set music and sound effects availability.
	InitSound();
#endif
}

//**********************************************************************************
CSound::~CSound(void)
//Destructor.
{
  if (!bNoSound)
  {
	  UnloadSoundEffects();
	  DeinitSound();

	  delete[] this->ChannelSoundEffects;
	  delete[] this->SoundEffectArray;
  }
}

//********************************************************************************
UINT CSound::GetLyricNoteCount(void)
//Returns the number of notes that have played for a song instrument that
//corresponds to lyrics.
const
{
	return m_wLyricNoteCount;
}

//********************************************************************************
void CSound::EnableSoundEffects(const bool bSetSoundEffectsOn)
//Turns sound effect playback on or off.
{
	this->bSoundEffectsOn=bSetSoundEffectsOn;
}

//********************************************************************************
void CSound::EnableMusic(const bool bSetMusicOn)
//Turns music playback on or off.
{
   this->bMusicOn=bSetMusicOn;
	if (!bSetMusicOn)
		StopSong();
}

//********************************************************************************
void CSound::SetSoundEffectsVolume(
//Sets volume of sound effects.
	const int volume	//(in) value between 0 (silent) and 255 (full)
)
{
#if !defined(NO_SOUND)
	ASSERT(volume >= 0 && volume <= 255);
	nSoundVolume = volume;
#if defined(USE_SDL_MIXER)
	// Not quite the same, this will change the volume for all channels.
        // The volume level goes from 0-128, instead of 0-255.
	Mix_Volume(-1, volume >> 1);
#elif defined(USE_FMOD)
	FSOUND_SetSFXMasterVolume(volume);
#endif
#endif
}

//********************************************************************************
void CSound::SetMusicVolume(
//Sets volume of music.
	const int volume	//(in) value between 0 (silent) and 255 (full)
)
{
#if !defined(NO_SOUND)
	ASSERT(volume >= 0 && volume <= 255);
	nMusicVolume = volume;
#if defined(USE_SDL_MIXER)
	// The volume level goes from 0-128, instead of 0-255.
	Mix_VolumeMusic(volume >> 1);
#elif defined(USE_FMOD)
	if (this->pModule) FMUSIC_SetMasterVolume(this->pModule,volume);
#endif
#endif
}

//********************************************************************************
bool CSound::IsSongFinished(void) const
//Returns whether the song has completed playing, or when the last order has
//finished playing.  This stays set even if the song loops.  Also returns true
//if no song is playing.
{
#if !defined(NO_SOUND)
	//Return without doing anything if no music is playing.
	if (!this->bMusicOn || !this->bMusicAvailable || !this->pModule)
		return false;

#if defined(USE_SDL_MIXER)
	return Mix_PlayingMusic();
#elif defined(USE_FMOD)
	return (FMUSIC_IsFinished(this->pModule) != 0);
#endif
#else
	return true;
#endif
}

//********************************************************************************
void CSound::PlaySong(
//Plays a song.
//
//Params:
	const UINT eSongID,			//(in)	Song to play.
	const int nLyricInstrumentNo)	//(in)	Which instrument in the song corresponds
									//		to lyrics for which notes will be counted.
									//		If -1 (default) then no lyric note
									//		counting will be performed.
{
#if !defined(NO_SOUND)
	//Return successful without doing anything if music has been disabled.
	if (!this->bMusicOn || !this->bMusicAvailable) return;

	//Is the requested song, currently playing?
	if (eSongID == this->eCurrentPlayingSongID) return; //Yes--nothing to do.

	//Stop any currently playing song.
	if (!StopSong()) ASSERTP(false, "Failed to stop song.");

	//Get filepath for new song to play.
	WSTRING wstrSongFilepath;
	if (!GetSongFilepath(eSongID, wstrSongFilepath))
   {
      CFiles f;
      f.AppendErrorLog("A song failed to load. Check whether ");
      char filepath[512];
      UnicodeToAscii(wstrSongFilepath, filepath);
      f.AppendErrorLog(filepath);
      f.AppendErrorLog(" is a valid filename.\r\n");
      ASSERTP(false, "Song failed to load.");
      return;
   }
	CFiles::GetTrueDatafileName(&*wstrSongFilepath.begin());

	//Load the song.
	CStretchyBuffer buffer;
	CFiles::ReadFileIntoBuffer(wstrSongFilepath.c_str(),buffer);
	if (CFiles::FileIsEncrypted(wstrSongFilepath.c_str()))
	{
		//Unencode encrpyted song file.
		buffer.Decode();
	}
#if defined(USE_SDL_MIXER)
        this->pModuleRWops = SDL_RWFromConstMem((const char*)(BYTE*)buffer, buffer.Size());
	this->pModule = Mix_LoadMUS_RW(this->pModuleRWops);
#elif defined(USE_FMOD)
	this->pModule = FMUSIC_LoadSongEx((const char*)(BYTE*)buffer,0,buffer.Size(),
	                                  FSOUND_LOADMEMORY,NULL,1);
#endif
	if (!this->pModule) {ASSERTP(false, "Failed to load song.(2)"); return;}

	//Set volume.
#if defined(USE_SDL_MIXER)
	// The volume level goes from 0-128, instead of 0-255.
	Mix_VolumeMusic(nMusicVolume >> 1);
#elif defined(USE_FMOD)
	FMUSIC_SetMasterVolume(this->pModule,nMusicVolume);
#endif

	//Set callback for counting lyric notes.
	if (nLyricInstrumentNo != -1)
	{
		// Not sure how to make this work without FMOD.
#if defined(USE_FMOD)
		if (!FMUSIC_SetInstCallback(this->pModule, OnLyricNotePlayed,
				nLyricInstrumentNo))
			ASSERTP(false, "Lyric counting isn't going to work.");
#endif
	}

	//Play the song.
#if defined(USE_SDL_MIXER)
	Mix_PlayMusic(this->pModule, 1);
#elif defined(USE_FMOD)
	if (!FMUSIC_PlaySong(this->pModule))
	{
		FMUSIC_FreeSong(this->pModule);
		this->pModule = NULL;
		ASSERTP(false, "Failed to play song.");
		return;
	}
#endif

	//Success.
	this->eCurrentPlayingSongID = eSongID;
#endif
}

//********************************************************************************
bool CSound::StopSong(void)
//Stops a currently playing song.
//
//Returns:
//True if no song is playing when function returns, false if not.
{
	bool bSuccess=true;
#if !defined(NO_SOUND)

	if (this->pModule)
	{
#if defined(USE_SDL_MIXER)
		Mix_HaltMusic();
		Mix_FreeMusic(this->pModule);
		SDL_FreeRW(this->pModuleRWops);
		bSuccess = true;
#elif defined(USE_FMOD)
		bSuccess = (FMUSIC_StopSong(this->pModule) != 0);
		FMUSIC_FreeSong(this->pModule);
#endif
		this->pModule = NULL;
	}

	this->eCurrentPlayingSongID = SOUNDLIB::SONGID_NONE;
	m_wLyricNoteCount = 0;
#endif
	return bSuccess;
}

//********************************************************************************
void CSound::PlaySoundEffect(
//Plays a sound effect.
//
//Params:
	const UINT eSEID)	//(in) A SEID_* constant indicating sound effect to play.
{
	//Return successful without doing anything if sound effects have been disabled.
	if (!this->bSoundEffectsOn || !this->bSoundEffectsAvailable) return;

    ASSERT(eSEID < CSound::SOUND_EFFECT_COUNT);

	//Play it.
	this->SoundEffectArray[eSEID].Play();

	//Keep track of what sound effect is playing on what channel.
	this->ChannelSoundEffects[this->SoundEffectArray[eSEID].GetChannel()] =
		eSEID;
}

//***********************************************************************************
bool CSound::IsSoundEffectPlaying(
//Is a sound effect playing now?
//
//Params:
	const UINT eSEID)	//(in)	Sound effect to check.
//
//Returns:
//True if it is, false if not.
const
{
#if !defined(NO_SOUND)
  //Return successful without doing anything if sound effects have been disabled.
	if (!this->bSoundEffectsOn || !this->bSoundEffectsAvailable) return false;

    ASSERT(eSEID < CSound::SOUND_EFFECT_COUNT);

	//Sound effect will play on a certain channel.
	int nChannel = this->SoundEffectArray[eSEID].GetChannel();

	//Other sound effects may play on the same channel.  Check that my sound effect
	//is the last one that was played.
	if (this->ChannelSoundEffects[nChannel] != eSEID) return false;

	//Check that a sample is currently playing on the channel.  If it is, then I
	//know that it is playing a sample for my sound effect.
#if defined(USE_SDL_MIXER)
	return Mix_Playing(nChannel);
#elif defined(USE_FMOD)
	return (FSOUND_IsPlaying(nChannel)!=false);
#endif
#else
	return true;
#endif
}

//***********************************************************************************
bool CSound::WaitForSoundEffectsToStop(
//Wait for all sound effects to stop playing.
//
//Params:
	const DWORD dwMaxWaitTime)	//(in)	Longest time in msecs to wait.  Default
								//		is 3000 (3 secs).
//
//Returns:
//True if all sound effects stopped, false if max wait time elapsed.
const
{
#if !defined(NO_SOUND)
	//Return successful without doing anything if sound effects have been disabled.
	if (!this->bSoundEffectsOn || !this->bSoundEffectsAvailable) return true;

	DWORD dwStartTime = SDL_GetTicks();
	do
	{
		//Check for samples playing on channels reserved for samples.  Channels
		//after the module channels are sample channels.
      UINT nChannelNo;
		for (nChannelNo = MODULE_CHANNEL_COUNT; nChannelNo < CHANNEL_COUNT;
				++nChannelNo)
		{
#if defined(USE_SDL_MIXER)
			if (Mix_Playing(nChannelNo)) break; //At least one channel is playing.
#elif defined(USE_FMOD)
			if (FSOUND_IsPlaying(nChannelNo)) break; //At least one channel is playing.
#endif
		}
		if (nChannelNo == CHANNEL_COUNT) return true; //Everything has stopped.

		//Let other threads run.
		SDL_Delay(100);
	}
	while (SDL_GetTicks() - dwStartTime < dwMaxWaitTime);

	//Timed out waiting.
	return false;
#else
	return true;
#endif
}

//
//CSound Private methods.
//

//***********************************************************************************
int CSound::GetLastFSOUNDError(
//Gets last FSOUND error code along with test description.
//
//Params:
	string &strErrDesc)	//(in/out)	Corresponds to error code.  Appends to end of
						//			string which may or may not be empty.
//
//Returns:
//Error code or FMOD_ERR_NONE if no FSOUND error.
const
{
#if defined(USE_FMOD)
	int nErrCode = FSOUND_GetError();
	char szTemp[30];
	sprintf(szTemp, "    FMOD Error #%d: ", nErrCode);
	strErrDesc += szTemp;

	//These are taken out of the FMOD 3.5 docs.  Update with new versions as
	//needed.
	switch (nErrCode)
	{
		case FMOD_ERR_NONE:
			strErrDesc +=	"No errors.\r\n";
		break;

		case FMOD_ERR_BUSY:
			strErrDesc +=	"Cannot call this command after FSOUND_Init. Call "
							"FSOUND_Close first.\r\n";
		break;

		case FMOD_ERR_UNINITIALIZED:
			strErrDesc +=	"This command failed because FSOUND_Init or "
							"FSOUND_SetOutput was not called.\r\n";
		break;

		case FMOD_ERR_INIT:
			strErrDesc +=	"Error initializing output device.\r\n";
		break;

		case FMOD_ERR_ALLOCATED:
			strErrDesc +=	"Error initializing output device, but more "
							"specifically, the output device is already in use and "
							"cannot be reused.\r\n";
 		break;

		case FMOD_ERR_PLAY:
			strErrDesc +=	"Playing the sound failed.\r\n";
 		break;

		case FMOD_ERR_OUTPUT_FORMAT:
			strErrDesc +=	"Soundcard does not support the features needed for "
							"this soundsystem (16bit stereo output).\r\n";
 		break;

		case FMOD_ERR_COOPERATIVELEVEL:
			strErrDesc +=	"Error setting cooperative level for hardware.\r\n";
 		break;

		case FMOD_ERR_CREATEBUFFER:
			strErrDesc +=	"Error creating hardware sound buffer.\r\n";
 		break;

		case FMOD_ERR_FILE_NOTFOUND:
			strErrDesc +=	"File not found.\r\n";
 		break;

		case FMOD_ERR_FILE_FORMAT:
			strErrDesc +=	"Unknown file format.\r\n";
 		break;

		case FMOD_ERR_FILE_BAD:
			strErrDesc +=	"Error loading file.\r\n";
 		break;

		case FMOD_ERR_MEMORY:
			strErrDesc +=	"Not enough memory or resources.\r\n";
 		break;

		case FMOD_ERR_VERSION:
			strErrDesc +=	"The version number of this file format is not "
							"supported.\r\n";
 		break;

		case FMOD_ERR_INVALID_PARAM:
			strErrDesc +=	"An invalid parameter was passed to this "
							"function.\r\n";
 		break;

		case FMOD_ERR_NO_EAX:
			strErrDesc +=	"Tried to use an EAX command on a non EAX enabled "
							"channel or output.\r\n";
 		break;

		case FMOD_ERR_CHANNEL_ALLOC:
			strErrDesc +=	"Failed to allocate a new channel.\r\n";
 		break;

		case FMOD_ERR_RECORD:
			strErrDesc +=	"Recording is not supported on this machine.\r\n";
 		break;

		case FMOD_ERR_MEDIAPLAYER:
			strErrDesc +=	"Windows Media Player not installed so cannot play wma "
							"or use internet streaming.\r\n";
		break;

		default:
			strErrDesc +=	"Description unavailable.\r\n";
	}

	return nErrCode;
#else
	return 0;
#endif
}

//**********************************************************************************
bool CSound::InitSound(void)
//Initializes sound module.
//
//Returns:
//true if at least sound effects will be available, false otherwise.
{
#if defined(USE_SDL_MIXER)
	this->bSoundEffectsAvailable = this->bMusicAvailable = true;
	Mix_Init(MIX_INIT_MOD);
	const int chunksize = 8096;
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, chunksize) ||
            Mix_AllocateChannels(CHANNEL_COUNT) != CHANNEL_COUNT)
	{
		this->bSoundEffectsAvailable = this->bMusicAvailable = false;
		//Cleanup.
		DeinitSound();
		return false;
	}

	return true;
#elif defined(USE_FMOD)
	this->bSoundEffectsAvailable = this->bMusicAvailable = true;

	//This log string will appear if an error occurs.
	string strInitLog = "This is what happened while initializing sound:\r\n";

	while (true) //non-looping.
	{
		//Compare loaded FMOD.DLL version to import library linked into .EXE.
		if (FSOUND_GetVersion() != FMOD_VERSION)
		{
			char szVersion[100];
			sprintf(szVersion, "  FMOD.DLL version is %g and import library is %g!\r\n",
					FSOUND_GetVersion(), FMOD_VERSION);
			strInitLog += szVersion;
			break;
		}
		else
			strInitLog += "  FMOD.DLL version matches import library.\r\n";

		//Init FSOUND--try best case params first.
		if (FSOUND_Init(44100, CHANNEL_COUNT, 0))
			strInitLog += "  FSOUND was initialized with best case params.\r\n";
		else
		{
			strInitLog += "  FSOUND_Init(44100, CHANNEL_COUNT, 0) failed.\r\n";

			//Maybe the mixing rate is too high?  Try 22mhz and 11mhz.
			if (FSOUND_Init(22050, CHANNEL_COUNT, 0)) return true;
			strInitLog += "  FSOUND_Init(22050, CHANNEL_COUNT, 0) failed.\r\n";
			if (FSOUND_Init(11025, CHANNEL_COUNT, 0)) return true;
			strInitLog += "  FSOUND_Init(11025, CHANNEL_COUNT, 0) failed.\r\n";

			//Maybe I'm asking for too many channels?  Try just enough for
			//sound effects, but not music.
			if (FSOUND_Init(44100, SAMPLE_CHANNEL_COUNT, 0))
			{
				//Not enough channels for music.
				CFiles Files;
				Files.AppendErrorLog("Not enough available channels for music.\r\n");
				this->bMusicAvailable = false;
				return true;
			}
			strInitLog += "  FSOUND_Init(44100, SAMPLE_CHANNEL_COUNT, 0) failed.\r\n";

			//I give up.
			break;
		}

		//Ready to play sound effects and probably music.
		return true;
	}

	//Sadly, there will be no sound.
	this->bMusicAvailable = this->bSoundEffectsAvailable = false;

	//Log the last FSOUND error which probably caused the failure.
	{
		GetLastFSOUNDError(strInitLog);
		CFiles Files;
		Files.AppendErrorLog(strInitLog.c_str());
	}

	//Cleanup.
	DeinitSound();
	return false;
#else
	return false;
#endif
}

//***********************************************************************************
void CSound::DeinitSound(void)
//Deinits sound module.
{
#if defined(USE_SDL_MIXER)
	if (this->pModule)
	{
		Mix_FreeMusic(this->pModule);
		SDL_FreeRW(this->pModuleRWops);
		this->pModule = NULL;
	}
        Mix_CloseAudio();
	Mix_Quit();
#elif defined(USE_FMOD)
	if (this->pModule)
	{
		FMUSIC_FreeSong(this->pModule);
		this->pModule = NULL;
	}
	FSOUND_Close();
#endif
}

//**********************************************************************************
void CSound::UnloadSoundEffects(void)
//Unloads sound effects from array.
{
	for (UINT nSEI = 0; nSEI < CSound::SOUND_EFFECT_COUNT; ++nSEI)
	{
		if (this->SoundEffectArray[nSEI].IsLoaded())
			this->SoundEffectArray[nSEI].Unload();
	}
}

// $Log: Sound.cpp,v $
// Revision 1.9  2004/05/20 17:38:42  mrimer
// Updated FMOD API to 3.72.
//
// Revision 1.8  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.7  2003/07/05 02:33:51  mrimer
// Put in some diagnostic error messages when sounds/music aren't loaded.
//
// Revision 1.6  2003/06/28 18:43:50  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.5  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.4  2003/06/09 19:22:16  mrimer
// Added optional max width/height params to FM::DrawTextXY().
//
// Revision 1.3  2003/05/29 19:59:03  mrimer
// Made LoadSoundEffects() pure virtual.  Called in derived class constructor.
//
// Revision 1.2  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.1  2003/05/22 21:49:31  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.32  2003/05/08 23:27:09  erikh2000
// Constructor accepts parameter to disable all sound and avoid FMOD calls entirely.
//
// Revision 1.31  2003/05/04 00:23:36  mrimer
// Revised CStretchyBuffer API.
//
// Revision 1.30  2003/04/13 02:04:35  mrimer
// Changed music function call to work with FMOD 3.62 API.
//
// Revision 1.29  2003/04/08 13:08:29  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.28  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.27  2002/11/15 02:23:23  mrimer
// Added sounds for cue events EvilEyeWoke and OrbActivatedByMimic.
//
// Revision 1.26  2002/10/25 02:55:14  erikh2000
// Added new method to tell when a song has finished playing.
//
// Revision 1.25  2002/10/25 01:38:45  erikh2000
// Made the music-disabling code a bit simpler.
//
// Revision 1.24  2002/10/23 23:02:33  erikh2000
// Made the sound initialization less assuming and added error logging to it.
//
// Revision 1.23  2002/08/30 22:43:44  erikh2000
// Added method that waits for all sound effects to stop playing.
//
// Revision 1.22  2002/08/28 21:40:54  erikh2000
// Added new sounds for breaking crumbling wall, tar stab, door opening, and last brain destroyed.
//
// Revision 1.21  2002/08/28 20:31:52  mrimer
// Added loading 'Neather sound IDs, and TIRED.
//
// Revision 1.20  2002/08/24 21:46:41  erikh2000
// You can now check if a sound effect is playing or not.
//
// Revision 1.19  2002/08/23 23:48:48  erikh2000
// Added missing "mrimer" to contributor list.
// Renamed several methods and types for consistency and accuracy.
// Added lyric tracking.
// Sound effect indicates a collection of one or more loaded samples.
// More than one sound effect can play on a single channel.
//
// Revision 1.18  2002/07/22 18:38:17  mrimer
// Changed waves and song consts to enumerated types.
//
// Revision 1.17  2002/07/18 20:17:42  mrimer
// Added loading of encrypted files with altered filenames.
//
// Revision 1.16  2002/07/02 23:56:11  mrimer
// Added file unencryption code.
//
// Revision 1.15  2002/06/23 10:59:26  erikh2000
// Added methods to get current sound and music volumes.
// Removed some dead code.
//
// Revision 1.14  2002/06/11 17:48:29  mrimer
// Now disabling music will stop any current song playing.
// Fixed #includes.
//
// Revision 1.13  2002/05/24 23:47:25  erikh2000
// Increased number of channels used by modules to 16, because one song was losing instruments.
//
// Revision 1.12  2002/05/16 19:37:42  mrimer
// Added SetSoundVolume() and SetMusicVolume().
//
// Revision 1.11  2002/05/15 01:31:42  erikh2000
// Reduced number of channels used for playing back songs to 12, since 24 channels aren't needed.
//
// Revision 1.10  2002/05/12 06:58:48  erikh2000
// Fixed competing sound bug.
//
// Revision 1.9  2002/04/25 09:34:40  erikh2000
// A call to PlaySong() to play the same song as already playing will now do nothing.
//
// Revision 1.8  2002/04/09 01:50:04  erikh2000
// Removed unneeded #includes.
//
// Revision 1.7  2002/03/13 06:31:36  erikh2000
// Fixed path problem so sounds would play.  (Committed on behalf of mrimer.)
//
// Revision 1.6  2002/03/05 01:52:59  erikh2000
// Added 2002 copyright notice to top of file.
//
// Revision 1.5  2001/11/06 09:13:49  erikh2000
// Fixed FMOD compatibility error in CSound::LoadWaves().  (Committed on behalf of j_wicks.)
//
// Revision 1.4  2001/11/06 08:46:13  erikh2000
// Added extra constant and mapping for trapdoor wave.  (Committed on behalf of jpburford.)
//
// Revision 1.3  2001/10/22 21:30:04  erikh2000
// Commented out call to FSOUND_Sample_LoadWav that is undefined in FMOD 3.4.
//
// Revision 1.2  2001/10/06 03:38:37  erikh2000
// Fixed errors in app initialization--should at least get to the title screen now.
// Fixed GetSongFilepath() to use new dir structure.
// Removed WinHelp calls.
//
// Revision 1.1.1.1  2001/10/01 22:18:07  erikh2000
// Initial check-in.
//
