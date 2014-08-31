--- Pretty-printing Lua tables.
-- Also provides a sandboxed Lua table reader and
-- a function to present large numbers in human-friendly format.
--
-- Dependencies: `pl.utils`, `pl.lexer`
-- @module pl.pretty

local append = table.insert
local concat = table.concat
local utils = require 'pl.utils'
local lexer = require 'pl.lexer'
local assert_arg = utils.assert_arg

local pretty = {}

local function save_string_index ()
    local SMT = getmetatable ''
    if SMT then
        SMT.old__index = SMT.__index
        SMT.__index = nil
    end
    return SMT
end

local function restore_string_index (SMT)
    if SMT then
        SMT.__index = SMT.old__index
    end
end

--- read a string representation of a Lua table.
-- Uses load(), but tries to be cautious about loading arbitrary code!
-- It is expecting a string of the form '{...}', with perhaps some whitespace
-- before or after the curly braces. A comment may occur beforehand.
-- An empty environment is used, and
-- any occurance of the keyword 'function' will be considered a problem.
-- in the given environment - the return value may be `nil`.
-- @param s {string} string of the form '{...}', with perhaps some whitespace
-- before or after the curly braces.
-- @return a table
function pretty.read(s)
    assert_arg(1,s,'string')
    if s:find '^%s*%-%-' then -- may start with a comment..
        s = s:gsub('%-%-.-\n','')
    end
    if not s:find '^%s*%b{}%s*$' then return nil,"not a Lua table" end
    if s:find '[^\'"%w_]function[^\'"%w_]' then
        local tok = lexer.lua(s)
        for t,v in tok do
            if t == 'keyword' then
                return nil,"cannot have functions in table definition"
            end
        end
    end
    s = 'return '..s
    local chunk,err = utils.load(s,'tbl','t',{})
    if not chunk then return nil,err end
    local SMT = save_string_index()
    local ok,ret = pcall(chunk)
    restore_string_index(SMT)
    if ok then return ret
    else
        return nil,ret
    end
end

--- read a Lua chunk.
-- @param s Lua code
-- @param env optional environment
-- @param paranoid prevent any looping constructs and disable string methods
-- @return the environment
function pretty.load (s, env, paranoid)
    env = env or {}
    if paranoid then
        local tok = lexer.lua(s)
        for t,v in tok do
            if t == 'keyword'
                and (v == 'for' or v == 'repeat' or v == 'function' or v == 'goto')
            then
                return nil,"looping not allowed"
            end
        end
    end
    local chunk,err = utils.load(s,'tbl','t',env)
    if not chunk then return nil,err end
    local SMT = paranoid and save_string_index()
    local ok,err = pcall(chunk)
    restore_string_index(SMT)
    if not ok then return nil,err end
    return env
end

local function quote_if_necessary (v)
    if not v then return ''
    else
        if v:find ' ' then v = '"'..v..'"' end
    end
    return v
end

local keywords

local function is_identifier (s)
    return type(s) == 'string' and s:find('^[%a_][%w_]*$') and not keywords[s]
end

local function quote (s)
    if type(s) == 'table' then
        return pretty.write(s,'')
    else
        return ('%q'):format(tostring(s))
    end
end

local function index (numkey,key)
    if not numkey then key = quote(key) end
    return '['..key..']'
end


---	Create a string representation of a Lua table.
--  This function never fails, but may complain by returning an
--  extra value. Normally puts out one item per line, using
--  the provided indent; set the second parameter to '' if
--  you want output on one line.
--	@param tbl {table} Table to serialize to a string.
--	@param space {string} (optional) The indent to use.
--	Defaults to two spaces; make it the empty string for no indentation
--	@param not_clever {bool} (optional) Use for plain output, e.g {['key']=1}.
--	Defaults to false.
--  @return a string
--  @return a possible error message
function pretty.write (tbl,space,not_clever)
    if type(tbl) ~= 'table' then
        local res = tostring(tbl)
        if type(tbl) == 'string' then return quote(tbl) end
        return res, 'not a table'
    end
    if not keywords then
        keywords = lexer.get_keywords()
    end
    local set = ' = '
    if space == '' then set = '=' end
    space = space or '  '
    local lines = {}
    local line = ''
    local tables = {}


    local function put(s)
        if #s > 0 then
            line = line..s
        end
    end

    local function putln (s)
        if #line > 0 then
            line = line..s
            append(lines,line)
            line = ''
        else
            append(lines,s)
        end
    end

    local function eat_last_comma ()
        local n,lastch = #lines
        local lastch = lines[n]:sub(-1,-1)
        if lastch == ',' then
            lines[n] = lines[n]:sub(1,-2)
        end
    end


    local writeit
    writeit = function (t,oldindent,indent)
        local tp = type(t)
        if tp ~= 'string' and  tp ~= 'table' then
            putln(quote_if_necessary(tostring(t))..',')
        elseif tp == 'string' then
            if t:find('\n') then
                putln('[[\n'..t..']],')
            else
                putln(quote(t)..',')
            end
        elseif tp == 'table' then
            if tables[t] then
                putln('<cycle>,')
                return
            end
            tables[t] = true
            local newindent = indent..space
            putln('{')
            local used = {}
            if not not_clever then
                for i,val in ipairs(t) do
                    put(indent)
                    writeit(val,indent,newindent)
                    used[i] = true
                end
            end
            for key,val in pairs(t) do
                local numkey = type(key) == 'number'
                if not_clever then
                    key = tostring(key)
                    put(indent..index(numkey,key)..set)
                    writeit(val,indent,newindent)
                else
                    if not numkey or not used[key] then -- non-array indices
                        if numkey or not is_identifier(key) then
                            key = index(numkey,key)
                        end
                        put(indent..key..set)
                        writeit(val,indent,newindent)
                    end
                end
            end
            tables[t] = nil
            eat_last_comma()
            putln(oldindent..'},')
        else
            putln(tostring(t)..',')
        end
    end
    writeit(tbl,'',space)
    eat_last_comma()
    return concat(lines,#space > 0 and '\n' or '')
end

---	Dump a Lua table out to a file or stdout.
--	@param t {table} The table to write to a file or stdout.
--	@param ... {string} (optional) File name to write too. Defaults to writing
--	to stdout.
function pretty.dump (t,...)
    if select('#',...)==0 then
        print(pretty.write(t))
        return true
    else
        return utils.writefile(...,pretty.write(t))
    end
end

local memp,nump = {'B','KiB','MiB','GiB'},{'','K','M','B'}

local comma
function comma (val)
    local thou = math.floor(val/1000)
    if thou > 0 then return comma(thou)..','..(val % 1000)
    else return tostring(val) end
end

--- format large numbers nicely for human consumption.
-- @param num a number
-- @param kind one of 'M' (memory in KiB etc), 'N' (postfixes are 'K','M' and 'B')
-- and 'T' (use commas as thousands separator)
-- @param prec number of digits to use for 'M' and 'N' (default 1)
function pretty.number (num,kind,prec)
    local fmt = '%.'..(prec or 1)..'f%s'
    if kind == 'T' then
        return comma(num)
    else
        local postfixes, fact
        if kind == 'M' then
            fact = 1024
            postfixes = memp
        else
            fact = 1000
            postfixes = nump
        end
        local div = fact
        local k = 1
        while num >= div and k <= #postfixes do
            div = div * fact
            k = k + 1
        end
        div = div / fact
        if k > #postfixes then k = k - 1; div = div/fact end
        if k > 1 then
            return fmt:format(num/div,postfixes[k] or 'duh')
        else
            return num..postfixes[1]
        end
    end
end

return pretty
