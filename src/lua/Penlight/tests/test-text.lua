local T = require 'pl.text'
local utils = require 'pl.utils'
local Template = T.Template
local asserteq = require 'pl.test'.asserteq
local OrderedMap = require 'pl.OrderedMap'

local t1 = Template [[
while true do
    $contents
end
]]

assert(t1:substitute {contents = 'print "hello"'},[[
while true do
    print "hello"
end
]])

assert(t1:indent_substitute {contents = [[
for i = 1,10 do
    gotcha(i)
end
]]},[[
while true do
    for i = 1,10 do
        gotcha(i)
    end
end
]])

asserteq(T.dedent [[
    one
    two
    three
]],[[
one
two
three
]])
asserteq(T.fill ([[
It is often said of Lua that it does not include batteries. That is because the goal of Lua is to produce a lean expressive language that will be used on all sorts of machines, (some of which don't even have hierarchical filesystems). The Lua language is the equivalent of an operating system kernel; the creators of Lua do not see it as their responsibility to create a full software ecosystem around the language. That is the role of the community.
]],20),[[
It is often said of Lua
that it does not include
batteries. That is because
the goal of Lua is to
produce a lean expressive
language that will be
used on all sorts of machines,
(some of which don't
even have hierarchical
filesystems). The Lua
language is the equivalent
of an operating system
kernel; the creators of
Lua do not see it as their
responsibility to create
a full software ecosystem
around the language. That
is the role of the community.
]])

local template = require 'pl.template'

local t = [[
# for i = 1,3 do
    print($(i+1))
# end
]]

asserteq(template.substitute(t),[[
    print(2)
    print(3)
    print(4)
]])

t = [[
> for i = 1,3 do
    print(${i+1})
> end
]]

asserteq(template.substitute(t,{_brackets='{}',_escape='>'}),[[
    print(2)
    print(3)
    print(4)
]])

--- iteration using pairs is usually unordered. But using OrderedMap
--- we can get the exact original ordering.

t = [[
# for k,v in pairs(T) do
    "$(k)", -- $(v)
# end
]]

if utils.lua51 then
    -- easy enough to define a general pairs in Lua 5.1
    local rawpairs = pairs
    function pairs(t)
        local mt = getmetatable(t)
        local f = mt and mt.__pairs
        if f then
            return f(t)
        else
            return rawpairs(t)
        end
    end
end


local Tee = OrderedMap{{Dog = 'Bonzo'}, {Cat = 'Felix'}, {Lion = 'Leo'}}

-- note that the template will also look up global functions using _parent
asserteq(template.substitute(t,{T=Tee,_parent=_G}),[[
    "Dog", -- Bonzo
    "Cat", -- Felix
    "Lion", -- Leo
]])

-- for those with a fondness for Python-style % formatting...
T.format_operator()
asserteq('[%s]' % 'home', '[home]')
asserteq('%s = %d' % {'fred',42},'fred = 42')

-- mostly works like string.format, except that %s forces use of tostring()
-- rather than throwing an error
local List = require 'pl.List'
asserteq('TBL:%s' % List{1,2,3},'TBL:{1,2,3}')

-- table with keys and format with $
asserteq('<$one>' % {one=1}, '<1>')
-- (second arg may also be a function, like os.getenv)
function subst(k)
    if k == 'A' then return 'ay'
    elseif k == 'B' then return 'bee'
    else return '?'
    end
end
asserteq(
    '$A & $B' % subst,'ay & bee'
)

t = [[
a whole lot
of love
]]

asserteq(T.indent(t,4),[[
    a whole lot
    of love
]])

asserteq(T.indent([[
easy

enough!
]],2,'*'),[[
**easy
**
**enough!
]])


