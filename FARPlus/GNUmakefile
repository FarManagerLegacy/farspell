CINCLUDE="C:/Program Files/Far/PlugDoc/Headers.c"
CC=g++ -I$(CINCLUDE)
LIB=ld
COMMON_FLAGS = -c -mno-cygwin -DUSE_FAR_170  -D_FARPLUS=1
RELEASE_FLAGS = -O3  -DNDEBUG
DEBUG_FLAGS =  -g -D_DEBUG -feliminate-unused-debug-symbols
FARPlusRTL = >FAR_SF.o \
 >FARArray.o \
 >FARDialog.o \
 >FAREd.o \
 >FARFile.o \
 >FARLog.o \
 >FARMenu.o \
 >FARPlus.o \
 >FARRegistry.o \
 >FARString.o \
 >FARXml.o

FARPlusOptionalRTL = >FARDbg.o >FarDbg_ImageHlp.o >MicroRTL.o
# Use these files explicitly:
#  FARDbg.cpp FarDbg_ImageHlp.cpp MicroRTL.cpp
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

FARPlusObj = ${FARPlusRTL:>%=.gcc.release/%}
FARPlusDebugObj = $(FARPlusRTL:>%=.gcc.debug/%)

FARPlusOptionalObj = $(FARPlusOptionalRTL:>%=.gcc.release/%)
FARPlusOptionalDebugObj = $(FARPlusOptionalRTL:>%=.gcc.debug/%)

all: debug release

debug: .gcc.debug .gcc.debug/farplus.a $(FARPlusOptionalDebugObj)
release: .gcc.release .gcc.release/farplus.a $(FARPlusOptionalObj) 

.gcc.release/farplus.a: $(FARPlusObj) 
	ar qs $@ $^

.gcc.debug/farplus.a: $(FARPlusDebugObj)  
	ar qs $@ $^

$(FARPlusObj): $(FARPlusH)
$(FARPlusDebugObj): $(FARPlusH)

.gcc.release/%.o: %.cpp
	$(CC) $(RELEASE_FLAGS) $(COMMON_FLAGS) $< -o $@ 

.gcc.debug/%.o: %.cpp
	$(CC) $(DEBUG_FLAGS) $(COMMON_FLAGS) $< -o $@ 

.gcc.release:
	mkdir $@

.gcc.debug:
	mkdir $@

clean:
	-rm -rf .gcc.release
	-rm -rf .gcc.debug

