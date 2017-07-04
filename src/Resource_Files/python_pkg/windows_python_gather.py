#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

import sys, os, inspect, shutil, platform, textwrap, py_compile, site
from python_paths import py_ver, py_lib, sys_dlls, py_exe, py_inc, py_dest, tmp_prefix, proj_name, include_pyqt5

# Python standard modules location
srcdir = os.path.dirname(inspect.getfile(os))
pybase = os.path.dirname(srcdir)

# Where we're going to copy stuff
lib_dir = os.path.join(tmp_prefix, 'Lib')
dll_dir = os.path.join(tmp_prefix, 'DLLs')
site_dest = os.path.join(lib_dir, 'site-packages')

PYQT_MODULES = ['%s.pyd' % x for x in (
    'Qt', 'QtCore', 'QtGui', 'QtNetwork', 'QtPrintSupport', 'QtSvg', 'QtWidgets'
    )]
EXCLUDED_UIC_WIDGET_PLUGINS = ['%s.py' % x for x in (
    'qaxcontainer', 'qscintilla', 'qtcharts', 'qtquickwidgets', 'qtwebenginewidgets', 'qtwebkit'
    )]

# Cherry-picked additional and/or modified site modules
site_packages = [ ('lxml', 'd'), 
                  ('six.py', 'f'), 
                  ('html5lib','d'), 
                  ('PIL', 'd'), 
                  ('regex.py','f'),
                  ('_regex.pyd','f'),
                  ('_regex_core.py','f'),
                  ('test_regex.py', 'f'),
                  ('cssselect', 'd'),
                  ('encutils', 'd'),
                  ('cssutils', 'd'),
                  ('webencodings', 'd'), # needed by html5lib
                  ('chardet', 'd'),
                  ('sip.pyd', 'f')]

if include_pyqt5:
    site_packages.append( ('PyQt5', 'd') )



def copy_site_packages():
    for pkg, typ in site_packages:
        found = False
        for path in site.getsitepackages():
            if not found:
                for entry in os.listdir(path):
                    if entry == pkg:
                        if typ == 'd' and os.path.isdir(os.path.join(path, entry)):
                            if pkg == 'PyQt5':
                                shutil.copytree(os.path.join(path, entry), os.path.join(site_dest, entry), ignore=ignore_in_pyqt5_dirs)
                            else:
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


def ignore_in_pyqt5_dirs(base, items, ignored_dirs=None):
    ans = []
    if ignored_dirs is None:
        ignored_dirs = {'.svn', '.bzr', '.git', 'doc', 'examples', 'includes', 'mkspecs',
                       'plugins', 'qml', 'qsci', 'qt', 'sip', 'translations', 'port_v2', '__pycache__'}
    for name in items:
        path = os.path.join(base, name)
        if os.path.isdir(path):
            if name in ignored_dirs:  # or not os.path.exists(os.path.join(path, '__init__.py')):
                ans.append(name)
        else:
            if name.rpartition('.')[-1] not in ('py', 'pyd'):
                ans.append(name)
            if name.rpartition('.')[-1] == 'pyd' and name not in PYQT_MODULES:
                ans.append(name)
            if name.rpartition('.')[-1] == 'py' and name in EXCLUDED_UIC_WIDGET_PLUGINS:
                ans.append(name)
    return ans


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
    fldrs = (pybase, sys_dlls)
    dll_found = False
    for fldr in fldrs:
        if os.path.exists(os.path.join(fldr, 'python%s.dll'%py_ver)):
            shutil.copy2(os.path.join(fldr, 'python%s.dll'%py_ver), tmp_prefix)
            dll_found = True
            try:
                shutil.copy2(os.path.join(fldr, 'python3.dll'), tmp_prefix)
            except:
                pass
            break
    if not dll_found:
        print ('Couldn\'t find the Python%s.dll file.'%py_ver)
        exit
    shutil.copy2(py_exe, os.path.join(tmp_prefix, "python3.exe"))


def copy_python():
    def ignore_lib(root, items):
        ans = []
        for x in items:
            ext = os.path.splitext(x)[1]
            if (not ext and (x in ('demos', 'tests', 'test', 'idlelib', 'lib2to3', '__pycache__', 'site-packages')) or x.startswith('plat-')) or \
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
                    py_compile.compile(y, cfile=y+'c',dfile=rel, doraise=True, optimize=2)
                    os.remove(y)
                    z = y+'o'
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
                    py_ver = "".join(map(str, sys.version[:3])).replace(".", "")
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
    with open(os.path.join(tmp_prefix, 'pyvenv.cfg'), 'wb') as f:
        f.write(bytes(textwrap.dedent('''\
        applocal = true
        '''), 'UTF-8'))


def create_qt_conf():
    with open(os.path.join(tmp_prefix, 'qt.conf'), 'wb') as f:
        f.write(bytes(textwrap.dedent('''\
        [Paths]
        Prefix = .
        Plugins = .
        Binaries = .
        '''), 'ascii'))


if __name__ == '__main__':
    dll_walk()
    copy_pylib()
    copy_python()
    copy_site_packages()
    create_site_py()
    # create_pyvenv()
    # create_qt_conf()
    copy_tk_tcl()
    compile_libs()
