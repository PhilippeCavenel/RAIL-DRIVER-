if(DEFINED CMAKE_PROJECT_INCLUDE_BEFORE AND CMAKE_PROJECT_INCLUDE_BEFORE MATCHES ".qtc/package-manager/auto-setup.cmake")
    unset(CMAKE_PROJECT_INCLUDE_BEFORE CACHE)
endif()

cmake_minimum_required(VERSION 3.11)
project(RailDriverSuperProject)

add_subdirectory(QtNodesLibrary)
add_subdirectory(RailManagerUI)

