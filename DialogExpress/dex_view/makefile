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

dex_view.dll: dex_view.cpp dex_view.def ../.msvc.aided-release/dialogres_static.lib
	cl /LD /MD /GL /Ox /Zp2 dex_view.cpp dex_view.def \
 -I../../../../PlugDoc/Headers.c \
 ../.msvc.aided-release/dialogres_static.lib \
 /Fm kernel32.lib

../.msvc.aided-release/dialogres_static.lib:
	cd ..
	$(MAKE) /nologo aided .msvc.aided-release/dialogres_static.lib
	cd "$(MAKEDIR)"

clean:
	-del dex_view.obj
	-del dex_view.lib
	-del dex_view.exp
	-del dex_view.dll
	-del dex_view.map
	-del dex_view.pdb
	-del dex_view.ilk
