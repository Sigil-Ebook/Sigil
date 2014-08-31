## 1.3.0

### Changes

  - class: RIP base method - not possible to implement correctly
  - lapp: short flags can now always be followed directly by their value, for instance,
`-I/usr/include/lua/5.1`
  - Date: new explicit `Date.Interval` class; `toUTC/toLocal` return new object; `Date.__tostring`
always returns ISO 8601 times for exact serialization.  `+/-` explicit operators. Date objects
are explicitly flagged as being UTC or not.

### Fixes

  - class: super method fixed.
  - Date: DST is now accounted for properly.
  - Date: weekday calculation borked.

### Features

  - All tests pass with no-5.1-compatible Lua 5.2; now always uses `utils.load` and
`utils.unpack` is always available.
  - types: new module containing `utils.is_xxx` methods plus new `to_bool`.
  - class: can be passed methods in a table (see `test=klass.lua`). This is
particularly convenient for using from Moonscript.
  - general documentation improvements, e.g `class`

## 1.2.1

### Changes

  - utils.set(get)fenv always defined (_not_ set as globals for 5.2 anymore!).
    These are defined in new module pl.compat, but still available through utils.
  - class.Frodo now puts 'Frodo' in _current environment_

### Fixes

  - lapp.add_type was broken (Pete Kazmier)
  - class broke with classes that redefined __newindex
  - Set.isdisjoint was broken because of misspelling; default ctor Set() now works as expected
  - tablex.transform was broken; result now has same keys as original (CoolistheName007)
  - xml match not handling empty matches (royalbee)
  - pl.strict: assigning nil to global declares it, as God intended. (Pierre Chapuis)
  - tests all work with pl.strict
  - 5.2 compatible load now respects mode
  - tablex.difference thought that a value of `false` meant 'not present' (Andrew Starke)

### Features

  - tablex.sort(t) iterates over sorted keys, tablex.sortv(t) iterates over sorted values (Pete Kazmier)
  - tablex.readonly(t) creates a read-only proxy for a table (John Schember)
  - utils.is_empty(o) true if o==nil, o is an empty table, or o is an empty string (John Schember)
  - utils.executeex(cmd,bin) returns true if successful, return code, plus stdout and stderr output as strings. (tieske)
  - class method base for calling inherited methods (theypsilon)
  - class supports pre-constructor _create for making a custom self (used in pl.List)
  - xml HTML mode improvements - can parse non-trivial well-formed HTML documents.
    xml.parsehtml is a parse function, no longer a flag
  - if a LOM document has ordered attributes, use these when stringifying
  - xml.tostring has yet another extra parm to force prefacing with <?xml...>
  - lapp boolean flags may have `true` default
  - lapp slack mode where 'short' flags can be multi-char
  - test.asserteq etc take extra arg, which is extra level where error must be reported at
  - path.currentdir,chdir,rmdir,mkdir and dir as alias to lfs are exported; no dependencies on luafilesystem outside pl.path, making it easier to plug in different implementations.



## 0.9.7

### Lua 5.2 compatibility

(These are all now defined in pl.utils)

- setfenv, getfenv defined for Lua 5.2 (by Sergey Rozhenko)

### Changes

- array2d.flatten is new
- OrderedMap:insert is new

### Fixes

- seq.reduce re-implemented to give correct order (Carl Ådahl)
- seq.unique was broken: new test
- tablex.icopy broken for last argument; new test
- utils.function_arg last parm 'msg' was missing
- array2d.product was broken; more sensible implementation
- array2d.range, .slice, .write were broken
- text optional operator % overload broken for 'fmt % fun'; new tests
- a few occurances of non-existent function utils.error removed


## 0.9.6

### Lua 5.2 compatibility

- Bad string escape in tests fixed

### Changes

- LuaJIT FFI used on Windows for Copy/MoveFile functionality

### Fixes

- Issue 13 seq.sort now calls seq.copy
- issue 14 bad pattern to escape trailing separators in path.abspath
- lexer: string tokens broken with some combinations
- lexer: long comments broken for Lua and C
- stringx.split behaves according to Python spec; extra parm meaning 'max splits'
- stringx.title behaves according to Python spec
- stringx.endswith broken for 2nd arg being table of postfixes
- OrderedMap.set broken when value was nil and key did not exist in map; ctor throws
  error if unhappy

## 0.9.5

### Lua 5.2 compatibility

 - defines Lua 5.2 beta compatible load()
 - defines table.pack()

### New functions

 - stringx.title(): translates "a dog's day" to "A Dog's Day"
 - path.normpath(): translates 'A//B','A/./B' and 'A/C/../B' to 'A/B'
 - utils.execute(): returns ok,return-code: compatible with 5.1 and 5.2

### Fixes

 - pretty.write() _always_ returns a string, but will return also an error string
if the argument is not a table. Non-integer indices between 1 and #t are no longer falsely considered part of the array
 - stringx.expandtabs() now works like the Python string method; it will expand each field up to the next tab stop
 - path.normcase() was broken, because of a misguided attempt to normalize the path.
 - UNC specific fix to path.abspath()
 - UNC paths recognized as absolute; dir.makedir() works here
 - utils.quit() varargs broken, e.g. utils.quit("answer was %d",42)
 - some stray globals caused trouble with 'strict'
