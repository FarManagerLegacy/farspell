#
#    makefile for build sample plugin by VC.
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

colorPlugin.dll: colorPlugin.c color.c colorPlugin.def
	cl /LD /MD /Ox /I"$(PLUGINSDK_HEADERS)" colorPlugin.c colorPlugin.def \
 /Fm /nodefaultlib kernel32.lib

color.c: color.dlg $(DEX)/metadialog_c.txt
	cd $(DEX)
	$(MAKE) -nologo aided
	cd "$(MAKEDIR)"
	$(DEX)\metadialog.exe $(DEX)\metadialog_c.txt color.dlg > $@

clean:
	-del color.c
	-del color.dlg.log
	-del colorPlugin.obj
	-del colorPlugin.lib
	-del colorPlugin.exp
	-del colorPlugin.dll
	-del colorPlugin.map
	-del colorPlugin.pdb
	-del colorPlugin.ilk
