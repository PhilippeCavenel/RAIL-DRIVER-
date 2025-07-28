# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\RailManagerUI_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\RailManagerUI_autogen.dir\\ParseCache.txt"
  "QtNodesLibrary_build\\CMakeFiles\\QtNodes_autogen.dir\\AutogenUsed.txt"
  "QtNodesLibrary_build\\CMakeFiles\\QtNodes_autogen.dir\\ParseCache.txt"
  "QtNodesLibrary_build\\QtNodes_autogen"
  "RailManagerUI_autogen"
  )
endif()
