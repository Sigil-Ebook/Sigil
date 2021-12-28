#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

import sys, os, inspect, shutil, platform, site, subprocess, py_compile

# the destination directory inside Sigil.app
app_dir = os.path.dirname(os.path.realpath(__file__))
app_dir = os.path.join(app_dir, 'Sigil.app','Contents','Frameworks')

# actual version of Python used to build Sigil
build_fwk = os.path.abspath(sys.prefix)

# get python version string
pversion = build_fwk.split(os.sep)[-1]

#get library directory and basename
stdlib_dir = os.path.dirname(inspect.getfile(os))
stdlib_name = stdlib_dir.split(os.sep)[-1]

print('build_fwk', build_fwk)
print('pversion', pversion)
print('stdlib_dir', stdlib_dir)
print('stdlib_name', stdlib_name)
print('app_dir', app_dir)
print('sys.prefix', os.path.abspath(sys.prefix))

# the main Python.framework directories
fwk_struct = ['Python.framework/Versions/' + pversion + '/lib/' + stdlib_name + '/site-packages',
              'Python.framework/Versions/' + pversion + '/bin'
]

# minimal set of PySide modules to support the plugin gui need *.so and *.pyi
PYSIDE6_MODULES = ['QtCore', 'QtDBus', 'QtGui', 'QtNetwork', 'QtPrintSupport',
                  'QtSvg', 'QtWidgets', 'QtWebEngine', 'QtWebEngineCore',
                   'QtWebEngineWidgets', 'QtWebChannel', 'QtUiTools',
                   'QtOpenGL', 'QtOpenGLWidgets']


# additional external python modules/packages that need to be included
site_packages = [ ('lxml', 'd'), 
                  ('six.py', 'f'), 
                  ('html5lib','d'), 
                  ('PIL', 'd'),
                  ('regex', 'd'),
                  ('certifi', 'd'),
                  ('cssselect', 'd'),
                  ('urllib3', 'd'),
                  ('dulwich', 'd'),
                  ('encutils', 'd'),
                  ('css_parser', 'd'),
                  ('webencodings', 'd'), # needed by html5lib
                  ('chardet', 'd'),
                  ('shiboken6', 'd'),
                  ('PySide6', 'd')]


def copy_python_tcltk(src_dir, dest_dir):
    for x in os.listdir(src_dir):
        y = os.path.join(src_dir, x)
        ext = os.path.splitext(x)[1]
        if os.path.isdir(y) and x not in ('python' + pversion, 'tcl8', 'pkgconfig', 'demos', 'tzdata'):
            shutil.copytree(y, os.path.join(dest_dir, x),
                    ignore=ignore_in_tcltk_dirs)
        if os.path.isfile(y) and ext not in ('.sh', '.chm', '.htm', '.txt'):
            if x != 'libpython' + pversion + '.dylib': 
                shutil.copy2(y, dest_dir)


def copy_python_stdlibrary(src_dir, dest_dir):
    for x in os.listdir(src_dir):
        y = os.path.join(src_dir, x)
        ext = os.path.splitext(x)[1]
        if os.path.isdir(y) and x not in ('test', 'hotshot', 'site-packages', 
                                          'idlelib', 'lib2to3', 'dist-packages', '__pycache__'):
            shutil.copytree(y, os.path.join(dest_dir, x),
                    ignore=ignore_in_dirs)
        if os.path.isfile(y) and ext in ('.py', '.so'):
            shutil.copy2(y, dest_dir)


def copy_site_packages(packages, site_dest):
    for pkg, typ in packages:
        found = False
        for apath in site.getsitepackages():
            if not found and os.path.exists(apath) and os.path.isdir(apath):
                apath = os.path.abspath(apath)
                for entry in os.listdir(apath):
                    if entry == pkg:
                        if typ == 'd' and os.path.isdir(os.path.join(apath, entry)):
                            if pkg == 'PySide6':
                                shutil.copytree(os.path.join(apath, entry), 
                                                os.path.join(site_dest, entry), ignore=ignore_in_pyside6_dirs)
                            else:
                                shutil.copytree(os.path.join(apath, entry), 
                                                os.path.join(site_dest, entry), ignore=ignore_in_dirs)
                            found = True
                            break
                        else:
                            if os.path.isfile(os.path.join(apath, entry)):
                                shutil.copy2(os.path.join(apath, entry), os.path.join(site_dest, entry))
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
            if name in ignored_dirs:
                ans.append(name)
    return ans


def ignore_in_pyside6_dirs(base, items, ignored_dirs=None):
    ans = []
    if ignored_dirs is None:
        ignored_dirs = {'.git', 'glue', 'include', 'typesystems', 'examples', 'Linguist.app',
                        'Assistant.app', 'Designer.app', '__pycache__'}
    for name in items:
        path = os.path.join(base, name)
        if os.path.isdir(path):
            if name in ignored_dirs:
                ans.append(name)
        else:
            if name.rpartition('.')[-1] not in ('so', 'py', 'dylib', 'pyi'):
                if name not in ('lupdate', 'lrelease', 'uic', 'rcc'):
                    ans.append(name)
            if name.rpartition('.')[-1] == 'so' and name.partition('.')[0]  not in PYSIDE6_MODULES:
                ans.append(name)
            if name.rpartition('.')[-1] == 'pyi' and name.partition('.')[0]  not in PYSIDE6_MODULES:
                ans.append(name)
    return ans


def ignore_in_tcltk_dirs(base, items, ignored_dirs=None):
    ans = []
    dylibname = 'libpython' + pversion + '.dylib'
    if ignored_dirs is None:
        ignored_dirs = {'tcl8', 'python' + pversion, 'pkgconfig', 'demos', 'tzdata'}
    for name in items:
        path = os.path.join(base, name)
        if os.path.isdir(path):
            if name in ignored_dirs:
                ans.append(name)
        else:
            if name.rpartition('.')[-1] in ('chm', 'htm', 'html', 'txt' ):
                ans.append(name)
            if name == dylibname:
                ans.append(name)
    return ans


def get_rpaths(path_to_executable):
    rpaths = []
    raw = subprocess.check_output(['otool', '-l', path_to_executable])
    found_rpath = False
    for line in raw.splitlines():
        if b'LC_RPATH' in line:
            found_rpath = True
            continue
        if found_rpath:
            if b'path ' in line:
                uline = line.decode('utf-8')
                bp = uline.find('path ') + 5
                ep = uline.find('(')
                rpath = uline[bp:ep].strip()
                rpaths.append(rpath)
                found_rpath = False
    return rpaths


def main():
    # create the location inside Sigil.app for Frameworks
    os.makedirs(app_dir, exist_ok=True)

    # create the basic Python.framework structure
    for pth in fwk_struct:
        os.makedirs(os.path.join(app_dir, pth), exist_ok=True)

    # first copy all python standard library files to their proper place in the framework
    dest_dir = os.path.join(app_dir,'Python.framework','Versions', pversion, 'lib', stdlib_name)
    copy_python_stdlibrary(stdlib_dir, dest_dir)

    # now handle the site-packages separately
    dest_dir = os.path.join(app_dir,'Python.framework','Versions', pversion, 'lib', stdlib_name, 'site-packages')
    copy_site_packages(site_packages, dest_dir)

    # now pre-compile all of the .py code and replace it by .pyc
    dest_dir = os.path.join(app_dir,'Python.framework','Versions', pversion, 'lib', stdlib_name)
    for x in os.walk(dest_dir):
        for f in x[-1]:
            if f.endswith('.py'):
                y = os.path.join(x[0], f)
                rel = os.path.relpath(y, dest_dir)
                try:
                    py_compile.compile(y, cfile=y+'c',dfile=rel, doraise=True, optimize=-1)
                    os.remove(y)
                except:
                    print ('Failed to byte-compile', y)

    # copy the embedded tcl/tk fpieces
    tcltksrcdir = os.path.join(build_fwk, 'lib')
    tcltkdestdir = os.path.join(app_dir,  'Python.framework', 'Versions', pversion, 'lib')
    copy_python_tcltk(tcltksrcdir, tcltkdestdir)

    # next copy the bin
    src_file = os.path.join(build_fwk, 'bin', 'python3')
    dest_file = os.path.join(app_dir,  'Python.framework', 'Versions', pversion, 'bin', 'python3')
    shutil.copy2(src_file, dest_file)

    # next copy the framework (dylib itself) itself
    src_file = os.path.join(build_fwk, 'Python')
    dest_file = os.path.join(app_dir,  'Python.framework','Versions', pversion, 'Python')
    shutil.copy2(src_file, dest_file)

    # copy the Resources recursively
    # Note: for copytree to work, the destination must NOT already exist
    src_dir = os.path.join(build_fwk, 'Resources')
    dest_dir = os.path.join(app_dir,  'Python.framework','Versions', pversion, 'Resources')
    shutil.copytree(src_dir, dest_dir)

    # now create proper symlinks to make everything work
    src_dir = os.path.join(app_dir, 'Python.framework/Versions')
    os.chdir(src_dir)
    os.symlink(pversion, 'Current')
    src_dir = os.path.join(app_dir, 'Python.framework')
    os.chdir(src_dir)
    os.symlink(os.path.join('Versions','Current', 'Python'), 'Python')
    os.symlink(os.path.join('Versions', 'Current', 'Resources'), 'Resources')

    os.chdir(os.path.join(app_dir, 'Python.framework', 'Versions', pversion, 'lib'))
    # dylibname = 'libpython' + pversion + 'm.dylib'
    dylibname = 'libpython' + pversion + '.dylib'
    os.symlink('../Python', dylibname)

    # Change any Python.framework rpaths in the Sigil executable to point to the new local Python.framework
    sigil_executable_path = os.path.abspath(os.path.join(app_dir,'..','MacOS','Sigil'))
    rpaths = get_rpaths(sigil_executable_path)
    for rpath in rpaths:
        if 'Python.framework' in rpath:
            new_rpath = '@executable_path/../Frameworks/Python.framework/Versions/' + pversion
            subprocess.check_call(['install_name_tool', '-rpath', rpath, new_rpath, sigil_executable_path])

if __name__ == '__main__':
    sys.exit(main())
