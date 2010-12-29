/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: COMPOSITION OPERATIONS
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "lspipat.h"

struct composeOperator {
	struct pat *(*str_pat)(VString, struct pat *);
	struct pat *(*pat_str)(struct pat *, VString);
	struct pat *(*chr_pat)(Character, struct pat *);
	struct pat *(*pat_chr)(struct pat *, Character);
	struct pat *(*pat_pat)(struct pat *, struct pat *);
};

 	/* at least one parameter must be a pattern, both are only allowed to be numbers, strings or patterns */

static int
genericComposeOperator(lua_State *L, struct composeOperator spipat)
{
	VString		str = VSTRING_INITIALIZER;
	PATTERN_WRAPPER *new;

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));
	lua_insert(L, 1);

	if (lua_isstring(L, 2)) {		/* lvalue number/string, rvalue is pattern */
		PATTERN_WRAPPER *rvalue = lua_touserdata(L, 3);

		if (!rvalue->pattern)
			L_ERROR(L_FREED);
		str.ptr = lua_tolstring(L, 2, (size_t *)&str.len);

		new->type = PATTERN_ONESUBPAT;
		new->u.onesubpat.pattern = luaL_ref(L, LUA_REGISTRYINDEX);

		new->pattern = str.len == 1 ? spipat.chr_pat(*str.ptr, rvalue->pattern)
					    : spipat.str_pat(str, rvalue->pattern);

		lua_pop(L, 1);			/* `new' at stack top */
	} else {				/* lvalue must be pattern */
		PATTERN_WRAPPER *lvalue = luaL_checkudata(L, 2, PATTERN_MT);

		if (!lvalue->pattern)
			L_ERROR(L_FREED);

		if (lua_isstring(L, 3)) {	/* rvalue number/string */
			str.ptr = lua_tolstring(L, 3, (size_t *)&str.len);

			new->pattern = str.len == 1 ? spipat.pat_chr(lvalue->pattern, *str.ptr)
						    : spipat.pat_str(lvalue->pattern, str);

			lua_pop(L, 1);

			new->type = PATTERN_ONESUBPAT;
			new->u.onesubpat.pattern = luaL_ref(L, LUA_REGISTRYINDEX);
		} else {			/* rvalue must be pattern */
			PATTERN_WRAPPER *rvalue = luaL_checkudata(L, 3, PATTERN_MT);

			if (!rvalue->pattern)
				L_ERROR(L_FREED);

			new->type = PATTERN_TWOSUBPAT;
			new->u.twosubpat.pattern2 = luaL_ref(L, LUA_REGISTRYINDEX);
			new->u.twosubpat.pattern1 = luaL_ref(L, LUA_REGISTRYINDEX);

			new->pattern = spipat.pat_pat(lvalue->pattern, rvalue->pattern);
		}
	}

	if (!new->pattern)
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;
}

#define STDCOMPOSEOP(LFNC, SPIFNC)						\
	LUA_SIG(LFNC)								\
	{									\
		return genericComposeOperator(L, (struct composeOperator) {	\
			.str_pat = SPIFNC##_str_pat,				\
			.pat_str = SPIFNC##_pat_str,				\
			.chr_pat = SPIFNC##_chr_pat,				\
			.pat_chr = SPIFNC##_pat_chr,				\
			.pat_pat = SPIFNC##_pat_pat				\
		});								\
	}

STDCOMPOSEOP(l_op_and,	spipat_and)
STDCOMPOSEOP(l_op_or,	spipat_or)

#undef STDCOMPOSEOP
