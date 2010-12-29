/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: SIMPLE PRIMITIVES/CONSTRUCTORS
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "lspipat.h"

struct simplePrimitive {
	struct pat *(*simple)(void);
};

static int
genericSimplePrimitive(lua_State *L, struct simplePrimitive spipat)
{
	int top = lua_gettop(L);
	PATTERN_WRAPPER *new;

	luaL_argcheck(L, !top, top, L_NUMBER);

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));

	if (!(new->pattern = spipat.simple()))
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;
}

#define STDSIMPLEPRIM(LFNC, SPIFNC)						\
	LUA_SIG(LFNC)								\
	{									\
		return genericSimplePrimitive(L, (struct simplePrimitive) {	\
			.simple = SPIFNC					\
		});								\
	}

STDSIMPLEPRIM(l_primitive_abort,	spipat_abort)
STDSIMPLEPRIM(l_primitive_arb,		spipat_arb)
STDSIMPLEPRIM(l_primitive_bal,		spipat_bal)
STDSIMPLEPRIM(l_primitive_fail,		spipat_fail)
STDSIMPLEPRIM(l_primitive_rem,		spipat_rem)
STDSIMPLEPRIM(l_primitive_succeed,	spipat_succeed)

#undef STDSIMPLEPRIM
