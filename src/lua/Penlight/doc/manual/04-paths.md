## Paths and Directories

### Working with Paths

Programs should not depend on quirks of your operating system. They will be
harder to read, and need to be ported for other systems.  The worst of course is
hardcoding paths like 'c:\\' in programs, and wondering why Vista complains so
much. But even something like `dir..'\\'..file` is a problem, since Unix can't
understand backslashes in this way. `dir..'/'..file` is _usually_ portable, but
it's best to put this all into a simple function, `path.join`. If you
consistently use `path.join`, then it's much easier to write cross-platform code,
since it handles the directory separator for you.

`pl.path` provides the same functionality as Python's `os.path` module (11.1).

    > p = 'c:\\bonzo\\DOG.txt'
    > = path.normcase (p)  ---> only makes sense on Windows
    c:\bonzo\dog.txt
    > = path.splitext (p)
    c:\bonzo\DOG    .txt
    > = path.extension (p)
    .txt
    > = path.basename (p)
    DOG.txt
    > = path.exists(p)
    false
    > = path.join ('fred','alice.txt')
    fred\alice.txt
    > = path.exists 'pretty.lua'
    true
    > = path.getsize 'pretty.lua'
    2125
    > = path.isfile 'pretty.lua'
    true
    > = path.isdir 'pretty.lua'
    false


It is very important for all programmers, not just on Unix, to only write to
where they are allowed to write. `path.expanduser` will expand '~' (tilde) into
the home directory. Depending on your OS, this will be a guaranteed place where
you can create files:

    > = path.expanduser '~/mydata.txt'
    'C:\Documents and Settings\SJDonova/mydata.txt'

    > = path.expanduser '~/mydata.txt'
    /home/sdonovan/mydata.txt

Under Windows, `os.tmpname` returns a path which leads to your drive root full of
temporary files. (And increasingly, you do not have access to this root folder.)
This is corrected by `path.tmpname`, which uses the environment variable TMP:

    > os.tmpname()  -- not a good place to put temporary files!
    '\s25g.'
    > path.tmpname()
    'C:\DOCUME~1\SJDonova\LOCALS~1\Temp\s25g.1'


A useful extra function is `pl.path.package_path`, which will tell you the path
of a particular Lua module.  So on my system, `package_path('pl.path')` returns
'C:\Program Files\Lua\5.1\lualibs\pl\path.lua', and `package_path('ifs')` returns
'C:\Program Files\Lua\5.1\clibs\lfs.dll'. It is implemented in terms of
`package.searchpath`, which is a new function in Lua 5.2 which has been
implemented for Lua 5.1 in Penlight.

### File Operations

`pl.file` is a new module that provides more sensible names for common file
operations. For instance, `file.read` and `file.write` are aliases for
`utils.readfile` and `utils.writefile`.

Smaller files can be efficiently read and written in one operation. `file.read`
is passed a filename and returns the contents as a string, if successful; if not,
then it returns `nil` and the actual error message. There is an optional boolean
parameter if you want the file to be read in binary mode (this makes no
difference on Unix but remains important with Windows.)

In previous versions of Penlight, `utils.readfile` would read standard input if
the file was not specified, but this can lead to nasty bugs; use `io.read '*a'`
to grab all of standard input.

Similarly, `file.write` takes a filename and a string which will be written to
that file.

For example, this little script converts a file into upper case:

    require 'pl'
	assert(#arg == 2, 'supply two filenames')
	text = assert(file.read(arg[1]))
    assert(file.write(arg[2],text:upper()))

Copying files is suprisingly tricky. `file.copy` and `file.move` attempt to use
the best implementation possible. On Windows, they link to the API functions
`CopyFile` and `MoveFile`, but only if the `alien` package is installed (this is
true for Lua for Windows.) Otherwise, the system copy command is used. This can
be ugly when writing Windows GUI applications, because of the dreaded flashing
black-box problem with launching processes.

### Directory Operations

`pl.dir` provides some useful functions for working with directories. `fnmatch`
will match a filename against a shell pattern, and `filter` will return any files
in the supplied list which match the given pattern, which correspond to the
functions in the Python `fnmatch` module. `getdirectories` will return all
directories contained in a directory, and `getfiles` will return all files in a
directory which match a shell pattern. These functions return the files as a
table, unlike `lfs.dir` which returns an iterator.)

`dir.makepath` can create a full path, creating subdirectories as necessary;
`rmtree` is the Nuclear Option of file deleting functions, since it will
recursively clear out and delete all directories found begining at a path (there
is a similar function with this name in the Python `shutils` module.)

    > = dir.makepath 't\\temp\\bonzo'
    > = path.isdir 't\\temp\\bonzo'
    true
    > = dir.rmtree 't'

`dir.rmtree` depends on `dir.walk`, which is a powerful tool for scanning a whole
directory tree. Here is the implementation of `dir.rmtree`:

    --- remove a whole directory tree.
    -- @param path A directory path
    function dir.rmtree(fullpath)
        for root,dirs,files in dir.walk(fullpath) do
            for i,f in ipairs(files) do
                os.remove(path.join(root,f))
            end
            lfs.rmdir(root)
        end
    end


`dir.clonetree` clones directory trees. The first argument is a path that must
exist, and the second path is the path to be cloned. (Note that this path cannot
be _inside_ the first path, since this leads to madness.)  By default, it will
then just recreate the directory structure. You can in addition provide a
function, which will be applied for all files found.

    -- make a copy of my libs folder
    require 'pl'
    p1 = [[d:\dev\lua\libs]]
    p2 = [[D:\dev\lua\libs\..\tests]]
    dir.clonetree(p1,p2,dir.copyfile)

A more sophisticated version, which only copies files which have been modified:

    -- p1 and p2 as before, or from arg[1] and arg[2]
    dir.clonetree(p1,p2,function(f1,f2)
      local res
      local t1,t2 = path.getmtime(f1),path.getmtime(f2)
	  -- f2 might not exist, so be careful about t2
      if not t2 or t1 > t2 then
        res = dir.copyfile(f1,f2)
      end
      return res -- indicates successful operation
    end)

`dir.clonetree` uses `path.common_prefix`. With `p1` and `p2` defined above, the
common path is 'd:\dev\lua'. So 'd:\dev\lua\libs\testfunc.lua' is copied to
'd:\dev\lua\test\testfunc.lua', etc.

If you need to find the common path of list of files, then `tablex.reduce` will
do the job:

    > p3 = [[d:\dev]]
    > = tablex.reduce(path.common_prefix,{p1,p2,p3})
    'd:\dev'

