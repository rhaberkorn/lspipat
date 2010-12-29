/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: STRING PRIMITIVES/CONSTRUCTORS
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "lspipat.h"

static VString
stringFncHandler(void *global __attribute__((unused)), void *local)
{
	struct retfncRefs	*retfnc = local;
	lua_State		*L = retfnc->cb.L;

	VString			ret;

	lua_rawgeti(L, LUA_REGISTRYINDEX, retfnc->cb.function);
	lua_rawgeti(L, LUA_REGISTRYINDEX, retfnc->cb.cookie);
#if 0
	lua_rawgeti(L, LUA_REGISTRYINDEX, *(int *)global);
#endif

	lua_call(L, 1, 1);

	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		L_ERROR(L_RETURN);	/* FIXME: is it safe to raise errors? */
	}

	ret.ptr = lua_tolstring(L, -1, (size_t *)&ret.len);
	ret.release = retfncUnrefRet;
	ret.cookie = retfnc;

			/*
			 * Register value so Lua doesn't free it until spipat
			 * doesn't need it anymore (value has to be popped now)
			 */
	retfnc->ret = luaL_ref(L, LUA_REGISTRYINDEX);
	return ret;
}

struct stringPrimitive {
	struct pat *(*chr)(Character);
	struct pat *(*str)(VString);
	struct pat *(*fnc)(VString (*)(void *, void*), void *);
};

static int
genericStringPrimitive(lua_State *L, struct stringPrimitive spipat)
{
	int top = lua_gettop(L);

	VString		str = VSTRING_INITIALIZER;
	PATTERN_WRAPPER	*new;

	luaL_argcheck(L, top, top, L_NUMBER);

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));

	switch (lua_type(L, 1)) {
	case LUA_TNUMBER:
	case LUA_TSTRING:
		luaL_argcheck(L, top == 1, top, L_NUMBER);

		str.ptr = lua_tolstring(L, 1, (size_t *)&str.len);

		new->pattern = str.len == 1 ? spipat.chr(*str.ptr)
					    : spipat.str(str);
		break;

	case LUA_TFUNCTION: {
		struct retfncRefs *retfnc;

		luaL_argcheck(L, top == 1 || top == 2, top, L_NUMBER);

		lua_insert(L, 1);	/* move wrapper to bottom */
		if (top == 1)
			lua_pushnil(L);	/* cookie will be LUA_REFNIL */

		new->type = PATTERN_RETFNC;

		retfnc = &new->u.retfnc;
		retfnc->cb.L = L;
		retfnc->cb.cookie = luaL_ref(L, LUA_REGISTRYINDEX);
		retfnc->cb.function = luaL_ref(L, LUA_REGISTRYINDEX);
					/* wrapper at top again */

		new->pattern = spipat.fnc(stringFncHandler, retfnc);
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

#define STDSTRPRIM(LFNC, SPIFNC)						\
	LUA_SIG(LFNC)								\
	{									\
		return genericStringPrimitive(L, (struct stringPrimitive) {	\
			.chr = SPIFNC##_chr,					\
			.str = SPIFNC##_str,					\
			.fnc = SPIFNC##_fnc					\
		});								\
	}

STDSTRPRIM(l_primitive_any,	spipat_any)
STDSTRPRIM(l_primitive_break,	spipat_break)
STDSTRPRIM(l_primitive_breakx,	spipat_breakx)
STDSTRPRIM(l_primitive_notany,	spipat_notany)
STDSTRPRIM(l_primitive_nspan,	spipat_nspan)
STDSTRPRIM(l_primitive_span,	spipat_span)

#undef STDSTRPRIM
