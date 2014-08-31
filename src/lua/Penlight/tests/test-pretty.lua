local pretty = require 'pl.pretty'
local utils = require 'pl.utils'
local test = require 'pl.test'
local asserteq, assertmatch = test.asserteq, test.assertmatch

t1 = {
    'one','two','three',{1,2,3},
    alpha=1,beta=2,gamma=3,['&']=true,[0]=false,
    _fred = {true,true},
    s = [[
hello dolly
you're so fine
]]
}

s = pretty.write(t1) --,' ',true)
t2,err = pretty.read(s)
if err then return print(err) end
asserteq(t1,t2)

res,err = pretty.read [[
  {
	['function'] = true,
	['do'] = true,
  }
]]
assert(res)

res,err = pretty.read [[
  {
	['function'] = true,
	['do'] = function() return end
  }
]]
assertmatch(err,'cannot have functions in table definition')

res,err = pretty.load([[
-- comments are ok
a = 2
bonzo = 'dog'
t = {1,2,3}
]])

asserteq(res,{a=2,bonzo='dog',t={1,2,3}})

--- another potential problem is string functions called implicitly as methods--
res,err = pretty.read [[
{s = ('woo'):gsub('w','wwwwww'):gsub('w','wwwwww')}
]]

assertmatch(err,utils.lua51 and 'attempt to index a string value' or "attempt to index constant 'woo'")

---- pretty.load has a _paranoid_ option
res,err = pretty.load([[
k = 0
for i = 1,1e12 do k = k + 1 end
]],{},true)

assertmatch(err,'looping not allowed')

-- Check to make sure that no spaces exist when write is told not to
local tbl = { "a", 2, "c", false, 23, 453, "poot", 34 }
asserteq( pretty.write( tbl, "" ), [[{"a",2,"c",false,23,453,"poot",34}]] )

-- Check that write correctly prevents cycles

local t1,t2 = {},{}
t1[1] = t1
asserteq( pretty.write(t1,""), [[{<cycle>}]] )
t1[1],t1[2],t2[1] = 42,t2,t1
asserteq( pretty.write(t1,""), [[{42,{<cycle>}}]] )

-- Check false positives in write's cycles prevention

t2 = {}
t1[1],t1[2] = t2,t2
asserteq( pretty.write(t1,""), [[{{},{}}]] )

function testm(x,s)
  asserteq(pretty.number(x,'M'),s)
end

testm(123,'123B')
testm(1234,'1.2KiB')
testm(10*1024,'10.0KiB')
testm(1024*1024,'1.0MiB')

function testn(x,s)
  asserteq(pretty.number(x,'N',2),s)
end

testn(123,'123')
testn(1234,'1.23K')
testn(10*1024,'10.24K')
testn(1024*1024,'1.05M')
testn(1024*1024*1024,'1.07B')

function testc(x,s)
  asserteq(pretty.number(x,'T'),s)
end

testc(123,'123')
testc(1234,'1,234')
testc(12345,'12,345')
testc(123456,'123,456')
testc(1234567,'1,234,567')
testc(12345678,'12,345,678')

asserteq(pretty.number(1e12,'N'),'1000.0B')

