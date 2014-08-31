local stringx = require 'pl.stringx'
local asserteq = require 'pl.test' . asserteq
local T = require 'pl.test'.tuple

local function FIX(s)
  io.stderr:write('FIX:' .. s .. '\n')
end


-- isalpha
asserteq(T(stringx.isalpha''), T(false))
asserteq(T(stringx.isalpha' '), T(false))
asserteq(T(stringx.isalpha'0'), T(false))
asserteq(T(stringx.isalpha'\0'), T(false))
asserteq(T(stringx.isalpha'azAZ'), T(true))
asserteq(T(stringx.isalpha'az9AZ'), T(false))

-- isdigit
asserteq(T(stringx.isdigit''), T(false))
asserteq(T(stringx.isdigit' '), T(false))
asserteq(T(stringx.isdigit'a'), T(false))
asserteq(T(stringx.isdigit'0123456789'), T(true))

-- isalnum
asserteq(T(stringx.isalnum''), T(false))
asserteq(T(stringx.isalnum' '), T(false))
asserteq(T(stringx.isalnum('azAZ01234567890')), T(true))

-- isspace
asserteq(T(stringx.isspace''), T(false))
asserteq(T(stringx.isspace' '), T(true))
asserteq(T(stringx.isspace' \r\n\f\t'), T(true))
asserteq(T(stringx.isspace' \r\n-\f\t'), T(false))

-- islower
asserteq(T(stringx.islower''), T(false))
asserteq(T(stringx.islower'az'), T(true))
asserteq(T(stringx.islower'aMz'), T(false))
asserteq(T(stringx.islower'a z'), T(true))

-- startswith
local startswith = stringx.startswith
asserteq(T(startswith('', '')), T(true))
asserteq(T(startswith('', 'a')), T(false))
asserteq(T(startswith('a', '')), T(true))
asserteq(T(startswith('a', 'a')), T(true))
asserteq(T(startswith('a', 'b')), T(false))
asserteq(T(startswith('a', 'ab')), T(false))
asserteq(T(startswith('abc', 'ab')), T(true))
asserteq(T(startswith('abc', 'bc')), T(false)) -- off by one
asserteq(T(startswith('abc', '.')), T(false)) -- Lua pattern char
asserteq(T(startswith('a\0bc', 'a\0b')), T(true)) -- '\0'


-- endswith
-- http://snippets.luacode.org/sputnik.lua?p=snippets/Check_string_ends_with_other_string_74
local endswith = stringx.endswith
asserteq(T(endswith("", "")), T(true))
asserteq(T(endswith("", "a")), T(false))
asserteq(T(endswith("a", "")), T(true))
asserteq(T(endswith("a", "a")), T(true))
asserteq(T(endswith("a", "A")), T(false)) -- case sensitive
asserteq(T(endswith("a", "aa")), T(false))
asserteq(T(endswith("abc", "")), T(true))
asserteq(T(endswith("abc", "ab")), T(false)) -- off by one
asserteq(T(endswith("abc", "c")), T(true))
asserteq(T(endswith("abc", "bc")), T(true))
asserteq(T(endswith("abc", "abc")), T(true))
asserteq(T(endswith("abc", " abc")), T(false))
asserteq(T(endswith("abc", "a")), T(false))
asserteq(T(endswith("abc", ".")), T(false)) -- Lua pattern char
asserteq(T(endswith("ab\0c", "b\0c")), T(true))     -- \0
asserteq(T(endswith("ab\0c", "b\0d")), T(false)) -- \0

asserteq(endswith('dollar.dot',{'.dot','.txt'}),true)
asserteq(endswith('dollar.txt',{'.dot','.txt'}),true)
asserteq(endswith('dollar.rtxt',{'.dot','.txt'}),false)

-- splitlines
asserteq(T(stringx.splitlines('')), T({''}))
asserteq(stringx.splitlines('a'), {'a'})
asserteq(stringx.splitlines('\n'), {''})
asserteq(stringx.splitlines('\n\n'), {'', ''})
asserteq(stringx.splitlines('\r\r'), {'', ''})
asserteq(stringx.splitlines('ab\ncd\n'), {'ab', 'cd'})

-- expandtabs
---FIX[[raises error
asserteq(T(stringx.expandtabs('',0)), T(''))
asserteq(T(stringx.expandtabs('',1)), T(''))
asserteq(T(stringx.expandtabs(' ',1)), T(' '))
-- expandtabs now works like Python's str.expandtabs (up to next tab stop)
asserteq(T(stringx.expandtabs(' \t ')), T((' '):rep(1+8)))
asserteq(T(stringx.expandtabs(' \t ',2)), T('   '))
--]]

-- lfind
asserteq(T(stringx.lfind('', '')), T(1))
asserteq(T(stringx.lfind('a', '')), T(1))
asserteq(T(stringx.lfind('ab', 'b')), T(2))
asserteq(T(stringx.lfind('abc', 'cd')), T(nil))
asserteq(T(stringx.lfind('abcbc', 'bc')), T(2))

-- rfind
asserteq(T(stringx.rfind('', '')), T(1))
asserteq(T(stringx.rfind('ab', '')), T(3))
asserteq(T(stringx.rfind('abcbc', 'bc')), T(4))
asserteq(T(stringx.rfind('abcbcb', 'bc')), T(4))
asserteq(T(stringx.rfind('ab..cd', '.')), T(4)) -- pattern char

-- replace
asserteq(T(stringx.replace('', '', '')), T(''))
asserteq(T(stringx.replace(' ', '', '')), T(' '))
asserteq(T(stringx.replace(' ', '', ' ')), T('   '))
asserteq(T(stringx.replace('    ', '  ', '')), T(''))
asserteq(T(stringx.replace('abcabcabc', 'bc', 'BC')), T('aBCaBCaBC'))
asserteq(T(stringx.replace('abcabcabc', 'bc', 'BC', 1)), T('aBCabcabc'))
asserteq(T(stringx.replace('abcabcabc', 'bc', 'BC', 0)), T('abcabcabc'))
asserteq(T(stringx.replace('abc', 'd', 'e')), T('abc'))
asserteq(T(stringx.replace('a.b', '.', '%d')), T('a%db'))

-- split
local split = stringx.split
asserteq(split('', ''), {''})
asserteq(split('', 'z'), {}) --FIX:intended and specified behavior?
asserteq(split('a', ''), {'a'}) --FIX:intended and specified behavior?
asserteq(split('a', 'a'), {''})
-- stringx.split now follows the Python pattern, so it uses a substring, not a pattern.
-- If you need to split on a pattern, use utils.split()
-- asserteq(split('ab1cd23ef%d', '%d+'), {'ab', 'cd', 'ef%d'}) -- pattern chars
-- note that leading space is ignored by the default
asserteq(split(' 1  2  3 '),{'1','2','3'})
asserteq(split('a*bb*c*ddd','*'),{'a','bb','c','ddd'})
asserteq(split('dog:fred:bonzo:alice',':',3), {'dog','fred','bonzo:alice'})
asserteq(split('///','/'),{'','','',''})
-- capitalize
asserteq(T(stringx.capitalize('')), T(''))
asserteq(T(stringx.capitalize('abC deF1')), T('Abc Def1')) -- Python behaviour

-- count
asserteq(T(stringx.count('', '')), T(0)) --infinite loop]]
asserteq(T(stringx.count('  ', '')), T(2)) --infinite loop]]
asserteq(T(stringx.count('a..c', '.')), T(2)) -- pattern chars
asserteq(T(stringx.count('a1c', '%d')), T(0)) -- pattern chars

-- ljust
asserteq(T(stringx.ljust('', 0)), T(''))
asserteq(T(stringx.ljust('', 2)), T('  '))
asserteq(T(stringx.ljust('ab', 3)), T('ab '))
asserteq(T(stringx.ljust('ab', 3, '%')), T('ab%'))
asserteq(T(stringx.ljust('abcd', 3)), T('abcd')) -- agrees with Python

-- rjust
asserteq(T(stringx.rjust('', 0)), T(''))
asserteq(T(stringx.rjust('', 2)), T('  '))
asserteq(T(stringx.rjust('ab', 3)), T(' ab'))
asserteq(T(stringx.rjust('ab', 3, '%')), T('%ab'))
asserteq(T(stringx.rjust('abcd', 3)), T('abcd')) -- agrees with Python

-- center
asserteq(T(stringx.center('', 0)), T(''))
asserteq(T(stringx.center('', 1)), T(' '))
asserteq(T(stringx.center('', 2)), T('  '))
asserteq(T(stringx.center('a', 1)), T('a'))
asserteq(T(stringx.center('a', 2)), T(' a'))
asserteq(T(stringx.center('a', 3)), T(' a '))


-- ltrim
-- http://snippets.luacode.org/sputnik.lua?p=snippets/trim_whitespace_from_string_76
local trim = stringx.lstrip
asserteq(T(trim''), T'')
asserteq(T(trim' '), T'')
asserteq(T(trim'  '), T'')
asserteq(T(trim'a'), T'a')
asserteq(T(trim' a'), T'a')
asserteq(T(trim'a '), T'a ')
asserteq(T(trim' a '), T'a ')
asserteq(T(trim'  a  '), T'a  ')
asserteq(T(trim'  ab cd  '), T'ab cd  ')
asserteq(T(trim' \t\r\n\f\va\000b \r\t\n\f\v'), T'a\000b \r\t\n\f\v')
-- more


-- rtrim
-- http://snippets.luacode.org/sputnik.lua?p=snippets/trim_whitespace_from_string_76
local trim = stringx.rstrip
asserteq(T(trim''), T'')
asserteq(T(trim' '), T'')
asserteq(T(trim'  '), T'')
asserteq(T(trim'a'), T'a')
asserteq(T(trim' a'), T' a')
asserteq(T(trim'a '), T'a')
asserteq(T(trim' a '), T' a')
asserteq(T(trim'  a  '), T'  a')
asserteq(T(trim'  ab cd  '), T'  ab cd')
asserteq(T(trim' \t\r\n\f\va\000b \r\t\n\f\v'), T' \t\r\n\f\va\000b')
-- more


-- trim
-- http://snippets.luacode.org/sputnik.lua?p=snippets/trim_whitespace_from_string_76
local trim = stringx.strip
asserteq(T(trim''), T'')
asserteq(T(trim' '), T'')
asserteq(T(trim'  '), T'')
asserteq(T(trim'a'), T'a')
asserteq(T(trim' a'), T'a')
asserteq(T(trim'a '), T'a')
asserteq(T(trim' a '), T'a')
asserteq(T(trim'  a  '), T'a')
asserteq(T(trim'  ab cd  '), T'ab cd')
asserteq(T(trim' \t\r\n\f\va\000b \r\t\n\f\v'), T'a\000b')
-- more


-- partition
-- as per str.partition in Python, delimiter must be non-empty;
-- interpreted as a plain string
--asserteq(T(stringx.partition('', '')), T('', '', '')) -- error]]
--asserteq(T(stringx.partition('a', '')), T('', '', 'a')) --error]]
asserteq(T(stringx.partition('a', 'a')), T('', 'a', ''))
asserteq(T(stringx.partition('abc', 'b')), T('a', 'b', 'c'))
asserteq(T(stringx.partition('abc', '.+')), T('abc','',''))
asserteq(T(stringx.partition('a,b,c', ',')), T('a',',','b,c'))
-- rpartition
asserteq(T(stringx.rpartition('a/b/c', '/')), T('a/b', '/', 'c'))
asserteq(T(stringx.rpartition('abc', 'b')), T('a', 'b', 'c'))


-- at (works like s:sub(idx,idx), so negative indices allowed
asserteq(T(stringx.at('a', 1)), T('a'))
asserteq(T(stringx.at('ab', 2)), T('b'))
asserteq(T(stringx.at('abcd', -1)), T('d'))

-- lines
local function merge(it, ...)
  assert(select('#', ...) == 0)
  local ts = {}
  for val in it do ts[#ts+1] = val end
  return ts
end
asserteq(merge(stringx.lines('')), {''})
asserteq(merge(stringx.lines('ab')), {'ab'})
asserteq(merge(stringx.lines('ab\ncd')), {'ab', 'cd'})

-- shorten
-- The returned string is always equal or less to the given size.
asserteq(T(stringx.shorten('', 0)), T'')
asserteq(T(stringx.shorten('a', 1)), T'a')
asserteq(T(stringx.shorten('ab', 1)), T'.') --FIX:ok?
asserteq(T(stringx.shorten('abc', 3)), T'abc')
asserteq(T(stringx.shorten('abcd', 3)), T'...')
asserteq(T(stringx.shorten('abcde', 5)), T'abcde')
asserteq(T(stringx.shorten('abcde', 4)), T'a...')
asserteq(T(stringx.shorten('abcde', 3)), T'...')
asserteq(T(stringx.shorten('abcde', 2)), T'..')
asserteq(T(stringx.shorten('abcde', 0)), T'')
asserteq(T(stringx.shorten('', 0, true)), T'')
asserteq(T(stringx.shorten('a', 1, true)), T'a')
asserteq(T(stringx.shorten('ab', 1, true)), T'.')
asserteq(T(stringx.shorten('abcde', 5, true)), T'abcde')
asserteq(T(stringx.shorten('abcde', 4, true)), T'...e')
asserteq(T(stringx.shorten('abcde', 3, true)), T'...')
asserteq(T(stringx.shorten('abcde', 2, true)), T'..')
asserteq(T(stringx.shorten('abcde', 0, true)), T'')

-- strip
asserteq(stringx.strip('    hello         '),'hello')
asserteq(stringx.strip('--[hello] -- - ','-[] '),'hello')
asserteq(stringx.rstrip('--[hello] -- - ','-[] '),'--[hello')

