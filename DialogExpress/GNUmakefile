#
#    makefile for build everything in this folder by GCC.
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
override CC = gcc -g -D_DEBUG $(COMMONFLAGS)
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
	-@$(MAKE) --no-print-directory 'TARGET=Aided Release' all

ifneq ("$(TARGET)","Release")
# Prevents from build of something useless cdialog.exe
executables: cdialog.exe$(OUTDIR) metadialog.exe$(OUTDIR)
$(OUTDIR)/cdialog.exe: cdialog.c $(OUTDIR)/dialogres-static.a
	$(CC) -o $@ cdialog.c $(OUTDIR)/dialogres-static.a $(LINKFLAGS)
cdialog.exe$(OUTDIR): $(OUTDIR)/cdialog.exe
	-$(RM) cdialog.exe.*.aided-release
	-$(RM) cdialog.exe.*.release
	-$(RM) cdialog.exe.*.debug
	cp $(OUTDIR)/cdialog.exe cdialog.exe
	echo Build stamp > cdialog.exe$(OUTDIR)
$(OUTDIR)/metadialog.exe: metadialog.c metaparse.c metaparse.h \
             $(OUTDIR)/dialogres-static.a
	$(CC) -o $@ metadialog.c metaparse.c $(OUTDIR)/dialogres-static.a $(LINKFLAGS)
metadialog.exe$(OUTDIR): $(OUTDIR)/metadialog.exe
	-$(RM) metadialog.exe.*
	cp $(OUTDIR)/metadialog.exe metadialog.exe
	echo Build stamp > $@
else
executables: cdialog.exe.gcc.aided-release metadialog.exe.gcc.aided-release
cdialog.exe.gcc.aided-release:
	$(MAKE) --no-print-directory 'TARGET=Aided Release' cdialog.exe.gcc.aided-release 
metadialog.exe.gcc.aided-release:
	$(MAKE) --no-print-directory 'TARGET=Aided Release' metadialog.exe.gcc.aided-release 
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

$(OUTDIR)/dialogres-static.a: $(OUTDIR)/parse.o $(OUTDIR)/tokenize.o \
 $(OUTDIR)/env.o $(OUTDIR)/dialogres.o
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
$(OUTDIR)/env.o: env.c $(OUTDIR)/env.h $(OUTDIR)/envcol.h dialogresInt.h
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

$(OUTDIR)/env.h: $(OUTDIR)/mkconstants.exe constants.txt
	$(CMD) "$(OUTDIR)\\mkconstants.exe" < constants.txt > $@

$(OUTDIR)/mkconstants.exe: mkconstants.c 
	$(MAKE) --no-print-directory $(OUTDIR)
	$(CC) -o $@ mkconstants.c 

$(OUTDIR)/envcol.h: $(OUTDIR)/mkcolors.exe farcolor.txt
	$(CMD) "$(OUTDIR)\\mkcolors.exe" < farcolor.txt > $@

$(OUTDIR)/mkcolors.exe: mkcolors.c
	$(MAKE) --no-print-directory $(OUTDIR)
	$(CC) -o $@ mkcolors.c

$(OUTDIR):
	mkdir $(OUTDIR)

$(LEMON):
	$(MAKE) -C ../make-tools lemon.exe

clean:
	-$(RM) --force parse.c parse.h parse.out
	-$(RM) --force keywordhash.h mkkeywordhash.exe cdialog.exe* metadialog.exe*
	-$(RM) --force *.o *.a
	-$(RM) --force -r .gcc.* 
	

distclean: clean
	$(MAKE) -C ../make-tools clean
