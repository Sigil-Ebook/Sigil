-- test-compare-no-order.lua

local T = require 'pl.tablex'
local P = require 'pl.permute'

local t = {10,20,5,5,10,'one',555}

local permutations = P.table(t)
print('permutations',#permutations)
for _,p in ipairs(permutations) do
	if not T.compare_no_order(t,p) then return print 'different!' end
end
print 'DONE'
