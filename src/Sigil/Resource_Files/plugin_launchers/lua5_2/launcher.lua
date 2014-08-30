local SUPPORTED_SCRIPT_TYPES = {
	input=false,
	output=false,
	edit=true
}

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

local function main(args)
    local ebook_root
    local outdir
    local script_type
    local target_file

    if #args ~= 4 then
        xml_out(nil, "Launcher: improper number of arguments passed to launcher.lua", nil, true)
        return 1
    end

    ebook_root  = args[1]
    outdir      = args[2]
    script_type = args[3]
    target_file = args[4]

    if not SUPPORTED_SCRIPT_TYPES[script_type] then
        xml_out(nil, "Launcher: script type " .. (script_type and script_type or "?") .. " is not supported")
        return 1
    end

    xml_out(nil, "Lua plugins are not fully supported/implemented at this time", nil, true)
    return 1
end

local args = {...}
return main(args)
