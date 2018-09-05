cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Relase)
endif()

project(flex LANGUAGES C)

# Create config.h
include(make_config_h)

# Build options
option(ENABLE_NLS "Enable NLS" FALSE)
if(ENABLE_NLS)
  add_definitions(-DENABLE_NLS -DYYENABLE_NLS)
endif()
option(ENABLE_C_ALLOCA "Enable alloca.c" FALSE)
option(ENABLE_MEM_SHIM "Replacement malloc, realloc" FALSE)

# Locate M4 path if not set
if(NOT M4_PATH)
  if(UNIX OR APPLE)
    set(SEARCH_CMD which)
  elseif(MINGW OR CYGWIN OR WIN32)
    set(SEARCH_CMD where)
  endif()

  execute_process(
    COMMAND ${SEARCH_CMD} m4
    OUTPUT_VARIABLE M4_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
MESSAGE(STATUS "Using M4: ${M4_PATH}")

# compiler specifics
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-D_GNU_SOURCE)
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
  add_definitions(-D_CONSOLE -D_MBCS -D_CRT_SECURE_NO_WARNINGS)
endif()

# generate parse.c
include(FindBISON)
bison_target(parse src/parse.y ${CMAKE_BINARY_DIR}/gen/parse.c)

# generate scan.c
include(FindFLEX)
FLEX_TARGET(scan src/scan.l ${CMAKE_BINARY_DIR}/gen/scan.c)

# generate skel.c
string(REPLACE ";" "" STRIPPED_VERSION "${VERSION}")
include(FindUnixCommands)
if(BASH)
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/gen/skel.c
  COMMAND ${BASH} -c "${CMAKE_SOURCE_DIR}/src/mkskel.sh ${CMAKE_SOURCE_DIR}/src ${M4_PATH} ${STRIPPED_VERSION}>skel.c"
  DEPENDS ${CMAKE_SOURCE_DIR}/src/mkskel.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gen
  COMMENT "[m4][flex.skl] Creating skel.c"
)
# TODO make this step target generic via CMake script
endif()

set(GENERATED_SOURCES
  ${BISON_parse_OUTPUT_SOURCE}
  ${FLEX_scan_OUTPUTS}
  ${CMAKE_BINARY_DIR}/gen/skel.c
)

# inlcude files
include_directories(src)
include_directories(${CMAKE_BINARY_DIR}/gen)

# source files
set(sources
  src/buf.c src/ccl.c src/dfa.c src/ecs.c
  src/scanflags.c src/filter.c src/flexdef.h
  src/gen.c src/misc.c src/nfa.c
  src/options.c src/options.h src/regex.c
  src/scanopt.c src/scanopt.h src/sym.c
  src/tables.c src/tables.h 
  src/tables_shared.c src/tables_shared.h
  src/tblcmp.c src/yylex.c
)

if(ENABLE_MEM_SHIM)
  list(APPEND sources lib/malloc.c lib/realloc.c)
endif()

# command line
add_executable(flex ${sources} src/main.c ${GENERATED_SOURCES})
add_library(fl SHARED ${sources} ${GENERATED_SOURCES})
if(UNIX AND NOT ANDROID)
  target_link_libraries(flex -lm)
  target_link_libraries(fl -lm)
  set(PKGCONFIG_LIBS "-lm -lc")
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS flex DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS fl DESTINATION ${CMAKE_INSTALL_LIBDIR})

# pkgconfig
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
configure_file(${CMAKE_SOURCE_DIR}/src/libfl.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libfl.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libfl.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# CMake
configure_package_config_file(cmake/FlexConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FlexConfigVersion.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/flex/cmake)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/FlexConfigVersion.cmake
    VERSION ${PACKAGE_VERSION} COMPATIBILITY ExactVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FlexConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/flex/cmake )

# uninstall
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/cmake/make_uninstall.cmake")
