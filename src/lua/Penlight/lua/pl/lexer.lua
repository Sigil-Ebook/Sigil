--- Lexical scanner for creating a sequence of tokens from text.
-- `lexer.scan(s)` returns an iterator over all tokens found in the
-- string `s`. This iterator returns two values, a token type string
-- (such as 'string' for quoted string, 'iden' for identifier) and the value of the
-- token.
--
-- Versions specialized for Lua and C are available; these also handle block comments
-- and classify keywords as 'keyword' tokens. For example:
--
--    > s = 'for i=1,n do'
--    > for t,v in lexer.lua(s)  do print(t,v) end
--    keyword for
--    iden    i
--    =       =
--    number  1
--    ,       ,
--    iden    n
--    keyword do
--
-- See the Guide for further @{06-data.md.Lexical_Scanning|discussion}
-- @module pl.lexer

local yield,wrap = coroutine.yield,coroutine.wrap
local strfind = string.find
local strsub = string.sub
local append = table.insert

local function assert_arg(idx,val,tp)
    if type(val) ~= tp then
        error("argument "..idx.." must be "..tp, 2)
    end
end

local lexer = {}

local NUMBER1 = '^[%+%-]?%d+%.?%d*[eE][%+%-]?%d+'
local NUMBER2 = '^[%+%-]?%d+%.?%d*'
local NUMBER3 = '^0x[%da-fA-F]+'
local NUMBER4 = '^%d+%.?%d*[eE][%+%-]?%d+'
local NUMBER5 = '^%d+%.?%d*'
local IDEN = '^[%a_][%w_]*'
local WSPACE = '^%s+'
local STRING0 = [[^(['\"]).-\\%1]]
local STRING1 = [[^(['\"]).-[^\]%1]]
local STRING3 = "^((['\"])%2)" -- empty string
local PREPRO = '^#.-[^\\]\n'

local plain_matches,lua_matches,cpp_matches,lua_keyword,cpp_keyword

local function tdump(tok)
    return yield(tok,tok)
end

local function ndump(tok,options)
    if options and options.number then
        tok = tonumber(tok)
    end
    return yield("number",tok)
end

-- regular strings, single or double quotes; usually we want them
-- without the quotes
local function sdump(tok,options)
    if options and options.string then
        tok = tok:sub(2,-2)
    end
    return yield("string",tok)
end

-- long Lua strings need extra work to get rid of the quotes
local function sdump_l(tok,options)
    if options and options.string then
        tok = tok:sub(3,-3)
    end
    return yield("string",tok)
end

local function chdump(tok,options)
    if options and options.string then
        tok = tok:sub(2,-2)
    end
    return yield("char",tok)
end

local function cdump(tok)
    return yield('comment',tok)
end

local function wsdump (tok)
    return yield("space",tok)
end

local function pdump (tok)
    return yield('prepro',tok)
end

local function plain_vdump(tok)
    return yield("iden",tok)
end

local function lua_vdump(tok)
    if lua_keyword[tok] then
        return yield("keyword",tok)
    else
        return yield("iden",tok)
    end
end

local function cpp_vdump(tok)
    if cpp_keyword[tok] then
        return yield("keyword",tok)
    else
        return yield("iden",tok)
    end
end

--- create a plain token iterator from a string or file-like object.
-- @string s the string
-- @tab matches an optional match table (set of pattern-action pairs)
-- @tab[opt] filter a table of token types to exclude, by default `{space=true}`
-- @tab[opt] options a table of options; by default, `{number=true,string=true}`,
-- which means convert numbers and strip string quotes.
function lexer.scan (s,matches,filter,options)
    --assert_arg(1,s,'string')
    local file = type(s) ~= 'string' and s
    filter = filter or {space=true}
    options = options or {number=true,string=true}
    if filter then
        if filter.space then filter[wsdump] = true end
        if filter.comments then
            filter[cdump] = true
        end
    end
    if not matches then
        if not plain_matches then
            plain_matches = {
                {WSPACE,wsdump},
                {NUMBER3,ndump},
                {IDEN,plain_vdump},
                {NUMBER1,ndump},
                {NUMBER2,ndump},
                {STRING3,sdump},
                {STRING0,sdump},
                {STRING1,sdump},
                {'^.',tdump}
            }
        end
        matches = plain_matches
    end
    local function lex ()
        local i1,i2,idx,res1,res2,tok,pat,fun,capt
        local line = 1
        if file then s = file:read()..'\n' end
        local sz = #s
        local idx = 1
        --print('sz',sz)
        while true do
            for _,m in ipairs(matches) do
                pat = m[1]
                fun = m[2]
                i1,i2 = strfind(s,pat,idx)
                if i1 then
                    tok = strsub(s,i1,i2)
                    idx = i2 + 1
                    if not (filter and filter[fun]) then
                        lexer.finished = idx > sz
                        res1,res2 = fun(tok,options)
                    end
                    if res1 then
                        local tp = type(res1)
                        -- insert a token list
                        if tp=='table' then
                            yield('','')
                            for _,t in ipairs(res1) do
                                yield(t[1],t[2])
                            end
                        elseif tp == 'string' then -- or search up to some special pattern
                            i1,i2 = strfind(s,res1,idx)
                            if i1 then
                                tok = strsub(s,i1,i2)
                                idx = i2 + 1
                                yield('',tok)
                            else
                                yield('','')
                                idx = sz + 1
                            end
                            --if idx > sz then return end
                        else
                            yield(line,idx)
                        end
                    end
                    if idx > sz then
                        if file then
                            --repeat -- next non-empty line
                                line = line + 1
                                s = file:read()
                                if not s then return end
                            --until not s:match '^%s*$'
                            s = s .. '\n'
                            idx ,sz = 1,#s
                            break
                        else
                            return
                        end
                    else break end
                end
            end
        end
    end
    return wrap(lex)
end

local function isstring (s)
    return type(s) == 'string'
end

--- insert tokens into a stream.
-- @param tok a token stream
-- @param a1 a string is the type, a table is a token list and
-- a function is assumed to be a token-like iterator (returns type & value)
-- @string a2 a string is the value
function lexer.insert (tok,a1,a2)
    if not a1 then return end
    local ts
    if isstring(a1) and isstring(a2) then
        ts = {{a1,a2}}
    elseif type(a1) == 'function' then
        ts = {}
        for t,v in a1() do
            append(ts,{t,v})
        end
    else
        ts = a1
    end
    tok(ts)
end

--- get everything in a stream upto a newline.
-- @param tok a token stream
-- @return a string
function lexer.getline (tok)
    local t,v = tok('.-\n')
    return v
end

--- get current line number.
-- Only available if the input source is a file-like object.
-- @param tok a token stream
-- @return the line number and current column
function lexer.lineno (tok)
    return tok(0)
end

--- get the rest of the stream.
-- @param tok a token stream
-- @return a string
function lexer.getrest (tok)
    local t,v = tok('.+')
    return v
end

--- get the Lua keywords as a set-like table.
-- So `res["and"]` etc would be `true`.
-- @return a table
function lexer.get_keywords ()
    if not lua_keyword then
        lua_keyword = {
            ["and"] = true, ["break"] = true,  ["do"] = true,
            ["else"] = true, ["elseif"] = true, ["end"] = true,
            ["false"] = true, ["for"] = true, ["function"] = true,
            ["if"] = true, ["in"] = true,  ["local"] = true, ["nil"] = true,
            ["not"] = true, ["or"] = true, ["repeat"] = true,
            ["return"] = true, ["then"] = true, ["true"] = true,
            ["until"] = true,  ["while"] = true
        }
    end
    return lua_keyword
end

--- create a Lua token iterator from a string or file-like object.
-- Will return the token type and value.
-- @string s the string
-- @tab[opt] filter a table of token types to exclude, by default `{space=true,comments=true}`
-- @tab[opt] options a table of options; by default, `{number=true,string=true}`,
-- which means convert numbers and strip string quotes.
function lexer.lua(s,filter,options)
    filter = filter or {space=true,comments=true}
    lexer.get_keywords()
    if not lua_matches then
        lua_matches = {
            {WSPACE,wsdump},
            {NUMBER3,ndump},
            {IDEN,lua_vdump},
            {NUMBER4,ndump},
            {NUMBER5,ndump},
            {STRING3,sdump},
            {STRING0,sdump},
            {STRING1,sdump},
            {'^%-%-%[%[.-%]%]',cdump},
            {'^%-%-.-\n',cdump},
            {'^%[%[.-%]%]',sdump_l},
            {'^==',tdump},
            {'^~=',tdump},
            {'^<=',tdump},
            {'^>=',tdump},
            {'^%.%.%.',tdump},
            {'^%.%.',tdump},
            {'^.',tdump}
        }
    end
    return lexer.scan(s,lua_matches,filter,options)
end

--- create a C/C++ token iterator from a string or file-like object.
-- Will return the token type type and value.
-- @string s the string
-- @tab[opt] filter a table of token types to exclude, by default `{space=true,comments=true}`
-- @tab[opt] options a table of options; by default, `{number=true,string=true}`,
-- which means convert numbers and strip string quotes.
function lexer.cpp(s,filter,options)
    filter = filter or {comments=true}
    if not cpp_keyword then
        cpp_keyword = {
            ["class"] = true, ["break"] = true,  ["do"] = true, ["sizeof"] = true,
            ["else"] = true, ["continue"] = true, ["struct"] = true,
            ["false"] = true, ["for"] = true, ["public"] = true, ["void"] = true,
            ["private"] = true, ["protected"] = true, ["goto"] = true,
            ["if"] = true, ["static"] = true,  ["const"] = true, ["typedef"] = true,
            ["enum"] = true, ["char"] = true, ["int"] = true, ["bool"] = true,
            ["long"] = true, ["float"] = true, ["true"] = true, ["delete"] = true,
            ["double"] = true,  ["while"] = true, ["new"] = true,
            ["namespace"] = true, ["try"] = true, ["catch"] = true,
            ["switch"] = true, ["case"] = true, ["extern"] = true,
            ["return"] = true,["default"] = true,['unsigned']  = true,['signed'] = true,
            ["union"] =  true, ["volatile"] = true, ["register"] = true,["short"] = true,
        }
    end
    if not cpp_matches then
        cpp_matches = {
            {WSPACE,wsdump},
            {PREPRO,pdump},
            {NUMBER3,ndump},
            {IDEN,cpp_vdump},
            {NUMBER4,ndump},
            {NUMBER5,ndump},
            {STRING3,sdump},
            {STRING1,chdump},
            {'^//.-\n',cdump},
            {'^/%*.-%*/',cdump},
            {'^==',tdump},
            {'^!=',tdump},
            {'^<=',tdump},
            {'^>=',tdump},
            {'^->',tdump},
            {'^&&',tdump},
            {'^||',tdump},
            {'^%+%+',tdump},
            {'^%-%-',tdump},
            {'^%+=',tdump},
            {'^%-=',tdump},
            {'^%*=',tdump},
            {'^/=',tdump},
            {'^|=',tdump},
            {'^%^=',tdump},
            {'^::',tdump},
            {'^.',tdump}
        }
    end
    return lexer.scan(s,cpp_matches,filter,options)
end

--- get a list of parameters separated by a delimiter from a stream.
-- @param tok the token stream
-- @string[opt=')'] endtoken end of list. Can be '\n'
-- @string[opt=','] delim separator
-- @return a list of token lists.
function lexer.get_separated_list(tok,endtoken,delim)
    endtoken = endtoken or ')'
    delim = delim or ','
    local parm_values = {}
    local level = 1 -- used to count ( and )
    local tl = {}
    local function tappend (tl,t,val)
        val = val or t
        append(tl,{t,val})
    end
    local is_end
    if endtoken == '\n' then
        is_end = function(t,val)
            return t == 'space' and val:find '\n'
        end
    else
        is_end = function (t)
            return t == endtoken
        end
    end
    local token,value
    while true do
        token,value=tok()
        if not token then return nil,'EOS' end -- end of stream is an error!
        if is_end(token,value) and level == 1 then
            append(parm_values,tl)
            break
        elseif token == '(' then
            level = level + 1
            tappend(tl,'(')
        elseif token == ')' then
            level = level - 1
            if level == 0 then -- finished with parm list
                append(parm_values,tl)
                break
            else
                tappend(tl,')')
            end
        elseif token == delim and level == 1 then
            append(parm_values,tl) -- a new parm
            tl = {}
        else
            tappend(tl,token,value)
        end
    end
    return parm_values,{token,value}
end

--- get the next non-space token from the stream.
-- @param tok the token stream.
function lexer.skipws (tok)
    local t,v = tok()
    while t == 'space' do
        t,v = tok()
    end
    return t,v
end

local skipws = lexer.skipws

--- get the next token, which must be of the expected type.
-- Throws an error if this type does not match!
-- @param tok the token stream
-- @string expected_type the token type
-- @bool no_skip_ws whether we should skip whitespace
function lexer.expecting (tok,expected_type,no_skip_ws)
    assert_arg(1,tok,'function')
    assert_arg(2,expected_type,'string')
    local t,v
    if no_skip_ws then
        t,v = tok()
    else
        t,v = skipws(tok)
    end
    if t ~= expected_type then error ("expecting "..expected_type,2) end
    return v
end

return lexer
