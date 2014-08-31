local tablex = require 'pl.tablex'
local utils = require ('pl.utils')
local L = utils.string_lambda
local test = require('pl.test')
-- bring tablex funtions into global namespace
utils.import(tablex)
local asserteq = test.asserteq

local cmp = deepcompare

function asserteq_no_order (x,y)
    if not compare_no_order(x,y) then
        test.complain(x,y,"these lists contained different elements")
    end
end


asserteq(
	copy {10,20,30},
	{10,20,30}
)

asserteq(
	deepcopy {10,20,{30,40}},
	{10,20,{30,40}}
)

asserteq(
	pairmap(function(i,v) return v end,{10,20,30}),
	{10,20,30}
)

asserteq_no_order(
	pairmap(L'_',{fred=10,bonzo=20}),
	{'fred','bonzo'}
)

asserteq_no_order(
	pairmap(function(k,v) return v end,{fred=10,bonzo=20}),
	{10,20}
)

asserteq_no_order(
	pairmap(function(i,v) return v,i end,{10,20,30}),
	{10,20,30}
)

asserteq(
	pairmap(function(k,v) return {k,v},k end,{one=1,two=2}),
	{one={'one',1},two={'two',2}}
)
-- same as above, using string lambdas
asserteq(
	pairmap(L'|k,v|{k,v},k',{one=1,two=2}),
	{one={'one',1},two={'two',2}}
)


asserteq(
	map(function(v) return v*v end,{10,20,30}),
	{100,400,900}
)

-- extra arguments to map() are passed to the function; can use
-- the abbreviations provided by pl.operator
asserteq(
    map('+',{10,20,30},1),
    {11,21,31}
)

asserteq(
    map(L'_+1',{10,20,30}),
    {11,21,31}
)

-- map2 generalizes for operations on two tables
asserteq(
    map2(math.max,{1,2,3},{0,4,2}),
    {1,4,3}
)

-- mapn operates over an arbitrary number of input tables (but use map2 for n=2)
asserteq(
	mapn(function(x,y,z) return x+y+z end, {1,2,3},{10,20,30},{100,200,300}),
	{111,222,333}
)

asserteq(
	mapn(math.max, {1,20,300},{10,2,3},{100,200,100}),
	{100,200,300}
)

asserteq(
    zip({10,20,30},{100,200,300}),
    {{10,100},{20,200},{30,300}}
)

assert(compare_no_order({1,2,3,4},{2,1,4,3})==true)
assert(compare_no_order({1,2,3,4},{2,1,4,4})==false)

asserteq(range(10,9),{})
asserteq(range(10,10),{10})
asserteq(range(10,11),{10,11})

-- update inserts key-value pairs from the second table
t1 = {one=1,two=2}
t2 = {three=3,two=20,four=4}
asserteq(update(t1,t2),{one=1,three=3,two=20,four=4})

-- the difference between move and icopy is that the second removes
-- any extra elements in the destination after end of copy
-- 3rd arg is the index to start in the destination, defaults to 1
asserteq(move({1,2,3,4,5,6},{20,30}),{20,30,3,4,5,6})
asserteq(move({1,2,3,4,5,6},{20,30},2),{1,20,30,4,5,6})
asserteq(icopy({1,2,3,4,5,6},{20,30},2),{1,20,30})
-- 5th arg determines how many elements to copy (default size of source)
asserteq(icopy({1,2,3,4,5,6},{20,30},2,1,1),{1,20})
-- 4th arg is where to stop copying from the source (default s to 1)
asserteq(icopy({1,2,3,4,5,6},{20,30},2,2,1),{1,30})

-- whereas insertvalues works like table.insert, but inserts a range of values
-- from the given table.
asserteq(insertvalues({1,2,3,4,5,6},2,{20,30}),{1,20,30,2,3,4,5,6})
asserteq(insertvalues({1,2},{3,4}),{1,2,3,4})

-- the 4th arg of move and icopy gives the start index in the source table
asserteq(move({1,2,3,4,5,6},{20,30},2,2),{1,30,3,4,5,6})
asserteq(icopy({1,2,3,4,5,6},{20,30},2,2),{1,30})

t = {1,2,3,4,5,6}
move(t,{20,30},2,1,1)
asserteq(t,{1,20,3,4,5,6})
set(t,0,2,3)
asserteq(t,{1,0,0,4,5,6})
insertvalues(t,1,{10,20})
asserteq(t,{10,20,1,0,0,4,5,6})

asserteq(merge({10,20,30},{nil, nil, 30, 40}), {[3]=30})
asserteq(merge({10,20,30},{nil, nil, 30, 40}, true), {10,20,30,40})


-- Function to check that the order of elements returned by the iterator
-- match the order of the elements in the list.
function assert_iter_order(iter,l)
   local i = 0
   for k,v in iter do
      i = i + 1
      asserteq(k,l[i][1])
      asserteq(v,l[i][2])
   end
end

local t = {a=10,b=9,c=8,d=7,e=6,f=5,g=4,h=3,i=2,j=1}

assert_iter_order(
   sort(t),
   {{'a',10},{'b',9},{'c',8},{'d',7},{'e',6},{'f',5},{'g',4},{'h',3},{'i',2},{'j',1}})

assert_iter_order(
   sortv(t),
   {{'j',1},{'i',2},{'h',3},{'g',4},{'f',5},{'e',6},{'d',7},{'c',8},{'b',9},{'a',10}})


asserteq(difference({a = true, b = true},{a = true, b = true}),{})

-- no longer confused by false values ;)
asserteq(difference({v = false},{v = false}),{})

asserteq(difference({a = true},{b = true}),{a=true})

-- symmetric difference
asserteq(difference({a = true},{b = true},true),{a=true,b=true})







