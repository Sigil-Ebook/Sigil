--- File manipulation functions: reading, writing, moving and copying.
--
-- Dependencies: `pl.utils`, `pl.dir`, `pl.path`
-- @module pl.file
local os = os
local utils = require 'pl.utils'
local dir = require 'pl.dir'
local path = require 'pl.path'

--[[
module ('pl.file',utils._module)
]]
local file = {}

--- return the contents of a file as a string
-- @function file.read
-- @string filename The file path
-- @return file contents
file.read = utils.readfile

--- write a string to a file
-- @function file.write
-- @string filename The file path
-- @string str The string
file.write = utils.writefile

--- copy a file.
-- @function file.copy
-- @string src source file
-- @string dest destination file
-- @bool flag true if you want to force the copy (default)
-- @return true if operation succeeded
file.copy = dir.copyfile

--- move a file.
-- @function file.move
-- @string src source file
-- @string dest destination file
-- @return true if operation succeeded, else false and the reason for the error.
file.move = dir.movefile

--- Return the time of last access as the number of seconds since the epoch.
-- @function file.access_time
-- @string path A file path
file.access_time = path.getatime

---Return when the file was created.
-- @function file.creation_time
-- @string path A file path
file.creation_time = path.getctime

--- Return the time of last modification
-- @function file.modified_time
-- @string path A file path
file.modified_time = path.getmtime

--- Delete a file
-- @function file.delete
-- @string path A file path
file.delete = os.remove

return file
