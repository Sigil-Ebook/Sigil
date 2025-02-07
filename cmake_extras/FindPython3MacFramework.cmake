#.rst:
# FindPython3MacFramework
# ----------------------
#
# Find first Python.framework from version 3 found in your system PATH
#
# This module finds if a Python.framework is installed and determines
# where the interpreter (executable) and libraries are.  
# This code sets the following variables:
#
# ::
#   Python3MacFramework_FOUND
#   Python3_EXECUTABLE         - path to the Python interpreter
#   Python3_LIBRARIES          - path to the python library
#   Python3_INCLUDE_DIRS       - path to where Python.h is found
#   Python3_VERSION_STRING     - version of the Python libs found 

set(_PYTHON_VERSIONS 3.14 3.13 3.12 3.11 3.10 3.9)

find_program(_PYTHON_EXECUTABLE python3 PATHS ENV PATH)

# determine python version string
if(_PYTHON_EXECUTABLE)
    execute_process(COMMAND "${_PYTHON_EXECUTABLE}" -c
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
        # it must be an acceptable version
        list( FIND _PYTHON_VERSIONS ${_VERSION_STRING} _ACC_VER)
        if ( ${_ACC_VER} GREATER -1 )
            # determine if this is a Python.framework
            # ie. /Library/Frameworks/Python.framework/Versions/3.13/bin/python3
            # ie. /Users/kbhend/ndevpython/libraries/Frameworks/Python.framework/Versions/3.13/bin/python3

            string(REPLACE "/" ";" _PATH_SEGMENTS ${_PYTHON_EXECUTABLE})
            set(_PATH_LIST ${_PATH_SEGMENTS} )
            # message("Excutable Path: " ${_PYTHON_EXECUTABLE} )
            list( FIND _PATH_LIST "Python.framework" _ISFWK )
            if( ${_ISFWK} GREATER -1 )
                # message("Found framework: " ${_ISFWK} )
                list(POP_BACK _PATH_LIST) # remove python3
                list(POP_BACK _PATH_LIST) # remove bin
                list(POP_BACK _PATH_LIST _CURRENT_VERSION)
                list(POP_BACK _PATH_LIST) # remove Versions
                list(JOIN _PATH_LIST "/" _PYTHON_FRAMEWORK_BASE)

                # Now we have found a valid Python3 Framework from the PATH
                set(Python3_VERSION_STRING ${_PYTHON_VERSION_STRING})
                set(Python3_EXECUTABLE "${_PYTHON_EXECUTABLE}")
                # Remember to add back the leading "/" since these are absolute paths
                set(Python3_LIBRARIES "/${_PYTHON_FRAMEWORK_BASE}/Versions/${_CURRENT_VERSION}/lib/libpython${_VERSION_STRING}.dylib")
                set(Python3_INCLUDE_DIRS  "/${_PYTHON_FRAMEWORK_BASE}/Versions/${_CURRENT_VERSION}/include/python${_VERSION_STRING}")
                # message("Python3_LIBRARIES: " ${Python3_LIBRARIES})
                # message("Python3_INCLUDE_DIRS: " ${Python3_INCLUDE_DIRS})
                # clean up
                unset(_CURRENT_VERSION)
                unset(_PYTHON_VERSION_STRING)
                unset(_PYTHON_FRAMEWORK_BASE)
            endif()
            # clean up
            unset(_IS_FWK)
            unset(_PATH_SEGMENTS)
            unset(_PATH_LIST)
        endif()
        # clean up
        unset(_PYTHON_VERSION_MAJOR)
        unset(_PYTHON_VERSION_MINOR)
        unset(_PYTHON_VERSION_PATCH)
        unset(_VERSION_STRING)
        unset(_ACC_VER)
    endif()
    # clean up
    unset(_VERSION)
    unset(_PYTHON_VERSION_RESULT)
endif()  

# cleanup
unset(_PYTHON_VERSIONS)
unset(_PYTHON_EXECUTABLE)

# handle the QUIETLY and REQUIRED arguments and set Python3MacFramework_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python3MacFramework REQUIRED_VARS Python3_EXECUTABLE 
                                                                    Python3_LIBRARIES
                                                                    Python3_INCLUDE_DIRS 
                                                      VERSION_VAR Python3_VERSION_STRING)
mark_as_advanced(Python3_EXECUTABLE)
mark_as_advanced(Python3_LIBRARIES)
mark_as_advanced(Python3_INCLUDE_DIRS)
