# Modified from FindReadline.cmake (PH Feb 2012)

if(EDITLINE_INCLUDE_DIR AND EDITLINE_LIBRARY)
  set(EDITLINE_FOUND TRUE)
else()
  find_path(EDITLINE_INCLUDE_DIR readline.h PATH_SUFFIXES editline edit/readline)

  find_library(EDITLINE_LIBRARY NAMES edit)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Editline DEFAULT_MSG EDITLINE_INCLUDE_DIR EDITLINE_LIBRARY)

  mark_as_advanced(EDITLINE_INCLUDE_DIR EDITLINE_LIBRARY)
endif()
