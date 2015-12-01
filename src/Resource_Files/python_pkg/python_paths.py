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
qt_libs_dir = '${QT_LIBRARY_DIR}'
qt_plugins_dir = '${QT_PLUGINS_DIR}'
pkg_name = '${PKG_NAME_PREFIX}'
sigil_src = '${CMAKE_SOURCE_DIR}'
installer_name = '${PKG_NAME_PREFIX}'
tk_lib = '${TK_LIBRARY}'
tcl_lib = '${TCL_LIBRARY}'
tcltk_support='${TCLTK_SUPPORT}'
