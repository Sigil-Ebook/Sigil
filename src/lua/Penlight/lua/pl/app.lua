--- Application support functions.
-- See @{01-introduction.md.Application_Support|the Guide}
--
-- Dependencies: `pl.utils`, `pl.path`
-- @module pl.app

local io,package,require = _G.io, _G.package, _G.require
local utils = require 'pl.utils'
local path = require 'pl.path'

local app = {}

local function check_script_name ()
    if _G.arg == nil then error('no command line args available\nWas this run from a main script?') end
    return _G.arg[0]
end

--- add the current script's path to the Lua module path.
-- Applies to both the source and the binary module paths. It makes it easy for
-- the main file of a multi-file program to access its modules in the same directory.
-- `base` allows these modules to be put in a specified subdirectory, to allow for
-- cleaner deployment and resolve potential conflicts between a script name and its
-- library directory.
-- @string base optional base directory.
-- @treturn string the current script's path with a trailing slash
function app.require_here (base)
    local p = path.dirname(check_script_name())
    if not path.isabs(p) then
        p = path.join(path.currentdir(),p)
    end
    if p:sub(-1,-1) ~= path.sep then
        p = p..path.sep
    end
    if base then
        p = p..base..path.sep
    end
    local so_ext = path.is_windows and 'dll' or 'so'
    local lsep = package.path:find '^;' and '' or ';'
    local csep = package.cpath:find '^;' and '' or ';'
    package.path = ('%s?.lua;%s?%sinit.lua%s%s'):format(p,p,path.sep,lsep,package.path)
    package.cpath = ('%s?.%s%s%s'):format(p,so_ext,csep,package.cpath)
    return p
end

--- return a suitable path for files private to this application.
-- These will look like '~/.SNAME/file', with '~' as with expanduser and
-- SNAME is the name of the script without .lua extension.
-- @string file a filename (w/out path)
-- @return a full pathname, or nil
-- @return 'cannot create' error
function app.appfile (file)
    local sname = path.basename(check_script_name())
    local name,ext = path.splitext(sname)
    local dir = path.join(path.expanduser('~'),'.'..name)
    if not path.isdir(dir) then
        local ret = path.mkdir(dir)
        if not ret then return utils.raise ('cannot create '..dir) end
    end
    return path.join(dir,file)
end

--- return string indicating operating system.
-- @return 'Windows','OSX' or whatever uname returns (e.g. 'Linux')
function app.platform()
    if path.is_windows then
        return 'Windows'
    else
        local f = io.popen('uname')
        local res = f:read()
        if res == 'Darwin' then res = 'OSX' end
        f:close()
        return res
    end
end

--- return the full command-line used to invoke this script.
-- Any extra flags occupy slots, so that `lua -lpl` gives us `{[-2]='lua',[-1]='-lpl'}`
-- @return command-line
-- @return name of Lua program used
function app.lua ()
    local args = _G.arg or error "not in a main program"
    local imin = 0
    for i in pairs(args) do
        if i < imin then imin = i end
    end
    local cmd, append = {}, table.insert
    for i = imin,-1 do
        local a = args[i]
        if a:match '%s' then
            a = '"'..a..'"'
        end
        append(cmd,a)
    end
    return table.concat(cmd,' '),args[imin]
end

--- parse command-line arguments into flags and parameters.
-- Understands GNU-style command-line flags; short (`-f`) and long (`--flag`).
-- These may be given a value with either '=' or ':' (`-k:2`,`--alpha=3.2`,`-n2`);
-- note that a number value can be given without a space.
-- Multiple short args can be combined like so: ( `-abcd`).
-- @tparam {string} args an array of strings (default is the global `arg`)
-- @tab flags_with_values any flags that take values, e.g. `{out=true}`
-- @return a table of flags (flag=value pairs)
-- @return an array of parameters
-- @raise if args is nil, then the global `args` must be available!
function app.parse_args (args,flags_with_values)
    if not args then
        args = _G.arg
        if not args then error "Not in a main program: 'arg' not found" end
    end
    flags_with_values = flags_with_values or {}
    local _args = {}
    local flags = {}
    local i = 1
    while i <= #args do
        local a = args[i]
        local v = a:match('^-(.+)')
        local is_long
        if v then -- we have a flag
            if v:find '^-' then
                is_long = true
                v = v:sub(2)
            end
            if flags_with_values[v] then
                if i == #_args or args[i+1]:find '^-' then
                    return utils.raise ("no value for '"..v.."'")
                end
                flags[v] = args[i+1]
                i = i + 1
            else
                -- a value can also be indicated with =
                local var,val =  utils.splitv (v,'=')
                var = var or v
                val = val or true
                if not is_long then
                    if #var > 1 then
                        if var:find '.%d+' then -- short flag, number value
                            val = var:sub(2)
                            var = var:sub(1,1)
                        else -- multiple short flags
                            for i = 1,#var do
                                flags[var:sub(i,i)] = true
                            end
                            val = nil -- prevents use of var as a flag below
                        end
                    else  -- single short flag (can have value, defaults to true)
                        val = val or true
                    end
                end
                if val then
                    flags[var] = val
                end
            end
        else
            _args[#_args+1] = a
        end
        i = i + 1
    end
    return flags,_args
end

return app
