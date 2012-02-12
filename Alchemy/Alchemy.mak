# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (x86) Console Application" 0x0103
# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=ALGUI - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to ALGUI - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Alchemy - Win32 Release" && "$(CFG)" !=\
 "Alchemy - Win32 Debug" && "$(CFG)" != "Kernel - Win32 Release" && "$(CFG)" !=\
 "Kernel - Win32 Debug" && "$(CFG)" != "ALConsole - Win32 Release" && "$(CFG)"\
 != "ALConsole - Win32 Debug" && "$(CFG)" != "CodeChain - Win32 Release" &&\
 "$(CFG)" != "CodeChain - Win32 Debug" && "$(CFG)" !=\
 "Aristotle - Win32 Release" && "$(CFG)" != "Aristotle - Win32 Debug" &&\
 "$(CFG)" != "CCLink - Win32 Release" && "$(CFG)" != "CCLink - Win32 Debug" &&\
 "$(CFG)" != "TerraFirma - Win32 Release" && "$(CFG)" !=\
 "TerraFirma - Win32 Debug" && "$(CFG)" != "Graphics - Win32 Release" &&\
 "$(CFG)" != "Graphics - Win32 Debug" && "$(CFG)" != "ALGUI - Win32 Release" &&\
 "$(CFG)" != "ALGUI - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Alchemy.mak" CFG="ALGUI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Alchemy - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Alchemy - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Kernel - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Kernel - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ALConsole - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ALConsole - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "CodeChain - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CodeChain - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Aristotle - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "Aristotle - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "CCLink - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "CCLink - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "TerraFirma - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TerraFirma - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Graphics - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Graphics - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ALGUI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ALGUI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "Alchemy - Win32 Release"

!IF  "$(CFG)" == "Alchemy - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "Graphics - Win32 Release" "CodeChain - Win32 Release"\
 "Kernel - Win32 Release" "..\Libraries\Alchemy.lib" {$(INCLUDE)}"\Alchemy.h"

CLEAN : 
	-@erase "..\Libraries\Alchemy.lib"
	-@erase ".\Release\Alchemy.obj"
	-@erase {$(INCLUDE)}"\Alchemy.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Alchemy.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Alchemy.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\Alchemy.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\Alchemy.lib" 
LIB32_OBJS= \
	"..\Libraries\codechain.lib" \
	"..\Libraries\Graphics.lib" \
	"..\Libraries\Kernel.lib" \
	".\Release\Alchemy.obj"

"..\Libraries\Alchemy.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\Alchemy.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\Alchemy.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy Alchemy.h d:\GPM\Includes

# End Custom Build

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

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
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "Graphics - Win32 Debug" "CodeChain - Win32 Debug" "Kernel - Win32 Debug"\
 "..\Libraries\Alchemy.lib" {$(INCLUDE)}"\Alchemy.h"

CLEAN : 
	-@erase "..\Libraries\Alchemy.lib"
	-@erase ".\Debug\Alchemy.obj"
	-@erase {$(INCLUDE)}"\Alchemy.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "DEBUG" /Fp"$(INTDIR)/Alchemy.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Alchemy.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\Alchemy.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\Alchemy.lib" 
LIB32_OBJS= \
	"..\Libraries\codechain.lib" \
	"..\Libraries\Graphics.lib" \
	"..\Libraries\Kernel.lib" \
	".\Debug\Alchemy.obj"

"..\Libraries\Alchemy.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\Alchemy.lib
SOURCE=$(InputPath)

BuildCmds= \
	copy Alchemy.h d:\GPM\Includes \
	

"d:\GPM\Includes\Alchemy.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"d:\GPM\Libraries\Alchemy.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Kernel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Kernel\Release"
# PROP BASE Intermediate_Dir "Kernel\Release"
# PROP BASE Target_Dir "Kernel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Kernel\Release"
# PROP Intermediate_Dir "Kernel\Release"
# PROP Target_Dir "Kernel"
OUTDIR=.\Kernel\Release
INTDIR=.\Kernel\Release

ALL : "..\Libraries\Kernel.lib" {$(INCLUDE)}"\Kernel.h"\
 {$(INCLUDE)}"\KernelObjID.h"

CLEAN : 
	-@erase "..\Libraries\Kernel.lib"
	-@erase ".\Kernel\Release\CArchiver.obj"
	-@erase ".\Kernel\Release\CAtomTable.obj"
	-@erase ".\Kernel\Release\CDataFile.obj"
	-@erase ".\Kernel\Release\CDictionary.obj"
	-@erase ".\Kernel\Release\CFileReadBlock.obj"
	-@erase ".\Kernel\Release\CFileReadStream.obj"
	-@erase ".\Kernel\Release\CFileWriteStream.obj"
	-@erase ".\Kernel\Release\CIDTable.obj"
	-@erase ".\Kernel\Release\CIntArray.obj"
	-@erase ".\Kernel\Release\CINTDynamicArray.obj"
	-@erase ".\Kernel\Release\CMemoryStream.obj"
	-@erase ".\Kernel\Release\CObject.obj"
	-@erase ".\Kernel\Release\CObjectArray.obj"
	-@erase ".\Kernel\Release\CResourceReadBlock.obj"
	-@erase ".\Kernel\Release\CString.obj"
	-@erase ".\Kernel\Release\CStringArray.obj"
	-@erase ".\Kernel\Release\CStructArray.obj"
	-@erase ".\Kernel\Release\CSymbolTable.obj"
	-@erase ".\Kernel\Release\CTextFileLog.obj"
	-@erase ".\Kernel\Release\Kernel.obj"
	-@erase ".\Kernel\Release\Math.obj"
	-@erase ".\Kernel\Release\Path.obj"
	-@erase ".\Kernel\Release\Pattern.obj"
	-@erase ".\Kernel\Release\Utilities.obj"
	-@erase {$(INCLUDE)}"\Kernel.h"
	-@erase {$(INCLUDE)}"\KernelObjID.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX"Kernel.h" /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Kernel.pch" /YX"Kernel.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Kernel\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Kernel.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\Kernel.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\Kernel.lib" 
LIB32_OBJS= \
	".\Kernel\Release\CArchiver.obj" \
	".\Kernel\Release\CAtomTable.obj" \
	".\Kernel\Release\CDataFile.obj" \
	".\Kernel\Release\CDictionary.obj" \
	".\Kernel\Release\CFileReadBlock.obj" \
	".\Kernel\Release\CFileReadStream.obj" \
	".\Kernel\Release\CFileWriteStream.obj" \
	".\Kernel\Release\CIDTable.obj" \
	".\Kernel\Release\CIntArray.obj" \
	".\Kernel\Release\CINTDynamicArray.obj" \
	".\Kernel\Release\CMemoryStream.obj" \
	".\Kernel\Release\CObject.obj" \
	".\Kernel\Release\CObjectArray.obj" \
	".\Kernel\Release\CResourceReadBlock.obj" \
	".\Kernel\Release\CString.obj" \
	".\Kernel\Release\CStringArray.obj" \
	".\Kernel\Release\CStructArray.obj" \
	".\Kernel\Release\CSymbolTable.obj" \
	".\Kernel\Release\CTextFileLog.obj" \
	".\Kernel\Release\Kernel.obj" \
	".\Kernel\Release\Math.obj" \
	".\Kernel\Release\Path.obj" \
	".\Kernel\Release\Pattern.obj" \
	".\Kernel\Release\Utilities.obj"

"..\Libraries\Kernel.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\Kernel.lib
SOURCE=$(InputPath)

BuildCmds= \
	copy Kernel\Kernel.h d:\GPM\Includes \
	copy Kernel\KernelObjID.h d:\GPM\Includes \
	

"d:\GPM\Includes\Kernel.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"d:\GPM\Includes\KernelObjID.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Kernel\Debug"
# PROP BASE Intermediate_Dir "Kernel\Debug"
# PROP BASE Target_Dir "Kernel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Kernel\Debug"
# PROP Intermediate_Dir "Kernel\Debug"
# PROP Target_Dir "Kernel"
OUTDIR=.\Kernel\Debug
INTDIR=.\Kernel\Debug

ALL : "..\Libraries\Kernel.lib" {$(INCLUDE)}"\Kernel.h"\
 {$(INCLUDE)}"\KernelObjID.h"

CLEAN : 
	-@erase "..\Libraries\Kernel.lib"
	-@erase ".\Kernel\Debug\CArchiver.obj"
	-@erase ".\Kernel\Debug\CAtomTable.obj"
	-@erase ".\Kernel\Debug\CDataFile.obj"
	-@erase ".\Kernel\Debug\CDictionary.obj"
	-@erase ".\Kernel\Debug\CFileReadBlock.obj"
	-@erase ".\Kernel\Debug\CFileReadStream.obj"
	-@erase ".\Kernel\Debug\CFileWriteStream.obj"
	-@erase ".\Kernel\Debug\CIDTable.obj"
	-@erase ".\Kernel\Debug\CIntArray.obj"
	-@erase ".\Kernel\Debug\CINTDynamicArray.obj"
	-@erase ".\Kernel\Debug\CMemoryStream.obj"
	-@erase ".\Kernel\Debug\CObject.obj"
	-@erase ".\Kernel\Debug\CObjectArray.obj"
	-@erase ".\Kernel\Debug\CResourceReadBlock.obj"
	-@erase ".\Kernel\Debug\CString.obj"
	-@erase ".\Kernel\Debug\CStringArray.obj"
	-@erase ".\Kernel\Debug\CStructArray.obj"
	-@erase ".\Kernel\Debug\CSymbolTable.obj"
	-@erase ".\Kernel\Debug\CTextFileLog.obj"
	-@erase ".\Kernel\Debug\Kernel.obj"
	-@erase ".\Kernel\Debug\Math.obj"
	-@erase ".\Kernel\Debug\Path.obj"
	-@erase ".\Kernel\Debug\Pattern.obj"
	-@erase ".\Kernel\Debug\Utilities.obj"
	-@erase {$(INCLUDE)}"\Kernel.h"
	-@erase {$(INCLUDE)}"\KernelObjID.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX"Kernel.h" /c
CPP_PROJ=/nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "DEBUG" /Fp"$(INTDIR)/Kernel.pch" /YX"Kernel.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Kernel\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Kernel.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\Kernel.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\Kernel.lib" 
LIB32_OBJS= \
	".\Kernel\Debug\CArchiver.obj" \
	".\Kernel\Debug\CAtomTable.obj" \
	".\Kernel\Debug\CDataFile.obj" \
	".\Kernel\Debug\CDictionary.obj" \
	".\Kernel\Debug\CFileReadBlock.obj" \
	".\Kernel\Debug\CFileReadStream.obj" \
	".\Kernel\Debug\CFileWriteStream.obj" \
	".\Kernel\Debug\CIDTable.obj" \
	".\Kernel\Debug\CIntArray.obj" \
	".\Kernel\Debug\CINTDynamicArray.obj" \
	".\Kernel\Debug\CMemoryStream.obj" \
	".\Kernel\Debug\CObject.obj" \
	".\Kernel\Debug\CObjectArray.obj" \
	".\Kernel\Debug\CResourceReadBlock.obj" \
	".\Kernel\Debug\CString.obj" \
	".\Kernel\Debug\CStringArray.obj" \
	".\Kernel\Debug\CStructArray.obj" \
	".\Kernel\Debug\CSymbolTable.obj" \
	".\Kernel\Debug\CTextFileLog.obj" \
	".\Kernel\Debug\Kernel.obj" \
	".\Kernel\Debug\Math.obj" \
	".\Kernel\Debug\Path.obj" \
	".\Kernel\Debug\Pattern.obj" \
	".\Kernel\Debug\Utilities.obj"

"..\Libraries\Kernel.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\Kernel.lib
SOURCE=$(InputPath)

BuildCmds= \
	copy Kernel\Kernel.h d:\GPM\Includes \
	copy Kernel\KernelObjID.h d:\GPM\Includes \
	

"d:\GPM\Includes\Kernel.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"d:\GPM\Includes\KernelObjID.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ALConsole - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ALConsole\Release"
# PROP BASE Intermediate_Dir "ALConsole\Release"
# PROP BASE Target_Dir "ALConsole"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ALConsole\Release"
# PROP Intermediate_Dir "ALConsole\Release"
# PROP Target_Dir "ALConsole"
OUTDIR=.\ALConsole\Release
INTDIR=.\ALConsole\Release

ALL : "Alchemy - Win32 Release" "..\Libraries\ALConsole.lib"\
 {$(INCLUDE)}"\ALConsole.h"

CLEAN : 
	-@erase "..\Libraries\ALConsole.lib"
	-@erase ".\ALConsole\Release\Main.obj"
	-@erase {$(INCLUDE)}"\ALConsole.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/ALConsole.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ALConsole\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ALConsole.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\ALConsole.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\ALConsole.lib" 
LIB32_OBJS= \
	".\ALConsole\Release\Main.obj"

"..\Libraries\ALConsole.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\ALConsole.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\ALConsole.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy ALConsole\ALConsole.h d:\GPM\Includes

# End Custom Build

!ELSEIF  "$(CFG)" == "ALConsole - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ALConsole\Debug"
# PROP BASE Intermediate_Dir "ALConsole\Debug"
# PROP BASE Target_Dir "ALConsole"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ALConsole\Debug"
# PROP Intermediate_Dir "ALConsole\Debug"
# PROP Target_Dir "ALConsole"
OUTDIR=.\ALConsole\Debug
INTDIR=.\ALConsole\Debug

ALL : "Alchemy - Win32 Debug" "..\Libraries\ALConsole.lib"\
 {$(INCLUDE)}"\ALConsole.h"

CLEAN : 
	-@erase "..\Libraries\ALConsole.lib"
	-@erase ".\ALConsole\Debug\Main.obj"
	-@erase {$(INCLUDE)}"\ALConsole.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "DEBUG" /Fp"$(INTDIR)/ALConsole.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ALConsole\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ALConsole.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\ALConsole.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\ALConsole.lib" 
LIB32_OBJS= \
	".\ALConsole\Debug\Main.obj"

"..\Libraries\ALConsole.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\ALConsole.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\ALConsole.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy ALConsole\ALConsole.h d:\GPM\Includes

# End Custom Build

!ELSEIF  "$(CFG)" == "CodeChain - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CodeChain\Release"
# PROP BASE Intermediate_Dir "CodeChain\Release"
# PROP BASE Target_Dir "CodeChain"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "CodeChain\Release"
# PROP Intermediate_Dir "CodeChain\Release"
# PROP Target_Dir "CodeChain"
OUTDIR=.\CodeChain\Release
INTDIR=.\CodeChain\Release

ALL : "..\Libraries\codechain.lib" {$(INCLUDE)}"\CodeChain.h"

CLEAN : 
	-@erase "..\Libraries\codechain.lib"
	-@erase ".\CodeChain\Release\CCAtomTable.obj"
	-@erase ".\CodeChain\Release\CCInteger.obj"
	-@erase ".\CodeChain\Release\CCItemPool.obj"
	-@erase ".\CodeChain\Release\CCLambda.obj"
	-@erase ".\CodeChain\Release\CCLinkedList.obj"
	-@erase ".\CodeChain\Release\CCNil.obj"
	-@erase ".\CodeChain\Release\CConsPool.obj"
	-@erase ".\CodeChain\Release\CCPrimitive.obj"
	-@erase ".\CodeChain\Release\CCString.obj"
	-@erase ".\CodeChain\Release\CCSymbolTable.obj"
	-@erase ".\CodeChain\Release\CCTrue.obj"
	-@erase ".\CodeChain\Release\CCVector.obj"
	-@erase ".\CodeChain\Release\CodeChain.obj"
	-@erase ".\CodeChain\Release\Functions.obj"
	-@erase ".\CodeChain\Release\ICCItem.obj"
	-@erase ".\CodeChain\Release\Link.obj"
	-@erase ".\CodeChain\Release\LoadApp.obj"
	-@erase {$(INCLUDE)}"\CodeChain.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/CodeChain.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\CodeChain\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/CodeChain.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\CodeChain.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\CodeChain.lib" 
LIB32_OBJS= \
	".\CodeChain\Release\CCAtomTable.obj" \
	".\CodeChain\Release\CCInteger.obj" \
	".\CodeChain\Release\CCItemPool.obj" \
	".\CodeChain\Release\CCLambda.obj" \
	".\CodeChain\Release\CCLinkedList.obj" \
	".\CodeChain\Release\CCNil.obj" \
	".\CodeChain\Release\CConsPool.obj" \
	".\CodeChain\Release\CCPrimitive.obj" \
	".\CodeChain\Release\CCString.obj" \
	".\CodeChain\Release\CCSymbolTable.obj" \
	".\CodeChain\Release\CCTrue.obj" \
	".\CodeChain\Release\CCVector.obj" \
	".\CodeChain\Release\CodeChain.obj" \
	".\CodeChain\Release\Functions.obj" \
	".\CodeChain\Release\ICCItem.obj" \
	".\CodeChain\Release\Link.obj" \
	".\CodeChain\Release\LoadApp.obj"

"..\Libraries\codechain.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\codechain.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\CodeChain.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy CodeChain\CodeChain.h d:\GPM\Includes

# End Custom Build

!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CodeChain\Debug"
# PROP BASE Intermediate_Dir "CodeChain\Debug"
# PROP BASE Target_Dir "CodeChain"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "CodeChain\Debug"
# PROP Intermediate_Dir "CodeChain\Debug"
# PROP Target_Dir "CodeChain"
OUTDIR=.\CodeChain\Debug
INTDIR=.\CodeChain\Debug

ALL : "..\Libraries\codechain.lib" {$(INCLUDE)}"\CodeChain.h"

CLEAN : 
	-@erase "..\Libraries\codechain.lib"
	-@erase ".\CodeChain\Debug\CCAtomTable.obj"
	-@erase ".\CodeChain\Debug\CCInteger.obj"
	-@erase ".\CodeChain\Debug\CCItemPool.obj"
	-@erase ".\CodeChain\Debug\CCLambda.obj"
	-@erase ".\CodeChain\Debug\CCLinkedList.obj"
	-@erase ".\CodeChain\Debug\CCNil.obj"
	-@erase ".\CodeChain\Debug\CConsPool.obj"
	-@erase ".\CodeChain\Debug\CCPrimitive.obj"
	-@erase ".\CodeChain\Debug\CCString.obj"
	-@erase ".\CodeChain\Debug\CCSymbolTable.obj"
	-@erase ".\CodeChain\Debug\CCTrue.obj"
	-@erase ".\CodeChain\Debug\CCVector.obj"
	-@erase ".\CodeChain\Debug\CodeChain.obj"
	-@erase ".\CodeChain\Debug\Functions.obj"
	-@erase ".\CodeChain\Debug\ICCItem.obj"
	-@erase ".\CodeChain\Debug\Link.obj"
	-@erase ".\CodeChain\Debug\LoadApp.obj"
	-@erase {$(INCLUDE)}"\CodeChain.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "DEBUG" /Fp"$(INTDIR)/CodeChain.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\CodeChain\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/CodeChain.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\CodeChain.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\CodeChain.lib" 
LIB32_OBJS= \
	".\CodeChain\Debug\CCAtomTable.obj" \
	".\CodeChain\Debug\CCInteger.obj" \
	".\CodeChain\Debug\CCItemPool.obj" \
	".\CodeChain\Debug\CCLambda.obj" \
	".\CodeChain\Debug\CCLinkedList.obj" \
	".\CodeChain\Debug\CCNil.obj" \
	".\CodeChain\Debug\CConsPool.obj" \
	".\CodeChain\Debug\CCPrimitive.obj" \
	".\CodeChain\Debug\CCString.obj" \
	".\CodeChain\Debug\CCSymbolTable.obj" \
	".\CodeChain\Debug\CCTrue.obj" \
	".\CodeChain\Debug\CCVector.obj" \
	".\CodeChain\Debug\CodeChain.obj" \
	".\CodeChain\Debug\Functions.obj" \
	".\CodeChain\Debug\ICCItem.obj" \
	".\CodeChain\Debug\Link.obj" \
	".\CodeChain\Debug\LoadApp.obj"

"..\Libraries\codechain.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\codechain.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\CodeChain.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy CodeChain\CodeChain.h d:\GPM\Includes

# End Custom Build

!ELSEIF  "$(CFG)" == "Aristotle - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Aristotle\Release"
# PROP BASE Intermediate_Dir "Aristotle\Release"
# PROP BASE Target_Dir "Aristotle"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Aristotle\Release"
# PROP Intermediate_Dir "Aristotle\Release"
# PROP Target_Dir "Aristotle"
OUTDIR=.\Aristotle\Release
INTDIR=.\Aristotle\Release

ALL : "ALConsole - Win32 Release" ".\Aristotle\Release\Aristotle.exe"

CLEAN : 
	-@erase ".\Aristotle\Release\Aristotle.exe"
	-@erase ".\Aristotle\Release\Aristotle.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/Aristotle.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Aristotle\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Aristotle.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# SUBTRACT LINK32 /verbose
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/Aristotle.pdb" /machine:I386\
 /out:"$(OUTDIR)/Aristotle.exe" 
LINK32_OBJS= \
	".\Aristotle\Release\Aristotle.obj"

".\Aristotle\Release\Aristotle.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Aristotle - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Aristotle\Debug"
# PROP BASE Intermediate_Dir "Aristotle\Debug"
# PROP BASE Target_Dir "Aristotle"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Aristotle\Debug"
# PROP Intermediate_Dir "Aristotle\Debug"
# PROP Target_Dir "Aristotle"
OUTDIR=.\Aristotle\Debug
INTDIR=.\Aristotle\Debug

ALL : "ALConsole - Win32 Debug" ".\Aristotle\Debug\Aristotle.exe"

CLEAN : 
	-@erase ".\Aristotle\Debug\Aristotle.exe"
	-@erase ".\Aristotle\Debug\Aristotle.ilk"
	-@erase ".\Aristotle\Debug\Aristotle.obj"
	-@erase ".\Aristotle\Debug\Aristotle.pdb"
	-@erase ".\Aristotle\Debug\vc40.idb"
	-@erase ".\Aristotle\Debug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "DEBUG" /Fp"$(INTDIR)/Aristotle.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Aristotle\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Aristotle.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# SUBTRACT LINK32 /verbose
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/Aristotle.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/Aristotle.exe" 
LINK32_OBJS= \
	".\Aristotle\Debug\Aristotle.obj"

".\Aristotle\Debug\Aristotle.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "CCLink - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CCLink\Release"
# PROP BASE Intermediate_Dir "CCLink\Release"
# PROP BASE Target_Dir "CCLink"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "CCLink\Release"
# PROP Intermediate_Dir "CCLink\Release"
# PROP Target_Dir "CCLink"
OUTDIR=.\CCLink\Release
INTDIR=.\CCLink\Release

ALL : "ALConsole - Win32 Release" ".\CCLink\Release\CCLink.exe"

CLEAN : 
	-@erase ".\CCLink\Release\CCLink.exe"
	-@erase ".\CCLink\Release\CCLink.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/CCLink.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\CCLink\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/CCLink.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/CCLink.pdb" /machine:I386\
 /out:"$(OUTDIR)/CCLink.exe" 
LINK32_OBJS= \
	".\CCLink\Release\CCLink.obj"

".\CCLink\Release\CCLink.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "CCLink - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CCLink\Debug"
# PROP BASE Intermediate_Dir "CCLink\Debug"
# PROP BASE Target_Dir "CCLink"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "CCLink\Debug"
# PROP Intermediate_Dir "CCLink\Debug"
# PROP Target_Dir "CCLink"
OUTDIR=.\CCLink\Debug
INTDIR=.\CCLink\Debug

ALL : "ALConsole - Win32 Debug" ".\CCLink\Debug\CCLink.exe"

CLEAN : 
	-@erase ".\CCLink\Debug\CCLink.exe"
	-@erase ".\CCLink\Debug\CCLink.ilk"
	-@erase ".\CCLink\Debug\CCLink.obj"
	-@erase ".\CCLink\Debug\CCLink.pdb"
	-@erase ".\CCLink\Debug\vc40.idb"
	-@erase ".\CCLink\Debug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "DEBUG" /Fp"$(INTDIR)/CCLink.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\CCLink\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/CCLink.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/CCLink.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/CCLink.exe" 
LINK32_OBJS= \
	".\CCLink\Debug\CCLink.obj"

".\CCLink\Debug\CCLink.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TerraFirma\Release"
# PROP BASE Intermediate_Dir "TerraFirma\Release"
# PROP BASE Target_Dir "TerraFirma"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TerraFirma\Release"
# PROP Intermediate_Dir "TerraFirma\Release"
# PROP Target_Dir "TerraFirma"
OUTDIR=.\TerraFirma\Release
INTDIR=.\TerraFirma\Release

ALL : "Alchemy - Win32 Release" ".\TerraFirma\Release\TerraFirma.exe"

CLEAN : 
	-@erase ".\TerraFirma\Release\CCommandShellEditor.obj"
	-@erase ".\TerraFirma\Release\CGenericWnd.obj"
	-@erase ".\TerraFirma\Release\CImageEditor.obj"
	-@erase ".\TerraFirma\Release\CLArea.obj"
	-@erase ".\TerraFirma\Release\CLibraryFile.obj"
	-@erase ".\TerraFirma\Release\CLImageArea.obj"
	-@erase ".\TerraFirma\Release\CLSizeOptions.obj"
	-@erase ".\TerraFirma\Release\CLWindow.obj"
	-@erase ".\TerraFirma\Release\CLWindowManager.obj"
	-@erase ".\TerraFirma\Release\CNodeEditor.obj"
	-@erase ".\TerraFirma\Release\CNodeListEditor.obj"
	-@erase ".\TerraFirma\Release\CommandLineWnd.obj"
	-@erase ".\TerraFirma\Release\CTerraFirma.obj"
	-@erase ".\TerraFirma\Release\Extensions.obj"
	-@erase ".\TerraFirma\Release\Main.obj"
	-@erase ".\TerraFirma\Release\MainWnd.obj"
	-@erase ".\TerraFirma\Release\TerraFirma.exe"
	-@erase ".\TerraFirma\Release\WindowsExt.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/TerraFirma.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\TerraFirma\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/TerraFirma.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/TerraFirma.pdb"\
 /machine:I386 /out:"$(OUTDIR)/TerraFirma.exe" 
LINK32_OBJS= \
	".\TerraFirma\Release\CCommandShellEditor.obj" \
	".\TerraFirma\Release\CGenericWnd.obj" \
	".\TerraFirma\Release\CImageEditor.obj" \
	".\TerraFirma\Release\CLArea.obj" \
	".\TerraFirma\Release\CLibraryFile.obj" \
	".\TerraFirma\Release\CLImageArea.obj" \
	".\TerraFirma\Release\CLSizeOptions.obj" \
	".\TerraFirma\Release\CLWindow.obj" \
	".\TerraFirma\Release\CLWindowManager.obj" \
	".\TerraFirma\Release\CNodeEditor.obj" \
	".\TerraFirma\Release\CNodeListEditor.obj" \
	".\TerraFirma\Release\CommandLineWnd.obj" \
	".\TerraFirma\Release\CTerraFirma.obj" \
	".\TerraFirma\Release\Extensions.obj" \
	".\TerraFirma\Release\Main.obj" \
	".\TerraFirma\Release\MainWnd.obj" \
	".\TerraFirma\Release\WindowsExt.obj"

".\TerraFirma\Release\TerraFirma.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TerraFirma\Debug"
# PROP BASE Intermediate_Dir "TerraFirma\Debug"
# PROP BASE Target_Dir "TerraFirma"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TerraFirma\Debug"
# PROP Intermediate_Dir "TerraFirma\Debug"
# PROP Target_Dir "TerraFirma"
OUTDIR=.\TerraFirma\Debug
INTDIR=.\TerraFirma\Debug

ALL : "Alchemy - Win32 Debug" ".\TerraFirma\Debug\TerraFirma.exe"

CLEAN : 
	-@erase ".\TerraFirma\Debug\CCommandShellEditor.obj"
	-@erase ".\TerraFirma\Debug\CGenericWnd.obj"
	-@erase ".\TerraFirma\Debug\CImageEditor.obj"
	-@erase ".\TerraFirma\Debug\CLArea.obj"
	-@erase ".\TerraFirma\Debug\CLibraryFile.obj"
	-@erase ".\TerraFirma\Debug\CLImageArea.obj"
	-@erase ".\TerraFirma\Debug\CLSizeOptions.obj"
	-@erase ".\TerraFirma\Debug\CLWindow.obj"
	-@erase ".\TerraFirma\Debug\CLWindowManager.obj"
	-@erase ".\TerraFirma\Debug\CNodeEditor.obj"
	-@erase ".\TerraFirma\Debug\CNodeListEditor.obj"
	-@erase ".\TerraFirma\Debug\CommandLineWnd.obj"
	-@erase ".\TerraFirma\Debug\CTerraFirma.obj"
	-@erase ".\TerraFirma\Debug\Extensions.obj"
	-@erase ".\TerraFirma\Debug\Main.obj"
	-@erase ".\TerraFirma\Debug\MainWnd.obj"
	-@erase ".\TerraFirma\Debug\TerraFirma.exe"
	-@erase ".\TerraFirma\Debug\vc40.idb"
	-@erase ".\TerraFirma\Debug\vc40.pdb"
	-@erase ".\TerraFirma\Debug\WindowsExt.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "DEBUG" /Fp"$(INTDIR)/TerraFirma.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\TerraFirma\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/TerraFirma.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /profile /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /profile /debug /machine:I386\
 /out:"$(OUTDIR)/TerraFirma.exe" 
LINK32_OBJS= \
	".\TerraFirma\Debug\CCommandShellEditor.obj" \
	".\TerraFirma\Debug\CGenericWnd.obj" \
	".\TerraFirma\Debug\CImageEditor.obj" \
	".\TerraFirma\Debug\CLArea.obj" \
	".\TerraFirma\Debug\CLibraryFile.obj" \
	".\TerraFirma\Debug\CLImageArea.obj" \
	".\TerraFirma\Debug\CLSizeOptions.obj" \
	".\TerraFirma\Debug\CLWindow.obj" \
	".\TerraFirma\Debug\CLWindowManager.obj" \
	".\TerraFirma\Debug\CNodeEditor.obj" \
	".\TerraFirma\Debug\CNodeListEditor.obj" \
	".\TerraFirma\Debug\CommandLineWnd.obj" \
	".\TerraFirma\Debug\CTerraFirma.obj" \
	".\TerraFirma\Debug\Extensions.obj" \
	".\TerraFirma\Debug\Main.obj" \
	".\TerraFirma\Debug\MainWnd.obj" \
	".\TerraFirma\Debug\WindowsExt.obj"

".\TerraFirma\Debug\TerraFirma.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Graphics - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Graphics\Release"
# PROP BASE Intermediate_Dir "Graphics\Release"
# PROP BASE Target_Dir "Graphics"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Graphics\Release"
# PROP Intermediate_Dir "Graphics\Release"
# PROP Target_Dir "Graphics"
OUTDIR=.\Graphics\Release
INTDIR=.\Graphics\Release

ALL : "..\Libraries\Graphics.lib" "..\Includes\Graphics.h"

CLEAN : 
	-@erase "..\Includes\Graphics.h"
	-@erase "..\Libraries\Graphics.lib"
	-@erase ".\Graphics\Release\CGBitmap.obj"
	-@erase ".\Graphics\Release\CGFont.obj"
	-@erase ".\Graphics\Release\CGImageCache.obj"
	-@erase ".\Graphics\Release\CGLockedSurface.obj"
	-@erase ".\Graphics\Release\CGResourceFile.obj"
	-@erase ".\Graphics\Release\CGScreen.obj"
	-@erase ".\Graphics\Release\DIB.obj"
	-@erase ".\Graphics\Release\GDI.obj"
	-@erase ".\Graphics\Release\Raw.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Graphics.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Graphics\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Graphics.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\Graphics.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\Graphics.lib" 
LIB32_OBJS= \
	".\Graphics\Release\CGBitmap.obj" \
	".\Graphics\Release\CGFont.obj" \
	".\Graphics\Release\CGImageCache.obj" \
	".\Graphics\Release\CGLockedSurface.obj" \
	".\Graphics\Release\CGResourceFile.obj" \
	".\Graphics\Release\CGScreen.obj" \
	".\Graphics\Release\DIB.obj" \
	".\Graphics\Release\GDI.obj" \
	".\Graphics\Release\Raw.obj"

"..\Libraries\Graphics.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\Graphics.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\Graphics.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy Graphics\Graphics.h d:\GPM\Includes

# End Custom Build

!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Graphics\Debug"
# PROP BASE Intermediate_Dir "Graphics\Debug"
# PROP BASE Target_Dir "Graphics"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Graphics\Debug"
# PROP Intermediate_Dir "Graphics\Debug"
# PROP Target_Dir "Graphics"
OUTDIR=.\Graphics\Debug
INTDIR=.\Graphics\Debug

ALL : "..\Libraries\Graphics.lib" "..\Includes\Graphics.h"

CLEAN : 
	-@erase "..\Includes\Graphics.h"
	-@erase "..\Libraries\Graphics.lib"
	-@erase ".\Graphics\Debug\CGBitmap.obj"
	-@erase ".\Graphics\Debug\CGFont.obj"
	-@erase ".\Graphics\Debug\CGImageCache.obj"
	-@erase ".\Graphics\Debug\CGLockedSurface.obj"
	-@erase ".\Graphics\Debug\CGResourceFile.obj"
	-@erase ".\Graphics\Debug\CGScreen.obj"
	-@erase ".\Graphics\Debug\DIB.obj"
	-@erase ".\Graphics\Debug\GDI.obj"
	-@erase ".\Graphics\Debug\Raw.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "DEBUG" /Fp"$(INTDIR)/Graphics.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Graphics\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Graphics.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\Graphics.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\Graphics.lib" 
LIB32_OBJS= \
	".\Graphics\Debug\CGBitmap.obj" \
	".\Graphics\Debug\CGFont.obj" \
	".\Graphics\Debug\CGImageCache.obj" \
	".\Graphics\Debug\CGLockedSurface.obj" \
	".\Graphics\Debug\CGResourceFile.obj" \
	".\Graphics\Debug\CGScreen.obj" \
	".\Graphics\Debug\DIB.obj" \
	".\Graphics\Debug\GDI.obj" \
	".\Graphics\Debug\Raw.obj"

"..\Libraries\Graphics.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\Graphics.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\Graphics.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy Graphics\Graphics.h d:\GPM\Includes

# End Custom Build

!ELSEIF  "$(CFG)" == "ALGUI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ALGUI\Release"
# PROP BASE Intermediate_Dir "ALGUI\Release"
# PROP BASE Target_Dir "ALGUI"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ALGUI\Release"
# PROP Intermediate_Dir "ALGUI\Release"
# PROP Target_Dir "ALGUI"
OUTDIR=.\ALGUI\Release
INTDIR=.\ALGUI\Release

ALL : "Alchemy - Win32 Release" "..\Libraries\ALGUI.lib" ".\asd"

CLEAN : 
	-@erase "..\Libraries\ALGUI.lib"
	-@erase ".\ALGUI\Release\AutoSizeDesc.obj"
	-@erase ".\ALGUI\Release\CUApplication.obj"
	-@erase ".\ALGUI\Release\CUButton.obj"
	-@erase ".\ALGUI\Release\CUFrame.obj"
	-@erase ".\ALGUI\Release\CULineEditor.obj"
	-@erase ".\ALGUI\Release\CUWindow.obj"
	-@erase ".\ALGUI\Release\Main.obj"
	-@erase ".\asd"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/ALGUI.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ALGUI\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ALGUI.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\ALGUI.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\ALGUI.lib" 
LIB32_OBJS= \
	"..\Libraries\Alchemy.lib" \
	".\ALGUI\Release\AutoSizeDesc.obj" \
	".\ALGUI\Release\CUApplication.obj" \
	".\ALGUI\Release\CUButton.obj" \
	".\ALGUI\Release\CUFrame.obj" \
	".\ALGUI\Release\CULineEditor.obj" \
	".\ALGUI\Release\CUWindow.obj" \
	".\ALGUI\Release\Main.obj"

"..\Libraries\ALGUI.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\ALGUI.lib
SOURCE=$(InputPath)

"asd" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy $(InputPath) d:\GPM\Libraries

# End Custom Build

!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ALGUI\Debug"
# PROP BASE Intermediate_Dir "ALGUI\Debug"
# PROP BASE Target_Dir "ALGUI"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ALGUI\Debug"
# PROP Intermediate_Dir "ALGUI\Debug"
# PROP Target_Dir "ALGUI"
OUTDIR=.\ALGUI\Debug
INTDIR=.\ALGUI\Debug

ALL : "Alchemy - Win32 Debug" "..\Libraries\ALGUI.lib" "..\Includes\ALGUI.h"

CLEAN : 
	-@erase "..\Includes\ALGUI.h"
	-@erase "..\Libraries\ALGUI.lib"
	-@erase ".\ALGUI\Debug\AutoSizeDesc.obj"
	-@erase ".\ALGUI\Debug\CUApplication.obj"
	-@erase ".\ALGUI\Debug\CUButton.obj"
	-@erase ".\ALGUI\Debug\CUFrame.obj"
	-@erase ".\ALGUI\Debug\CULineEditor.obj"
	-@erase ".\ALGUI\Debug\CUWindow.obj"
	-@erase ".\ALGUI\Debug\Main.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "DEBUG" /Fp"$(INTDIR)/ALGUI.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ALGUI\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ALGUI.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\ALGUI.lib"
LIB32_FLAGS=/nologo /out:"..\Libraries\ALGUI.lib" 
LIB32_OBJS= \
	"..\Libraries\Alchemy.lib" \
	".\ALGUI\Debug\AutoSizeDesc.obj" \
	".\ALGUI\Debug\CUApplication.obj" \
	".\ALGUI\Debug\CUButton.obj" \
	".\ALGUI\Debug\CUFrame.obj" \
	".\ALGUI\Debug\CULineEditor.obj" \
	".\ALGUI\Debug\CUWindow.obj" \
	".\ALGUI\Debug\Main.obj"

"..\Libraries\ALGUI.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=\GPM\Libraries\ALGUI.lib
SOURCE=$(InputPath)

"d:\GPM\Includes\ALGUI.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy ALGUI\ALGUI.h d:\GPM\Includes

# End Custom Build

!ENDIF 

################################################################################
# Begin Target

# Name "Alchemy - Win32 Release"
# Name "Alchemy - Win32 Debug"

!IF  "$(CFG)" == "Alchemy - Win32 Release"

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "Kernel"

!IF  "$(CFG)" == "Alchemy - Win32 Release"

"Kernel - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Kernel - Win32 Release" 

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

"Kernel - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Kernel - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\Alchemy.cpp

!IF  "$(CFG)" == "Alchemy - Win32 Release"

DEP_CPP_ALCHE=\
	".\Alchemy.h"\
	

".\Release\Alchemy.obj" : $(SOURCE) $(DEP_CPP_ALCHE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

DEP_CPP_ALCHE=\
	"..\Includes\Graphics.h"\
	".\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

".\Debug\Alchemy.obj" : $(SOURCE) $(DEP_CPP_ALCHE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "CodeChain"

!IF  "$(CFG)" == "Alchemy - Win32 Release"

"CodeChain - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="CodeChain - Win32 Release" 

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

"CodeChain - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="CodeChain - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=\GPM\Libraries\Kernel.lib

!IF  "$(CFG)" == "Alchemy - Win32 Release"

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\GPM\Libraries\codechain.lib

!IF  "$(CFG)" == "Alchemy - Win32 Release"

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Graphics"

!IF  "$(CFG)" == "Alchemy - Win32 Release"

"Graphics - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Graphics - Win32 Release" 

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

"Graphics - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Graphics - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=\GPM\Libraries\Graphics.lib

!IF  "$(CFG)" == "Alchemy - Win32 Release"

!ELSEIF  "$(CFG)" == "Alchemy - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "Kernel - Win32 Release"
# Name "Kernel - Win32 Debug"

!IF  "$(CFG)" == "Kernel - Win32 Release"

!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Kernel\Utilities.cpp
DEP_CPP_UTILI=\
	".\Kernel\Kernel.h"\
	

!IF  "$(CFG)" == "Kernel - Win32 Release"


".\Kernel\Release\Utilities.obj" : $(SOURCE) $(DEP_CPP_UTILI) "$(INTDIR)"
   $(CPP) /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Kernel.pch" /YX"Kernel.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

# ADD CPP /Z7 /Od

".\Kernel\Debug\Utilities.obj" : $(SOURCE) $(DEP_CPP_UTILI) "$(INTDIR)"
   $(CPP) /nologo /MTd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "DEBUG" /Fp"$(INTDIR)/Kernel.pch" /YX"Kernel.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CDictionary.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CDICT=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CDictionary.obj" : $(SOURCE) $(DEP_CPP_CDICT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CDICT=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CDictionary.obj" : $(SOURCE) $(DEP_CPP_CDICT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CFileReadStream.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CFILE=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CFileReadStream.obj" : $(SOURCE) $(DEP_CPP_CFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CFILE=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CFileReadStream.obj" : $(SOURCE) $(DEP_CPP_CFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CFileWriteStream.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CFILEW=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CFileWriteStream.obj" : $(SOURCE) $(DEP_CPP_CFILEW)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CFILEW=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CFileWriteStream.obj" : $(SOURCE) $(DEP_CPP_CFILEW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CIntArray.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CINTA=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CIntArray.obj" : $(SOURCE) $(DEP_CPP_CINTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CINTA=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CIntArray.obj" : $(SOURCE) $(DEP_CPP_CINTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CINTDynamicArray.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CINTD=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CINTDynamicArray.obj" : $(SOURCE) $(DEP_CPP_CINTD)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CINTD=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CINTDynamicArray.obj" : $(SOURCE) $(DEP_CPP_CINTD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CMemoryStream.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CMEMO=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CMemoryStream.obj" : $(SOURCE) $(DEP_CPP_CMEMO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CMEMO=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CMemoryStream.obj" : $(SOURCE) $(DEP_CPP_CMEMO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CObject.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_COBJE=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CObject.obj" : $(SOURCE) $(DEP_CPP_COBJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_COBJE=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CObject.obj" : $(SOURCE) $(DEP_CPP_COBJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CObjectArray.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_COBJEC=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CObjectArray.obj" : $(SOURCE) $(DEP_CPP_COBJEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_COBJEC=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CObjectArray.obj" : $(SOURCE) $(DEP_CPP_COBJEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CString.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CSTRI=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CString.obj" : $(SOURCE) $(DEP_CPP_CSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CSTRI=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CString.obj" : $(SOURCE) $(DEP_CPP_CSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CStringArray.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CSTRIN=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CStringArray.obj" : $(SOURCE) $(DEP_CPP_CSTRIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CSTRIN=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CStringArray.obj" : $(SOURCE) $(DEP_CPP_CSTRIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CSymbolTable.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CSYMB=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CSymbolTable.obj" : $(SOURCE) $(DEP_CPP_CSYMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CSYMB=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CSymbolTable.obj" : $(SOURCE) $(DEP_CPP_CSYMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\Kernel.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_KERNE=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\Kernel.obj" : $(SOURCE) $(DEP_CPP_KERNE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_KERNE=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\Kernel.obj" : $(SOURCE) $(DEP_CPP_KERNE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\Math.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_MATH_=\
	".\Kernel\Kernel.h"\
	

".\Kernel\Release\Math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_MATH_=\
	".\Kernel\Kernel.h"\
	

".\Kernel\Debug\Math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CArchiver.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CARCH=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CArchiver.obj" : $(SOURCE) $(DEP_CPP_CARCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CARCH=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CArchiver.obj" : $(SOURCE) $(DEP_CPP_CARCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\Pattern.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_PATTE=\
	".\Kernel\Kernel.h"\
	

".\Kernel\Release\Pattern.obj" : $(SOURCE) $(DEP_CPP_PATTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_PATTE=\
	".\Kernel\Kernel.h"\
	

".\Kernel\Debug\Pattern.obj" : $(SOURCE) $(DEP_CPP_PATTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CDataFile.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CDATA=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CDataFile.obj" : $(SOURCE) $(DEP_CPP_CDATA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CDATA=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CDataFile.obj" : $(SOURCE) $(DEP_CPP_CDATA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\Path.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_PATH_=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\Path.obj" : $(SOURCE) $(DEP_CPP_PATH_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_PATH_=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\Path.obj" : $(SOURCE) $(DEP_CPP_PATH_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CFileReadBlock.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CFILER=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CFileReadBlock.obj" : $(SOURCE) $(DEP_CPP_CFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CFILER=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CFileReadBlock.obj" : $(SOURCE) $(DEP_CPP_CFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CResourceReadBlock.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CRESO=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CResourceReadBlock.obj" : $(SOURCE) $(DEP_CPP_CRESO)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CRESO=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CResourceReadBlock.obj" : $(SOURCE) $(DEP_CPP_CRESO)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CStructArray.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CSTRU=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CStructArray.obj" : $(SOURCE) $(DEP_CPP_CSTRU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CSTRU=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CStructArray.obj" : $(SOURCE) $(DEP_CPP_CSTRU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CAtomTable.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CATOM=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CAtomTable.obj" : $(SOURCE) $(DEP_CPP_CATOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CATOM=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CAtomTable.obj" : $(SOURCE) $(DEP_CPP_CATOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CTextFileLog.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CTEXT=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CTextFileLog.obj" : $(SOURCE) $(DEP_CPP_CTEXT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CTEXT=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CTextFileLog.obj" : $(SOURCE) $(DEP_CPP_CTEXT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Kernel\CIDTable.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

DEP_CPP_CIDTA=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Release\CIDTable.obj" : $(SOURCE) $(DEP_CPP_CIDTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

DEP_CPP_CIDTA=\
	".\Kernel\Kernel.h"\
	".\Kernel\KernelObjID.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	

".\Kernel\Debug\CIDTable.obj" : $(SOURCE) $(DEP_CPP_CIDTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "ALConsole - Win32 Release"
# Name "ALConsole - Win32 Debug"

!IF  "$(CFG)" == "ALConsole - Win32 Release"

!ELSEIF  "$(CFG)" == "ALConsole - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ALConsole\Main.cpp
DEP_CPP_MAIN_=\
	"..\Includes\Graphics.h"\
	".\ALConsole\ALConsole.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALConsole - Win32 Release"


".\ALConsole\Release\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALConsole - Win32 Debug"


".\ALConsole\Debug\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Alchemy"

!IF  "$(CFG)" == "ALConsole - Win32 Release"

"Alchemy - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Alchemy - Win32 Release" 

!ELSEIF  "$(CFG)" == "ALConsole - Win32 Debug"

"Alchemy - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Alchemy - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "CodeChain - Win32 Release"
# Name "CodeChain - Win32 Debug"

!IF  "$(CFG)" == "CodeChain - Win32 Release"

!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\CodeChain\CodeChain.cpp
DEP_CPP_CODEC=\
	".\CodeChain\CodeChain.h"\
	".\CodeChain\DefPrimitives.h"\
	".\CodeChain\Functions.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CodeChain.obj" : $(SOURCE) $(DEP_CPP_CODEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CodeChain.obj" : $(SOURCE) $(DEP_CPP_CODEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\Link.cpp
DEP_CPP_LINK_=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\Link.obj" : $(SOURCE) $(DEP_CPP_LINK_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\Link.obj" : $(SOURCE) $(DEP_CPP_LINK_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\ICCItem.cpp
DEP_CPP_ICCIT=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\ICCItem.obj" : $(SOURCE) $(DEP_CPP_ICCIT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\ICCItem.obj" : $(SOURCE) $(DEP_CPP_ICCIT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCInteger.cpp
DEP_CPP_CCINT=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCInteger.obj" : $(SOURCE) $(DEP_CPP_CCINT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCInteger.obj" : $(SOURCE) $(DEP_CPP_CCINT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCString.cpp
DEP_CPP_CCSTR=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCString.obj" : $(SOURCE) $(DEP_CPP_CCSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCString.obj" : $(SOURCE) $(DEP_CPP_CCSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCLinkedList.cpp
DEP_CPP_CCLIN=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCLinkedList.obj" : $(SOURCE) $(DEP_CPP_CCLIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCLinkedList.obj" : $(SOURCE) $(DEP_CPP_CCLIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCItemPool.cpp
DEP_CPP_CCITE=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCItemPool.obj" : $(SOURCE) $(DEP_CPP_CCITE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCItemPool.obj" : $(SOURCE) $(DEP_CPP_CCITE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCNil.cpp
DEP_CPP_CCNIL=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCNil.obj" : $(SOURCE) $(DEP_CPP_CCNIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCNil.obj" : $(SOURCE) $(DEP_CPP_CCNIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCTrue.cpp
DEP_CPP_CCTRU=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCTrue.obj" : $(SOURCE) $(DEP_CPP_CCTRU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCTrue.obj" : $(SOURCE) $(DEP_CPP_CCTRU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\Functions.cpp
DEP_CPP_FUNCT=\
	".\CodeChain\CodeChain.h"\
	".\CodeChain\Functions.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\Functions.obj" : $(SOURCE) $(DEP_CPP_FUNCT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\Functions.obj" : $(SOURCE) $(DEP_CPP_FUNCT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCPrimitive.cpp
DEP_CPP_CCPRI=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCPrimitive.obj" : $(SOURCE) $(DEP_CPP_CCPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCPrimitive.obj" : $(SOURCE) $(DEP_CPP_CCPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCSymbolTable.cpp
DEP_CPP_CCSYM=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCSymbolTable.obj" : $(SOURCE) $(DEP_CPP_CCSYM)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCSymbolTable.obj" : $(SOURCE) $(DEP_CPP_CCSYM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCLambda.cpp
DEP_CPP_CCLAM=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCLambda.obj" : $(SOURCE) $(DEP_CPP_CCLAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCLambda.obj" : $(SOURCE) $(DEP_CPP_CCLAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\LoadApp.cpp
DEP_CPP_LOADA=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\LoadApp.obj" : $(SOURCE) $(DEP_CPP_LOADA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\LoadApp.obj" : $(SOURCE) $(DEP_CPP_LOADA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CConsPool.cpp
DEP_CPP_CCONS=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CConsPool.obj" : $(SOURCE) $(DEP_CPP_CCONS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CConsPool.obj" : $(SOURCE) $(DEP_CPP_CCONS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCAtomTable.cpp
DEP_CPP_CCATO=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCAtomTable.obj" : $(SOURCE) $(DEP_CPP_CCATO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCAtomTable.obj" : $(SOURCE) $(DEP_CPP_CCATO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CodeChain\CCVector.cpp
DEP_CPP_CCVEC=\
	".\CodeChain\CodeChain.h"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CodeChain - Win32 Release"


".\CodeChain\Release\CCVector.obj" : $(SOURCE) $(DEP_CPP_CCVEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"


".\CodeChain\Debug\CCVector.obj" : $(SOURCE) $(DEP_CPP_CCVEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "Aristotle - Win32 Release"
# Name "Aristotle - Win32 Debug"

!IF  "$(CFG)" == "Aristotle - Win32 Release"

!ELSEIF  "$(CFG)" == "Aristotle - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Aristotle\Aristotle.cpp
DEP_CPP_ARIST=\
	"..\Includes\Graphics.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\ALConsole.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Aristotle - Win32 Release"


".\Aristotle\Release\Aristotle.obj" : $(SOURCE) $(DEP_CPP_ARIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Aristotle - Win32 Debug"


".\Aristotle\Debug\Aristotle.obj" : $(SOURCE) $(DEP_CPP_ARIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ALConsole"

!IF  "$(CFG)" == "Aristotle - Win32 Release"

"ALConsole - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="ALConsole - Win32 Release" 

!ELSEIF  "$(CFG)" == "Aristotle - Win32 Debug"

"ALConsole - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="ALConsole - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "CCLink - Win32 Release"
# Name "CCLink - Win32 Debug"

!IF  "$(CFG)" == "CCLink - Win32 Release"

!ELSEIF  "$(CFG)" == "CCLink - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "ALConsole"

!IF  "$(CFG)" == "CCLink - Win32 Release"

"ALConsole - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="ALConsole - Win32 Release" 

!ELSEIF  "$(CFG)" == "CCLink - Win32 Debug"

"ALConsole - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="ALConsole - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\CCLink\CCLink.cpp
DEP_CPP_CCLINK=\
	"..\Includes\Graphics.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\ALConsole.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "CCLink - Win32 Release"


".\CCLink\Release\CCLink.obj" : $(SOURCE) $(DEP_CPP_CCLINK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CCLink - Win32 Debug"


".\CCLink\Debug\CCLink.obj" : $(SOURCE) $(DEP_CPP_CCLINK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "TerraFirma - Win32 Release"
# Name "TerraFirma - Win32 Debug"

!IF  "$(CFG)" == "TerraFirma - Win32 Release"

!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\TerraFirma\Main.cpp
DEP_CPP_MAIN_=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\MainWnd.cpp
DEP_CPP_MAINW=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\MainWnd.obj" : $(SOURCE) $(DEP_CPP_MAINW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\MainWnd.obj" : $(SOURCE) $(DEP_CPP_MAINW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Alchemy"

!IF  "$(CFG)" == "TerraFirma - Win32 Release"

"Alchemy - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Alchemy - Win32 Release" 

!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"

"Alchemy - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Alchemy - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CTerraFirma.cpp
DEP_CPP_CTERR=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CTerraFirma.obj" : $(SOURCE) $(DEP_CPP_CTERR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CTerraFirma.obj" : $(SOURCE) $(DEP_CPP_CTERR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CLibraryFile.cpp
DEP_CPP_CLIBR=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CLibraryFile.obj" : $(SOURCE) $(DEP_CPP_CLIBR)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CLibraryFile.obj" : $(SOURCE) $(DEP_CPP_CLIBR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CommandLineWnd.cpp
DEP_CPP_COMMA=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CommandLineWnd.obj" : $(SOURCE) $(DEP_CPP_COMMA)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CommandLineWnd.obj" : $(SOURCE) $(DEP_CPP_COMMA)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\Extensions.cpp
DEP_CPP_EXTEN=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\Extensions.obj" : $(SOURCE) $(DEP_CPP_EXTEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\Extensions.obj" : $(SOURCE) $(DEP_CPP_EXTEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CNodeEditor.cpp
DEP_CPP_CNODE=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CNodeEditor.obj" : $(SOURCE) $(DEP_CPP_CNODE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CNodeEditor.obj" : $(SOURCE) $(DEP_CPP_CNODE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CCommandShellEditor.cpp
DEP_CPP_CCOMM=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CCommandShellEditor.obj" : $(SOURCE) $(DEP_CPP_CCOMM)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CCommandShellEditor.obj" : $(SOURCE) $(DEP_CPP_CCOMM)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CNodeListEditor.cpp
DEP_CPP_CNODEL=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CNodeListEditor.obj" : $(SOURCE) $(DEP_CPP_CNODEL)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CNodeListEditor.obj" : $(SOURCE) $(DEP_CPP_CNODEL)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CGenericWnd.cpp
DEP_CPP_CGENE=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CGenericWnd.obj" : $(SOURCE) $(DEP_CPP_CGENE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CGenericWnd.obj" : $(SOURCE) $(DEP_CPP_CGENE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CImageEditor.cpp
DEP_CPP_CIMAG=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CImageEditor.obj" : $(SOURCE) $(DEP_CPP_CIMAG)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CImageEditor.obj" : $(SOURCE) $(DEP_CPP_CIMAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CLWindow.cpp
DEP_CPP_CLWIN=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	".\TerraFirma\WindowsExt.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CLWindow.obj" : $(SOURCE) $(DEP_CPP_CLWIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CLWindow.obj" : $(SOURCE) $(DEP_CPP_CLWIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\WindowsExt.cpp
DEP_CPP_WINDO=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	".\TerraFirma\WindowsExt.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\WindowsExt.obj" : $(SOURCE) $(DEP_CPP_WINDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\WindowsExt.obj" : $(SOURCE) $(DEP_CPP_WINDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CLWindowManager.cpp
DEP_CPP_CLWIND=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	".\TerraFirma\WindowsExt.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CLWindowManager.obj" : $(SOURCE) $(DEP_CPP_CLWIND)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CLWindowManager.obj" : $(SOURCE) $(DEP_CPP_CLWIND)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CLSizeOptions.cpp
DEP_CPP_CLSIZ=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	".\TerraFirma\WindowsExt.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CLSizeOptions.obj" : $(SOURCE) $(DEP_CPP_CLSIZ)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CLSizeOptions.obj" : $(SOURCE) $(DEP_CPP_CLSIZ) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CLArea.cpp
DEP_CPP_CLARE=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	".\TerraFirma\WindowsExt.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CLArea.obj" : $(SOURCE) $(DEP_CPP_CLARE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CLArea.obj" : $(SOURCE) $(DEP_CPP_CLARE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TerraFirma\CLImageArea.cpp
DEP_CPP_CLIMA=\
	"..\Includes\Graphics.h"\
	".\TerraFirma\TerraFirma.h"\
	".\TerraFirma\WindowsExt.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "TerraFirma - Win32 Release"


".\TerraFirma\Release\CLImageArea.obj" : $(SOURCE) $(DEP_CPP_CLIMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TerraFirma - Win32 Debug"


".\TerraFirma\Debug\CLImageArea.obj" : $(SOURCE) $(DEP_CPP_CLIMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "Graphics - Win32 Release"
# Name "Graphics - Win32 Debug"

!IF  "$(CFG)" == "Graphics - Win32 Release"

!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Graphics\Raw.cpp
DEP_CPP_RAW_C=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Graphics - Win32 Release"


".\Graphics\Release\Raw.obj" : $(SOURCE) $(DEP_CPP_RAW_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"


".\Graphics\Debug\Raw.obj" : $(SOURCE) $(DEP_CPP_RAW_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\DIB.cpp
DEP_CPP_DIB_C=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Graphics - Win32 Release"


".\Graphics\Release\DIB.obj" : $(SOURCE) $(DEP_CPP_DIB_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"


".\Graphics\Debug\DIB.obj" : $(SOURCE) $(DEP_CPP_DIB_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\GDI.cpp
DEP_CPP_GDI_C=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Graphics - Win32 Release"


".\Graphics\Release\GDI.obj" : $(SOURCE) $(DEP_CPP_GDI_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"


".\Graphics\Debug\GDI.obj" : $(SOURCE) $(DEP_CPP_GDI_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\CGScreen.cpp
DEP_CPP_CGSCR=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Graphics - Win32 Release"


".\Graphics\Release\CGScreen.obj" : $(SOURCE) $(DEP_CPP_CGSCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"


".\Graphics\Debug\CGScreen.obj" : $(SOURCE) $(DEP_CPP_CGSCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\CGLockedSurface.cpp
DEP_CPP_CGLOC=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Graphics - Win32 Release"


".\Graphics\Release\CGLockedSurface.obj" : $(SOURCE) $(DEP_CPP_CGLOC)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"


".\Graphics\Debug\CGLockedSurface.obj" : $(SOURCE) $(DEP_CPP_CGLOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\CGImageCache.cpp
DEP_CPP_CGIMA=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Graphics - Win32 Release"


".\Graphics\Release\CGImageCache.obj" : $(SOURCE) $(DEP_CPP_CGIMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"


".\Graphics\Debug\CGImageCache.obj" : $(SOURCE) $(DEP_CPP_CGIMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\CGResourceFile.cpp
DEP_CPP_CGRES=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "Graphics - Win32 Release"


".\Graphics\Release\CGResourceFile.obj" : $(SOURCE) $(DEP_CPP_CGRES)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"


".\Graphics\Debug\CGResourceFile.obj" : $(SOURCE) $(DEP_CPP_CGRES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\CGBitmap.cpp

!IF  "$(CFG)" == "Graphics - Win32 Release"

DEP_CPP_CGBIT=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

".\Graphics\Release\CGBitmap.obj" : $(SOURCE) $(DEP_CPP_CGBIT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"

DEP_CPP_CGBIT=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

".\Graphics\Debug\CGBitmap.obj" : $(SOURCE) $(DEP_CPP_CGBIT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Graphics\CGFont.cpp

!IF  "$(CFG)" == "Graphics - Win32 Release"

DEP_CPP_CGFON=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

".\Graphics\Release\CGFont.obj" : $(SOURCE) $(DEP_CPP_CGFON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Graphics - Win32 Debug"

DEP_CPP_CGFON=\
	".\Graphics\Graphics.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

".\Graphics\Debug\CGFont.obj" : $(SOURCE) $(DEP_CPP_CGFON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "ALGUI - Win32 Release"
# Name "ALGUI - Win32 Debug"

!IF  "$(CFG)" == "ALGUI - Win32 Release"

!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ALGUI\Main.cpp
DEP_CPP_MAIN_=\
	"..\Includes\Graphics.h"\
	".\ALGUI\ALGUI.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALGUI - Win32 Release"


".\ALGUI\Release\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"


".\ALGUI\Debug\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ALGUI\CUWindow.cpp
DEP_CPP_CUWIN=\
	"..\Includes\Graphics.h"\
	".\ALGUI\ALGUI.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALGUI - Win32 Release"


".\ALGUI\Release\CUWindow.obj" : $(SOURCE) $(DEP_CPP_CUWIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"


".\ALGUI\Debug\CUWindow.obj" : $(SOURCE) $(DEP_CPP_CUWIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ALGUI\CUApplication.cpp
DEP_CPP_CUAPP=\
	"..\Includes\Graphics.h"\
	".\ALGUI\ALGUI.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALGUI - Win32 Release"


".\ALGUI\Release\CUApplication.obj" : $(SOURCE) $(DEP_CPP_CUAPP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"


".\ALGUI\Debug\CUApplication.obj" : $(SOURCE) $(DEP_CPP_CUAPP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ALGUI\AutoSizeDesc.cpp
DEP_CPP_AUTOS=\
	"..\Includes\Graphics.h"\
	".\ALGUI\ALGUI.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALGUI - Win32 Release"


".\ALGUI\Release\AutoSizeDesc.obj" : $(SOURCE) $(DEP_CPP_AUTOS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"


".\ALGUI\Debug\AutoSizeDesc.obj" : $(SOURCE) $(DEP_CPP_AUTOS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ALGUI\CUFrame.cpp
DEP_CPP_CUFRA=\
	"..\Includes\Graphics.h"\
	".\ALGUI\ALGUI.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALGUI - Win32 Release"


".\ALGUI\Release\CUFrame.obj" : $(SOURCE) $(DEP_CPP_CUFRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"


".\ALGUI\Debug\CUFrame.obj" : $(SOURCE) $(DEP_CPP_CUFRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ALGUI\CULineEditor.cpp
DEP_CPP_CULIN=\
	"..\Includes\Graphics.h"\
	".\ALGUI\ALGUI.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALGUI - Win32 Release"


".\ALGUI\Release\CULineEditor.obj" : $(SOURCE) $(DEP_CPP_CULIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"


".\ALGUI\Debug\CULineEditor.obj" : $(SOURCE) $(DEP_CPP_CULIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\GPM\Libraries\Alchemy.lib

!IF  "$(CFG)" == "ALGUI - Win32 Release"

!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Alchemy"

!IF  "$(CFG)" == "ALGUI - Win32 Release"

"Alchemy - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Alchemy - Win32 Release" 

!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"

"Alchemy - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\Alchemy.mak" CFG="Alchemy - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ALGUI\CUButton.cpp
DEP_CPP_CUBUT=\
	"..\Includes\Graphics.h"\
	".\ALGUI\ALGUI.h"\
	{$(INCLUDE)}"\Alchemy.h"\
	{$(INCLUDE)}"\CodeChain.h"\
	{$(INCLUDE)}"\DDRAW.H"\
	{$(INCLUDE)}"\GlobalObjID.h"\
	{$(INCLUDE)}"\Kernel.h"\
	{$(INCLUDE)}"\KernelObjID.h"\
	

!IF  "$(CFG)" == "ALGUI - Win32 Release"


".\ALGUI\Release\CUButton.obj" : $(SOURCE) $(DEP_CPP_CUBUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ALGUI - Win32 Debug"


".\ALGUI\Debug\CUButton.obj" : $(SOURCE) $(DEP_CPP_CUBUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
