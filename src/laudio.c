#include "audio_resource.h"
#include "audio_decoder.h"
#include "audio_error.h"
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alut.h"
#include <assert.h>
#include <lua.h>
#include <lauxlib.h>

extern const char *asset_path_get();

#define METAAUDIO  "AUDIO*"

struct audio_data {
    ALuint source;
    ALuint buffer;
    int loop;
    float pitch;
    float pan;
    float gain;
};

static int
linit(lua_State *L) {
    if (alutInit(0, 0) == AL_FALSE) {
        lua_pushnil(L);
        lua_pushstring(L,alutGetErrorString(alutGetError())); 
        return 2;
    }

    const struct audio_decoder_ops *ops;
    for (ops = g_audio_decoders; ops->init != NULL; ops++) {
        ops->init();
    }
    lua_pushboolean(L,1);
    return 1;
}

static int
lfini(lua_State *L) {
    const struct audio_decoder_ops *ops;
    for (ops = g_audio_decoders; ops->fini != NULL; ops++) {
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
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", asset_path_get(), name);
    struct audio_resource res;
    res.type = AUDIO_RESOURCE_TFILE;
    res.name = path;

    //res.fp = fopen(name, "rb");
    //if (!res.fp) {
        //lua_pushnil(L);
        //lua_pushfstring(L, "Can not read audio file: %s", name);
        //return 2;
    //}

    ALuint buffer = AL_NONE;
    ALuint source = AL_NONE;

    int succeed = 0;
    const struct audio_decoder_ops *ops;
    for (ops = g_audio_decoders; ops->decode != NULL; ops++) {
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
    int err;
    __alcheckerrorret(err);
    if (err != AL_NO_ERROR) {
        alDeleteBuffers(1, &buffer);
        lua_pushnil(L);
        lua_pushliteral(L, "alGenSources error");
        return 2;
    }

    alSourcei(source, AL_BUFFER, buffer);
    __alcheckerror();

    struct audio_data *a = lua_newuserdata(L, sizeof(struct audio_data));
    a->source = source;
    a->buffer = buffer;
    a->loop = 0;
    a->pitch = 1.0f;
    a->pan = 0.0f;
    a->gain = 1.0f;
    return 1;
}

static int
lunload(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    alSourceStop(a->source);
    __alcheckerror();
    
    alDeleteSources(1, &a->source);
    __alcheckerror();

    alDeleteBuffers(1, &a->buffer);
    __alcheckerror();
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
    __alcheckerror();
    return 0;
}

static int
lstop(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    alSourceStop(a->source);
    __alcheckerror();
    return 0;
}

static int
lpause(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    ALint state;
    alGetSourcei(a->source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING)
        alSourcePause(a->source);
    __alcheckerror();
    return 0;
}

static int
lresume(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    ALint state;
    alGetSourcei(a->source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED)
        alSourcePlay(a->source);
    __alcheckerror();
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
    __alcheckerror();
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
    a->loop = lua_toboolean(L, 2);
    return 0;
}

static int
lsetpan(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    a->pan = luaL_checknumber(L, 2);
    return 0;
}

static int
lsetpitch(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    a->pitch = luaL_checknumber(L, 2);
    return 0;
}

static int
lsetgain(lua_State *L) {
    struct audio_data *a = toaudiodata(L);
    a->gain = luaL_checknumber(L, 2);
    return 0;
}

static void
lsetter(lua_State *L) {
    luaL_Reg l[] = {
        {"loop",    lsetloop},
        {"pan",     lsetpan},
        {"gain",    lsetgain},
        {"pitch",   lsetpitch},
        {NULL, NULL},
    };
    luaL_newlib(L,l);
}

static void
lgetter(lua_State *L) {
    luaL_Reg l[] = {
        {"loop",    lgetloop},
        {"pan",     lgetpan},
        {"gain",    lgetgain},
        {"pitch",   lgetpitch},
        {NULL, NULL},
    };
    luaL_newlib(L,l);
}

static void
lmethod(lua_State *L) {
    luaL_Reg l[] = {
        {"unload",  lunload},
        {"play",    lplay},
        {"stop",    lstop},
        {"pause",   lpause},
        {"resume",  lresume},
        {"rewind",  lrewind},
        {NULL, NULL},
    };
    luaL_newlib(L,l);
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

    lmethod(L);
    lua_setfield(L, -2, "method");
    lgetter(L);
    lua_setfield(L, -2, "get");
    lsetter(L);
    lua_setfield(L, -2, "set");
    return 1;
}
