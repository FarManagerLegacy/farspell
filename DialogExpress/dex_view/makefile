dex_view.dll: dex_view.c dex_view.def ../.msvc.aided-release/dialogres_static.lib
	cl /LD /MD /GL /Ox /Zp2 dex_view.c dex_view.def \
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
