#include "audio_resource.h"
#include "audio_decoder.h"
#include "openal/al.h"
#include "openal/alc.h"
#include "alut/alut.h"

#include <lua.h>
#include <lauxlib.h>

#define METAAUDIO  "AUDIO*"
#define METAAUDIOD "AUDIOD*"

struct audio_data {
    ALuint source;
    ALuint buffer;
    int loop;
    float pitch;
    float pan;
    float gain;
};

static int 
checkalerr(const char *tag) {
    int err = alGetError();
    if (err != AL_NO_ERROR) {
        switch (err) {
            case AL_INVALID_NAME:
                fprintf(stderr, "AL_INVALID_NAME in %s\n", tag);
                break;
            case AL_INVALID_ENUM:
                fprintf(stderr, "AL_INVALID_ENUM in %s\n", tag);
                break;
            case AL_INVALID_VALUE:
                fprintf(stderr, "AL_INVALID_VALUE in %s\n", tag);
                break;
            case AL_INVALID_OPERATION:
                fprintf(stderr, "AL_INVALID_OPERATION in %s\n", tag);
                break;
            case AL_OUT_OF_MEMORY:
                fprintf(stderr, "AL_OUT_OF_MEMORY in %s\n", tag);
                break;
        }
    }
    return err;
}

static int
linit(lua_State *L) {
    alutInit(0, 0);
    const struct audio_decoder_ops *ops;
    for (ops = *g_audio_decoders; ops != NULL; ops++) {
        ops->init();
    }
    return 0;
}

static int
lfini(lua_State *L) {
    const struct audio_decoder_ops *ops;
    for (ops = *g_audio_decoders; ops != NULL; ops++) {
        ops->fini();
    }
    alutExit();
    return 0;
}

static inline struct audio_data *toaudiodata(lua_State *L) {
    struct audio_data *a = (struct audio_data *)lua_touserdata(L,1);
    if (a == NULL) {
        luaL_error(L, "Need audio_data");
    }
    return a;
}

static int
lload(lua_State *L) {
    const char *name = luaL_checkstring(L,1);

    struct audio_resource res;
    res.type = AUDIO_RESOURCE_TFILE;
    res.fp = fopen(name, "rb");
    if (!res.fp) {
        lua_pushnil(L);
        lua_pushfstring(L, "Can not read audio file: %s", name);
        return 2;
    }

    ALuint buffer = AL_NONE;
    ALuint source = AL_NONE;

    int succeed = 0;
    const struct audio_decoder_ops *ops;
    for (ops = *g_audio_decoders; ops != NULL; ops++) {
        if (ops->decode(&res, &buffer) == 0) {
            succeed = 1;
            break;
        }
    }

    if (!succeed) {
        lua_pushnil(L);
        lua_pushfstring(L, "Can not decode audio: %s", name);
        return 2;
    }

    alGenSources(1, &source);

    if (checkalerr("alGenSources") != AL_NO_ERROR) {
        alDeleteBuffers(1, &buffer);
        lua_pushnil(L);
        lua_pushliteral(L, "alGenSources error");
        return 2;
    }

    alSourcei(source, AL_BUFFER, buffer);
   
    struct audio_data *a = lua_newuserdata(L, sizeof(struct audio_data));
    a->source = source;
    a->buffer = buffer;
    a->loop = 0;
    a->pitch = 1.0f;
    a->pan = 0.0f;
    a->gain = 1.0f;
    luaL_setmetatable(L,METAAUDIOD);
    return 1;
}

static int
lunload(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    alSourceStop(a->source);
    checkalerr("stop");
    
    alDeleteSources(1, &a->source);
    checkalerr("delete source");

    alDeleteBuffers(1, &a->buffer);
    checkalerr("delete buffer");
    return 0;
}

static int
lplay(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    float volume;
    if (lua_gettop(L) == 2)
        volume = luaL_checknumber(L,2);
    else
        volume = 1.0f;
    alSourcei(a->source, AL_LOOPING, a->loop ? AL_TRUE : AL_FALSE);
    alSourcef(a->source, AL_GAIN, volume * a->gain);
    alSourcef(a->source, AL_PITCH, a->pitch);
    float pos[] = {a->pan, 0.0f, 0.0f};//Set position - just using left and right panning
    alSourcefv(a->source, AL_POSITION, pos);
    alSourcePlay(a->source);
    checkalerr("play");
    return 0;
}

static int
lstop(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    alSourceStop(a->source);
    checkalerr("stop");
    return 0;
}

static int
lpause(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    ALint state;
    alGetSourcei(a->source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING)
        alSourcePause(a->source);
    checkalerr("pause");
    return 0;
}

static int
lresume(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    ALint state;
    alGetSourcei(a->source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED)
        alSourcePlay(a->source);
    checkalerr("resume");
    return 0;
}

static int
lrewind(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    ALint state;
    alGetSourcei(a->source, AL_SOURCE_STATE, &state);
    alSourceRewind(a->source);
    if (state == AL_PLAYING) {
        alSourcePlay(a->source);
    } else if (state == AL_PAUSED) {
        alSourcePlay(a->source);
        alSourcePause(a->source);
    }
    checkalerr("rewind");
    return 0;
}

static int
lgetloop(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    lua_pushboolean(L, a->loop);
    return 1;
}

static int
lgetpan(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    lua_pushnumber(L, a->pan);
    return 1;
}

static int
lgetpitch(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    lua_pushnumber(L, a->pitch);
    return 1;
}

static int
lgetgain(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    lua_pushnumber(L, a->gain);
    return 1;
}

static int
lsetloop(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    a->loop = lua_toboolean(L,1);
    return 0;
}

static int
lsetpan(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    a->pan = luaL_checknumber(L, 1);
    return 0;
}

static int
lsetpitch(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    a->pitch = luaL_checknumber(L, 1);
    return 0;
}

static int
lsetgain(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    a->gain = luaL_checknumber(L, 1);
    return 0;
}

static void
createmeta_audiodata(lua_State *L) {
    luaL_Reg lmethod[] = {
        {"unload",  lunload},
        {"play",    lplay},
        {"stop",    lstop},
        {"pause",   lpause},
        {"resume",  lresume},
        {"rewind",  lrewind},

        {"loop",    lgetloop},
        {"pan",     lgetpan},
        {"gain",    lgetgain},
        {"pitch",   lgetpitch},
        {NULL, NULL},
    };

    luaL_Reg lsetter[] = {
        {"loop",    lsetloop},
        {"pan",     lsetpan},
        {"gain",    lsetgain},
        {"pitch",   lsetpitch},
        {NULL, NULL},
    };

    luaL_newmetatable(L, METAAUDIOD);

    luaL_newlib(L, lmethod);
    lua_setfield(L, -2, "__index");

    luaL_newlib(L, lsetter);
    lua_setfield(L, -2, "__newindex");
   
    lua_pop(L, 1);
}

int 
luaopen_audio_c(lua_State *L) {
    luaL_Reg l[] = {
        {"init",    linit},
        {"load",    lload},
        {NULL, NULL},
    };
	luaL_newlib(L, l);
    luaL_newmetatable(L, METAAUDIO);
    lua_pushcfunction(L, lfini);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -1);

    createmeta_audiodata(L);
    return 1;
}