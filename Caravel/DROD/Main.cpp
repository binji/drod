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
 * Portions created by the Initial Developer are Copyright (C) 2002 Caravel
 * Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#ifdef WIN32
#	include <windows.h> //Should be first include.
#	pragma warning(disable:4786)
#endif

#ifdef __linux__
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <unistd.h>
#	include <errno.h>
#endif

#include "DrodFontManager.h"
#include "DrodBitmapManager.h"
#include "DrodScreenManager.h"
#include "DrodSound.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbPlayers.h"
#include "../DRODLib/GameConstants.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Ports.h>
#include <BackEndLib/Date.h>
#include <string>
#if !defined(__native_client__)
#include <SDL_syswm.h>
#endif

#if defined(__native_client__) && defined(__GLIBC__)
#define stricmp strcasecmp
#endif

using namespace std;

//For running only a single instance of the app.
#ifdef WIN32
//This value is used to set a window property of DROD so it will be distinguished from windows of
//other applications.  The value is arbitrary--in this case it is just hex encoding for
//the text "Drod".
static const HANDLE hDrodWindowProp = (HANDLE)0x44726F64;  //Drod
#endif
#ifdef __linux__
static char *lockfile = NULL;
#endif

//
//Module-scope vars.
//

CFiles *	m_pFiles = NULL;

#ifdef BETA
char windowTitle[] = "DROD BETA BUILD 40 - DO NOT DISTRIBUTE (buggy releases make us look bad)";
#else
char windowTitle[] = "DROD";
#endif

//
//Private function prototypes.
//

static MESSAGE_ID   CheckAvailableMemory();
static void         Deinit();
static void         DeinitDB();
static void         DeinitGraphics();
static void         DeinitSound();
static void         DisplayInitErrorMessage(MESSAGE_ID dwMessageID);
static int          FindArg(int argc, char *argv[], const char *pszArgName);
static void         GetAppPath(const char *pszArg0, WSTRING &wstrAppPath);
static bool         ShouldUpgradeData();
static MESSAGE_ID   Init(const bool bNoFullscreen, const bool bNoSound);
static void         InitCDate(void);
static MESSAGE_ID   InitDB();
static MESSAGE_ID   InitGraphics(const bool bNoFullscreen);
static MESSAGE_ID   InitSound(const bool bNoSound);
static bool         IsAppAlreadyRunning();

//*****************************************************************************
#if defined(__native_client__)
int drod_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    LOGCONTEXT("main");

    bool bNoFullscreen = FindArg(argc, argv, "nofullscreen") != -1;
    bool bNoSound = FindArg(argc, argv, "nosound") != -1;

#ifndef __linux__
    //Disallow running more than one instance of the app at a time.
    if (IsAppAlreadyRunning()) return 1;
#endif

    //Initialize the app.
    WSTRING wstrPath;
    GetAppPath(argv[0], wstrPath);
    m_pFiles = new CFiles(wstrPath.c_str(), wszDROD, wszDROD_VER);
#ifdef __linux__
    //Need init'ed CFiles for the linux part
    if (IsAppAlreadyRunning())
    {
        delete m_pFiles;
        if (lockfile) delete [] lockfile;
        return 1;
    }
#endif
    MESSAGE_ID ret = Init(bNoFullscreen, bNoSound);

	if (ret != MID_Success && ret != MID_DatCorrupted_Restored)
		DisplayInitErrorMessage(ret);
	else
	{
       SCREENTYPE eNextScreen = SCR_None;

       if (ret == MID_DatCorrupted_Restored)
            DisplayInitErrorMessage(ret);

       //Go to player creation or selection screen (if needed), then to title screen.
       //If a player already exists, the NewPlayer screen will be skipped (not activate).
       //If a single player exists, the SelectPlayer screen will be skipped also.
       g_pTheSM->InsertReturnScreen(SCR_Title);
       g_pTheSM->InsertReturnScreen(SCR_SelectPlayer);

       //Find out if setup info is available.  Setup info is written to DROD.INI
       //by the the setup executable.  This information is used by DROD on the first run
       //to perform setup tasks that requiring DROD to at least be partially installed.
       //After the tasks are completed, the information will be removed from DROD.INI.
       if (ShouldUpgradeData())
       {
           //Go to setup screen first, then new player.
           eNextScreen = SCR_Setup;
           g_pTheSM->InsertReturnScreen(SCR_NewPlayer);
       } else {
          eNextScreen = SCR_NewPlayer;
       }

       //Event-handling will happen in the execution of ActivateScreen().
       //ActivateScreen() will return when player exits a screen.
       while (eNextScreen != SCR_None) //If SCR_None, then app is exiting.
       {
           #ifdef _DEBUG
                string strActivatingScreen;
                g_pTheSM->GetScreenName(eNextScreen, strActivatingScreen);
                string strContext = "Active screen: ";
                strContext += strActivatingScreen;
                LOGCONTEXT(strContext.c_str());
           #endif

	       eNextScreen = (SCREENTYPE)g_pTheSM->ActivateScreen(eNextScreen);
       }

        //Show hourglass.
        SDL_SetCursor(g_pTheSM->GetCursor(CUR_Wait));
	}

    //Deinitialize the app.
    Deinit();
    return ret!=MID_Success;
}

//*****************************************************************************
static MESSAGE_ID Init(
//Top-level function for all application initialization.
//
//Init() may leave app in a partially initialized state, which caller should
//clean up with Deinit().  Init() does not call Deinit() here because a partial
//initialization can be useful for reporting errors to the user after init
//returns.
//
//Params:
  const bool bNoFullscreen,         //(in)  If true, then app will run windowed regardless
                              //      of player settings.
  const bool bNoSound)              //(in)  If true, then all sound will be disabled
                              //      regardless of player settings.
//
//Returns:
//MID_Success or Message ID of a failure message to display to user.
{
    LOGCONTEXT("Init");

	MESSAGE_ID ret;
    bool bRestoredFromCorruption = false;

    //Check memory availability.  App may need to exit or a warning message may be needed.
    ret = CheckAvailableMemory();
    if (ret == MID_MemLowExitNeeded)
        return ret;
    if (ret == MID_MemLowWarning || ret == MID_MemPerformanceWarning)
        DisplayInitErrorMessage(ret); //Init() can continue after the warning.

	//Open the database before other things, because it gives access to
	//localized messages that can be used to describe any Init() failures.
	ret = InitDB();
    if (ret == MID_DatCorrupted_Restored)
        bRestoredFromCorruption = true;
    else if (ret) return ret;

	//Initialize graphics before other things, because I want to show the
	//screen quickly for the user.
	ret = InitGraphics(bNoFullscreen);
	if (ret) return ret;

	//Initialize sound.  Music will not play until title screen loads.
	ret = InitSound(bNoSound);
	if (ret) return ret;

	//Enable international support.
	SDL_EnableUNICODE(1);

	//Success.
	return (bRestoredFromCorruption) ? MID_DatCorrupted_Restored : MID_Success;
}

//*****************************************************************************
static void Deinit()
//Top-level function for all application deinitialization.
//
//Deinit() should be able to handle partial initialization without problems.
//All error reporting should be done through CFiles::AppendErrorLog().
{
    LOGCONTEXT("Deinit");

	//Generally, deinit subsystems in reverse order of their initialization.
	DeinitSound();
	DeinitGraphics();
	DeinitDB();

	delete m_pFiles;
}

//*****************************************************************************
static MESSAGE_ID InitDB()
//Initialize the database so that it is available for the entire app.
//
//Returns:
//MID_Success or an error message ID.
{
    LOGCONTEXT("InitDB");
	ASSERT(!g_pTheDB);

	//Create hostage reference to database that will be around for life of app.
	g_pTheDB = new CDb;
	if (!g_pTheDB) return MID_OutOfMemory;

	MESSAGE_ID ret = g_pTheDB->Open();
	if (ret == MID_DatMissing)
	{
		//Try to find correct location of drod1_6.dat and fix incorrect DataPath.txt file
        CFiles Files;
		Files.TryToFixDataPath();
		ret = g_pTheDB->Open(); //Uses data path set in above call to find location of DB files.
	}

    //Initialize the CDate class with month text from database.
    if (g_pTheDB->IsOpen()) InitCDate();

	ASSERT(ret != MID_Success || g_pTheDB->IsOpen());
	return ret;
}

//*****************************************************************************
static void DeinitDB()
//Deinits the database.
{
    LOGCONTEXT("DeinitDB");

	//Release hostage reference, causing database to close.
	if (g_pTheDB)
	{
		g_pTheDB->Close();
		delete g_pTheDB;
		g_pTheDB = NULL;
	}

	//GetDbRefCount() returns the number of CDbBase-derived objects still in
	//memory.  If not zero, then there are some objects that should have been
	//deleted somewhere.
	ASSERT(GetDbRefCount()==0L);
}

//*****************************************************************************
static MESSAGE_ID InitGraphics(
//Initializes SDL, screen manager, bitmap manager, font manager, and brings up
//a window.
//
//Params:
  const bool bNoFullscreen) //(in)  If true, then fullscreen window will not be used.
//
//Returns:
//MID_Success or an error message ID.
{
    LOGCONTEXT("InitGraphics");

	char szErrMsg[80];
	//Initialize the library.
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
        CFiles Files;
		sprintf(szErrMsg, "Couldn't initialize SDL: %s\n", SDL_GetError());
		Files.AppendErrorLog(szErrMsg);
		return MID_SDLInitFailed;
	}

	//Set icon.
	{
      static const WCHAR wszBmps[] = {W_t('B'),W_t('i'),W_t('t'),W_t('m'),W_t('a'),W_t('p'),W_t('s'),W_t(0)};
		static const WCHAR wszIcon[] = {W_t('I'),W_t('c'),W_t('o'),W_t('n'),W_t('.'),W_t('b'),W_t('m'),W_t('p'),W_t(0)};
		WSTRING wstrIconFilepath;
        CFiles Files;
		wstrIconFilepath = Files.GetResPath();
		wstrIconFilepath += wszSlash;
		wstrIconFilepath += wszBmps;
		wstrIconFilepath += wszSlash;
		wstrIconFilepath += wszIcon;
		CStretchyBuffer bitmap;
		CFiles::ReadFileIntoBuffer(wstrIconFilepath.c_str(), bitmap);
		SDL_WM_SetIcon(SDL_LoadBMP_RW(SDL_RWFromMem((BYTE*)bitmap, bitmap.Size()), 0), NULL);
	}

	//Get a 640x480x24 screen.  Decide whether it's fullscreen or not.
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
   bool bFullscreen = !bNoFullscreen && (pCurrentPlayer ? pCurrentPlayer->Settings.GetVar(
			"Fullscreen", false) : false);
	const Uint32 flags = bFullscreen ? SDL_FULLSCREEN : 0;
	delete pCurrentPlayer;
	SDL_Surface *pScreenSurface = SDL_SetVideoMode(640, 480, 24, flags);

   if (!pScreenSurface)
	{
        CFiles Files;
		sprintf(szErrMsg, "Couldn't set 640x480x24 video mode: %s\n",
	   	SDL_GetError());
		Files.AppendErrorLog(szErrMsg);
		return MID_SDLInitFailed;
	}
	SDL_WM_SetCaption(windowTitle, NULL);

	//Init the bitmap manager.
	ASSERT(!g_pTheBM);
	g_pTheDBM = new CDrodBitmapManager();
	g_pTheBM = (CBitmapManager*)g_pTheDBM;
	if (!g_pTheBM) return MID_OutOfMemory;
	MESSAGE_ID ret = (MESSAGE_ID)g_pTheDBM->Init();
	if (ret) return ret;

	//Init the font manager.
	ASSERT(!g_pTheFM);
	g_pTheDFM = new CDrodFontManager();
	g_pTheFM = (CFontManager*)g_pTheDFM;
	if (!g_pTheFM) return MID_OutOfMemory;
	ret = (MESSAGE_ID)g_pTheDFM->Init();
	if (ret) return ret;

	//Init the screen manager.
	ASSERT(!g_pTheSM);
	g_pTheDSM = new CDrodScreenManager(pScreenSurface);
	g_pTheSM = (CScreenManager*)g_pTheDSM;
	if (!g_pTheSM) return MID_OutOfMemory;
	ret = (MESSAGE_ID)g_pTheDSM->Init();
	if (ret) return ret;

    //In Windows, set focus to window in case it was lost during startup.  This is to avoid
    //losing the input focus.
#   ifdef WIN32
    {
        SDL_SysWMinfo Info;
        SDL_VERSION(&Info.version);
        SDL_GetWMInfo(&Info);
        HWND hwndRet = SetFocus(Info.window);
        if (CFiles::WindowsCanBrowseUnicode()) {
           WSTRING wstr;
           AsciiToUnicode("DrodProp", wstr);
           HANDLE hProp = hDrodWindowProp;
           SetPropW(Info.window, wstr.c_str(), hProp);
        }
        else {
           SetPropA(Info.window, "DrodProp", hDrodWindowProp);
        }
    }
#   endif

	//Success.
	return MID_Success;
}

//*****************************************************************************
static void DeinitGraphics()
//Deinit graphics.
{
    LOGCONTEXT("DeinitGraphics");

	//Delete the screen manager.
	if (g_pTheSM)
	{
		delete g_pTheSM;
		g_pTheSM = NULL;
	}

	//Delete the font manager.
	if (g_pTheFM)
	{
		delete g_pTheFM;
		g_pTheFM = NULL;
	}

	//Delete the bitmap manager.
	if (g_pTheBM)
	{
		delete g_pTheBM;
		g_pTheBM = NULL;
	}

	SDL_Quit();
}

//*****************************************************************************
static MESSAGE_ID InitSound(
//After this call, g_pTheSound can be used.
//
//Params:
  const bool bNoSound)  //(in)  If true, then all sound will be disabled.  Unlike,
                  //      partial disablement, this will prevent any sound library
                  //      calls to be made during this session.
//
//Returns:
//MID_Success or an error message ID.
{
    LOGCONTEXT("InitSound");
	ASSERT(!g_pTheSound);

	//Create global instance of CSound object.
	g_pTheSound = (CSound*)new CDrodSound(bNoSound);
	ASSERT(g_pTheSound);
	if (bNoSound) return MID_Success;

	//CSound() disables itself if a failure occurs during construction.
	//Future calls to a disabled g_pTheSound don't do anything, which is okay.
	//Sound is not a big enough deal to fail app initialization for.  The
	//player can not have sound, and still be able to play the game.

	//Disable sound if player settings say to.
	CDbPlayer *pPlayer = g_pTheDB->GetCurrentPlayer();
	if (pPlayer)
	{
		g_pTheSound->EnableSoundEffects(pPlayer->Settings.GetVar("SoundEffects", true)!=false);
		g_pTheSound->SetSoundEffectsVolume(pPlayer->Settings.GetVar("SoundEffectsVolume", (BYTE)127));
		g_pTheSound->EnableMusic(pPlayer->Settings.GetVar("Music", true)!=false);
		g_pTheSound->SetMusicVolume(pPlayer->Settings.GetVar("MusicVolume", (BYTE)127));
		delete pPlayer;
	} else {
		//First time startup.
		g_pTheSound->EnableSoundEffects(true);
		g_pTheSound->SetSoundEffectsVolume(127);
		g_pTheSound->EnableMusic(true);
		g_pTheSound->SetMusicVolume(127);
	}

	return MID_Success;
}

//*****************************************************************************
static void DeinitSound()
//Deinits sound.
{
    LOGCONTEXT("DeinitSound");
	if (g_pTheSound)
	{
		g_pTheSound->WaitForSoundEffectsToStop();
		delete g_pTheSound;
		g_pTheSound = NULL;
	}
}

//*****************************************************************************
static void DisplayInitErrorMessage(
//Displays an error message for the user, taking into account the possible
//states of partial initialization the application may be in.
//
//Params:
	MESSAGE_ID dwMessageID)	//(in)	Message to display.
{
	//Get message text from database if possible.
	const WCHAR *pwczMessage = NULL;
	if (dwMessageID > MID_LastUnstored)
		pwczMessage = g_pTheDB->GetMessageText(dwMessageID);

	//Can't get message from database, so there is a database failure.  The
	//error should involve the database.  Provide hard-coded text to
	//describe this.
	WSTRING wstrMessage;
	if (!pwczMessage)
	{
		switch (dwMessageID)
		{
			case MID_DatMissing:
				AsciiToUnicode("Couldn't find DROD data.  This problem might be corrected by "
						"reinstalling DROD.", wstrMessage);
			break;

			case MID_DatNoAccess:
				AsciiToUnicode("Couldn't access DROD data.  If you are running DROD from a networked "
						"location, this could be a cause of the problem.  It's also possible that another "
						"application is trying to access DROD data at the same time; you may wish to "
						"retry running DROD after other applications have been closed.", wstrMessage);
			break;

            case MID_DatCorrupted_NoBackup:
                AsciiToUnicode("Your DROD data was corrupted due to an error.  DROD tried to restore "
                        "from the last good copy of the data, but the operation failed.  I recommend "
                        "reinstalling DROD, but unfortunately, you will lose all your data.", wstrMessage);
            break;

            case MID_DatCorrupted_Restored:
                AsciiToUnicode("Your DROD data was corrupted due to an error, so it was necessary to "
                        "restore from the last good copy of the data. Unfortunately, you've lost saved "
                        "games and other changes from your last session.", wstrMessage);
            break;


			case MID_CouldNotOpenDB:
				AsciiToUnicode("Couldn't open DROD data.  This points to corruption of a required "
						"DROD file.  This problem might be corrected by reinstalling DROD.",
						wstrMessage);
			break;

            case MID_MemPerformanceWarning:
                AsciiToUnicode("DROD should run without any problems, but its performance may be improved "
                        "by freeing memory on your system.  If there are any open applications you can "
                        "close, this would help.", wstrMessage);
            break;

            case MID_MemLowWarning:
                AsciiToUnicode("Your system is running a little low on memory.  DROD will probably run "
                        "without problems, but if other applications are started while DROD is running, "
                        "you might see some crashes.  To avoid this kind of thing and get better "
                        "performance from DROD, you could close other applications that are now open.",
                        wstrMessage);
            break;

            case MID_MemLowExitNeeded:
                AsciiToUnicode("There is not enough memory to run DROD.  It might help to close other "
                        "applications that are now open, and try running DROD again.  DROD will now exit.",
                        wstrMessage);

			default:
				AsciiToUnicode("An unexpected error occurred, and DROD was not able to retrieve a "
						"description of the problem.  This problem might be corrected by "
						"reinstalling DROD.", wstrMessage);
				ASSERTP(false, "Unexpected MID value."); //Probably forgot to add a message to the database.
		}
		pwczMessage = wstrMessage.c_str();
	}
	else
	{
		wstrMessage = pwczMessage;
	}

  //Switch to windowed mode if in fullscreen.
   {
      SDL_Surface *pScreenSurface = GetWidgetScreenSurface();
      if (pScreenSurface && (pScreenSurface->flags & SDL_FULLSCREEN) != 0)
      {
         pScreenSurface = SDL_SetVideoMode(640, 480, 24, 0);
         if (pScreenSurface)
            SetWidgetScreenSurface(pScreenSurface);
      }
   }

	//O/S-specific code to display an error message.
	WSTRING wstrTitle;
	AsciiToUnicode("Problem Starting DROD", wstrTitle);
#ifdef WIN32
	MessageBoxW(NULL, pwczMessage, wstrTitle.c_str(), MB_OK | MB_ICONEXCLAMATION);
#else
	char buffer[1024];
	UnicodeToAscii(wstrMessage, buffer);
	fprintf(stderr, "%s\n", buffer);
	#warning Message display code is not provided. Using the console.
#endif
}

//*****************************************************************************
static int FindArg(
//Find an argument by its name.
//
//Params:
  int argc,                 //(in)  From main.
  char *argv[],             //(in)  From main.
  const char *pszArgName)   //(in)  Check for this arg--case insensitive.
//
//Returns:
//-1 if argument was not found, or arg index if found.
{
   for (int nArgNo=0; nArgNo < argc; ++nArgNo)
      if (stricmp(argv[nArgNo],pszArgName)==0) return nArgNo;
   return -1;
}

//*****************************************************************************
static bool ShouldUpgradeData()
{
    CFiles files;
    string strStatus;
    if (!files.GetGameProfileString("Setup", "Status", strStatus)) return false;
    return (strStatus != "complete" && strStatus != "cancelled");
}

//*****************************************************************************
#ifdef WIN32
BOOL CALLBACK DetectDrodWindow(HWND hwnd, LPARAM lParam)
{
   if (CFiles::WindowsCanBrowseUnicode())
   {
        WSTRING wstr;
        AsciiToUnicode("DrodProp", wstr);
        HANDLE hProp = GetPropW(hwnd, wstr.c_str());
        if (hProp == hDrodWindowProp) {
           // found a DROD window with the right magic number - bring it to the top.
           SetForegroundWindow(hwnd);
           WINDOWPLACEMENT wpl;
           GetWindowPlacement(hwnd, &wpl);
           wpl.showCmd = SW_RESTORE;
           SetWindowPlacement(hwnd, &wpl);
           return FALSE;
        } else {
           // not the right window - return TRUE to continue processing windows
           return TRUE;
        }
   }
   else
   {
        HANDLE hProp = GetPropA(hwnd, "DrodProp");
        if (hProp == hDrodWindowProp) {
           // found a DROD window with the right magic number - bring it to the top.
           SetForegroundWindow(hwnd);
           WINDOWPLACEMENT wpl;
           GetWindowPlacement(hwnd, &wpl);
           wpl.showCmd = SW_RESTORE;
           SetWindowPlacement(hwnd, &wpl);
           return FALSE;
        } else {
           // not the right window - return TRUE to continue processing windows
           return TRUE;
        }
   }
}
#endif

//*****************************************************************************
#ifdef __linux__
// atexit callback
static void DeleteLockFile()
{
	ASSERT(lockfile);
	unlink(lockfile);
	delete [] lockfile;
}
#endif

//*****************************************************************************
static bool IsAppAlreadyRunning()
{
#ifdef WIN32
   return !EnumWindows(DetectDrodWindow, 0);
#elif defined __linux__
	// Use a lock file (Pre-Cond: need inited CFiles!)
	WSTRING tmp = m_pFiles->GetDatPath();
	tmp += wszSlash;
	tmp += wszDROD;
	tmp += (WCHAR[]){{'.'},{'p'},{'i'},{'d'},{0}};
	const UINT lflen = tmp.length();
	if (!(lockfile = new char[lflen + 1])) return true;
	UnicodeToAscii(tmp, lockfile);
	// Try opening an existing lockfile first
	FILE *fp = fopen(lockfile, "r");
	char pidst[128];
	strcpy(pidst, "/proc/");
	if (fp)
	{
		size_t pidlen = fread(pidst + 6, sizeof(char), 122, fp);
		pidst[pidlen + 6] = 0;
		bool bPidOk = pidlen;
		fclose(fp);
		// Make sure this is a decimal number
		while (pidlen--)
			if (pidst[pidlen + 6] < '0' || pidst[pidlen + 6] > '9')
			{
				bPidOk = false;
				break;
			}
		if (bPidOk)
		{
			if ((fp = fopen(pidst, "r"))) fclose(fp);
			if (fp || errno == EISDIR)
			{
				fprintf(stderr, "DROD is already running.\n");
				return true;
			}
		}
		// Corrupt lockfile or nonexistant pid; ignore it
		unlink(lockfile);
	}
	char tmplockfile[lflen + 64];
	strcpy(tmplockfile, lockfile);
	sprintf(tmplockfile + lflen, "%u", getpid());
	// Create a new lockfile
	fp = fopen(tmplockfile, "w");
	if (!fp || fwrite(tmplockfile + lflen,
			strlen(tmplockfile + lflen) * sizeof(char), 1, fp) != 1)
	{
		if (fp) fclose(fp);
		fprintf(stderr, "Couldn't create lock file, check permissions (%s).\n", tmplockfile);
		return true;
	}
	fclose(fp);
	// Atomic write operation
	if (link(tmplockfile, lockfile))
	{
		struct stat st;
		if (stat(tmplockfile, &st) || st.st_nlink != 2)
		{
			unlink(tmplockfile);
			fprintf(stderr, "DROD is already running.\n");
			return true;
		}
	}
	// Success
	unlink(tmplockfile);
	atexit(DeleteLockFile);
	return false;
#elif defined(__native_client__)
        return false;
#else
#  error Disallow running more than one instance of the app at a time.
#endif
}

//*****************************************************************************
static void GetAppPath(
    const char *pszArg0,    //(in)  First command-line argument which will be used
                            //      to find application path if it is the best
                            //      available way.
    WSTRING &wstrAppPath)   //(out) App path.
{

    //
    //Try to use an O/S-specific means of getting the application path.
    //

#ifdef __linux__
	char exepath[MAX_PATH];
	int len = readlink("/proc/self/exe", exepath, MAX_PATH - 1);
	if (len && len != -1)
	{
		exepath[len] = 0;
		AsciiToUnicode(exepath, wstrAppPath);
		return;
	}
#elif (defined WIN32)
    WCHAR wszPathBuffer[MAX_PATH+1];
    if (GetModuleFileName(NULL, wszPathBuffer, MAX_PATH))
    {
        wstrAppPath = wszPathBuffer;
        return;
    }
    else //On older versions of Windows, Unicode functions fail.
    {
        char szPathBuffer[MAX_PATH+1];
        if (GetModuleFileNameA(NULL, szPathBuffer, MAX_PATH))
        {
            AsciiToUnicode(szPathBuffer, wstrAppPath);
            return;
        }
    }
#endif

    //Fallback solution--use the command-line argument.
    AsciiToUnicode(pszArg0, wstrAppPath);
}

//*****************************************************************************
static MESSAGE_ID CheckAvailableMemory()
//Checks if there is enough memory for DROD to run.
//
//Returns:
//MID_Success                   If there is plenty of memory.
//MID_MemPerformanceWarning     If there is enough memory, but performance may
//                              suffer, i.e. virtual memory is used.
//MID_MemLowWarning             There is barely enough memory.
//MID_MemLowExitNeeded          There is not enough memory to start DROD.
{
#if defined(__native_client__)
  return MID_Success;
#else
    //The amount of memory used before Init() is called.  In other words, after
    //libraries and the executable have been loaded into memory, but no code has
    //been executed that would require additional memory.  This is the time when
    //CheckAvailableMemory() is called.
#if defined(WIN32) || defined(__linux__)
#  ifdef __linux__
#warning TODO: Verify this for linux.
#  endif
   const DWORD MEM_USED_BEFORE_INIT = 5000000; //5mb
#else
#  error Define MEM_USED_BEFORE_INIT for this platform.
#endif

    //The estimated amount of memory DROD will need when fully loaded.  To get a good
    //estimate of this amount, you need to visit all of the screens in the application.
    //In 1.7, screens are unloaded when not used, but in 1.6 they remain in memory.
#if defined(WIN32) || defined(__linux__)
#  ifdef __linux__
#warning TODO: Verify this for linux.
#  endif
   const DWORD MEM_USED_FULLY_LOADED = 28000000; //28mb
#else
#  error Define MEM_USED_FULLY_LOADED for this platform.
#endif

    //How much memory will be needed later in execution?
    const DWORD MEM_STILL_NEEDED = MEM_USED_FULLY_LOADED - MEM_USED_BEFORE_INIT;

    //How much additional memory is needed to not worry about low-memory conditions
    //caused by other apps and background O/S functions.
    const DWORD MEM_COMFORT_PAD = 2000000; //2mb

    //Get available virtual and physical memory.
#ifdef WIN32
    DWORD dwAvailablePhysical, dwAvailableTotal;
    MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    dwAvailablePhysical = ms.dwAvailPhys;
    dwAvailableTotal = ms.dwAvailVirtual; //Includes physical + virtual-only memory.
#elif defined(__linux__)
	unsigned int dwAvailablePhysical, dwAvailableTotal, dwCache;
	FILE *fp = fopen("/proc/meminfo", "r");
	//Try new meminfo format first (in kb)
	if (fp && fscanf(fp, "MemTotal: %*u kB\nMemFree: %u kB\n"
			"Buffers: %*u kB\nCached: %u kB\nSwapCached: %*u kB\n"
			"Active: %*u kB\nInactive: %*u kB\nHighTotal: %*u kB\n"
			"HighFree: %*u kB\nLowTotal: %*u kB\nLowFree: %*u kB\n"
			"SwapTotal: %*u kB\nSwapFree: %u kB", //skip the rest
			&dwAvailablePhysical, &dwCache, &dwAvailableTotal) == 3)
	{
		fclose(fp);
		dwAvailableTotal = (dwAvailableTotal + dwAvailablePhysical) * 1024;
		dwAvailablePhysical = (dwAvailablePhysical + dwCache) * 1024;
	}
	//Fall back to old meminfo format (in bytes)
	else if (fp && (!fclose(fp) || true) && (fp = fopen("/proc/meminfo", "r"))
			&& fscanf(fp, " total: used: free: shared: buffers: cached:\n"
			"Mem: %*u %*u %u %*u %*u %u\nSwap: %*u %*u %u", //skip the rest
			&dwAvailablePhysical, &dwCache, &dwAvailableTotal) == 3)
	{
		fclose(fp);
		dwAvailableTotal += dwAvailablePhysical;
		dwAvailablePhysical += dwCache;
	} else {
		if (fp) fclose(fp);
		// Not available or wrong format, cheat
		fprintf(stderr, "Warning: Couldn't get memory information, assuming it's ok.\n");
		dwAvailableTotal = dwAvailablePhysical = MEM_STILL_NEEDED + MEM_COMFORT_PAD;
	}
#else
#   error Need code to get available virtual and physical memory for this platform.
#endif

    //Is there enough memory left to run DROD?
    if (dwAvailableTotal < MEM_STILL_NEEDED) //No.
        return MID_MemLowExitNeeded;
    else //Yes.
    {
        //Comfortably?
        if (dwAvailableTotal < MEM_STILL_NEEDED + MEM_COMFORT_PAD) //No.
            return MID_MemLowWarning;
        else //Yes, DROD can run without fear of sudden death.
        {
            //But will it run fast?
            if (dwAvailablePhysical < MEM_STILL_NEEDED + MEM_COMFORT_PAD) //Probably not.
                return MID_MemPerformanceWarning;
            else //Probably.
                return MID_Success;
        }
    }
#endif  // __native_client__
}

//*****************************************************************************
static void InitCDate(void)
//Initialize CDate class with month name texts.
{
    LOGCONTEXT("InitCDate");

    WSTRING wstrMonthNames[MONTH_COUNT];
    const WCHAR * pwzMonthNames[MONTH_COUNT];
    wstrMonthNames[0] = g_pTheDB->GetMessageText(MID_January);
    wstrMonthNames[1] = g_pTheDB->GetMessageText(MID_February);
    wstrMonthNames[2] = g_pTheDB->GetMessageText(MID_March);
    wstrMonthNames[3] = g_pTheDB->GetMessageText(MID_April);
    wstrMonthNames[4] = g_pTheDB->GetMessageText(MID_May);
    wstrMonthNames[5] = g_pTheDB->GetMessageText(MID_June);
    wstrMonthNames[6] = g_pTheDB->GetMessageText(MID_July);
    wstrMonthNames[7] = g_pTheDB->GetMessageText(MID_August);
    wstrMonthNames[8] = g_pTheDB->GetMessageText(MID_September);
    wstrMonthNames[9] = g_pTheDB->GetMessageText(MID_October);
    wstrMonthNames[10] = g_pTheDB->GetMessageText(MID_November);
    wstrMonthNames[11] = g_pTheDB->GetMessageText(MID_December);
    for (UINT wMonthNo = 0; wMonthNo < MONTH_COUNT; ++wMonthNo)
        pwzMonthNames[wMonthNo] = wstrMonthNames[wMonthNo].c_str();

    CDate::InitClass(pwzMonthNames);
}


// $Log: Main.cpp,v $
// Revision 1.80  2004/07/18 13:31:31  gjj
// New Linux data searcher (+ copy data to home if read-only), made some vars
// static, moved stuff to .cpp-files to make the exe smaller, couple of
// bugfixes, etc.
//
// Revision 1.79  2003/12/02 00:08:52  erikh2000
// Reduced threshold of available memory needed for DROD to start without complaint.
//
// Revision 1.78  2003/10/27 20:22:05  mrimer
// Tweaking.
//
// Revision 1.77  2003/10/07 21:12:08  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.76  2003/10/06 02:44:19  erikh2000
// Changes to database initialization.
//
// Revision 1.75  2003/09/11 02:29:59  mrimer
// Added const to some method params.
//
// Revision 1.74  2003/09/11 02:02:25  mrimer
// Linux fixes (committed on behlaf of Gerry JJ).
//
// Revision 1.73  2003/09/05 20:05:25  erikh2000
// Fixed a few bugs in the new memory check function.
//
// Revision 1.72  2003/09/05 19:27:21  schik
// Changed detection of already running drod avoid depending on window names.
//
// Revision 1.71  2003/09/05 18:43:23  erikh2000
// Added memory check code.
//
// Revision 1.70  2003/09/03 21:39:18  erikh2000
// Removed try...catch code from main().
//
// Revision 1.69  2003/08/27 18:35:37  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.68  2003/08/26 21:54:31  schik
// Changed IsAppAlreadyRunning() to detect the DROD app only, not just a window named the same.
//
// Revision 1.67  2003/08/25 18:07:23  erikh2000
// Changed the get-app-path logic so it would avoid 8.3 filenames on Win98/95.
//
// Revision 1.66  2003/08/25 14:28:08  mrimer
// Fixed bug (tentative): not getting app path correctly on Windows systems w/o Unicode support.
//
// Revision 1.65  2003/08/20 22:42:35  mrimer
// Reinserted GetModuleFileName() for Windows to get the correct path on startup.
//
// Revision 1.64  2003/08/19 20:11:29  mrimer
// Linux maintenance (committed on behalf of Gerry JJ).
//
// Revision 1.63  2003/08/12 23:18:17  erikh2000
// Updated window caption for build 36.  This got checked in just after "Build_36" tag.
//
// Revision 1.62  2003/08/07 20:13:24  erikh2000
// Fixed (maybe) a problem with losing input focus.
//
// Revision 1.61  2003/08/07 16:54:08  mrimer
// Added Data/Resource path seperation (committed on behalf of Gerry JJ).
//
// Revision 1.60  2003/08/05 00:31:15  erikh2000
// Changed window caption for build 35.
//
// Revision 1.59  2003/08/01 02:35:18  erikh2000
// Updated window caption for latest build.
//
// Revision 1.58  2003/07/27 21:57:41  erikh2000
// Changed window caption for build 33.
//
// Revision 1.57  2003/07/24 02:48:53  erikh2000
// Updated caption for latest build.
//
// Revision 1.56  2003/07/21 23:46:31  erikh2000
// Updated window caption with new build#.
//
// Revision 1.55  2003/07/19 22:50:04  erikh2000
// Updated window caption for build 30.
//
// Revision 1.54  2003/07/15 01:01:49  erikh2000
// Updated window caption for build 29.
//
// Revision 1.53  2003/07/13 22:34:37  erikh2000
// Updated title bar for build 28.
//
// Revision 1.52  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.51  2003/07/06 18:04:01  erikh2000
// Updated window caption for build 27.
//
// Revision 1.50  2003/07/03 08:07:11  mrimer
// Fixed some assertions in the screen list logic.
//
// Revision 1.49  2003/07/02 21:08:35  schik
// The code from CNewPlayerScreen is now in CSelectPlayerScreen.
// CNewPlayerScreen now is a different dialog with fewer options.
//
// Revision 1.48  2003/06/29 05:32:52  schik
// If an instance of DROD exists, restore the window and bring it to the top.
//
// Revision 1.47  2003/06/28 20:08:03  erikh2000
// Changed build# in window caption.
//
// Revision 1.46  2003/06/28 18:43:49  mrimer
// Linux port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.45  2003/06/23 18:23:36  erikh2000
// Added build# to window caption.
//
// Revision 1.44  2003/06/21 06:52:14  mrimer
// Revised exception handling code.
//
// Revision 1.43  2003/06/20 23:56:06  mrimer
// Now also Close() the database on exception catch.
//
// Revision 1.42  2003/06/17 21:50:00  mrimer
// Fixed inconsistent spelling.
//
// Revision 1.41  2003/06/16 20:16:21  erikh2000
// Changed main() logic to always use first command-line param to determine app path.
//
// Revision 1.40  2003/06/09 23:57:05  mrimer
// Added #ifndef _DEBUG to the try/catch block (to find location of access violations when debugging).
//
// Revision 1.39  2003/06/06 19:43:04  mrimer
// Added enforcing only one instance of the app on Windows.
//
// Revision 1.38  2003/06/06 18:22:02  mrimer
// Added a check on making a windowed screen.
//
// Revision 1.37  2003/05/30 04:05:25  mrimer
// Tweaking.
//
// Revision 1.36  2003/05/30 01:02:52  mrimer
// Added exception catching to exit more robustly on error.
//
// Revision 1.35  2003/05/29 03:38:49  erikh2000
// Fixed a logic error causing setup screen to come up when it shouldn't.
//
// Revision 1.34  2003/05/28 23:07:04  erikh2000
// The first screen will be setup screen if data needs to be upgraded.
// Hourglass shown when app exits.
//
// Revision 1.33  2003/05/26 01:56:18  erikh2000
// Changed calls that get profile strings.
//
// Revision 1.32  2003/05/25 22:48:12  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.31  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.30  2003/05/22 23:35:35  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.29  2003/05/08 23:28:18  erikh2000
// Two optional command-line options "nosound" and "nofullscreen" will disable sound and start in windowed mode, respectively.
//
// Revision 1.28  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.27  2003/04/08 13:08:28  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.26  2003/04/06 03:54:00  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.25  2003/02/24 17:15:06  erikh2000
// Database is opened without specifying a filepath param.
//
// Revision 1.24  2003/02/17 03:28:09  erikh2000
// Removed L() macro.
//
// Revision 1.23  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.22  2002/12/22 02:37:13  mrimer
// Removed unneeded includes.
//
// Revision 1.21  2002/11/15 03:07:59  mrimer
// Added multiple player handling, and for case if no players exist yet.
//
// Revision 1.20  2002/10/21 20:25:04  mrimer
// Added call to SDL_EnableUNICODE(1).
//
// Revision 1.19  2002/09/24 21:44:02  mrimer
// Added CFiles member var.  Revised handling of CFiles method calls.
//
// Revision 1.18  2002/09/05 18:52:58  mrimer
// When DROD starts the first time, asks for player's name.
//
// Revision 1.17  2002/08/30 22:41:56  erikh2000
// Sound effects (button click) will finish playing before application exits.
//
// Revision 1.16  2002/08/30 00:26:47  erikh2000
// Caption will display different text when BETA preproc define is present.
//
// Revision 1.15  2002/08/23 23:39:39  erikh2000
// Changed calls to match renamed members of CSound.
//
// Revision 1.14  2002/07/20 23:12:24  erikh2000
// Revised #includes.
//
// Revision 1.13  2002/06/25 05:46:21  mrimer
// Revised #includes.
//
// Revision 1.12  2002/06/15 18:32:51  erikh2000
// Changed database instance to a pointer instead of module-scope var to have more control of when its destroyed.
// Added reference count checking on CDbBase-derived classes.
// Added explicit call to CDb::Close() which will guarantee database writes are committed before exiting.
//
// Revision 1.11  2002/06/11 22:55:05  mrimer
// Allow starting in full screen mode.
//
// Revision 1.10  2002/06/09 06:43:33  erikh2000
// Music and sound effects are now sett from DB-stored player settings at start.
//
// Revision 1.9  2002/05/15 23:13:37  mrimer
// Set application icon to Icon.bmp.
//
// Revision 1.8  2002/05/10 22:37:29  erikh2000
// Changed window creation code so that window is not resizable.
//
// Revision 1.7  2002/04/25 18:14:05  mrimer
// Allow app window to be maximized.
//
// Revision 1.6  2002/04/25 09:30:52  erikh2000
// Added code to disable sound effects/music if DROD.INI says to.
//
// Revision 1.5  2002/04/20 08:24:18  erikh2000
// Fixed a problem caused by initializing screen manager before font manager.
//
// Revision 1.4  2002/04/16 10:45:55  erikh2000
// Font manager is initialized and deinitialized.
// Error messages during init are now displayed--loaded from database if possible.
//
// Revision 1.3  2002/04/11 10:18:00  erikh2000
// Added initialization for the bitmap manager.
//
// Revision 1.2  2002/04/09 10:05:38  erikh2000
// Fixed revision log macro.
//
