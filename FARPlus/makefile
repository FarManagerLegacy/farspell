ROOT = ..
!include $(ROOT)\config.mak

CC=cl
LIB=lib /nologo
COMMON_FLAGS = /c  /nologo /DUSE_FAR_170  /Zp2 /D_export= /D_FARPLUS=1 \
 -I"$(PLUGINSDK_HEADERS)"
RELEASE_FLAGS = /LD /Ox /Gy  /GL
DEBUG_FLAGS =  /LD /Od /Ge /Zi /D_DEBUG
FARPlusCPP = FAR_SF.cpp \
  FARArray.cpp \
  FARDialog.cpp \
  FAREd.cpp \
  FARFile.cpp \
  FARLog.cpp \
  FARMenu.cpp \
  FARPlus.cpp \
  FARRegistry.cpp \
  FARString.cpp \
  FARXml.cpp

FARPlusOptionalCPP = FARDbg.cpp FarDbg_PDB.cpp MicroRTL.cpp
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

FARPlusObj = $(FARPlusCPP:.cpp=.obj_r)
FARPlusDebugObj = $(FARPlusCPP:.cpp=.obj_d)
FARPlusOptionalObj = $(FARPlusOptionalCPP:.cpp=.obj_r)
FARPlusOptionalDebugObj = $(FARPlusOptionalCPP:.cpp=.obj_d)

all: debug release

debug: farplus_d.lib $(FARPlusOptionalDebugObj)
release: farplus_r.lib $(FARPlusOptionalObj) 

farplus_r.lib: $(FARPlusObj) 
	$(LIB) /OUT:farplus_r.lib $(FARPlusObj)

farplus_d.lib: $(FARPlusDebugObj)  
	$(LIB) /OUT:farplus_d.lib $(FARPlusDebugObj)

$(FARPlusObj): $(FARPlusH)
$(FARPlusDebugObj): $(FARPlusH)

.cpp.obj_r: 
	$(CC) $(RELEASE_FLAGS) $(COMMON_FLAGS) $< /Fo$@ -Fdfarplus_r.pdb

.cpp.obj_d:
	$(CC) $(DEBUG_FLAGS) $(COMMON_FLAGS) $< /Fo$@ -Fdfarplus_d.pdb

clean:
	-del $(FARPlusObj) 
	-del $(FARPlusDebugObj) 
	-del $(FARPlusOptionalObj)
	-del $(FARPlusOptionalDebugObj)
	-del farplus_r.lib 
	-del farplus_d.lib 
	-del *.pdb
