#Penlight Lua Libraries

##Why a new set of libraries?

Penlight brings together a set of generally useful pure Lua modules,
focussing on input data handling (such as reading configuration files),
functional programming (such as map, reduce, placeholder expressions,etc),
and OS path management.  Much of the functionality is inspired by the
Python standard libraries.

## Module Overview

### Paths, Files and Directories

  * `path`: queries like `isdir`,`isfile`,`exists`, splitting paths like `dirname` and `basename`
  * `dir`: listing files in directories (`getfiles`,`getallfiles`) and creating/removing directory paths
  * `file`: `copy`,`move`; read/write contents with `read` and `write`
	
### Application Support

  * `app`: `require_here` to rebase `require` to work with main script path; simple argument parsing `parse_args`
  * `lapp`: sophisticated usage-text-driven argument parsing for applications
  * `config`: flexibly read Unix config files and Windows INI files
  * `strict`: check for undefined global variables - can use `strict.module` for modules
  * `utils`,`compat`: Penlight support for unified Lua 5.1/5.2 codebases
  * `types`: predicates like `is_callable` and `is_integer`; extended `type` function.
  
### Extra String Operations
 
  * `utils`: can split a string with a delimiter using `utils.split`
  * `stringx`: extended string functions covering the Python `string` type
  * `stringio`:  open strings for reading, and creating strings using standard Lua IO methods
  * `lexer`:  lexical scanner for splitting text into tokens; special cases for Lua and C
  * `text`:  indenting and dedenting text, wrapping paragraphs; optionally make `%` work as in Python
  * `template`:  small but powerful template expansion engine
  * `sip`:  Simple Input Patterns - higher-level string patterns for parsing text
  
### Extra Table Operations

  * `tablex`: copying, comparing and mapping over
  * `pretty`: pretty-printing Lua tables, and various safe ways to load Lua as data  
  * `List`: implementation of Python 'list' type - slices, concatenation and partitioning
  * `Map`, `Set`, `OrderedMap`: classes for specialized kinds of tables
  * `Data`: reading tabular data into 2D arrays and efficient queries
  * `array2d`: operations on 2D arrays
  * `permute`: generate permutations
  
 ### Iterators, OOP and Functional
  
   * `seq`:  working with iterator pipelines; collecting iterators as tables
   * `class`: a simple reusable class framework
   * `func`: symbolic manipulation of expressions and lambda expressions
   * `utils`: `utils.string_lambda` converts short strings like '|x| x^2' into functions
   * `comprehension`: list comprehensions: `C'x for x=1,4'()=={1,2,3,4}`

##Requirements

The file and directory functions depend on LuaFileSystem (lfs). If you want
dir.copyfile to work elegantly on Windows, then you need Alien. (Both are
present in Lua for Windows.)

##Known Issues

Error handling is still hit and miss.

There are 7581 lines of source and 1764 lines of formal tests, 
which is not an ideal ratio.

Formal documentation for comprehension and luabalanced is missing.

##Installation

The directory structure is

  lua
     pl 
       (module files)
  examples
      (examples)
  tests
      (tests)	          
  docs
    (index.html)
    api
       (index.html)
       modules

All you need to do is copy the pl directory into your Lua module path, which
is typically /usr/local/share/lua/5.1 on a Linux system (of course, you
can set LUA_PATH appropriately.)

With Lua for Windows,  if LUA stands for 'c:\Program Files\Lua\5.1',
then pl goes into LUA\lua, docs goes into LUA\examples\penlight and
both examples and tests goes into LUA\examples

##Building the Documentation

The Users Guide is processed by markdown.lua. If you like the section headers,
you'll need to download my modified version:

http://mysite.mweb.co.za/residents/sdonovan/lua/markdown.zip

docgen.lua will preprocess the documentation (handles @see references)
and use markdown.

gen_modules.bat does the LuaDoc stuff.

###What's new with 0.8b ?

####Features:

pl.app provides useful stuff like simple command-line argument parsing and require_here(), which 
makes subsequent require() calls look in the local directory by preference.

p.file provides useful functions like copy(),move(), read() and write().  (These are aliases to
dir.copyfile(),movefile(),utils.readfile(),writefile())

Custom error trace will only show the functions in user code.

More robust argument checking.

In function arguments, now supports 'string lambdas', e.g. '|x| 2*x'

utils.readfile,writefile now insist on being given filenames. This will cause less confusion.

tablex.search() is new: will look recursively in an arbitrary table; can specify tables not to follow.
tablex.move() will work with source and destination tables the same, with overlapping ranges.

####Bug Fixes:

dir.copyfile() now works fine without Alien on Windows

dir.makepath() and rmtree() had problems.

tablex.compare_no_order() is now O(NlogN), as expected.
tablex.move() had a problem with source size

###What's New with 0.7.0b?

####Features:

utils.is_type(v,tp) can say is_type(s,'string') and is_type(l,List).
utils.is_callable(v) either a function, or has a __call metamethod.

Sequence wrappers: can write things like this:

seq(s):last():filter('<'):copy()

seq:mapmethod(s,name) - map using a named method over a sequence.

seq:enum(s)  If s is a simple sequence, then 
     for i,v in seq.enum(s) do print(i,v) end

seq:take(s,n)  Grab the next n values from a (possibly infinite)
sequence.

In a related change suggested by Flemming Madsden, the in-place List
methods like reverse() and sort() return the list, allowing for
method chaining.

list.join()  explicitly converts using tostring first.

tablex.count_map() like seq.count_map(), but takes an equality function.

tablex.difference()  set difference
tablex.set()  explicit set generator given a list of values

Template.indent_substitute() is a new Template method which adjusts
for indentation and can also substitute templates themselves.

pretty.read(). This reads a Lua table (as dumped by pretty.write)
and attempts to be paranoid about its contents.

sip.match_at_start(). Convenience function for anchored SIP matches.

####Bug Fixes:

tablex.deepcompare() was confused by false boolean values, which
it thought were synonymous with being nil.

pretty.write() did not handle cycles, and could not display tables
with 'holes' properly (Flemming Madsden)

The SIP pattern '$(' was not escaped properly.
sip.match() did not pass on options table.

seq.map() was broken for double-valued sequences.
seq.copy_tuples() did not use default_iter(), so did not e.g. like
table arguments.

dir.copyfile() returns the wrong result for *nix operations.
dir.makepath() was broken for non-Windows paths.

###What's New with 0.6.3?

The map and reduce functions now take the function first, as Nature intended.

The Python-like overloading of '*' for strings has been dropped, since it
is silly. Also, strings are no longer callable; use 's:at(1)' instead of
's(1)' - this tended to cause Obscure Error messages.

Wherever a function argument is expected, you can use the operator strings
like '+','==',etc as well as pl.operator.add, pl.operator.eq, etc.
(see end of pl/operator.lua for the full list.)

tablex now has compare() and compare_no_order(). An explicit set()
function has been added which constructs a table with the specified
keys, all set to a value of true.

List has reduce() and partition() (This is a cool function which 
separates out elements of a list depending on a classifier function.)

There is a new array module which generalizes tablex operations like
map and reduce for two-dimensional arrays.

The famous iterator over permutations from PiL 9.3 has been included.

David Manura's list comprehension library has been included.

Also, utils now contains his memoize function, plus a useful function
args which captures the case where varargs contains nils.

There was a bug with dir.copyfile() where the flag was the wrong way round.

config.lines() had a problem with continued lines.

Some operators were missing in pl.operator; have renamed them to be
consistent with the Lua metamethod names.


