-- another SIP example, shows how an awkward log file format
-- can be parsed. It also prints out the actual Lua string
-- pattern generated:
-- SYNC%s*%[([+%-%d]%d*)%]%s*([+%-%d]%d*)%s*([+%-%d]%d*)

require 'pl'

s = [[
SYNC [1] 0 547 (14679 sec)
SYNC [2] 0 555 (14679 sec)
SYNC [3] 0 563 (14679 sec)
SYNC [4] 0 571 (14679 sec)
SYNC [5] -1 580 (14679 sec)
SYNC [6] 0 587 (14679 sec)
]]


local first = true
local start
local res = {}
local pat = 'SYNC [$i{seq}] $i{diff} $i{val}'
print(sip.create_pattern(pat))
local match = sip.compile(pat)
for line in stringx.lines(s) do
  if match(line,res) then
    if first then
      expected = res.val
      first = false
    end
    print(res.val,expected - res.val)
    expected = expected + 8
  end
end
