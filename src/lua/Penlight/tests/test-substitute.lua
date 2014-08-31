local subst = require 'pl.template'.substitute
local List = require 'pl.List'
local asserteq = require 'pl.test'.asserteq

asserteq(subst([[
# for i = 1,2 do
<p>Hello $(tostring(i))</p>
# end
]],_G),[[
<p>Hello 1</p>
<p>Hello 2</p>
]])

asserteq(subst([[
<ul>
# for name in ls:iter() do
   <li>$(name)</li>
#end
</ul>
]],{ls = List{'john','alice','jane'}}),[[
<ul>
   <li>john</li>
   <li>alice</li>
   <li>jane</li>
</ul>
]])

-- can change the default escape from '#' so we can do C/C++ output.
-- note that the environment can have a parent field.
asserteq(subst([[
> for i,v in ipairs{'alpha','beta','gamma'} do
    cout << obj.${v} << endl;
> end
]],{_parent=_G, _brackets='{}', _escape='>'}),[[
    cout << obj.alpha << endl;
    cout << obj.beta << endl;
    cout << obj.gamma << endl;
]])



