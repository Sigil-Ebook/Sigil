-- running the tests and examples
require 'pl'
local lfs = require 'lfs'

local function quote_if_needed (s)
    if s:match '%s' then
        s = '"'..s..'"'
    end
    return s
end

local function print_exit(msg)
    print()
    print(string.rep("*",#msg + 4))
    print("* "..msg.." *")
    print(string.rep("*",#msg + 4))
end

-- get the Lua command-line used to invoke this script
local cmd = app.lua()

function do_lua_files ()
    for _,f in ipairs(dir.getfiles('.','*.lua')) do
        print(cmd..' '..f)
        local res,code = utils.execute(cmd..' '..f)
        if not res then
            print_exit ('process failed with non-zero result: ['..code..'] '..f)
            os.exit(1)
        end
    end
end

if #arg == 0 then arg[1] = 'tests'; arg[2] = 'examples' end

for _,dir in ipairs(arg) do
    print('directory',dir)
    lfs.chdir(dir)
    do_lua_files()
    lfs.chdir('..')
end

