---- Dealing with Detailed Type Information

-- Dependencies `pl.utils`
-- @module pl.types

local utils = require 'pl.utils'
local types = {}

--- is the object either a function or a callable object?.
-- @param obj Object to check.
function types.is_callable (obj)
    return type(obj) == 'function' or getmetatable(obj) and getmetatable(obj).__call
end

--- is the object of the specified type?.
-- If the type is a string, then use type, otherwise compare with metatable
-- @param obj An object to check
-- @param tp String of what type it should be
-- @function is_type
types.is_type = utils.is_type

local fileMT = getmetatable(io.stdout)

--- a string representation of a type.
-- For tables with metatables, we assume that the metatable has a `_name`
-- field. Knows about Lua file objects.
-- @param obj an object
-- @return a string like 'number', 'table' or 'List'
function types.type (obj)
    local t = type(obj)
    if t == 'table' or t == 'userdata' then
        local mt = getmetatable(obj)
        if mt == fileMT then
            return 'file'
        else
            return mt._name or "unknown "..t
        end
    else
        return t
    end
end

--- is this number an integer?
-- @param x a number
-- @raise error if x is not a number
function types.is_integer (x)
    return math.ceil(x)==x
end

--- Check if the object is "empty".
-- An object is considered empty if it is nil, a table with out any items (key,
-- value pairs or indexes), or a string with no content ("").
-- @param o The object to check if it is empty.
-- @param ignore_spaces If the object is a string and this is true the string is
-- considered empty is it only contains spaces.
-- @return true if the object is empty, otherwise false.
function types.is_empty(o, ignore_spaces)
    if o == nil or (type(o) == "table" and not next(o)) or (type(o) == "string" and (o == "" or (ignore_spaces and o:match("^%s+$")))) then
        return true
    end
    return false
end

local function check_meta (val)
    if type(val) == 'table' then return true end
    return getmetatable(val)
end

--- is an object 'array-like'?
-- @param val any value.
function types.is_indexable (val)
    local mt = check_meta(val)
    if mt == true then return true end
    return not(mt and mt.__len and mt.__index)
end

--- can an object be iterated over with `ipairs`?
-- @param val any value.
function types.is_iterable (val)
    local mt = check_meta(val)
    if mt == true then return true end
    return not(mt and mt.__pairs)
end

--- can an object accept new key/pair values?
-- @param val any value.
function types.is_writeable (val)
    local mt = check_meta(val)
    if mt == true then return true end
    return not(mt and mt.__newindex)
end

-- Strings that should evaluate to true.
local trues = { yes=true, y=true, ["true"]=true, t=true, ["1"]=true }
-- Conditions types should evaluate to true.
local true_types = {
    boolean=function(o, true_strs, check_objs) return o end,
    string=function(o, true_strs, check_objs)
        if trues[o:lower()] then
            return true
        end
        -- Check alternative user provided strings.
        for _,v in ipairs(true_strs or {}) do
            if type(v) == "string" and o == v:lower() then
                return true
            end
        end
        return false
    end,
    number=function(o, true_strs, check_objs) return o ~= 0 end,
    table=function(o, true_strs, check_objs) if check_objs and next(o) ~= nil then return true end return false end
}
--- Convert to a boolean value.
-- True values are:
--
-- * boolean: true.
-- * string: 'yes', 'y', 'true', 't', '1' or additional strings specified by `true_strs`.
-- * number: Any non-zero value.
-- * table: Is not empty and `check_objs` is true.
-- * object: Is not `nil` and `check_objs` is true.
--
-- @param o The object to evaluate.
-- @param[opt] true_strs optional Additional strings that when matched should evaluate to true. Comparison is case insensitive.
-- This should be a List of strings. E.g. "ja" to support German.
-- @param[opt] check_objs True if objects should be evaluated. Default is to evaluate objects as true if not nil
-- or if it is a table and it is not empty.
-- @return true if the input evaluates to true, otherwise false.
function types.to_bool(o, true_strs, check_objs)
    local true_func
    if true_strs then
        utils.assert_arg(2, true_strs, "table")
    end
    true_func = true_types[type(o)]
    if true_func then
        return true_func(o, true_strs, check_objs)
    elseif check_objs and o ~= nil then
        return true
    end
    return false
end


return types
