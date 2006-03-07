dex_view.dll: dex_view.o dex_view.def ../.gcc.aided-release/dialogres-static.a
	dllwrap -mno-cygwin -o $@ dex_view.o ../.gcc.aided-release/dialogres-static.a --def dex_view.def -Wl,-s,-Map,dex_view.map,--enable-stdcall-fixup

dex_view.o: dex_view.c ../dialogres.h 
	gcc -c -Wall -mdll -mno-cygwin -O3 -I../../../../PlugDoc/Headers.c $< -o $@


../.gcc.aided-release/dialogres-static.a:
	$(MAKE) -C .. aided .gcc.aided-release/dialogres-static.a

clean:
	-$(RM) dex_view.o
	-$(RM) dex_view.lib
	-$(RM) dex_view.exp
	-$(RM) dex_view.dll
	-$(RM) dex_view.map