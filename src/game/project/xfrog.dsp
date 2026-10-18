# Microsoft Developer Studio Project File - Name="xfrog" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=xfrog - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xfrog.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xfrog.mak" CFG="xfrog - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xfrog - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xfrog - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "xfrog - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xfrog - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib dxguid.lib winmm.lib dinput.lib zlib.lib dsound.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /out:"../bintest/xfrog.exe"
# SUBTRACT LINK32 /profile /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "xfrog - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib dxguid.lib winmm.lib dinput.lib zlib.lib dsound.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /out:"../bintest/xfrog_debug.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "xfrog - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "xfrog - Win32 Release"
# Name "xfrog - Win32 Debug"
# Name "xfrog - Win32 Profile"
# Begin Group "C++ Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\car.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cfs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\engine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\error.cpp
# End Source File
# Begin Source File

SOURCE=..\src\font.cpp
# End Source File
# Begin Source File

SOURCE=..\src\game.cpp
# End Source File
# Begin Source File

SOURCE=..\src\highscore.cpp
# End Source File
# Begin Source File

SOURCE=..\src\hud.cpp
# End Source File
# Begin Source File

SOURCE=..\src\input.cpp
# End Source File
# Begin Source File

SOURCE=..\src\item.cpp
# End Source File
# Begin Source File

SOURCE=..\src\mem_man.cpp
# End Source File
# Begin Source File

SOURCE=..\src\menu.cpp
# End Source File
# Begin Source File

SOURCE=..\src\player.cpp
# End Source File
# Begin Source File

SOURCE=..\src\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tga.cpp
# End Source File
# Begin Source File

SOURCE=..\src\thief.cpp
# End Source File
# Begin Source File

SOURCE=..\src\timer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\video.cpp
# End Source File
# Begin Source File

SOURCE=..\src\xfrog_main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\car.h
# End Source File
# Begin Source File

SOURCE=..\src\cfs.h
# End Source File
# Begin Source File

SOURCE=..\src\engine.h
# End Source File
# Begin Source File

SOURCE=..\src\ent.h
# End Source File
# Begin Source File

SOURCE=..\src\error.h
# End Source File
# Begin Source File

SOURCE=..\src\font.h
# End Source File
# Begin Source File

SOURCE=..\src\game.h
# End Source File
# Begin Source File

SOURCE=..\src\highscore.h
# End Source File
# Begin Source File

SOURCE=..\src\hud.h
# End Source File
# Begin Source File

SOURCE=..\src\input.h
# End Source File
# Begin Source File

SOURCE=..\src\item.h
# End Source File
# Begin Source File

SOURCE=..\src\mem_man.h
# End Source File
# Begin Source File

SOURCE=..\src\menu.h
# End Source File
# Begin Source File

SOURCE=..\src\player.h
# End Source File
# Begin Source File

SOURCE=..\src\sound.h
# End Source File
# Begin Source File

SOURCE=..\src\tga.h
# End Source File
# Begin Source File

SOURCE=..\src\thief.h
# End Source File
# Begin Source File

SOURCE=..\src\timer.h
# End Source File
# Begin Source File

SOURCE=..\src\types.h
# End Source File
# Begin Source File

SOURCE=..\src\version.h
# End Source File
# Begin Source File

SOURCE=..\src\video.h
# End Source File
# Begin Source File

SOURCE=..\src\xfrog_defs.h
# End Source File
# Begin Source File

SOURCE=..\src\xfrog_main.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Log Work"

# PROP Default_Filter "*.txt"
# Begin Source File

SOURCE=..\log\log.txt
# End Source File
# End Group
# Begin Group "ASM Files"

# PROP Default_Filter "*.asm"
# Begin Source File

SOURCE=..\src\asm\mem_funcs.asm

!IF  "$(CFG)" == "xfrog - Win32 Release"

# Begin Custom Build - Assembling with NASM $(InputPath)...
IntDir=.\Release
InputPath=..\src\asm\mem_funcs.asm
InputName=mem_funcs

"$(IntDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -o $(IntDir)\$(InputName).obj -f win32   $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "xfrog - Win32 Debug"

# Begin Custom Build - Assembling with NASM $(InputPath)...
IntDir=.\Debug
InputPath=..\src\asm\mem_funcs.asm
InputName=mem_funcs

"$(IntDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -o $(IntDir)\$(InputName).obj -f win32  -g  $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "xfrog - Win32 Profile"

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
