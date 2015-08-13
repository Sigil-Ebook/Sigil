#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

import sys, os, inspect, shutil, platform, textwrap, py_compile, site
from python_paths import py_ver, py_lib, sys_dlls, py_exe, py_inc, py_dest, tmp_prefix, proj_name

# Python standard modules location
srcdir = os.path.dirname(inspect.getfile(os))
pybase = os.path.dirname(srcdir)


# Where we're going to copy stuff
lib_dir = os.path.join(py_dest, 'Lib')
dll_dir = os.path.join(py_dest, 'DLLs')
site_dest = os.path.join(lib_dir, 'site-packages')

# A hack. This must eventually be set by a postinstall script, or
# possibly Sigil itself at runtime. Installing the 32-bit version
# of Sigil on 64-bit Windows will break this hardcoded crap.
pyvenv_home_dir = r'C:\Program Files\%s\Python3'% proj_name

# Cherry-picked additional and/or modified site modules
site_packages = [ ('lxml', 'd'), 
                  ('six.py', 'f'), 
                  ('bs4', 'd'), 
                  ('html5lib','d'), 
                  ('PIL', 'd'), 
                  ('regex.py','f'),
                  ('_regex.pyd','f'),
                  ('_regex_core.py','f'),
                  ('test_regex.py', 'f')]


def copy_site_packages():
    for pkg, typ in site_packages:
        found = False
        for path in site.getsitepackages():
            if not found:
                for entry in os.listdir(path):
                    if entry == pkg:
                        if typ == 'd' and os.path.isdir(os.path.join(path, entry)):
                            shutil.copytree(os.path.join(path, entry), os.path.join(site_dest, entry), ignore=ignore_in_dirs)
                            found = True
                            break
                        else:
                            if os.path.isfile(os.path.join(path, entry)):
                                shutil.copy2(os.path.join(path, entry), os.path.join(site_dest, entry))
                                found = True
                                break
            else:
                break

def ignore_in_dirs(base, items, ignored_dirs=None):
    ans = []
    if ignored_dirs is None:
        ignored_dirs = {'.svn', '.bzr', '.git', 'test', 'tests', 'testing', '__pycache__'}
    for name in items:
        path = os.path.join(base, name)
        if os.path.isdir(path):
            if name in ignored_dirs: # or not os.path.exists(os.path.join(path, '__init__.py')):
                ans.append(name)
        #else:
        #    if name.rpartition('.')[-1] not in ('py', 'pyd', 'dll'):
        #        ans.append(name)
    return ans

def dll_walk():
    shutil.copytree(os.path.join(pybase, "DLLs"), dll_dir,
                ignore=shutil.ignore_patterns('msvc*.dll', 'Microsoft.*'))

    '''for dirpath, dirnames, filenames in os.walk(r'C:\Python%s\Lib'%py_ver):
        if os.path.basename(dirpath) == 'pythonwin':
            continue
        for f in filenames:
            if f.lower().endswith('.dll'):
                f = os.path.join(dirpath, f)
                shutil.copy2(f, dll_dir)'''

def copy_tk_tcl():
    def ignore_lib(root, items):
            ans = []
            for x in items:
                ext = os.path.splitext(x)[1]
                if (not ext and (x in ('demos', 'tzdata'))) or \
                    (ext in ('.chm', '.htm', '.txt')):
                    ans.append(x)
            return ans

    src = os.path.join(pybase, "tcl")
    for entry in os.listdir(src):
        if entry in ('tk8.6', 'tcl8.6'):
            if os.path.isdir(os.path.join(src, entry)):
                shutil.copytree(os.path.join(src, entry), os.path.join(lib_dir, entry), ignore=ignore_lib)

def copy_pylib():
    #shutil.copy2(py_lib, py_dest)
    try: 
        shutil.copy2(os.path.join(sys_dlls, 'python%s.dll'%py_ver), py_dest)
        shutil.copy2(os.path.join(sys_dlls, 'python%s.dll'%py_ver), tmp_prefix)
    except:
        print ('Couldn\'t find the Python%s.dll file. May need to include -DSYS_DLL_DIR="c:\windows\syswow64" in the cmake command.'%py_ver)
        exit
    try:
        shutil.copy2(os.path.join(sys_dlls, 'pywintypes%s.dll'%py_ver), py_dest)
        shutil.copy2(os.path.join(sys_dlls, 'pythoncom%s.dll'%py_ver), py_dest)
    except:
        pass
    shutil.copy2(py_exe, os.path.join(py_dest, "sigil-python3.exe"))


def copy_python():
    def ignore_lib(root, items):
        ans = []
        for x in items:
            ext = os.path.splitext(x)[1]
            if (not ext and (x in ('demos', 'tests', 'test', 'idlelib', 'lib2to3', '__pycache__', 'site-packages'))) or \
                (ext in ('.chm', '.htm', '.txt')):
                ans.append(x)
        return ans

    shutil.copytree(os.path.join(pybase, "Lib"), lib_dir,
                ignore=ignore_lib)
    

def compile_libs():
    for x in os.walk(lib_dir):
        for f in x[-1]:
            if f.endswith('.py'):
                y = os.path.join(x[0], f)
                rel = os.path.relpath(y, lib_dir)
                try:
                    py_compile.compile(y, cfile=y+'o',dfile=rel, doraise=True, optimize=2)
                    os.remove(y)
                    z = y+'c'
                    if os.path.exists(z):
                        os.remove(z)
                except:
                    print ('Failed to byte-compile', y)

def create_site_py():
    with open(os.path.join(lib_dir, 'site.py'), 'wb') as f:
        f.write(bytes(textwrap.dedent('''\
        import sys
        import builtins
        import os
        import _sitebuiltins

        def set_helper():
            builtins.help = _sitebuiltins._Helper()

        def fix_sys_path():
            if os.sep == '/':
                sys.path.append(os.path.join(sys.prefix, "lib",
                                "python" + sys.version[:3],
                                "site-packages"))
            else:
                for path in sys.path:
                    py_ver = "".join(map(str, sys.version_info[:2]))
                    if os.path.basename(path) == "python" + py_ver + ".zip":
                        sys.path.remove(path)
                sys.path.append(os.path.join(sys.prefix, "lib", "site-packages"))

        def main():
            try:
                fix_sys_path()
                set_helper()
            except SystemExit as err:
                if err.code is None:
                    return 0
                if isinstance(err.code, int):
                    return err.code
                print (err.code)
                return 1
            except:
                import traceback
                traceback.print_exc()
            return 1

        if not sys.flags.no_site:
            main()
            '''), 'UTF-8'))

def create_pyvenv():
    with open(os.path.join(py_dest, 'pyvenv.cfg'), 'wb') as f:
        f.write(bytes(textwrap.dedent('''\
        home = %s
        include-system-site-packages = false
        version = 3.4.0
        ''') % pyvenv_home_dir, 'UTF-8'))


if __name__ == '__main__':
    dll_walk()
    copy_pylib()
    copy_python()
    copy_site_packages()
    create_site_py()
    create_pyvenv()
    copy_tk_tcl()
    compile_libs()
