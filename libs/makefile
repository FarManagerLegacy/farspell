!ifndef CUSTOM_CODEPAGE_OUT
CUSTOM_CODEPAGE_OUT = .
!endif

"$(CUSTOM_CODEPAGE_OUT)\CustomCodePage.lib": \
 "$(CUSTOM_CODEPAGE_OUT)\CustomCodePage.obj" \
 "$(CUSTOM_CODEPAGE_OUT)\CustomCodePageHook.obj"
	$(LIB) -ltcg -out:$@ \
 "$(CUSTOM_CODEPAGE_OUT)\CustomCodePageHook.obj" \
 "$(CUSTOM_CODEPAGE_OUT)\CustomCodePage.obj"

"$(CUSTOM_CODEPAGE_OUT)\CustomCodePage.obj": CustomCodePage.c 
	$(CC) -c -MT -GL -Ox CustomCodePage.c -Fo$@ -D_NO_IMAGEHLP_LIB 

"$(CUSTOM_CODEPAGE_OUT)\CustomCodePageHook.obj": CustomCodePageHook.c
	$(CC) -c -MT -GL -Ox CustomCodePageHook.c -Fo$@ -D_NO_IMAGEHLP_LIB 

clean:
	-@del "$(CUSTOM_CODEPAGE_OUT)\CustomCodePage.obj"
	-@del "$(CUSTOM_CODEPAGE_OUT)\CustomCodePageHook.obj"
	-@del "$(CUSTOM_CODEPAGE_OUT)\CustomCodePage.lib"