#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([SNOBOL/SPITBOL Patterns for Lua], [0.1], [robin.haberkorn@googlemail.com], [lspipat])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/lspipat.c])
AC_CONFIG_HEADERS([config.h])

LT_INIT([disable-static])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Lua compiler (optional)
AC_CHECK_PROG(LUAC, luac5.1, luac5.1)
AC_CHECK_PROG(LUAC, luac, luac)

LUAC_FLAGS=
AC_SUBST(LUAC_FLAGS)

# XSLTProc (optional)
AC_CHECK_PROG(XSLTPROC, xsltproc, xsltproc)

XSLT_FLAGS="--xinclude"
AC_SUBST(XSLT_FLAGS)

# Checks for libraries.

# libspipat
AC_CHECK_LIB([spipat], [spipat_match2], , [
	AC_MSG_ERROR([libspipat (Spipat library) not found!])
])

# liblua (care about different distributions)
AC_CHECK_LIB([lua5.1], [lua_call], , [
	AC_CHECK_LIB([lua], [lua_call], , [
		AC_MSG_ERROR([liblua (Lua 5.1 library) not found!])
	])
])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h stdbool.h])

# spipat headers
AC_CHECK_HEADERS([spipat.h], , [
	AC_MSG_ERROR([Spipat header not found!])
], [
	#include <stdint.h>
	#include <stdbool.h>
])

# spipat_impl.h/spipat_image.h are not installed by default and are thus optional
AC_CHECK_HEADERS([spipat_impl.h spipat_image.h], , [
	AC_MSG_WARN([Optional spipat header not found! You are strongly encouraged to specify spipat's source dir in CPPFLAGS.])
], [
	#include <stdint.h>
	#include <stdbool.h>
	#include <spipat.h>
])

# Lua headers (care about different distributions)
AC_CHECK_HEADERS([lua5.1/lua.h lua5.1/lauxlib.h lua5.1/lualib.h], , [
	AC_CHECK_HEADERS([lua.h lauxlib.h lualib.h], , [
		AC_MSG_ERROR([Lua 5.1 headers not found!])
	])
	break
])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_STDBOOL
AC_TYPE_UINT32_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

# Package Configuration

AC_ARG_ENABLE(lua-libdir,
        AS_HELP_STRING([--enable-lua-libdir=DIR],
                       [Install lspipat into this directory (default is LIBDIR/lua/5.1)]),
        [lualibdir=$enable_lua_libdir], [lualibdir=${libdir}/lua/5.1])
AC_SUBST(lualibdir)
lualib_lspipatdir=${lualibdir}/lspipat
AC_SUBST(lualib_lspipatdir)

AC_ARG_ENABLE(lua-precompile,
        AS_HELP_STRING([--enable-lua-precompile],
                       [Enable precompilation of Lua source files (default is yes)]),
	[lua_precompile=$enableval], [lua_precompile=yes])
AM_CONDITIONAL([LUA_PRECOMPILE], [test x$lua_precompile = xyes])

if test x$lua_precompile = xyes -a x$LUAC = x; then
	AC_MSG_ERROR([Lua chunk precompilation enabled, but Lua 5.1 compiler not found! Try --disable-lua-precompile.])
fi

AC_ARG_ENABLE(lua-strip,
        AS_HELP_STRING([--enable-lua-strip],
                       [Strip compiled Lua source files (default is yes)]),
	[lua_strip=$enableval], [lua_strip=yes])
if test x$lua_strip = xyes; then
	LUAC_FLAGS+=" -s"
fi

AC_ARG_ENABLE(html-doc,
        AS_HELP_STRING([--enable-html-doc],
                       [Generate HTML documentation (default is yes)]),
	[html_doc=$enableval], [html_doc=yes])

if test x$html_doc = xyes -a x$XSLTPROC = x; then
	AC_MSG_ERROR([Enabled generating documentation, but XSLTProc not found! Try --disable-html-doc.])
fi

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile])
AC_OUTPUT
