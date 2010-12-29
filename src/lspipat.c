/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: LIBSPIPAT <-> LUA INTERACTION
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "lspipat.h"

/*
 * Module and Pattern methods
 */

/* TODO: support global cookies */

LUA_SIG(l_smatch)
{
	int top = lua_gettop(L);

	struct spipat_match	match;
	enum spipat_match_ret	ret;

	luaL_argcheck(L, top == 2 || top == 3, top, L_NUMBER);

	memset(&match, 0, sizeof(match));
	match.subject.ptr = luaL_checklstring(L, 1, (size_t *)&match.subject.len);
	match.flags = luaL_optint(L, 3, 0);

	if (lua_isstring(L, 2)) {
		VString str = VSTRING_INITIALIZER;
		str.ptr = lua_tolstring(L, 2, (size_t *)&str.len);

		match.pattern = str.len == 1 ? spipat_char(*str.ptr)
					     : spipat_string(str);
		if (!match.pattern)
			L_ERROR(L_ALLOC);
	} else {
		PATTERN_WRAPPER *wrapper = luaL_checkudata(L, 2, PATTERN_MT);
		luaL_argcheck(L, wrapper->pattern, 2, L_FREED);

		match.pattern = wrapper->pattern;
		spipat_hold(match.pattern);
	}

	ret = spipat_match2(&match);
	spipat_free(match.pattern); /* only frees the temporary pattern for string params */
	if (ret == SPIPAT_MATCH_EXCEPTION)
		L_ERROR("%s", match.exception);

	if (ret == SPIPAT_MATCH_FAILURE) {
		lua_pushnil(L);
		return 1;
	}

	/* SPIPAT_MATCH_SUCCESS */
	lua_pushinteger(L, match.start);
	lua_pushinteger(L, match.stop);
	return 2;
}

		/* should we check __topattern operations in types metatables just like tostring does? */
LUA_SIG(l_topattern)
{
	int top = lua_gettop(L);

	luaL_argcheck(L, top == 1, top, L_NUMBER);

	switch (lua_type(L, 1)) {
	case LUA_TNUMBER:
	case LUA_TSTRING: {
		PATTERN_WRAPPER	*wrapper;
		VString		str = VSTRING_INITIALIZER;

		if (!(wrapper = lua_newuserdata(L, sizeof(PATTERN_WRAPPER))))
			L_ERROR(L_ALLOC);
		memset(wrapper, 0, sizeof(PATTERN_WRAPPER));

		str.ptr = lua_tolstring(L, 1, (size_t *)&str.len);

		wrapper->pattern = str.len == 1 ? spipat_char(*str.ptr)
						: spipat_string(str);
		if (!wrapper->pattern)
			L_ERROR(L_ALLOC);

		luaL_getmetatable(L, PATTERN_MT);
		lua_setmetatable(L, -2);

		return 1;
	}
	case LUA_TUSERDATA:
		/* FIXME: check whether it's a PATTERN_MT (without raising an error) */
		return 1;

	default:
		return 0;
	}

	/* not reached */
}

LUA_SIG(l_dump)
{
	PATTERN_WRAPPER	*wrapper;
	int		top = lua_gettop(L);

	luaL_argcheck(L, top == 1, top, L_NUMBER);
	wrapper = luaL_checkudata(L, 1, PATTERN_MT);
	luaL_argcheck(L, wrapper->pattern, 1, L_FREED);

	spipat_dump(wrapper->pattern);
	return 0;
}

/*
 * Finalizer
 */

static inline void
unrefCallback(struct cbRefs *cb)
{
	luaL_unref(cb->L, LUA_REGISTRYINDEX, cb->function);
	luaL_unref(cb->L, LUA_REGISTRYINDEX, cb->cookie);
}

LUA_SIG(l_finalize_pattern)
{
	int		top = lua_gettop(L);
	PATTERN_WRAPPER	*wrapper;

	luaL_argcheck(L, top == 1, top, L_NUMBER);
	wrapper = luaL_checkudata(L, 1, PATTERN_MT);

	if (!wrapper->pattern)
		return 0; /* already freed */

	spipat_free(wrapper->pattern);	/* should also release any strings/patterns */
	wrapper->pattern = NULL;	/* (remove from registry using release functions) returned by some callback */

	switch (wrapper->type) {
	case PATTERN_OTHER:
		break;
	case PATTERN_ONESUBPAT:
		luaL_unref(L, LUA_REGISTRYINDEX, wrapper->u.onesubpat.pattern);
		break;
	case PATTERN_TWOSUBPAT:
		luaL_unref(L, LUA_REGISTRYINDEX, wrapper->u.twosubpat.pattern1);
		luaL_unref(L, LUA_REGISTRYINDEX, wrapper->u.twosubpat.pattern2);
		break;
	case PATTERN_CALL:
		luaL_unref(L, LUA_REGISTRYINDEX, wrapper->u.call.pattern);
		unrefCallback(&wrapper->u.call.cb);
		break;
	case PATTERN_RETFNC:
		unrefCallback(&wrapper->u.retfnc.cb);
		break;
	case PATTERN_SIMPLEFNC:
		unrefCallback(&wrapper->u.simplefnc.cb);
		break;
	default:
		L_ERROR(L_MISC);
	}

	return 0;
}

/*
 * Cookie release function for function return values
 */

void
retfncUnrefRet(void *arg)
{
	struct retfncRefs *retfnc = arg;

	luaL_unref(retfnc->cb.L, LUA_REGISTRYINDEX, retfnc->ret);
}

/*
 * Loader
 */

int
luaopen_lspipat_core(lua_State *L)
{
	static const luaL_Reg spipat[] = {
		{"smatch",	l_smatch},

		{"topattern",	l_topattern},
		{"dump",	l_dump},

		{"free",	l_finalize_pattern},
		{NULL, NULL}
	};

	static const luaL_Reg primitives[] = {
		/* string primitives */
		{"Any",		l_primitive_any},
		{"Break",	l_primitive_break},
		{"BreakX",	l_primitive_breakx},
		{"NotAny",	l_primitive_notany},
		{"NSpan",	l_primitive_nspan},
		{"Span",	l_primitive_span},

		/* unsigned integer primitives */
		{"Len",		l_primitive_len},
		{"Pos",		l_primitive_pos},
		{"RPos",	l_primitive_rpos},
		{"RTab",	l_primitive_rtab},
		{"Tab",		l_primitive_tab},

		/* simple primitives */
		{"Abort",	l_primitive_abort},
		{"Arb",		l_primitive_arb},
		{"Bal",		l_primitive_bal},
		{"Fail",	l_primitive_fail},
		{"Rem",		l_primitive_rem},
		{"Succeed",	l_primitive_succeed},

		/* misc. primitives */
		{"Arbno",	l_primitive_arbno},
		{"Fence",	l_primitive_fence},

		/* primitives for unary operators */
		{"Setcur",	l_setcur},
		{"Pred",	l_pred},
		{NULL, NULL}
	};

	static const luaL_Reg methods[] = {
		{"free",	l_finalize_pattern},
		{NULL, NULL}
	};

	static const luaL_Reg operations[] = {
		{"__mul",	l_op_and},
		{"__add",	l_op_or},
		{"__mod",	l_op_call_immed},
		{"__div",	l_op_call_onmatch},

		{"__tostring",	l_tostring},

		{"__gc",	l_finalize_pattern},
		{NULL, NULL}
	};

	static const LUA_CONSTANT mapping[] = {
		{"match_debug",		SPIPAT_DEBUG},
		{"match_anchored",	SPIPAT_ANCHORED},
		{NULL, 0}
	};

			/* module methods, primitives & constants */

	luaL_register(L, "spipat", spipat);
	luaL_register(L, NULL, primitives);

	for (const LUA_CONSTANT *m = mapping; m->lua; m++) {
		lua_pushinteger(L, m->c);
		lua_setfield(L, -2, m->lua);
	}
			/* module table should be at stack index 2 */

			/* global methods & primitives */
			/* FIXME: make it optional (function or submodule) */

	for (const luaL_Reg *p = primitives; p->name; p++) {
		lua_pushcfunction(L, p->func);
		lua_setglobal(L, p->name);
	}
	lua_pushcfunction(L, l_topattern);
	lua_setglobal(L, "topattern");

			/* "patch" string meta table with some methods */

	lua_pushstring(L, "foo"); /* FIXME: use luaL_getmetatable */
	lua_getmetatable(L, -1);
	lua_getfield(L, 2, "_Pred"); /* ok, this is hairy: will only be available if string cannot be converted to a number */
	lua_setfield(L, -2, "__unm");
	lua_getfield(L, -1, "__index");
	lua_pushcfunction(L, l_smatch);	/* maybe split "spipat" and use luaL_register */
	lua_setfield(L, -2, "smatch");
	lua_getfield(L, 2, "ssub"); /* maybe write aux function to register Lua functions */
	lua_setfield(L, -2, "ssub");
	lua_getfield(L, 2, "siter");
	lua_setfield(L, -2, "siter");
	lua_pushcfunction(L, l_topattern);
	lua_setfield(L, -2, "topattern");
	lua_pop(L, 3);
	/* TODO: maybe also set the pattern-specific operations - adapt l_op_or/and to cope with two strings
	   however, arithmetic ops are already defined for strings if they can be converted to numbers */

			/* "patch" number meta table with some methods */

	lua_pushinteger(L, 23); /* FIXME: use luaL_getmetatable */
	if (!lua_getmetatable(L, -1)) {
		lua_newtable(L);
		lua_newtable(L);
	} else
		lua_getfield(L, -1, "__index");
	lua_pushcfunction(L, l_topattern);
	lua_setfield(L, -2, "topattern");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

			/* "patch" function meta table with operators */

	lua_pushcfunction(L, l_smatch); /* FIXME: use luaL_getmetatable */
	if (!lua_getmetatable(L, -1))
		lua_newtable(L);
	lua_pushcfunction(L, l_setcur);
	lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, l_pred);
	lua_setfield(L, -2, "__unm");
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

			/* pattern metatable: methods & operations/events */

	luaL_newmetatable(L, PATTERN_MT);
	luaL_register(L, NULL, operations);
	lua_newtable(L);
	luaL_register(L, NULL, methods);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	/* module table should be on top of the stack again */
	return 1;
}