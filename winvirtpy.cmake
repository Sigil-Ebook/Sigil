if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/sigilpy")
    find_package(Python3 3.9 COMPONENTS Interpreter)
    set(REQUIREMENTS "${PROJECT_SOURCE_DIR}/src/Resource_Files/python_pkg/winreqs.txt")
    configure_file(${REQUIREMENTS} ${CMAKE_CURRENT_BINARY_DIR}/requirements.txt)
    # Create venv with system python
    message(STATUS "Creating virtual python environment in ${CMAKE_CURRENT_BINARY_DIR}/sigilpy")
    execute_process(COMMAND "${Python3_EXECUTABLE}" -m venv "${CMAKE_CURRENT_BINARY_DIR}/sigilpy")
    # Update venv pip
    execute_process(COMMAND "${CMAKE_CURRENT_BINARY_DIR}/sigilpy/Scripts/python.exe" -m pip install -U pip)
    # Pip install -r requirements.txt into venv
    message(STATUS "Downloading/installing required python modules into venv using pip requirements file: ${CMAKE_CURRENT_BINARY_DIR}/requirements.txt")
    execute_process(COMMAND "${CMAKE_CURRENT_BINARY_DIR}/sigilpy/Scripts/python.exe" -m pip install -r "${CMAKE_CURRENT_BINARY_DIR}/requirements.txt")
    # Unset system python so venv python can be found
    unset(Python3_EXECUTABLE)
else()
    # Try to reuse venv if already present
    message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/sigilpy already exists. Will try to use it.")
    set (Python3_EXECUTABLE  "${CMAKE_CURRENT_BINARY_DIR}/sigilpy/Scripts/python.exe")
endif()
