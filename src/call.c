/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: CALL OPERATIONS
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "lspipat.h"

static void
callFncHandler(VString matched, void *global __attribute__((unused)), void *local)
{
	struct callRefs	*call = local;
	lua_State	*L = call->cb.L;

	lua_rawgeti(L, LUA_REGISTRYINDEX, call->cb.function);
	lua_pushlstring(L, matched.ptr, matched.len);
	lua_rawgeti(L, LUA_REGISTRYINDEX, call->cb.cookie);
#if 0
	lua_rawgeti(L, LUA_REGISTRYINDEX, *(int *)global);
#endif

	lua_call(L, 2, 0);
}

struct callOperator {
	struct pat *(*call)(struct pat *, void (*)(VString, void *, void *), void *);
};

		/* TODO: local cookie support, this would also allow helper functions for assignment to global variables */
		/* at least one parameter is a pattern, the lvalue has to be it */
static int
genericCallOperator(lua_State *L, struct callOperator spipat)
{
	PATTERN_WRAPPER	*new;
	struct callRefs	*call;

	PATTERN_WRAPPER *lvalue = luaL_checkudata(L, 1, PATTERN_MT);
	if (!lvalue->pattern)
		L_ERROR(L_FREED);
	if (!lua_isfunction(L, 2))
		L_ERROR(L_TYPE);

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));
	lua_insert(L, 1);	/* move wrapper below lvalue */

	new->type = PATTERN_CALL;

	call = &new->u.call;
	call->cb.L = L;
	call->cb.cookie = LUA_REFNIL;
	call->cb.function = luaL_ref(L, LUA_REGISTRYINDEX);
	call->pattern = luaL_ref(L, LUA_REGISTRYINDEX);
				/* wrapper at top again */

	new->pattern = spipat.call(lvalue->pattern, callFncHandler, call);
	if (!new->pattern)
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;
}

#define STDCALLOP(LFNC, SPIFNC)							\
	LUA_SIG(LFNC)								\
	{									\
		return genericCallOperator(L, (struct callOperator) {		\
			.call = SPIFNC						\
		});								\
	}

STDCALLOP(l_op_call_immed,	spipat_call_immed)
STDCALLOP(l_op_call_onmatch,	spipat_call_onmatch)

#undef STDCALLOP
