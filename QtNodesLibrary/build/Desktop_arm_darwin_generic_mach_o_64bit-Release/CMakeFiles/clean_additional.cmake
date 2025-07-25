# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/QtNodes_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/QtNodes_autogen.dir/ParseCache.txt"
  "QtNodes_autogen"
  )
endif()
