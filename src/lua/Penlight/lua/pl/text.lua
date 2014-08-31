--- Text processing utilities.
--
-- This provides a Template class (modeled after the same from the Python
-- libraries, see string.Template). It also provides similar functions to those
-- found in the textwrap module.
--
-- See  @{03-strings.md.String_Templates|the Guide}.
--
-- Calling `text.format_operator()` overloads the % operator for strings to give Python/Ruby style formated output.
-- This is extended to also do template-like substitution for map-like data.
--
--    > require 'pl.text'.format_operator()
--    > = '%s = %5.3f' % {'PI',math.pi}
--    PI = 3.142
--    > = '$name = $value' % {name='dog',value='Pluto'}
--    dog = Pluto
--
-- Dependencies: `pl.utils`, `pl.types`
-- @module pl.text

local gsub = string.gsub
local concat,append = table.concat,table.insert
local utils = require 'pl.utils'
local bind1,usplit,assert_arg = utils.bind1,utils.split,utils.assert_arg
local is_callable = require 'pl.types'.is_callable
local unpack = utils.unpack

local function lstrip(str)  return (str:gsub('^%s+',''))  end
local function strip(str)  return (lstrip(str):gsub('%s+$','')) end
local function make_list(l)  return setmetatable(l,utils.stdmt.List) end
local function split(s,delim)  return make_list(usplit(s,delim)) end

local function imap(f,t,...)
    local res = {}
    for i = 1,#t do res[i] = f(t[i],...) end
    return res
end

--[[
module ('pl.text',utils._module)
]]

local text = {}

local function _indent (s,sp)
    local sl = split(s,'\n')
    return concat(imap(bind1('..',sp),sl),'\n')..'\n'
end

--- indent a multiline string.
-- @param s the string
-- @param n the size of the indent
-- @param ch the character to use when indenting (default ' ')
-- @return indented string
function text.indent (s,n,ch)
    assert_arg(1,s,'string')
    assert_arg(2,n,'number')
    return _indent(s,string.rep(ch or ' ',n))
end

--- dedent a multiline string by removing any initial indent.
-- useful when working with [[..]] strings.
-- @param s the string
-- @return a string with initial indent zero.
function text.dedent (s)
    assert_arg(1,s,'string')
    local sl = split(s,'\n')
    local i1,i2 = sl[1]:find('^%s*')
    sl = imap(string.sub,sl,i2+1)
    return concat(sl,'\n')..'\n'
end

--- format a paragraph into lines so that they fit into a line width.
-- It will not break long words, so lines can be over the length
-- to that extent.
-- @param s the string
-- @param width the margin width, default 70
-- @return a list of lines
function text.wrap (s,width)
    assert_arg(1,s,'string')
    width = width or 70
    s = s:gsub('\n',' ')
    local i,nxt = 1
    local lines,line = {}
    while i < #s do
        nxt = i+width
        if s:find("[%w']",nxt) then -- inside a word
            nxt = s:find('%W',nxt+1) -- so find word boundary
        end
        line = s:sub(i,nxt)
        i = i + #line
        append(lines,strip(line))
    end
    return make_list(lines)
end

--- format a paragraph so that it fits into a line width.
-- @param s the string
-- @param width the margin width, default 70
-- @return a string
-- @see wrap
function text.fill (s,width)
    return concat(text.wrap(s,width),'\n') .. '\n'
end

local Template = {}
text.Template = Template
Template.__index = Template
setmetatable(Template, {
    __call = function(obj,tmpl)
        return Template.new(tmpl)
    end})

function Template.new(tmpl)
    assert_arg(1,tmpl,'string')
    local res = {}
    res.tmpl = tmpl
    setmetatable(res,Template)
    return res
end

local function _substitute(s,tbl,safe)
    local subst
    if is_callable(tbl) then
        subst = tbl
    else
        function subst(f)
            local s = tbl[f]
            if not s then
                if safe then
                    return f
                else
                    error("not present in table "..f)
                end
            else
                return s
            end
        end
    end
    local res = gsub(s,'%${([%w_]+)}',subst)
    return (gsub(res,'%$([%w_]+)',subst))
end

--- substitute values into a template, throwing an error.
-- This will throw an error if no name is found.
-- @param tbl a table of name-value pairs.
function Template:substitute(tbl)
    assert_arg(1,tbl,'table')
    return _substitute(self.tmpl,tbl,false)
end

--- substitute values into a template.
-- This version just passes unknown names through.
-- @param tbl a table of name-value pairs.
function Template:safe_substitute(tbl)
    assert_arg(1,tbl,'table')
    return _substitute(self.tmpl,tbl,true)
end

--- substitute values into a template, preserving indentation. <br>
-- If the value is a multiline string _or_ a template, it will insert
-- the lines at the correct indentation. <br>
-- Furthermore, if a template, then that template will be subsituted
-- using the same table.
-- @param tbl a table of name-value pairs.
function Template:indent_substitute(tbl)
    assert_arg(1,tbl,'table')
    if not self.strings then
        self.strings = split(self.tmpl,'\n')
    end
    -- the idea is to substitute line by line, grabbing any spaces as
    -- well as the $var. If the value to be substituted contains newlines,
    -- then we split that into lines and adjust the indent before inserting.
    local function subst(line)
        return line:gsub('(%s*)%$([%w_]+)',function(sp,f)
			local subtmpl
            local s = tbl[f]
            if not s then error("not present in table "..f) end
			if getmetatable(s) == Template then
				subtmpl = s
				s = s.tmpl
			else
				s = tostring(s)
			end
            if s:find '\n' then
                s = _indent(s,sp)
            end
			if subtmpl then return _substitute(s,tbl)
			else return s
			end
        end)
    end
    local lines = imap(subst,self.strings)
    return concat(lines,'\n')..'\n'
end

------- Python-style formatting operator ------
-- (see <a href="http://lua-users.org/wiki/StringInterpolation">the lua-users wiki</a>) --

function text.format_operator()

    local format = string.format

    -- a more forgiving version of string.format, which applies
    -- tostring() to any value with a %s format.
    local function formatx (fmt,...)
        local args = {...}
        local i = 1
        for p in fmt:gmatch('%%.') do
            if p == '%s' and type(args[i]) ~= 'string' then
                args[i] = tostring(args[i])
            end
            i = i + 1
        end
        return format(fmt,unpack(args))
    end

    local function basic_subst(s,t)
        return (s:gsub('%$([%w_]+)',t))
    end

    -- Note this goes further than the original, and will allow these cases:
    -- 1. a single value
    -- 2. a list of values
    -- 3. a map of var=value pairs
    -- 4. a function, as in gsub
    -- For the second two cases, it uses $-variable substituion.
    getmetatable("").__mod = function(a, b)
        if b == nil then
            return a
        elseif type(b) == "table" and getmetatable(b) == nil then
            if #b == 0 then -- assume a map-like table
                return _substitute(a,b,true)
            else
                return formatx(a,unpack(b))
            end
        elseif type(b) == 'function' then
            return basic_subst(a,b)
        else
            return formatx(a,b)
        end
    end
end

return text
