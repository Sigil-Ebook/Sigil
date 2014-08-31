-- a simple implementation of the which command. This looks for
-- the given file on the path. On windows, it will assume an extension
-- of .exe if no extension is given.
local List = require 'pl.List'
local path = require 'pl.path'
local app = require 'pl.app'

local pathl = List.split(os.getenv 'PATH',path.dirsep)

function which (file)
    local res = pathl:map(path.join,file)
    res = res:filter(path.exists)
    if res then return res[1] end
end

local _,lua = app.lua()
local file = arg[1] or lua -- i.e. location of lua executable
local try

if not file then return print 'must provide a filename' end

if path.extension(file) == '' and path.is_windows then
    try = which(file..'.exe')
else
    try = which(file)
end

if try then print(try) else print 'cannot find on path' end


