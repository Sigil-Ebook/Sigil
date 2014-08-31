require 'pl'
relpath = path.relpath

path = '/a/b/c'

function slash (p)
    return (p:gsub('\\','/'))
end

function try (p,r)
    test.asserteq(slash(relpath(p,path)),r)
end

try('/a/b/c/one.lua','one.lua')
try('/a/b/c/bonzo/two.lua','bonzo/two.lua')
try('/a/b/three.lua','../three.lua')
try('/a/four.lua','../../four.lua')
try('one.lua','one.lua')
try('../two.lua','../two.lua')


