if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/sigilpy")
    find_package(Python3 3.9 COMPONENTS Interpreter)
    set(REQUIREMENTS "${PROJECT_SOURCE_DIR}/src/Resource_Files/python_pkg/winreqs.txt")
    # Create venv with system python
    message(STATUS "Creating virtual python environment in ${CMAKE_CURRENT_BINARY_DIR}/sigilpy")
    execute_process(COMMAND "${Python3_EXECUTABLE}" -m venv "${CMAKE_CURRENT_BINARY_DIR}/sigilpy")
    # Pip install -r requirements.txt into venv
    message(STATUS "Downloading/installing required python modules into venv using pip requirements file: ${REQUIREMENTS}")
    execute_process(COMMAND "${CMAKE_CURRENT_BINARY_DIR}/sigilpy/Scripts/python.exe" -m pip install -r "${REQUIREMENTS}")
    # Unset system python so venv python can be found
    unset(Python3_EXECUTABLE)
else()
    # Try to reuse venv if already present
    message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/sigilpy already exists. Will try to use it.")
    set (Python3_EXECUTABLE  "${CMAKE_CURRENT_BINARY_DIR}/sigilpy/Scripts/python.exe")
endif()
