#
#    makefile for build FarSpell by Visual C compiler.
#    Copyright (c) Sergey Shishmintzev, Kiev 2005-2006
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

ROOT = ..
!include $(ROOT)\config.mak

!ifdef EXINFO
!include makefile.exinfo
!endif

COMMON_LIBS = $(ROOT)\libs
Hunspell = F:\devel\spell\hunspell-1.1.4-koifix\src

FARPlus = $(ROOT)\FARPlus
DEX = $(ROOT)\DialogExpress
CINCLUDE = /I"$(PLUGINSDK_HEADERS)" /I"$(ROOT)" /I$(Hunspell)
 
CPPFILES = farspell.cpp FarSpell_UI.cpp \
  ParserFactory.cpp SpellFactory.cpp \
  DecisionTable.cpp DictViewFactory.cpp
DIST = farspell

CFLAGS = /nologo /Zp2 /LD /EHsc /DUSE_FAR_170 /D_export= $(CINCLUDE)
LINKFLAGS = /nologo /dll /opt:ref
!ifdef DEBUG
!ifdef RELEASE
!error DEBUG/RELEASE conflict!
!endif
BUILT_STAMP = farspell.dll-debug
OBJ_FILES = $(CPPFILES:.cpp=.obj_d)
HUNSPELL_LIBS = $(Hunspell)/hunspell/hunspell_d.lib \
  $(Hunspell)/parsers/hunparsers_d.lib
FARPLUS_LIB = $(FARPlus)/farplus_d.lib
FARPLUS_OPTS = $(FARPlus)/FARDbg.obj_d $(FARPlus)/FarDbg_PDB.obj_d
CC = cl $(CFLAGS) /D_DEBUG /Ge /Od /Zi
#CC_DLL = $(CC)
CC_DLL = link $(LINKFLAGS) /debug
.cpp.obj_d:
	$(CC) /c $< /Fo$@
!else
!ifdef DEBUG
!error DEBUG/RELEASE conflict!
!endif DEBUG
BUILT_STAMP = farspell.dll-release
OBJ_FILES = $(CPPFILES:.cpp=.obj_r)
HUNSPELL_LIBS = $(Hunspell)/hunspell/hunspell_r.lib \
  $(Hunspell)/parsers/hunparsers_r.lib
FARPLUS_LIB = $(FARPlus)/farplus_r.lib
FARPLUS_OPTS =
CC = cl $(CFLAGS) /DNDEBUG /Ox /Gy /GL
#CC_DLL = $(CC)
CC_DLL = link $(LINKFLAGS) /ltcg
.cpp.obj_r:
	$(CC) /c $< /Fo$@
!endif !DEBUG
MYLIBS = $(FARPLUS_OPTS) $(FARPLUS_LIB) $(HUNSPELL_LIBS) CustomCodePage.lib
LIBS = $(MYLIBS) advapi32.lib user32.lib kernel32.lib ole32.lib oleaut32.lib

MJU_SED = $(ROOT)\make-tools\mju-sed.exe

!ifdef RELEASE_BUILT
!ifdef DEBUG_BUILT
!error Something wrong, please run "nmake clean"
!endif DEBUG_BUILT
!endif RELEASE_BUILT

!ifdef DEBUG_BUILT
!ifdef RELEASE_BUILT
!error Something wrong, please run "nmake clean"
!endif RELEASE_BUILT
!endif DEBUG_BUILT

default: makefile.exinfo
!ifdef RELEASE_BUILT
	$(MAKE) /nologo EXINFO=1 release
!elseifdef DEBUG_BUILT
	$(MAKE) /nologo EXINFO=1 debug
!elseifdef EXINFO
	$(MAKE) /nologo EXINFO=1 debug
!else
	$(MAKE) /nologo EXINFO=1 default
	-del makefile.exinfo
!endif !RELEASE_BUILT

release: makefile.exinfo
	-@mkdir farspell > nul
	$(COMSPEC) /c if exist farspell.dll-debug del $(DIST)\farspell.dll-debug $(DIST)\farspell.dll $(DIST)\farspell.obj
	$(MAKE) /nologo EXINFO=1 RELEASE=1 $(DIST)\farspell.dll
!ifndef EXINFO
	del makefile.exinfo
!endif !EXINFO

farspell.dll-release:
	echo "Release built at &[TIMESTAMP]" | $(MJU_SED) > $@

debug: makefile.exinfo
	-@mkdir farspell > nul
	$(COMSPEC) /c if exist farspell.dll-release del $(DIST)\farspell.dll-release $(DIST)\farspell.dll $(DIST)\farspell.obj
	$(MAKE) /nologo EXINFO=1 DEBUG=1 $(DIST)\farspell.dll
!ifndef EXINFO
	del makefile.exinfo
!endif !EXINFO

farspell.dll-debug: 
	echo "Debug built at &[TIMESTAMP]" | $(MJU_SED) > $@

makefile.exinfo: $(MJU_SED)
  @echo Put time-stamp to makefile.exinfo
  @$(MJU_SED) > $@ < <<
TIMESTAMP = &Y&m&d
<<NOKEEP
	@if exist farspell.dll-release echo RELEASE_BUILT = 1 >> $@
	@if exist farspell.dll-release echo DIST_FILE = farspell-$$(TIMESTAMP) >> $@
	@if exist farspell.dll-debug echo DEBUG_BUILT = 1 >> $@
	@if exist farspell.dll-debug echo DIST_FILE = farspell-$$(TIMESTAMP)-debug >> $@

$(MJU_SED):
	cd $(@D)
	$(MAKE) /nologo -f makefile.vc mju-sed.exe
	cd "$(MAKEDIR)"

farspell.hpp: FarSpell.h ParserFactory.hpp SpellFactory.hpp DictViewFactory.cpp
farspell.cpp: farspell.hpp codepages.cpp 
farspell_UI.cpp: farspell.hpp dialogs.cpp ftl.hpp
ParserFactory.cpp: ParserFactory.hpp FarSpell.h
SpellFactory.cpp: SpellFactory.hpp

$(DIST)\farspell.dll: templates $(OBJ_FILES) farspell.def $(DIST)\farspell.res \
 	$(MYLIBS)
	-del farspell.dll-release
	-del farspell.dll-debug
#	$(CC_DLL) $(LIBS) $(OBJ_FILES) farspell.def /link /opt:noref
	$(CC_DLL) $(OBJ_FILES) /def:farspell.def $(LIBS) /incremental:no \
	          $(DIST)\farspell.res /map:$(DIST)\farspell.map /out:$@
	$(MAKE) /nologo $(BUILT_STAMP)

.rc.res:
	$(RC) $<

dialogs.cpp: FarSpell.dlg $(DEX)\metadialog.exe $(DEX)\metadialog_cpp.txt
	$(DEX)\metadialog.exe $(DEX)\metadialog_cpp.txt FarSpell.dlg > $@

FarSpell.dlg: FarSpell.h

$(DEX)\metadialog.exe:
	cd "$(DEX)"
	$(MAKE) debug
	cd "$(MAKEDIR)"

CustomCodePage.lib:
	cd "$(COMMON_LIBS)"
	$(MAKE) "CUSTOM_CODEPAGE_OUT=$(MAKEDIR)" "$(MAKEDIR)\CustomCodePage.lib" "LIB=link /lib"
	cd "$(MAKEDIR)"

$(FARPLUS_LIB) $(FARPLUS_OPTS):
	cd $(FARPlus)
        $(MAKE) /nologo
	cd "$(MAKEDIR)"

$(Hunspell)\hunspell\makefile.vc: makefile.hunspell
	copy makefile.hunspell $@

$(Hunspell)\parsers\makefile.vc: makefile.parsers
	copy makefile.parsers $@

$(HUNSPELL_LIBS): $(Hunspell)\hunspell\makefile.vc $(Hunspell)\parsers\makefile.vc
	cd $(Hunspell)\hunspell
        $(MAKE) /nologo /f makefile.vc
	cd $(ROOT)\parsers
        $(MAKE) /nologo /f makefile.vc
	cd "$(MAKEDIR)"

clean:
	cd "$(COMMON_LIBS)"
	-$(MAKE) /nologo "-DCUSTOM_CODEPAGE_OUT=$(MAKEDIR)" clean
	cd "$(MAKEDIR)"
	-del farspell.dll* farspell.exp farspell.ilk farspell.lib *.obj* *.pdb
	-del www\index.html
	-del farspell\*.* /Q
	-rmdir farspell
	-del dialogs.cpp
	-del FarSpell.dlg.log

distclean: clean
	cd $(DEX)
	$(MAKE) /nologo -f makefile clean
	cd "$(MAKEDIR)"
	cd $(ROOT)\make-tools
	$(MAKE) /nologo -f makefile.vc clean
	cd "$(MAKEDIR)"
	cd $(FARPlus)
        -$(MAKE) /nologo clean
	cd $(Hunspell)\hunspell
        -$(MAKE) /nologo /f makefile.vc clean
        -del makefile.vc
	cd (ROOT)\parsers
        -$(MAKE) /nologo /f makefile.vc clean
        -del makefile.vc
	cd "$(MAKEDIR)"

templates: $(MJU_SED) $(DIST)\file_id.diz $(DIST)\FarSpell.rc \
  $(DIST)\history.txt $(DIST)\todo.txt $(DIST)\TechInfo.txt \
  $(DIST)\dbghelp.dll \
  $(DIST)\FarSpellEng.hlf $(DIST)\FarSpellEng.lng \
  $(DIST)\FarSpellRus.hlf $(DIST)\FarSpellRus.lng \
  $(DIST)\WordChars.reg $(DIST)\cp1125.txt
#  $(DIST)\FarSpell.dlg \

www: $(MJU_SED) www\index.html

$(DIST)\file_id.diz: file_id.diz
	-@mkdir farspell > nul
	$(MJU_SED) < $** > $@
$(DIST)\FarSpell.rc: FarSpell.rc
	$(MJU_SED) < $** > $@
#$(DIST)\FarSpell.dlg: FarSpell.dlg
#	$(MJU_SED) < $** > $@
$(DIST)\FarSpellEng.hlf: FarSpellEng.hlf
	copy $** $@
$(DIST)\FarSpellEng.lng: FarSpellEng.lng
	copy $** $@
$(DIST)\FarSpellRus.hlf: FarSpellRus.hlf
	copy $** $@
$(DIST)\FarSpellRus.lng: FarSpellRus.lng
	copy $** $@
$(DIST)\dbghelp.dll: $(FARPlus)\dbghelp.dll
	copy $** $@
$(DIST)\history.txt: history.txt
	copy $** $@
$(DIST)\todo.txt: todo.txt
	copy $** $@
$(DIST)\WordChars.reg: WordChars.reg
	copy $** $@
$(DIST)\cp1125.txt: cp1125.txt
	copy $** $@
www\index.html: www\index_t.html
	$(MJU_SED) < $** > $@
$(DIST)\TechInfo.txt: TechInfo.txt
	$(MJU_SED) < $** > $@

dist: makefile.exinfo templates
!ifndef EXINFO
#	if not exist makefile.exinfo !error
	$(MAKE) /nologo makefile.exinfo
	if exist farspell.dll-release $(MAKE) /nologo EXINFO=1 RELEASE=1 dist
	if exist farspell.dll-debug $(MAKE) /nologo EXINFO=1 DEBUG=1 dist
	del makefile.exinfo
!else
	-@del $(DIST_FILE).7z > nul
	if exist $(DIST_FILE).7z !error Unable to delete farspell-$(TIMESTAMP)$(DIST_SUFFIX).7z
	cd $(DIST)
	$(SEVEN_ZIP) a -t7z -mx9 -ms ..\$(DIST_FILE) file_id.diz 
	cd ..
	$(SEVEN_ZIP) a -t7z -mx9 -ms $(DIST_FILE) @<< 
 !clean_winxp.reg
 !clean_win9x.reg
 !clean.txt
$(DIST)\farspell.dll 
$(DIST)\file_id.diz 
$(DIST)\history.txt 
$(DIST)\todo.txt 
$(DIST)\TechInfo.txt
$(DIST)\WordChars.reg
$(DIST)\cp1125.txt
$(DIST)\FarSpellEng.hlf 
$(DIST)\FarSpellEng.lng 
$(DIST)\FarSpellRus.hlf 
$(DIST)\FarSpellRus.lng 
$(DIST)\FarSpell.map
!ifdef DEBUG_BUILT
$(DIST)\dbghelp.dll 
$(DIST)\farspell.pdb
!endif DEBUG
<<NOKEEP
!endif EXINFO

full:
	$(MAKE) /nologo clean debug dist 
	$(MAKE) /nologo release dist www
#	$(MAKE) /nologo www