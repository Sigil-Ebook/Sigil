--- Path manipulation and file queries.
--
-- This is modelled after Python's os.path library (10.1); see @{04-paths.md|the Guide}.
--
-- Dependencies: `pl.utils`, `lfs`
-- @module pl.path

-- imports and locals
local _G = _G
local sub = string.sub
local getenv = os.getenv
local tmpnam = os.tmpname
local attributes, currentdir, link_attrib
local package = package
local io = io
local append = table.insert
local ipairs = ipairs
local utils = require 'pl.utils'
local assert_arg,assert_string,raise = utils.assert_arg,utils.assert_string,utils.raise

local attrib
local path = {}

local res,lfs = _G.pcall(_G.require,'lfs')
if res then
    attributes = lfs.attributes
    currentdir = lfs.currentdir
    link_attrib = lfs.symlinkattributes
else
    error("pl.path requires LuaFileSystem")
end

attrib = attributes
path.attrib = attrib
path.link_attrib = link_attrib

--- Lua iterator over the entries of a given directory.
-- Behaves like `lfs.dir`
path.dir = lfs.dir

--- Creates a directory.
path.mkdir = lfs.mkdir

--- Removes a directory.
path.rmdir = lfs.rmdir

---- Get the working directory.
path.currentdir = currentdir

--- Changes the working directory.
path.chdir = lfs.chdir


--- is this a directory?
-- @string P A file path
function path.isdir(P)
	assert_string(1,P)
    if P:match("\\$") then
        P = P:sub(1,-2)
    end
    return attrib(P,'mode') == 'directory'
end

--- is this a file?.
-- @string P A file path
function path.isfile(P)
	assert_string(1,P)
    return attrib(P,'mode') == 'file'
end

-- is this a symbolic link?
-- @string P A file path
function path.islink(P)
	assert_string(1,P)
    if link_attrib then
        return link_attrib(P,'mode')=='link'
    else
        return false
    end
end

--- return size of a file.
-- @string P A file path
function path.getsize(P)
	assert_string(1,P)
    return attrib(P,'size')
end

--- does a path exist?.
-- @string P A file path
-- @return the file path if it exists, nil otherwise
function path.exists(P)
	assert_string(1,P)
    return attrib(P,'mode') ~= nil and P
end

--- Return the time of last access as the number of seconds since the epoch.
-- @string P A file path
function path.getatime(P)
	assert_string(1,P)
    return attrib(P,'access')
end

--- Return the time of last modification
-- @string P A file path
function path.getmtime(P)
    return attrib(P,'modification')
end

---Return the system's ctime.
-- @string P A file path
function path.getctime(P)
	assert_string(1,P)
    return path.attrib(P,'change')
end


local function at(s,i)
    return sub(s,i,i)
end

path.is_windows = utils.dir_separator == '\\'

local other_sep
-- !constant sep is the directory separator for this platform.
if path.is_windows then
    path.sep = '\\'; other_sep = '/'
    path.dirsep = ';'
else
    path.sep = '/'
    path.dirsep = ':'
end
local sep,dirsep = path.sep,path.dirsep

--- are we running Windows?
-- @class field
-- @name path.is_windows

--- path separator for this platform.
-- @class field
-- @name path.sep

--- separator for PATH for this platform
-- @class field
-- @name path.dirsep

--- given a path, return the directory part and a file part.
-- if there's no directory part, the first value will be empty
-- @string P A file path
function path.splitpath(P)
    assert_string(1,P)
    local i = #P
    local ch = at(P,i)
    while i > 0 and ch ~= sep and ch ~= other_sep do
        i = i - 1
        ch = at(P,i)
    end
    if i == 0 then
        return '',P
    else
        return sub(P,1,i-1), sub(P,i+1)
    end
end

--- return an absolute path.
-- @string P A file path
-- @string[opt] pwd optional start path to use (default is current dir)
function path.abspath(P,pwd)
    assert_string(1,P)
	if pwd then assert_string(2,pwd) end
    local use_pwd = pwd ~= nil
    if not use_pwd and not currentdir then return P end
    P = P:gsub('[\\/]$','')
    pwd = pwd or currentdir()
    if not path.isabs(P) then
        P = path.join(pwd,P)
    elseif path.is_windows and not use_pwd and at(P,2) ~= ':' and at(P,2) ~= '\\' then
        P = pwd:sub(1,2)..P -- attach current drive to path like '\\fred.txt'
    end
    return path.normpath(P)
end

--- given a path, return the root part and the extension part.
-- if there's no extension part, the second value will be empty
-- @string P A file path
-- @treturn string root part
-- @treturn string extension part (maybe empty)
function path.splitext(P)
    assert_string(1,P)
    local i = #P
    local ch = at(P,i)
    while i > 0 and ch ~= '.' do
        if ch == sep or ch == other_sep then
            return P,''
        end
        i = i - 1
        ch = at(P,i)
    end
    if i == 0 then
        return P,''
    else
        return sub(P,1,i-1),sub(P,i)
    end
end

--- return the directory part of a path
-- @string P A file path
function path.dirname(P)
    assert_string(1,P)
    local p1,p2 = path.splitpath(P)
    return p1
end

--- return the file part of a path
-- @string P A file path
function path.basename(P)
    assert_string(1,P)
    local p1,p2 = path.splitpath(P)
    return p2
end

--- get the extension part of a path.
-- @string P A file path
function path.extension(P)
    assert_string(1,P)
    local p1,p2 = path.splitext(P)
    return p2
end

--- is this an absolute path?.
-- @string P A file path
function path.isabs(P)
    assert_string(1,P)
    if path.is_windows then
        return at(P,1) == '/' or at(P,1)=='\\' or at(P,2)==':'
    else
        return at(P,1) == '/'
    end
end

--- return the path resulting from combining the individual paths.
-- if the second (or later) path is absolute, we return the last absolute path (joined with any non-absolute paths following).
-- empty elements (except the last) will be ignored.
-- @string p1 A file path
-- @string p2 A file path
-- @string ... more file paths
function path.join(p1,p2,...)
    assert_string(1,p1)
    assert_string(2,p2)
    if select('#',...) > 0 then
        local p = path.join(p1,p2)
        local args = {...}
        for i = 1,#args do
            assert_string(i,args[i])
            p = path.join(p,args[i])
        end
        return p
    end
    if path.isabs(p2) then return p2 end
    local endc = at(p1,#p1)
    if endc ~= path.sep and endc ~= other_sep and endc ~= "" then
        p1 = p1..path.sep
    end
    return p1..p2
end

--- normalize the case of a pathname. On Unix, this returns the path unchanged;
--  for Windows, it converts the path to lowercase, and it also converts forward slashes
-- to backward slashes.
-- @string P A file path
function path.normcase(P)
    assert_string(1,P)
    if path.is_windows then
        return (P:lower():gsub('/','\\'))
    else
        return P
    end
end

local np_gen1,np_gen2 = '[^SEP]+SEP%.%.SEP?','SEP+%.?SEP'
local np_pat1, np_pat2

--- normalize a path name.
--  A//B, A/./B and A/foo/../B all become A/B.
-- @string P a file path
function path.normpath(P)
    assert_string(1,P)
    if path.is_windows then
        if P:match '^\\\\' then -- UNC
            return '\\\\'..path.normpath(P:sub(3))
        end
        P = P:gsub('/','\\')
    end
    if not np_pat1 then
        np_pat1 = np_gen1:gsub('SEP',sep)
        np_pat2 = np_gen2:gsub('SEP',sep)
    end
    local k
    repeat -- /./ -> /
        P,k = P:gsub(np_pat2,sep)
    until k == 0
    repeat -- A/../ -> (empty)
        P,k = P:gsub(np_pat1,'')
    until k == 0
    if P == '' then P = '.' end
    return P
end

local function ATS (P)
    if at(P,#P) ~= path.sep then
        P = P..path.sep
    end
    return path.normcase(P)
end

--- relative path from current directory or optional start point
-- @string P a path
-- @string[opt] start optional start point (default current directory)
function path.relpath (P,start)
    assert_string(1,P)
	if start then assert_string(2,start) end
    local split,normcase,min,append = utils.split, path.normcase, math.min, table.insert
    P = normcase(path.abspath(P,start))
    start = start or currentdir()
    start = normcase(start)
    local startl, Pl = split(start,sep), split(P,sep)
    local n = min(#startl,#Pl)
    local k = n+1 -- default value if this loop doesn't bail out!
    for i = 1,n do
        if startl[i] ~= Pl[i] then
            k = i
            break
        end
    end
    local rell = {}
    for i = 1, #startl-k+1 do rell[i] = '..' end
    if k <= #Pl then
        for i = k,#Pl do append(rell,Pl[i]) end
    end
    return table.concat(rell,sep)
end


--- Replace a starting '~' with the user's home directory.
-- In windows, if HOME isn't set, then USERPROFILE is used in preference to
-- HOMEDRIVE HOMEPATH. This is guaranteed to be writeable on all versions of Windows.
-- @string P A file path
function path.expanduser(P)
    assert_string(1,P)
    if at(P,1) == '~' then
        local home = getenv('HOME')
        if not home then -- has to be Windows
            home = getenv 'USERPROFILE' or (getenv 'HOMEDRIVE' .. getenv 'HOMEPATH')
        end
        return home..sub(P,2)
    else
        return P
    end
end


---Return a suitable full path to a new temporary file name.
-- unlike os.tmpnam(), it always gives you a writeable path (uses TEMP environment variable on Windows)
function path.tmpname ()
    local res = tmpnam()
    if path.is_windows then res = getenv('TEMP')..res end
    return res
end

--- return the largest common prefix path of two paths.
-- @string path1 a file path
-- @string path2 a file path
function path.common_prefix (path1,path2)
    assert_string(1,path1)
    assert_string(2,path2)
    path1, path2 = path.normcase(path1), path.normcase(path2)
    -- get them in order!
    if #path1 > #path2 then path2,path1 = path1,path2 end
    for i = 1,#path1 do
        local c1 = at(path1,i)
        if c1 ~= at(path2,i) then
            local cp = path1:sub(1,i-1)
            if at(path1,i-1) ~= sep then
                cp = path.dirname(cp)
            end
            return cp
        end
    end
    if at(path2,#path1+1) ~= sep then path1 = path.dirname(path1) end
    return path1
    --return ''
end

--- return the full path where a particular Lua module would be found.
-- Both package.path and package.cpath is searched, so the result may
-- either be a Lua file or a shared library.
-- @string mod name of the module
-- @return on success: path of module, lua or binary
-- @return on error: nil,error string
function path.package_path(mod)
    assert_string(1,mod)
    local res
    mod = mod:gsub('%.',sep)
    res = package.searchpath(mod,package.path)
    if res then return res,true end
    res = package.searchpath(mod,package.cpath)
    if res then return res,false end
    return raise 'cannot find module on path'
end


---- finis -----
return path
