.PHONY: audio.so 

CFLAGS=-g -Wall -Werror -DLUA_COMPAT_APIINTCASTS

UNAME=$(shell uname)
SYS=$(if $(filter Linux%,$(UNAME)),linux,\
	    $(if $(filter MINGW%,$(UNAME)),mingw,\
	    $(if $(filter Darwin%,$(UNAME)),macosx,\
	        undefined\
)))

all: $(SYS)

undefined:
	@echo "I can't guess your platform, please do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "      linux mingw macosx"

mingw: SHARED := -shared -fPIC
mingw: audio.so

macosx: SHARED := -fPIC -dynamiclib -Wl,-undefined,dynamic_lookup
macosx: audio.so

audio.so: src/laudio.c src/audio_decoder.c
	gcc $(CFLAGS) $(SHARED) -o $@ $^ -Isrc -I/usr/local/include -Idepinc -lalut -lmpg123 #-llua

clean:
	rm -rf audio.so audio.so.*
