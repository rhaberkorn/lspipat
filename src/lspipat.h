/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 */

#ifndef _LSPIPAT_H
#define _LSPIPAT_H

#ifdef HAVE_LUA5_1_LUA_H
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#else
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <spipat.h>

#if defined(HAVE_SPIPAT_IMPL_H) && defined(HAVE_SPIPAT_IMAGE_H)
#define USE_SPIPAT_IMAGE_CUSTOM

#include <spipat_impl.h>
#include <spipat_image.h>

#endif

#define VSTRING_INITIALIZER {NULL, 0, NULL, NULL}

	/* Lua error raising */

#define L_ALLOC		"Allocation error"
#define L_MISC		"Miscellaneous error"
#define L_TYPE		"Invalid type"
#define L_NUMBER	"Invalid number of parameters"
#define L_VALUE		"Invalid value for this parameter"
#define L_FREED		"Pattern already freed"
#define L_RETURN	"Invalid return value"

#define L_ERROR(MSG, ...) do {			\
	luaL_error(L, MSG "\n", ##__VA_ARGS__);	\
} while (0)	/* return omitted, so it works for all functions */

	/* metatables */

#define PATTERN_MT "SPIPAT.PATTERN_MT"

	/* structures */

typedef struct {
	const char	*lua;
	int		c;
} LUA_CONSTANT;

struct cbRefs {				/* wraps references necessary for callbacks */
	lua_State	*L;

	int		function;
	int		cookie;		/* local cookie */
};

typedef struct {
	struct pat *pattern;

	enum {				/* Lua reference classes of patterns */
		PATTERN_OTHER = 0,
		PATTERN_ONESUBPAT,
		PATTERN_TWOSUBPAT,
		PATTERN_CALL,
		PATTERN_RETFNC,
		PATTERN_SIMPLEFNC
	} type;

	union {				/* references to control garbage collection */
		struct onesubpatRefs {
			int pattern;
		} onesubpat;

		struct twosubpatRefs {
			int pattern1;
			int pattern2;
		} twosubpat;

		struct callRefs {
			int pattern;
			struct cbRefs cb;
		} call;

		struct retfncRefs {
			struct cbRefs cb;
			int ret;
		} retfnc;

		struct simplefncRefs {
			struct cbRefs cb;
		} simplefnc;
	} u;
} PATTERN_WRAPPER;

	/* Lua functions */

#define LUA_SIG(FNC) \
	int FNC(lua_State *L)

LUA_SIG(l_smatch);
LUA_SIG(l_topattern);
LUA_SIG(l_dump);

LUA_SIG(l_tostring);

LUA_SIG(l_op_and);
LUA_SIG(l_op_or);

LUA_SIG(l_op_call_immed);
LUA_SIG(l_op_call_onmatch);

LUA_SIG(l_setcur);
LUA_SIG(l_pred);

LUA_SIG(l_primitive_any);
LUA_SIG(l_primitive_break);
LUA_SIG(l_primitive_breakx);
LUA_SIG(l_primitive_notany);
LUA_SIG(l_primitive_nspan);
LUA_SIG(l_primitive_span);

LUA_SIG(l_primitive_len);
LUA_SIG(l_primitive_pos);
LUA_SIG(l_primitive_rpos);
LUA_SIG(l_primitive_rtab);
LUA_SIG(l_primitive_tab);

LUA_SIG(l_primitive_abort);
LUA_SIG(l_primitive_arb);
LUA_SIG(l_primitive_bal);
LUA_SIG(l_primitive_fail);
LUA_SIG(l_primitive_rem);
LUA_SIG(l_primitive_succeed);

LUA_SIG(l_primitive_arbno);
LUA_SIG(l_primitive_fence);

void retfncUnrefRet(void *);

#endif