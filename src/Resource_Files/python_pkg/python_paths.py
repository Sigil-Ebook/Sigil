#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

import sys

py_ver = ''.join(map(str, sys.version_info[:2]))
py_exe = '${PYTHON_EXECUTABLE}'
py_lib_temp = '${PYTHON_LIBRARIES}'
if not sys.platform.startswith('win') and not sys.platform.startswith('darwin'):    
    if not py_lib_temp.endswith('.1.0'):
        py_lib_temp = py_lib_temp + '.1.0'
py_lib = py_lib_temp
py_inc = '${PYTHON_INCLUDE_DIRS}'
sys_dlls = '${SYS_DLL_DIR}'
py_dest = '${PYTHON_DEST_DIR}'
tmp_prefix = '${MAIN_PACKAGE_DIR}'
proj_name = '${PROJECT_NAME}'
cmake_build_root = '${CMAKE_BINARY_DIR}'

# Only used by Linux installer packaging from this point on.
qt_libs_dir = '${QT_LIBRARY_DIR}'
qt_plugins_dir = '${QT_PLUGINS_DIR}'
sigil_src = '${CMAKE_SOURCE_DIR}'
installer_name = '${PKG_NAME_PREFIX}'

# Manually define these in the cmake command ( i.e. -DTK_LIBRARY=/usr/local/lib/libtk86.so)
tk_lib = '${TK_LIBRARY}'           # the path to libtk8x.so
tcl_lib = '${TCL_LIBRARY}'         # the path to libtcl8x.so
tcltk_support='${TCLTK_SUPPORT}'   # the prefix to the tcl8x and tk8x directories (contains *.tcl files)

# *optional* full path to a file that contains additional files to include (one filepath per line)
extra_manifest='${EXTRA_MANIFEST}'
