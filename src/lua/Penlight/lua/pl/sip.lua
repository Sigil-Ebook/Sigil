--- Simple Input Patterns (SIP).
-- SIP patterns start with '$', then a
-- one-letter type, and then an optional variable in curly braces.
--
--    sip.match('$v=$q','name="dolly"',res)
--    ==> res=={'name','dolly'}
--    sip.match('($q{first},$q{second})','("john","smith")',res)
--    ==> res=={second='smith',first='john'}
--
-- ''Type names''
--
--    v    identifier
--    i     integer
--    f     floating-point
--    q    quoted string
--    ([{<  match up to closing bracket
--
-- See @{08-additional.md.Simple_Input_Patterns|the Guide}
--
-- @module pl.sip

local loadstring = rawget(_G,'loadstring') or load
local unpack = rawget(_G,'unpack') or rawget(table,'unpack')

local append,concat = table.insert,table.concat
local ipairs,loadstring,type,unpack = ipairs,loadstring,type,unpack
local io,_G = io,_G
local print,rawget = print,rawget

local patterns = {
    FLOAT = '[%+%-%d]%d*%.?%d*[eE]?[%+%-]?%d*',
    INTEGER = '[+%-%d]%d*',
    IDEN = '[%a_][%w_]*',
    FILE = '[%a%.\\][:%][%w%._%-\\]*',
    OPTION = '[%a_][%w_%-]*',
}

local function assert_arg(idx,val,tp)
    if type(val) ~= tp then
        error("argument "..idx.." must be "..tp, 2)
    end
end


--[[
module ('pl.sip',utils._module)
]]

local sip = {}

local brackets = {['<'] = '>', ['('] = ')', ['{'] = '}', ['['] = ']' }
local stdclasses = {a=1,c=0,d=1,l=1,p=0,u=1,w=1,x=1,s=0}

local _patterns = {}


local function group(s)
    return '('..s..')'
end

-- escape all magic characters except $, which has special meaning
-- Also, un-escape any characters after $, so $( passes through as is.
local function escape (spec)
    --_G.print('spec',spec)
    local res = spec:gsub('[%-%.%+%[%]%(%)%^%%%?%*]','%%%1'):gsub('%$%%(%S)','$%1')
    --_G.print('res',res)
    return res
end

local function imcompressible (s)
    return s:gsub('%s+','\001')
end

-- [handling of spaces in patterns]
-- spaces may be 'compressed' (i.e will match zero or more spaces)
-- unless this occurs within a number or an identifier. So we mark
-- the four possible imcompressible patterns first and then replace.
-- The possible alnum patterns are v,f,a,d,x,l and u.
local function compress_spaces (s)
    s = s:gsub('%$[vifadxlu]%s+%$[vfadxlu]',imcompressible)
    s = s:gsub('[%w_]%s+[%w_]',imcompressible)
    s = s:gsub('[%w_]%s+%$[vfadxlu]',imcompressible)
    s = s:gsub('%$[vfadxlu]%s+[%w_]',imcompressible)
    s = s:gsub('%s+','%%s*')
    s = s:gsub('\001',' ')
    return s
end

local pattern_map = {
  v = group(patterns.IDEN),
  i = group(patterns.INTEGER),
  f = group(patterns.FLOAT),
  o = group(patterns.OPTION),
  r = '(%S.*)',
  p = '([%a]?[:]?[\\/%.%w_]+)'
}

function sip.custom_pattern(flag,patt)
    pattern_map[flag] = patt
end

--- convert a SIP pattern into the equivalent Lua string pattern.
-- @param spec a SIP pattern
-- @param options a table; only the <code>at_start</code> field is
-- currently meaningful and esures that the pattern is anchored
-- at the start of the string.
-- @return a Lua string pattern.
function sip.create_pattern (spec,options)
    assert_arg(1,spec,'string')
    local fieldnames,fieldtypes = {},{}

    if type(spec) == 'string' then
        spec = escape(spec)
    else
        local res = {}
        for i,s in ipairs(spec) do
            res[i] = escape(s)
        end
        spec = concat(res,'.-')
    end

    local kount = 1

    local function addfield (name,type)
        if not name then name = kount end
        if fieldnames then append(fieldnames,name) end
        if fieldtypes then fieldtypes[name] = type end
        kount = kount + 1
    end

    local named_vars, pattern
    named_vars = spec:find('{%a+}')
    pattern = '%$%S'

    if options and options.at_start then
        spec = '^'..spec
    end
    if spec:sub(-1,-1) == '$' then
        spec = spec:sub(1,-2)..'$r'
        if named_vars then spec = spec..'{rest}' end
    end


    local names

    if named_vars then
        names = {}
        spec = spec:gsub('{(%a+)}',function(name)
            append(names,name)
            return ''
        end)
    end
    spec = compress_spaces(spec)

    local k = 1
    local err
    local r = (spec:gsub(pattern,function(s)
        local type,name
        type = s:sub(2,2)
        if names then name = names[k]; k=k+1 end
        -- this kludge is necessary because %q generates two matches, and
        -- we want to ignore the first. Not a problem for named captures.
        if not names and type == 'q' then
            addfield(nil,'Q')
        else
            addfield(name,type)
        end
        local res
        if pattern_map[type] then
            res = pattern_map[type]
        elseif type == 'q' then
            -- some Lua pattern matching voodoo; we want to match '...' as
            -- well as "...", and can use the fact that %n will match a
            -- previous capture. Adding the extra field above comes from needing
            -- to accomodate the extra spurious match (which is either ' or ")
            addfield(name,type)
            res = '(["\'])(.-)%'..(kount-2)
        else
            local endbracket = brackets[type]
            if endbracket then
                res = '(%b'..type..endbracket..')'
            elseif stdclasses[type] or stdclasses[type:lower()] then
                res = '(%'..type..'+)'
            else
                err = "unknown format type or character class"
            end
        end
        return res
    end))
    --print(r,err)
    if err then
        return nil,err
    else
        return r,fieldnames,fieldtypes
    end
end


local function tnumber (s)
    return s == 'd' or s == 'i' or s == 'f'
end

function sip.create_spec_fun(spec,options)
    local fieldtypes,fieldnames
    local ls = {}
    spec,fieldnames,fieldtypes = sip.create_pattern(spec,options)
    if not spec then return spec,fieldnames end
    local named_vars = type(fieldnames[1]) == 'string'
    for i = 1,#fieldnames do
        append(ls,'mm'..i)
    end
    local fun = ('return (function(s,res)\n\tlocal %s = s:match(%q)\n'):format(concat(ls,','),spec)
    fun = fun..'\tif not mm1 then return false end\n'
    local k=1
    for i,f in ipairs(fieldnames) do
        if f ~= '_' then
            local var = 'mm'..i
            if tnumber(fieldtypes[f]) then
                var = 'tonumber('..var..')'
            elseif brackets[fieldtypes[f]] then
                var = var..':sub(2,-2)'
            end
            if named_vars then
                fun = ('%s\tres.%s = %s\n'):format(fun,f,var)
            else
                if fieldtypes[f] ~= 'Q' then -- we skip the string-delim capture
                    fun = ('%s\tres[%d] = %s\n'):format(fun,k,var)
                    k = k + 1
                end
            end
        end
    end
    return fun..'\treturn true\nend)\n', named_vars
end

--- convert a SIP pattern into a matching function.
-- The returned function takes two arguments, the line and an empty table.
-- If the line matched the pattern, then this function return true
-- and the table is filled with field-value pairs.
-- @param spec a SIP pattern
-- @param options optional table; {anywhere=true} will stop pattern anchoring at start
-- @return a function if successful, or nil,<error>
function sip.compile(spec,options)
    assert_arg(1,spec,'string')
    local fun,names = sip.create_spec_fun(spec,options)
    if not fun then return nil,names end
    if rawget(_G,'_DEBUG') then print(fun) end
    local chunk,err = loadstring(fun,'tmp')
    if err then return nil,err end
    return chunk(),names
end

local cache = {}

--- match a SIP pattern against a string.
-- @param spec a SIP pattern
-- @param line a string
-- @param res a table to receive values
-- @param options (optional) option table
-- @return true or false
function sip.match (spec,line,res,options)
    assert_arg(1,spec,'string')
    assert_arg(2,line,'string')
    assert_arg(3,res,'table')
    if not cache[spec] then
        cache[spec] = sip.compile(spec,options)
    end
    return cache[spec](line,res)
end

--- match a SIP pattern against the start of a string.
-- @param spec a SIP pattern
-- @param line a string
-- @param res a table to receive values
-- @return true or false
function sip.match_at_start (spec,line,res)
    return sip.match(spec,line,res,{at_start=true})
end

--- given a pattern and a file object, return an iterator over the results
-- @param spec a SIP pattern
-- @param f a file - use standard input if not specified.
function sip.fields (spec,f)
    assert_arg(1,spec,'string')
    f = f or io.stdin
    local fun,err = sip.compile(spec)
    if not fun then return nil,err end
    local res = {}
    return function()
        while true do
            local line = f:read()
            if not line then return end
            if fun(line,res) then
                local values = res
                res = {}
                return unpack(values)
            end
        end
    end
end

--- register a match which will be used in the read function.
-- @string spec a SIP pattern
-- @func fun a function to be called with the results of the match
-- @see read
function sip.pattern (spec,fun)
    assert_arg(1,spec,'string')
    local pat,named = sip.compile(spec)
    append(_patterns,{pat=pat,named=named,callback=fun or false})
end

--- enter a loop which applies all registered matches to the input file.
-- @param f a file object; if nil, then io.stdin is assumed.
function sip.read (f)
    local owned,err
    f = f or io.stdin
    if type(f) == 'string' then
        f,err = io.open(f)
        if not f then return nil,err end
        owned = true
    end
    local res = {}
    for line in f:lines() do
        for _,item in ipairs(_patterns) do
            if item.pat(line,res) then
                if item.callback then
                    if item.named then
                        item.callback(res)
                    else
                        item.callback(unpack(res))
                    end
                end
                res = {}
                break
            end
        end
    end
    if owned then f:close() end
end

return sip
