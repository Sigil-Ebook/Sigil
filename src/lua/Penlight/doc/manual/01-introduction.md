## Introduction

### Purpose

It is often said of Lua that it does not include batteries. That is because the
goal of Lua is to produce a lean expressive language that will be used on all
sorts of machines, (some of which don't even have hierarchical filesystems). The
Lua language is the equivalent of an operating system kernel; the creators of Lua
do not see it as their responsibility to create a full software ecosystem around
the language. That is the role of the community.

A principle of software design is to recognize common patterns and reuse them. If
you find yourself writing things like `io.write(string.format('the answer is %d
',42))` more than a number of times then it becomes useful just to define a
function `printf`. This is good, not just because repeated code is harder to
maintain, but because such code is easier to read, once people understand your
libraries.

Penlight captures many such code patterns, so that the intent of your code
becomes clearer. For instance, a Lua idiom to copy a table is `{unpack(t)}`, but
this will only work for 'small' tables (for a given value of 'small') so it is
not very robust. Also, the intent is not clear. So `tablex.deepcopy` is provided,
which will also copy nested tables and and associated metatables, so it can be
used to clone complex objects.

The default error handling policy follows that of the Lua standard libraries: if
a argument is the wrong type, then an error will be thrown, but otherwise we
return `nil,message` if there is a problem. There are some exceptions; functions
like `input.fields` default to shutting down the program immediately with a
useful message. This is more appropriate behaviour for a _script_ than providing
a stack trace. (However, this default can be changed.) The lexer functions always
throw errors, to simplify coding, and so should be wrapped in `pcall`.

If you are used to Python conventions, please note that all indices consistently
start at 1.

The Lua function `table.foreach` has been deprecated in favour of the `for in`
statement, but such an operation becomes particularly useful with the
higher-order function support in Penlight. Note that `tablex.foreach` reverses
the order, so that the function is passed the value and then the key. Although
perverse, this matches the intended use better.

The only important external dependence of Penlight is
[LuaFileSystem](http://keplerproject.github.com/luafilesystem/manual.html)
(`lfs`), and if you want `dir.copyfile` to work cleanly on Windows, you will need
either [alien](http://alien.luaforge.net/) or be using
[LuaJIT](http://luajit.org) as well. (The fallback is to call the equivalent
shell commands.)

### To Inject or not to Inject?

It was realized a long time ago that large programs needed a way to keep names
distinct by putting them into tables (Lua), namespaces (C++) or modules
(Python).  It is obviously impossible to run a company where everyone is called
'Bruce', except in Monty Python skits. These 'namespace clashes' are more of a
problem in a simple language like Lua than in C++, because C++ does more
complicated lookup over 'injected namespaces'.  However, in a small group of
friends, 'Bruce' is usually unique, so in particular situations it's useful to
drop the formality and not use last names. It depends entirely on what kind of
program you are writing, whether it is a ten line script or a ten thousand line
program.

So the Penlight library provides the formal way and the informal way, without
imposing any preference. You can do it formally like:

    local utils = require 'pl.utils'
    utils.printf("%s\n","hello, world!")

or informally like:

    require 'pl'
    utils.printf("%s\n","That feels better")

`require 'pl'` makes all the separate Penlight modules available, without needing
to require them each individually.

Generally, the formal way is better when writing modules, since then there are no
global side-effects and the dependencies of your module are made explicit.

Andrew Starks has contributed another way, which balances nicely between the
formal need to keep the global table uncluttered and the informal need for
convenience. `require'pl.import_into'` returns a function, which accepts a table
for injecting Penlight into, or if no table is given, it passes back a new one.

    local pl = require'pl.import_into'()

The table `pl` is a 'lazy table' which loads modules as needed, so we can then
use `pl.utils.printf` and so forth, without an explicit `require' or harming any
globals.

If you are using `_ENV` with Lua 5.2 to define modules, then here is a way to
make Penlight available within a module:

    local _ENV,M = require 'pl.import_into' ()

    function answer ()
        -- all the Penlight modules are available!
        return pretty.write(utils.split '10 20  30', '')
    end

    return M

The default is to put Penlight into `\_ENV`, which has the unintended effect of
making it available from the module (much as `module(...,package.seeall)` does).
To satisfy both convenience and safety, you may pass `true` to this function, and
then the _module_ `M` is not the same as `\_ENV`, but only contains the exported
functions.

Otherwise, Penlight will _not_ bring in functions into the global table, or
clobber standard tables like 'io'.  require('pl') will bring tables like
'utils','tablex',etc into the global table _if they are used_. This
'load-on-demand' strategy ensures that the whole kitchen sink is not loaded up
front,  so this method is as efficient as explicitly loading required modules.

You have an option to bring the `pl.stringx` methods into the standard string
table. All strings have a metatable that allows for automatic lookup in `string`,
so we can say `s:upper()`. Importing `stringx` allows for its functions to also
be called as methods: `s:strip()`,etc:

    require 'pl'
    stringx.import()

or, more explicitly:

    require('pl.stringx').import()

A more delicate operation is importing tables into the local environment. This is
convenient when the context makes the meaning of a name very clear:

    > require 'pl'
    > utils.import(math)
    > = sin(1.2)
    0.93203908596723

`utils.import` can also be passed a module name as a string, which is first
required and then imported. If used in a module, `import` will bring the symbols
into the module context.

Keeping the global scope simple is very necessary with dynamic languages. Using
global variables in a big program is always asking for trouble, especially since
you do  not have the spell-checking provided by a compiler. The `pl.strict`
module enforces a simple rule: globals must be 'declared'.  This means that they
must be assigned before use; assigning to `nil` is sufficient.

    > require 'pl.strict'
    > print(x)
    stdin:1: variable 'x' is not declared
    > x = nil
    > print(x)
    nil

The `strict` module provided by Penlight is compatible with the 'load-on-demand'
scheme used by `require 'pl`.

`strict` also disallows assignment to global variables, except in the main
program. Generally, modules have no business messing with global scope; if you
must do it, then use a call to `rawset`. Similarly, if you have to check for the
existence of a global, use `rawget`.

If you wish to enforce strictness globally, then just add `require 'pl.strict'`
at the end of `pl/init.lua`, otherwise call it from your main program.

As from 1.1.0, this module provides a `strict.module` function which creates (or
modifies) modules so that accessing an unknown function or field causes an error.

For example,

    -- mymod.lua
    local strict = require 'pl.strict'
    local M = strict.module (...)

    function M.answer ()
        return 42
    end

    return M

If you were to accidently type `mymod.Answer()`, then you would get a runtime
error: "variable 'Answer' is not declared in 'mymod'".

This can be applied to existing modules. You may desire to have the same level
of checking for the Lua standard libraries:

    strict.make_all_strict(_G)

Thereafter a typo such as `math.cosine` will give you an explicit error, rather
than merely returning a `nil` that will cause problems later.

### What are function arguments in Penlight?

Many functions in Penlight themselves take function arguments, like `map` which
applies a function to a list, element by element.  You can use existing
functions, like `math.max`, anonymous functions (like `function(x,y) return x > y
end` ), or operations by name (e.g '*' or '..').  The module `pl.operator` exports
all the standard Lua operations, like the Python module of the same name.
Penlight allows these to be referred to by name, so `operator.gt` can be more
concisely expressed as '>'.

Note that the `map` functions pass any extra arguments to the function, so we can
have `ls:filter('>',0)`, which is a shortcut for
`ls:filter(function(x) return x > 0 end)`.

Finally, `pl.func` supports _placeholder expressions_ in the Boost lambda style,
so that an anonymous function to multiply the two arguments can be expressed as
`\_1*\_2`.

To use them directly, note that _all_ function arguments in Penlight go through
`utils.function_arg`. `pl.func` registers itself with this function, so that you
can directly use placeholder expressions with standard methods:

    > _1 = func._1
    > = List{10,20,30}:map(_1+1)
    {11,21,31}

Another option for short anonymous functions is provided by
`utils.string_lambda`; this is invoked automatically:

    > = List{10,20,30}:map '|x| x + 1'
    {11,21,31}

### Pros and Cons of Loopless Programming

The standard loops-and-ifs 'imperative' style of programming is dominant, and
often seems to be the 'natural' way of telling a machine what to do. It is in
fact very much how the machine does things, but we need to take a step back and
find ways of expressing solutions in a higher-level way.  For instance, applying
a function to all elements of a list is a common operation:

    local res = {}
    for i = 1,#ls do
        res[i] = fun(ls[i])
    end

This can be efficiently and succintly expressed as `ls:map(fun)`. Not only is
there less typing but the intention of the code is clearer. If readers of your
code spend too much time trying to guess your intention by analyzing your loops,
then you have failed to express yourself clearly. Similarly, `ls:filter('>',0)`
will give you all the values in a list greater than zero. (Of course, if you
don't feel like using `List`, or have non-list-like tables, then `pl.tablex`
offers the same facilities. In fact, the `List` methods are implemented using
`tablex` functions.)

A common observation is that loopless programming is less efficient, particularly
in the way it uses memory. `ls1:map2('*',ls2):reduce '+'` will give you the dot
product of two lists, but an unnecessary temporary list is created.  But
efficiency is relative to the actual situation, it may turn out to be _fast
enough_, or may not appear in any crucial inner loops, etc.

Writing loops is 'error-prone and tedious', as Stroustrup says. But any
half-decent editor can be taught to do much of that typing for you. The question
should actually be: is it tedious to _read_ loops?  As with natural language,
programmers tend to read chunks at a time. A for-loop causes no surprise, and
probably little brain activity. One argument for loopless programming is the
loops that you _do_ write stand out more, and signal 'something different
happening here'.  It should not be an all-or-nothing thing, since most programs
require a mixture of idioms that suit the problem.  Some languages (like APL) do
nearly everything with map and reduce operations on arrays, and so solutions can
sometimes seem forced. Wisdom is knowing when a particular idiom makes a
particular problem easy to _solve_ and the solution easy to _explain_ afterwards.

### Generally useful functions.

The function `printf` discussed earlier is included in `pl.utils` because it
makes properly formatted output easier. (There is an equivalent `fprintf` which
also takes a file object parameter, just like the C function.)

Splitting a string using a delimiter is a fairly common operation, hence `split`.

Utility functions like `is_type` help with identifying what
kind of animal you are dealing with.
The Lua `type` function handles the basic types, but can't distinguish between
different kinds of objects, which are all tables. So `is_type` handles both
cases, like `is_type(s,"string")` and `is_type(ls,List)`.

A common pattern when working with Lua varargs is capturing all the arguments in
a table:

    function t(...)
        local args = {...}
        ...
    end

But this will bite you someday when `nil` is one of the arguments, since this
will put a 'hole' in your table. In particular, `#ls` will only give you the size
upto the `nil` value.  Hence the need for `table.pack` - this is a new Lua 5.2
function which Penlight defines also for Lua 5.1.

    function t(...)
        local args,n = table.pack(...)
        for i = 1,n do
          ...
        end
    end

The 'memoize' pattern occurs when you have a function which is expensive to call,
but will always return the same value subsequently. `utils.memoize` is given a
function, and returns another function. This calls the function the first time,
saves the value for that argument, and thereafter for that argument returns the
saved value.  This is a more flexible alternative to building a table of values
upfront, since in general you won't know what values are needed.

    sum = utils.memoize(function(n)
        local sum = 0
        for i = 1,n do sum = sum + i end
        return sum
    end)
    ...
    s = sum(1e8) --takes time!
    ...
    s = sum(1e8) --returned saved value!

Penlight is fully compatible with Lua 5.1, 5.2 and LuaJIT 2. To ensure this,
`utils` also defines the global Lua 5.2
[load](http://www.lua.org/work/doc/manual.html#pdf-load) function as `utils.load`

 * the input (either a string or a function)
 * the source name used in debug information
 * the mode is a string that can have either or both of 'b' or 't', depending on
whether the source is a binary chunk or text code (default is 'bt')
 * the environment for the compiled chunk

Using `utils.load` should reduce the need to call the deprecated function `setfenv`,
and make your Lua 5.1 code 5.2-friendly.

The `utils` module exports `getfenv` and `setfenv` for
Lua 5.2 as well, based on code by Sergey Rozhenko.  Note that these functions can fail
for functions which don't access any globals.

### Application Support

`app.parse_args` is a simple command-line argument parser. If called without any
arguments, it tries to use the global `arg` array.  It returns the _flags_
(options begining with '-') as a table of name/value pairs, and the _arguments_
as an array.  It knows about long GNU-style flag names, e.g. `--value`, and
groups of short flags are understood, so that `-ab` is short for `-a -b`. The
flags result would then look like `{value=true,a=true,b=true}`.

Flags may take values. The command-line `--value=open -n10` would result in
`{value='open',n='10'}`; generally you can use '=' or ':' to separate the flag
from its value, except in the special case where a short flag is followed by an
integer.  Or you may specify upfront that some flags have associated values, and
then the values will follow the flag.

	> require 'pl'
	> flags,args = app.parse_args({'-o','fred','-n10','fred.txt'},{o=true})
	> pretty.dump(flags)
	{o='fred',n='10'}

`parse_args` is not intelligent or psychic; it will not convert any flag values
or arguments for you, or raise errors. For that, have a look at
@{08-additional.md.Command_line_Programs_with_Lapp|Lapp}.

An application which consists of several files usually cannot use `require` to
load files in the same directory as the main script.  `app.require_here()`
ensures that the Lua module path is modified so that files found locally are
found first. In the `examples` directory, `test-symbols.lua` uses this function
to ensure that it can find `symbols.lua` even if it is not run from this directory.

`app.appfile` will create a filename that your application can use to store its
private data, based on the script name. For example, `app.appfile "test.txt"`
from a script called `testapp.lua` produces the following file on my Windows
machine:

    @plain
	C:\Documents and Settings\SJDonova\.testapp\test.txt

and the equivalent on my Linux machine:

    @plain
	/home/sdonovan/.testapp/test.txt

If `.testapp` does not exist, it will be created.

Penlight makes it convenient to save application data in Lua format. You can use
`pretty.dump(t,file)` to write a Lua table in a human-readable form to a file,
and `pretty.read(file.read(file))` to generate the table again, using the
`pretty` module.


### Simplifying Object-Oriented Programming in Lua

Lua is similar to JavaScript in that the concept of class is not directly
supported by the language. In fact, Lua has a very general mechanism for
extending the behaviour of tables which makes it straightforward to implement
classes. A table's behaviour is controlled by its metatable. If that metatable
has a `\_\_index` function or table, this will handle looking up anything which is
not found in the original table. A class is just a table with an `__index` key
pointing to itself. Creating an object involves making a table and setting its
metatable to the class; then when handling `obj.fun`, Lua first looks up `fun` in
the table `obj`, and if not found it looks it up in the class. `obj:fun(a)` is
just short for `obj.fun(obj,a)`. So with the metatable mechanism and this bit of
syntactic sugar, it is straightforward to implement classic object orientation.

    -- animal.lua

    class = require 'pl.class'

    class.Animal()

    function Animal:_init(name)
        self.name = name
    end

    function Animal:__tostring()
      return self.name..': '..self:speak()
    end

    class.Dog(Animal)

    function Dog:speak()
      return 'bark'
    end

    class.Cat(Animal)

    function Cat:_init(name,breed)
        self:super(name)  -- must init base!
        self.breed = breed
    end

    function Cat:speak()
      return 'meow'
    end

    class.Lion(Cat)

    function Lion:speak()
      return 'roar'
    end

    fido = Dog('Fido')
    felix = Cat('Felix','Tabby')
    leo = Lion('Leo','African')

    $ lua -i animal.lua
    > = fido,felix,leo
    Fido: bark      Felix: meow     Leo: roar
    > = leo:is_a(Animal)
    true
    > = leo:is_a(Dog)
    false
    > = leo:is_a(Cat)
    true

All Animal does is define `\_\_tostring`, which Lua will use whenever a string
representation is needed of the object. In turn, this relies on `speak`, which is
not defined. So it's what C++ people would call an abstract base class; the
specific derived classes like Dog define `speak`. Please note that _if_ derived
classes have their own constructors, they must explicitly call the base
constructor for their base class; this is conveniently available as the `super`
method.

Note that (as always) there are multiple ways to implement OOP in Lua; this method
uses the classic 'a class is the __index of its objects' but does 'fat inheritance';
methods from the base class are copied into the new class. The advantage of this is
that you are not penalized for long inheritance chains, for the price of larger classes,
but generally objects outnumber classes! (If not, something odd is going on with your design.)

All such objects will have a `is_a` method, which looks up the inheritance chain
to find a match.  Another form is `class_of`, which can be safely called on all
objects, so instead of `leo:is_a(Animal)` one can say `Animal:class_of(leo)`.

There are two ways to define a class, either `class.Name()` or `Name = class()`;
both work identically, except that the first form will always put the class in
the current environment (whether global or module); the second form provides more
flexibility about where to store the class. The first form does _name_ the class
by setting the `_name` field, which can be useful in identifying the objects of
this type later. This session illustrates the usefulness of having named classes,
if no `__tostring` method is explicitly defined.

    > class.Fred()
    > a = Fred()
    > = a
    Fred: 00459330
    > Alice = class()
    > b = Alice()
    > = b
    table: 00459AE8
    > Alice._name = 'Alice'
    > = b
    Alice: 00459AE8

So `Alice = class(); Alice._name = 'Alice'` is exactly the same as `class.Alice()`.

This useful notation is borrowed from Hugo Etchegoyen's
[classlib](http://lua-users.org/wiki/MultipleInheritanceClasses) which further
extends this concept to allow for multiple inheritance. Notice that the
more convenient form puts the class name in the _current environment_! That is,
you may use it safely within modules using the old-fashioned `module()`
or the new `_ENV` mechanism.

There is always more than one way of doing things in Lua; some may prefer this
style for creating classes:

    local class = require 'pl.class'

    class.Named {
        _init = function(self,name)
            self.name = name
        end;

        __tostring = function(self)
            return 'boo '..self.name
        end;
    }

    b = Named 'dog'
    print(b)
    --> boo dog

Note that you have to explicitly declare `self` and end each function definition
with a semi-colon or comma, since this is a Lua table. To inherit from a base class,
set the special field `_base` to the class in this table.

Penlight provides a number of useful classes; there is `List`, which is a Lua
clone of the standard Python list object, and `Set` which represents sets. There
are three kinds of _map_ defined: `Map`, `MultiMap` (where a key may have
multiple values) and `OrderedMap` (where the order of insertion is remembered.).
There is nothing special about these classes and you may inherit from them.

A powerful thing about dynamic languages is that you can redefine existing classes
and functions, which is often called 'monkey patching' It's entertaining and convenient,
but ultimately anti-social; you may modify `List` but then any other modules using
this _shared_ resource can no longer be sure about its behaviour. (This is why you
must say `stringx.import()` explicitly if you want the extended string methods - it
would be a bad default.)  Lua is particularly open to modification but the
community is not as tolerant of monkey-patching as the Ruby community, say. You may
wish to add some new methods to `List`? Cool, but that's what subclassing is for.

    class.Strings(List)

    function Strings:my_method()
    ...
    end

It's definitely more useful to define exactly how your objects behave
in _unknown_ conditions. All classes have a `catch` method you can use to set
a handler for unknown lookups; the function you pass looks exactly like the
`__index` metamethod.

    Strings:catch(function(self,name)
        return function() error("no such method "..name,2) end
    end)

In this case we're just customizing the error message, but
creative things can be done. Consider this code from `test-vector.lua`:

    Strings:catch(List.default_map_with(string))

    ls = Strings{'one','two','three'}
    asserteq(ls:upper(),{'ONE','TWO','THREE'})
    asserteq(ls:sub(1,2),{'on','tw','th'})

So we've converted a unknown method invocation into a map using the function of
that name found in `string`.  So for a `Vector` (which is a specialization of `List`
for numbers) it makes sense to make `math` the default map so that `v:sin()` makes
sense.

Note that `map` operations return a object of the same type - this is often called
_covariance_. So `ls:upper()` itself returns a `Strings` object.

This is not _always_ what you want, but objects can always be cast to the desired type.
(`cast` doesn't create a new object, but returns the object passed.)

    local sizes = ls:map '#'
    asserteq(sizes, {3,3,5})
    asserteq(utils.type(sizes),'Strings')
    asserteq(sizes:is_a(Strings),true)
    sizes = Vector:cast(sizes)
    asserteq(utils.type(sizes),'Vector')
    asserteq(sizes+1,{4,4,6})

About `utils.type`: it can only return a string for a class type if that class does
in fact have a `_name` field.


_Properties_ are a useful object-oriented pattern. We wish to control access to a
field, but don't wish to force the user of the class to say `obj:get_field()`
etc. This excerpt from `tests/test-class.lua` shows how it is done:


    local MyProps = class(class.properties)
    local setted_a, got_b

    function MyProps:_init ()
        self._a = 1
        self._b = 2
    end

    function MyProps:set_a (v)
        setted_a = true
        self._a = v
    end

    function MyProps:get_b ()
        got_b = true
        return self._b
    end

    local mp = MyProps()

    mp.a = 10

    asserteq(mp.a,10)
    asserteq(mp.b,2)
    asserteq(setted_a and got_b, true)

The convention is that the internal field name is prefixed with an underscore;
when reading `mp.a`, first a check for an explicit _getter_ `get_a` and then only
look for `_a`. Simularly, writing `mp.a` causes the _setter_ `set_a` to be used.

This is cool behaviour, but like much Lua metaprogramming, it is not free. Method
lookup on such objects goes through `\_\_index` as before, but now `\_\_index` is a
function which has to explicitly look up methods in the class, before doing any
property indexing, which is not going to be as fast as field lookup. If however,
your accessors actually do non-trivial things, then the extra overhead could be
worth it.

This is not really intended for _access control_ because external code can write
to `mp._a` directly. It is possible to have this kind of control in Lua, but it
again comes with run-time costs.
