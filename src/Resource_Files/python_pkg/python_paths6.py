#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

import sys

using_new_findpython3 = bool(${USE_NEWER_FINDPYTHON3})
py_ver = ''.join(map(str, sys.version_info[:2]))
if using_new_findpython3:
    py_exe = '${Python3_EXECUTABLE}'
    py_lib_temp = '${Python3_LIBRARIES}'
    py_inc = '${Python3_INCLUDE_DIRS}'
else:
    py_exe = '${PYTHON_EXECUTABLE}'
    py_lib_temp = '${PYTHON_LIBRARIES}'
    py_inc = '${PYTHON_INCLUDE_DIRS}'

if not sys.platform.startswith('win') and not sys.platform.startswith('darwin'):    
    if not py_lib_temp.endswith('.1.0'):
        py_lib_temp = py_lib_temp + '.1.0'
py_lib = py_lib_temp
sys_dlls = '${SYS_DLL_DIR}'
py_dest = '${PYTHON_DEST_DIR}'
tmp_prefix = '${MAIN_PACKAGE_DIR}'
proj_name = '${PROJECT_NAME}'
cmake_build_root = '${CMAKE_BINARY_DIR}'
include_pyside6 = bool(${PACKAGE_PYSIDE6})
