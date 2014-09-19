local SUPPORTED_SCRIPT_TYPES = {
	input=false,
	output=false,
	edit=true
}

-- Redirect print to store it in a variable. We'll output this as part of our
-- log so plugins can still do print().
local print_result = {}
local function myprint(...)
    local builder = {}

    for i,v in ipairs({...}) do
        builder[#builder+1] = tostring(v)
    end

    print_result[#print_result+1] = table.concat(builder, "\t")
end
print = myprint

-- System dir separator.
local SEP = _G.package.config:sub(1,1)

-- Check if a string ends with a given value.
local function string_endswith(s, e)
    return #s >= #e and s:find(e, #s-#e+1, true) and true or false
end

-- Insert a path into the package.(c)path.
local function insert_package_path(path, isdir)
    local p

    -- Figure out if we have a full path or just name.
    p = path
    if not path:find(SEP) then
    	if not isdir then
            p = "." .. SEP
        end
    else
    	if not isdir then
            p, _ = path:match("(.*" .. SEP .. ")(.*)")
        end
    end

    if not string_endswith(p, SEP) then
    	p = p .. SEP
    end

    -- Add the dir to the package paths.
    package.path  = p .. "?.lua"      .. ";" .. package.path 
    package.path  = p .. "?/init.lua" .. ";" .. package.path 
    package.cpath = p .. "?.so"       .. ";" .. package.cpath
    package.cpath = p .. "?.dylib"    .. ";" .. package.cpath
    package.cpath = p .. "?.dll"      .. ";" .. package.cpath
end

-- Add the launcher's dir and the target script's dir to the package
-- paths so we can load anything in those locations with require.
local function enhance_package_path(arg0, target_file)
    insert_package_path(target_file, false)
    insert_package_path(arg0, false)
end

-- Result xml for the plugin execution.
local function xml_out(script_type, msg, data, fail)
    local wrapper = { '<?xml version="1.0" encoding="UTF-8"?>' }

    wrapper[#wrapper+1] = "<wrapper"
    if script_type then
        wrapper[#wrapper+1] = ' type="' .. script_type .. '"'
    end
    wrapper[#wrapper+1] = ">"

    wrapper[#wrapper+1] = "<result>"
    if fail then
        wrapper[#wrapper+1] = "failed"
    else
        wrapper[#wrapper+1] = "success"
    end
    wrapper[#wrapper+1] = "</result>"

    wrapper[#wrapper+1] = "<msg>" .. (msg and msg or "") .. "</msg>"
    if data then
        wrapper[#wrapper+1] = data
    end
    wrapper[#wrapper+1] = "</wrapper>"

    io.stdout:write(table.concat(wrapper, "").."\n")
end

-- Run the plugin.
local function main()
    local ebook_root
    local outdir
    local script_type
    local target_file

    if #arg ~= 4 then
        xml_out(nil, "Launcher: improper number of arguments passed to launcher.lua", nil, true)
        return 1
    end

    ebook_root  = arg[1]
    outdir      = arg[2]
    script_type = arg[3]
    target_file = arg[4]

    enhance_package_path(arg[0], target_file)

    if not SUPPORTED_SCRIPT_TYPES[script_type] then
        xml_out(nil, "Launcher: script type " .. (script_type and script_type or "?") .. " is not supported")
        return 1
    end

    xml_out(nil, "Lua plugins are not fully supported/implemented at this time".."\n\n"..table.concat(print_result, "\n"), nil, true)
    return 1
end

return main()
