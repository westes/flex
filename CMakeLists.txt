cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

project(flex LANGUAGES C)

include(make_config_h)

option(ENABLE_NLS "Enable NLS" FALSE)
option(ENABLE_C_ALLOCA "Enable alloca.c" FALSE)

# GNU M4 macro processor
# where vs which
if(NOT M4)
  set(M4 "/usr/bin/m4")
endif()

file(GLOB sources src/*.c)
include_directories(src)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-D_GNU_SOURCE)
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
  add_definitions(-D_CONSOLE -D_MBCS -D_CRT_SECURE_NO_WARNINGS)
  list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/libmain.c)
  list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/libyywrap.c)
endif()

find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
  add_definitions(-DHAVE_PTHREAD)
endif (CMAKE_USE_PTHREADS_INIT)

add_executable(flex ${sources})
