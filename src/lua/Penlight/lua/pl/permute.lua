--- Permutation operations.
--
-- Dependencies: `pl.utils`, `pl.tablex`
-- @module pl.permute
local tablex = require 'pl.tablex'
local utils = require 'pl.utils'
local copy = tablex.deepcopy
local append = table.insert
local coroutine = coroutine
local resume = coroutine.resume
local assert_arg = utils.assert_arg


local permute = {}

-- PiL, 9.3

local permgen
permgen = function (a, n, fn)
  if n == 0 then
    fn(a)
  else
    for i=1,n do
      -- put i-th element as the last one
      a[n], a[i] = a[i], a[n]

      -- generate all permutations of the other elements
      permgen(a, n - 1, fn)

      -- restore i-th element
      a[n], a[i] = a[i], a[n]

    end
  end
end

--- an iterator over all permutations of the elements of a list.
-- Please note that the same list is returned each time, so do not keep references!
-- @param a list-like table
-- @return an iterator which provides the next permutation as a list
function permute.iter (a)
    assert_arg(1,a,'table')
    local n = #a
    local co = coroutine.create(function () permgen(a, n, coroutine.yield) end)
    return function ()   -- iterator
        local code, res = resume(co)
        return res
    end
end

--- construct a table containing all the permutations of a list.
-- @param a list-like table
-- @return a table of tables
-- @usage permute.table {1,2,3} --> {{2,3,1},{3,2,1},{3,1,2},{1,3,2},{2,1,3},{1,2,3}}
function permute.table (a)
    assert_arg(1,a,'table')
    local res = {}
    local n = #a
    permgen(a,n,function(t) append(res,copy(t)) end)
    return res
end

return permute
