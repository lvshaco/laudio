#include <lua.h>
#include <lauxlib.h>

static int
lload(lua_State *L) {
    return 0;
}

static int
lunload(lua_State *L) {
    return 0;
}

static int
lplay(lua_State *L) {
    return 0;
}

static int
lstop(lua_State *L) {
    return 0;
}

static int
lpause(lua_State *L) {
    return 0;
}

static int
lresume(lua_State *L) {
    return 0;
}

static int
lrewind(lua_State *L) {
    return 0;
}

int 
luaopen_sound_c(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
        {"load",    lload},
        {"unload",  lunload},
        {"play",    lplay},
        {"stop",    lstop},
        {"pause",   lpause},
        {"resume",  lresume},
        {"rewind",  lrewind},
        {NULL, NULL},
    };
	luaL_newlib(L, l);
    return 1;
}
