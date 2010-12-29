--
-- LSPIPAT - LUA SPIPAT WRAPPER
-- Copyright (C) 2010, Robin Haberkorn
-- License: LGPL
--
-- ADDITIONAL METHODS IMPLEMENTED IN LUA
--

module("spipat", package.seeall)

--
-- Module and Pattern methods
--

function ssub(str, pattern, repl, n, flags)
	assert(type(repl) == "string" or type(repl) == "function",
	       "Invalid replacement specified!")
	assert(type(n) == "nil" or type(n) == "number",
	       "Invalid repeat value specified!")

	local cMatches = 0
	repeat
				-- cares about the remaining checks
		local s, e = smatch(str, pattern, flags)
		if not s then break end

		local res = type(repl) == "string" and repl or repl(s, e)
		assert(type(res) == "nil" or type(res) == "string",
		       "Replacement function returned invalid value!")

		if res then str = str:sub(1, s - 1)..res..str:sub(e + 1) end

		if type(n) == "number" then n = n - 1 end
		cMatches = cMatches + 1
	until n == 0

	return str, cMatches
end

function siter(str, pattern, flags)
	local endPos = 0
	pattern = Pos(function() return endPos end) * Arb() *
		  #function(p) startPos = p + 1 end * pattern * #function(p) endPos = p end

	return function()
		if not smatch(str, pattern, flags) then return end
		return startPos, endPos
	end
end

--
-- Primitives (shortcuts for deferring global variables)
--

local function genericSetGlobal(val, name) _G[name] = val end

function _Setcur(name) return Setcur(genericSetGlobal, name) end
_G._Setcur = _Setcur
-- unfortunately, we can't register this as __len to strings...

		-- NOTE: if global `name' is of an invalid type,
		-- lspipat will raise an error automatically
local function genericGetGlobal(name) return _G[name] end

for _, prim in ipairs{
	"Pred",							-- _Pred will be registered as __unm to strings
	"Any", "Break", "BreakX", "NotAny", "NSpan", "Span",	-- string primitives
	"Len", "Pos", "RPos", "RTab", "Tab"			-- number primitives
} do
	local _prim = "_"..prim

	spipat[_prim] = function(name) return spipat[prim](genericGetGlobal, name) end
	_G[_prim] = spipat[_prim]
end

-- FIXME: local cookie support for assignments -> shortcuts for assignment of global variables

--
-- POSIX Extended Regular Expressions To SPITBOL Pattern Compiler
--

function RegExp(str, captures)
	assert(type(captures) == "nil" or type(captures) == "table",
	       "Invalid captures table given!")

	local stack = {}
	local function push(v) table.insert(stack, v) end
	local function pop() return table.remove(stack) end
	local r2p = {["."] = Len(1), ["^"] = Pos(0), ["$"] = RPos(0)}

	local set
	local function add(c) table.insert(set, c) return c end

	local classes = {
		blank = " \t",
		punct = [[-!"#$%&'()*+,./:;<=>?@[\]^_`{|}~]],
		lower = "abcdefghijklmnopqrstuvwxyz",
		digit = "0123456789"
	}
	classes.upper = classes.lower:upper()
	classes.alpha = classes.upper..classes.lower
	classes.alnum = classes.alpha..classes.digit
	classes.word = classes.alnum.."_"
	classes.xdigit = classes.upper:sub(1, 6)..classes.lower:sub(1, 6)..classes.digit
	classes.space = classes.blank.."\r\n\v\f"
	-- TODO: some character classes are still missing...

	local function exp() return exp end
	local function seq() return seq end
	local atom = ( "\\" * (Len(1) % push)
		     + NotAny(".[]^$()*+?|{}") % push
	             + Any(".^$") % function(r) push(r2p[r]) end
	             + "[" * ( "^" * -function() push(NotAny) set = {} end
	       	             +       -function() push(Any) set = {} end )
		           * (topattern("]") % add + "")
		           * Arbno( "[:" * (Break(":") % push) * ":]" * -function() return add(classes[pop()]) ~= nil end
			   	  + Len(1) * "-" * Len(1)
			          % function(range) for c = range:byte(), range:byte(3) do add(string.char(c)) end end
			          + Len(1) % add )
		           * "]" * -function() push(pop()(table.concat(set))) end
	             + "(" * -exp * ")"
		     	   * -function() if captures then
			   		 push(topattern(pop()) / function(cap) table.insert(captures, cap) end) end end )
	             * ( "*" * ( "?" * -function() push(Arbno(pop())) end
	     	               +       -function() local r; r = pop() * -function() return r end + ""
	     				           push(r) end )
		       + "+" * -function() local r; r = pop() * (-function() return r end + "")
	     			           push(r) end
		       + "?" * -function() push(topattern("") + pop()) end
		       + "{" * ( Span(classes.digit) % push ) * ","
		       	     * ( Span(classes.digit)
		               % function(max) local min, c = pop()
			       		       local r; r = pop() * -function() c = c + 1
					       					return c >= tonumber(max) or r end + ""
			     		       push(-function() c = 0 end * r * -function() return c >= tonumber(min) end) end )
			     * "}"
		       + "" )
	seq = ( atom * -function() local rvalue, lvalue = pop(), pop()
				   push(type(lvalue) == "string" and type(rvalue) == "string" and
				        lvalue..rvalue or lvalue * rvalue) end
	      * (-seq + "") + "" )
	    * ( "|" * -exp * -function() local pat = pop() push(pop() + topattern(pat)) end
	      + "" )
	exp = atom * seq

	assert(smatch(str, exp * RPos(0), match_anchored),
	       "Invalid regular expression!")

	return stack[1]
end
_G.RegExp = RegExp

		-- load C core, also registers Lua functions into metatables we cannot
		-- access from Lua
require "lspipat.core"