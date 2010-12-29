/*
 * LSPIPAT - LUA SPIPAT WRAPPER
 * Copyright (C) 2010, Robin Haberkorn
 * License: LGPL
 *
 * CORE: RENDER-TO-STRING OPERATION
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "lspipat.h"

#ifdef USE_SPIPAT_IMAGE_CUSTOM

static const char *lspipat_strs[] = {		/* left out elements that can't be constructed with lspipat */
	[PC_Abort]		= "Abort",
	[PC_Alt]		= " + ",
	[PC_Any_CH]		= "Any",
	[PC_Any_CS]		= "Any",
	[PC_Any_VF]		= "Any",
	[PC_Arb_X]		= "Arb",
	[PC_Arbno_S]		= "Arbno",
	[PC_Arbno_X]		= "Arbno",
	[PC_Bal]		= "Bal",
	[PC_BreakX_CH]		= "BreakX",
	[PC_BreakX_CS]		= "BreakX",
	[PC_BreakX_VF]		= "BreakX",
	[PC_Break_CH]		= "Break",
	[PC_Break_CS]		= "Break",
	[PC_Break_VF]		= "Break",
	[PC_Call_Imm]		= " % ",
	[PC_Call_OnM]		= " / ",
	[PC_Fail]		= "Fail",
	[PC_Fence]		= "Fence",
	[PC_Fence_X]		= "Fence",
	[PC_Len_NF]		= "Len",
	[PC_Len_Nat]		= "Len",
	[PC_NSpan_CH]		= "NSpan",
	[PC_NSpan_CS]		= "NSpan",
	[PC_NSpan_VF]		= "NSpan",
	[PC_NotAny_CH]		= "NotAny",
	[PC_NotAny_CS]		= "NotAny",
	[PC_NotAny_VF]		= "NotAny",
	[PC_Null]		= "\"\"",
	[PC_Pos_NF]		= "Pos",
	[PC_Pos_Nat]		= "Pos",
	[PC_RPos_NF]		= "RPos",
	[PC_RPos_Nat]		= "RPos",
	[PC_RTab_NF]		= "RTab",
	[PC_RTab_Nat]		= "RTab",
	[PC_Rem]		= "Rem",
	[PC_Setcur_Func]	= "#",		/* also: Setcur */
	[PC_Span_CH]		= "Span",
	[PC_Span_CS]		= "Span",
	[PC_Span_VF]		= "Span",
	[PC_Succeed]		= "Succeed",
	[PC_Tab_NF]		= "Tab",
	[PC_Tab_Nat]		= "Tab",
	[PC_Dynamic_Func]	= "-"		/* also: Pred */
};

/* TODO: Define some custom Append functions */

LUA_SIG(l_tostring)
{
	char		buf[1024], *bigbuf;
	unsigned	len;

	struct state	state = {
		.ptr = buf,
		.size = sizeof(buf)
	};

	PATTERN_WRAPPER	*wrapper = lua_touserdata(L, 1); /* parameter is definitely a pattern */

	luaL_argcheck(L, wrapper->pattern, 1, L_FREED);

	spipat_image_init_state(&state);
	state.cquote = "\"";
	state.concat = " * ";
	state.strings = lspipat_strs;

	len = spipat_image_custom(&state, wrapper->pattern);
	if (len < sizeof(buf)) {
		lua_pushlstring(L, buf, len);
		return 1;
	}

	/* sizeof(buf) was too small */

	state.size = len + 1;
	if (!(bigbuf = malloc(state.size)))
		L_ERROR(L_ALLOC);
	state.ptr = bigbuf;

	spipat_image_custom(&state, wrapper->pattern);
	lua_pushlstring(L, bigbuf, len);

	free(bigbuf);

	return 1;
}

#else

LUA_SIG(l_tostring)
{
	char		buf[1024], *bigbuf;
	unsigned	len;

	PATTERN_WRAPPER	*wrapper = lua_touserdata(L, 1); /* parameter is definitely a pattern */

	luaL_argcheck(L, wrapper->pattern, 1, L_FREED);

	len = spipat_image(wrapper->pattern, buf, sizeof(buf));
	if (len < sizeof(buf)) {
		lua_pushlstring(L, buf, len);
		return 1;
	}

	/* sizeof(buf) was too small */

	if (!(bigbuf = malloc(len + 1)))
		L_ERROR(L_ALLOC);

	spipat_image(wrapper->pattern, bigbuf, len + 1);
	lua_pushlstring(L, bigbuf, len);

	free(bigbuf);

	return 1;
}

#endif
