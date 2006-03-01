colorPlugin.dll: colorPlugin.c color.c colorPlugin.def
	cl /LD /MD /GL /Ox colorPlugin.c colorPlugin.def /Fm kernel32.lib

color.c: color.dlg
	cd ..
	$(MAKE) -nologo cdialog.exe
	cd "$(MAKEDIR)"
	..\cdialog.exe color.dlg > $@

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
