local pretty = require 'pl.pretty'

tb = {
    'one','two','three',{1,2,3},
    alpha=1,beta=2,gamma=3,['&']=true,[0]=false,
    _fred = {true,true},
    s = [[
hello dolly
you're so fine
]]
}

print(pretty.write(tb))
