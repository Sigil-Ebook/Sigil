## Functional Programming

### Sequences

@lookup pl.seq

A Lua iterator (in its simplest form) is a function which can be repeatedly
called to return a set of one or more values. The `for in` statement understands
these iterators, and loops until the function returns `nil`. There are standard
sequence adapters for tables in Lua (`ipairs` and `pairs`), and `io.lines`
returns an iterator over all the lines in a file. In the Penlight libraries, such
iterators are also called _sequences_.  A sequence of single values (say from
`io.lines`) is called _single-valued_, whereas the sequence defined by `pairs` is
_double-valued_.

`pl.seq` provides a number of useful iterators, and some functions which operate
on sequences.  At first sight this example looks like an attempt to write Python
in Lua, (with the sequence being inclusive):

    > for i in seq.range(1,4) do print(i) end
    1
    2
    3
    4

But `range` is actually equivalent to Python's `xrange`, since it generates a
sequence, not a list.  To get a list, use `seq.copy(seq.range(1,10))`, which
takes any single-value sequence and makes a table from the result. `seq.list` is
like `ipairs` except that it does not give you the index, just the value.

    > for x in seq.list {1,2,3} do print(x) end
    1
    2
    3

`enum` takes a sequence and turns it into a double-valued sequence consisting of
a sequence number and the value, so `enum(list(ls))` is actually equivalent to
`ipairs`. A more interesting example prints out a file with line numbers:

    for i,v in seq.enum(io.lines(fname)) do print(i..' '..v) end

Sequences can be _combined_, either by 'zipping' them or by concatenating them.

    > for x,y in seq.zip(l1,l2) do print(x,y) end
    10      1
    20      2
    30      3
    > for x in seq.splice(l1,l2) do print(x) end
    10
    20
    30
    1
    2
    3

`seq.printall` is useful for printing out single-valued sequences, and provides
some finer control over formating, such as a delimiter, the number of fields per
line, and a format string to use (@see string.format)

    > seq.printall(seq.random(10))
    0.0012512588885159 0.56358531449324 0.19330423902097 ....
    > seq.printall(seq.random(10), ',', 4, '%4.2f')
    0.17,0.86,0.71,0.51
    0.30,0.01,0.09,0.36
    0.15,0.17,

`map` will apply a function to a sequence.

    > seq.printall(seq.map(string.upper, {'one','two'}))
    ONE TWO
    > seq.printall(seq.map('+', {10,20,30}, 1))
    11 21 31

`filter` will filter a sequence using a boolean function (often called a
_predicate_). For instance, this code only prints lines in a file which are
composed of digits:

    for l in seq.filter(io.lines(file), stringx.isdigit) do print(l) end

The following returns a table consisting of all the positive values in the
original table (equivalent to `tablex.filter(ls, '>', 0)`)

    ls = seq.copy(seq.filter(ls, '>', 0))

We're already encounted `seq.sum` when discussing `input.numbers`. This can also
be expressed with `seq.reduce`:

    > seq.reduce(function(x,y) return x + y end, seq.list{1,2,3,4})
    10

`seq.reduce` applies a binary function in a recursive fashion, so that:

    reduce(op,{1,2,3}) => op(1,reduce(op,{2,3}) => op(1,op(2,3))

it's now possible to easily generate other cumulative operations; the standard
operations declared in `pl.operator` are useful here:

    > ops = require 'pl.operator'
    > -- can also say '*' instead of ops.mul
    > = seq.reduce(ops.mul,input.numbers '1 2 3 4')
    24

There are functions to extract statistics from a sequence of numbers:

    > l1 = List {10,20,30}
    > l2 = List {1,2,3}
    > = seq.minmax(l1)
    10      30
    > = seq.sum(l1)
    60      3

It is common to get sequences where values are repeated, say the words in a file.
`count_map` will take such a sequence and count the values, returning a table
where the _keys_ are the unique values, and the value associated with each key is
the number of times they occurred:

    > t = seq.count_map {'one','fred','two','one','two','two'}
    > = t
    {one=2,fred=1,two=3}

This will also work on numerical sequences, but you cannot expect the result to
be a proper list, i.e. having no 'holes'. Instead, you always need to use `pairs`
to iterate over the result - note that there is a hole at index 5:

    > t = seq.count_map {1,2,4,2,2,3,4,2,6}
    > for k,v in pairs(t) do print(k,v) end
    1       1
    2       4
    3       1
    4       2
    6       1

`unique` uses `count_map` to return a list of the unique values, that is, just
the keys of the resulting table.

`last` turns a single-valued sequence into a double-valued sequence with the
current value and the last value:

    > for current,last in seq.last {10,20,30,40} do print (current,last) end
    20      10
    30      20
    40      30

This makes it easy to do things like identify repeated lines in a file, or
construct differences between values. `filter` can handle double-valued sequences
as well, so one could filter such a sequence to only return cases where the
current value is less than the last value by using `operator.lt` or just '<'.
This code then copies the resulting code into a table.

    > ls = {10,9,10,3}
    > = seq.copy(seq.filter(seq.last(s),'<'))
    {9,3}


### Sequence Wrappers

The functions in `pl.seq` cover the common patterns when dealing with sequences,
but chaining these functions together can lead to ugly code. Consider the last
example of the previous section; `seq` is repeated three times and the resulting
expression has to be read right-to-left. The first issue can be helped by local
aliases, so that the expression becomes `copy(filter(last(s),'<'))` but the
second issue refers to the somewhat unnatural order of functional application.
We tend to prefer reading operations from left to right, which is one reason why
object-oriented notation has become popular. Sequence adapters allow this
expression to be written like so:

    seq(s):last():filter('<'):copy()

With this notation, the operation becomes a chain of method calls running from
left to right.

'Sequence' is not a basic Lua type, they are generally functions or callable
objects. The expression `seq(s)` wraps a sequence in a _sequence wrapper_, which
is an object which understands all the functions in `pl.seq` as methods. This
object then explicitly represents sequences.

As a special case, the  constructor (which is when you call the table `seq`) will
make a wrapper for a plain list-like table. Here we apply the length operator to
a sequence of strings, and print them out.

    > seq{'one','tw','t'} :map '#' :printall()
    3 2 1

As a convenience, there is a function `seq.lines` which behaves just like
`io.lines` except it wraps the result as an explicit sequence type. This takes
the first 10 lines from standard input, makes it uppercase, turns it into a
sequence with a count and the value, glues these together with the concatenation
operator, and finally prints out the sequence delimited by a newline.

    seq.lines():take(10):upper():enum():map('..'):printall '\n'

Note the method `upper`, which is not a `seq` function. if an unknown method is
called, sequence wrappers apply that method to all the values in the sequence
(this is implicit use of `mapmethod`)

It is straightforward to create custom sequences that can be used in this way. On
Unix, `/dev/random` gives you an _endless_ sequence of random bytes, so we use
`take` to limit the sequence, and then `map` to scale the result into the desired
range. The key step is to use `seq` to wrap the iterator function:

    -- random.lua
    local seq = require 'pl.seq'

    function dev_random()
        local f = io.open('/dev/random')
        local byte = string.byte
        return seq(function()
            -- read two bytes into a string and convert into a 16-bit number
            local s = f:read(2)
            return byte(s,1) + 256*byte(s,2)
        end)
    end

    -- print 10 random numbers from 0 to 1 !
    dev_random():take(10):map('%',100):map('/',100):printall ','


Another Linux one-liner depends on the `/proc` filesystem and makes a list of all
the currently running processes:

    pids = seq(lfs.dir '/proc'):filter(stringx.isdigit):map(tonumber):copy()

This version of Penlight has an experimental feature which relies on the fact
that _all_ Lua types can have metatables, including functions. This makes
_implicit sequence wrapping_ possible:

    > seq.import()
    > seq.random(5):printall(',',5,'%4.1f')
     0.0, 0.1, 0.4, 0.1, 0.2

This avoids the awkward `seq(seq.random(5))` construction. Or the iterator can
come from somewhere else completely:

    > ('one two three'):gfind('%a+'):printall(',')
    one,two,three,

After `seq.import`, it is no longer necessary to explicitly wrap sequence
functions.

But there is a price to pay for this convenience. _Every_ function is affected,
so that any function can be used, appropriate or not:

    > math.sin:printall()
    ..seq.lua:287: bad argument #1 to '(for generator)' (number expected, got nil)
    > a = tostring
    > = a:find(' ')
    function: 0042C920

What function is returned? It's almost certain to be something that makes no
sense in the current context. So implicit sequences may make certain kinds of
programming mistakes harder to catch - they are best used for interactive
exploration and small scripts.

<a id="comprehensions"/>

### List Comprehensions

List comprehensions are a compact way to create tables by specifying their
elements. In Python, you can say this:

    ls = [x for x in range(5)]  # == [0,1,2,3,4]

In Lua, using `pl.comprehension`:

    > C = require('pl.comprehension').new()
    > = C ('x for x=1,10') ()
    {1,2,3,4,5,6,7,8,9,10}

`C` is a function which compiles a list comprehension _string_ into a _function_.
In this case, the function has no arguments. The parentheses are redundant for a
function taking a string argument, so this works as well:

    > = C 'x^2 for x=1,4' ()
    {1,4,9,16}
    > = C '{x,x^2} for x=1,4' ()
    {{1,1},{2,4},{3,9},{4,16}}

Note that the expression can be _any_ function of the variable `x`!

The basic syntax so far is `<expr> for <set>`, where `<set>` can be anything that
the Lua `for` statement understands. `<set>` can also just be the variable, in
which case the values will come from the _argument_ of the comprehension. Here
I'm emphasizing that a comprehension is a function which can take a list argument:

    > = C '2*x for x' {1,2,3}
    {2,4,6}
    > dbl = C '2*x for x'
    > = dbl {10,20,30}
    {20,40,60}

Here is a somewhat more explicit way of saying the same thing; `_1` is a
_placeholder_ refering to the _first_ argument passed to the comprehension.

    > = C '2*x for _,x in pairs(_1)' {10,20,30}
    {20,40,60}
    > = C '_1(x) for x'(tostring,{1,2,3,4})
    {'1','2','3','4'}

This extended syntax is useful when you wish to collect the result of some
iterator, such as `io.lines`. This comprehension creates a function which creates
a table of all the lines in a file:

    > f = io.open('array.lua')
    > lines = C 'line for line in _1:lines()' (f)
    > = #lines
    118

There are a number of functions that may be applied to the result of a
comprehension:

    > = C 'min(x for x)' {1,44,0}
    0
    > = C 'max(x for x)' {1,44,0}
    44
    > = C 'sum(x for x)' {1,44,0}
    45

(These are equivalent to a reduce operation on a list.)

After the `for` part, there may be a condition, which filters the output. This
comprehension collects the even numbers from a list:

    > = C 'x for x if x % 2 == 0' {1,2,3,4,5}
    {2,4}

There may be a number of `for` parts:

    > = C '{x,y} for x = 1,2 for y = 1,2' ()
    {{1,1},{1,2},{2,1},{2,2}}
    > = C '{x,y} for x for y' ({1,2},{10,20})
    {{1,10},{1,20},{2,10},{2,20}}

These comprehensions are useful when dealing with functions of more than one
variable, and are not so easily achieved with the other Penlight functional forms.

<a id="func"/>

### Creating Functions from Functions

@lookup pl.func

Lua functions may be treated like any other value, although of course you cannot
multiply or add them. One operation that makes sense is _function composition_,
which chains function calls (so `(f * g)(x)` is `f(g(x))`.)

    > func = require 'pl.func'
    > printf = func.compose(io.write,string.format)
    > printf("hello %s\n",'world')
    hello world
    true

Many functions require you to pass a function as an argument, say to apply to all
values of a sequence or as a callback. Often useful functions have the wrong
number of arguments. So there is a need to construct a function of one argument
from one of two arguments, _binding_ the extra argument to a given value.

_partial application_ takes a function of n arguments and returns a function of n-1
arguments where the first argument is bound to some value:

    > p2 = func.bind1(print,'start>')
    > p2('hello',2)
    start>  hello   2
    > ops = require 'pl.operator'
    > = tablex.filter({1,-2,10,-1,2},bind1(ops.gt,0))
    {-2,-1}
    > tablex.filter({1,-2,10,-1,2},bind1(ops.le,0))
    {1,10,2}

The last example unfortunately reads backwards, because `bind1` alway binds the
first argument!  Also unfortunately, in my youth I confused 'currying' with
'partial application', so the old name for `bind1` is `curry` - this alias still exists.

This is a specialized form of function argument binding. Here is another way
to say the `print` example:

    > p2 = func.bind(print,'start>',func._1,func._2)
    > p2('hello',2)
    start>  hello   2

where `_1` and `_2` are _placeholder variables_, corresponding to the first and
second argument respectively.

Having `func` all over the place is distracting, so it's useful to pull all of
`pl.func` into the local context. Here is the filter example, this time the right
way around:

    > utils.import 'pl.func'
    > tablex.filter({1,-2,10,-1,2},bind(ops.gt, _1, 0))
    {1,10,2}

`tablex.merge` does a general merge of two tables. This example shows the
usefulness of binding the last argument of a function.

    > S1 = {john=27, jane=31, mary=24}
    > S2 = {jane=31, jones=50}
    > intersection = bind(tablex.merge, _1, _2, false)
    > union = bind(tablex.merge, _1, _2, true)
    > = intersection(S1,S2)
    {jane=31}
    > = union(S1,S2)
    {mary=24,jane=31,john=27,jones=50}

When using `bind` with `print`, we got a function of precisely two arguments,
whereas we really want our function to use varargs like `print`. This is the role
of `_0`:

    > _DEBUG = true
    > p = bind(print,'start>', _0)
    return function (fn,_v1)
        return function(...) return fn(_v1,...) end
    end

    > p(1,2,3,4,5)
    start>  1       2       3       4       5

I've turned on the global `_DEBUG` flag, so that the function generated is
printed out. It is actually a function which _generates_ the required function;
the first call _binds the value_ of `_v1` to 'start>'.

### Placeholder Expressions

A common pattern in Penlight is a function which applies another function to all
elements in a table or a sequence, such as `tablex.map` or `seq.filter`. Lua does
anonymous functions well, although they can be a bit tedious to type:

    > = tablex.map(function(x) return x*x end, {1,2,3,4})
    {1,4,9,16}

`pl.func` allows you to define _placeholder expressions_, which can cut down on
the typing required, and also make your intent clearer. First, we bring contents
of `pl.func` into our context, and then supply an expression using placeholder
variables, such as `_1`,`_2`,etc. (C++ programmers will recognize this from the
Boost libraries.)

    > utils.import 'pl.func'
    > = tablex.map(_1*_1, {1,2,3,4})
    {1,4,9,16}

Functions of up to 5 arguments can be generated.

    > = tablex.map2(_1+_2,{1,2,3}, {10,20,30})
    {11,22,33}

These expressions can use arbitrary functions, altho they must first be
registered with the functional library. `func.register` brings in a single
function, and `func.import` brings in a whole table of functions, such as `math`.

    > sin = register(math.sin)
    > = tablex.map(sin(_1), {1,2,3,4})
    {0.8414709848079,0.90929742682568,0.14112000805987,-0.75680249530793}
    > import 'math'
    > = tablex.map(cos(2*_1),{1,2,3,4})
    {-0.41614683654714,-0.65364362086361,0.96017028665037,-0.14550003380861}

A common operation is calling a method of a set of objects:

    > = tablex.map(_1:sub(1,1), {'one','four','x'})
    {'o','f','x'}

There are some restrictions on what operators can be used in PEs. For instance,
because the `__len` metamethod cannot be overriden by plain Lua tables, we need
to define a special function to express `#_1':

    > = tablex.map(Len(_1), {'one','four','x'})
    {3,4,1}

Likewise for comparison operators, which cannot be overloaded for _different_
types, and thus also have to be expressed as a special function:

    > = tablex.filter(Gt(_1,0), {1,-1,2,4,-3})
    {1,2,4}

It is useful to express the fact that a function returns multiple values. For
instance, `tablex.pairmap`  expects a function that will be called with the key
and the value, and returns the new value and the key, in that order.

    > = pairmap(Args(_2,_1:upper()),{fred=1,alice=2})
    {ALICE=2,FRED=1}

PEs cannot contain `nil` values, since PE function arguments are represented as
an array. Instead, a special value called `Nil` is provided.  So say
`_1:f(Nil,1)` instead of `_1:f(nil,1)`.

A placeholder expression cannot be automatically used as a Lua function. The
technical reason is that the call operator must be overloaded to construct
function calls like `_1(1)`.  If you want to force a PE to return a function, use
`func.I`.

    > = tablex.map(_1(10),{I(2*_1),I(_1*_1),I(_1+2)})
    {20,100,12}

Here we make a table of functions taking a single argument, and then call them
all with a value of 10.

The essential idea with PEs is to 'quote' an expression so that it is not
immediately evaluated, but instead turned into a function that can be applied
later to some arguments. The basic mechanism is to wrap values and placeholders
so that the usual Lua operators have the effect of building up an _expression
tree_. (It turns out that you can do _symbolic algebra_ using PEs, see
`symbols.lua` in the examples directory, and its test runner `testsym.lua`, which
demonstrates symbolic differentiation.)

The rule is that if any operator has a PE operand, the result will be quoted.
Sometimes we need to quote things explicitly. For instance, say we want to pass a
function to a filter that must return true if the element value is in a set.
`set[_1]` is the obvious expression, but it does not give the desired result,
since it evaluates directly, giving `nil`. Indexing works differently than a
binary operation like addition (set+_1 _is_ properly quoted) so there is a need
for an explicit quoting or wrapping operation. This is the job of the `_`
function; the PE in this case should be `_(set)[_1]`.  This works for functions
as well, as a convenient alternative to registering functions: `_(math.sin)(_1)`.
This is equivalent to using the `lines' method:

    for line in I(_(f):read()) do print(line) end

Now this will work for _any_ 'file-like' object which which has a `read` method
returning the next line. If you had a LuaSocket client which was being 'pushed'
by lines sent from a server, then `_(s):receive '*l'` would create an iterator
for accepting input. These forms can be convenient for adapting your data flow so
that it can be passed to the sequence functions in `pl.seq'.

Placeholder expressions can be mixed with sequence wrapper expressions.
`lexer.lua` will give us a double-valued sequence of tokens, where the first
value is a type, and the second is a value. We filter out only the values where
the type is 'iden', extract the actual value using `map`, get the unique values
and finally copy to a list.

    > str = 'for i=1,10 do for j = 1,10 do print(i,j) end end'
    > = seq(lexer.lua(str)):filter('==','iden'):map(_2):unique():copy()
    {i,print,j}

This is a particularly intense line (and I don't always suggest making everything
a one-liner!); the key is the behaviour of `map`, which will take both values of
the sequence, so `_2` returns the value part. (Since `filter` here takes extra
arguments, it only operates on the type values.)

There are some performance considerations to using placeholder expressions.
Instantiating a PE requires constructing and compiling a function, which is not
such a fast operation. So to get best performance, factor out PEs from loops like
this;

    local fn = I(_1:f() + _2:g())
    for i = 1,n do
        res[i] = tablex.map2(fn,first[i],second[i])
    end


