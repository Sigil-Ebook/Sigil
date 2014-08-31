local path = require 'pl.path'
asserteq = require 'pl.test'.asserteq

function quote(s)
	return '"'..s..'"'
end

function print2(s1,s2)
	print(quote(s1),quote(s2))
end

function testpath(pth,p1,p2,p3)
    local dir,rest = path.splitpath(pth)
    local name,ext = path.splitext(rest)
    asserteq(dir,p1)
    asserteq(name,p2)
    asserteq(ext,p3)
end

testpath ([[/bonzo/dog_stuff/cat.txt]],[[/bonzo/dog_stuff]],'cat','.txt')
testpath ([[/bonzo/dog/cat/fred.stuff]],'/bonzo/dog/cat','fred','.stuff')
testpath ([[../../alice/jones]],'../../alice','jones','')
testpath ([[alice]],'','alice','')
testpath ([[/path-to/dog/]],[[/path-to/dog]],'','')

asserteq( path.isdir( "../doc" ), true )
asserteq( path.isdir( "../doc/config.ld" ), false )

asserteq( path.isfile( "../doc" ), false )
asserteq( path.isfile( "../doc/config.ld" ), true )

local norm = path.normpath
local p = norm '/a/b'

asserteq(norm '/a/fred/../b',p)
asserteq(norm '/a//b',p)

function testnorm(p1,p2)
    asserteq(p2,norm(p1):gsub('\\','/'))
end

testnorm('a/b/..','a/')
testnorm('a/b/../..','.')
testnorm('a/b/../c/../../d','d')
testnorm('a/.','a/.')
testnorm('a/./','a/')
testnorm('a/b/.././..','.')

if path.is_windows then
  asserteq(norm [[\a\.\b]],p)
  -- UNC paths
  asserteq(norm [[\\bonzo\..\dog]], [[\\dog]])
  asserteq(norm [[\\?\c:\bonzo\dog\.\]],[[\\?\c:\bonzo\dog\]])
end

asserteq(norm '1/2/../3/4/../5',norm '1/3/5')

assert(path.join("somepath",".") == "somepath"..path.sep..".")
assert(path.join(".","readme.txt") == "."..path.sep.."readme.txt")
assert(path.join("/a_dir", "abs_path/") == "/a_dir"..path.sep.."abs_path/")
assert(path.join("a_dir", "/abs_path/") == "/abs_path/")
assert(path.join("a_dir", "/abs_path/", "/abs_path2/") == "/abs_path2/")
assert(path.join("a_dir", "/abs_path/", "not_abs_path2/") == "/abs_path/not_abs_path2/")
assert(path.join("a_dir", "/abs_path/", "not_abs_path2/", "/abs_path3/", "not_abs_path4/") == "/abs_path3/not_abs_path4/")
assert(path.join("first","second","third") == "first"..path.sep.."second"..path.sep.."third")
assert(path.join("first","second","") == "first"..path.sep.."second"..path.sep)
assert(path.join("first","","third") == "first"..path.sep.."third")
assert(path.join("","second","third") == "second"..path.sep.."third")
assert(path.join("","") == "")

