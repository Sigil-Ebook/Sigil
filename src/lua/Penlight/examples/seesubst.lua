-- shows how replacing '@see module' in the Markdown documentation
-- can be done more elegantly using PL.
-- We either have something like 'pl.config' (a module reference)
-- or 'pl.seq.map' (a function reference); these cases must be distinguished
-- and a Markdown link generated pointing to the LuaDoc file.

require 'pl'

local res = {}
s = [[
(@see pl.bonzo.dog)
remember about @see pl.bonzo

]]

local _gsub_patterns = {}

function gsub (s,pat,subst,start)
    local fpat = _gsub_patterns[pat]
    if not fpat then
        -- use SIP to generate a proper string pattern.
        -- the _whole thing_ is a capture, to get the whole match
        -- and the unnamed capture.
        fpat = '('..sip.create_pattern(pat)..')'
        _gsub_patterns[pat] = fpat
    end
    return s:gsub(fpat,subst,start)
end


local mod = sip.compile '$v.$v'
local fun = sip.compile '$v.$v.$v'

for line in stringx.lines(s) do
    line = gsub(line,'@see $p',function(see,path)
        if fun(path,res) or mod(path,res) then
            local ret = ('[see %s](%s.%s.html'):format(path,res[1],res[2])
            if res[3] then
                return ret..'#'..res[3]..')'
            else
                return ret..')'
            end
        end
    end)
    print(line)
end








