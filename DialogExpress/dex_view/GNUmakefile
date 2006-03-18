#
#    makefile for build Dialog resources viewer plugin by GCC.
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

dex_view.dll: dex_view.o dex_view.def ../.gcc.aided-release/dialogres-static.a farlng.o
	dllwrap -mno-cygwin -o $@ dex_view.o --def dex_view.def \
 ../.gcc.aided-release/dialogres-static.a farlng.o \
 -Wl,-s,-Map,dex_view.map,--enable-stdcall-fixup

dex_view.o: dex_view.cpp ../dialogres.h
	g++ -c -Wall -mdll -mno-cygwin -O3 -I../../../../PlugDoc/Headers.c $< -o $@

farlng.o: ../../FarLNG/farlng.cpp
	g++ -c -Wall -mdll -mno-cygwin -O3 $< -o $@

../.gcc.aided-release/dialogres-static.a:
	$(MAKE) -C .. aided .gcc.aided-release/dialogres-static.a

clean:
	-$(RM) dex_view.o
	-$(RM) dex_view.lib
	-$(RM) dex_view.exp
	-$(RM) dex_view.dll
	-$(RM) dex_view.map
	-$(RM) farlng.o