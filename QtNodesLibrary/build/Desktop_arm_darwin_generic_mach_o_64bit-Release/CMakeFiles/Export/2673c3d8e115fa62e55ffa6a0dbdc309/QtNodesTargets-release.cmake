#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "QtNodes::QtNodes" for configuration "Release"
set_property(TARGET QtNodes::QtNodes APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(QtNodes::QtNodes PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libQtNodes.dylib"
  IMPORTED_SONAME_RELEASE "@rpath/libQtNodes.dylib"
  )

list(APPEND _cmake_import_check_targets QtNodes::QtNodes )
list(APPEND _cmake_import_check_files_for_QtNodes::QtNodes "${_IMPORT_PREFIX}/lib/libQtNodes.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
