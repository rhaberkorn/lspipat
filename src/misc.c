/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: MISCELLANEOUS PRIMITIVES/CONSTRUCTORS
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "lspipat.h"

LUA_SIG(l_primitive_arbno)
{
	int top = lua_gettop(L);

	VString		str = VSTRING_INITIALIZER;
	PATTERN_WRAPPER	*new;

	luaL_argcheck(L, top == 1, top, L_NUMBER);

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));

	if (lua_isstring(L, 1)) {
		str.ptr = lua_tolstring(L, 1, (size_t *)&str.len);

		new->pattern = str.len == 1 ? spipat_arbno_chr(*str.ptr)
					    : spipat_arbno_str(str);
	} else {
		PATTERN_WRAPPER *wrapper = luaL_checkudata(L, 1, PATTERN_MT);
		luaL_argcheck(L, wrapper->pattern, 1, L_FREED);

		lua_insert(L, 1);		/* move wrapper to bottom */
		new->type = PATTERN_ONESUBPAT;
		new->u.onesubpat.pattern = luaL_ref(L, LUA_REGISTRYINDEX);
						/* wrapper at top again */

		new->pattern = spipat_arbno(wrapper->pattern);
	}

	if (!new->pattern)
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;

}

LUA_SIG(l_primitive_fence)
{
	int top = lua_gettop(L);
	PATTERN_WRAPPER *new;

	luaL_argcheck(L, top < 2, top, L_NUMBER);

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));

	if (!top) {
		new->pattern = spipat_fence_simple();
	} else {
		PATTERN_WRAPPER *wrapper = luaL_checkudata(L, 1, PATTERN_MT);
		luaL_argcheck(L, wrapper->pattern, 1, L_FREED);

		lua_insert(L, 1);		/* move wrapper to bottom */
		new->type = PATTERN_ONESUBPAT;
		new->u.onesubpat.pattern = luaL_ref(L, LUA_REGISTRYINDEX);
						/* wrapper at top again */

		new->pattern = spipat_fence_function(wrapper->pattern);
	}

	if (!new->pattern)
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;
}
