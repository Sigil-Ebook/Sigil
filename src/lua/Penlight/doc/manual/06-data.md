## Data

### Reading Data Files

The first thing to consider is this: do you actually need to write a custom file
reader? And if the answer is yes, the next question is: can you write the reader
in as clear a way as possible? Correctness, Robustness, and Speed; pick the first
two and the third can be sorted out later, _if necessary_.

A common sort of data file is the configuration file format commonly used on Unix
systems. This format is often called a _property_ file in the Java world.

    # Read timeout in seconds
    read.timeout=10

    # Write timeout in seconds
    write.timeout=10

Here is a simple Lua implementation:

    -- property file parsing with Lua string patterns
    props = []
    for line in io.lines() do
        if line:find('#',1,true) ~= 1 and not line:find('^%s*$') then
            local var,value = line:match('([^=]+)=(.*)')
            props[var] = value
        end
    end

Very compact, but it suffers from a similar disease in equivalent Perl programs;
it uses odd string patterns which are 'lexically noisy'. Noisy code like this
slows the casual reader down. (For an even more direct way of doing this, see the
next section, 'Reading Configuration Files')

Another implementation, using the Penlight libraries:

    -- property file parsing with extended string functions
    require 'pl'
    stringx.import()
    props = []
    for line in io.lines() do
        if not line:startswith('#') and not line:isspace() then
            local var,value = line:splitv('=')
            props[var] = value
        end
    end

This is more self-documenting; it is generally better to make the code express
the _intention_, rather than having to scatter comments everywhere - comments are
necessary, of course, but mostly to give the higher view of your intention that
cannot be expressed in code. It is slightly slower, true, but in practice the
speed of this script is determined by I/O, so further optimization is unnecessary.

### Reading Unstructured Text Data

Text data is sometimes unstructured, for example a file containing words. The
`pl.input` module has a number of functions which makes processing such files
easier. For example, a script to count the number of words in standard input
using `import.words`:

    -- countwords.lua
    require 'pl'
    local k = 1
    for w in input.words(io.stdin) do
        k = k + 1
    end
    print('count',k)

Or this script to calculate the average of a set of numbers using `input.numbers`:

    -- average.lua
    require 'pl'
    local k = 1
    local sum = 0
    for n in input.numbers(io.stdin) do
        sum = sum + n
        k = k + 1
    end
    print('average',sum/k)

These scripts can be improved further by _eliminating loops_ In the last case,
there is a perfectly good function `seq.sum` which can already take a sequence of
numbers and calculate these numbers for us:

    -- average2.lua
    require 'pl'
    local total,n = seq.sum(input.numbers())
    print('average',total/n)

A further simplification here is that if `numbers` or `words` are not passed an
argument, they will grab their input from standard input.  The first script can
be rewritten:

    -- countwords2.lua
    require 'pl'
    print('count',seq.count(input.words()))

A useful feature of a sequence generator like `numbers` is that it can read from
a string source. Here is a script to calculate the sums of the numbers on each
line in a file:

    -- sums.lua
    for line in io.lines() do
        print(seq.sum(input.numbers(line))
    end

### Reading Columnar Data

It is very common to find data in columnar form, either space or comma-separated,
perhaps with an initial set of column headers. Here is a typical example:

    EventID	Magnitude	LocationX	LocationY	LocationZ
    981124001	2.0	18988.4	10047.1	4149.7
    981125001	0.8	19104.0	9970.4	5088.7
    981127003	0.5	19012.5	9946.9	3831.2
    ...

`input.fields` is designed to extract several columns, given some delimiter
(default to whitespace).  Here is a script to calculate the average X location of
all the events:

    -- avg-x.lua
    require 'pl'
    io.read() -- skip the header line
    local sum,count = seq.sum(input.fields {3})
    print(sum/count)

`input.fields` is passed either a field count, or a list of column indices,
starting at one as usual. So in this case we're only interested in column 3.  If
you pass it a field count, then you get every field up to that count:

    for id,mag,locX,locY,locZ in input.fields (5) do
    ....
    end

`input.fields` by default tries to convert each field to a number. It will skip
lines which clearly don't match the pattern, but will abort the script if there
are any fields which cannot be converted to numbers.

The second parameter is a delimiter, by default spaces. ' ' is understood to mean
'any number of spaces', i.e. '%s+'. Any Lua string pattern can be used.

The third parameter is a _data source_, by default standard input (defined by
`input.create_getter`.) It assumes that the data source has a `read` method which
brings in the next line, i.e. it is a 'file-like' object. As a special case, a
string will be split into its lines:

    > for x,y in input.fields(2,' ','10 20\n30 40\n') do print(x,y) end
    10      20
    30      40

Note the default behaviour for bad fields, which is to show the offending line
number:

    > for x,y in input.fields(2,' ','10 20\n30 40x\n') do print(x,y) end
    10      20
    line 2: cannot convert '40x' to number

This behaviour of `input.fields` is appropriate for a script which you want to
fail immediately with an appropriate _user_ error message if conversion fails.
The fourth optional parameter is an options table: `{no_fail=true}` means that
conversion is attempted but if it fails it just returns the string, rather as AWK
would operate. You are then responsible for checking the type of the returned
field. `{no_convert=true}` switches off conversion altogether and all fields are
returned as strings.

@lookup pl.data

Sometimes it is useful to bring a whole dataset into memory, for operations such
as extracting columns. Penlight provides a flexible reader specifically for
reading this kind of data, using the `data` module. Given a file looking like this:

    x,y
    10,20
    2,5
    40,50

Then `data.read` will create a table like this, with each row represented by a
sublist:

    > t = data.read 'test.txt'
    > pretty.dump(t)
    {{10,20},{2,5},{40,50},fieldnames={'x','y'},delim=','}

You can now analyze this returned table using the supplied methods. For instance,
the method `column_by_name` returns a table of all the values of that column.

    -- testdata.lua
    require 'pl'
    d = data.read('fev.txt')
    for _,name in ipairs(d.fieldnames) do
        local col = d:column_by_name(name)
        if type(col[1]) == 'number' then
            local total,n = seq.sum(col)
            utils.printf("Average for %s is %f\n",name,total/n)
        end
    end

`data.read` tries to be clever when given data; by default it expects a first
line of column names, unless any of them are numbers. It tries to deduce the
column delimiter by looking at the first line. Sometimes it guesses wrong; these
things can be specified explicitly. The second optional parameter is an options
table: can override `delim` (a string pattern), `fieldnames` (a list or
comma-separated string), specify `no_convert` (default is to convert), numfields
(indices of columns known to be numbers, as a list) and `thousands_dot` (when the
thousands separator in Excel CSV is '.')

A very powerful feature is a way to execute SQL-like queries on such data:

    -- queries on tabular data
    require 'pl'
    local d = data.read('xyz.txt')
    local q = d:select('x,y,z where x > 3 and z < 2 sort by y')
    for x,y,z in q do
        print(x,y,z)
    end

Please note that the format of queries is restricted to the following syntax:

    FIELDLIST [ 'where' CONDITION ] [ 'sort by' FIELD [asc|desc]]

Any valid Lua code can appear in `CONDITION`; remember it is _not_ SQL and you
have to use `==` (this warning comes from experience.)

For this to work, _field names must be Lua identifiers_. So `read` will massage
fieldnames so that all non-alphanumeric chars are replaced with underscores.
However, the `original_fieldnames` field always contains the original un-massaged
fieldnames.

`read` can handle standard CSV files fine, although doesn't try to be a
full-blown CSV parser.  With the `csv=true` option, it's possible to have
double-quoted fields, which may contain commas; then trailing commas become
significant as well.

Spreadsheet programs are not always the best tool to
process such data, strange as this might seem to some people. This is a toy CSV
file; to appreciate the problem, imagine thousands of rows and dozens of columns
like this:

    Department Name,Employee ID,Project,Hours Booked
    sales,1231,overhead,4
    sales,1255,overhead,3
    engineering,1501,development,5
    engineering,1501,maintenance,3
    engineering,1433,maintenance,10

The task is to reduce the dataset to a relevant set of rows and columns, perhaps
do some processing on row data, and write the result out to a new CSV file. The
`write_row` method uses the delimiter to write the row to a file;
`Data.select_row` is like `Data.select`, except it iterates over _rows_, not
fields; this is necessary if we are dealing with a lot of columns!

    names = {[1501]='don',[1433]='dilbert'}
    keepcols = {'Employee_ID','Hours_Booked'}
    t:write_row (outf,{'Employee','Hours_Booked'})
    q = t:select_row {
        fields=keepcols,
        where=function(row) return row[1]=='engineering' end
    }
    for row in q do
        row[1] = names[row[1]]
        t:write_row(outf,row)
    end

`Data.select_row` and `Data.select` can be passed a table specifying the query; a
list of field names, a function defining the condition and an optional parameter
`sort_by`. It isn't really necessary here, but if we had a more complicated row
condition (such as belonging to a specified set) then it is not generally
possible to express such a condition as a query string, without resorting to
hackery such as global variables.

With 1.0.3, you can specify explicit conversion functions for selected columns.
For instance, this is a log file with a Unix date stamp:

    Time Message
    1266840760 +# EE7C0600006F0D00C00F06010302054000000308010A00002B00407B00
    1266840760 closure data 0.000000 1972 1972 0
    1266840760 ++ 1266840760 EE 1
    1266840760 +# EE7C0600006F0D00C00F06010302054000000408020A00002B00407B00
    1266840764 closure data 0.000000 1972 1972 0

We would like the first column as an actual date object, so the `convert`
field sets an explicit conversion for column 1. (Note that we have to explicitly
convert the string to a number first.)

    Date = require 'pl.Date'

    function date_convert (ds)
        return Date(tonumber(ds))
    end

    d = data.read(f,{convert={[1]=date_convert},last_field_collect=true})

This gives us a two-column dataset, where the first column contains `Date` objects
and the second column contains the rest of the line. Queries can then easily
pick out events on a day of the week:

    q = d:select "Time,Message where Time:weekday_name()=='Sun'"

Data does not have to come from files, nor does it necessarily come from the lab
or the accounts department. On Linux, `ps aux` gives you a full listing of all
processes running on your machine. It is straightforward to feed the output of
this command into `data.read` and perform useful queries on it. Notice that
non-identifier characters like '%' get converted into underscores:

        require 'pl'
        f = io.popen 'ps aux'
        s = data.read (f,{last_field_collect=true})
        f:close()
        print(s.fieldnames)
        print(s:column_by_name 'USER')
        qs = 'COMMAND,_MEM where _MEM > 5 and USER=="steve"'
        for name,mem in s:select(qs) do
            print(mem,name)
        end

I've always been an admirer of the AWK programming language; with `filter` you
can get Lua programs which are just as compact:

    -- printxy.lua
    require 'pl'
    data.filter 'x,y where x > 3'

It is common enough to have data files without headers of field names.
`data.read` makes a special exception for such files if all fields are numeric.
Since there are no column names to use in query expressions, you can use AWK-like
column indexes, e.g. '$1,$2 where $1 > 3'.  I have a little executable script on
my system called `lf` which looks like this:

    #!/usr/bin/env lua
    require 'pl.data'.filter(arg[1])

And it can be used generally as a filter command to extract columns from data.
(The column specifications may be expressions or even constants.)

    $ lf '$1,$5/10' < test.dat

(As with AWK, please note the single-quotes used in this command; this prevents
the shell trying to expand the column indexes. If you are on Windows, then you
must quote the expression in double-quotes so
it is passed as one argument to your batch file.)

As a tutorial resource, have a look at `test-data.lua` in the PL tests directory
for other examples of use, plus comments.

The data returned by `read` or constructed by `Data.copy_select` from a query is
basically just an array of rows: `{{1,2},{3,4}}`. So you may use `read` to pull
in any array-like dataset, and process with any function that expects such a
implementation. In particular, the functions in `array2d` will work fine with
this data. In fact, these functions are available as methods; e.g.
`array2d.flatten` can be called directly like so to give us a one-dimensional list:

    v = data.read('dat.txt'):flatten()

The data is also in exactly the right shape to be treated as matrices by
[LuaMatrix](http://lua-users.org/wiki/LuaMatrix):

    > matrix = require 'matrix'
    > m = matrix(data.read 'mat.txt')
    > = m
    1       0.2     0.3
    0.2     1       0.1
    0.1     0.2     1
    > = m^2  -- same as m*m
    1.07    0.46    0.62
    0.41    1.06    0.26
    0.24    0.42    1.05

`write` will write matrices back to files for you.

Finally, for the curious, the global variable `_DEBUG` can be used to print out
the actual iterator function which a query generates and dynamically compiles. By
using code generation, we can get pretty much optimal performance out of
arbitrary queries.

    > lua -lpl -e "_DEBUG=true" -e "data.filter 'x,y where x > 4 sort by x'" < test.txt
    return function (t)
            local i = 0
            local v
            local ls = {}
            for i,v in ipairs(t) do
                if v[1] > 4  then
                        ls[#ls+1] = v
                end
            end
            table.sort(ls,function(v1,v2)
                return v1[1] < v2[1]
            end)
            local n = #ls
            return function()
                i = i + 1
                v = ls[i]
                if i > n then return end
                return v[1],v[2]
            end
    end

    10,20
    40,50

### Reading Configuration Files

The `config` module provides a simple way to convert several kinds of
configuration files into a Lua table. Consider the simple example:

    # test.config
    # Read timeout in seconds
    read.timeout=10

    # Write timeout in seconds
    write.timeout=5

    #acceptable ports
    ports = 1002,1003,1004

This can be easily brought in using `config.read` and the result shown using
`pretty.write`:

    -- readconfig.lua
    local config = require 'pl.config'
    local pretty= require 'pl.pretty'

    local t = config.read(arg[1])
    print(pretty.write(t))

and the output of `lua readconfig.lua test.config` is:

    {
      ports = {
        1002,
        1003,
        1004
      },
      write_timeout = 5,
      read_timeout = 10
    }

That is, `config.read` will bring in all key/value pairs, ignore # comments, and
ensure that the key names are proper Lua identifiers by replacing non-identifier
characters with '_'. If the values are numbers, then they will be converted. (So
the value of `t.write_timeout` is the number 5). In addition, any values which
are separated by commas will be converted likewise into an array.

Any line can be continued with a backslash. So this will all be considered one
line:

    names=one,two,three, \
    four,five,six,seven, \
    eight,nine,ten


Windows-style INI files are also supported. The section structure of INI files
translates naturally to nested tables in Lua:

    ; test.ini
    [timeouts]
    read=10 ; Read timeout in seconds
    write=5 ; Write timeout in seconds
    [portinfo]
    ports = 1002,1003,1004

 The output is:

    {
      portinfo = {
        ports = {
          1002,
          1003,
          1004
        }
      },
      timeouts = {
        write = 5,
        read = 10
      }
    }

You can now refer to the write timeout as `t.timeouts.write`.

As a final example of the flexibility of `config.read`, if passed this simple
comma-delimited file

    one,two,three
    10,20,30
    40,50,60
    1,2,3

it will produce the following table:

    {
      { "one", "two", "three" },
      { 10, 20, 30 },
      { 40, 50, 60  },
      { 1, 2, 3 }
    }

`config.read` isn't designed to read all CSV files in general, but intended to
support some Unix configuration files not structured as key-value pairs, such as
'/etc/passwd'.

This function is intended to be a Swiss Army Knife of configuration readers, but
it does have to make assumptions, and you may not like them. So there is an
optional extra parameter which allows some control, which is table that may have
the following fields:

    {
       variablilize = true,
       convert_numbers = tonumber,
       trim_space = true,
       list_delim = ',',
       trim_quotes = true,
       ignore_assign = false,
       keysep = '=',
       smart = false,
    }

`variablilize` is the option that converted `write.timeout` in the first example
to the valid Lua identifier `write_timeout`.  If `convert_numbers` is true, then
an attempt is made to convert any string that starts like a number. You can
specify your own function (say one that will convert a string like '5224 kb' into
a number.)

`trim_space` ensures that there is no starting or trailing whitespace with
values, and `list_delim` is the character that will be used to decide whether to
split a value up into a list (it may be a Lua string pattern such as '%s+'.)

For instance, the password file in Unix is colon-delimited:

    t = config.read('/etc/passwd',{list_delim=':'})

This produces the following output on my system (only last two lines shown):

    {
      ...
      {
        "user",
        "x",
        "1000",
        "1000",
        "user,,,",
        "/home/user",
        "/bin/bash"
      },
      {
        "sdonovan",
        "x",
        "1001",
        "1001",
        "steve donovan,28,,",
        "/home/sdonovan",
        "/bin/bash"
      }
    }

You can get this into a more sensible format, where the usernames are the keys,
with this (the `tablex.pairmap` function must return value, key!)

    t = tablex.pairmap(function(k,v) return v,v[1] end,t)

and you get:

    { ...
      sdonovan = {
        "sdonovan",
        "x",
        "1001",
        "1001",
        "steve donovan,28,,",
        "/home/sdonovan",
        "/bin/bash"
      }
    ...
    }

Many common Unix configuration files can be read by tweaking these parameters.
For `/etc/fstab`, the options `{list_delim='%s+',ignore_assign=true}` will
correctly separate the columns.  It's common to find 'KEY VALUE' assignments in
files such as `/etc/ssh/ssh_config`; the options `{keysep=' '}` make
`config.read` return a table where each KEY has a value VALUE.

Files in the Linux `procfs` usually use ':` as the field delimiter:

    > t = config.read('/proc/meminfo',{keysep=':'})
    > = t.MemFree
    220140 kB

That result is a string, since `tonumber` doesn't like it, but defining the
`convert_numbers` option as `function(s) return tonumber((s:gsub(' kB$','')))
end` will get the memory figures as actual numbers in the result. (The extra
parentheses are necessary so that `tonumber` only gets the first result from
`gsub`). From `tests/test-config.lua':

    testconfig([[
    MemTotal:        1024748 kB
    MemFree:          220292 kB
    ]],
    { MemTotal = 1024748, MemFree = 220292 },
    {
     keysep = ':',
     convert_numbers = function(s)
        s = s:gsub(' kB$','')
        return tonumber(s)
      end
     }
    )


The `smart` option lets `config.read` make a reasonable guess for you; there
are examples in `tests/test-config.lua`, but basically these common file
formats (and those following the same pattern) can be processed directly in
smart mode: 'etc/fstab', '/proc/XXXX/status', 'ssh_config' and 'pdatedb.conf'.

Please note that `config.read` can be passed a _file-like object_; if it's not a
string and supports the `read` method, then that will be used. For instance, to
read a configuration from a string, use `stringio.open`.


<a id="lexer"/>

### Lexical Scanning

Although Lua's string pattern matching is very powerful, there are times when
something more powerful is needed.  `pl.lexer.scan` provides lexical scanners
which _tokenize_ a string, classifying tokens into numbers, strings, etc.

    > lua -lpl
    Lua 5.1.4  Copyright (C) 1994-2008 Lua.org, PUC-Rio
    > tok = lexer.scan 'alpha = sin(1.5)'
    > = tok()
    iden    alpha
    > = tok()
    =       =
    > = tok()
    iden    sin
    > = tok()
    (       (
    > = tok()
    number  1.5
    > = tok()
    )       )
    > = tok()
    (nil)

The scanner is a function, which is repeatedly called and returns the _type_ and
_value_ of the token.  Recognized basic types are 'iden','string','number', and
'space'. and everything else is represented by itself. Note that by default the
scanner will skip any 'space' tokens.

'comment' and 'keyword' aren't applicable to the plain scanner, which is not
language-specific, but a scanner which understands Lua is available. It
recognizes the Lua keywords, and understands both short and long comments and
strings.

    > for t,v in lexer.lua 'for i=1,n do' do print(t,v) end
    keyword for
    iden    i
    =       =
    number  1
    ,       ,
    iden    n
    keyword do

A lexical scanner is useful where you have highly-structured data which is not
nicely delimited by newlines. For example, here is a snippet of a in-house file
format which it was my task to maintain:

    points
(818344.1,-20389.7,-0.1),(818337.9,-20389.3,-0.1),(818332.5,-20387.8,-0.1)
        ,(818327.4,-20388,-0.1),(818322,-20387.7,-0.1),(818316.3,-20388.6,-0.1)
        ,(818309.7,-20389.4,-0.1),(818303.5,-20390.6,-0.1),(818295.8,-20388.3,-0.1)
        ,(818290.5,-20386.9,-0.1),(818285.2,-20386.1,-0.1),(818279.3,-20383.6,-0.1)
        ,(818274,-20381.2,-0.1),(818274,-20380.7,-0.1);

Here is code to extract the points using `pl.lexer`:

    -- assume 's' contains the text above...
    local lexer = require 'pl.lexer'
    local expecting = lexer.expecting
    local append = table.insert

    local tok = lexer.scan(s)

    local points = {}
    local t,v = tok() -- should be 'iden','points'

    while t ~= ';' do
        c = {}
        expecting(tok,'(')
        c.x = expecting(tok,'number')
        expecting(tok,',')
        c.y = expecting(tok,'number')
        expecting(tok,',')
        c.z = expecting(tok,'number')
        expecting(tok,')')
        t,v = tok()  -- either ',' or ';'
        append(points,c)
    end

The `expecting` function grabs the next token and if the type doesn't match, it
throws an error. (`pl.lexer`, unlike other PL libraries, raises errors if
something goes wrong, so you should wrap your code in `pcall` to catch the error
gracefully.)

The scanners all have a second optional argument, which is a table which controls
whether you want to exclude spaces and/or comments. The default for `lexer.lua`
is `{space=true,comments=true}`.  There is a third optional argument which
determines how string and number tokens are to be processsed.

The ultimate highly-structured data is of course, program source. Here is a
snippet from 'text-lexer.lua':

    require 'pl'

    lines = [[
    for k,v in pairs(t) do
        if type(k) == 'number' then
            print(v) -- array-like case
        else
            print(k,v)
        end
    end
    ]]

    ls = List()
    for tp,val in lexer.lua(lines,{space=true,comments=true}) do
        assert(tp ~= 'space' and tp ~= 'comment')
        if tp == 'keyword' then ls:append(val) end
    end
    test.asserteq(ls,List{'for','in','do','if','then','else','end','end'})

Here is a useful little utility that identifies all common global variables found
in a lua module (ignoring those declared locally for the moment):

    -- testglobal.lua
    require 'pl'

    local txt,err = utils.readfile(arg[1])
    if not txt then return print(err) end

    local globals = List()
    for t,v in lexer.lua(txt) do
        if t == 'iden' and _G[v] then
            globals:append(v)
        end
    end
    pretty.dump(seq.count_map(globals))

Rather then dumping the whole list, with its duplicates, we pass it through
`seq.count_map` which turns the list into a table where the keys are the values,
and the associated values are the number of times those values occur in the
sequence. Typical output looks like this:

    {
      type = 2,
      pairs = 2,
      table = 2,
      print = 3,
      tostring = 2,
      require = 1,
      ipairs = 4
    }

You could further pass this through `tablex.keys` to get a unique list of
symbols. This can be useful when writing 'strict' Lua modules, where all global
symbols must be defined as locals at the top of the file.

For a more detailed use of `lexer.scan`, please look at `testxml.lua` in the
examples directory.

### XML

New in the 0.9.7 release is some support for XML. This is a large topic, and
Penlight does not provide a full XML stack, which is properly the task of a more
specialized library.

#### Parsing and Pretty-Printing

The semi-standard XML parser in the Lua universe is [lua-expat](http://matthewwild.co.uk/projects/luaexpat/).
In particular,
it has a function called `lxp.lom.parse` which will parse XML into the Lua Object
Model (LOM) format. However, it does not provide a way to convert this data back
into XML text.  `xml.parse` will use this function, _if_ `lua-expat` is
available, and otherwise switches back to a pure Lua parser originally written by
Roberto Ierusalimschy.

The resulting document object knows how to render itself as a string, which is
useful for debugging:

    > d = xml.parse "<nodes><node id='1'>alice</node></nodes>"
    > = d
    <nodes><node id='1'>alice</node></nodes>
    > pretty.dump (d)
    {
      {
        "alice",
        attr = {
          "id",
          id = "1"
        },
        tag = "node"
      },
      attr = {
      },
      tag = "nodes"
    }

Looking at the actual shape of the data reveals the structure of LOM:

  * every element has a `tag` field with its name
  * plus a `attr` field which is a table containing the attributes as fields, and
also as an array. It is always present.
  * the children of the element are the array part of the element, so `d[1]` is
the first child of `d`, etc.

It could be argued that having attributes also as the array part of `attr` is not
essential (you cannot depend on attribute order in XML) but that's how
it goes with this standard.

`lua-expat` is another _soft dependency_ of Penlight; generally, the fallback
parser is good enough for straightforward XML as is commonly found in
configuration files, etc. `doc.basic_parse` is not intended to be a proper
conforming parser (it's only sixty lines) but it handles simple kinds of
documents that do not have comments or DTD directives. It is intelligent enough
to ignore the `<?xml` directive and that is about it.

You can get pretty-printing by explicitly calling `xml.tostring` and passing it
the initial indent and the per-element indent:

    > = xml.tostring(d,'','  ')

    <nodes>
      <node id='1'>alice</node>
    </nodes>

There is a fourth argument which is the _attribute indent_:

    > a = xml.parse "<frodo name='baggins' age='50' type='hobbit'/>"
    > = xml.tostring(a,'','  ','  ')

    <frodo
      type='hobbit'
      name='baggins'
      age='50'
    />

#### Parsing and Working with Configuration Files

It's common to find configurations expressed with XML these days. It's
straightforward to 'walk' the [LOM](http://matthewwild.co.uk/projects/luaexpat/lom.html)
data and extract the data in the form you want:

    require 'pl'

    local config = [[
    <config>
        <alpha>1.3</alpha>
        <beta>10</beta>
        <name>bozo</name>
    </config>
    ]]
    local d,err = xml.parse(config)

    local t = {}
    for item in d:childtags() do
        t[item.tag] = item[1]
    end

    pretty.dump(t)
    --->
    {
      beta = "10",
      alpha = "1.3",
      name = "bozo"
    }

The only gotcha is that here we must use the `Doc:childtags` method, which will
skip over any text elements.

A more involved example is this excerpt from `serviceproviders.xml`, which is
usually found at `/usr/share/mobile-broadband-provider-info/serviceproviders.xml`
on Debian/Ubuntu Linux systems.

    d = xml.parse [[
    <serviceproviders format="2.0">
    ...
    <country code="za">
        <provider>
            <name>Cell-c</name>
            <gsm>
                <network-id mcc="655" mnc="07"/>
                <apn value="internet">
                    <username>Cellcis</username>
                    <dns>196.7.0.138</dns>
                    <dns>196.7.142.132</dns>
                </apn>
            </gsm>
        </provider>
        <provider>
            <name>MTN</name>
            <gsm>
                <network-id mcc="655" mnc="10"/>
                <apn value="internet">
                    <dns>196.11.240.241</dns>
                    <dns>209.212.97.1</dns>
                </apn>
            </gsm>
        </provider>
        <provider>
            <name>Vodacom</name>
            <gsm>
                <network-id mcc="655" mnc="01"/>
                <apn value="internet">
                    <dns>196.207.40.165</dns>
                    <dns>196.43.46.190</dns>
                </apn>
                <apn value="unrestricted">
                    <name>Unrestricted</name>
                    <dns>196.207.32.69</dns>
                    <dns>196.43.45.190</dns>
                </apn>
            </gsm>
        </provider>
        <provider>
            <name>Virgin Mobile</name>
            <gsm>
                <apn value="vdata">
                    <dns>196.7.0.138</dns>
                    <dns>196.7.142.132</dns>
                </apn>
            </gsm>
        </provider>
    </country>
    ....
    </serviceproviders>
    ]]

Getting the names of the providers per-country is straightforward:

    local t = {}
    for country in d:childtags() do
        local providers = {}
        t[country.attr.code] = providers
        for provider in country:childtags() do
            table.insert(providers,provider:child_with_name('name'):get_text())
        end
    end

    pretty.dump(t)
    -->
    {
      za = {
        "Cell-c",
        "MTN",
        "Vodacom",
        "Virgin Mobile"
      }
      ....
    }

#### Generating XML with 'xmlification'

This feature is inspired by the `htmlify` function used by
[Orbit](http://keplerproject.github.com/orbit/) to simplify HTML generation,
except that no function environment magic is used; the `tags` function returns a
set of _constructors_ for elements of the given tag names.

    > nodes, node = xml.tags 'nodes, node'
    > = node 'alice'
    <node>alice</node>
    > = nodes { node {id='1','alice'}}
    <nodes><node id='1'>alice</node></nodes>

The flexibility of Lua tables is very useful here, since both the attributes and
the children of an element can be encoded naturally. The argument to these tag
constructors is either a single value (like a string) or a table where the
attributes are the named keys and the children are the array values.

#### Generating XML using Templates

A template is a little XML document which contains dollar-variables. The `subst`
method on a document is fed an array of tables containing values for these
variables. Note how the parent tag name is specified:

    > templ = xml.parse "<node id='$id'>$name</node>"
    > = templ:subst {tag='nodes', {id=1,name='alice'},{id=2,name='john'}}
    <nodes><node id='1'>alice</node><node id='2'>john</node></nodes>

Substitution is very related to _filtering_ documents. One of the annoying things
about XML is that it is a document markup language first, and a data language
second. Standard parsers will assume you really care about all those extra
text elements. Consider this fragment, which has been changed by a five-year old:

    T = [[
      <weather>
        boops!
        <current_conditions>
          <condition data='$condition'/>
          <temp_c data='$temp'/>
          <bo>whoops!</bo>
        </current_conditions>
      </weather>
    ]]

Conformant parsers will give you text elements with the line feed after `<current_conditions>`
although it makes handling the data more irritating.

    local function parse (str)
        return xml.parse(str,false,true)
    end

Second argument means 'string, not file' and third argument means use the built-in
Lua parser (instead of LuaExpat if available) which _by default_ is not interested in
keeping such strings.

How to remove the string `boops!`?  `clone` (also called `filter` when called as a
method) copies a LOM document. It can be passed a filter function, which is applied
to each string found. The powerful thing about this is that this function receives
structural information - the parent node, and whether this was a tag name, a text
element or a attribute name:

    d = parse (T)
    c = d:filter(function(s,kind,parent)
        print(stringx.strip(s),kind,parent and parent.tag or '?')
        if kind == '*TEXT' and #parent > 1 then return nil end
        return s
    end)
    --->
    weather	*TAG	?
    boops!	*TEXT	weather
    current_conditions	*TAG	weather
    condition	*TAG	current_conditions
    $condition	data	condition
    temp_c	*TAG	current_conditions
    $temp	data	temp_c
    bo	*TAG	current_conditions
    whoops!	*TEXT	bo

We can pull out 'boops' and not 'whoops' by discarding text elements which are not
the single child of an element.



#### Extracting Data using Templates

Matching goes in the opposite direction.  We have a document, and would like to
extract values from it using a pattern.

A common use of this is parsing the XML result of API queries.  The
[(undocumented and subsequently discontinued) Google Weather
API](http://blog.programmableweb.com/2010/02/08/googles-secret-weather-api/) is a
good example. Grabbing the result of
`http://www.google.com/ig/api?weather=Johannesburg,ZA" we get something like
this, after pretty-printing:

    <xml_api_reply version='1'>
      <weather module_id='0' tab_id='0' mobile_zipped='1' section='0' row='0'
mobile_row='0'>
        <forecast_information>
          <city data='Johannesburg, Gauteng'/>
          <postal_code data='Johannesburg,ZA'/>
          <latitude_e6 data=''/>
          <longitude_e6 data=''/>
          <forecast_date data='2010-10-02'/>
          <current_date_time data='2010-10-02 18:30:00 +0000'/>
          <unit_system data='US'/>
        </forecast_information>
        <current_conditions>
          <condition data='Clear'/>
          <temp_f data='75'/>
          <temp_c data='24'/>
          <humidity data='Humidity: 19%'/>
          <icon data='/ig/images/weather/sunny.gif'/>
          <wind_condition data='Wind: NW at 7 mph'/>
        </current_conditions>
        <forecast_conditions>
          <day_of_week data='Sat'/>
          <low data='60'/>
          <high data='89'/>
          <icon data='/ig/images/weather/sunny.gif'/>
          <condition data='Clear'/>
        </forecast_conditions>
        ....
       </weather>
    </xml_api_reply>

Assume that the above XML has been read into `google`. The idea is to write a
pattern looking like a template, and use it to extract some values of interest:

    t = [[
      <weather>
        <current_conditions>
          <condition data='$condition'/>
          <temp_c data='$temp'/>
        </current_conditions>
      </weather>
    ]]

    local res, ret = google:match(t)
    pretty.dump(res)

And the output is:

    {
      condition = "Clear",
      temp = "24"
    }

The `match` method can be passed a LOM document or some text, which will be
parsed first.

But what if we need to extract values from repeated elements? Match templates may
contain 'array matches' which are enclosed in '{{..}}':

      <weather>
        {{<forecast_conditions>
          <day_of_week data='$day'/>
          <low data='$low'/>
          <high data='$high'/>
          <condition data='$condition'/>
        </forecast_conditions>}}
      </weather>

And the match result is:

    {
      {
        low = "60",
        high = "89",
        day = "Sat",
        condition = "Clear",
      },
      {
        low = "53",
        high = "86",
        day = "Sun",
        condition = "Clear",
      },
      {
        low = "57",
        high = "87",
        day = "Mon",
        condition = "Clear",
      },
      {
        low = "60",
        high = "84",
        day = "Tue",
        condition = "Clear",
      }
    }

With this array of tables, you can use `tablex` or `List`
to reshape into the desired form, if you choose.  Just as with reading a Unix password
file with `config`, you can make the array into a map of days to conditions using:

    `tablex.pairmap`('|k,v| v,v.day',conditions)

(Here using the alternative string lambda option)

However, xml matches can shape the structure of the output. By replacing the `day_of_week`
line of the template with `<day_of_week data='$_'/>` we get the same effect; `$_` is
a special symbol that means that this captured value (or simply _capture_) becomes the key.

Note that `$NUMBER` means a numerical index, so
that `$1` is the first element of the resulting array, and so forth. You can mix
numbered and named captures, but it's strongly advised to make the numbered captures
form a proper array sequence (everything from `1` to `n` inclusive). `$0` has a
special meaning; if it is the only capture (`{[0]='foo'}`) then the table is
collapsed into 'foo'.

      <weather>
        {{<forecast_conditions>
          <day_of_week data='$_'/>
          <low data='$1'/>
          <high data='$2'/>
          <condition data='$3'/>
        </forecast_conditions>}}
      </weather>

Now the result is:

    {
      Tue = {
        "60",
        "84",
        "Clear"
      },
      Sun = {
        "53",
        "86",
        "Clear"
      },
      Sat = {
        "60",
        "89",
        "Clear"
      },
      Mon = {
        "57",
        "87",
        "Clear"
      }
    }

Applying matches to this config file poses another problem, because the actual
tags matched are themselves meaningful.

    <config>
        <alpha>1.3</alpha>
        <beta>10</beta>
        <name>bozo</name>
    </config>

So there are tag 'wildcards' which are element names ending with a hyphen.

    <config>
        {{<key->$value</key->}}
    </config>

You will then get `{{alpha='1.3'},...}`. The most convenient format would be
returned by this (note that `_-` behaves just like `$_`):

    <config>
        {{<_->$0</_->}}
    </config>

which would return `{alpha='1.3',beta='10',name='bozo'}`.

We could play this game endlessly, and encode ways of converting captures, but
the scheme is complex enough, and it's easy to do the conversion later

    local numbers = {alpha=true,beta=true}
    for k,v in pairs(res) do
        if numbers[v] then res[k] = tonumber(v) end
    end


#### HTML Parsing

HTML is an unusually degenerate form of XML, and Dennis Schridde has contributed
a feature which makes parsing it easier.  For instance, from the tests:

    doc = xml.parsehtml [[
    <BODY>
    Hello dolly<br>
    HTML is <b>slack</b><br>
    </BODY>
    ]]

    asserteq(xml.tostring(doc),[[
    <body>
    Hello dolly<br/>
    HTML is <b>slack</b><br/></body>]])

That is, all tags are converted to lowercase, and empty HTML elements like `br`
are properly closed; attributes do not need to be quoted.

Also, DOCTYPE directives and comments are skipped. For truly badly formed HTML,
this is not the tool for you!



