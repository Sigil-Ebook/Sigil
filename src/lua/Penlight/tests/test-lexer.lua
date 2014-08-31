asserteq = require('pl.test').asserteq
T = require 'pl.test' . tuple
lexer = require 'pl.lexer'
seq = require 'pl.seq'
List = require ('pl.List')
copy2 = seq.copy2

s = '20 = hello'
 asserteq(copy2(lexer.scan (s,nil,{space=false},{number=false})),
    {{'number','20'},{'space',' '},{'=','='},{'space',' '},{'iden','hello'}})

 asserteq(copy2(lexer.scan (s,nil,{space=true},{number=true})),
    {{'number',20},{'=','='},{'iden','hello'}})

asserteq(copy2(lexer.lua('test(20 and a > b)',{space=true})),
    {{'iden','test'},{'(','('},{'number',20},{'keyword','and'},{'iden','a'},
      {'>','>'},{'iden','b'},{')',')'}} )

lines = [[
for k,v in pairs(t) do
    if type(k) == 'number' then
        print(v) -- array-like case
    else
        print(k,v)
    end -- if
end
]]

ls = List()
for tp,val in lexer.lua(lines,{space=true,comments=true}) do
    assert(tp ~= 'space' and tp ~= 'comment')
    if tp == 'keyword' then ls:append(val) end
end
asserteq(ls,List{'for','in','do','if','then','else','end','end'})

tok = lexer.scan([[
    'help'  "help" "dolly you're fine" "a \"quote\" here"
]],nil,{space=true,string=true})

function t2() local t,v = tok(); return v end

asserteq(t2(),'help')
asserteq(t2(),'help')
asserteq(t2(),"dolly you're fine")
asserteq(t2(),"a \\\"quote\\\" here")  --> NOT convinced this is correct!

tok = lexer.lua('10+2.3') ---> '+' is no longer considered part of the number!
asserteq(T(tok()),T('number',10))
asserteq(T(tok()),T('+','+'))
asserteq(T(tok()),T('number',2.3))

local txt = [==[
-- comment
--[[
block
comment
]][[
hello dammit
]][[hello]]
]==]

tok = lexer.lua(txt,{})
asserteq(tok(),'comment')
asserteq(tok(),'comment')
asserteq(tok(),'string')
asserteq(tok(),'string')
asserteq(tok(),'space')

txt = [[
// comment
/* a long
set of words */ // more
]]

tok = lexer.cpp(txt,{})
asserteq(tok(),'comment')
asserteq(tok(),'comment')
asserteq(tok(),'space')
asserteq(tok(),'comment')

local function teststring (s)
    local tok = lexer.lua(s,{},{string=false})
    local t,v = tok()
    asserteq(t,"string")
    asserteq(v,s)
end

teststring [["hello\\"]]
teststring [["hello\"dolly"]]
teststring [['hello\'dolly']]
teststring [['']]
teststring [[""]]


