#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

import sys, os, inspect, shutil, platform, site, subprocess

# the destination directory inside Sigil.app
app_dir = os.path.dirname(os.path.realpath(__file__))
app_dir = os.path.join(app_dir, 'Sigil.app','Contents','Frameworks')

# actual version of Python used to build Sigil
build_fwk = os.path.abspath(sys.prefix + '../../../')

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


# the main Python.framework directories
fwk_struct = ['Python.framework/Versions/' + pversion + '/lib/' + stdlib_name + '/site-packages',
              'Python.framework/Versions/' + pversion + '/bin'
]


def copy_python_stdlibrary(src_dir, dest_dir):
    for x in os.listdir(src_dir):
        y = os.path.join(src_dir, x)
        ext = os.path.splitext(x)[1]
        if os.path.isdir(y) and x not in ('test', 'hotshot', 'distutils',
                'site-packages', 'idlelib', 'lib2to3', 'dist-packages', '__pycache__'):
            shutil.copytree(y, os.path.join(dest_dir, x),
                    ignore=ignore_in_dirs)
        if os.path.isfile(y) and ext in ('.py', '.so'):
            shutil.copy2(y, dest_dir)

site_packages = [ ('lxml', 'd'), 
                  ('six.py', 'f'), 
                  ('html5lib','d'), 
                  ('PIL', 'd'), 
                  ('regex.py','f'),
                  ('_regex.so','f'),
                  ('_regex_core.py','f'),
                  ('test_regex.py', 'f')]

def copy_site_packages(packages, site_dest):
    for pkg, typ in packages:
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
            if name in ignored_dirs or not os.path.exists(os.path.join(path, '__init__.py')):
                ans.append(name)
        else:
            if name.rpartition('.')[-1] not in ('so', 'py'):
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
    src_dir = os.path.join(app_dir, 'Python.framework')
    os.chdir(src_dir)
    os.symlink(os.path.join('Versions', pversion, 'Python'), 'Python')
    os.symlink(os.path.join('Versions', pversion, 'Resources'), 'Resources')

    os.chdir(os.path.join(app_dir, 'Python.framework', 'Versions', pversion, 'lib'))
    dylibname = 'libpython' + pversion + 'm.dylib'
    os.symlink('../../../Python', dylibname)

    # finally change any Python.framework rpaths in the Sigil executable to point to the new local Python.framework
    sigil_executable_path = os.path.abspath(os.path.join(app_dir,'..','MacOS','Sigil'))
    rpaths = get_rpaths(sigil_executable_path)
    for rpath in rpaths:
        if 'Python.framework' in rpath:
            new_rpath = '@executable_path/../Frameworks/Python.framework/Versions/' + pversion
            subprocess.check_call(['install_name_tool', '-rpath', rpath, new_rpath, sigil_executable_path])

if __name__ == '__main__':
    sys.exit(main())
