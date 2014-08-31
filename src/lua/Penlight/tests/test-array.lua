local array = require 'pl.array2d'
local asserteq = require('pl.test').asserteq
local L = require 'pl.utils'. string_lambda

local A = {
	{1,2,3,4},
	{10,20,30,40},
	{100,200,300,400},
	{1000,2000,3000,4000},
}

asserteq(array.column(A,2),{2,20,200,2000})
asserteq(array.reduce_rows('+',A),{10,100,1000,10000})
asserteq(array.reduce_cols('+',A),{1111,2222,3333,4444})

--array.write(A)

local dump = require 'pl.pretty'.dump

asserteq(array.range(A,'A1:B1'),{1,2})

asserteq(array.range(A,'A1:B2'),{{1,2},{10,20}})

asserteq(
    array.product('..',{1,2,3},{'a','b','c'}),
    {{'1a','2a','3a'},{'1b','2b','3b'},{'1c','2c','3c'}}
)

asserteq(
    array.product('{}',{1,2},{'a','b','c'}),
    {{{1,'a'},{2,'a'}},{{1,'b'},{2,'b'}},{{1,'c'},{2,'c'}}}
)

asserteq(
    array.flatten {{1,2},{3,4},{5,6}},
    {1,2,3,4,5,6}
)


A = {{1,2,3},{4,5,6}}

-- flatten in column order!
asserteq(
    array.reshape(A,1,true),
    {{1,4,2,5,3,6}}
)

-- regular row-order reshape
asserteq(
    array.reshape(A,3),
    {{1,2},{3,4},{5,6}}
)

asserteq(
    array.new(3,3,0),
    {{0,0,0},{0,0,0},{0,0,0}}
)

asserteq(
    array.new(3,3,L'|i,j| i==j and 1 or 0'),
    {{1,0,0},{0,1,0},{0,0,1}}
)

asserteq(
    array.reduce2('+','*',{{1,10},{2,10},{3,10}}),
    60 -- i.e. 1*10 + 2*10 + 3*10
)

A = array.new(4,4,0)
B = array.new(3,3,1)
array.move(A,2,2,B)
asserteq(A,{{0,0,0,0},{0,1,1,1},{0,1,1,1},{0,1,1,1}})





