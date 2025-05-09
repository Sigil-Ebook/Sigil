#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

import sys, os, subprocess, glob, inspect, shutil, platform, textwrap, site

# Get "real" python binary, libs and stdlibs regardless if a venv is being used.
global base_dest
global py_ver
global pylib
global py_exe

#AppImage Destination directories
global lib_dir
global lib_dynload
global site_dest
global bin_dir


# QUiTools needs QtOpenGlWidgets (on Windows anyway)
PYSIDE6_MODULES = [
    'QtCore', 'QtDBus', 'QtGui', 'QtNetwork', 'QtOpenGLWidgets', 'QtPdf', 'QtPrintSupport',
    'QtUiTools','QtWebEngine', 'QtWebEngineCore', 'QtWebEngineWidgets',
    'QtWebChannel', 'QtSvg', 'QtWidgets', 'Shiboken'
    ]

# Cherry-picked additional and/or modified site modules
site_packages = [ ('lxml', 'd'), 
                  ('six.py', 'f'),
                  ('urllib3', 'd'),
                  ('certifi', 'd'),
                  ('dulwich', 'd'),
                  ('css_parser', 'd'),
                  ('html5lib','d'), 
                  ('PIL', 'd'),
                  ('pillow.libs', 'd'), 
                  ('regex','d'),
                  ('cssselect', 'd'),
                  ('webencodings', 'd'), # needed by html5lib
                  ('chardet', 'd'),
                  ('shiboken6', 'd'),
                  ('PySide6', 'd')]


def set_rpath(f, depth=0, addorigin=False):
    if depth == 0:
        subprocess.check_call(['patchelf', '--set-rpath', '$ORIGIN', f])
        print('Setting rpath of {} to $ORIGIN'.format(os.path.basename(f)))
        return
    else:
        recurse = ''.join(['../' for x in range(depth)])
        if addorigin:
             rpath = '$ORIGIN:$ORIGIN/{}{}'.format(recurse, 'lib')
        else:
            rpath = '$ORIGIN/{}{}'.format(recurse, 'lib')
        subprocess.check_call(['patchelf', '--set-rpath', rpath, f])
        print('Setting rpath of {} to {}'.format(os.path.basename(f), rpath))
        return


def copy_site_packages():
    if not os.path.exists(site_dest):
        os.makedirs(site_dest)
    for pkg, typ in site_packages:
        found = False
        # Uses site-packages from venv or base python
        for path in site.getsitepackages():
            if not found:
                for entry in os.listdir(path):
                    if entry == pkg:
                        if typ == 'd' and os.path.isdir(os.path.join(path, entry)):
                            if pkg in ('PySide6', 'shiboken6'):
                                shutil.copytree(os.path.join(path, entry), os.path.join(site_dest, entry), dirs_exist_ok=True, ignore=ignore_in_pyside6_dirs)
                                print('Got some pyside6!')
                            else:
                                print('Here we are dir!')
                                print(f'Src: {os.path.join(path, entry)}')
                                print(f'Dest: {os.path.join(site_dest, entry)}')
                                if entry == 'pillow.libs':
                                    shutil.copytree(os.path.join(path, entry), os.path.join(site_dest, entry), dirs_exist_ok=True)
                                else:
                                    shutil.copytree(os.path.join(path, entry), os.path.join(site_dest, entry), dirs_exist_ok=True, ignore=ignore_in_dirs)
                            found = True
                            break
                        else:
                            if os.path.isfile(os.path.join(path, entry)):
                                print('Here we are file!')
                                print(f'Src: {os.path.join(path, entry)}')
                                print(f'Dest: {os.path.join(site_dest, entry)}')
                                shutil.copy2(os.path.join(path, entry), os.path.join(site_dest, entry))
                                found = True
                                break
            else:
                break
        if not found:
            print('WARNING: %s not found in site_packages. Your python environment could potentially be incomplete.' % pkg)


def ignore_in_pyside6_dirs(base, items, ignored_dirs=None):
    ans = []
    if ignored_dirs is None:
        ignored_dirs = {'.svn', '.bzr', '.git', 'docs', 'examples', 'glue', 'include', 'metatypes', 'modules', 
                       'plugins', 'Qt', 'qml', 'resources', 'scripts', 'support', 'translations', 'typesystems', '__pycache__'}
    for name in items:
        path = os.path.join(base, name)
        if os.path.isdir(path):
            if name in ignored_dirs:  # or not os.path.exists(os.path.join(path, '__init__.py')):
                ans.append(name)
        else:
            #if name.rpartition('.')[-1] == '':
            #    ans.append(name)
            if not os.path.splitext(name)[1]:
                ans.append(name)
            if name.startswith('libpyside6') or name.startswith('libshiboken6'):
                pass
            if name.rpartition('.')[-1] in ('py', 'pyi', 'so'):
                pass
            if name.startswith('Qt') and name.partition('.')[0] not in PYSIDE6_MODULES:
                ans.append(name)
            if name.rpartition('.')[-1] == 'pyi' and name.partition('.')[0] not in PYSIDE6_MODULES:
                ans.append(name)
            if name.rpartition('.')[-1] == 'so' and name.partition('.')[0] not in PYSIDE6_MODULES:
                ans.append(name)
    return ans


def ignore_in_dirs(base, items, ignored_dirs=None):
    ans = []
    if ignored_dirs is None:
        ignored_dirs = {'.svn', '.bzr', '.git', 'test', 'tests', 'testing', 'turtledemo', '__pycache__'}
    for name in items:
        path = os.path.join(base, name)
        if os.path.isdir(path):
            if name in ignored_dirs: # or not os.path.exists(os.path.join(path, '__init__.py')):
                ans.append(name)
        #else:
        #    if name.rpartition('.')[-1] not in ('so', 'py'):
        #        ans.append(name)
    return ans



def copy_tk_tcl():
    def ignore_lib(root, items):
            ans = []
            for x in items:
                ext = os.path.splitext(x)[1]
                if (not ext and (x in ('demos', 'tzdata'))) or \
                    (ext in ('.chm', '.htm', '.txt')):
                    ans.append(x)
            return ans


    src = os.path.join('/usr/share', 'tcltk')
    for entry in os.listdir(src):
        if entry in ('tk8.6', 'tcl8.6'):
            if os.path.isdir(os.path.join(src, entry)):
                main_lib_dir = os.path.abspath(os.path.join(lib_dir, ".."))
                shutil.copytree(os.path.join(src, entry), os.path.join(main_lib_dir, entry), dirs_exist_ok=True, ignore=ignore_lib)


def copy_pybin():
    new_py_binary = os.path.join(bin_dir, 'python3')
    shutil.copy2(py_exe, new_py_binary)
    # Set RPATH for python binary
    set_rpath(new_py_binary, depth=1)


def copy_python():
    def ignore_lib(root, items):
        ans = []
        for x in items:
            ext = os.path.splitext(x)[1]
            if (not ext and (x in ('turtledemo', 'demos', 'tests', 'test', 'idlelib', 'lib2to3', '__pycache__', 'site-packages')) or x.startswith('plat-')) or \
                (ext in ('.chm', '.htm', '.txt')):
                ans.append(x)
        return ans

    shutil.copytree(pylib, lib_dir, ignore=ignore_lib)

    # Set RPATH for shared libraries in lib-dynload to root lib dir
    #root = os.path.join(py_dir, 'lib-dynload')
    for item in os.listdir(lib_dynload):
        f = os.path.join(lib_dynload, item)
        if os.path.isfile(f) and os.path.splitext(f)[1] == '.so':
            set_rpath(f, depth=3, addorigin=True)


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


def fix_pyside6_qt_conf():
    _conf_ = os.path.join(site_dest, 'PySide6', 'qt.conf')
    if os.path.exists(_conf_) and os.path.isfile(_conf_):
        with open(_conf_, 'wb') as f:
            f.write(bytes(textwrap.dedent('''\
            [Paths]
            Prefix = ../../..
            '''), 'ascii'))


def patch_pillow_init():
    # PyPi-installed Pillow's __init__.py trys to modify doc strings
    # that are unavailable since we compile all of them away.
    _init_ = os.path.join(site_dest, 'PIL', '__init__.py')
    criteria = '(__doc__ =.*)$'
    if os.path.exists(_init_) and os.path.isfile(_init_):
        import re
        with open(_init_, 'r') as f:
            content = f.read()
            new_content = re.sub(criteria, r'#\1', content, flags = re.M)
        with open(_init_, 'wb') as f:
            f.write(bytes(new_content, 'UTF-8'))


if __name__ == '__main__':
    base_dest = sys.argv[1]
    py_ver = sys.argv[2]

    print(f'base_dest: {base_dest}')
    print(f'py_ver: {py_ver}')
    
    pybase = sys.base_prefix
    print(f'Base Prefix: {pybase}')
    
    pylib = os.path.join(pybase, 'lib', f'python{py_ver}')
    py_exe = os.path.join(pybase, 'bin', 'python3')
    print(site.getsitepackages())
    #files = os.listdir(pylib)
    #print(files)

    lib_dir = os.path.join(base_dest, 'usr', 'lib', f'python{py_ver}',)
    lib_dynload = os.path.join(lib_dir, 'lib-dynload')
    #os.makedirs(lib_dir, exist_ok=True)
    bin_dir = os.path.join(base_dest, 'usr', 'bin')
    os.makedirs(bin_dir, exist_ok=True)
    site_dest = os.path.join(lib_dir, 'site-packages')
    #os.makedirs(site_dest, exist_ok=True)
    #dll_walk()
    copy_pybin()
    copy_python()
    copy_site_packages()
    #create_site_py()
    # create_pyvenv()
    # create_qt_conf()
    copy_tk_tcl()
    #fix_pyside6_qt_conf()
    #patch_pillow_init()
    #compile_libs()
    pass