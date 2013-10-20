# Microsoft Developer Studio Project File - Name="DRODLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DRODLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DRODLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DRODLib.mak" CFG="DRODLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DRODLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DRODLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DRODLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DRODLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "UNICODE" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "DRODLib - Win32 Release"
# Name "DRODLib - Win32 Debug"
# Begin Group "General"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Compat.cpp
# End Source File
# Begin Source File

SOURCE=.\Compat.h
# End Source File
# Begin Source File

SOURCE=.\CueEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\CueEvents.h
# End Source File
# Begin Source File

SOURCE=.\CurrentGame.cpp
# End Source File
# Begin Source File

SOURCE=.\CurrentGame.h
# End Source File
# Begin Source File

SOURCE=.\GameConstants.h
# End Source File
# Begin Source File

SOURCE=.\Swordsman.cpp
# End Source File
# Begin Source File

SOURCE=.\Swordsman.h
# End Source File
# Begin Source File

SOURCE=.\TileConstants.h
# End Source File
# End Group
# Begin Group "Data Access"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Db.cpp
# End Source File
# Begin Source File

SOURCE=.\Db.h
# End Source File
# Begin Source File

SOURCE=.\DbBase.cpp
# End Source File
# Begin Source File

SOURCE=.\DbBase.h
# End Source File
# Begin Source File

SOURCE=.\DbCommands.cpp
# End Source File
# Begin Source File

SOURCE=.\DbCommands.h
# End Source File
# Begin Source File

SOURCE=.\DbDemos.cpp
# End Source File
# Begin Source File

SOURCE=.\DbDemos.h
# End Source File
# Begin Source File

SOURCE=.\DbHolds.cpp
# End Source File
# Begin Source File

SOURCE=.\DbHolds.h
# End Source File
# Begin Source File

SOURCE=.\DbLevels.cpp
# End Source File
# Begin Source File

SOURCE=.\DbLevels.h
# End Source File
# Begin Source File

SOURCE=.\DbMessageText.cpp
# End Source File
# Begin Source File

SOURCE=.\DbMessageText.h
# End Source File
# Begin Source File

SOURCE=.\DbPackedVars.cpp
# End Source File
# Begin Source File

SOURCE=.\DbPackedVars.h
# End Source File
# Begin Source File

SOURCE=.\DbPlayers.cpp
# End Source File
# Begin Source File

SOURCE=.\DbPlayers.h
# End Source File
# Begin Source File

SOURCE=.\DBProps.h
# End Source File
# Begin Source File

SOURCE=.\DbRefs.cpp
# End Source File
# Begin Source File

SOURCE=.\DbRefs.h
# End Source File
# Begin Source File

SOURCE=.\DbRooms.cpp
# End Source File
# Begin Source File

SOURCE=.\DbRooms.h
# End Source File
# Begin Source File

SOURCE=.\DbSavedGames.cpp
# End Source File
# Begin Source File

SOURCE=.\DbSavedGames.h
# End Source File
# Begin Source File

SOURCE=.\DbVDInterface.h
# End Source File
# Begin Source File

SOURCE=.\DbXML.cpp
# End Source File
# Begin Source File

SOURCE=.\DbXML.h
# End Source File
# Begin Source File

SOURCE=.\ImportInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ImportInfo.h
# End Source File
# End Group
# Begin Group "Monsters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Brain.cpp
# End Source File
# Begin Source File

SOURCE=.\Brain.h
# End Source File
# Begin Source File

SOURCE=.\EvilEye.cpp
# End Source File
# Begin Source File

SOURCE=.\EvilEye.h
# End Source File
# Begin Source File

SOURCE=.\Goblin.cpp
# End Source File
# Begin Source File

SOURCE=.\Goblin.h
# End Source File
# Begin Source File

SOURCE=.\Mimic.cpp
# End Source File
# Begin Source File

SOURCE=.\Mimic.h
# End Source File
# Begin Source File

SOURCE=.\Monster.cpp
# End Source File
# Begin Source File

SOURCE=.\Monster.h
# End Source File
# Begin Source File

SOURCE=.\MonsterFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\MonsterFactory.h
# End Source File
# Begin Source File

SOURCE=.\MonsterMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\MonsterMessage.h
# End Source File
# Begin Source File

SOURCE=.\Neather.cpp
# End Source File
# Begin Source File

SOURCE=.\Neather.h
# End Source File
# Begin Source File

SOURCE=.\PathMap.cpp
# End Source File
# Begin Source File

SOURCE=.\PathMap.h
# End Source File
# Begin Source File

SOURCE=.\RedSerpent.cpp
# End Source File
# Begin Source File

SOURCE=.\RedSerpent.h
# End Source File
# Begin Source File

SOURCE=.\Roach.cpp
# End Source File
# Begin Source File

SOURCE=.\Roach.h
# End Source File
# Begin Source File

SOURCE=.\RoachEgg.cpp
# End Source File
# Begin Source File

SOURCE=.\RoachEgg.h
# End Source File
# Begin Source File

SOURCE=.\RoachQueen.cpp
# End Source File
# Begin Source File

SOURCE=.\RoachQueen.h
# End Source File
# Begin Source File

SOURCE=.\Serpent.cpp
# End Source File
# Begin Source File

SOURCE=.\Serpent.h
# End Source File
# Begin Source File

SOURCE=.\Spider.cpp
# End Source File
# Begin Source File

SOURCE=.\Spider.h
# End Source File
# Begin Source File

SOURCE=.\TarBaby.cpp
# End Source File
# Begin Source File

SOURCE=.\TarBaby.h
# End Source File
# Begin Source File

SOURCE=.\TarMother.cpp
# End Source File
# Begin Source File

SOURCE=.\TarMother.h
# End Source File
# Begin Source File

SOURCE=.\Wraithwing.cpp
# End Source File
# Begin Source File

SOURCE=.\Wraithwing.h
# End Source File
# End Group
# End Target
# End Project
