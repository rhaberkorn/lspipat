#!/usr/bin/lua

require "lspipat"

function EXIT(...)
	io.stderr:write(string.format(...))
	os.exit()
end

stack = {}
function push(val) table.insert(stack, val) end
function binop()
	table.insert(stack, {
		l = table.remove(stack),
		type = table.remove(stack),
		r = table.remove(stack)
	})
end

function compile(node)
	if type(node) ~= "table" then return string.rep("+", tonumber(node)) end

	local ret = compile(node.l)..">"..compile(node.r)
	node.type:smatch( Any("+-") % function(o) ret = ret.."[<"..o..">-]<" end
			+ "*" * -function() ret = ">>"..ret.."[<[<+<+>>-]<[>+<-]>>-]<[-]<<" end
			+ "/" * -function() ret = ">"..ret.."<[->->+>>+<<<[>>+>[-]<<<-]>>"..
						  "[<<+>>-]>[-<<[<+>-]<<<+>>>>>]<<<<]>[-]>[-]<<<" end,
			  spipat.match_anchored )

	return ret
end

if #arg ~= 1 then EXIT("Invalid number of parameters\n") end

space = NSpan(" ")
pre = space * ("(" * -"exp" * space * ")" + Span("0123456789") % push)
post = space * ( Any("+-") % push * -"exp" * -binop
	       + Any("*/") % push * pre * -binop * -"post" ) + ""
exp = pre * post

if not arg[1]:smatch(exp * RPos(0), spipat.match_anchored) then EXIT("Invalid expression!\n") end

src = compile(stack[1])..
      "[>++++++++++<[->->+>>+<<<[>>+>[-]<<<-]>>[<<+>>-]"..
      ">[-<<[<+>-]>>>+<]<<<<]>>>>>[<<<<<+>>>>>-]>[>]"..string.rep("+", string.byte("0"))..
      "[<]<<<[>>>>[>]<+[<]<<<-]<[-]<]>>>>>>[>]<[.<]"

print(src)
