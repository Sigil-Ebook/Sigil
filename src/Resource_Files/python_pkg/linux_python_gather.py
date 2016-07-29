#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

import sys, os, inspect, shutil, subprocess, glob, platform, textwrap, py_compile, site
from python_paths import py_ver, py_lib, py_exe, sigil_src, installer_name, tk_lib, tcl_lib, tcltk_support
from python_paths import cmake_build_root, qt_libs_dir, qt_plugins_dir, extra_manifest

# Python standard modules location
srcdir = os.path.dirname(inspect.getfile(os))
# print ('srcdir', srcdir)

temp_folder = os.path.join(cmake_build_root, 'temp_folder')
# print ('tmp_folder', temp_folder)

app_folder = os.path.join(temp_folder, 'sigil-ebook')
#print ('app_folder', app_folder)

py_dest = os.path.join(app_folder, 'python3')
# print ('py_dest', py_dest)

# Where we're going to copy stuff
py_dir = os.path.join(py_dest, 'lib', os.path.basename(srcdir))
# print ('py_dir', py_dir)

site_dest = os.path.join(py_dir, 'site-packages')
# print ('site_dest', site_dest)

if os.path.exists(temp_folder) and os.path.isdir(temp_folder):
    shutil.rmtree(temp_folder)

os.makedirs(py_dir)
os.makedirs(os.path.join(py_dest, 'bin'))
if not os.path.exists(os.path.join(cmake_build_root, 'installer')):
    os.makedirs(os.path.join(cmake_build_root, 'installer'))


# Cherry-picked additional and/or modified modules
site_packages = [ ('lxml', 'd'), 
                  ('six.py', 'f'), 
                  ('html5lib','d'), 
                  ('PIL', 'd'), 
                  ('regex.py','f'),
                  ('_regex.cpython-34m.so','f'),
                  ('_regex_core.py','f'),
                  ('test_regex.py', 'f'),
                  ('cssselect', 'd'),
                  ('encutils', 'd'),
                  ('cssutils', 'd'),
                  ('chardet', 'd')]


def copy_site_packages(packages, dest):
    #if not os.path.exists(dest):
    #    os.mkdir(dest)
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

def copy_pylib():
    shutil.copy2(py_lib, app_folder)
    shutil.copy2(py_exe, os.path.join(py_dest, 'bin', "sigil-python3"))


def copy_python():
    
    if not os.path.exists(py_dir):
        os.mkdir(py_dir)

    for x in os.listdir(srcdir):
        y = os.path.join(srcdir, x)
        ext = os.path.splitext(x)[1]
        if os.path.isdir(y) and x not in ('test', 'hotshot', 'distutils',
                'site-packages', 'idlelib', 'lib2to3', 'dist-packages', '__pycache__'):
            shutil.copytree(y, os.path.join(py_dir, x),
                    ignore=ignore_in_dirs)
        if os.path.isfile(y) and ext in ('.py', '.so'):
            shutil.copy2(y, py_dir)

    #site_dest = os.path.join(py_dir, 'site-packages')
    copy_site_packages(site_packages, site_dest)
    create_site_py()

    for x in os.walk(py_dir):
        for f in x[-1]:
            if f.endswith('.py'):
                y = os.path.join(x[0], f)
                rel = os.path.relpath(y, py_dir)
                try:
                    py_compile.compile(y, cfile=y+'o',dfile=rel, doraise=True, optimize=2)
                    os.remove(y)
                    z = y+'c'
                    if os.path.exists(z):
                        os.remove(z)
                except:
                    print ('Failed to byte-compile', y)

def copy_tcltk():
    shutil.copy2(tk_lib, app_folder)
    shutil.copy2(tcl_lib, app_folder)

    def ignore_lib(root, items):
            ans = []
            for x in items:
                ext = os.path.splitext(x)[1]
                if (not ext and (x in ('demos', 'tzdata'))) or \
                    (ext in ('.chm', '.htm', '.txt')):
                    ans.append(x)
            return ans

    for entry in os.listdir(tcltk_support):
        if entry in ('tk8.6', 'tcl8.6'):
            if os.path.isdir(os.path.join(tcltk_support, entry)):
                shutil.copytree(os.path.join(tcltk_support, entry), os.path.join(py_dest, 'lib', entry), ignore=ignore_lib)

def create_site_py():
    with open(os.path.join(py_dir, 'site.py'), 'wb') as f:
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

def create_pyvenv(name, prefix):
    pyvenv_home = os.path.join(prefix, 'python3', 'pyvenv.cfg')
    with open(os.path.join(temp_folder, name), 'wb') as f:
        f.write(bytes(textwrap.dedent('''\
        home = %s
        include-system-site-packages = false
        version = 3.4.3
        ''') % pyvenv_home, 'UTF-8'))

def copy_strip_qt5():
    qtlibs = ['Qt5Concurrent', 'Qt5Core', 'Qt5DBus', 'Qt5Gui', 'Qt5Multimedia', 'Qt5MultimediaWidgets', 'Qt5Network',
              'Qt5OpenGL', 'Qt5Positioning', 'Qt5PrintSupport', 'Qt5Qml', 'Qt5Quick', 'Qt5Sensors', 'Qt5Sql', 'Qt5Svg',
              'Qt5WebChannel', 'Qt5WebKit', 'Qt5WebKitWidgets', 'Qt5Widgets', 'Qt5Xml', 'Qt5XmlPatterns']
    for lib in qtlibs:
        name = os.path.join(qt_libs_dir, 'lib'+lib+'.so.5')
        shutil.copy2(name, app_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(app_folder, 'lib'+lib+'.so.5')])

def copy_strip_icu5(ver='53'):
    iculibs = ['icudata', 'icui18n', 'icuuc']
    for lib in iculibs:
        name = os.path.join(qt_libs_dir, 'lib'+lib+'.so.'+ver)
        shutil.copy2(name, app_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(app_folder, 'lib'+lib+'.so.'+ver)])

def copy_strip_qt_plugins():
    # Copy iconengines plugins
    dest_folder = os.path.join(app_folder, 'iconengines')
    os.mkdir(dest_folder)
    shutil.copy2(os.path.join(qt_plugins_dir, 'iconengines', 'libqsvgicon.so'), dest_folder)
    subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'libqsvgicon.so')])

    # Copy imageformats plugins
    imagelibs = ['qgif', 'qico', 'qjpeg', 'qmng', 'qsvg', 'qtiff', 'qtga', 'qwbmp']
    dest_folder = os.path.join(app_folder, 'imageformats')
    os.mkdir(dest_folder)
    for lib in imagelibs:
        name = os.path.join(qt_plugins_dir, 'imageformats', 'lib'+lib+'.so')
        shutil.copy2(name, dest_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'lib'+lib+'.so')])

    # Copy audio plugins
    audiolibs = ['qtaudio_alsa', 'qtmedia_pulse']
    dest_folder = os.path.join(app_folder, 'audio')
    os.mkdir(dest_folder)
    for lib in audiolibs:
        name = os.path.join(qt_plugins_dir, 'audio', 'lib'+lib+'.so')
        shutil.copy2(name, dest_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'lib'+lib+'.so')])

    # Copy mediaservice plugins
    medialibs = ['gstmediaplayer', 'gstaudiodecoder']
    dest_folder = os.path.join(app_folder, 'mediaservice')
    os.mkdir(dest_folder)
    for lib in medialibs:
        name = os.path.join(qt_plugins_dir, 'mediaservice', 'lib'+lib+'.so')
        shutil.copy2(name, dest_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'lib'+lib+'.so')])

    # Copy platform plugins
    platlibs = ['qminimal', 'qxcb', 'qoffscreen', 'qminimalegl', 'qlinuxfb', 'qeglfs']
    dest_folder = os.path.join(app_folder, 'platforms')
    os.mkdir(dest_folder)
    for lib in platlibs:
        name = os.path.join(qt_plugins_dir, 'platforms', 'lib'+lib+'.so')
        shutil.copy2(name, dest_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'lib'+lib+'.so')])

    # Copy platformtheme plugins
    dest_folder = os.path.join(app_folder, 'platformthemes')
    os.mkdir(dest_folder)
    shutil.copy2(os.path.join(qt_plugins_dir, 'platformthemes', 'libqgtk2.so'), dest_folder)
    subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'libqgtk2.so')])

    # Copy platforminputcontexts plugins
    platinputlibs = ['composeplatforminputcontextplugin', 'ibusplatforminputcontextplugin']
    dest_folder = os.path.join(app_folder, 'platforminputcontexts')
    os.mkdir(dest_folder)
    for lib in platinputlibs:
        name = os.path.join(qt_plugins_dir, 'platforminputcontexts', 'lib'+lib+'.so')
        shutil.copy2(name, dest_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'lib'+lib+'.so')])

    # Copy printsupport plugins
    dest_folder = os.path.join(app_folder, 'printsupport')
    os.mkdir(dest_folder)
    shutil.copy2(os.path.join(qt_plugins_dir, 'printsupport', 'libcupsprintersupport.so'), dest_folder)
    subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'libcupsprintersupport.so')])

    # Copy sqldrivers plugins
    sqllibs = ['qsqlite', 'qsqlmysql', 'qsqlpsql']
    dest_folder = os.path.join(app_folder, 'sqldrivers')
    os.mkdir(dest_folder)
    for lib in sqllibs:
        name = os.path.join(qt_plugins_dir, 'sqldrivers', 'lib'+lib+'.so')
        shutil.copy2(name, dest_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(dest_folder, 'lib'+lib+'.so')])

def copy_resource_files():
    resource_dir = os.path.join(sigil_src, 'src', 'Resource_Files')

    # Copy the translation qm files
    trans_dir = os.path.join(cmake_build_root, 'src')
    dest_folder = os.path.join(app_folder, 'translations')
    if not os.path.exists(dest_folder):
        os.makedirs(dest_folder)
    filenames = glob.glob('*.qm')
    for filename in filenames:
        shutil.copy2(os.path.join(cmake_build_root, 'src', filename), dest_folder)

    # Copy the hunspell dictionary files
    dict_dir = os.path.join(resource_dir, 'dictionaries')
    dest_folder = os.path.join(app_folder, 'hunspell_dictionaries')
    shutil.copytree(dict_dir, dest_folder)

    # Copy the MathJax.js file
    mathjax_file = os.path.join(resource_dir, 'polyfills', "MathJax.js")
    dest_folder = os.path.join(app_folder, 'polyfills')
    if not os.path.exists(dest_folder):
        os.makedirs(dest_folder)
    shutil.copy2(mathjax_file, dest_folder)

    # plugin launcher files
    launcher_dir = os.path.join(resource_dir, 'plugin_launchers', 'python')
    dest_folder = os.path.join(app_folder, 'plugin_launchers', 'python')
    shutil.copytree(launcher_dir, dest_folder)

    # Copy the python3lib
    python3lib_dir = os.path.join(resource_dir, 'python3lib')
    dest_folder = os.path.join(app_folder, 'python3lib')
    shutil.copytree(python3lib_dir, dest_folder)

    # Copy the example files
    ex_dir = os.path.join(resource_dir, 'examples')
    dest_folder = os.path.join(app_folder, 'examples')
    shutil.copytree(ex_dir, dest_folder)

def copy_libs_bins():
    lib_src_dir = os.path.join(cmake_build_root, 'lib')
    bin_src_dir = os.path.join(cmake_build_root, 'bin')

    for lib in ['libhunspell.so', 'libsigilgumbo.so']:
        shutil.copy2(os.path.join(lib_src_dir, lib), app_folder)
        subprocess.check_call(['strip', '--strip-unneeded', os.path.join(app_folder, lib)])
    shutil.copy2(os.path.join(bin_src_dir, 'sigil'), app_folder)
    subprocess.check_call(['strip', '--strip-unneeded', os.path.join(app_folder, 'sigil')])
    subprocess.check_call(['chrpath', '-d', os.path.join(app_folder, 'sigil')])

def copy_misc_files():
    resource_dir = os.path.join(sigil_src, 'src', 'Resource_Files')

    # Copy the Unix launcher that sets the proper environment variables before launching the Sigil binary
    #shutil.copy2(os.path.join(cmake_build_root, 'sigil-sh_install_configured'), os.path.join(temp_folder, 'sigil.sh'))

    # Copy the Changelog
    shutil.copy2(os.path.join(sigil_src, 'ChangeLog.txt'), app_folder)

    # Copy the license file
    shutil.copy2(os.path.join(sigil_src, 'COPYING.txt'), app_folder)

    # Copy the icon file (used on Linux for the application icon)
    shutil.copy2(os.path.join(resource_dir, 'icon', 'app_icon_48.png'), temp_folder)

    # Copy the desktop file (used on Linux for the application settings)
    shutil.copy2(os.path.join(resource_dir, 'freedesktop', 'sigil.desktop'), temp_folder)

def copy_strip_extra_manifest(manifest_file):
    with open(manifest_file, 'r') as f:
        for line in f:
            filepath = line.strip()
            if len(filepath) and not filepath.startswith('#'):
                shutil.copy2(filepath, app_folder)
                try:
                    subprocess.check_call(['strip', '--strip-unneeded', os.path.join(app_folder, os.path.basename(filepath))])
                except:
                    pass

def create_launcher(filename, ldpath, perms):
    with open(os.path.join(temp_folder, filename), 'wb') as f:
        f.write(bytes(textwrap.dedent('''\
        #!/bin/sh

        # Entry point for Sigil on Unix systems.

        LIB_DIR=%s
        SIGIL_PREFS_DIR="$HOME/Documents/sigil_prefs"
        export SIGIL_PREFS_DIR

        if [ -z "$LD_LIBRARY_PATH" ]; then
          LD_LIBRARY_PATH="$LIB_DIR"
        else
          LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
        fi

        # Consolidate all of Sigil's files in one directory
        if [ -z "$SIGIL_EXTRA_ROOT" ]; then
          SIGIL_EXTRA_ROOT=$LIB_DIR
          export SIGIL_EXTRA_ROOT
        fi

        export LD_LIBRARY_PATH

        exec "$LIB_DIR/sigil" "$@"
        ''') % ldpath, 'UTF-8'))
    os.chmod(os.path.join(temp_folder, filename), perms)

def create_setup():
    with open(os.path.join(temp_folder, 'setup.sh'), 'wb') as f:
        f.write(bytes(textwrap.dedent('''\
        #!/bin/bash

        if [ $(id -u) -ne 0 ]; then
            DEST="$(getent passwd $USER | awk -F ':' '{print $6}')"
            LAUNCHER=./user_sigil.sh
            PYVENV=./user_pyvenv.cfg
            echo "home = $DEST/sigil-ebook/python3/pyvenv.cfg\\ninclude-system-site-packages = false\\nversion = 3.4.3" > "$PYVENV"
            DESKTOP="$DEST/.local/share/applications"
            perl -pi -e "s!Exec=!Exec=$DEST/bin/!g" ./sigil.desktop
            ICON="$DEST/.icons"
            BINDIR="$DEST/bin"
            #chmod -R g-xr,o-xr .
            MSG="Continue with the installation of Sigil to $DEST/sigil-ebook? (rerun the installer with root privileges to install Sigil system-wide)"
        else
            DEST=/opt
            LAUNCHER=./system_sigil.sh
            PYVENV=./system_pyvenv.cfg
            DESKTOP=/usr/share/applications
            ICON=/usr/share/pixmaps
            BINDIR=/usr/bin
            MSG="Continue with the installation of Sigil to /opt/sigil-ebook? (rerun the installer WITHOUT root privileges to install Sigil to your home directory)"
        fi

        read -r -p "$MSG [y/N] " response
        case $response in
            [yY][eE][sS]|[yY])
                if [ -d "$DEST/sigil-ebook" ]; then
                    rm -rf "$DEST/sigil-ebook"
                fi
                printf "\\nCopying files to $DEST/sigil-ebook ...\\n"
                \cp -rf ./sigil-ebook "$DEST/sigil-ebook"
                \cp -f "$LAUNCHER" "$DEST/sigil-ebook/sigil.sh"
                \cp -f "$PYVENV" "$DEST/sigil-ebook/python3/pyvenv.cfg"

                printf "\\nCreating desktop and icon entries ...\\n"
                if [ ! -d "$DESKTOP" ] && [ $(id -u) -ne 0 ]; then
                    mkdir -p "$DESKTOP"
                fi
                #perl -pi -e "s/Name=Sigil/Name=Sigil-App/g" ./sigil.desktop
                \cp -fv ./sigil.desktop "$DESKTOP/sigil.desktop"

                if [ ! -d "$ICON" ] && [ $(id -u) -ne 0 ]; then
                    mkdir -p "$ICON"
                fi
                \cp -fv ./app_icon_48.png "$ICON/sigil.png"

                printf "\\nCreating link(s) ...\\n"
                if [ ! -d "$BINDIR" ] && [ $(id -u) -ne 0 ]; then
                    mkdir -p "$BINDIR"
                fi
                ln -sfv "$DEST/sigil-ebook/sigil.sh" "$BINDIR/sigil"
                printf "\\nSigil installation complete.\\n"
                ;;
            *)
                printf "\\nSigil installation cancelled.\\n"
                exit 0
                ;;
        esac
        '''), 'UTF-8'))
    os.chmod(os.path.join(temp_folder, 'setup.sh'), 0o744)

if __name__ == '__main__':
    copy_pylib()
    copy_python()
    copy_tcltk()

    copy_strip_qt5()
    copy_strip_icu5()
    copy_strip_qt_plugins()
    copy_resource_files()
    copy_libs_bins()
    copy_misc_files()
    if len(extra_manifest) and os.path.exists(extra_manifest):
        copy_strip_extra_manifest(extra_manifest)
    create_launcher('user_sigil.sh', '~/sigil-ebook', 0o755)
    create_launcher('system_sigil.sh', '/opt/sigil-ebook', 0o755)
    create_pyvenv('user_pyvenv.cfg', '~/sigil-ebook')
    create_pyvenv('system_pyvenv.cfg', '/opt/sigil-ebook' )
    create_setup()
    subprocess.check_call(['makeself.sh', '--bzip2', temp_folder, os.path.join(cmake_build_root, 'installer',
                            installer_name+'.bz2.run'), 'Sigil Installer', './setup.sh'])
