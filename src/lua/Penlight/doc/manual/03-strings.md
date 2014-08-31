## Strings. Higher-level operations on strings.

### Extra String Methods

@lookup pl.stringx

These are convenient borrowings from Python, as described in 3.6.1 of the Python
reference, but note that indices in Lua always begin at one. `stringx` defines
functions like `isalpha` and `isdigit`, which return `true` if s is only composed
of letters or digits respectively. `startswith` and `endswith` are convenient
ways to find substrings. (`endswith` works as in Python 2.5, so that `f:endswith
{'.bat','.exe','.cmd'}` will be true for any filename which ends with these
extensions.) There are justify methods and whitespace trimming functions like
`strip`.

    > stringx.import()
    > ('bonzo.dog'):endswith {'.dog','.cat'}
    true
    > ('bonzo.txt'):endswith {'.dog','.cat'}
    false
    > ('bonzo.cat'):endswith {'.dog','.cat'}
    true
    > (' stuff'):ljust(20,'+')
    '++++++++++++++ stuff'
    > ('  stuff '):lstrip()
    'stuff '
    > ('  stuff '):rstrip()
    '  stuff'
    > ('  stuff '):strip()
    'stuff'
    > for s in ('one\ntwo\nthree\n'):lines() do print(s) end
    one
    two
    three

Most of these can be fairly easily implemented using the Lua string library,
which is more general and powerful. But they are convenient operations to have
easily at hand. Note that can be injected into the `string` table if you use
`stringx.import`, but a simple alias like `local stringx = require 'pl.stringx'`
is preferrable. This is the recommended practice when writing modules for
consumption by other people, since it is bad manners to change the global state
of the rest of the system. Magic may be used for convenience, but there is always
a price.


### String Templates

@lookup pl.text

Another borrowing from Python, string templates allow you to substitute values
looked up in a table:

    local Template = require ('pl.text').Template
    t = Template('${here} is the $answer')
    print(t:substitute {here = 'Lua', answer = 'best'})
    ==>
    Lua is the best

'$ variables' can optionally have curly braces; this form is useful if you are
glueing text together to make variables, e.g `${prefix}_name_${postfix}`. The
`substitute` method will throw an error if a $ variable is not found in the
table, and the `safe_substitute` method will not.

The Lua implementation has an extra method, `indent_substitute` which is very
useful for inserting blocks of text, because it adjusts indentation. Consider
this example:

    -- testtemplate.lua
    local Template = require ('pl.text').Template

    t = Template [[
        for i = 1,#$t do
            $body
        end
    ]]

    body = Template [[
    local row = $t[i]
    for j = 1,#row do
        fun(row[j])
    end
    ]]

    print(t:indent_substitute {body=body,t='tbl'})

And the output is:

        for i = 1,#tbl do
            local row = tbl[i]
            for j = 1,#row do
                fun(row[j])
            end
        end

`indent_substitute` can substitute templates, and in which case they themselves
will be substituted using the given table. So in this case, `$t` was substituted
twice.

`pl.text` also has a number of useful functions like `dedent`, which strips all
the initial indentation from a multiline string. As in Python, this is useful for
preprocessing multiline strings if you like indenting them with your code. The
function `wrap` is passed a long string (a _paragraph_) and returns a list of
lines that fit into a desired line width. As an extension, there is also `indent`
for indenting multiline strings.

New in Penlight with the 0.9 series is `text.format_operator`. Calling this
enables Python-style string formating using the modulo operator `%`:

    > text.format_operator()
    > = '%s[%d]' % {'dog',1}
    dog[1]

So in its simplest form it saves the typing involved with `string.format`; it
will also expand `$` variables using named fields:

    > = '$animal[$num]' % {animal='dog',num=1}
    dog[1]

As with `stringx.import` you have to do this explicitly, since all strings share the same
metatable. But in your own scripts you can feel free to do this.

### Another Style of Template

A new module is `template`, which is a version of Rici Lake's [Lua
Preprocessor](http://lua-users.org/wiki/SlightlyLessSimpleLuaPreprocessor).  This
allows you to mix Lua code with your templates in a straightforward way. There
are only two rules:

  - Lines begining with `#` are Lua
  - Otherwise, anything inside `$()` is a Lua expression.

So a template generating an HTML list would look like this:

    <ul>
    # for i,val in ipairs(T) do
    <li>$(i) = $(val:upper())</li>
    # end
    </ul>

Assume the text is inside `tmpl`, then the template can be expanded using:

    local template = require 'pl.template'
    res = template.substitute(tmpl,{T = {'one','two','three'}})

and we get

    <ul>
    <li>1 = ONE</li>
    <li>2 = TWO</li>
    <li>3 = THREE</li>
    </ul>

There is a single function, `template.substitute` which is passed a template
string and an environment table.   This table may contain some special fields,
like `\_parent` which can be set to a table representing a 'fallback' environment
in case a symbol was not found. `\_brackets` is usually '()' and `\_escape` is
usually '#' but it's sometimes necessary to redefine these if the defaults
interfere with the target language - for instance, `$(V)` has another meaning in
Make, and `#` means a preprocessor line in C/C++.

Finally, if something goes wrong, passing `_debug` will cause the intermediate
Lua code to be dumped if there's a problem.

Here is a C code generation example; something that could easily be extended to
be a minimal Lua extension skeleton generator.

    local subst = require 'pl.template'.substitute

    local templ = [[
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>

    > for _,f in ipairs(mod) do
    static int l_$(f.name) (lua_State *L) {

    }
    > end

    static const luaL_reg $(mod.name)[] = {
    > for _,f in ipairs(mod) do
        {"$(f.name)",l_$(f.name)},
    > end
        {NULL,NULL}
    };

    int luaopen_$(mod.name) {
       luaL_register (L, "$(mod.name)", $(mod.name));
        return 1;
    }
    ]]

    print(subst(templ,{
        _escape = '>',
        ipairs = ipairs,
        mod = {
            name = 'baggins';
            {name='frodo'},
            {name='bilbo'}
        }
    }))


### File-style I/O on Strings

`pl.stringio`  provides just three functions; `stringio.open` is passed a string,
and returns a file-like object for reading. It supports a `read` method, which
takes the same arguments as standard file objects:

    > f = stringio.open 'first line\n10 20 30\n'
    > = f:read()
    first line
    > = f:read('*n','*n','*n')
    10	20	30

`lines` and `seek` are also supported.

`stringio.lines` is a useful short-cut for iterating over all the lines in a
string.

`stringio.create` creates a writeable file-like object. You then use `write` to
this stream, and finally extract the builded string using `value`.  This 'string
builder' pattern is useful for efficiently creating large strings.

