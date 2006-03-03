colorPlugin.dll: colorPlugin.o colorPlugin.def
	dllwrap -mno-cygwin -o $@ colorPlugin.o --def colorPlugin.def -Wl,-s,-Map,colorPlugin.map,--enable-stdcall-fixup

colorPlugin.o: colorPlugin.c color.c
	gcc -c -mdll -mno-cygwin -O3 $< -o $@

color.c: color.dlg
	$(MAKE) -C .. aided
	../cdialog.exe $< > $@

clean:
	-$(RM) color.c
	-$(RM) color.dlg.log
	-$(RM) colorPlugin.o
	-$(RM) colorPlugin.lib
	-$(RM) colorPlugin.exp
	-$(RM) colorPlugin.dll
	-$(RM) colorPlugin.map