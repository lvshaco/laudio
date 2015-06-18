.PHONY: audio.so 

CFLAGS=-g -Wall -Werror -DLUA_COMPAT_APIINTCASTS

UNAME=$(shell uname)
SYS=$(if $(filter Linux%,$(UNAME)),linux,\
	    $(if $(filter MINGW%,$(UNAME)),mingw,\
	    $(if $(filter Darwin%,$(UNAME)),macosx,\
	        undefined\
)))

mingw: SHARED := -shared -fPIC
macosx: SHARED := -fPIC -dynamiclib -Wl,-undefined,dynamic_lookup

audio.so: src/laudio.c 
	gcc $(CFLAGS) $(SHARED) -o $@ $^ -I/usr/local/include #-llua

clean:
	rm -f audio.so
