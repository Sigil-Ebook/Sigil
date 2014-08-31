local stringio = require 'pl.stringio'
local config = require 'pl.config'

function dump(t,indent)
    if type(t) == 'table' then
        io.write(indent,'{\n')
        local newindent = indent..'  '
        for k,v in pairs(t) do
            io.write(newindent,k,'=')
            dump(v,indent)
            io.write('\n')
        end
        io.write(newindent,'},\n')
    else
        io.write(indent,t,'(',type(t),')')
    end
end


function testconfig(test)
    local f = stringio.open(test)
    local c = config.read(f)
    f:close()
    dump(c,'  ')
    print '-----'
end

testconfig [[
 ; comment 2 (an ini file)
[section!]
bonzo.dog=20,30
config_parm=here we go again
depth = 2
[another]
felix="cat"
]]

testconfig [[
# this is a more Unix-y config file
fred = 1
alice = 2
home = /bonzo/dog/etc
]]

testconfig [[
# this is just a set of comma-separated values
1000,444,222
44,555,224
]]


