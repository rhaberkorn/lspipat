/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: UNARY OPERATORS (ALSO USED AS PRIMITIVES)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>
#include <string.h>

#include "lspipat.h"

static void
setcurFncHandler(unsigned pos, void *global __attribute__((unused)), void *local)
{
	struct simplefncRefs	*simplefnc = local;
	lua_State		*L = simplefnc->cb.L;

	lua_rawgeti(L, LUA_REGISTRYINDEX, simplefnc->cb.function);
	lua_pushinteger(L, pos);
	lua_rawgeti(L, LUA_REGISTRYINDEX, simplefnc->cb.cookie);
#if 0
	lua_rawgeti(L, LUA_REGISTRYINDEX, *(int *)global);
#endif

	lua_call(L, 2, 0);
}

		/*
		 * if called as an operator, there will be a nil on top of the stack
		 */
LUA_SIG(l_setcur)
{
	int top = lua_gettop(L);

	PATTERN_WRAPPER		*new;
	struct simplefncRefs	*simplefnc;

	luaL_argcheck(L, top == 1 || top == 2, top, L_NUMBER);
	luaL_argcheck(L, lua_isfunction(L, 1), 1, L_TYPE);

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));

	lua_insert(L, 1);	/* move wrapper to bottom */
	if (top == 1)
		lua_pushnil(L);	/* cookie will be LUA_REFNIL */

	new->type = PATTERN_SIMPLEFNC;

	simplefnc = &new->u.simplefnc;
	simplefnc->cb.L = L;
	simplefnc->cb.cookie = luaL_ref(L, LUA_REGISTRYINDEX);
	simplefnc->cb.function = luaL_ref(L, LUA_REGISTRYINDEX);
				/* wrapper at top again */

	new->pattern = spipat_setcur_fnc(setcurFncHandler, simplefnc);
	if (!new->pattern)
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;
}

static void
predFncHandler(void *global __attribute__((unused)), void *local, struct dynamic *ret)
{
	struct retfncRefs	*retfnc = local;
	lua_State		*L = retfnc->cb.L;

	lua_rawgeti(L, LUA_REGISTRYINDEX, retfnc->cb.function);
	lua_rawgeti(L, LUA_REGISTRYINDEX, retfnc->cb.cookie);
#if 0
	lua_rawgeti(L, LUA_REGISTRYINDEX, *(int *)global);
#endif

	lua_call(L, 1, 1);

	switch (lua_type(L, -1)) {
	case LUA_TNUMBER:
	case LUA_TSTRING: {
		VString *str = &ret->val.str;

		ret->type = DY_VSTR;

		str->ptr = lua_tolstring(L, -1, (size_t *)&str->len);
		str->release = retfncUnrefRet;
		str->cookie = retfnc;

				/*
				 * Register value so Lua doesn't free it until spipat
				 * doesn't need it anymore (value has to be popped now)
				 */
		retfnc->ret = luaL_ref(L, LUA_REGISTRYINDEX);
		return;
	}
	case LUA_TNIL: /* default behaviour: continue matching (Succeed) */
		ret->type = DY_BOOL;

		ret->val.pred = true;

		lua_pop(L, 1);
		return;

	case LUA_TBOOLEAN:
		ret->type = DY_BOOL;

		ret->val.pred = lua_toboolean(L, -1);

		lua_pop(L, 1);
		return;

	case LUA_TUSERDATA: { /* FIXME: check whether it's really a Pattern */
		PATTERN_WRAPPER *wrapper = lua_touserdata(L, -1);
		if (!wrapper->pattern) {
			lua_pop(L, 1);
			L_ERROR(L_RETURN);
		}

		ret->type = DY_PAT;

		ret->val.pat.p = wrapper->pattern;
		ret->val.pat.release = retfncUnrefRet;
		ret->val.pat.cookie = retfnc;

				/*
				 * Register value so Lua doesn't free it until spipat
				 * doesn't need it anymore (value has to be popped now)
				 */
		retfnc->ret = luaL_ref(L, LUA_REGISTRYINDEX);
		return;
	}
	default:
		lua_pop(L, 1);
		L_ERROR(L_RETURN);
	}

	/* not reached */
}

LUA_SIG(l_pred)
{
	int top = lua_gettop(L);

	PATTERN_WRAPPER		*new;
	struct retfncRefs	*retfnc;

	luaL_argcheck(L, top == 1 || top == 2, top, L_NUMBER);
	luaL_argcheck(L, lua_isfunction(L, 1), 1, L_TYPE);

	if (!(new = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
		L_ERROR(L_ALLOC);
	memset(new, 0, sizeof(PATTERN_WRAPPER));

	lua_insert(L, 1);	/* move wrapper to bottom */
	if (top == 1)
		lua_pushnil(L);	/* cookie will be LUA_REFNIL */

	new->type = PATTERN_RETFNC;
	retfnc = &new->u.retfnc;
	retfnc->cb.L = L;
	retfnc->cb.cookie = luaL_ref(L, LUA_REGISTRYINDEX);
	retfnc->cb.function = luaL_ref(L, LUA_REGISTRYINDEX);
				/* wrapper at top again */

	new->pattern = spipat_dynamic_fnc(predFncHandler, retfnc);
	if (!new->pattern)
		L_ERROR(L_ALLOC);

	luaL_getmetatable(L, PATTERN_MT);
	lua_setmetatable(L, -2);

	return 1;
}
