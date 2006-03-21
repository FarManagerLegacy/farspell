#
#    makefile for build Dialog resources viewer plugin by VC.
#    Copyright (c) Sergey Shishmintzev, Kiev 2006
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    Contributor(s):
#      Sergey Shishmintzev <sergey.shishmintzev@gmail.com>

ROOT = ..\..
!include $(ROOT)\config.mak
DEX = ..

!ifndef TARGET
TARGET = Release
!endif            
!message Target is $(TARGET)

COMMONFLAGS =  -nologo 
FARPLUS = $(ROOT)\FARPlus

!if "$(TARGET)"=="Release"
DIALOGRES_LIB = .msvc.aided-release/dialogres_static.lib
LIBS = 
CC = cl $(COMMONFLAGS) -MD -GL -Og -Ox -DNDEBUG 
!endif

!if "$(TARGET)"=="Debug"
DIALOGRES_LIB = .msvc.debug/dialogres_static.lib
LIBS =  $(FARPLUS)/FarDbg_PDB.cpp
CC = cl $(COMMONFLAGS) -MT -Od -Fd -Zi -D_DEBUG -EHsc
!endif

dex_view.dll: dex_view.cpp dex_view.def $(DEX)/$(DIALOGRES_LIB) $(LIBS) \
   $(ROOT)/FarLNG/farlng.h farlng.obj
	$(CC) /o $@ -LD dex_view.cpp dex_view.def \
 -I"$(PLUGINSDK_HEADERS)" \
 $(DEX)/$(DIALOGRES_LIB) $(LIBS) farlng.obj user32.lib kernel32.lib -Fm \
 -link -NODEFAULTLIB:LIBC

farlng.obj: $(ROOT)\FarLNG\farlng.cpp
	$(CC) /c /o $@ $(ROOT)\FarLNG\farlng.cpp

$(DEX)/$(DIALOGRES_LIB):
	cd $(DEX)
	$(MAKE) -nologo aided $(DIALOGRES_LIB)
	cd "$(MAKEDIR)"

clean:
	-del dex_view.obj
	-del dex_view.lib
	-del dex_view.exp
	-del dex_view.dll
	-del dex_view.map
	-del dex_view.pdb
	-del dex_view.ilk
	-del farlng.obj
	-del vc70.pdb
	-del FarDbg_PDB.obj
