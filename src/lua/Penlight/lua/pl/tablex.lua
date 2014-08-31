--- Extended operations on Lua tables.
--
-- See @{02-arrays.md.Useful_Operations_on_Tables|the Guide}
--
-- Dependencies: `pl.utils`, `pl.types`
-- @module pl.tablex
local utils = require ('pl.utils')
local types = require ('pl.types')
local getmetatable,setmetatable,require = getmetatable,setmetatable,require
local tsort,append,remove = table.sort,table.insert,table.remove
local min,max = math.min,math.max
local pairs,type,unpack,next,select,tostring = pairs,type,utils.unpack,next,select,tostring
local function_arg = utils.function_arg
local Set = utils.stdmt.Set
local List = utils.stdmt.List
local Map = utils.stdmt.Map
local assert_arg = utils.assert_arg

local tablex = {}

-- generally, functions that make copies of tables try to preserve the metatable.
-- However, when the source has no obvious type, then we attach appropriate metatables
-- like List, Map, etc to the result.
local function setmeta (res,tbl,def)
    local mt = getmetatable(tbl) or def
    return setmetatable(res, mt)
end

local function makelist (res)
    return setmetatable(res,List)
end

local function complain (idx,msg)
    error(('argument %d is not %s'):format(idx,msg),3)
end

local function assert_arg_indexable (idx,val)
    if not types.is_indexable(val) then
        complain(idx,"indexable")
    end
end

local function assert_arg_iterable (idx,val)
    if not types.is_iterable(val) then
        complain(idx,"iterable")
    end
end

local function assert_arg_writeable (idx,val)
    if not types.is_writeable(val) then
        complain(idx,"writeable")
    end
end

--- copy a table into another, in-place.
-- @within Copying
-- @tab t1 destination table
-- @tab t2 source (actually any iterable object)
-- @return first table
function tablex.update (t1,t2)
    assert_arg_writeable(1,t1)
    assert_arg_iterable(2,t2)
    for k,v in pairs(t2) do
        t1[k] = v
    end
    return t1
end

--- total number of elements in this table.
-- Note that this is distinct from `#t`, which is the number
-- of values in the array part; this value will always
-- be greater or equal. The difference gives the size of
-- the hash part, for practical purposes. Works for any
-- object with a __pairs metamethod.
-- @tab t a table
-- @return the size
function tablex.size (t)
    assert_arg_iterable(1,t)
    local i = 0
    for k in pairs(t) do i = i + 1 end
    return i
end

--- make a shallow copy of a table
-- @within Copying
-- @tab t an iterable source
-- @return new table
function tablex.copy (t)
    assert_arg_iterable(1,t)
    local res = {}
    for k,v in pairs(t) do
        res[k] = v
    end
    return res
end

--- make a deep copy of a table, recursively copying all the keys and fields.
-- This will also set the copied table's metatable to that of the original.
-- @within Copying
-- @tab t A table
-- @return new table
function tablex.deepcopy(t)
    if type(t) ~= 'table' then return t end
    assert_arg_iterable(1,t)
    local mt = getmetatable(t)
    local res = {}
    for k,v in pairs(t) do
        if type(v) == 'table' then
            v = tablex.deepcopy(v)
        end
        res[k] = v
    end
    setmetatable(res,mt)
    return res
end

local abs, deepcompare = math.abs

--- compare two values.
-- if they are tables, then compare their keys and fields recursively.
-- @within Comparing
-- @param t1 A value
-- @param t2 A value
-- @bool[opt] ignore_mt if true, ignore __eq metamethod (default false)
-- @number[opt] eps if defined, then used for any number comparisons
-- @return true or false
function tablex.deepcompare(t1,t2,ignore_mt,eps)
    local ty1 = type(t1)
    local ty2 = type(t2)
    if ty1 ~= ty2 then return false end
    -- non-table types can be directly compared
    if ty1 ~= 'table' then
        if ty1 == 'number' and eps then return abs(t1-t2) < eps end
        return t1 == t2
    end
    -- as well as tables which have the metamethod __eq
    local mt = getmetatable(t1)
    if not ignore_mt and mt and mt.__eq then return t1 == t2 end
    for k1 in pairs(t1) do
        if t2[k1]==nil then return false end
    end
    for k2 in pairs(t2) do
        if t1[k2]==nil then return false end
    end
    for k1,v1 in pairs(t1) do
        local v2 = t2[k1]
        if not deepcompare(v1,v2,ignore_mt,eps) then return false end
    end

    return true
end

deepcompare = tablex.deepcompare

--- compare two arrays using a predicate.
-- @within Comparing
-- @array t1 an array
-- @array t2 an array
-- @func cmp A comparison function
function tablex.compare (t1,t2,cmp)
    assert_arg_indexable(1,t1)
    assert_arg_indexable(2,t2)
    if #t1 ~= #t2 then return false end
    cmp = function_arg(3,cmp)
    for k = 1,#t1 do
        if not cmp(t1[k],t2[k]) then return false end
    end
    return true
end

--- compare two list-like tables using an optional predicate, without regard for element order.
-- @within Comparing
-- @array t1 a list-like table
-- @array t2 a list-like table
-- @param cmp A comparison function (may be nil)
function tablex.compare_no_order (t1,t2,cmp)
    assert_arg_indexable(1,t1)
    assert_arg_indexable(2,t2)
    if cmp then cmp = function_arg(3,cmp) end
    if #t1 ~= #t2 then return false end
    local visited = {}
    for i = 1,#t1 do
        local val = t1[i]
        local gotcha
        for j = 1,#t2 do if not visited[j] then
            local match
            if cmp then match = cmp(val,t2[j]) else match = val == t2[j] end
            if match then
                gotcha = j
                break
            end
        end end
        if not gotcha then return false end
        visited[gotcha] = true
    end
    return true
end


--- return the index of a value in a list.
-- Like string.find, there is an optional index to start searching,
-- which can be negative.
-- @within Finding
-- @array t A list-like table
-- @param val A value
-- @int idx index to start; -1 means last element,etc (default 1)
-- @return index of value or nil if not found
-- @usage find({10,20,30},20) == 2
-- @usage find({'a','b','a','c'},'a',2) == 3
function tablex.find(t,val,idx)
    assert_arg_indexable(1,t)
    idx = idx or 1
    if idx < 0 then idx = #t + idx + 1 end
    for i = idx,#t do
        if t[i] == val then return i end
    end
    return nil
end

--- return the index of a value in a list, searching from the end.
-- Like string.find, there is an optional index to start searching,
-- which can be negative.
-- @within Finding
-- @array t A list-like table
-- @param val A value
-- @param idx index to start; -1 means last element,etc (default 1)
-- @return index of value or nil if not found
-- @usage rfind({10,10,10},10) == 3
function tablex.rfind(t,val,idx)
    assert_arg_indexable(1,t)
    idx = idx or #t
    if idx < 0 then idx = #t + idx + 1 end
    for i = idx,1,-1 do
        if t[i] == val then return i end
    end
    return nil
end


--- return the index (or key) of a value in a table using a comparison function.
-- @within Finding
-- @tab t A table
-- @func cmp A comparison function
-- @param arg an optional second argument to the function
-- @return index of value, or nil if not found
-- @return value returned by comparison function
function tablex.find_if(t,cmp,arg)
    assert_arg_iterable(1,t)
    cmp = function_arg(2,cmp)
    for k,v in pairs(t) do
        local c = cmp(v,arg)
        if c then return k,c end
    end
    return nil
end

--- return a list of all values in a table indexed by another list.
-- @tab tbl a table
-- @array idx an index table (a list of keys)
-- @return a list-like table
-- @usage index_by({10,20,30,40},{2,4}) == {20,40}
-- @usage index_by({one=1,two=2,three=3},{'one','three'}) == {1,3}
function tablex.index_by(tbl,idx)
    assert_arg_indexable(1,tbl)
    assert_arg_indexable(2,idx)
    local res = {}
    for i = 1,#idx do
        res[i] = tbl[idx[i]]
    end
    return setmeta(res,tbl,List)
end

--- apply a function to all values of a table.
-- This returns a table of the results.
-- Any extra arguments are passed to the function.
-- @within MappingAndFiltering
-- @func fun A function that takes at least one argument
-- @tab t A table
-- @param ... optional arguments
-- @usage map(function(v) return v*v end, {10,20,30,fred=2}) is {100,400,900,fred=4}
function tablex.map(fun,t,...)
    assert_arg_iterable(1,t)
    fun = function_arg(1,fun)
    local res = {}
    for k,v in pairs(t) do
        res[k] = fun(v,...)
    end
    return setmeta(res,t)
end

--- apply a function to all values of a list.
-- This returns a table of the results.
-- Any extra arguments are passed to the function.
-- @within MappingAndFiltering
-- @func fun A function that takes at least one argument
-- @array t a table (applies to array part)
-- @param ... optional arguments
-- @return a list-like table
-- @usage imap(function(v) return v*v end, {10,20,30,fred=2}) is {100,400,900}
function tablex.imap(fun,t,...)
    assert_arg_indexable(1,t)
    fun = function_arg(1,fun)
    local res = {}
    for i = 1,#t do
        res[i] = fun(t[i],...) or false
    end
    return setmeta(res,t,List)
end

--- apply a named method to values from a table.
-- @within MappingAndFiltering
-- @string name the method name
-- @array t a list-like table
-- @param ... any extra arguments to the method
function tablex.map_named_method (name,t,...)
    utils.assert_string(1,name)
    assert_arg_indexable(2,t)
    local res = {}
    for i = 1,#t do
        local val = t[i]
        local fun = val[name]
        res[i] = fun(val,...)
    end
    return setmeta(res,t,List)
end

--- apply a function to all values of a table, in-place.
-- Any extra arguments are passed to the function.
-- @func fun A function that takes at least one argument
-- @tab t a table
-- @param ... extra arguments
function tablex.transform (fun,t,...)
    assert_arg_iterable(1,t)
    fun = function_arg(1,fun)
    for k,v in pairs(t) do
        t[k] = fun(v,...)
    end
end

--- generate a table of all numbers in a range
-- @int start  number
-- @int finish number
-- @int[opt=1] step  (-1 for decreasing)
function tablex.range (start,finish,step)
    if start == finish then return {start}
    elseif start > finish then return {}
    end
    local res = {}
    local k = 1
    if not step then
        if finish > start then step = finish > start and 1 or -1 end
    end
    for i=start,finish,step do res[k]=i; k=k+1 end
    return res
end

--- apply a function to values from two tables.
-- @within MappingAndFiltering
-- @func fun a function of at least two arguments
-- @tab t1 a table
-- @tab t2 a table
-- @param ... extra arguments
-- @return a table
-- @usage map2('+',{1,2,3,m=4},{10,20,30,m=40}) is {11,22,23,m=44}
function tablex.map2 (fun,t1,t2,...)
    assert_arg_iterable(1,t1)
    assert_arg_iterable(2,t2)
    fun = function_arg(1,fun)
    local res = {}
    for k,v in pairs(t1) do
        res[k] = fun(v,t2[k],...)
    end
    return setmeta(res,t1,List)
end

--- apply a function to values from two arrays.
-- The result will be the length of the shortest array.
-- @within MappingAndFiltering
-- @func fun a function of at least two arguments
-- @array t1 a list-like table
-- @array t2 a list-like table
-- @param ... extra arguments
-- @usage imap2('+',{1,2,3,m=4},{10,20,30,m=40}) is {11,22,23}
function tablex.imap2 (fun,t1,t2,...)
    assert_arg_indexable(2,t1)
    assert_arg_indexable(3,t2)
    fun = function_arg(1,fun)
    local res,n = {},math.min(#t1,#t2)
    for i = 1,n do
        res[i] = fun(t1[i],t2[i],...)
    end
    return res
end

--- 'reduce' a list using a binary function.
-- @func fun a function of two arguments
-- @array t a list-like table
-- @return the result of the function
-- @usage reduce('+',{1,2,3,4}) == 10
function tablex.reduce (fun,t)
    assert_arg_indexable(2,t)
    fun = function_arg(1,fun)
    local n = #t
    local res = t[1]
    for i = 2,n do
        res = fun(res,t[i])
    end
    return res
end

--- apply a function to all elements of a table.
-- The arguments to the function will be the value,
-- the key and _finally_ any extra arguments passed to this function.
-- Note that the Lua 5.0 function table.foreach passed the _key_ first.
-- @within Iterating
-- @tab t a table
-- @func fun a function with at least one argument
-- @param ... extra arguments
function tablex.foreach(t,fun,...)
    assert_arg_iterable(1,t)
    fun = function_arg(2,fun)
    for k,v in pairs(t) do
        fun(v,k,...)
    end
end

--- apply a function to all elements of a list-like table in order.
-- The arguments to the function will be the value,
-- the index and _finally_ any extra arguments passed to this function
-- @within Iterating
-- @array t a table
-- @func fun a function with at least one argument
-- @param ... optional arguments
function tablex.foreachi(t,fun,...)
    assert_arg_indexable(1,t)
    fun = function_arg(2,fun)
    for i = 1,#t do
        fun(t[i],i,...)
    end
end

--- Apply a function to a number of tables.
-- A more general version of map
-- The result is a table containing the result of applying that function to the
-- ith value of each table. Length of output list is the minimum length of all the lists
-- @within MappingAndFiltering
-- @func fun a function of n arguments
-- @tab ... n tables
-- @usage mapn(function(x,y,z) return x+y+z end, {1,2,3},{10,20,30},{100,200,300}) is {111,222,333}
-- @usage mapn(math.max, {1,20,300},{10,2,3},{100,200,100}) is	{100,200,300}
-- @param fun A function that takes as many arguments as there are tables
function tablex.mapn(fun,...)
    fun = function_arg(1,fun)
    local res = {}
    local lists = {...}
    local minn = 1e40
    for i = 1,#lists do
        minn = min(minn,#(lists[i]))
    end
    for i = 1,minn do
        local args,k = {},1
        for j = 1,#lists do
            args[k] = lists[j][i]
            k = k + 1
        end
        res[#res+1] = fun(unpack(args))
    end
    return res
end

--- call the function with the key and value pairs from a table.
-- The function can return a value and a key (note the order!). If both
-- are not nil, then this pair is inserted into the result. If only value is not nil, then
-- it is appended to the result.
-- @within MappingAndFiltering
-- @func fun A function which will be passed each key and value as arguments, plus any extra arguments to pairmap.
-- @tab t A table
-- @param ... optional arguments
-- @usage pairmap(function(k,v) return v end,{fred=10,bonzo=20}) is {10,20} _or_ {20,10}
-- @usage pairmap(function(k,v) return {k,v},k end,{one=1,two=2}) is {one={'one',1},two={'two',2}}
function tablex.pairmap(fun,t,...)
    assert_arg_iterable(1,t)
    fun = function_arg(1,fun)
    local res = {}
    for k,v in pairs(t) do
        local rv,rk = fun(k,v,...)
        if rk then
            res[rk] = rv
        else
            res[#res+1] = rv
        end
    end
    return res
end

local function keys_op(i,v) return i end

--- return all the keys of a table in arbitrary order.
-- @within Extraction
--  @tab t A table
function tablex.keys(t)
    assert_arg_iterable(1,t)
    return makelist(tablex.pairmap(keys_op,t))
end

local function values_op(i,v) return v end

--- return all the values of the table in arbitrary order
-- @within Extraction
--  @tab t A table
function tablex.values(t)
    assert_arg_iterable(1,t)
    return makelist(tablex.pairmap(values_op,t))
end

local function index_map_op (i,v) return i,v end

--- create an index map from a list-like table. The original values become keys,
-- and the associated values are the indices into the original list.
-- @array t a list-like table
-- @return a map-like table
function tablex.index_map (t)
    assert_arg_indexable(1,t)
    return setmetatable(tablex.pairmap(index_map_op,t),Map)
end

local function set_op(i,v) return true,v end

--- create a set from a list-like table. A set is a table where the original values
-- become keys, and the associated values are all true.
-- @array t a list-like table
-- @return a set (a map-like table)
function tablex.makeset (t)
    assert_arg_indexable(1,t)
    return setmetatable(tablex.pairmap(set_op,t),Set)
end

--- combine two tables, either as union or intersection. Corresponds to
-- set operations for sets () but more general. Not particularly
-- useful for list-like tables.
-- @within Merging
-- @tab t1 a table
-- @tab t2 a table
-- @bool dup true for a union, false for an intersection.
-- @usage merge({alice=23,fred=34},{bob=25,fred=34}) is {fred=34}
-- @usage merge({alice=23,fred=34},{bob=25,fred=34},true) is {bob=25,fred=34,alice=23}
-- @see tablex.index_map
function tablex.merge (t1,t2,dup)
    assert_arg_iterable(1,t1)
    assert_arg_iterable(2,t2)
    local res = {}
    for k,v in pairs(t1) do
        if dup or t2[k] then res[k] = v end
    end
    if dup then
      for k,v in pairs(t2) do
        res[k] = v
      end
    end
    return setmeta(res,t1,Map)
end

--- a new table which is the difference of two tables.
-- With sets (where the values are all true) this is set difference and
-- symmetric difference depending on the third parameter.
-- @within Merging
-- @tab s1 a map-like table or set
-- @tab s2 a map-like table or set
-- @bool symm symmetric difference (default false)
-- @return a map-like table or set
function tablex.difference (s1,s2,symm)
    assert_arg_iterable(1,s1)
    assert_arg_iterable(2,s2)
    local res = {}
    for k,v in pairs(s1) do
        if s2[k] == nil then res[k] = v end
    end
    if symm then
        for k,v in pairs(s2) do
            if s1[k] == nil then res[k] = v end
        end
    end
    return setmeta(res,s1,Map)
end

--- A table where the key/values are the values and value counts of the table.
-- @array t a list-like table
-- @func cmp a function that defines equality (otherwise uses ==)
-- @return a map-like table
-- @see seq.count_map
function tablex.count_map (t,cmp)
    assert_arg_indexable(1,t)
    local res,mask = {},{}
    cmp = function_arg(2,cmp)
    local n = #t
    for i = 1,#t do
        local v = t[i]
        if not mask[v] then
            mask[v] = true
            -- check this value against all other values
            res[v] = 1  -- there's at least one instance
            for j = i+1,n do
                local w = t[j]
                local ok
                if cmp then
                    ok = cmp(v,w)
                else
                    ok = v == w
                end
                if ok then
                    res[v] = res[v] + 1
                    mask[w] = true
                end
            end
        end
    end
    return setmetatable(res,Map)
end

--- filter an array's values using a predicate function
-- @within MappingAndFiltering
-- @array t a list-like table
-- @func pred a boolean function
-- @param arg optional argument to be passed as second argument of the predicate
function tablex.filter (t,pred,arg)
    assert_arg_indexable(1,t)
    pred = function_arg(2,pred)
    local res,k = {},1
    for i = 1,#t do
        local v = t[i]
        if pred(v,arg) then
            res[k] = v
            k = k + 1
        end
    end
    return setmeta(res,t,List)
end

--- return a table where each element is a table of the ith values of an arbitrary
-- number of tables. It is equivalent to a matrix transpose.
-- @within Merging
-- @usage zip({10,20,30},{100,200,300}) is {{10,100},{20,200},{30,300}}
-- @array ... arrays to be zipped
function tablex.zip(...)
    return tablex.mapn(function(...) return {...} end,...)
end

local _copy
function _copy (dest,src,idest,isrc,nsrc,clean_tail)
    idest = idest or 1
    isrc = isrc or 1
    local iend
    if not nsrc then
        nsrc = #src
        iend = #src
    else
        iend = isrc + min(nsrc-1,#src-isrc)
    end
    if dest == src then -- special case
        if idest > isrc and iend >= idest then -- overlapping ranges
            src = tablex.sub(src,isrc,nsrc)
            isrc = 1; iend = #src
        end
    end
    for i = isrc,iend do
        dest[idest] = src[i]
        idest = idest + 1
    end
    if clean_tail then
        tablex.clear(dest,idest)
    end
    return dest
end

--- copy an array into another one, clearing `dest` after `idest+nsrc`, if necessary.
-- @within Copying
-- @array dest a list-like table
-- @array src a list-like table
-- @int[opt=1] idest where to start copying values into destination
-- @int[opt=1] isrc where to start copying values from source
-- @int[opt=#src] nsrc number of elements to copy from source
function tablex.icopy (dest,src,idest,isrc,nsrc)
    assert_arg_indexable(1,dest)
    assert_arg_indexable(2,src)
    return _copy(dest,src,idest,isrc,nsrc,true)
end

--- copy an array into another one.
-- @within Copying
-- @array dest a list-like table
-- @array src a list-like table
-- @int[opt=1] idest where to start copying values into destination
-- @int[opt=1] isrc where to start copying values from source
-- @int[opt=#src] nsrc number of elements to copy from source
function tablex.move (dest,src,idest,isrc,nsrc)
    assert_arg_indexable(1,dest)
    assert_arg_indexable(2,src)
    return _copy(dest,src,idest,isrc,nsrc,false)
end

function tablex._normalize_slice(self,first,last)
  local sz = #self
  if not first then first=1 end
  if first<0 then first=sz+first+1 end
  -- make the range _inclusive_!
  if not last then last=sz end
  if last < 0 then last=sz+1+last end
  return first,last
end

--- Extract a range from a table, like  'string.sub'.
-- If first or last are negative then they are relative to the end of the list
-- eg. sub(t,-2) gives last 2 entries in a list, and
-- sub(t,-4,-2) gives from -4th to -2nd
-- @within Extraction
-- @array t a list-like table
-- @int first An index
-- @int last An index
-- @return a new List
function tablex.sub(t,first,last)
    assert_arg_indexable(1,t)
    first,last = tablex._normalize_slice(t,first,last)
    local res={}
    for i=first,last do append(res,t[i]) end
    return setmeta(res,t,List)
end

--- set an array range to a value. If it's a function we use the result
-- of applying it to the indices.
-- @array t a list-like table
-- @param val a value
-- @int[opt=1] i1 start range
-- @int[opt=#t] i2 end range
function tablex.set (t,val,i1,i2)
    assert_arg_indexable(1,t)
    i1,i2 = i1 or 1,i2 or #t
    if types.is_callable(val) then
        for i = i1,i2 do
            t[i] = val(i)
        end
    else
        for i = i1,i2 do
            t[i] = val
        end
    end
end

--- create a new array of specified size with initial value.
-- @int n size
-- @param val initial value (can be `nil`, but don't expect `#` to work!)
-- @return the table
function tablex.new (n,val)
    local res = {}
    tablex.set(res,val,1,n)
    return res
end

--- clear out the contents of a table.
-- @array t a list
-- @param istart optional start position
function tablex.clear(t,istart)
    istart = istart or 1
    for i = istart,#t do remove(t) end
end

--- insert values into a table.
-- similar to `table.insert` but inserts values from given table `values`,
-- not the object itself, into table `t` at position `pos`.
-- @within Copying
-- @array t the list
-- @int[opt] position (default is at end)
-- @array values
function tablex.insertvalues(t, ...)
    assert_arg(1,t,'table')
    local pos, values
    if select('#', ...) == 1 then
        pos,values = #t+1, ...
    else
        pos,values = ...
    end
    if #values > 0 then
        for i=#t,pos,-1 do
            t[i+#values] = t[i]
        end
        local offset = 1 - pos
        for i=pos,pos+#values-1 do
            t[i] = values[i + offset]
        end
    end
    return t
end

--- remove a range of values from a table.
-- End of range may be negative.
-- @array t a list-like table
-- @int i1 start index
-- @int i2 end index
-- @return the table
function tablex.removevalues (t,i1,i2)
    assert_arg(1,t,'table')
    i1,i2 = tablex._normalize_slice(t,i1,i2)
    for i = i1,i2 do
        remove(t,i1)
    end
    return t
end

local _find
_find = function (t,value,tables)
    for k,v in pairs(t) do
        if v == value then return k end
    end
    for k,v in pairs(t) do
        if not tables[v] and type(v) == 'table' then
            tables[v] = true
            local res = _find(v,value,tables)
            if res then
                res = tostring(res)
                if type(k) ~= 'string' then
                    return '['..k..']'..res
                else
                    return k..'.'..res
                end
            end
        end
    end
end

--- find a value in a table by recursive search.
-- @within Finding
-- @tab t the table
-- @param value the value
-- @array[opt] exclude any tables to avoid searching
-- @usage search(_G,math.sin,{package.path}) == 'math.sin'
-- @return a fieldspec, e.g. 'a.b' or 'math.sin'
function tablex.search (t,value,exclude)
    assert_arg_iterable(1,t)
    local tables = {[t]=true}
    if exclude then
        for _,v in pairs(exclude) do tables[v] = true end
    end
    return _find(t,value,tables)
end

--- return an iterator to a table sorted by its keys
-- @within Iterating
-- @tab t the table
-- @func f an optional comparison function (f(x,y) is true if x < y)
-- @usage for k,v in tablex.sort(t) do print(k,v) end
-- @return an iterator to traverse elements sorted by the keys
function tablex.sort(t,f)
    local keys = {}
    for k in pairs(t) do keys[#keys + 1] = k end
    tsort(keys,f)
    local i = 0
    return function()
        i = i + 1
        return keys[i], t[keys[i]]
    end
end

--- return an iterator to a table sorted by its values
-- @within Iterating
-- @tab t the table
-- @func f an optional comparison function (f(x,y) is true if x < y)
-- @usage for k,v in tablex.sortv(t) do print(k,v) end
-- @return an iterator to traverse elements sorted by the values
function tablex.sortv(t,f)
    local rev = {}
    for k,v in pairs(t) do rev[v] = k end
    local next = tablex.sort(rev,f)
    return function()
        local value,key = next()
        return key,value
    end
end

--- modifies a table to be read only.
-- This only offers weak protection. Tables can still be modified with
-- `table.insert` and `rawset`.
-- @tab t the table
-- @return the table read only.
function tablex.readonly(t)
    local mt = {
        __index=t,
        __newindex=function(t, k, v) error("Attempt to modify read-only table", 2) end,
        __pairs=function() return pairs(t) end,
        __ipairs=function() return ipairs(t) end,
        __len=function() return #t end,
        __metatable=false
    }
    return setmetatable({}, mt)
end

return tablex
