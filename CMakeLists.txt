cmake_minimum_required(VERSION 2.6)
project(mt-translator)

include(FindPkgConfig)
pkg_check_modules(MTDEV REQUIRED mtdev)

link_directories(${MTDEV_STATIC_LIBRARY_DIRS})
include_directories(${MTDEV_STATIC_INCLUDE_DIRS})

add_definitions(-DPACKAGE_NAME="mt-translator" -DPACKAGE_VERSION="0.1")

add_subdirectory(src)
