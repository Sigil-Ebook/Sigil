require 'pl'
-- force us to look in the script's directory when requiring...
app.require_here()
require 'symbols'

local MT = getmetatable(_1)

add = MT.__add
mul = MT.__mul
pow = MT.__pow


function testeq (e1,e2)
    if not equals(e1,e2) then
        print ('Not equal',repr(e1),repr(e2))
    end
end

sin = register(math.sin,'sin')

f = register(function(x,y,z) end)

--[[
testeq (_1,_1)
testeq (_1+_2,_1+_2)
testeq (_1 + 3*_2,_1 + 3*_2)
testeq (_2+_1,_1+_2)
testeq (sin(_1),sin(_1))
testeq (1+f(10,20,'ok'),f(10,20,'ok')+1)
--]]


function testexpand (e)
    print(repr(fold(expand(e)))) --fold
end

--[[
testexpand (a*(a+1))

testexpand ((x+2)*(b+1))
]]--

function testfold (e)
    print(repr(fold(e)))
end

a,b,c,x,y = Var 'a,b,c,x,y'

--~ testfold(_1 + _2)
--~ testfold(add(10,20))
--~ testfold(add(mul(2,_1),mul(3,_2)))
--[[
testfold(sin(a))
e = a^(b+2)
testfold(e)
bindval(b,1)
testfold(e)
bindval(a,2)
testfold(e)

bindval(a)
bindval(b)
]]



function testdiff (e)
    balance(e)
    e = diff(e,x)
    balance(e)
    print('+ ',e)
    e = fold(e)
    print('- ',e)
end


testdiff(x^2+1)
testdiff(3*x^2)
testdiff(x^2 + 2*x^3)
testdiff(x^2 + 2*a*x^3 + x^4)
testdiff(2*a*x^3)
testdiff(x*x*x)



