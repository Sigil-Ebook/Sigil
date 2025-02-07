#.rst:
# FindPythonMacFramework
# ----------------------
#
# Find first Python.framework found in Path 
#
# This module finds if a Python.framework is installed and determines
# where the interpreter (executable) and libraries are.  
# This code sets the following variables:
#
# ::
#   PYTHONMACFRAMEWORK_FOUND
#   PYTHON_EXECUTABLE          - path to the Python interpreter
#   PYTHON_VERSION_STRING      - Python version found e.g. 3.13.2
#   PYTHON_VERSION_MAJOR       - Python major version found e.g. 3
#   PYTHON_VERSION_MINOR       - Python minor version found e.g. 13
#   PYTHON_VERSION_PATCH       - Python patch version found e.g. 2
#   PYTHON_LIBRARIES           - path to the python library
#   PYTHON_INCLUDE_DIRS        - path to where Python.h is found
#   PYTHONLIBS_VERSION_STRING  - version of the Python libs found 

set(_PYTHON_VERSIONS 3.14 3.13 3.12 3.11 3.10 3.9)

find_program(PYTHON_EXECUTABLE python3 PATHS ENV PATH)

# determine python version string
if(PYTHON_EXECUTABLE)
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" -c
                            "import sys; sys.stdout.write(';'.join([str(x) for x in sys.version_info[:3]]))"
                    OUTPUT_VARIABLE _VERSION
                    RESULT_VARIABLE _PYTHON_VERSION_RESULT
                    ERROR_QUIET)

    if(NOT _PYTHON_VERSION_RESULT)
        list(GET _VERSION 0 _PYTHON_VERSION_MAJOR)
        list(GET _VERSION 1 _PYTHON_VERSION_MINOR)
        list(GET _VERSION 2 _PYTHON_VERSION_PATCH)
        list(JOIN _VERSION "."  _PYTHON_VERSION_STRING)
        set(_VERSION_STRING "${_PYTHON_VERSION_MAJOR}.${_PYTHON_VERSION_MINOR}")
        unset(_VERSION)
        unset(_PYTHON_VERSION_RESULT)
        # it must be an acceptable version
        list( FIND _PYTHON_VERSIONS ${_VERSION_STRING} _ACC_VER)
        if ( ${_ACC_VER} GREATER -1 ) 
            # determine if this is a Python.framework
            # ie. /Library/Frameworks/Python.framework/Versions/3.13/bin/python3
            # ie. /Users/kbhend/ndevpython/libraries/Frameworks/Python.framework/Versions/3.13/bin/python3

            string(REPLACE "/" ";" _PATH_SEGMENTS ${PYTHON_EXECUTABLE})
            set(_PATH_LIST ${_PATH_SEGMENTS} )
            unset(_PATH_SEGMENTS)
            # message("Excutable Path: " ${PYTHON_EXECUTABLE} )
            list( FIND _PATH_LIST "Python.framework" _ISFWK )
            if( ${_ISFWK} GREATER -1 )
                # message("Found framework: " ${_ISFWK} )
                set(PYTHON_VERSION_MAJOR ${_PYTHON_VERSION_MAJOR})
                set(PYTHON_VERSION_MINOR ${_PYTHON_VERSION_MINOR})
                set(PYTHON_VERSION_PATCH ${_PYTHON_VERSION_PATCH})
                set(PYTHON_VERSION_STRING ${_PYTHON_VERSION_STRING})
                list(POP_BACK _PATH_LIST) # remove python3
                list(POP_BACK _PATH_LIST) # remove bin
                list(POP_BACK _PATH_LIST _CURRENT_VERSION)
                list(POP_BACK _PATH_LIST) # remove Versions
                list(JOIN _PATH_LIST "/" PYTHON_FRAMEWORK_BASE)
                # Remember to add back the leading "/" since these are absolute paths
                set(PYTHON_LIBRARIES "/${PYTHON_FRAMEWORK_BASE}/Versions/${_CURRENT_VERSION}/lib/libpython${_VERSION_STRING}.dylib")
                set(PYTHON_INCLUDE_DIRS  "/${PYTHON_FRAMEWORK_BASE}/Versions/${_CURRENT_VERSION}/include/python${_VERSION_STRING}")
                # message("PYTHON_LIBRARIES: " ${PYTHON_LIBRARIES})
                # message("PYTHON_INCLUDE_DIRS: " ${PYTHON_INCLUDE_DIRS})
            endif()
            unset(_PYTHON_VERSION_MAJOR)
            unset(_PYTHON_VERSION_MINOR)
            unset(_PYTHON_VERSION_PATCH)
            unset(_VERSION_STRING)
            unset(_PATH_LIST)
        endif()
    endif()
endif()  

# handle the QUIETLY and REQUIRED arguments and set PYTHONMACFRAMEWORK_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonMacFramework REQUIRED_VARS PYTHON_EXECUTABLE 
                                                                   PYTHON_VERSION_MAJOR 
                                                                   PYTHON_VERSION_MINOR
                                                                   PYTHON_VERSION_PATCH
                                                                   PYTHON_LIBRARIES
                                                                   PYTHON_INCLUDE_DIRS 
                                                     VERSION_VAR PYTHON_VERSION_STRING)
mark_as_advanced(PYTHON_EXECUTABLE)
mark_as_advanced(PYTHON_LIBRARIES)
mark_as_advanced(PYTHON_INCLUDE_DIRS)
