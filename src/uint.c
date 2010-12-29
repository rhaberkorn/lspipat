/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: UNSIGNED INTEGER PRIMITIVES/CONSTRUCTORS
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "lspipat.h"

static unsigned
uintFncHandler(void *global __attribute__((unused)), void *local)
{
	struct simplefncRefs	*simplefnc = local;
	lua_State		*L = simplefnc->cb.L;

	int			val;

	lua_rawgeti(L, LUA_REGISTRYINDEX, simplefnc->cb.function);
	lua_rawgeti(L, LUA_REGISTRYINDEX, simplefnc->cb.cookie);
#if 0
	lua_rawgeti(L, LUA_REGISTRYINDEX, *(int *)global);
#endif

	lua_call(L, 1, 1);

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return 0; /* default value */
	}

	if (!lua_isnumber(L, -1)) {
		lua_pop(L, 1);
		L_ERROR(L_RETURN);	/* FIXME: is it safe to raise errors? */
	}

	val = lua_tointeger(L, -1);
	lua_pop(L, 1);
	if (val < 0)
		L_ERROR(L_RETURN);

	return (unsigned)val;
}

struct uintPrimitive {
	struct pat *(*uint)(unsigned);
	struct pat *(*fnc)(unsigned (*)(void *, void *), void *);
};

static int
genericUIntPrimitive(lua_State *L, struct uintPrimitive spipat)
{
	int top = lua_gettop(L);
	PATTERN_WRAPPER *new;

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));

	switch (lua_type(L, 1)) {
	case LUA_TNONE:
	case LUA_TNIL:
	case LUA_TNUMBER:
	case LUA_TSTRING: {
		int val;

		luaL_argcheck(L, top < 2, top, L_NUMBER);
		val = luaL_optint(L, 1, 0);
		luaL_argcheck(L, val >= 0, 1, L_VALUE);

		new->pattern = spipat.uint((unsigned)val);
		break;
	}
	case LUA_TFUNCTION: {
		struct simplefncRefs *simplefnc;

		luaL_argcheck(L, top == 1 || top == 2, top, L_NUMBER);

		lua_insert(L, 1);	/* move wrapper to bottom */
		if (top == 1)
			lua_pushnil(L);	/* cookie will be LUA_REFNIL */

		new->type = PATTERN_SIMPLEFNC;

		simplefnc = &new->u.simplefnc;
		simplefnc->cb.L = L;
		simplefnc->cb.cookie = luaL_ref(L, LUA_REGISTRYINDEX);
		simplefnc->cb.function = luaL_ref(L, LUA_REGISTRYINDEX);
					/* wrapper at top again */

		new->pattern = spipat.fnc(uintFncHandler, simplefnc);
		break;
	}
	default:
		return luaL_argerror(L, 1, L_TYPE);
	}

	if (!new->pattern)
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;
}

#define STDUINTPRIM(LFNC, SPIFNC)						\
	LUA_SIG(LFNC)								\
	{									\
		return genericUIntPrimitive(L, (struct uintPrimitive) {		\
			.uint	= SPIFNC,					\
			.fnc	= SPIFNC##_fnc					\
		});								\
	}

STDUINTPRIM(l_primitive_len,	spipat_len)
STDUINTPRIM(l_primitive_pos,	spipat_pos)
STDUINTPRIM(l_primitive_rpos,	spipat_rpos)
STDUINTPRIM(l_primitive_rtab,	spipat_rtab)
STDUINTPRIM(l_primitive_tab,	spipat_tab)

#undef STDUINTPRIM
