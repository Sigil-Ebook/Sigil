## Tables and Arrays

<a id="list"/>

### Python-style Lists

One of the elegant things about Lua is that tables do the job of both lists and
dicts (as called in Python) or vectors and maps, (as called in C++), and they do
it efficiently.  However, if we are dealing with 'tables with numerical indices'
we may as well call them lists and look for operations which particularly make
sense for lists. The Penlight `List` class was originally written by Nick Trout
for Lua 5.0, and translated to 5.1 and extended by myself.  It seemed that
borrowing from Python was a good idea, and this eventually grew into Penlight.

Here is an example showing `List` in action; it redefines `__tostring`, so that
it can print itself out more sensibly:

    > List = require 'pl.List'  --> automatic with require 'pl' <---
    > l = List()
    > l:append(10)
    > l:append(20)
    > = l
    {10,20}
    > l:extend {30,40}
    {10,20,30,40}
    > l:insert(1,5)
    {5,10,20,30,40}
    > = l:pop()
    40
    > = l
    {5,10,20,30}
    > = l:index(30)
    4
    > = l:contains(30)
    true
    > = l:reverse()  ---> note: doesn't make a copy!
    {30,20,10,5}

Although methods like `sort` and `reverse` operate in-place and change the list,
they do return the original list. This makes it possible to do _method chaining_,
like `ls = ls:append(10):append(20):reverse():append(1)`. But (and this is an
important but) no extra copy is made, so `ls` does not change identity. `List`
objects (like tables) are _mutable_, unlike strings. If you want a copy of a
list, then `List(ls)` will do the job, i.e. it acts like a copy constructor.
However, if passed any other table, `List` will just set the metatable of the
table and _not_ make a copy.

A particular feature of Python lists is _slicing_. This is fully supported in
this version of `List`, except we use 1-based indexing. So `List.slice` works
rather like `string.sub`:

    > l = List {10,20,30,40}
    > = l:slice(1,1)  ---> note: creates a new list!
    {10}
    > = l:slice(2,2)
    {20}
    > = l:slice(2,3)
    {20,30}
    > = l:slice(2,-2)
    {20,30}
    > = l:slice_assign(2,2,{21,22,23})
    {10,21,22,23,30,40}
    > = l:chop(1,1)
    {21,22,23,30,40}

Functions like `slice_assign` and `chop` modify the list; the first is equivalent
to Python`l[i1:i2] = seq` and the second to `del l[i1:i2]`.

List objects are ultimately just Lua 'list-like' tables, but they have extra
operations defined on them, such as equality and concatention.  For regular
tables, equality is only true if the two tables are _identical objects_, whereas
two lists are equal if they have the same contents, i.e. that `l1[i]==l2[i]` for
all elements.

    > l1 = List {1,2,3}
    > l2 = List {1,2,3}
    > = l1 == l2
    true
    > = l1..l2
    {1,2,3,1,2,3}

The `List` constructor can be passed a function. If so, it's assumed that this is
an iterator function that can be repeatedly called to generate a sequence.  One
such function is `io.lines`; the following short, intense little script counts
the number of lines in standard input:

    -- linecount.lua
    require 'pl'
    ls = List(io.lines())
    print(#ls)

`List.iterate` captures what `List` considers a sequence. In particular, it can
also iterate over all 'characters' in a string:

    > for ch in List.iterate 'help' do io.write(ch,' ') end
    h e l p >

Since the function `iterate` is used internally by the `List` constructor,
strings can be made into lists of character strings very easily.

There are a number of operations that go beyond the standard Python methods. For
instance, you can _partition_ a list into a table of sublists using a function.
In the simplest form, you use a predicate (a function returning a boolean value)
to partition the list into two lists, one of elements matching and another of
elements not matching. But you can use any function; if we use `type` then the
keys will be the standard Lua type names.

    > ls = List{1,2,3,4}
    > ops = require 'pl.operator'
    > ls:partition(function(x) return x > 2 end)
    {false={1,2},true={3,4}}
    > ls = List{'one',math.sin,List{1},10,20,List{1,2}}
    > ls:partition(type)
    {function={function: 00369110},string={one},number={10,20},table={{1},{1,2}}}

This is one `List` method which returns a table which is not a `List`. Bear in
mind that you can always call a `List` method on a plain table argument, so
`List.partition(t,type)` works as expected. But these functions will only operate
on the array part of the table.

The 'nominal' type of the returned table is `pl.Multimap`, which describes a mapping
between keys and multiple values. This does not mean that `pl.Multimap` is automatically
loaded whenever you use `partition` (or `List` for that matter); this is one of the
standard metatables which are only filled out when the appropriate module is loaded.
This allows tables to be tagged appropriately without causing excessive coupling.

Stacks occur everywhere in computing. `List` supports stack-like operations;
there is already `pop` (remove and return last value) and `append` acts like
`push` (add a value to the end). `push` is provided as an alias for `append`, and
the other stack operation (size) is simply the size operator `#`.  Queues can
also be implemented; you use `pop` to take values out of the queue, and `put` to
insert a value at the begining.

You may derive classes from `List`, and since the list-returning methods
are covariant, the result of `slice` etc will return lists of the derived type,
not `List`. For instance, consider the specialization of a `List` type that contains
numbers in `tests/test-list.lua`:

    n1 = NA{10,20,30}
    n2 = NA{1,2,3}
    ns = n1 + 2*n2
    asserteq(ns,{12,24,36})
    min,max = ns:slice(1,2):minmax()
    asserteq(T(min,max),T(12,24))
    asserteq(n1:normalize():sum(),1,1e-8)


### Map and Set classes

The `Map` class exposes what Python would call a 'dict' interface, and accesses
the hash part of the table. The name 'Map' is used to emphasize the interface,
not the implementation; it is an object which maps keys onto values; `m['alice']`
or the equivalent `m.alice` is the access operation.  This class also provides
explicit `set` and `get` methods, which are trivial for regular maps but get
interesting when `Map` is subclassed. The other operation is `update`, which
extends a map by copying the keys and values from another table, perhaps
overwriting existing keys:

    > Map = require 'pl.Map'
    > m = Map{one=1,two=2}
    > m:update {three=3,four=4,two=20}
    > = m == M{one=1,two=20,three=3,four=4}
    true

The method `values` returns a list of the values, and `keys` returns a list of
the keys; there is no guarantee of order. `getvalues` is given a list of keys and
returns a list of values associated with these keys:

    > m = Map{one=1,two=2,three=3}
    > = m:getvalues {'one','three'}
    {1,3}
    > = m:getvalues(m:keys()) == m:values()
    true

When querying the value of a `Map`, it is best to use the `get` method:

    > print(m:get 'one', m:get 'two')
    1     2

The reason is that `m[key]` can be ambiguous; due to the current implementation,
`m["get"]` will always succeed, because if a value is not present in the map, it
will be looked up in the `Map` metatable, which contains a method `get`. There is
currently no simple solution to this annoying restriction.

There are some useful classes which inherit from `Map`. An `OrderedMap` behaves
like a `Map` but keeps its keys in order if you use its `set` method to add keys
and values.  Like all the 'container' classes in Penlight, it defines an `iter`
method for iterating over its values; this will return the keys and values in the
order of insertion; the `keys` and `values` methods likewise.

A `MultiMap` allows multiple values to be associated with a given key. So `set`
(as before) takes a key and a value, but calling it with the same key and a
different value does not overwrite but adds a new value. `get` (or using `[]`)
will return a list of values.

A `Set` can be seen as a special kind of `Map`, where all the values are `true`,
the keys are the values, and the order is not important. So in this case
`Set.values` is defined to return a list of the keys.  Sets can display
themselves, and the basic operations like `union` (`+`) and `intersection` (`*`)
are defined.

    > Set = require 'pl.Set'
    > = Set{'one','two'} == Set{'two','one'}
    true
    > fruit = Set{'apple','banana','orange'}
    > = fruit['banana']
    true
    > = fruit['hazelnut']
    nil
    > = fruit:values()
    {apple,orange,banana}
    > colours = Set{'red','orange','green','blue'}
    > = fruit,colours
    [apple,orange,banana]   [blue,green,orange,red]
    > = fruit+colours
    [blue,green,apple,red,orange,banana]
    > = fruit*colours
    [orange]

There are also the functions `Set.difference` and `Set.symmetric_difference`. The
first answers the question 'what fruits are not colours?' and the second 'what
are fruits and colours but not both?'

    > = fruit - colours
    [apple,banana]
    > = fruit ^ colours
    [blue,green,apple,red,banana]

Adding elements to a set is simply `fruit['peach'] = true` and removing is
`fruit['apple'] = nil` . To make this simplicity work properly, the `Set` class has no
methods - either you use the operator forms or explicitly use `Set.intersect`
etc. In this way we avoid the ambiguity that plagues `Map`.


(See `pl.Map` and `pl.Set`)

### Useful Operations on Tables

@lookup pl.tablex

Some notes on terminology: Lua tables are usually _list-like_ (like an array) or
_map-like_ (like an associative array or dict); they can of course have a
list-like and a map-like part. Some of the table operations only make sense for
list-like tables, and some only for map-like tables. (The usual Lua terminology
is the array part and the hash part of the table, which reflects the actual
implementation used; it is more accurate to say that a Lua table is an
associative map which happens to be particularly efficient at acting like an
array.)

The functions provided in `table` provide all the basic manipulations on Lua
tables, but as we saw with the `List` class, it is useful to build higher-level
operations on top of those functions. For instance, to copy a table involves this
kind of loop:

    local res = {}
    for k,v in pairs(T) do
        res[k] = v
    end
    return res

The `tablex` module provides this as `copy`, which does a _shallow_ copy of a
table. There is also `deepcopy` which goes further than a simple loop in two
ways; first, it also gives the copy the same metatable as the original (so it can
copy objects like `List` above) and any nested tables will also be copied, to
arbitrary depth. There is also `icopy` which operates on list-like tables, where
you can set optionally set the start index of the source and destination as well.
It ensures that any left-over elements will be deleted:

    asserteq(icopy({1,2,3,4,5,6},{20,30}),{20,30})   -- start at 1
    asserteq(icopy({1,2,3,4,5,6},{20,30},2),{1,20,30}) -- start at 2
    asserteq(icopy({1,2,3,4,5,6},{20,30},2,2),{1,30}) -- start at 2, copy from 2

(This code from the `tablex` test module shows the use of `pl.test.asserteq`)

Whereas, `move` overwrites but does not delete the rest of the destination:

    asserteq(move({1,2,3,4,5,6},{20,30}),{20,30,3,4,5,6})
    asserteq(move({1,2,3,4,5,6},{20,30},2),{1,20,30,4,5,6})
    asserteq(move({1,2,3,4,5,6},{20,30},2,2),{1,30,3,4,5,6})

(The difference is somewhat like that between C's `strcpy` and `memmove`.)

To summarize, use `copy` or `deepcopy` to make a copy of an arbitrary table. To
copy into a map-like table, use `update`; to copy into a list-like table use
`icopy`, and `move` if you are updating a range in the destination.

To complete this set of operations, there is `insertvalues` which works like
`table.insert` except that one provides a table of values to be inserted, and
`removevalues` which removes a range of values.

    asserteq(insertvalues({1,2,3,4},2,{20,30}),{1,20,30,2,3,4})
    asserteq(insertvalues({1,2},{3,4}),{1,2,3,4})

Another example:

    > T = require 'pl.tablex'
    > t = {10,20,30,40}
    > = T.removevalues(t,2,3)
    {10,40}
    > = T.insertvalues(t,2,{20,30})
    {10,20,30,40}


In a similar spirit to `deepcopy`, `deepcompare` will take two tables and return
true only if they have exactly the same values and structure.

    > t1 = {1,{2,3},4}
    > t2 = deepcopy(t1)
    > = t1 == t2
    false
    > = deepcompare(t1,t2)
    true

`find` will return the index of a given value in a list-like table. Note that
like `string.find` you can specify an index to start searching, so that all
instances can be found. There is an optional fourth argument, which makes the
search start at the end and go backwards, so we could define `rfind` like so:

    function rfind(t,val,istart)
        return tablex.find(t,val,istart,true)
    end

`find` does a linear search, so it can slow down code that depends on it.  If
efficiency is required for large tables, consider using an _index map_.
`index_map` will return a table where the keys are the original values of the
list, and the associated values are the indices. (It is almost exactly the
representation needed for a _set_.)

    > t = {'one','two','three'}
    > = tablex.find(t,'two')
    2
    > = tablex.find(t,'four')
    nil
    > il = tablex.index_map(t)
    > = il['two']
    2
    > = il.two
    2

A version of `index_map` called `makeset` is also provided, where the values are
just `true`. This is useful because two such sets can be compared for equality
using `deepcompare`:

    > = deepcompare(makeset {1,2,3},makeset {2,1,3})
    true

Consider the problem of determining the new employees that have joined in a
period. Assume we have two files of employee names:

    (last-month.txt)
    smith,john
    brady,maureen
    mongale,thabo

    (this-month.txt)
    smith,john
    smit,johan
    brady,maureen
    mogale,thabo
    van der Merwe,Piet

To find out differences, just make the employee lists into sets, like so:

    require 'pl'

    function read_employees(file)
      local ls = List(io.lines(file)) -- a list of employees
      return tablex.makeset(ls)
    end

    last = read_employees 'last-month.txt'
    this = read_employees 'this-month.txt'

    -- who is in this but not in last?
    diff = tablex.difference(this,last)

    -- in a set, the keys are the values...
    for e in pairs(diff) do print(e) end

    --  *output*
    -- van der Merwe,Piet
    -- smit,johan

The `difference` operation is easy to write and read:

    for e in pairs(this) do
      if not last[e] then
        print(e)
      end
    end

Using `difference` here is not that it is a tricky thing to code, it is that you
are stating your intentions clearly to other readers of your code. (And naturally
to your future self, in six months time.)

`find_if` will search a table using a function. The optional third argument is a
value which will be passed as a second argument to the function. `pl.operator`
provides the Lua operators conveniently wrapped as functions, so the basic
comparison functions are available:

    > ops = require 'pl.operator'
    > = tablex.find_if({10,20,30,40},ops.gt,20)
    3       true

Note that `find_if` will also return the _actual value_ returned by the function,
which of course is usually just  `true` for a boolean function, but any value
which is not `nil` and not `false` can be usefully passed back.

`deepcompare` does a thorough recursive comparison, but otherwise using the
default equality operator. `compare` allows you to specify exactly what function
to use when comparing two list-like tables, and `compare_no_order` is true if
they contain exactly the same elements. Do note that the latter does not need an
explicit comparison function - in this case the implementation is actually to
compare the two sets, as above:

    > = compare_no_order({1,2,3},{2,1,3})
    true
    > = compare_no_order({1,2,3},{2,1,3},'==')
    true

(Note the special string '==' above; instead of saying `ops.gt` or `ops.eq` we
can use the strings '>' or '==' respectively.)

`sort` and `sortv` return iterators that will iterate through the
sorted elements of a table. `sort` iterates by sorted key order, and
`sortv` iterates by sorted value order. For example, given a table
with names and ages, it is trivial to iterate over the elements:

    > t = {john=27,jane=31,mary=24}
    > for name,age in tablex.sort(t) do print(name,age) end
    jane    31
    john    27
    mary    24
    > for name,age in tablex.sortv(t) do print(name,age) end
    mary    24
    john    27
    jane    31

There are several ways to merge tables in PL. If they are list-like, then see the
operations defined by `pl.List`, like concatenation. If they are map-like, then
`merge` provides two basic operations. If the third arg is false, then the result
only contains the keys that are in common between the two tables, and if true,
then the result contains all the keys of both tables. These are in fact
generalized set union and intersection operations:

    > S1 = {john=27,jane=31,mary=24}
    > S2 = {jane=31,jones=50}
    > = tablex.merge(S1, S2, false)
    {jane=31}
    > = tablex.merge(S1, S2, true)
    {mary=24,jane=31,john=27,jones=50}

When working with tables, you will often find yourself writing loops like in the
first example. Loops are second nature to programmers, but they are often not the
most elegant and self-describing way of expressing an operation. Consider the
`map` function, which creates a new table by applying a function to each element
of the original:

    > = map(math.sin, {1,2,3,4})
    {  0.84,  0.91,  0.14, -0.76}
    > = map(function(x) return x*x end, {1,2,3,4})
    {1,4,9,16}

`map` saves you from writing a loop, and the resulting code is often clearer, as
well as being shorter. This is not to say that 'loops are bad' (although you will
hear that from some extremists), just that it's good to capture standard
patterns. Then the loops you do write will stand out and acquire more significance.

`pairmap` is interesting, because the function works with both the key and the
value.

    > t = {fred=10,bonzo=20,alice=4}
    > = pairmap(function(k,v) return v end, t)
    {4,10,20}
    > = pairmap(function(k,v) return k end, t)
    {'alice','fred','bonzo'}

(These are common enough operations that the first is defined as `values` and the
second as `keys`.) If the function returns two values, then the _second_ value is
considered to be the new key:

    > = pairmap(t,function(k,v) return v+10, k:upper() end)
    {BONZO=30,FRED=20,ALICE=14}

`map2` applies a function to two tables:

    > map2(ops.add,{1,2},{10,20})
    {11,22}
    > map2('*',{1,2},{10,20})
    {10,40}

The various map operations generate tables; `reduce` applies a function of two
arguments over a table and returns the result as a scalar:

    > reduce ('+', {1,2,3})
    6
    > reduce ('..', {'one','two','three'})
    'onetwothree'

Finally, `zip` sews different tables together:

    > = zip({1,2,3},{10,20,30})
    {{1,10},{2,20},{3,30}}

Browsing through the documentation, you will find that `tablex` and `List` share
methods.  For instance, `tablex.imap` and `List.map` are basically the same
function; they both operate over the array-part of the table and generate another
table. This can also be expressed as a _list comprehension_ `C 'f(x) for x' (t)`
which makes the operation more explicit. So why are there different ways to do
the same thing? The main reason is that not all tables are Lists: the expression
`ls:map('#')` will return a _list_ of the lengths of any elements of `ls`. A list
is a thin wrapper around a table, provided by the metatable `List`. Sometimes you
may wish to work with ordinary Lua tables; the `List` interface is not a
compulsory way to use Penlight table operations.

### Operations on two-dimensional tables

@lookup pl.array2d

Two-dimensional tables are of course easy to represent in Lua, for instance
`{{1,2},{3,4}}` where we store rows as subtables and index like so `A[col][row]`.
This is the common representation used by matrix libraries like
[LuaMatrix](http://lua-users.org/wiki/LuaMatrix). `pl.array2d` does not provide
matrix operations, since that is the job for a specialized library, but rather
provides generalizations of the higher-level operations provided by `pl.tablex`
for one-dimensional arrays.

`iter` is a useful generalization of `ipairs`. (The extra parameter determines
whether you want the indices as well.)

    > a = {{1,2},{3,4}}
    > for i,j,v in array2d.iter(a,true) do print(i,j,v) end
    1       1       1
    1       2       2
    2       1       3
    2       2       4

Note that you can always convert an arbitrary 2D array into a 'list of lists'
with `List(tablex.map(List,a))`

`map` will apply a function over all elements (notice that extra arguments can be
provided, so this operation is in effect `function(x) return x-1 end`)

    > array2d.map('-',a,1)
    {{0,1},{2,3}}

2D arrays are stored as an array of rows, but columns can be extracted:

    > array2d.column(a,1)
    {1,3}

There are three equivalents to `tablex.reduce`. You can either reduce along the
rows (which is the most efficient) or reduce along the columns. Either one will
give you a 1D array. And `reduce2` will apply two operations: the first one
reduces the rows, and the second reduces the result.

    > array2d.reduce_rows('+',a)
    {3,7}
    > array2d.reduce_cols('+',a)
    {4,6}
    > -- same as tablex.reduce('*',array.reduce_rows('+',a))
    > array2d.reduce2('*','+',a)
    21    `

`tablex.map2` applies an operation to two tables, giving another table.
`array2d.map2` does this for 2D arrays. Note that you have to provide the _rank_
of the arrays involved, since it's hard to always correctly deduce this from the
data:

    > b = {{10,20},{30,40}}
    > a = {{1,2},{3,4}}
    > = array2d.map2('+',2,2,a,b)  -- two 2D arrays
    {{11,22},{33,44}}
    > = array2d.map2('+',1,2,{10,100},a)  -- 1D, 2D
    {{11,102},{13,104}}
    > = array2d.map2('*',2,1,a,{1,-1})  -- 2D, 1D
    {{1,-2},{3,-4}}

Of course, you are not limited to simple arithmetic. Say we have a 2D array of
strings, and wish to print it out with proper right justification. The first step
is to create all the string lengths by mapping `string.len` over the array, the
second is to reduce this along the columns using `math.max` to get maximum column
widths, and last, apply `stringx.rjust` with these widths.

    maxlens = reduce_cols(math.max,map('#',lines))
    lines = map2(stringx.rjust,2,1,lines,maxlens)

There is `product` which returns  the _Cartesian product_ of two 1D arrays. The
result is a 2D array formed from applying the function to all possible pairs from
the two arrays.

    > array2d.product('{}',{1,2},{'a','b'})
    {{{1,'b'},{2,'a'}},{{1,'a'},{2,'b'}}}

There is a set of operations which work in-place on 2D arrays. You can
`swap_rows` and `swap_cols`; the first really is a simple one-liner, but the idea
here is to give the operation a name. `remove_row` and `remove_col` are
generalizations of `table.remove`. Likewise, `extract_rows` and `extract_cols`
are given arrays of indices and discard anything else. So, for instance,
`extract_cols(A,{2,4})` will leave just columns 2 and 4 in the array.

`List.slice` is often useful on 1D arrays; `slice` does the same thing, but is
generally given a start (row,column) and a end (row,column).

    > A = {{1,2,3},{4,5,6},{7,8,9}}
    > B = slice(A,1,1,2,2)
    > write(B)
     1 2
     4 5
    > B = slice(A,2,2)
    > write(B,nil,'%4.1f')
     5.0 6.0
     8.0 9.0

Here `write` is used to print out an array nicely; the second parameter is `nil`,
which is the default (stdout) but can be any file object and the third parameter
is an optional format (as used in `string.format`).

`parse_range` will take a spreadsheet range like 'A1:B2' or 'R1C1:R2C2' and
return the range as four numbers, which can be passed to `slice`. The rule is
that `slice` will return an array of the appropriate shape depending on the
range; if a range represents a row or a column, the result is 1D, otherwise 2D.

This applies to `iter` as well, which can also optionally be given a range:


    > for i,j,v in iter(A,true,2,2) do print(i,j,v) end
    2       2       5
    2       3       6
    3       2       8
    3       3       9

`new` will construct a new 2D array with the given dimensions. You provide an
initial value for the elements, which is interpreted as a function if it's
callable. With `L` being `utils.string_lambda` we then have the following way to
make an _identity matrix_:

    asserteq(
        array.new(3,3,L'|i,j| i==j and 1 or 0'),
        {{1,0,0},{0,1,0},{0,0,1}}
    )

Please note that most functions in `array2d` are _covariant_, that is, they
return an array of the same type as they receive.  In particular, any objects
created with `data.new` or `matrix.new` will remain data or matrix objects when
reshaped or sliced, etc.  Data objects have the `array2d` functions available as
methods.


