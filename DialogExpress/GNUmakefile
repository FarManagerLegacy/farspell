ifndef TARGET
TARGET = Debug
endif
$(warning Target is $(TARGET))

CMD = cmd.exe /c
LEMON = ../make-tools/lemon.exe
CLIB = ar
COMMONFLAGS =  -mno-cygwin -I../../../PlugDoc/Headers.c

ifeq ("$(TARGET)","Release")
override CC = gcc -O3 -DNDEBUG $(COMMONFLAGS)
override LINKFLAGS = -Wl,-s
override OUTDIR = .gcc.release
endif
ifeq ("$(TARGET)","Aided Release")
override LEMONFLAGS = -c
override CC = gcc -O3 -DNDEBUG $(COMMONFLAGS)
override LINKFLAGS = -Wl,-s
override OUTDIR = .gcc.aided-release
endif
ifeq ("$(TARGET)","Debug")
override LEMONFLAGS = -c
override CC = gcc -g $(COMMONFLAGS)
override LINKFLAGS =
override OUTDIR = .gcc.debug
endif

ifndef OUTDIR
$(error OUTDIR undefined)
endif

all: executables $(OUTDIR)/dialogres.pdll
debug: all
release:
	-@$(MAKE) --no-print-directory TARGET=Release all
aided:
	-@$(MAKE) --no-print-directory "TARGET=Aided Release" all

ifneq ("$(TARGET)","Release")
# Prevents from build of something useless cdialog.exe
executables: cdialog.exe$(OUTDIR)
$(OUTDIR)/cdialog.exe: cdialog.c $(OUTDIR)/dialogres-static.a
	$(CC) -o $@ cdialog.c $(OUTDIR)/dialogres-static.a $(LINKFLAGS)
cdialog.exe$(OUTDIR): $(OUTDIR)/cdialog.exe
	-$(RM) cdialog.exe.*.aided-release
	-$(RM) cdialog.exe.*.release
	-$(RM) cdialog.exe.*.debug
	cp $(OUTDIR)/cdialog.exe cdialog.exe
	echo Build stamp > cdialog.exe$(OUTDIR)
else
executables: cdialog.exe.gcc.aided-release
cdialog.exe.gcc.aided-release:
	$(MAKE) TARGET="Aided Release" cdialog.exe.gcc.aided-release
endif

$(OUTDIR)/dialogres.pdll: $(OUTDIR)/dialogres-static.a $(OUTDIR)/dialogres_dll.o dialogres.def
#	$(CC) -o $@ -mdll $(OUTDIR)/dialogres-static.a --def dialogres.def $(LINKFLAGS)
	dllwrap -mno-cygwin -o $@ $(OUTDIR)/dialogres_dll.o \
		$(OUTDIR)/dialogres-static.a  \
		--def dialogres.def --output-lib $(OUTDIR)/dialogres.lib  \
		--output-exp $(OUTDIR)/dialogres.exp \
		-Wl,-Map,$(OUTDIR)/dialogres.map
ifneq ("$(TARGET)","Debug")
	strip $@
endif

$(OUTDIR)/dialogres-static.a: $(OUTDIR)/parse.o $(OUTDIR)/tokenize.o $(OUTDIR)/dialogres.o
	$(AR) qs $@ $^

dialogresInt.h: dialogres.h
$(OUTDIR)/dialogres_dll.o: dialogres_dll.c
	$(CC) -c $< -o $@
$(OUTDIR)/dialogres.o: dialogres.c dialogresInt.h
	$(CC) -c $< -o $@
$(OUTDIR)/tokenize.o: tokenize.c $(OUTDIR)/parse.h keywordhash.h
	$(CC) -c $< -o $@  -I$(OUTDIR)
$(OUTDIR)/parse.o: $(OUTDIR)/parse.c $(OUTDIR)/parse.h dialogresInt.h
	$(CC) -c $< -o $@ -I$(OUTDIR)

keywordhash.h: $(OUTDIR)/mkkeywordhash.exe keywords.txt
	$(CMD) "$(OUTDIR)\\mkkeywordhash.exe" < keywords.txt > keywordhash.h 2> nul

$(OUTDIR)/parse.c $(OUTDIR)/parse.h: parse.y 
	$(MAKE) --no-print-directory $(OUTDIR) $(LEMON)
	$(LEMON) $(LEMONFLAGS) parse.y
	mv parse.c $(OUTDIR)/parse.c
	mv parse.h $(OUTDIR)/parse.h

$(OUTDIR)/mkkeywordhash.exe: mkkeywordhash.c 
	$(MAKE) --no-print-directory $(OUTDIR)
	$(CC) -o $@ mkkeywordhash.c 

$(OUTDIR):
	mkdir $(OUTDIR)

$(LEMON):
	$(MAKE) -C ../make-tools lemon.exe

clean:
	-$(RM) --force parse.c parse.h parse.out
	-$(RM) --force keywordhash.h mkkeywordhash.exe cdialog.exe*
	-$(RM) --force *.o *.a
	-$(RM) --force -r .gcc.* 
	

distclean: clean
	$(MAKE) -C ../make-tools clean
