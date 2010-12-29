# Main lspipat Automake file
# processed automatically

AM_CFLAGS = -std=c99 -Wall

lualib_lspipat_LTLIBRARIES = core.la
core_la_SOURCES = lspipat.c lspipat.h \
		  call.c compose.c unary.c render.c misc.c \
		  simple.c string.c uint.c
core_la_LDFLAGS = -module

if LUA_PRECOMPILE

lualib_DATA = lspipat.out
CLEANFILES = $(lualib_DATA)
EXTRA_DIST = lspipat.lua

lspipat.out : lspipat.lua
	@LUAC@ @LUAC_FLAGS@ -o $@ $<

install-data-hook :
	mv -f $(DESTDIR)$(lualibdir)/lspipat.out $(DESTDIR)$(lualibdir)/lspipat.lua

else

dist_lualib_DATA = lspipat.lua

endif
