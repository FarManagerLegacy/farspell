ROOT = ..
!include $(ROOT)\config.mak

CC=cl
LIB=lib /nologo
COMMON_FLAGS = /c  /nologo /DUSE_FAR_170  /Zp2 /D_export= /D_FARPLUS=1 \
 -I"$(PLUGINSDK_HEADERS)"
RELEASE_FLAGS = /LD /Ox /Gy  /GL
DEBUG_FLAGS =  /LD /Od /Ge /Zi /D_DEBUG
FARPlusCPP = >FAR_SF.cpp \
  >FARArray.cpp \
  >FARDialog.cpp \
  >FAREd.cpp \
  >FARFile.cpp \
  >FARLog.cpp \
  >FARMenu.cpp \
  >FARPlus.cpp \
  >FARRegistry.cpp \
  >FARString.cpp \
  >FARXml.cpp

FARPlusOptionalCPP = >FARDbg.cpp >FarDbg_PDB.cpp >MicroRTL.cpp
# Use these files explicitly:
#  FARDbg.cpp FarDbg_PDB.cpp MicroRTL.cpp
#  (i.e. nmake MicroRTL.obj_d & link ... MicroRTL.obj_d ... )

FARPlusH = FARArray.h \
  FARDbg.h \
  FARDialog.h \
  FAREd.h \
  FARFile.h \
  FARLog.h \
  FARMemory.h \
  FARMenu.h \
  FARPlus.h \
  FARRegistry.h \
  FARString.h \
  FARXml.h 

FARPlusObj = $(FARPlusCPP:.cpp=.obj)
FARPlusReleaseObj = $(FARPlusObj:>=.msvc.release\)
FARPlusDebugObj = $(FARPlusObj:>=.msvc.debug\)

FARPlusOptionalObj = $(FARPlusOptionalCPP:.cpp=.obj)
FARPlusOptionalReleaseObj = $(FARPlusOptionalObj:>=.msvc.release\)
FARPlusOptionalDebugObj = $(FARPlusOptionalObj:>=.msvc.debug\)

all: debug release

debug: .msvc.debug .msvc.debug\farplus.lib $(FARPlusOptionalDebugObj)
release: .msvc.release .msvc.release\farplus.lib $(FARPlusOptionalReleaseObj) 

".msvc.release\farplus.lib": $(FARPlusReleaseObj) 
	$(LIB) /OUT:$@ $(FARPlusReleaseObj)

".msvc.debug\farplus.lib": $(FARPlusDebugObj)
	$(LIB) /OUT:$@ $(FARPlusDebugObj)

$(FARPlusReleaseObj): $(FARPlusH)
$(FARPlusDebugObj): $(FARPlusH)
$(FARPlusOptionalReleaseObj): $(FARPlusH)
$(FARPlusOptionalDebugObj): $(FARPlusH)

.cpp{.msvc.release}.obj: 
	$(CC) $(RELEASE_FLAGS) $(COMMON_FLAGS) $< /Fo$@ -Fd$(@D)\farplus.pdb

".msvc.release":
	mkdir $@

.cpp{.msvc.debug}.obj:
	$(CC) $(DEBUG_FLAGS) $(COMMON_FLAGS) $< /Fo$@ -Fd$(@D)\farplus.pdb

".msvc.debug":
	mkdir $@

clean:
	-del /Q .msvc.release\*.obj
	-del /Q .msvc.release\*.lib
	-del /Q .msvc.release\*.pdb
	-rmdir.msvc.release
	-del /Q .msvc.debug\*.obj
	-del /Q .msvc.debug\*.lib
	-del /Q .msvc.debug\*.pdb
	-rmdir .msvc.debug
	-del *.pdb
