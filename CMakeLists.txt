cmake_minimum_required(VERSION 2.6)
project(mt-translator)

include(FindPkgConfig)
pkg_check_modules(MTDEV REQUIRED mtdev)

link_directories(${MTDEV_STATIC_LIBRARY_DIRS})
include_directories(${MTDEV_STATIC_INCLUDE_DIRS})

include(CheckIncludeFiles)

check_include_files("linux/uinput.h" DETECTED_HAVE_LINUX_UINPUT_H)
option(WITH_UINPUT "compile uinput backend" "OFF")

if(NOT DETECTED_HAVE_LINUX_UINPUT_H AND WITH_UINPUT)
	message(FATAL_ERROR "can't find linux/uinput.h which is required for the uinput backend")
endif()

if(WITH_UINPUT)
	set(DETECTED_HAVE_LINUX_UINPUT_H 1)
endif()

configure_file(config.h.cmake-in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DPACKAGE_NAME="mt-translator" -DPACKAGE_VERSION="0.1" -D_POSIX_C_SOURCE=200809L)

set(CMAKE_C_FLAGS "-std=c99")

add_subdirectory(src)
