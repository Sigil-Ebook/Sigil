## Additional Libraries

Libraries in this section are no longer considered to be part of the Penlight
core, but still provide specialized functionality when needed.

<a id="sip"/>

### Simple Input Patterns

Lua string pattern matching is very powerful, and usually you will not need a
traditional regular expression library.  Even so, sometimes Lua code ends up
looking like Perl, which happens because string patterns are not always the
easiest things to read, especially for the casual reader.  Here is a program
which needs to understand three distinct date formats:

    -- parsing dates using Lua string patterns
    months={Jan=1,Feb=2,Mar=3,Apr=4,May=5,Jun=6,
    Jul=7,Aug=8,Sep=9,Oct=10,Nov=11,Dec=12}

    function check_and_process(d,m,y)
        d = tonumber(d)
        m = tonumber(m)
        y = tonumber(y)
        ....
    end

    for line in f:lines() do
        -- ordinary (English) date format
        local d,m,y = line:match('(%d+)/(%d+)/(%d+)')
        if d then
            check_and_process(d,m,y)
        else -- ISO date??
            y,m,d = line:match('(%d+)%-(%d+)%-(%d+)')
            if y then
                check_and_process(d,m,y)
            else -- <day> <month-name> <year>?
                d,mm,y = line:match('%(d+)%s+(%a+)%s+(%d+)')
                m = months[mm]
                check_and_process(d,m,y)
            end
        end
    end

These aren't particularly difficult patterns, but already typical issues are
appearing, such as having to escape '-'. Also, `string.match` returns its
captures, so that we're forced to use a slightly awkward nested if-statement.

Verification issues will further cloud the picture, since regular expression
people try to enforce constraints (like year cannot be more than four digits)
using regular expressions, on the usual grounds that one shouldn't stop using a
hammer when one is enjoying oneself.

`pl.sip` provides a simple, intuitive way to detect patterns in strings and
extract relevant parts.

    > sip = require 'pl.sip'
    > dump = require('pl.pretty').dump
    > res = {}
    > c = sip.compile 'ref=$S{file}:$d{line}'
    > = c('ref=hello.c:10',res)
    true
    > dump(res)
    {
      line = 10,
      file = "hello.c"
    }
    > = c('ref=long name, no line',res)
    false

`sip.compile` creates a pattern matcher function, which is given a string and a
table. If it matches the string, then `true` is returned and the table is
populated according to the _named fields_ in the pattern.

Here is another version of the date parser:

    -- using SIP patterns
    function check(t)
        check_and_process(t.day,t.month,t.year)
    end

    shortdate = sip.compile('$d{day}/$d{month}/$d{year}')
    longdate = sip.compile('$d{day} $v{mon} $d{year}')
    isodate = sip.compile('$d{year}-$d{month}-$d{day}')

    for line in f:lines() do
        local res = {}
        if shortdate(str,res) then
            check(res)
        elseif isodate(str,res) then
            check(res)
        elseif longdate(str,res) then
            res.month = months[res.mon]
            check(res)
        end
    end

SIP patterns start with '$', then a one-letter type, and then an optional
variable in curly braces.

    Type    Meaning
    v         variable, or identifier.
    i          possibly signed integer
    f          floating-point number
    r          'rest of line'
    q         quoted string (either ' or ")
    p         a path name
    (         anything inside (...)
    [         anything inside [...]
    {         anything inside {...}
    <         anything inside <...>
    [---------------------------------]
    S         non-space
    d         digits
    ...

If a type is not one of v,i,f,r or q, then it's assumed to be one of the standard
Lua character classes.  Any spaces you leave in your pattern will match any
number of spaces.  And any 'magic' string characters will be escaped.

SIP captures (like `$v{mon}`) do not have to be named. You can use just `$v`, but
you have to be consistent; if a pattern contains unnamed captures, then all
captures must be unnamed. In this case, the result table is a simple list of
values.

`sip.match` is a useful shortcut if you like your matches to be 'in place'. (It
caches the result, so it is not much slower than explicitly using `sip.compile`.)

    > sip.match('($q{first},$q{second})','("john","smith")',res)
    true
    > res
    {second='smith',first='john'}
    > res = {}
    > sip.match('($q,$q)','("jan","smit")',res)  -- unnamed captures
    true
    > res
    {'jan','smit'}
    > sip.match('($q,$q)','("jan", "smit")',res)
    false   ---> oops! Can't handle extra space!
    > sip.match('( $q , $q )','("jan", "smit")',res)
    true

As a general rule, allow for whitespace in your patterns.

Finally, putting a ' $' at the end of a pattern means 'capture the rest of the
line, starting at the first non-space'.

    > sip.match('( $q , $q ) $','("jan", "smit") and a string',res)
    true
    > res
    {'jan','smit','and a string'}
    > res = {}
    > sip.match('( $q{first} , $q{last} ) $','("jan", "smit") and a string',res)
    true
    > res
    {first='jan',rest='and a string',last='smit'}


<a id="lapp"/>

### Command-line Programs with Lapp

`pl.lapp` is a small and focused Lua module which aims to make standard
command-line parsing easier and intuitive. It implements the standard GNU style,
i.e. short flags with one letter start with '-', and there may be an additional
long flag which starts with '--'. Generally options which take an argument expect
to find it as the next parameter (e.g. 'gcc test.c -o test') but single short
options taking a value can dispense with the space (e.g. 'head -n4
test.c' or `gcc -I/usr/include/lua/5.1 ...`)

As far as possible, Lapp will convert parameters into their equivalent Lua types,
i.e. convert numbers and convert filenames into file objects. If any conversion
fails, or a required parameter is missing, an error will be issued and the usage
text will be written out. So there are two necessary tasks, supplying the flag
and option names and associating them with a type.

For any non-trivial script, even for personal consumption, it's necessary to
supply usage text. The novelty of Lapp is that it starts from that point and
defines a loose format for usage strings which can specify the names and types of
the parameters.

An example will make this clearer:

    -- scale.lua
      lapp = require 'pl.lapp'
      local args = lapp [[
      Does some calculations
        -o,--offset (default 0.0)  Offset to add to scaled number
        -s,--scale  (number)  Scaling factor
         <number> (number )  Number to be scaled
      ]]

      print(args.offset + args.scale * args.number)

Here is a command-line session using this script:

      $ lua scale.lua
      scale.lua:missing required parameter: scale

      Does some calculations
       -o,--offset (default 0.0)  Offset to add to scaled number
       -s,--scale  (number)  Scaling factor
        <number> (number )  Number to be scaled

      $ lua scale.lua -s 2.2 10
      22

      $ lua scale.lua -s 2.2 x10
      scale.lua:unable to convert to number: x10

      ....(usage as before)

There are two kinds of lines in Lapp usage strings which are meaningful; option
and parameter lines. An option line gives the short option, optionally followed
by the corresponding long option. A type specifier in parentheses may follow.
Similarly, a parameter line starts with '<' PARAMETER '>', followed by a type
specifier. Type specifiers are either of the form '(default ' VALUE ')' or '('
TYPE ')'; the default specifier means that the parameter or option has a default
value and is not required. TYPE is one of 'string','number','file-in' or
'file-out'; VALUE is a number, one of ('stdin','stdout','stderr') or a token. The
rest of the line is not parsed and can be used for explanatory text.

This script shows the relation between the specified parameter names and the
fields in the output table.

      -- simple.lua
      local args = require ('pl.lapp') [[
      Various flags and option types
        -p          A simple optional flag, defaults to false
        -q,--quiet  A simple flag with long name
        -o  (string)  A required option with argument
        <input> (default stdin)  Optional input file parameter
      ]]

      for k,v in pairs(args) do
          print(k,v)
      end

I've just dumped out all values of the args table; note that args.quiet has
become true, because it's specified; args.p defaults to false. If there is a long
name for an option, that will be used in preference as a field name. A type or
default specifier is not necessary for simple flags, since the default type is
boolean.

      $ simple -o test -q simple.lua
      p       false
      input   file (781C1BD8)
      quiet   true
      o       test
      input_name      simple.lua
      D:\dev\lua\lapp>simple -o test simple.lua one two three
      1       one
      2       two
      3       three
      p       false
      quiet   false
      input   file (781C1BD8)
      o       test
      input_name      simple.lua

The parameter input has been set to an open read-only file object - we know it
must be a read-only file since that is the type of the default value. The field
input_name is automatically generated, since it's often useful to have access to
the original filename.

Notice that any extra parameters supplied will be put in the result table with
integer indices, i.e. args[i] where i goes from 1 to #args.

Files don't really have to be closed explicitly for short scripts with a quick
well-defined mission, since the result of garbage-collecting file objects is to
close them.

#### Enforcing a Range for a Parameter

The type specifier can also be of the form '(' MIN '..' MAX ')'.

    local lapp = require 'pl.lapp'
    local args = lapp [[
        Setting ranges
        <x> (1..10)  A number from 1 to 10
        <y> (-5..1e6) Bigger range
    ]]

    print(args.x,args.y)

Here the meaning is that the value is greater or equal to MIN and less or equal
to MAX; there is no provision for forcing a parameter to be a whole number.

You may also define custom types that can be used in the type specifier:

    lapp = require ('pl.lapp')

    lapp.add_type('integer','number',
        function(x)
            lapp.assert(math.ceil(x) == x, 'not an integer!')
        end
    )

    local args =  lapp [[
        <ival> (integer) Process PID
    ]]

    print(args.ival)

`lapp.add_type` takes three parameters, a type name, a converter and a constraint
function. The constraint function is expected to throw an assertion if some
condition is not true; we use lapp.assert because it fails in the standard way
for a command-line script. The converter argument can either be a type name known
to Lapp, or a function which takes a string and generates a value.

#### 'varargs' Parameter Arrays

    lapp = require 'pl.lapp'
    local args = lapp [[
    Summing numbers
        <numbers...> (number) A list of numbers to be summed
    ]]

    local sum = 0
    for i,x in ipairs(args.numbers) do
        sum = sum + x
    end
    print ('sum is '..sum)

The parameter number has a trailing '...', which indicates that this parameter is
a 'varargs' parameter. It must be the last parameter, and args.number will be an
array.

Consider this implementation of the head utility from Mac OS X:

        -- implements a BSD-style head
        -- (see http://www.manpagez.com/man/1/head/osx-10.3.php)

        lapp = require ('pl.lapp')

        local args = lapp [[
        Print the first few lines of specified files
           -n         (default 10)    Number of lines to print
           <files...> (default stdin) Files to print
        ]]

        -- by default, lapp converts file arguments to an actual Lua file object.
        -- But the actual filename is always available as <file>_name.
        -- In this case, 'files' is a varargs array, so that 'files_name' is
        -- also an array.
        local nline = args.n
        local nfile = #args.files
        for i = 1,nfile do
            local file = args.files[i]
            if nfile > 1 then
                print('==> '..args.files_name[i]..' <==')
            end
            local n = 0
            for line in file:lines() do
                print(line)
                n = n + 1
                if n == nline then break end
            end
        end

Note how we have access to all the filenames, because the auto-generated field
`files_name` is also an array!

(This is probably not a very considerate script, since Lapp will open all the
files provided, and only close them at the end of the script. See the `xhead.lua`
example for another implementation.)

Flags and options may also be declared as vararg arrays, and can occur anywhere.
Bear in mind that short options can be combined (like 'tar -xzf'), so it's
perfectly legal to have '-vvv'. But normally the value of args.v is just a simple
`true` value.

    local args = require ('pl.lapp') [[
       -v...  Verbosity level; can be -v, -vv or -vvv
    ]]
    vlevel = not args.v[1] and 0 or #args.v
    print(vlevel)

The vlevel assigment is a bit of Lua voodoo, so consider the cases:

    * No -v flag, v is just { false }
    * One -v flags, v is { true }
    * Two -v flags, v is { true, true }
    * Three -v flags, v is { true, true, true }

#### Defining a Parameter Callback

If a script implements `lapp.callback`, then Lapp will call it after each
argument is parsed. The callback is passed the parameter name, the raw unparsed
value, and the result table. It is called immediately after assignment of the
value, so the corresponding field is available.

    lapp = require ('pl.lapp')

    function lapp.callback(parm,arg,args)
        print('+',parm,arg)
    end

    local args = lapp [[
    Testing parameter handling
        -p               Plain flag (defaults to false)
        -q,--quiet       Plain flag with GNU-style optional long name
        -o  (string)     Required string option
        -n  (number)     Required number option
        -s (default 1.0) Option that takes a number, but will default
        <start> (number) Required number argument
        <input> (default stdin)  A parameter which is an input file
        <output> (default stdout) One that is an output file
    ]]
    print 'args'
    for k,v in pairs(args) do
        print(k,v)
    end

This produces the following output:

    $ args -o name -n 2 10 args.lua
    +       o       name
    +       n       2
    +       start   10
    +       input   args.lua
    args
    p       false
    s       1
    input_name      args.lua
    quiet   false
    output  file (781C1B98)
    start   10
    input   file (781C1BD8)
    o       name
    n       2

Callbacks are needed when you want to take action immediately on parsing an
argument.

#### Slack Mode

If you'd like to use a multi-letter 'short' parameter you need to set
the `lapp.slack` variable to `true`.

In the following example we also see how default `false` and default `true` flags can be used
and how to overwrite the default `-h` help flag (`--help` still works fine) - this applies
to non-slack mode as well.

    -- Parsing the command line ----------------------------------------------------
    -- test.lua
    local lapp = require 'pl.lapp'
    local pretty = require 'pl.pretty'
    lapp.slack = true
    local args = lapp [[
    Does some calculations
       -v, --video              (string)             Specify input video
       -w, --width              (default 256)        Width of the video
       -h, --height             (default 144)        Height of the video
       -t, --time               (default 10)         Seconds of video to process
       -sk,--seek               (default 0)          Seek number of seconds
       -f1,--flag1                                   A false flag
       -f2,--flag2                                   A false flag
       -f3,--flag3              (default true)       A true flag
       -f4,--flag4              (default true)       A true flag
    ]]

    pretty.dump(args)

And here we can see the output of `test.lua`:

    $> lua test.lua -v abc --time 40 -h 20 -sk 15 --flag1 -f3
    ---->
    {
      width = 256,
      flag1 = true,
      flag3 = false,
      seek = 15,
      flag2 = false,
      video = abc,
      time = 40,
      height = 20,
      flag4 = true
    }
