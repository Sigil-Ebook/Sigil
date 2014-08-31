## Technical Choices

### Modularity and Granularity

In an ideal world, a program should only load the libraries it needs. Penlight is
intended to work in situations where an extra 100Kb of bytecode could be a
problem. It is straightforward but tedious to load exactly what you need:

    local data = require 'pl.data'
    local List = require 'pl.List'
    local array2d = require 'pl.array2d'
    local seq = require 'pl.seq'
    local utils = require 'pl.utils'

This is the style that I follow in Penlight itself, so that modules don't mess
with the global environment; also, `stringx.import()` is not used because it will
update the global `string` table.

But `require 'pl'` is more convenient in scripts; the question is how to ensure
that one doesn't load the whole kitchen sink as the price of convenience. The
strategy is to only load modules when they are referenced. In 'init.lua' (which
is loaded by `require 'pl'`) a metatable is attached to the global table with an
`__index` metamethod. Any unknown name is looked up in the list of modules, and
if found, we require it and make that module globally available. So when
`tablex.deepcompare` is encountered, looking up `tablex` causes 'pl.tablex' to be
required.  .

Modifying the behaviour of the global table has consequences. For instance, there
is the famous module `strict` which comes with Lua itself (perhaps the only
standard Lua module written in Lua itself) which also does this modification so
that global variiables must be defined before use.  So the implementation in
'init.lua' allows for a 'not found' hook, which 'pl.strict.lua' uses.  Other
libraries may install their own metatables for `_G`, but Penlight will now
forward any unknown name to the `__index` defined by the original metatable.

But the strategy is worth the effort: the old 'kitchen sink' 'init.lua' would
pull in about 260K of bytecode, whereas now typical programs use about 100K less,
and short scripts even better - for instance, if they were only needing
functionality in `utils`.

There are some functions which mark their output table with a special metatable,
when it seems particularly appropriate. For instance, `tablex.makeset` creates a
`Set`, and `seq.copy` creates a `List`. But this does not automatically result in
the loading of `pl.Set` and `pl.List`; only if you try to access any of these
methods.  In 'utils.lua', there is an exported table called `stdmt`:

    stdmt = { List = {}, Map = {}, Set = {}, MultiMap = {} }

If you go through 'init.lua', then these plain little 'identity' tables get an
`__index` metamethod which forces the loading of the full functionality. Here is
the code from 'list.lua' which starts the ball rolling for lists:

    List = utils.stdmt.List
    List.__index = List
    List._name = "List"
    List._class = List

The 'load-on-demand' strategy helps to modularize the library.  Especially for
more casual use, `require 'pl'` is a good compromise between convenience and
modularity.

In this current version, I have generally reduced the amount of trickery
involved. Previously, `Map` was defined in `pl.class`; now it is sensibly defined
in `pl.Map`; `pl.class` only contains the basic class mechanism (and returns that
function.) For consistency, `List` is returned directly by  `require 'pl.List'`
(note the uppercase 'L'),  Also, the amount of module dependencies in the
non-core libraries like `pl.config` have been reduced.

### Defining what is Callable

'utils.lua' exports `function_arg` which is used extensively throughout Penlight.
It defines what is meant by 'callable'.  Obviously true functions are immediately
passed back. But what about strings? The first option is that it represents an
operator in 'operator.lua', so that '<' is just an alias for `operator.lt`.

We then check whether there is a _function factory_ defined for the metatable of
the value.

(It is true that strings can be made callable, but in practice this turns out to
be a cute but dubious idea, since _all_ strings share the same metatable. A
common programming error is to pass the wrong kind of object to a function, and
it's better to get a nice clean 'attempting to call a string' message rather than
some obscure trace from the bowels of your library.)

The other module that registers a function factory is `pl.func`. Placeholder
expressions cannot be directly calleable, and so need to be instantiated and
cached in as efficient way as possible.

(An inconsistency is that `utils.is_callable` does not do this thorough check.)


