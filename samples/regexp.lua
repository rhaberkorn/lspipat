-- Parse IP address using regular expression compiler

require "lspipat"


exp = [=[^([[:digit:]]{1,3})(\.([[:digit:]]{1,3})){3,3}$]=]

ip1 = RegExp(exp)
print(ip1)

local captures = {}
ip2 = RegExp(exp, captures)
print(ip2)

print(spipat.smatch("192.168.0.1", ip1))
print(spipat.smatch("192.168.000.001", ip1))
print(spipat.smatch("192.168.0.XXX", ip1))

print(spipat.smatch("192.168.0.1", ip2))

-- remove captures due to grouping around "."
table.remove(captures, 3)
table.remove(captures, 5)
table.remove(captures, 7)

print(table.concat(captures, "."))
